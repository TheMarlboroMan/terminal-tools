#include <tools/terminal_in.h>

#include <iostream>
#include <stdexcept>
#include <stdlib.h>

#include <string>
#include <cstring>
#include <climits> //CHAR_BIT

#include <fcntl.h>

#include <tools/utf8-tools.h>

using namespace tools;

terminal_in_data::terminal_in_data():
	type{types::none}, 
	arrow{arrowkeys::none}, 
	control{controls::none} 
{

	buffer.fill(0);
}

void terminal_in_data::set_unknown() {

	type=types::unknown;
}

void terminal_in_data::set_arrow(
	arrowkeys _val
) {

	type=types::arrow;
	arrow=_val;
}

void terminal_in_data::set_char() {

	type=types::chr;
}

void terminal_in_data::set_utf8() {

	type=types::utf8;
}

void terminal_in_data::set_control(
	controls _c
) {

	control=_c;
	type=types::control;
}

void terminal_in_data::set_function(
	int _f
) {

	type=types::function;
	function=_f;
}

void terminal_in_data::reset() {

	type=types::none;
	buffer.fill(0);
	arrow=arrowkeys::none;
	control=controls::none;
}

////////////////////////////////////////////////////////////////////////////////

terminal_in::terminal_in(
	bool _compulsory_sequence_match
) {

	if(nullptr==getenv("TERM")) {

		throw std::runtime_error("could not grab terminal type");
	}

	std::string terminal_name=getenv("TERM");
	setup_sequences(terminal_name);
	if(nullptr==sequence.get() && _compulsory_sequence_match) {

		throw std::runtime_error(std::string{"could not find a sequence matcher for terminal "}+terminal_name);
	}
	
	//These are nasty macros to reset a file descriptor set and to add one
	//to it... First we clear it, then we add the stdin.
	FD_ZERO(&set);
	FD_SET(STDIN_FILENO, &set);

	memset(&terminal_savestate, 0, sizeof(termios));

	//Grab the current flags of stdin
	int fl;
	if ((fl = fcntl (STDIN_FILENO, F_GETFL)) < 0) {
		throw std::runtime_error("Unable to capture stdin flags");
	}

	//Set the same flags plus non-blocking input.
	if (fcntl(STDIN_FILENO, F_SETFL, fl | O_NONBLOCK)) {
		throw std::runtime_error("Unable to set stdin flags");
	}

	//Save terminal state so we can restore it on exit.
	if (0 > tcgetattr(STDIN_FILENO, &terminal_savestate)) {
		throw std::runtime_error("Unable save terminal attributes");
	}

	//Set up a new state which is non in canonical (line by line) mode,
	//making data available as soon as it is entered.
	termios terminal_newstate(terminal_savestate);
	terminal_newstate.c_lflag &= ~(ICANON);

	//TODO: It would be good to choose to enable OR disable the echo.
	terminal_newstate.c_lflag &= ~(ECHO);

	//setting up how a call to read is complete: min > 0 & time = 0 means 
	//blocking read in which blocks are read until min are available (see
	//termios man).
	terminal_newstate.c_cc[VMIN]=1;
	terminal_newstate.c_cc[VTIME]=0;

	flush();

	//set new terminal attributes.
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

	if(0 > tcflush(STDIN_FILENO, TCIFLUSH)) {

		throw std::runtime_error("Unable to flush input");
	}
}

terminal_in_data& terminal_in::get() {

	data.reset();

	//This prevents blocking.
	timeval tv {0, 10000};
	auto cp=set;
	if(select(STDIN_FILENO+1, &cp, nullptr, nullptr, &tv) <= 0) {

		return data;
	}

	data.read=read(STDIN_FILENO, &data.buffer, data.buffer_size);

	//one ordinary character...
	if(data.read==1) {

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

		return data;
	}

	//an utf-8 sequence?
	if(tools::is_utf8(data.buffer[0])) {

		data.set_utf8();
		return data;
	}

	//if possible, derive to specific terminal sequences...
	data.set_unknown();
	if(nullptr!=sequence) {

		sequence->read_sequences(data);
	}

	return data;
}

