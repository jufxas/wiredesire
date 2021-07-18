FLAGS = -Wall -pedantic -o game
LINKER_FLAGS =  -lsfml-window -lsfml-audio -lsfml-graphics  -lsfml-network -lsfml-system 
INCLUDE = -I /usr/local/Cellar/sfml/2.5.1_1/include
LIB = -L /usr/local/Cellar/sfml/2.5.1_1/lib
LIB_SDL_MAIN = -L /usr/local/Cellar/sfml/2.5.1_1/lib/
FILES = wire.cpp

game:
	g++ -std=c++11 $(FILES) $(INCLUDE) $(LIB) $(LIB_SDL_MAIN) $(FLAGS) $(LINKER_FLAGS)