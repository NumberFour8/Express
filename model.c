#include "model.h"
#include "drawing.h"

// Funkce pro práci s vektory
Vector2 addVectors(Vector2 A,Vector2 B)
{ Vector2 C = {A.x+B.x,A.y+B.y}; return C; }

Vector2 multiplyVector(Vector2 A,float B)
{ Vector2 C = {A.x*B,A.y*B}; return C; }

int magnitudeSquared(Vector2 V)
{ return V.x*V.x+V.y*V.y; }

// Zjisti smìrový jednotkový vektor spojnice dvou vrcholù
Vector2 getDirection(Vertex* A,Vertex* B)
{ 
	Vector2 AB = {B->position.x-A->position.x,B->position.y-A->position.y};
	float mag = sqrt(magnitudeSquared(AB));
	if (mag < 5) mag = 5;
	return multiplyVector(AB,1/mag); 
}

// Získá adresu vrcholu podle ID
Vertex* GetVertexAddressById(Vertex* pVertices,unsigned int Size,unsigned int ID)
{
	if (!pVertices) return NULL;
	
	for (unsigned int i = 0;i < Size;++i){
	   if (pVertices[i].id == ID)
	     return (pVertices+i);
	}
	return NULL;
}

// Spoète kvadrát vzdálenosti dvou vrcholù (tj. délku hrany)
float GetVertexDistanceSquared(Vertex* A,Vertex* B)
{
	return pow(A->position.x-B->position.x,2)+pow(A->position.y-B->position.y,2);
}

// Sestaví model podle daného GML souboru
int BuildModel(const char* szFile,Model* pModel)
{
	// Zkus otevøít soubor s grafem
	FILE* fp = fopen(szFile,"r");
	if (!fp){
	  fprintf(stderr,"Unable to open %s for reading.\n",szFile);
	  return 0;
	}
	
	// Naalokuj potøebné struktury a oparsuj soubor
	struct GML_stat* stat=(struct GML_stat*)malloc(sizeof(struct GML_stat));
	stat->key_list = NULL;
	
	struct GML_pair* list = GML_parser(fp, stat, 0);
	
	// Zkontroluje, zda nenastala chyba pøi parsování
	if (stat->err.err_num != GML_OK){
	  fprintf(stderr,"Error reading GML file on %d : %d\n", stat->err.line, stat->err.column);
	  fclose(fp);
	  free(stat);
	  return 0;
	}
	
	// Zinicializuj promìnné parseru
	struct GML_pair* ptr = list,*inner = NULL;
	int nEdges = 0,nVertices = 0;
	memset((void*)pModel,0,sizeof(Model));
	
	// Vybírej vrcholy
	for (;ptr != NULL;ptr=ptr->next){
		if (strcmp(ptr->key,"graph") == 0 && ptr->kind == GML_LIST && ptr->value.list != NULL)
		  ptr = ptr->value.list;
		if (strcmp(ptr->key,"node") == 0 && ptr->kind == GML_LIST){
		  // Realokuj pole vrcholù na novou velikost
		  pModel->pVertices = (Vertex*)realloc((void*)pModel->pVertices,(nVertices+1)*sizeof(Vertex));
		  Vertex* NewV = (pModel->pVertices)+nVertices;
		  memset((void*)NewV,0,sizeof(Vertex));
		  NewV->charge = 1;
		  ++nVertices;
		  
		  // Èti hodnoty pøíslu¹ící k vrcholu
		  inner = ptr->value.list;
		  for (;inner != NULL;inner=inner->next){
			if (strcmp(inner->key,"id") == 0 && inner->kind == GML_INT)
				NewV->id = (unsigned int)inner->value.integer;
			else if (strcmp(inner->key,"label") == 0 && inner->kind == GML_STRING)
			  memcpy((void*)NewV->szVertexName,inner->value.string,strlen(inner->value.string) > MAX_LENGTH_LABEL ? MAX_LENGTH_LABEL : strlen(inner->value.string));
			else continue; 
		  }
		}
		else if (strcmp(ptr->key,"edge") == 0 && ptr->kind == GML_LIST){
		  // Realokuj pole hran na novou velikost
		  pModel->pEdges = (Edge*)realloc((void*)pModel->pEdges,(nEdges+1)*sizeof(Edge));
		  Edge* NewE = (pModel->pEdges)+nEdges;
		  memset((void*)NewE,0,sizeof(Edge));
		  ++nEdges;
		  
		  // Èti hodnoty pøíslu¹ící k hranì
		  inner = ptr->value.list;
		  for (;inner != NULL;inner=inner->next){
			if (strcmp(inner->key,"source") == 0 && inner->kind == GML_INT)
			   NewE->idFrom = (unsigned int)inner->value.integer;
			else if (strcmp(inner->key,"target") == 0 && inner->kind == GML_INT)
			   NewE->idTo = (unsigned int)inner->value.integer;
			else if (strcmp(inner->key,"value") == 0 && inner->kind == GML_INT)
			   NewE->value = (unsigned int)inner->value.integer;
			else continue;
		  }
		}
		else continue;
	}
	
	// Uvolni pamìt asociovanou s parserem a uzavøi soubor
	GML_free_list(list, stat->key_list);
	free(stat);
	fclose(fp);
	
	// ... zapi¹ velikosti polí
	pModel->uCountVertices = (unsigned int)nVertices;
	pModel->uCountEdges = (unsigned int)nEdges;

	// Zkontroluj jestli byl pøeèten alespoò jeden vrchol
	if (nVertices == 0){
	  fprintf(stderr,"No vertices read from the GML file. No simulation possible.\n");	
	  FreeModel(pModel);
	  return 0;
	}
	
	// Teï u¾ máme v¹echny vrcholy, proveï napojení hran na vrcholy
	for (int i = 0;i < nEdges;++i){
	  Edge* c = (pModel->pEdges+i);
	  if (c->idTo == c->idFrom){ // Nedovol cyklické hrany!
	    fprintf(stderr,"Invalid GML file - cyclic edges are not supported!\n");
	    FreeModel(pModel);
	    return 0;
	  }
	  // Najdi adresy vrcholù podle jejich ID
	  c->pTo = GetVertexAddressById(pModel->pVertices,nVertices,c->idTo);
	  c->pFrom = GetVertexAddressById(pModel->pVertices,nVertices,c->idFrom);
	  if (c->pTo == NULL || c->pFrom == NULL){
	    fprintf(stderr,"Invalid GML file - vertices in edge not found!\n");
	    FreeModel(pModel);
	    return 0;
	  }
	}
	
	return 1;
}

