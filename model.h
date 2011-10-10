// Definuje funkce pro práci s modelem a simulací
//====================================================

#ifndef _MODEL
#define _MODEL

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
	float x,y;
}Vector2;

typedef struct {
	unsigned int id;
	float charge;
	Vector2 position,velocity,force;
	char szVertexName[MAX_LENGTH_LABEL];
	SDL_Surface *VertexSurface;
}Vertex;

typedef struct {
	Vertex* pFrom;
	Vertex* pTo;
	float value;
	unsigned int idFrom,idTo;
}Edge;

typedef struct {
	Vertex* pVertices;
	Edge* pEdges;
	unsigned int uCountEdges,uCountVertices;
}Model;

typedef struct {
	char szFontFile[255];
	SDL_Color innerCircle,outterCircle,lineColor,fontColor;
	unsigned int uFontSize,uNodeRadius,uScreenWidth,uScreenHeight,uBPP,
				 uMaxSurfaceW,uMaxSurfaceH;
}GraphicCfg;

typedef struct {
	float fDamping,fSpringConstant,fSimStep,fCoulombConstant;
	unsigned int uMinimumKineticEnergy,uMinimumSpringLength;
}SimulationCfg;

int BuildModel(const char* szFile,Model* pModel);
void FreeModel(Model* pModel);

int CreateModelSurfaces(Model* pModel,GraphicCfg *Config);
void SetRandomLocations(Model* pModel,const GraphicCfg Config);

unsigned int SimulationStep(Model* pModel,const SimulationCfg Config);

#endif
