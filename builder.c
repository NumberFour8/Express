#include "builder.h"

int BuildLinkedVertices(const char* szFile,Vertex** lpVertices,Edge** lpEdges,unsigned int *uSzV,unsigned int *uSzE)
{
	FILE* fp = fopen(szFile,"r");
	if (!fp){
		fprintf(stderr,"Unable to open %s for reading.\n",szFile);
		return 0;
	}
	
	struct GML_stat* stat=(struct GML_stat*)malloc(sizeof(struct GML_stat));
	struct GML_pair* list = GML_parser (file, stat, 0);
	
	if (stat->err.err_num != GML_OK){
		fprintf(stderr,"Error reading GML file on %d : %d", stat->err.line, stat->err.column);
		fclose(fp);
		free(stat);
		return 0;
	}
	
	
	
	GML_free_list (list, stat->key_list);
	free(stat);
	fclose(fp);
	return 1;
}