// Uvolní pøeètený model z pamìti
void FreeModel(Model* pModel)
{
	// Je tøeba uvolnit v¹echny povrchy
	for (unsigned int i = 0;i < pModel->uCountVertices;++i){
	   if (pModel->pVertices[i].VertexSurface) 
	     SDL_FreeSurface(pModel->pVertices[i].VertexSurface);
	}

	// Uvolni v¹echny vrcholy a hrany
	if (pModel->pVertices)
	  free(pModel->pVertices);
	if (pModel->pEdges)
	  free(pModel->pEdges);
}

// Vytvoøí povrchy s vrcholy a jejich názvy
int CreateModelSurfaces(Model* pModel,GraphicCfg *Config)
{
	// Zkontroluj zda se nejedná o prázdný model
	if (pModel->uCountVertices == 0 || !pModel->pVertices)
	  return 0;
	
	const int Gap = 2; // Mezera mezi kruhem a textem
	
	// Zkus otevøít daný font
	TTF_Font* LabelFont = TTF_OpenFont((char*)Config->szFontFile, Config->uFontSize);
	if (!LabelFont){
	  fprintf(stderr,"Cannot open font file %s\n",(char*)Config->szFontFile);
	  return 0;
	}
	
	int hLabel = 0,wLabel = 0,lineSkip = 0,realWidth = 0,realHeight = 0,maxW = 0,maxH = 0;
	for (unsigned int i = 0;i < pModel->uCountVertices;++i){
		// Zjisti reálnou velikost fontu
		TTF_SizeText(LabelFont,(char*)pModel->pVertices[i].szVertexName,&wLabel,&hLabel);
		lineSkip = TTF_FontLineSkip(LabelFont);
		
		// Spoèti reálnou velikost povrchu a souèastnì hledej jejich maximum
		realWidth = ((Config->uNodeRadius*2+3) > wLabel ? (Config->uNodeRadius*2+3) : wLabel+3);
		realHeight = Config->uNodeRadius*2+2+hLabel+lineSkip+Gap;
		if (realWidth > maxW) maxW = realWidth;
		if (realHeight > maxH) maxH = realHeight;
		
		// Zkus vytvoøit nový povrch
		pModel->pVertices[i].VertexSurface = SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA,realWidth,realHeight,
								 	   Config->uBPP,0x000000FF,0x0000FF00,0x00FF0000,0xFF000000);
		SDL_Surface *c = pModel->pVertices[i].VertexSurface;
		
		if (!c){ // Nepodaøilo se vytvoøit povrch, v¹e uvolni a skonèi
		  TTF_CloseFont(LabelFont);
		  fprintf(stderr,"Vertex surface allocation failed.\n");
		  return 0;
		}
		
		// Vyèisti povrch a namapuj barvy
		SDL_FillRect(c, 0, SDL_MapRGBA(c->format,0,0,0,0));
		unsigned int inner = SDL_MapRGBA(c->format, Config->innerCircle.r,Config->innerCircle.g,Config->innerCircle.b,0xff),
			     outter = SDL_MapRGBA(c->format, Config->outterCircle.r,Config->outterCircle.g,Config->outterCircle.b,0xff);

		// Uzamkni povrch a vykresli koleèko
		doLock(c);
		FillCircle(c,realWidth/2,Config->uNodeRadius,Config->uNodeRadius,inner,outter);
		doUnlock(c);
		
		// ... spoleènì s textem		
		SDL_Surface *Text = TTF_RenderText_Solid(LabelFont,(char*)pModel->pVertices[i].szVertexName,Config->fontColor);
		DrawSurface(Text,c,2,Config->uNodeRadius*2+2+Gap);
		SDL_Flip(c);
		
		SDL_FreeSurface(Text);
	}
	Config->uMaxSurfaceW = maxW; Config->uMaxSurfaceH = maxH;
	
	// Zavøi font a skonèi
	TTF_CloseFont(LabelFont);
	
	return 1;
}

