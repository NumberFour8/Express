#include <stdio.h>

#include "drawing.h"
#include "model.h"

void RenderScene(SDL_Surface* Screen,Model* pModel,const GraphicCfg Config)
{	
	// Vyma¾ scénu
	SDL_FillRect(Screen, 0, SDL_MapRGBA(Screen->format,0xff,0xff,0xff,0));
	
	unsigned int x0,y0,x1,y1;
	// Vykresli v¹echny hrany
	doLock(Screen);
	for (unsigned int i = 0;i < pModel->uCountEdges;++i){
	   x0 = (int)pModel->pEdges[i].pFrom->position.x+Config.uNodeRadius+5;
 	   y0 = (int)pModel->pEdges[i].pFrom->position.y+Config.uNodeRadius+5;
	   x1 = (int)pModel->pEdges[i].pTo->position.x+Config.uNodeRadius+5;
	   y1 = (int)pModel->pEdges[i].pTo->position.y+Config.uNodeRadius+5;
	   if (x0 < 0) x0 = 0; if (y0 < 0) y0 = 0;
	   if (x1 < 0) x1 = 0; if (y1 < 0) y1 = 0;

	   if (x0 >= Config.uScreenWidth) x0 = Config.uScreenWidth-1; if (y0 >= Config.uScreenHeight) y0 = Config.uScreenHeight-1;
	   if (x1 >= Config.uScreenWidth) x1 = Config.uScreenWidth-1; if (y1 >= Config.uScreenHeight) y1 = Config.uScreenHeight-1;

	   DrawLine(Screen,x0,y0,x1,y1,SDL_MapRGB(Screen->format,Config.lineColor.r,Config.lineColor.g,Config.lineColor.b));
	}
	doUnlock(Screen);
	
	// Vykresli v¹echny vrcholy
	int xCorr = 0,yCorr = 0;
	for (unsigned int i = 0;i < pModel->uCountVertices;++i){

	  xCorr = (int)pModel->pVertices[i].position.x;
	  yCorr = (int)pModel->pVertices[i].position.y;

	  DrawSurface(pModel->VertexSurfaces[i],Screen,xCorr,yCorr);
	}
	
	// Pøepni hlavní povrch
	SDL_Flip(Screen);
}

int main(int argc,char* argv[])
{

	// Zkontroluj argumenty programu
	if (argc < 2){
	  fprintf(stderr,"Usage: express [path-to-gml-file]\n");
	  exit(0);
	}

	SDL_Surface *screen;
	
	// Konfigurace zobrazení, barev a velikostí
	GraphicCfg Conf;
	memset(&Conf,0,sizeof(GraphicCfg));
	SDL_Color blue = {0xc8,0xd8,0xff},black = {0,0,0},light_blue = {0xc8,0xd5,0xff};
	strcpy(Conf.szFontFile,"calibri.ttf");
	Conf.innerCircle = blue;
	Conf.outterCircle = Conf.fontColor = black;
	Conf.lineColor = light_blue;
	Conf.uFontSize = 12; Conf.uNodeRadius = 12;
	Conf.uScreenWidth = 800; Conf.uScreenHeight = 600; Conf.uBPP = 32;

	// Inicializace SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0 ) {
	  fprintf(stderr,"Couldn't initialize SDL: %s\n", SDL_GetError());
          exit(1);
	}
	atexit(SDL_Quit);

	// Inicializace SDL_TTF
	if (TTF_Init() == -1){
		fprintf(stderr,"Couldn't initialize SDL_ttf: %s\n", TTF_GetError());
		exit(1);
	}
	atexit(TTF_Quit);
	
	// Inicializace GML parseru a generátoru pseudonáhodných èísel
	GML_init();
	srand(time(0));
	
	// Inicializace grafiky
    	screen = SDL_SetVideoMode(Conf.uScreenWidth, Conf.uScreenHeight, Conf.uBPP, SDL_HWSURFACE|SDL_DOUBLEBUF);
	SDL_SetAlpha(screen,SDL_SRCALPHA,SDL_ALPHA_OPAQUE);

    	if (screen == NULL) {
	  fprintf(stderr, "Couldn't set %d x %d x %d video mode: %s\n",Conf.uScreenWidth,Conf.uScreenHeight,Conf.uBPP,SDL_GetError());
  	  exit(1);
    	}
	SDL_WM_SetCaption("Express",NULL);
	
	// Naèti model z parametru programu
	Model MyModel;
	memset(&MyModel,0,sizeof(Model));
	
	if (!BuildModel(argv[1],&MyModel)){
	   fprintf(stderr, "Couldn't read GML file: %s\n",argv[1]);
	   exit(1);
	}	
	
	// Zkus vytvoøit povrchy pro vrcholy a nastav jim náhodné polohy
	if (!CreateModelSurfaces(&MyModel,&Conf)){
	  fprintf(stderr, "Error rendering graph surfaces.\n");
	  FreeModel(&MyModel);
	  exit(1);
	}
	SetRandomLocations(&MyModel,Conf);
	
	// Nastavení fyzikálních konstant simulace
	SimulationCfg SimConf;
	SimConf.fDamping = 0.5;
	SimConf.fSpringConstant = 45;
	SimConf.fSimStep = 0.02;
	SimConf.fCoulombConstant = 8.70E+6;

	// Vyèisti scénu bílou barvou
	SDL_FillRect(screen, 0, SDL_MapRGBA(screen->format,0xff,0xff,0xff,0));

	SDL_Event event; 
	int run = 1,framesCounter = 0,motionAllowed = 1;
	float waitTime = 0.01;
	unsigned int lastTick = SDL_GetTicks();
	
	// Hlavní smyèka
	while (run){
	    // O¹etøi události
	    while (SDL_PollEvent(&event)){
            	// Zajisti vypnutí aplikace
		if (event.type == SDL_QUIT){
		  run = 0;
		  break;
		}		
		// Pøi pøekreslení okna obnovuj scénu
		if (event.type == SDL_VIDEOEXPOSE)
		  RenderScene(screen,&MyModel,Conf);
	    }

	    // Pokud energie nespadla pod povolené minimum, provádìj simulaci
	    if (motionAllowed){
	      // Posuò simulaci
	      if (SDL_GetTicks()-lastTick >= SimConf.fSimStep*1000){
		SimulationStep(&MyModel,SimConf);
	        lastTick = SDL_GetTicks();		
	      }
	    
	      // Pøekresli scénu (5 ~ 20 FPS)
	      if (framesCounter == 3){
	        RenderScene(screen,&MyModel,Conf);
	        framesCounter = 0;
	      }
	    }

	    SDL_Delay(1000*waitTime);
	    ++framesCounter;
	}
	
	// Uvolni model a skonèi
	FreeModel(&MyModel);
	exit(0);
}
