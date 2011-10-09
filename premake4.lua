solution "Zapoctak"
	configurations { "Debug", "Release" }
	language "C"	
	location "build"

project "gml-parser"
	kind "SharedLib"
	files { "gml/*.c", "gml/*.h" }
	configuration "Debug"
		flags { "Symbols" }
	configuration "Release"
		flags { "OptimizeSpeed" }
	
project "Express"
	kind "ConsoleApp"
	files { "main.c", "main.h", "drawing.c" , "drawing.h", "model.c", "model.h" }
	links { "SDL", "SDLmain", "SDL_ttf", "gml-parser"}
	targetname "express"
	configuration "Debug"
		defines { "DEBUG" }
		flags { "Symbols", "ExtraWarnings", "FatalWarnings" }
		buildoptions { "-std=c99", "-pedantic" }
	configuration "Release"
		defines { "NDEBUG" }
		flags { "OptimizeSpeed" }
		buildoptions { "-std=c99" , "-pedantic" }
