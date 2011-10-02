#include <stdio.h>

#include "drawing.h"

TTF_Font* MyFont = NULL;

void RenderScene(SDL_Surface* Screen)
{
	Uint32 yellow = SDL_MapRGB(Screen->format,0xff,0xff,0);
	
	doLock(Screen);
	
	FillCircle(Screen,Screen->w/2,Screen->h/2,100,yellow);
	DrawLine(Screen,Screen->w/2,Screen->h/2,Screen->w-10,Screen->h-10,yellow);
	
	doUnlock(Screen);
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
	
	// Inicializace grafiky
    screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    if (screen == NULL) {
        fprintf(stderr, "Couldn't set 640x480x32 video mode: %s\n", SDL_GetError());
        exit(1);
    }
	
	// Nacteni fontu
	MyFont = TTF_OpenFont("test_font.ttf", 60);
	if (!MyFont){
		fprintf(stderr,"Unable to open font");
		exit(1);
	}
	
	// Vytvor text
	SDL_Color black = {0, 0, 0};
	SDL_Surface *Text = TTF_RenderText_Blended(MyFont,"Awesome hero !",black);
	
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
			 DrawImage(Text,screen,1,1);
			 SDL_Flip(screen);
			}
        }
		
		SDL_Delay(10);
	}
	
	// Uvolni font a s nim asociovany povrch
	TTF_CloseFont(MyFont);
	SDL_FreeSurface(Text);
	
	exit(0);
}