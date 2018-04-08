#include "terminaltools.h"

#include <iostream>
#include <iterator>
#include <stdexcept>

#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>

const std::string esseq="\033["; //! <Escape sequence.
std::function<void(int)> fn_winch_handler;	//! <std function to help with window resizing.
//std::function<void(int)> tools::fn_winch_handler=nullptr;	//! <std function to help with window resizing.

void tools::do_sigwinch_handler(int _sig) {
	if(_sig==SIGWINCH && fn_winch_handler) {
		fn_winch_handler(_sig);
	}
}

void tools::set_size_change_handler(const std::function<void(int)>& fn) {
	signal(SIGWINCH, do_sigwinch_handler);
	fn_winch_handler=fn;
}

tools::termsize tools::get_termsize() {
	winsize ws;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
	return tools::termsize{ws.ws_col, ws.ws_row};
}


//Set of functions with stream as parameter.
std::ostream& tools::f::save_pos(std::ostream& _s) {
	return _s<<esseq<<"s";
}

std::ostream& tools::f::load_pos(std::ostream& _s) {
	return _s<<esseq<<"u";
}

std::ostream& tools::f::pos(std::ostream& _s, int x, int y) {
	return _s<<esseq<<y<<";"<<x<<"f";
}

std::ostream& tools::f::clear_line(std::ostream& _s, int _y) {
	if(_y) {
		tools::f::pos(_s, 1, _y);
	}
	return _s<<esseq<<"2K";
}

std::ostream& tools::f::clear_line(std::ostream& _s, int _from, int _to) {

	//TODO: Check with the same value.
	if(_to < _from) {
		throw std::runtime_error("invalid clear line parameters");
	}

	tools::f::pos(_s, 1, _from);
	for(_from; _from<_to; _from++) {
		_s<<esseq<<"2K";
		tools::f::move(_s, tools::mv::down);
	}

	return _s;
}

std::ostream& tools::f::clear_right(std::ostream& _s) {
	return _s<<esseq<<"0K";
}

std::ostream& tools::f::clear_left(std::ostream& _s) {
	return _s<<esseq<<"1K";
}

std::ostream& tools::f::reset(std::ostream& _s) {
	return _s<<esseq<<"2J";
}

std::ostream& tools::f::move(std::ostream& _s, tools::mv _d, int _dist) {
	switch(_d) {
		case tools::mv::up: 
			return _s<<esseq<<_dist<<"A"; break;
		case tools::mv::right: 
			return _s<<esseq<<_dist<<"C"; break;
		case tools::mv::down: 
			return _s<<esseq<<_dist<<"B"; break;
		case tools::mv::left:  
			return _s<<esseq<<_dist<<"D"; break;
	}
}

std::ostream& tools::f::reset_text(std::ostream& _s) {
	return _s<<esseq<<"0m";
}

std::ostream& tools::f::text_effect(std::ostream& _s, int _eff) {
	return _s<<esseq<<_eff<<"m";
}

std::ostream& tools::f::text_effect(std::ostream& _s, const std::vector<int>& _eff) {
	_s<<esseq;
	for(const auto& i : _eff) {
		_s<<i<<(i==*std::prev(std::end(_eff)) ? 'm' : ';');
	}
	return _s;
}

std::ostream& tools::f::text_color(std::ostream& _s, int _clr){
	if(_clr <= tools::txt_min || _clr >= tools::txt_max) {
		throw std::runtime_error("invalid ANSI foreground color code");
	}
	return tools::f::text_effect(_s, _clr);
}

std::ostream& tools::f::background_color(std::ostream& _s, int _clr) {
	if(_clr <= tools::bg_min || _clr >= tools::bg_max) {
		throw std::runtime_error("invalid ANSI background color code");
	}
	return tools::f::text_effect(_s, _clr);
}

std::ostream& tools::f::flush(std::ostream& _s) {
	return std::flush(_s); //May seem redundant, but well... for completeness sake :).
}

std::ostream& tools::f::hide(std::ostream& _s) {
	_s<<esseq<<"[?25l";
}

std::ostream& tools::f::show(std::ostream& _s) {
	_s<<esseq<<"[?25h";
}

//Set of stream manipulator functions...
std::ostream& tools::s::operator<<(std::ostream& _s, const tools::s::save_pos& _t) {return tools::f::save_pos(_s);}
std::ostream& tools::s::operator<<(std::ostream& _s, const tools::s::load_pos& _t) {return tools::f::load_pos(_s);}
std::ostream& tools::s::operator<<(std::ostream& _s, const tools::s::pos& _t) {return tools::f::pos(_s, _t.x, _t.y);}
std::ostream& tools::s::operator<<(std::ostream& _s, const tools::s::move& _t) {return tools::f::move(_s, _t.m, _t.d);}
std::ostream& tools::s::operator<<(std::ostream& _s, const tools::s::clear_line& _t) {return _t.t==-1 ? tools::f::clear_line(_s, _t.f) : tools::f::clear_line(_s, _t.f, _t.t);}
std::ostream& tools::s::operator<<(std::ostream& _s, const tools::s::clear_left& _t) {return tools::f::clear_left(_s);}
std::ostream& tools::s::operator<<(std::ostream& _s, const tools::s::clear_right& _t) {return tools::f::clear_right(_s);}
std::ostream& tools::s::operator<<(std::ostream& _s, const tools::s::reset& _t) {return tools::f::reset(_s);}
std::ostream& tools::s::operator<<(std::ostream& _s, const tools::s::text_color& _t) {return tools::f::text_color(_s, _t.v);}
std::ostream& tools::s::operator<<(std::ostream& _s, const tools::s::background_color& _t) {return tools::f::background_color(_s, _t.v);}
std::ostream& tools::s::operator<<(std::ostream& _s, const tools::s::text_effect& _t) {return _t.v==-1 ? tools::f::text_effect(_s, _t.vec) : tools::f::text_effect(_s, _t.v);}
std::ostream& tools::s::operator<<(std::ostream& _s, const tools::s::reset_text& _t) {return tools::f::reset_text(_s);}
std::ostream& tools::s::operator<<(std::ostream& _s, const tools::s::flush& _t) {return tools::f::flush(_s);}
std::ostream& tools::s::operator<<(std::ostream& _s, const tools::s::hide& _t) {return tools::f::hide(_s);}
std::ostream& tools::s::operator<<(std::ostream& _s, const tools::s::show& _t) {return tools::f::show(_s);}
