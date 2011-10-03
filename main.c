#include <stdio.h>

#include "drawing.h"
#include "model.h"

void RenderScene(SDL_Surface* Screen)
{
	Uint32 blue = SDL_MapRGB(Screen->format,0xc8,0xd8,0xff);
	Uint32 black = SDL_MapRGB(Screen->format,0,0,0);
	
	/*doLock(Screen);
	
	FillCircle(Screen,Screen->w/2,Screen->h/2,100,blue,black);	
	DrawLine(Screen,Screen->w/2,Screen->h/2,Screen->w-10,Screen->h-10,black);
	
	doUnlock(Screen);*/
	Model Mod;
	memset(&Mod,0,sizeof(Model));
	
	BuildModel("circle.gml",&Mod);
}

int WinMain(int argc,char* argv[])
{
	SDL_Surface *screen;

    // Inicializace knihoven
    if(SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        fprintf(stderr,"Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);

	if (TTF_Init() == -1){
		fprintf(stderr,"Unable to initialize SDL_ttf: %s\n", TTF_GetError());
		exit(1);
	}
	atexit(TTF_Quit);
	
	GML_init();
	
	// Inicializace grafiky
    screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    if (screen == NULL) {
        fprintf(stderr, "Couldn't set 640x480x32 video mode: %s\n", SDL_GetError());
        exit(1);
    }
	
	// Vycisti scenu bilou barvou
	SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0xff, 0xff, 0xff));
	
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
		     RenderScene(screen);
			 SDL_Flip(screen);
			}
        }
		
		SDL_Delay(10);
	}
	
	exit(0);
}