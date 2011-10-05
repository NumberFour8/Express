#include "model.h"
#include "drawing.h"

// Funkce pro práci s vektory
Vector2 addVectors(Vector2 A,Vector2 B)
{ Vector2 C = {A.x+B.x,A.y+B.y}; return C; }
Vector2 multiplyVector(Vector2 A,float B)
{ Vector2 C = {(int)A.x*B,(int)A.y*B}; return C; }
int magnitudeSquared(Vector2 V)
{ return V.x*V.x+V.y*V.y; }

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

// Získá adresy všech bezprostøedních sousedù
Vertex** GetAllNeighbours(Model* pModel,Vertex* pVertex,unsigned int *uSize)
{
	Vertex** Ret = NULL;
	unsigned int nCount = 0;
	for (unsigned int i = 0;i < pModel->uCountEdges;++i){
		if (pModel->pEdges[i].pFrom == pVertex){ // Hrana na níž jsme zdrojovým vrcholem
		 Ret = (Vertex**)realloc((void*)Ret,(nCount+1)*sizeof(void*));
		 *(Ret+nCount) = pModel->pEdges[i].pTo;
		 ++nCount;
		}
		else if (pModel->pEdges[i].pTo == pVertex){ // Hrana na níž jsme cílovým vrcholem
		 Ret = (Vertex**)realloc((void*)Ret,(nCount+1)*sizeof(void*));
		 *(Ret+nCount) = pModel->pEdges[i].pFrom;
		 ++nCount;
		}
	}
	*uSize = nCount;
	return Ret;
}

// Spoète vzdálenost dvou vrcholù (tj. délku hrany)
float GetVertexDistance(Vertex* A,Vertex* B)
{
	return sqrt(pow(A->position.x-B->position.x,2)+pow(A->position.y-B->position.y,2));
}

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
	
	if (stat->err.err_num != GML_OK){
		fprintf(stderr,"Error reading GML file on %d : %d\n", stat->err.line, stat->err.column);
		fclose(fp);
		free(stat);
		return 0;
	}
	
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
		  NewV->mass = 5; // TODO: Zatím nastavena konstantní váha vrcholu!
		  ++nVertices;
		  
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
	
	// ... zapiš velikosti polí
	pModel->uCountVertices = (unsigned int)nVertices;
	pModel->uCountEdges = (unsigned int)nEdges;
	
	// Teï už máme všechny vrcholy, proveï napojení hran na vrcholy
	for (int i = 0;i < nEdges;++i){
	  Edge* c = (pModel->pEdges+i);
	  if (c->idTo == c->idFrom){ // Nedovol cyklické hrany!
		fprintf(stderr,"Invalid GML file - cyclic edges are not supported!");
		FreeModel(pModel);
		return 0;
	  }
	  // Najdi adresy vrcholù podle jejich ID
	  c->pTo = GetVertexAddressById(pModel->pVertices,nVertices,c->idTo);
	  c->pFrom = GetVertexAddressById(pModel->pVertices,nVertices,c->idFrom);
	}
	
	return 1;
}

void FreeModel(Model* pModel)
{
	// Uvolni všechny vrcholy a hrany
	if (pModel->pVertices)
	  free(pModel->pVertices);
	if (pModel->pEdges)
	  free(pModel->pEdges);
	
	// Když nejsou žádné povrchy, jsme hotovi
	if (!pModel->VertexSurfaces) return;
	
	// Jinak je tøeba uvolni všechny povrchy
	for (unsigned int i = 0;i < pModel->uCountVertices;++i)
	   SDL_FreeSurface(pModel->VertexSurfaces[i]);
	free(pModel->VertexSurfaces);
}

