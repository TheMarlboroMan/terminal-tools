#include "terminal_in.h"

#include <iostream>
#include <stdexcept>

#include <fcntl.h>

using namespace tools;

terminal_in::terminal_in() {

	FD_ZERO(&set);
	FD_SET(STDIN_FILENO, &set);

	int fl;
	if ((fl = fcntl (STDIN_FILENO, F_GETFL)) < 0) {
		throw std::runtime_error("Unable to capture stdin flags");
	}

	if (fcntl(STDIN_FILENO, F_SETFL, fl | O_NONBLOCK)) {
		throw std::runtime_error("Unable to set stdin flags");
	}

	if (0 > tcgetattr(STDIN_FILENO, &terminal_savestate)) {
		throw std::runtime_error("Unable save terminal attributes");
	}

	termios terminal_newstate(terminal_savestate);
	terminal_newstate.c_lflag &= ~(ICANON);

	//TODO: It would be good to choose to enable OR disable the echo.
	terminal_newstate.c_lflag &= ~(ECHO);
	terminal_newstate.c_cc[VMIN]=1;
	terminal_newstate.c_cc[VTIME]=0;

	flush();

	if(0 > tcsetattr (STDIN_FILENO, TCSAFLUSH, &terminal_newstate)) {
		throw std::runtime_error("Unable to set new attributes");
	}
}

terminal_in::~terminal_in() {

	flush();

	if(tcsetattr (STDIN_FILENO, TCSANOW, &terminal_savestate)) {
		throw std::runtime_error("Unable to restore terminal state");
	}
}

void terminal_in::flush() {
	//TODO: Experiment with discarding shit.
	if(0 > tcflush(STDIN_FILENO, TCIFLUSH)) {
		throw std::runtime_error("Unable to flush input");
	}
}

	//TODO like... terrible XD!.
terminal_in_data& terminal_in::get() {

	//This prevents blocking.
	timeval tv {0, 10000};
	auto cp=set;
	if(select(STDIN_FILENO+1, &cp, nullptr, nullptr, &tv)) {

		//TODO: Please do this right... 
		//We can either read 3 at a time and hope for the best
		//or read two if we get a scape.
		char c;
		read(STDIN_FILENO, &c, 1);

		//TODO: No magic numbers!
		//This is the begin of a scape sequence...
		if(c==27) {
			read(STDIN_FILENO, &c, 1);
			//This is "["
			//TODO; No magic numbers!
			if(c==91) {
				read(STDIN_FILENO, &c, 1);
				//These are actually the cursors!.
				switch(c) {
					case 'A': data.arrow=terminal_in_data::arrowkeys::up; break;
					case 'B': data.arrow=terminal_in_data::arrowkeys::down; break;
					case 'C': data.arrow=terminal_in_data::arrowkeys::right; break;
					case 'D': data.arrow=terminal_in_data::arrowkeys::left; break;
				}
			}
			//Control + keys and stuff... Control+C will still work :D.
			else {
				//TODO... Ok??? Shouldn't we just consume what we need?
				flush(); //Lol
			}
		}
		//Backspace, chars...
		//TODO: No ,magic sequences!
		else if(c==127 || isprint(c)) {
			data.c=c;
		}
		else {
			//TODO... Ok??? Shouldn't we just consume what we need?
			flush(); //Lol
		}
	}

	return data;
}

