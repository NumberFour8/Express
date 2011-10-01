#ifndef _DRAWING
#define _DRAWING

#include <SDL/SDL.h>

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

void DrawCircle(SDL_Surface *where,int x,int y,int r);
void DrawLine(SDL_Surface *where,int x0,int y0,int x1,int y1);

void FillCircle(SDL_Surface *where,int x0,int y0,int r);
#endif