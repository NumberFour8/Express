#include "model.h"
#include "drawing.h"

Vertex* GetVertexAddressById(Vertex* pVertices,unsigned int Size,unsigned int ID)
{
	if (!pVertices) return NULL;
	
	for (unsigned int i = 0;i < Size;++i){
	   if (pVertices[i].id == ID)
	     return (pVertices+i);
	}
	return NULL;
}

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

int BuildModel(const char* szFile,Model* pModel)
{
	// Zkus otevøít soubor s grafem
	FILE* fp = fopen(szFile,"r");
	if (!fp){
		fprintf(stderr,"Unable to open %s for reading.\n",szFile);
		return 0;
	}
	
	// Naalokuj potøebné struktury a parsuj soubor
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

int CreateModelSurfaces(Model* pModel,const char *szFont,Uint32 InnerColor,Uint32 OutterColor,SDL_Color FontColor)
{
	// Zkontroluj zda se nejedná o prázdný model
	if (pModel->uCountVertices == 0 || !pModel->pVertices)
	  return 0;
	
	const int FontSize = 20,Height = 100,Width = 100,Gap = 5;
	
	// Zkus otevøít daný font
	TTF_Font* LabelFont = TTF_OpenFont(szFont, FontSize);
	if (!LabelFont)
	  return 0;
	
	// Naalokuj potøebný poèet povrchù pro vrcholy
	pModel->VertexSurfaces = (SDL_Surface**)malloc(pModel->uCountVertices*sizeof(void*));
	memset(pModel->VertexSurfaces,0,pModel->uCountVertices*sizeof(void*));
	
	int h = 0,w = 0,lineSkip = 0;
	for (unsigned int i = 0;i < pModel->uCountVertices;++i){
		// Zjisti reálnou velikost fontu
		TTF_SizeText(LabelFont,(char*)pModel->pVertices[i].szVertexName,&w,&h);
		lineSkip = TTF_FontLineSkip(LabelFont);
		
		// Zkus vytvoøit nový povrch
		pModel->VertexSurfaces[i] = SDL_CreateRGBSurface(SDL_HWSURFACE,Height+h+lineSkip+Gap,(Width > w ? Width : w)+1,32,0,0,0,0);
		SDL_Surface *c = pModel->VertexSurfaces[i];
		
		if (!c){ // Nepodaøilo se vytvoøit povrch, vše uvolni a skonèi
		  TTF_CloseFont(LabelFont);
		  free((void*)pModel->VertexSurfaces);
		  return 0;
		}
		
		// Uzamkni povrch a vykresli koleèko
		doLock(c);
		FillCircle(c,Width/2,Height/2,Height/2,InnerColor,OutterColor);
		doUnlock(c);
		
		// ... spoleènì s textem
		SDL_Surface *Text = TTF_RenderText_Blended(LabelFont,(char*)pModel->pVertices[i].szVertexName,FontColor);
		DrawSurface(Text,c,1,Height+Gap);
		SDL_Flip(c);
		
		SDL_FreeSurface(Text);
	}
	
	// Zavøi font a skonèi
	TTF_CloseFont(LabelFont);
	
	return 1;
}

void SetRandomLocations(Model* pModel,unsigned int uXmax,unsigned int uYmax)
{
	// Zkontroluj zda se nejedná o prázdný model
	if (pModel->uCountVertices == 0 || !pModel->pVertices)
	  return;
	  
	srand(time(0));
	for (unsigned int i = 0;i < pModel->uCountVertices;++i){
		(pModel->pVertices+i)->position.x = rand()%uXmax;
		(pModel->pVertices+i)->position.y = rand()%uYmax;
	}
}
