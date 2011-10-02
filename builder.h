#ifndef _BUILDER
#define _BUILDER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gml/gml_parser.h"

#define MAX_LENGTH_LABEL 256

typedef struct {
	unsigned int X,Y,id;
	int dX,dY;
	char szVertexName[MAX_LENGTH_LABEL];
}Vertex;

typedef struct {
	Vertex* pFrom;
	Vertex* pTo;
}Edge;

int BuildLinkedVertices(const char* szFile,Vertex** pVertices,Edge** pEdges,unsigned int *uSzV,unsigned int *uSzE);

#endif