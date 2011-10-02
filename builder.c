#include "builder.h"

Vertex* GetVertexAddressById(Vertex* pVertices,unsigned int Size,unsigned int ID)
{
	if (!pVertices) return NULL;
	
	for (unsigned int i = 0;i < Size;++i){
	   if (pVertices[i].id == ID)
	     return (pVertices+i);
	}
	return NULL;
}

int BuildLinkedVertices(const char* szFile,Vertex** pVertices,Edge** pEdges,unsigned int *uSzV,unsigned int *uSzE)
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
	
	*pVertices = NULL;
	*pEdges = NULL;
	
	struct GML_pair* ptr = list,*inner = NULL;
	
	// Vybírej vrcholy
	for (;ptr != NULL;++ptr){
		if (strcmp(ptr->key,"node") == 0 && ptr->kind == GML_LIST){
		  // Realokuj pole vrcholù na novou velikost
		  *pVertices = (Vertex*)realloc(pVertices,(nVertices+1)*sizeof(Vertex));
		  Vertex* NewV = (*pVertices)+nVertices;
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
		  *pEdges = (Edge*)realloc(pEdges,(nEdges+1)*sizeof(Edge));
		  Edge* NewE = (*pEdges)+nEdges;
		  memset((void*)NewE,0,sizeof(Edge));
		  ++nEdges;
		  
		  inner = ptr->value.list;
		  for (;inner != NULL;++inner){
			if (strcmp(inner->key,"source") == 0 && inner->kind == GML_INT)
			   NewE->pFrom = GetVertexAddressById(*pVertices,nVertices,(unsigned int)inner->value.integer);
			if (strcmp(inner->key,"target") == 0 && inner->kind == GML_INT)
			   NewE->pTo = GetVertexAddressById(*pVertices,nVertices,(unsigned int)inner->value.integer);
		  }
		}
		else continue;
	}
	
	// ... hotovo
	*uSzV = (unsigned int)nVertices;
	*uSzE = (unsigned int)nEdges;
	
	GML_free_list (list, stat->key_list);
	free(stat);
	fclose(fp);
	
	return 1;
}