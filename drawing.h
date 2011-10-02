#ifndef _DRAWING
#define _DRAWING

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
void doLock(SDL_Surface* what);
void doUnlock(SDL_Surface* what);

void DrawCircle(SDL_Surface *where, int x, int y, int radius, Uint32 pix);
void DrawLine(SDL_Surface *where, int x0, int y0, int x1, int y1, Uint32 pix);

void FillCircle(SDL_Surface *where, int x0, int y0, int radius, Uint32 pix);

void DrawImage(SDL_Surface* source, SDL_Surface* destination, int x, int y);
#endif