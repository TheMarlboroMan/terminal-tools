#include "terminal_in.h"

#include <iostream>
#include <stdexcept>

///#include <string> //Getline... TODO
#include <cstring>
#include <climits> //CHAR_BIT

#include <fcntl.h>

using namespace tools;

void terminal_in_data::set_unknown() {
	type=types::unknown;
}

void terminal_in_data::set_arrow_from_char(char _c) {

	type=types::arrow;
	switch(_c) {
		case 'A': arrow=arrowkeys::up; break;
		case 'B': arrow=arrowkeys::down; break;
		case 'C': arrow=arrowkeys::right; break;
		case 'D': arrow=arrowkeys::left; break;
		default: type=types::unknown; break;
	}
}

void terminal_in_data::set_char(char _c) {
	c=_c;
	type=types::chr;
}

void terminal_in_data::set_control(controls _c) {
	control=_c;
	type=types::control;
}

void terminal_in_data::reset() {
	type=types::none;
	c=0; 
	arrow=arrowkeys::none;
	control=controls::none;
}

terminal_in_data::terminal_in_data():
	type{types::none}, c{0}, arrow{arrowkeys::none}, control{controls::none} {
}

////////////////////////////////////////////////////////////////////////////////

terminal_in::terminal_in() {

	FD_ZERO(&set);
	FD_SET(STDIN_FILENO, &set);

	memset(&terminal_savestate, 0, sizeof(termios));

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

terminal_in_data& terminal_in::get() {

	data.reset();

	//TODO... Oh, I see...
	auto debug_buffer=[this](int read) {
		std::cout<<"READ "<<read<<" CHARS"<<std::endl;
		for(int i=0; i<read; i++) {
			std::cout<<i<<":"<<(unsigned short)buffer[i]<<" ("<<sizeof(buffer[i])<<") "<<CHAR_BIT<<" | ";
		}
		std::cout<<std::endl;
	};

	//This prevents blocking.
	timeval tv {0, 10000};
	auto cp=set;
	if(select(STDIN_FILENO+1, &cp, nullptr, nullptr, &tv) > 0) {

		memset(buffer, 0, sizeof(buffer_size));
		int readcount=read(STDIN_FILENO, buffer, buffer_size);

		//This is an alternative too...
//		std::string buffer;
//		std::getline(std::cin, buffer);
//		std::cin.clear();
//		int readcount=buffer.size();

		if(readcount==1) {
			switch(buffer[0]) {
				case cc_backspace:
					data.set_control(terminal_in_data::controls::backspace); break;
				case cc_tab:
					data.set_control(terminal_in_data::controls::tab); break;
				case cc_enter:
					data.set_control(terminal_in_data::controls::enter); break;
				default:
					if(isprint(buffer[0])) {
						data.set_char(buffer[0]);
					}
					else {
						data.set_unknown();
						debug_buffer(readcount);
					}
				break;
			}
		}
		else {
			if(buffer[0]==27) {
				//This is "["
				//TODO; No magic numbers!
				if(buffer[1]==91) {
					data.set_arrow_from_char(buffer[2]);
				}
				else {
					data.set_unknown();
					debug_buffer(readcount);
				}
			}
			else {
				data.set_unknown();
				debug_buffer(readcount);
			}
		}
	}

	return data;
}

