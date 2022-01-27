#include <iostream>

#include <utf8-tools.h>

#include "src/terminal_out.h"
#include "src/terminal_in.h"


/*
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
*/

int main(int argc, char ** argv) {

	const size_t		max_history_size=10;

	tools::terminal_in		ti;
	std::string 			command;
	std::vector<std::string> 	history;
	bool 				running=true,
					redraw=false;
	size_t 				arrow_presses=0;

	history.reserve(max_history_size+1);

	std::cout<<tools::s::reset();

	while(running) {
		std::flush(std::cout);
		auto id=ti.get();

		if(id) {
			switch(id.type) {
				case id.types::chr:
				case id.types::utf8:
					command+=id.get_string_data(); break;
				case id.types::arrow:
					std::cout<<tools::s::pos(1,1)<<"Good arrow press ("<<arrow_presses<<")..."<<std::endl; 
					++arrow_presses; break;
				case id.types::function:
					std::cout<<tools::s::pos(1,1)<<"Function key press ("<<id.function<<")..."<<std::endl; break;
				case id.types::control:
					switch(id.control) {
						case id.controls::backspace:
							if(command.size()) {
								tools::utf8_pop(command); 
							}
						break;
						case id.controls::enter:
							history.insert(std::begin(history), command);
							if(history.size() > max_history_size) history.pop_back();
							redraw=true;
							if(command=="exit") running=false;
							else command.clear();
						break;
						case id.controls::tab:
							command+="\t"; break;
						case id.controls::escape:
							running=false;
						break;
						case id.controls::none: break;
					}
				break;
				case id.types::none:
				case id.types::unknown: break;
			}
		}

		//TODO: We could be a bit more efficient and only redraw when needed...
		std::cout<<tools::s::pos(1,2)<<tools::s::clear_line()<<">>"<<command<<"\n";
		if(redraw) {
			for(const auto& _s : history) {
				std::cout<<tools::s::clear_right()<<"\t"<<_s<<"\n";
			}
			redraw=false;
		}
		std::cout<<tools::s::pos(3+tools::utf8_size(command),2)<<tools::s::flush();
	}

	return 0;
}
