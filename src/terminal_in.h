#ifndef TOOLS_TERMINAL_IN_H
#define TOOLS_TERMINAL_IN_H

#include <termios.h> 	//termios.
#include <unistd.h>
#include <fcntl.h>

namespace tools {

//! Holds the character/arrow/special function retrieved by the terminal.
struct terminal_in_data {
			operator bool() const {return is_input();}
	bool 		is_input() const {return c || arrow!=arrowkeys::none;}
	bool 		is_arrow() const {return arrow!=arrowkeys::none;}
	bool 		is_char() const {return c;}
	bool 		is_backspace() const {return c==127;}
	//TODO: And TAB????
	//TODO: And ENTER???

			terminal_in_data():c{0}, arrow{arrowkeys::none} {
	
	}

	char 		c;
	enum class 	arrowkeys {none, up, down, left, right} arrow;

};

//!Mediates with the terminal to disable line buffering and provide "real-time" key presses.

//!Once constructed, regular input operations must be performed through it until
//!it has been destroyed.

class terminal_in {
	public:

				terminal_in();
				~terminal_in();
	//!Flushes the input stream.
	void			flush();
	//!Gets pressed key data. Non blocking.
	terminal_in_data&	get();

	private:

	termios			terminal_savestate;
	terminal_in_data	data;
	fd_set 			set;
};

}

#endif
