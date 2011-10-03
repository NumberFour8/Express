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

int BuildModel(const char* szFile,Model* pModel)
{
	FILE* fp = fopen(szFile,"r");
	if (!fp){
		fprintf(stderr,"Unable to open %s for reading.\n",szFile);
		return 0;
	}
	
	struct GML_stat* stat=(struct GML_stat*)malloc(sizeof(struct GML_stat));
	struct GML_pair* list = GML_parser (fp, stat, 0);
	
	if (stat->err.err_num != GML_OK){
		fprintf(stderr,"Error reading GML file on %d : %d", stat->err.line, stat->err.column);
		fclose(fp);
		free(stat);
		return 0;
	}
	
	int nEdges = 0,nVertices = 0;
	
	memset((void*)pModel,0,sizeof(Model));
	
	struct GML_pair* ptr = list,*inner = NULL;
	
	// Vybírej vrcholy
	for (;ptr != NULL;++ptr){
		if (strcmp(ptr->key,"node") == 0 && ptr->kind == GML_LIST){
		  // Realokuj pole vrcholù na novou velikost
		  pModel->pVertices = (Vertex*)realloc(pModel->pVertices,(nVertices+1)*sizeof(Vertex));
		  Vertex* NewV = (pModel->pVertices)+nVertices;
		  memset((void*)NewV,0,sizeof(Vertex));
		  ++nVertices;
		  
		  inner = ptr->value.list;
		  for (;inner != NULL;++inner){
			if (strcmp(inner->key,"id") == 0 && inner->kind == GML_INT)
				NewV->id = (unsigned int)inner->value.integer;
			if (strcmp(inner->key,"label") == 0 && inner->kind == GML_STRING)
			  memcpy((void*)NewV->szVertexName,inner->value.string,strlen(inner->value.string) > MAX_LENGTH_LABEL ? MAX_LENGTH_LABEL : strlen(inner->value.string));
		  }
		}
		else continue;
	}
	
	ptr = list; inner = NULL;
	
	// Všechny vrcholy pøeèteny, teï vybírej hrany a spojuj vrcholy
	for (;ptr != NULL;++ptr){
		if (strcmp(ptr->key,"edge") == 0 && ptr->kind == GML_LIST){
		  // Realokuj pole hran na novou velikost
		  pModel->pEdges = (Edge*)realloc(pModel->pEdges,(nEdges+1)*sizeof(Edge));
		  Edge* NewE = (pModel->pEdges)+nEdges;
		  memset((void*)NewE,0,sizeof(Edge));
		  ++nEdges;
		  
		  inner = ptr->value.list;
		  for (;inner != NULL;++inner){
			if (strcmp(inner->key,"source") == 0 && inner->kind == GML_INT)
			   NewE->pFrom = GetVertexAddressById(pModel->pVertices,nVertices,(unsigned int)inner->value.integer);
			if (strcmp(inner->key,"target") == 0 && inner->kind == GML_INT)
			   NewE->pTo = GetVertexAddressById(pModel->pVertices,nVertices,(unsigned int)inner->value.integer);
			if (strcmp(inner->key,"value") == 0 && inner->kind == GML_INT)
			   NewE->value = (unsigned int)inner->value.integer;
		  }
		}
		else continue;
	}
	
	// ... hotovo
	pModel->uCountVertices = (unsigned int)nVertices;
	pModel->uCountEdges = (unsigned int)nEdges;
	
	GML_free_list(list, stat->key_list);
	free(stat);
	fclose(fp);
	
	return 1;
}

void FreeModel(Model* pModel)
{
	if (pModel->pVertices)
	  free(pModel->pVertices);
	if (pModel->pEdges)
	  free(pModel->pEdges);
	
	if (!pModel->VertexSurfaces) return;
	
	for (unsigned int i = 0;i < pModel->uCountVertices;++i)
	   SDL_FreeSurface(pModel->VertexSurfaces[i]);
	free(pModel->VertexSurfaces);
}

int CreateModelSurfaces(Model* pModel,const char *szFont,Uint32 InnerColor,Uint32 OutterColor,SDL_Color FontColor)
{
	if (pModel->uCountVertices == 0 || !pModel->pVertices)
	  return 0;
	
	const int FontSize = 20,Height = 100,Width = 100,Gap = 5;
	
	pModel->VertexSurfaces = (SDL_Surface**)malloc(pModel->uCountVertices*sizeof(void*));
	memset(pModel->VertexSurfaces,0,pModel->uCountVertices*sizeof(void*));
	
	TTF_Font* LabelFont = TTF_OpenFont(szFont, FontSize);
	
	int h = 0,w = 0,lineSkip = 0;
	for (unsigned int i = 0;i < pModel->uCountVertices;++i){
		TTF_SizeText(LabelFont,(char*)pModel->pVertices[i].szVertexName,&w,&h);
		lineSkip = TTF_FontLineSkip(LabelFont);
		
		pModel->VertexSurfaces[i] = SDL_CreateRGBSurface(SDL_HWSURFACE,Height+h+lineSkip+Gap,(Width > w ? Width : w)+1,32,0,0,0,0);
		SDL_Surface *c = pModel->VertexSurfaces[i];
		
		doLock(c);
		FillCircle(c,Width/2,Height/2,Height/2,InnerColor,OutterColor);
		doUnlock(c);
		
		SDL_Surface *Text = TTF_RenderText_Blended(LabelFont,(char*)pModel->pVertices[i].szVertexName,FontColor);
		DrawImage(Text,c,1,Height+Gap);
		SDL_FreeSurface(Text);
	}
	
	TTF_CloseFont(LabelFont);
}