int CreateModelSurfaces(Model* pModel,GraphicCfg *Config)
{
	// Zkontroluj zda se nejedná o prázdný model
	if (pModel->uCountVertices == 0 || !pModel->pVertices)
	  return 0;
	
	pModel->VertexSurfaces = NULL;	
	const int Gap = 2; // Mezera mezi koleèkem a textem
	
	// Zkus otevøít daný font
	TTF_Font* LabelFont = TTF_OpenFont((char*)Config->szFontFile, Config->uFontSize);
	if (!LabelFont)
	  return 0;
	
	// Naalokuj potøebný poèet povrchù pro vrcholy
	pModel->VertexSurfaces = (SDL_Surface**)malloc(pModel->uCountVertices*sizeof(void*));
	memset(pModel->VertexSurfaces,0,pModel->uCountVertices*sizeof(void*));
	
	int hLabel = 0,wLabel = 0,lineSkip = 0,realWidth = 0,realHeight = 0,maxW = 0,maxH = 0;
	for (unsigned int i = 0;i < pModel->uCountVertices;++i){
		// Zjisti reálnou velikost fontu
		TTF_SizeText(LabelFont,(char*)pModel->pVertices[i].szVertexName,&wLabel,&hLabel);
		lineSkip = TTF_FontLineSkip(LabelFont);
		
		realWidth = ((Config->uNodeRadius*2+3) > wLabel ? (Config->uNodeRadius*2+3) : wLabel+3);
		realHeight = Config->uNodeRadius*2+2+hLabel+lineSkip+Gap;
		if (realWidth > maxW) maxW = realWidth;
		if (realHeight > maxH) maxH = realHeight;
		
		// Zkus vytvoøit nový povrch
		pModel->VertexSurfaces[i] = SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA,realWidth,realHeight,
														 Config->uBPP,0x000000FF,0x0000FF00,0x00FF0000,0xFF000000);
		SDL_Surface *c = pModel->VertexSurfaces[i];
		
		if (!c){ // Nepodaøilo se vytvoøit povrch, vše uvolni a skonèi
		  TTF_CloseFont(LabelFont);
		  free((void*)pModel->VertexSurfaces);
		  pModel->VertexSurfaces = NULL;
		  return 0;
		}
		
		Uint32 inner = SDL_MapRGBA(c->format, Config->innerCircle.r,Config->innerCircle.g,Config->innerCircle.b,0xff),
			   outter = SDL_MapRGBA(c->format, Config->outterCircle.r,Config->outterCircle.g,Config->outterCircle.b,0xff);
			   
		SDL_FillRect(c, 0, SDL_MapRGBA(c->format,0,0,0,0));
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

void SetRandomLocations(Model* pModel,const GraphicCfg Config)
{
	// Zkontroluj zda se nejedná o prázdný model
	if (pModel->uCountVertices == 0 || !pModel->pVertices)
	  return;

	// Najdi náhodné rozmístìní v oknì
	for (unsigned int i = 0;i < pModel->uCountVertices;++i){
		(pModel->pVertices+i)->position.x = Config.uNodeRadius+rand()%(Config.uScreenWidth-Config.uMaxSurfaceW);
		(pModel->pVertices+i)->position.y = Config.uNodeRadius+rand()%(Config.uScreenHeight-Config.uMaxSurfaceH);
	}
}

unsigned int SimulationStep(Model* pModel,const SimulationCfg Config)
{
	// Zkontroluj zda se nejedná o prázdný model
	if (pModel->uCountVertices == 0 || !pModel->pVertices)
	  return 0;
	  
	unsigned int uEnergy = 0,uNbrs = 0;
	Vector2 totalForce;
	Vertex *a,*b;
	for (int i = 0;i < pModel->uCountVertices;++i){
	  memset(&totalForce,0,sizeof(Vector2));
	  a = (pModel->pVertices+i);
	  
	  /*for (int j = 0;j < pModel->uCountVertices;++j){
	    
	  }	  
	  Vertex** nbr = GetAllNeighbours(pModel,c,&uNbrs);
	  for (int j = 0;j < uNbrs;++j){
	  
	  }*/
	  
	  a->velocity = multiplyVector(addVectors(a->velocity,multiplyVector(totalForce,Config.fSimStep)),Config.fDamping);
	  a->position = addVectors(a->position,multiplyVector(a->velocity,Config.fSimStep));
	  uEnergy += a->mass*magnitudeSquared(a->velocity);
	}
	return uEnergy;
}

