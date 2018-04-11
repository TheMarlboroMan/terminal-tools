#ifndef TOOLS_TERMINAL_IN_H
#define TOOLS_TERMINAL_IN_H

#include <termios.h> 	//termios.
#include <unistd.h>
#include <fcntl.h>

namespace tools {

//! Holds the character/arrow/special function retrieved by the terminal.
struct terminal_in_data {

	enum class	types {chr, arrow, control, none, unknown};
	enum class 	arrowkeys {none, up, down, left, right};
	enum class	controls {none, enter, backspace, tab};

			operator bool() const {return type!=types::none;}

	//TODO: In implementation file.
	bool 		is_char() const {return c;}
	
	void		set_unknown() {
		type=types::unknown;
	}

	void		set_arrow_from_char(char _c) {

		type=types::arrow;
		switch(_c) {
			case 'A': arrow=arrowkeys::up; break;
			case 'B': arrow=arrowkeys::down; break;
			case 'C': arrow=arrowkeys::right; break;
			case 'D': arrow=arrowkeys::left; break;
			default: type=types::unknown; break;
		}
	}

	void		set_char(char _c) {
		c=_c;
		type=types::chr;
	}

	void		set_control(controls _c) {
		control=_c;
		type=types::control;
	}

	void		reset() {
		type=types::none;
		c=0; 
		arrow=arrowkeys::none;
		control=controls::none;
	}
	//TODO: And TAB????
	//TODO: And ENTER???
	//TODO: And function keys??
	//TODO: And unicode crap???

			terminal_in_data():
		type{types::none}, c{0}, arrow{arrowkeys::none}, control{controls::none} {
	
	}

	types		get_type() const {return type;}

	types		type;
	char 		c;
	arrowkeys 	arrow;
	controls	control;

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

	enum	control_chars {cc_tab=9, cc_backspace=127, cc_enter=10};

	static const size_t	buffer_size=16;

	termios			terminal_savestate;
	terminal_in_data	data;
	fd_set 			set;
	char			buffer[buffer_size];
};

}

#endif
