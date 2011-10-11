/*
	Zápoètový program pro pøedmìt Programování v C++ 1
	Luká¹ Pohanka, 2011
	pohanluk@fjfi.cvut.cz
	Fakulta Jaderná a Fyzikálnì In¾enýrská ÈVUT
*/

#include <stdio.h>

#include "drawing.h"
#include "model.h"

// Pøekresluje scénu
void RenderScene(SDL_Surface* Screen,Model* pModel,const GraphicCfg Config)
{	
	// Vyma¾ scénu
	SDL_FillRect(Screen,0,SDL_MapRGBA(Screen->format,0xff,0xff,0xff,0));
	
	int x0,y0,x1,y1;
	Vertex *a,*b;

	// Vykresli v¹echny hrany
	doLock(Screen);
	for (unsigned int i = 0;i < pModel->uCountEdges;++i){
	   a = pModel->pEdges[i].pFrom; b = pModel->pEdges[i].pTo;

	   // Zaokrouhli vektory na celá èísla
	   x0 = (int)a->position.x;  y0 = (int)a->position.y;
	   x1 = (int)b->position.x;  y1 = (int)b->position.y;

	   // O¹etøi pozice mimo kreslitelnou oblast
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
	  a = pModel->pVertices+i;
   	  
	  xCorr = (int)a->position.x-a->VertexSurface->w/2;
	  yCorr = (int)a->position.y-Config.uNodeRadius;

	  DrawSurface(pModel->pVertices[i].VertexSurface,Screen,xCorr,yCorr);
	}
	
	// Pøepni hlavní povrch
	SDL_Flip(Screen);
}

// Oparsuje konfiguraèní soubor
int ReadConfigurationFile(const char* szFile,GraphicCfg *GConf,SimulationCfg *SConf)
{
	FILE* fp = fopen(szFile,"r");
	if (!fp) return 0;
	
	memset(GConf,0,sizeof(GraphicCfg));
	memset(SConf,0,sizeof(SimulationCfg));
	
	char line[100];
	memset((char*)line,0,100);
	
	while (fgets((char*)line,100,fp)){
		if (strstr((char*)line,"innerColor:"))
		  sscanf((char*)line,"%*s%d,%d,%d",(int*)&GConf->innerCircle.r,(int*)&GConf->innerCircle.g,(int*)&GConf->innerCircle.b);
		else if (strstr((char*)line,"outterColor:"))
		  sscanf((char*)line,"%*s%d,%d,%d",(int*)&GConf->outterCircle.r,(int*)&GConf->outterCircle.g,(int*)&GConf->outterCircle.b);
		else if (strstr((char*)line,"lineColor:"))
		  sscanf((char*)line,"%*s%d,%d,%d",(int*)&GConf->lineColor.r,(int*)&GConf->lineColor.g,(int*)&GConf->lineColor.b);
		else if (strstr((char*)line,"fontColor:"))
		  sscanf((char*)line,"%*s%d,%d,%d",(int*)&GConf->fontColor.r,(int*)&GConf->fontColor.g,(int*)&GConf->fontColor.b);
		else if (strstr((char*)line,"fontName:"))
		  sscanf((char*)line,"%*s%s",(char*)GConf->szFontFile);
		else if (strstr((char*)line,"fontSize:"))
		  sscanf((char*)line,"%*s%d",(int*)&GConf->uFontSize);
		else if (strstr((char*)line,"nodeRadius:"))
		  sscanf((char*)line,"%*s%d",(int*)&GConf->uNodeRadius);
		else if (strstr((char*)line,"screenConfig:"))
		  sscanf((char*)line,"%*s%dx%dx%d",(int*)&GConf->uScreenWidth,(int*)&GConf->uScreenHeight,(int*)&GConf->uBPP);
		else if (strstr((char*)line,"damping:"))
		  sscanf((char*)line,"%*s%f",&SConf->fDamping);
		else if (strstr((char*)line,"springConstant:"))
		  sscanf((char*)line,"%*s%f",&SConf->fSpringConstant);
		else if (strstr((char*)line,"simulationStep:"))
		  sscanf((char*)line,"%*s%f",&SConf->fSimStep);
		else if (strstr((char*)line,"coulombConstant:"))
		  sscanf((char*)line,"%*s%f",&SConf->fCoulombConstant);
		else if (strstr((char*)line,"springLength:"))
		  sscanf((char*)line,"%*s%d",(int*)&SConf->uMinimumSpringLength);
		else continue;
	}
	
	fclose(fp);
	return 1;
}

int main(int argc,char* argv[])
{

	// Zkontroluj argumenty programu
	if (argc < 3){
	  fprintf(stderr,"Usage: express [path-to-gml-file] [energetic minimum] [manual-motion]\nLukas Pohanka 2011\n\n");
	  exit(0);
	}

	// Pøeèti konfiguraci ze souboru
	const char* szCfgFile = "config";
	GraphicCfg GrConf;
	SimulationCfg SimConf;

	if (!ReadConfigurationFile(szCfgFile,&GrConf,&SimConf)){
	  fprintf(stderr,"Cannot read configuraton file: %s\n",szCfgFile);
	  exit(0);
	}
	
	// Inicializace SDL
	SDL_Surface *MyScreen;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
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
    	MyScreen = SDL_SetVideoMode(GrConf.uScreenWidth,GrConf.uScreenHeight,GrConf.uBPP,SDL_HWSURFACE|SDL_DOUBLEBUF);
	SDL_SetAlpha(MyScreen,SDL_SRCALPHA,SDL_ALPHA_OPAQUE);

    	if (MyScreen == NULL) {
	  fprintf(stderr, "Couldn't set %d x %d x %d video mode: %s\n",GrConf.uScreenWidth,GrConf.uScreenHeight,GrConf.uBPP,SDL_GetError());
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
	if (!CreateModelSurfaces(&MyModel,&GrConf)){
	  fprintf(stderr, "Error rendering graph surfaces.\n");
	  FreeModel(&MyModel);
	  exit(1);
	}
	SetRandomLocations(&MyModel,GrConf);

	// Vyèisti scénu bílou barvou
	SDL_FillRect(MyScreen, 0, SDL_MapRGBA(MyScreen->format,0xff,0xff,0xff,0));

	SDL_Event event; 
	int run = 1,framesCounter = 0,motionAllowed = 1,
	    manualMotion = (argc > 3 && strcmp(argv[3],"1") == 0);

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
		else if (event.type == SDL_VIDEOEXPOSE) 
		  // Pøi pøekreslení okna obnovuj scénu
		  RenderScene(MyScreen,&MyModel,GrConf); 
		else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE && manualMotion)
		  // Kdy¾ je povolen manuální posuv, posouvej simulaci stiskem mezerníku
		  motionAllowed = SimulationStep(&MyModel,SimConf) > SimConf.uMinimumKineticEnergy; 
	    }

	    // Pokud energie nespadla pod povolené minimum, provádìj simulaci
	    if (motionAllowed){
	      // Posuò simulaci
	      if (SDL_GetTicks()-lastTick >= SimConf.fSimStep*1000 && !manualMotion){
			motionAllowed = SimulationStep(&MyModel,SimConf) > SimConf.uMinimumKineticEnergy; 
	        lastTick = SDL_GetTicks();		
	      }
	    
	      // Pøekresli scénu (5 ~ 20 FPS)
	      if (framesCounter == 3){
	        RenderScene(MyScreen,&MyModel,GrConf);
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
