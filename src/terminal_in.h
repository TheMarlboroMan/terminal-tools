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

			terminal_in_data();
			operator bool() const {return type!=types::none;}
	void		set_unknown();
	void		set_arrow_from_char(char _c);
	void		set_char(char _c);
	void		set_control(controls _c);
	void		reset();

	types		type;
	char 		c;
	//TODO: What about unicode?? We should have a char buffer or a std string.
	//TODO: And function keys??

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
