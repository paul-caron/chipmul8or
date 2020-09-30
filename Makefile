chipmul8or: main.o chip8.o
	g++ -std=c++11 -o chipmul8or main.o chip8.o -lncurses

main.o: main.cpp
	g++ -std=c++11 -c -o main.o main.cpp

chip8.o: chip8.cpp chip8.hpp
	g++ -std=c++11 -c -o chip8.o chip8.cpp
