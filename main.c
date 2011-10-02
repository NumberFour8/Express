#include <stdio.h>

#include "drawing.h"

void RenderScene(SDL_Surface* Screen)
{
	Uint32 yellow = SDL_MapRGB(Screen->format,0xff,0xff,0);
	
	FillCircle(Screen,Screen->w/2-50,Screen->h/2-50,100,yellow);
	DrawLine(Screen,Screen->w/2,Screen->h/2,Screen->w-10,Screen->h-10,yellow);
}

int WinMain(int argc,char** argv)
{
	SDL_Surface *screen;

    /* Initialize the SDL library */
    if( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        fprintf(stderr,"Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    /* Clean up on exit */
    atexit(SDL_Quit);
    
    /*
     * Initialize the display in a 640x480 8-bit palettized mode,
     * requesting a software surface
     */
    screen = SDL_SetVideoMode(640, 480, 8, SDL_SWSURFACE);
    if (screen == NULL) {
        fprintf(stderr, "Couldn't set 640x480x8 video mode: %s\n", SDL_GetError());
        exit(1);
    }
	
	RenderScene(screen);
	
	getc(stdin);
	exit(0);
}