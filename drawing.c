#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <SDL/SDL.h>

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    const int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp){
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
    };
}

void doLock(SDL_Surface* what)
{
	if (SDL_MUSTLOCK(what)){
        if (SDL_LockSurface(what) < 0)
            fprintf(stderr, "Can't lock surface: %s\n", SDL_GetError());
    }
}

void doUnlock(SDL_Surface* what)
{
	if (SDL_MUSTLOCK(what)) SDL_UnlockSurface(what);
}

void DrawLine(SDL_Surface *where,int x0,int y0,int x1,int y1,Uint32 pixel)
{
   int dx = abs(x1-x0),dy = abs(y1-y0);
   int sx,sy,err = dx-dy,e2 = 0;
   
   if (x0 < x1) sx = 1; else sx = -1;
   if (y0 < y1) sy = 1; else sy = -1;
 
   
   doLock(where);
   while(1) {
     putpixel(where,x0,y0,pixel);
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

void DrawCircle(SDL_Surface *where,int x0, int y0, int radius, Uint32 pixel)
{
   int error = -radius;
   int x = radius;
   int y = 0;

   doLock(where);
   while (x >= y)
   {
       putpixel(where, x0 + x, y0 + y, pixel);
       putpixel(where, x0 + y, y0 + x, pixel);
           
       if (x != 0){
         putpixel(where, x0 - x, y0 + y, pixel);
         putpixel(where, x0 + y, y0 - x, pixel);
       }
       
       if (y != 0){
         putpixel(where, x0 + x, y0 - y, pixel);
         putpixel(where, x0 - y, y0 + x, pixel);
       }
       
       if (x != 0 && y != 0){
         putpixel(where, x0 - x, y0 - y, pixel);
         putpixel(where, x0 - y, y0 - x, pixel);
       }
           
       error += y;
       ++y;
       error += y;

       if (error >= 0){
         --x;
         error -= x;
         error -= x;
       }
   }
   doUnlock(where);
   SDL_UpdateRect(where, x0-radius, y0-radius, 2*radius, 2*radius);
}

void FillCircle(SDL_Surface *where, int x0, int y0, double r, Uint32 pix) 
{
   const int BPP = where->format->BytesPerPixel;
	
   doLock(where);
   for (double dy = 1;dy <= r;dy += 1.0){
       double dx = floor(sqrt((2.0 * r * dy) - (dy * dy)));
       int x = x0 - dx;

       Uint8 *target_pixel_a = (Uint8*)where->pixels + ((int)(y0 + r - dy)) * where->pitch + x * BPP;
       Uint8 *target_pixel_b = (Uint8*)where->pixels + ((int)(y0 - r + dy)) * where->pitch + x * BPP;
               
       for (;x <= x0 + dx;++x){
           *(Uint32*)target_pixel_a = pix;
           *(Uint32*)target_pixel_b = pix;
           target_pixel_a += BPP;
           target_pixel_b += BPP;
       }
   }
   doUnlock(where);
   
   SDL_UpdateRect(where, x0-r, y0-r, 2*r, 2*r);
} 