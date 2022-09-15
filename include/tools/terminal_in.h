#pragma once

#include <array>
#include <memory>
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
	void        set_arrow(arrowkeys);
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

/**
* base class for a translator of terminal sequences that can understand 
* different function keys, cursors and so on for different terminals.
*/
class terminal_sequence {

	public:
	virtual void read_sequences(terminal_in_data&)=0;
};

/**
* sequences for the linux terminals (tty 1 to 6 or so...).
*/
class linux_terminal_sequences:
	public terminal_sequence {
	public:
	void        read_sequences(terminal_in_data&);
};

/**
*sequences for xterm, which are most (if not all) graphical terminals I have 
tried.
*/
class xterm_terminal_sequences:
	public terminal_sequence {
	public:
	void        read_sequences(terminal_in_data&);
};

//!Mediates with the terminal to disable line buffering and provide "real-time" key presses.

//!Once constructed, regular input operations must be performed through it until
//!it has been destroyed.

class terminal_in {
	public:

				terminal_in(bool=true);
				~terminal_in();
	//!Flushes the input stream.
	void			flush();
	//!Gets pressed key data. Non blocking.
	terminal_in_data&	get();

	private:

	void                setup_sequences(const std::string&);
	//TODO: actually, these will... depend.
	enum	control_chars {cc_tab=9, cc_backspace=127, cc_enter=10, cc_escape=27};

	//The showkey command is our friend.
	static const int	escape_code_start=27;
	static const int    escape_code_end=91;
	static const int	f1_code=65;

	std::unique_ptr<terminal_sequence> sequence{nullptr};
	termios             terminal_savestate;
	terminal_in_data    data;
	fd_set              set;
};

}

