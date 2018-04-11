#include <iostream>
#include "src/terminal_out.h"
#include "src/terminal_in.h"

//TODO: Perhaps create a new class for all this crap.
void draw_hline(int _pos, int _w, char _reg, char _bgn=0, char _end=0) {

	_bgn=_bgn ? _bgn : _reg;
	_end=_end ? _end : _reg;

	tools::f::pos(std::cout, 1, _pos);
	for(int i=1; i<=_w; i++) {
		std::cout<<(i==1 ? _bgn : i==_w ? _end : _reg);
	}
}

void draw_vline(int _pos, int _h, char _reg, char _bgn=0, char _end=0) {

	_bgn=_bgn ? _bgn : _reg;
	_end=_end ? _end : _reg;

	tools::f::pos(std::cout, _pos, 1);
	for(int i=1; i<=_h; i++) {
		std::cout<<(i==1 ? _bgn : i==_h ? _end : _reg)<<tools::s::move(tools::mv::down, 1)<<tools::s::move(tools::mv::left, 1);
	}
}

void draw_box() {
	auto ts=tools::get_termsize();

	using namespace tools;

	std::cout<<s::reset()<<s::text_color(tools::txt_red)<<s::background_color(tools::bg_blue);
	draw_hline(1, ts.w, '=', '#', '#');
	draw_hline(ts.h, ts.w, '=', '#', '#');
	draw_vline(1, ts.h, '|', '#', '#');
	draw_vline(ts.w, ts.h, '|', '#', '#');
	std::cout<<s::reset_text()<<s::flush();
}

int main(int argc, char ** argv) {

	draw_box();
	std::cout<<tools::s::pos(2, 2)<<"Hit x to exit"<<tools::s::flush();

	try {
		tools::terminal_in ti;
		while(true) {
			auto d=ti.get();
			if(d) {
				if(d.c=='x' || d.c=='X') {
					break;
				}
				else {
					std::cout<<tools::s::pos(2, 2)<<"Please, hit x to exit"<<tools::s::flush();
				}
			}
		}
	}
	catch(...) {
	}
}
