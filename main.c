#include <stdio.h>

#include "drawing.h"

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
    if ( screen == NULL ) {
        fprintf(stderr, "Couldn't set 640x480x8 video mode: %s\n", SDL_GetError());
        exit(1);
    }
	
	DrawCircle(screen,screen->w/2,screen->h/2,100);
	DrawLine(screen,screen->w/2,screen->h/2,screen->w-10,screen->h-10);
	
	while(1);
	exit(0);
}