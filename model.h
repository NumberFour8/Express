#ifndef _BUILDER
#define _BUILDER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "gml/gml_parser.h"

#define MAX_LENGTH_LABEL 256

typedef struct {
	int x,y;
}Vector2;

typedef struct {
	unsigned int id,mass;
	Vector2 position,velocity,force;
	char szVertexName[MAX_LENGTH_LABEL];
}Vertex;

typedef struct {
	Vertex* pFrom;
	Vertex* pTo;
	int value;
	unsigned int idFrom,idTo;
}Edge;

typedef struct {
	Vertex* pVertices;
	Edge* pEdges;
	unsigned int uCountEdges,uCountVertices;
	SDL_Surface** VertexSurfaces;
}Model;

int BuildModel(const char* szFile,Model* pModel);
void FreeModel(Model* pModel);

int CreateModelSurfaces(Model* pModel,const char* szFont,Uint32 InnerCircle,Uint32 OutterCircle,SDL_Color FontColor);
void SetRandomLocations(Model* pModel,unsigned int uXmax,unsigned int uYmax);

#endif