// Rozestaví vrcholy na náhodné pozice
void SetRandomLocations(Model* pModel,const GraphicCfg Config)
{
	// Zkontroluj zda se nejedná o prázdný model
	if (pModel->uCountVertices == 0 || !pModel->pVertices)
	  return;

	// Najdi náhodné rozmístìní v oknì
	for (unsigned int i = 0;i < pModel->uCountVertices;++i){
	   (pModel->pVertices+i)->position.x = (float)(Config.uScreenWidth/4+rand()%(Config.uScreenWidth/2-Config.uMaxSurfaceW));
	   (pModel->pVertices+i)->position.y = (float)(Config.uScreenHeight/4+rand()%(Config.uScreenHeight/2-Config.uMaxSurfaceH));
	}
}

// Provede jeden simulaèní krok
unsigned int SimulationStep(Model* pModel,const SimulationCfg Config)
{
	// Zkontroluj zda se nejedná o prázdný model
	if (pModel->uCountVertices == 0 || !pModel->pVertices)
	  return 0;
	  
	unsigned int uEnergy = 0;
	float distSq = 0,CoulombForce = 0,SpringForce = 0;

	Vector2 totalForce;
	Vertex *a,*b;

	// Procházej v¹echny dvojice vrcholù
	for (int i = 0;i < pModel->uCountVertices;++i){
	  memset(&totalForce,0,sizeof(Vector2));
	  a = (pModel->pVertices+i);
	  
	  for (int j = 0;j < pModel->uCountVertices;++j){
    	    if (i == j) continue;
    	    b = (pModel->pVertices+j);
	    distSq = GetVertexDistanceSquared(a,b);
	    if (distSq < 5) distSq = 5;

	    // Spoèti columbickou sílu
	    CoulombForce = Config.fCoulombConstant*b->charge*a->charge/distSq;
	    totalForce = addVectors(totalForce,multiplyVector(getDirection(a,b),-CoulombForce));

	    // Najdi spoleèné hrany a spoèti sílu pru¾nosti
	    for (int k = 0;k < pModel->uCountEdges;++k){
		if ((pModel->pEdges[k].pFrom == a && pModel->pEdges[k].pTo == b) ||
		    (pModel->pEdges[k].pTo == a && pModel->pEdges[k].pFrom == b)){
		  SpringForce = Config.fSpringConstant*sqrt(distSq);
	    	  totalForce = addVectors(totalForce,multiplyVector(getDirection(a,b),SpringForce));    
		}
	    }
	  }	  
	  
	  // Integruj eulerovsky
	  a->velocity = multiplyVector(addVectors(a->velocity,multiplyVector(totalForce,Config.fSimStep)),Config.fDamping);
	  a->position = addVectors(a->position,multiplyVector(a->velocity,Config.fSimStep));

	  // Spoèti a vra» kinetickou energii systému (hmotnost èásti je jednotková)
	  uEnergy += magnitudeSquared(a->velocity);
	}

	return uEnergy;
}

