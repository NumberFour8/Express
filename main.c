#include <stdio.h>

#include "drawing.h"
#include "model.h"

void RenderScene(SDL_Surface* Screen,Model* pModel,const GraphicCfg Config)
{	
	Uint32 line = SDL_MapRGB(Screen->format,Config.lineColor.r,Config.lineColor.g,Config.lineColor.b);
	
	doLock(Screen);
	for (unsigned int i = 0;i < pModel->uCountEdges;++i){
	   DrawLine(Screen,pModel->pEdges[i].pFrom->position.x+Config.uNodeRadius+5,pModel->pEdges[i].pFrom->position.y+Config.uNodeRadius+5,
					   pModel->pEdges[i].pTo->position.x+Config.uNodeRadius+5,pModel->pEdges[i].pTo->position.y+Config.uNodeRadius+5,line);
	}
	doUnlock(Screen);
	
	for (unsigned int i = 0;i < pModel->uCountVertices;++i){
	  DrawSurface(pModel->VertexSurfaces[i],Screen,pModel->pVertices[i].position.x,pModel->pVertices[i].position.y);
	}
	
}

int WinMain(int argc,char* argv[])
{
	// Chybí-li cesta ke GML souboru, vypiš pouze zprávu o použití
	/*if (argc < 2){
	  fprintf(stderr,"Usage: express.exe [path-to-gml-file]");
	  exit(0);
	}*/

	SDL_Surface *screen;
	SDL_Color blue = {0xc8,0xd8,0xff},black = {0,0,0},light_blue = {0xc8,0xd5,0xff};
	
	// Konfigurace zobrazení
	GraphicCfg Conf;
	memset(&Conf,0,sizeof(GraphicCfg));
	strcpy(Conf.szFontFile,"calibri.ttf");
	Conf.innerCircle = blue;
	Conf.outterCircle = Conf.fontColor = black;
	Conf.lineColor = light_blue;
	Conf.uFontSize = 12; Conf.uNodeRadius = 12;
	Conf.uScreenWidth = 800; Conf.uScreenHeight = 600; Conf.uBPP = 32;

    // Inicializace knihoven
    if(SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        fprintf(stderr,"Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);

	if (TTF_Init() == -1){
		fprintf(stderr,"Couldn't initialize SDL_ttf: %s\n", TTF_GetError());
		exit(1);
	}
	atexit(TTF_Quit);
	
	GML_init();
	
	// Inicializace grafiky
    screen = SDL_SetVideoMode(Conf.uScreenWidth, Conf.uScreenHeight, Conf.uBPP, SDL_HWSURFACE|SDL_DOUBLEBUF);
	SDL_SetAlpha(screen,SDL_SRCALPHA,SDL_ALPHA_OPAQUE);
    if (screen == NULL) {
        fprintf(stderr, "Couldn't set %d x %d x %d video mode: %s\n",Conf.uScreenWidth,Conf.uScreenHeight,Conf.uBPP,SDL_GetError());
        exit(1);
    }
	
	// Naèti model z parametru programu
	Model MyModel;
	memset(&MyModel,0,sizeof(Model));
	
	if (!BuildModel("relations.gml",&MyModel)){
	   fprintf(stderr, "Couldn't read GML file: %s\n",argv[1]);
	   exit(1);
	}
	
	// Získej potøebné barvy
	
	// Zkus vytvoøit povrchy pro vrcholy a nastav jim náhodné polohy
	if (!CreateModelSurfaces(&MyModel,Conf)){
	  fprintf(stderr, "Error rendering graph surfaces.\n");
	  FreeModel(&MyModel);
	  exit(1);
	}
	SetRandomLocations(&MyModel,Conf);
	
	// Vycisti scenu bilou barvou
	SDL_FillRect(screen, 0, SDL_MapRGBA(screen->format,0xff,0xff,0xff,0));
	
	SDL_Event event; 
	int run = 1;
	while (run){
        // Osetri udalosti
		while (SDL_WaitEvent(&event)){
            if (event.type == SDL_QUIT){
			  run = 0;
			  break;
			}
			
		    if (event.type == SDL_VIDEOEXPOSE){
		     RenderScene(screen,&MyModel,Conf);
			 SDL_Flip(screen);
			}
        }
		
		SDL_Delay(10); // Šetøíme CPU
	}
	
	FreeModel(&MyModel);
	exit(0);
}