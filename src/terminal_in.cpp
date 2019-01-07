#include "terminal_in.h"

#include <iostream>
#include <stdexcept>

///#include <string> //Getline... TODO
#include <cstring>
#include <climits> //CHAR_BIT

#include <fcntl.h>

#include <utf8-tools.h>

using namespace tools;

terminal_in_data::terminal_in_data():
	type{types::none}, arrow{arrowkeys::none}, control{controls::none} {
	buffer.fill(0);
}

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

void terminal_in_data::set_char() {
	type=types::chr;
}

void terminal_in_data::set_utf8() {
	type=types::utf8;
}

void terminal_in_data::set_control(controls _c) {
	control=_c;
	type=types::control;
}

void terminal_in_data::set_function(int _f) {
	type=types::function;
	function=_f+1; //Function keys are labelled 1 to 12.
}

void terminal_in_data::reset() {
	type=types::none;
	buffer.fill(0);
	arrow=arrowkeys::none;
	control=controls::none;
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

//TODO: Remove in non debug builds...
	auto debug_buffer=[this](int read) {
		std::cout<<"READ "<<read<<" CHARS"<<std::endl;
		for(int i=0; i<read; i++) {
			std::cout<<i<<":"<<(unsigned short)data.buffer[i]<<" ("<<sizeof(data.buffer[i])<<") "<<CHAR_BIT<<" | ";
		}
		std::cout<<std::endl;
	};

	//This prevents blocking.
	timeval tv {0, 10000};
	auto cp=set;
	if(select(STDIN_FILENO+1, &cp, nullptr, nullptr, &tv) > 0) {

		int readcount=read(STDIN_FILENO, &data.buffer, data.buffer_size);

		//This is an alternative too...
//		std::string buffer;
//		std::getline(std::cin, buffer);
//		std::cin.clear();
//		int readcount=buffer.size();

		if(readcount==1) {
			switch(data.buffer[0]) {
				case cc_backspace:
					data.set_control(terminal_in_data::controls::backspace); break;
				case cc_tab:
					data.set_control(terminal_in_data::controls::tab); break;
				case cc_enter:
					data.set_control(terminal_in_data::controls::enter); break;
				case cc_escape:
					data.set_control(terminal_in_data::controls::escape); break;
				default:
					if(isprint(data.buffer[0])) {
						data.set_char();
					}
					else { //Unprintable.
						data.set_unknown();
					}
				break;
			}
		}
		else {
			//TODO: SWITCH ON THE BUFFER SIZE, SEEMS BETTER.
			switch(data.buffer[0]) {

				

				case escape_code:
					switch(data.buffer[1]) {
						case escape_arrow:
							data.set_arrow_from_char(data.buffer[2]); break;
						//TODO: This does not work with the tty1-4 terminals, which have different codes.
						case escape_function_key_1_to_4:
							data.set_function(data.buffer[2]-f1_code); break;
						default:
						//TODO: What about the rest?
							std::cout<<"???"<<data.buffer[2]<<" "<<(int)data.buffer[2]<<std::endl;
							data.set_unknown();
//							debug_buffer(readcount); 
						break;
					}
				break;
				default:
					if(tools::is_utf8(data.buffer[0])) {
						data.set_utf8();
					}
					else {
						data.set_unknown();
						debug_buffer(readcount);
					}
				break;
			}
		}
	}

	return data;
}

