INCLUDES=-I../terminal-tools/src/ -I ../tools/class/ -I ../log/src/
COMPILER=g++
DEBUG=-g
CFLAGS=-Wno-deprecated -Wall -ansi -pedantic -std=c++11 -Wfatal-errors

all: objdir main.cpp obj/terminal_out.o obj/terminal_in.o
	$(COMPILER) $(CFLAGS) $(DEBUG) main.cpp obj/terminal_out.o obj/terminal_in.o -o a.out

objdir:
	mkdir obj;

clean:
	rm obj/*; rm ./a.out;


obj/terminal_out.o: src/terminal_out.h src/terminal_out.cpp
	$(COMPILER) $(CFLAGS) $(DEBUG) -c src/terminal_out.cpp -o obj/terminal_out.o

obj/terminal_in.o: src/terminal_in.h src/terminal_in.cpp
	$(COMPILER) $(CFLAGS) $(DEBUG) -c src/terminal_in.cpp -o obj/terminal_in.o
