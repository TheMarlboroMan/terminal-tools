#ifndef TOOLS_TERMINAL_IN_H
#define TOOLS_TERMINAL_IN_H

#include <array>

#include <termios.h> 	//termios.
#include <unistd.h>
#include <fcntl.h>

namespace tools {

//! Holds the character/arrow/special function retrieved by the terminal.
struct terminal_in_data {

	enum class	types {chr, utf8, arrow, control, function, none, unknown};
	enum class 	arrowkeys {none, up, down, left, right};
	enum class	controls {none, enter, backspace, tab, escape};

			terminal_in_data();
			operator bool() const {return type!=types::none;}
	std::string	get_string_data() const {return std::string(buffer.data());}
	bool		is_stringlike() const {return type==types::chr || type==types::utf8;}

	void		set_unknown();
	void		set_char();
	void		set_utf8();
	void		set_arrow_from_char(char);
	void		set_control(controls);
	void		set_function(int);
	void		reset();

	static const size_t		buffer_size=8; //TODO: check how much do we need.º
	std::size_t			read; //how much was read in the last chunk.
	types				type;
	std::array<char, buffer_size>	buffer;

	arrowkeys 			arrow;
	controls			control;
	int				function;

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

	enum	control_chars {cc_tab=9, cc_backspace=127, cc_enter=10, cc_escape=27};

	//The showkey command is our friend.
	static const int	escape_code_start=27;
	static const int    escape_code_end=91;
	static const int	f1_code=65;

	termios			terminal_savestate;
	terminal_in_data	data;
	fd_set 			set;
};

}

#endif
