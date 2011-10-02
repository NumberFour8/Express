#ifndef _PARSE
#define _PARSE

#include <stdio.h>
#include <stdlib.h>

#include "gml/gml_parser.h"

struct Vertex {
	unsigned int X,Y,id;
	int dX,dY;
	char szVertexName[256];
};

struct Edge {
	Vertex* pFrom;
	Vertex* pTo;
};

int BuildLinkedVertices(const char* szFile,Vertex** lpVertices,Edge** lpEdges,unsigned int *uSzV,unsigned int *uSzE);

#endif