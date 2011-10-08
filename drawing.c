#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <SDL/SDL.h>

// Klasická funkce na vykreslení pixelů
void putpixel(SDL_Surface *surface, int x, int y, unsigned int pixel)
{
    const int bpp = surface->format->BytesPerPixel;
    unsigned char *p = (unsigned char*)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp){
		case 1:
			*p = pixel;
			break;

		case 2:
			*(unsigned short*)p = pixel;
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
			*(unsigned int*)p = pixel;
			break;
    };
}

// Zkusí uzamknout povrch pro přímý přístup
void doLock(SDL_Surface* what)
{
	if (SDL_MUSTLOCK(what)){
        if (SDL_LockSurface(what) < 0)
            fprintf(stderr, "Can't lock surface: %s\n", SDL_GetError());
    }
}

// Odemkne povrch pro přímý přístup
void doUnlock(SDL_Surface* what)
{
	if (SDL_MUSTLOCK(what)) SDL_UnlockSurface(what);
}

// Vykreslí čáru z bodu do bodu pomocí Bressenhamova algoritmu
void DrawLine(SDL_Surface *where,int x0,int y0,int x1,int y1,unsigned int pixel)
{
   int dx = abs(x1-x0),dy = abs(y1-y0);
   int sx,sy,err = dx-dy,e2 = 0;
   
   if (x0 < x1) sx = 1; else sx = -1;
   if (y0 < y1) sy = 1; else sy = -1;
 
   
   while (1) {
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
}

// Vykreslí kružnici pomocí algoritmu mid-point 
void DrawCircle(SDL_Surface *where,int x0, int y0, int radius, unsigned int pixel)
{
   int error = -radius;
   int x = radius;
   int y = 0;

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
}

// Vykreslí kruh aproximací
void FillCircle(SDL_Surface *where, int x0, int y0, int radius, unsigned int inner,unsigned int outter) 
{
   const int BPP = where->format->BytesPerPixel;
	
   double r = (double)radius;
   
   for (double dy = 1;dy <= r;dy += 1.0){
       double dx = floor(sqrt((2.0 * r * dy) - (dy * dy)));
       int x = x0 - dx;

       unsigned char *target_pixel_a = (unsigned char*)where->pixels + ((int)(y0 + r - dy)) * where->pitch + x * BPP;
       unsigned char *target_pixel_b = (unsigned char*)where->pixels + ((int)(y0 - r + dy)) * where->pitch + x * BPP;
               
       for (;x <= x0 + dx;++x){
           *(unsigned int*)target_pixel_a = inner;
           *(unsigned int*)target_pixel_b = inner;
           target_pixel_a += BPP;
           target_pixel_b += BPP;
       }
   }
   DrawCircle(where,x0,y0,radius,outter);
} 

// Provede blit jednoho porvrchu na druhý
void DrawSurface(SDL_Surface* source, SDL_Surface* destination, int x, int y) 
{
    if (!source || !destination) return;

    SDL_Rect destRect;
    destRect.x = x;
    destRect.y = y;
    SDL_BlitSurface(source,0,destination,&destRect);
}
