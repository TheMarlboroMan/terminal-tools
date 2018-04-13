INCLUDES=-I ../utf8-tools/src/
DEPS=../utf8-tools/obj/utf8-tools.o
COMPILER=g++
#DEBUG=-g
CFLAGS=-Wno-deprecated -Wall -ansi -pedantic -std=c++11 -Wfatal-errors


all: objdir main.cpp obj/terminal_out.o obj/terminal_in.o $(DEPS)
	$(COMPILER) $(CFLAGS) $(DEBUG) $(INCLUDES) main.cpp obj/terminal_out.o obj/terminal_in.o $(DEPS) -o a.out

objdir:
	mkdir -p obj;

clean:
	rm -rf obj; rmdir obj; rm ./a.out; 


obj/terminal_out.o: src/terminal_out.h src/terminal_out.cpp
	$(COMPILER) $(CFLAGS) $(DEBUG) $(INCLUDES) -c src/terminal_out.cpp -o obj/terminal_out.o

obj/terminal_in.o: src/terminal_in.h src/terminal_in.cpp
	$(COMPILER) $(CFLAGS) $(DEBUG) $(INCLUDES) -c src/terminal_in.cpp -o obj/terminal_in.o
