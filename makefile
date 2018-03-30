CFLAGS=--std=c++11 

clean:
	rm obj/*; rm ./a.out;

all: main.cpp obj/terminaltools.o
	g++ $(CFLAGS) main.cpp obj/terminaltools.o -o a.out

obj/terminaltools.o: src/terminaltools.h src/terminaltools.cpp
	g++ $(CFLAGS) -c src/terminaltools.cpp -o obj/terminaltools.o