void terminal_in::setup_sequences(
	const std::string& _termname
) {
	//It would be nicer if we could read the terminfo files, but I don't know the
	//format. There is https://github.com/sabotage-linux/netbsd-curses/blob/master/infocmp/infocmp.c
	//which should be able to teach us a thing or two.
	if(_termname=="linux") {

		sequence.reset(new linux_terminal_sequences{});
		return;
	}

	if(_termname.substr(0, 5)=="xterm") {

		sequence.reset(new xterm_terminal_sequences{});
		return; } }

void linux_terminal_sequences::read_sequences( terminal_in_data& _in
) {

	//cursors are 27 + 91 + x
	if(3==_in.read) {

		//the first character is a control character...
		if(27 != _in.buffer[0]) {

			return;
		}

		if(91 == _in.buffer[1]) {

			switch(_in.buffer[2]) {

				case 65: _in.set_arrow(_in.arrowkeys::up); return;
				case 66: _in.set_arrow(_in.arrowkeys::down); return;
				case 67: _in.set_arrow(_in.arrowkeys::right); return;
				case 68: _in.set_arrow(_in.arrowkeys::left); return;
			}
		}
	}

	//f1 to f5 are 27 91 91 x
	if(4==_in.read) {

		if(!
			(27==_in.buffer[0] && 91 ==_in.buffer[1] && 91 == _in.buffer[2])
		) {

			return;
		}

		switch(_in.buffer[3]) {
			case 65: _in.set_function(1); return;
			case 66: _in.set_function(2); return;
			case 67: _in.set_function(3); return;
			case 68: _in.set_function(4); return;
			case 69: _in.set_function(5); return;
		}
	}

	//the rest of function keys are 27 91 x y 126
	if(5==_in.read) {

		if(!
			(27==_in.buffer[0] && 91 ==_in.buffer[1] && 126==_in.buffer[4])
		) {

			return;
		}

		switch(_in.buffer[2]) {
			case 49:
				switch(_in.buffer[3]) {
					case 55: _in.set_function(6); return;
					case 56: _in.set_function(7); return;
					case 57: _in.set_function(8); return;
				}
			break;
			case 50:
				switch(_in.buffer[3]) {
					case 48: _in.set_function(9); return;
					case 49: _in.set_function(10); return;
					case 51: _in.set_function(11); return;
					case 52: _in.set_function(12); return;
				break;
			}
		}
	}
}

void xterm_terminal_sequences::read_sequences(
	terminal_in_data& _in
) {

	//We might put these into std::arrays and test for equality, but there
	//would be some rendundancy in the checks...
	if(3==_in.read) {

		//the first character is a control character...
		if(27 != _in.buffer[0]) {

			return;
		}

		if(91 == _in.buffer[1]) {

			switch(_in.buffer[2]) {

				case 65: _in.set_arrow(_in.arrowkeys::up); return;
				case 66: _in.set_arrow(_in.arrowkeys::down); return;
				case 67: _in.set_arrow(_in.arrowkeys::right); return;
				case 68: _in.set_arrow(_in.arrowkeys::left); return;
			}
		}

		if(79 == _in.buffer[1]) {

			switch(_in.buffer[2]) {
				case 80: _in.set_function(1); return; 
				case 81: _in.set_function(2); return; 
				case 82: _in.set_function(3); return; 
				case 83: _in.set_function(4); return; 
			}
		}
	}

	if(5==_in.read) {

		//assume the sequences are 27+91+x+y+126...
		if(! 
			(_in.buffer[0]==27 && _in.buffer[1]==91 && _in.buffer[4]==126)
		) {

			return;
		}

		if(_in.buffer[2]==49) {

			switch(_in.buffer[3]) {
				case 53: _in.set_function(5); return;
				case 55: _in.set_function(6); return;
				case 56: _in.set_function(7); return;
				case 57: _in.set_function(8); return;
			}
		}

		if(_in.buffer[2]==50) {

			switch(_in.buffer[3]) {
				case 48: _in.set_function(9); return;
				case 49: _in.set_function(10); return;
				case 51: _in.set_function(11); return;
				case 52: _in.set_function(12); return;
			}
		}
	}
}
