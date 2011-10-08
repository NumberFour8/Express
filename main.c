#include <stdio.h>

#include "drawing.h"
#include "model.h"

void RenderScene(SDL_Surface* Screen,Model* pModel,const GraphicCfg Config)
{	
	// Vyma� sc�nu
	SDL_FillRect(Screen, 0, SDL_MapRGBA(Screen->format,0xff,0xff,0xff,0));
	
	unsigned int x0,y0,x1,y1;
	Vertex *a,*b;

	// Vykresli v�echny hrany
	doLock(Screen);
	for (unsigned int i = 0;i < pModel->uCountEdges;++i){
	   a = pModel->pEdges[i].pFrom; b = pModel->pEdges[i].pTo;

	   x0 = (int)a->position.x;  y0 = (int)a->position.y;
	   x1 = (int)b->position.x;  y1 = (int)b->position.y;

	   if (x0 < 0) x0 = 0; if (y0 < 0) y0 = 0;
	   if (x1 < 0) x1 = 0; if (y1 < 0) y1 = 0;

	   if (x0 >= Config.uScreenWidth) x0 = Config.uScreenWidth-1; if (y0 >= Config.uScreenHeight) y0 = Config.uScreenHeight-1;
	   if (x1 >= Config.uScreenWidth) x1 = Config.uScreenWidth-1; if (y1 >= Config.uScreenHeight) y1 = Config.uScreenHeight-1;

	   DrawLine(Screen,x0,y0,x1,y1,SDL_MapRGB(Screen->format,Config.lineColor.r,Config.lineColor.g,Config.lineColor.b));
	}
	doUnlock(Screen);
	
	// Vykresli v�echny vrcholy
	int xCorr = 0,yCorr = 0;
	for (unsigned int i = 0;i < pModel->uCountVertices;++i){
	  a = pModel->pVertices+i;
   	  
	  xCorr = (int)a->position.x-a->VertexSurface->w/2;
	  yCorr = (int)a->position.y-Config.uNodeRadius;

	  DrawSurface(pModel->pVertices[i].VertexSurface,Screen,xCorr,yCorr);
	}
	
	// P�epni hlavn� povrch
	SDL_Flip(Screen);
}

int main(int argc,char* argv[])
{

	// Zkontroluj argumenty programu
	if (argc < 2){
	  fprintf(stderr,"Usage: express [path-to-gml-file]\n");
	  exit(0);
	}
	
	// Konfigurace zobrazen�, barev a velikost�
	GraphicCfg Conf;
	memset(&Conf,0,sizeof(GraphicCfg));
	SDL_Color blue = {0xc8,0xd8,0xff},black = {0,0,0},light_blue = {0xc8,0xd5,0xff};
	strcpy(Conf.szFontFile,"calibri.ttf");
	Conf.innerCircle = blue;
	Conf.outterCircle = Conf.fontColor = black;
	Conf.lineColor = light_blue;
	Conf.uFontSize = 12; Conf.uNodeRadius = 12;
	Conf.uScreenWidth = 800; Conf.uScreenHeight = 600; Conf.uBPP = 32;
	///////////////////////////////////////////////////////////////////////////////////////

	// Nastaven� fyzik�ln�ch konstant simulace
	SimulationCfg SimConf;
	SimConf.fDamping = 0.5;
	SimConf.fSpringConstant = 50;
	SimConf.fSimStep = 0.02;
	SimConf.fCoulombConstant = 8.70E+6;
	SimConf.uMinimumKineticEnergy = 100;
	///////////////////////////////////////////////////////////////////////////////////////

	// Inicializace SDL
	SDL_Surface *screen;
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
	
	// Inicializace GML parseru a gener�toru pseudon�hodn�ch ��sel
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
	
	// Na�ti model z parametru programu
	Model MyModel;
	memset(&MyModel,0,sizeof(Model));
	
	if (!BuildModel(argv[1],&MyModel)){
	   fprintf(stderr, "Couldn't read GML file: %s\n",argv[1]);
	   exit(1);
	}	
	
	// Zkus vytvo�it povrchy pro vrcholy a nastav jim n�hodn� polohy
	if (!CreateModelSurfaces(&MyModel,&Conf)){
	  fprintf(stderr, "Error rendering graph surfaces.\n");
	  FreeModel(&MyModel);
	  exit(1);
	}
	SetRandomLocations(&MyModel,Conf);

	// Vy�isti sc�nu b�lou barvou
	SDL_FillRect(screen, 0, SDL_MapRGBA(screen->format,0xff,0xff,0xff,0));

	SDL_Event event; 
	int run = 1,framesCounter = 0,motionAllowed = 1;
	float waitTime = 0.01;
	unsigned int lastTick = SDL_GetTicks();
	
	// Hlavn� smy�ka
	while (run){
	    // O�et�i ud�losti
	    while (SDL_PollEvent(&event)){
            	// Zajisti vypnut� aplikace
		if (event.type == SDL_QUIT){
		  run = 0;
		  break;
		}		
		// P�i p�ekreslen� okna obnovuj sc�nu
		if (event.type == SDL_VIDEOEXPOSE)
		  RenderScene(screen,&MyModel,Conf);
	    }

	    // Pokud energie nespadla pod povolen� minimum, prov�d�j simulaci
	    if (motionAllowed){
	      // Posu� simulaci
	      if (SDL_GetTicks()-lastTick >= SimConf.fSimStep*1000){
		motionAllowed = SimulationStep(&MyModel,SimConf) > SimConf.uMinimumKineticEnergy; 
	        lastTick = SDL_GetTicks();		
	      }
	    
	      // P�ekresli sc�nu (5 ~ 20 FPS)
	      if (framesCounter == 3){
	        RenderScene(screen,&MyModel,Conf);
	        framesCounter = 0;
	      }
	    }

	    SDL_Delay(1000*waitTime);
	    ++framesCounter;
	}
	
	// Uvolni model a skon�i
	FreeModel(&MyModel);
	exit(0);
}
