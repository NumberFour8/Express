#include <stdio.h>

#include "drawing.h"
#include "model.h"

void RenderScene(SDL_Surface* Screen,Model* pModel)
{
	
	/*doLock(Screen);
	
	FillCircle(Screen,Screen->w/2,Screen->h/2,100,blue,black);	
	DrawLine(Screen,Screen->w/2,Screen->h/2,Screen->w-10,Screen->h-10,black);
	
	doUnlock(Screen);*/
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
	SDL_Color blue = {0xc8,0xd8,0xff},black = {0,0,0};
	
	// Konfigurace zobrazení
	GraphicCfg Conf;
	memset(&Conf,0,sizeof(GraphicCfg));
	strcpy(Conf.szFontFile,"calibri.ttf");
	Conf.innerCircle = blue;
	Conf.outterCircle = Conf.lineColor = Conf.fontColor = black;
	Conf.uFontSize = 12; Conf.uNodeRadius = 12;
	Conf.uScreenWidth = 640; Conf.uScreenHeight = 480; Conf.uBPP = 32;

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
    if (screen == NULL) {
        fprintf(stderr, "Couldn't set %d x %d x %d video mode: %s\n",Conf.uScreenWidth,Conf.uScreenHeight,Conf.uBPP,SDL_GetError());
        exit(1);
    }
	
	// Naèti model z parametru programu
	Model MyModel;
	memset(&MyModel,0,sizeof(Model));
	
	if (!BuildModel("circle.gml",&MyModel)){
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
	SDL_FillRect(screen, 0, SDL_MapRGB(screen->format,0xff,0xff,0xff));
	
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
		     RenderScene(screen,&MyModel);
			 SDL_Flip(screen);
			}
        }
		
		SDL_Delay(10); // Šetøíme CPU
	}
	
	FreeModel(&MyModel);
	exit(0);
}