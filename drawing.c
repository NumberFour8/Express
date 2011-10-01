#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <SDL/SDL.h>

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

void doLock(SDL_Surface* what)
{
	if (SDL_MUSTLOCK(what)) {
        if (SDL_LockSurface(what) < 0)
            fprintf(stderr, "Can't lock surface: %s\n", SDL_GetError());
    }
}

void doUnlock(SDL_Surface* what)
{
	if (SDL_MUSTLOCK(what)) SDL_UnlockSurface(what);
}

void DrawCircle(SDL_Surface *where,int x,int y,int r)
{
    /* Map the color yellow to this display (R=0xff, G=0xFF, B=0x00)
       Note:  If the display is palettized, you must set the palette first.
    */
    Uint32 yellow = SDL_MapRGB(where->format, 0xff, 0xff, 0x00);

    /* Lock the screen for direct access to the pixels */

	doLock(where);
	
	for (int i = 0;i < 360;i++)
       putpixel(where, x + r*cos((float)i), y + r*sin((float)i), yellow);
    
	doUnlock(where);
	
    /* Update just the part of the display that we've changed */
    SDL_UpdateRect(where, x-r, y-r, 2*r, 2*r);
}

void DrawLine(SDL_Surface *where,int x0,int y0,int x1,int y1)
{
   int dx = abs(x1-x0),dy = abs(y1-y0);
   int sx,sy,err = dx-dy,e2 = 0;
   
   if (x0 < x1) sx = 1; else sx = -1;
   if (y0 < y1) sy = 1; else sy = -1;
 
   Uint32 yellow = SDL_MapRGB(where->format, 0xff, 0xff, 0x00);
   
   doLock(where);
   
   while(1) {
     putpixel(where,x0,y0,yellow);
     if (x0 == x1 && y0 == y1) break;
     e2 = 2*err;
	 
     if (e2 > -dy){ 
       err = err - dy;
       x0 = x0 + sx;
     }
	 
     if (e2 < dx){ 
       err = err + dx;
       y0 = y0 + sy;
     }
   }
   
   doUnlock(where);
   
   SDL_UpdateRect(where, 0, 0, x1, y1);
}