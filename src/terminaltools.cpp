#include "terminaltools.h"

#include <iostream>
#include <iterator>
#include <stdexcept>

#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>

using namespace tools;

void tools::do_sigwinch_handler(int _sig) {
	if(_sig==SIGWINCH && fn_winch_handler) {
		fn_winch_handler(_sig);
	}
}

void tools::set_size_change_handler(const std::function<void(int)>& fn) {
	signal(SIGWINCH, do_sigwinch_handler);
	fn_winch_handler=fn;
}

termsize tools::get_termsize() {
	winsize ws;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
	return termsize{ws.ws_col, ws.ws_row};
}


//Set of functions with stream as parameter.
std::ostream& tools::f::save_pos(std::ostream& _s) {
	return _s<<tools::esseq<<"s";
}

std::ostream& tools::f::load_pos(std::ostream& _s) {
	return _s<<tools::esseq<<"u";
}

std::ostream& tools::f::pos(std::ostream& _s, int x, int y) {
	return _s<<tools::esseq<<y<<";"<<x<<"f";
}

std::ostream& tools::f::clear_line(std::ostream& _s, int _y) {
	if(_y) {
		tools::f::pos(_s, 1, _y);
	}
	return _s<<tools::esseq<<"2K";
}

std::ostream& tools::f::clear_line(std::ostream& _s, int _from, int _to) {

	//TODO: Check with the same value.
	if(_to < _from) {
		throw std::runtime_error("invalid clear line parameters");
	}

	tools::f::pos(_s, 1, _from);
	for(_from; _from<_to; _from++) {
		_s<<tools::esseq<<"2K";
		tools::f::move(_s, tools::mv::down);
	}

	return _s;
}

std::ostream& tools::f::reset(std::ostream& _s) {
	return _s<<tools::esseq<<"2J";
}

std::ostream& tools::f::move(std::ostream& _s, tools::mv _d, int _dist) {
	switch(_d) {
		case tools::mv::up: 
			return _s<<tools::esseq<<_dist<<"A"; break;
		case tools::mv::right: 
			return _s<<tools::esseq<<_dist<<"C"; break;
		case tools::mv::down: 
			return _s<<tools::esseq<<_dist<<"B"; break;
		case tools::mv::left:  
			return _s<<tools::esseq<<_dist<<"D"; break;
	}
}

std::ostream& tools::f::reset_text(std::ostream& _s) {
	return _s<<tools::esseq<<"0m";
}

std::ostream& tools::f::text_effect(std::ostream& _s, int _eff) {
	return _s<<tools::esseq<<_eff<<"m";
}

std::ostream& tools::f::text_effect(std::ostream& _s, const std::vector<int>& _eff) {
	_s<<tools::esseq;
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

