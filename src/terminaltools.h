#ifndef TERMINAL_TOOLS_H
#define TERMINAL_TOOLS_H

//!Raw set of terminal tools. May be easily wrapped in a class.

#include <string>
#include <vector>
#include <functional>
#include <ostream>

namespace tools {

//!Width and height (in characters) of the terminal.
struct termsize {
	int w, h;
};

//!To be used with "text_color" function.
enum txt_colors {txt_min=29, txt_black, txt_red, txt_green, txt_yellow, 
		txt_blue, txt_magenta, txt_cyan, txt_white, txt_max};

//!To be used with "background_color" function.
enum bg_colors {bg_min=39, bg_black, bg_red, bg_green, bg_yellow, bg_blue, 
		bg_magenta, bg_cyan, bg_white, bg_max};

//!To be used with "move" function.
enum class mv{up, right, down, left};

//!Internal use: will call the fn_winch_handler function if set.
void do_sigwinch_handler(int _sig);
//!Sets the fn_winch_handler function to handle window resizing.
void set_size_change_handler(const std::function<void(int)>& fn);

//!Gets the terminal size in columns and rows.
termsize get_termsize();


//This set of functions needs a ostream as a parameter (such as std::cout). 
//Below there is a set of structs and overloads of the << operator into a
//stream that will do the same thing. These all return the stream, just to
//make it easier to implement the stream solution.

	namespace f {

	//!Saves cursor position.
	std::ostream& save_pos(std::ostream&);
	//!Restores cursor position from a previous save.
	std::ostream& load_pos(std::ostream&);
	//!Moves cursor to position.
	std::ostream& pos(std::ostream&, int, int);
	//!Moves cursor into direction mv for n spaces.
	std::ostream& move(std::ostream&, mv, int=1);
	//!Clears the current or specified line
	std::ostream& clear_line(std::ostream&, int=0);
	std::ostream& clear_line(std::ostream&, int, int);
	//!Resets the terminal.
	std::ostream& reset(std::ostream&);
	//!Sets the text color. May throw if the value is not in the range of txt_colors.
	std::ostream& text_color(std::ostream&, int);
	//!Sets the background color. May throw if the value is not in the range of bg_colors.
	std::ostream& background_color(std::ostream&, int); 
	//!Sets the text effect (see table below).
	std::ostream& text_effect(std::ostream&, int);
	//!Sets the vector of text effects (see table below).
	std::ostream& text_effect(std::ostream&, const std::vector<int>&);
	//!Resets all text effects and colours.
	std::ostream& reset_text(std::ostream&);
	//!Flushes the stream.
	std::ostream& flush(std::ostream&);
}

//These are the stream ones. They seem messy, but are actually small proxies
//for the non-stream function.

	namespace s {

	//!Saves cursor position.
	struct save_pos{};
	std::ostream& operator<<(std::ostream& _s, const save_pos& _t);

	//!Restores cursor position from a previous save.
	struct load_pos{};
	std::ostream& operator<<(std::ostream& _s, const load_pos& _t);

	//!Moves cursor to position.
	struct pos{int x, y; pos(int _x, int _y):x(_x), y(_y){}};
	std::ostream& operator<<(std::ostream& _s, const pos& _t);

	//!Moves cursor into direction mv for n spaces.
	struct move{mv m; int d; move(mv _m, int _d=1):m(_m), d(_d){}};
	std::ostream& operator<<(std::ostream& _s, const move& _t);

	//!Clears the current or specified line
	struct clear_line{int f, t; clear_line(int _f=0, int _t=-1):f(_f), t(_t){}};
	std::ostream& operator<<(std::ostream& _s, const clear_line& _t);

	//!Resets the terminal.
	struct reset{};
	std::ostream& operator<<(std::ostream& _s, const reset& _t);

	//!Sets the text color. May throw if the value is not in the range of txt_colors.
	struct text_color{int v; text_color(int _v):v(_v) {}};
	std::ostream& operator<<(std::ostream& _s, const text_color& _t);

	//!Sets the background color. May throw if the value is not in the range of bg_colors.
	struct background_color{int v; background_color(int _v):v(_v) {}};
	std::ostream& operator<<(std::ostream& _s, const background_color& _t);

	//!Sets the text effect (see table below).
	struct text_effect{int v; std::vector<int> vec;
		text_effect(int _v):v(_v) {}
		text_effect(const std::vector<int>& _v):v(-1), vec(_v) {}
	};
	std::ostream& operator<<(std::ostream& _s, const text_effect& _t);

	//!Resets all text effects and colours.
	struct reset_text{};
	std::ostream& operator<<(std::ostream& _s, const reset_text& _t);

	//!Flushes the stream.
	struct flush{};
	std::ostream& operator<<(std::ostream& _s, const flush& _t);
}

/*
Thanks https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences
╔══════════╦════════════════════════════════╦═════════════════════════════════════════════════════════════════════════╗
║  Code    ║             Effect             ║                                   Note                                  ║
╠══════════╬════════════════════════════════╬═════════════════════════════════════════════════════════════════════════╣
║ 0        ║  Reset / Normal                ║  all attributes off                                                     ║
║ 1        ║  Bold or increased intensity   ║                                                                         ║
║ 2        ║  Faint (decreased intensity)   ║  Not widely supported.                                                  ║
║ 3        ║  Italic                        ║  Not widely supported. Sometimes treated as inverse.                    ║
║ 4        ║  Underline                     ║                                                                         ║
║ 5        ║  Slow Blink                    ║  less than 150 per minute                                               ║
║ 6        ║  Rapid Blink                   ║  MS-DOS ANSI.SYS; 150+ per minute; not widely supported                 ║
║ 7        ║  [[reverse video]]             ║  swap foreground and background colors                                  ║
║ 8        ║  Conceal                       ║  Not widely supported.                                                  ║
║ 9        ║  Crossed-out                   ║  Characters legible, but marked for deletion.  Not widely supported.    ║
║ 10       ║  Primary(default) font         ║                                                                         ║
║ 11–19    ║  Alternate font                ║  Select alternate font `n-10`                                           ║
║ 20       ║  Fraktur                       ║  hardly ever supported                                                  ║
║ 21       ║  Bold off or Double Underline  ║  Bold off not widely supported; double underline hardly ever supported. ║
║ 22       ║  Normal color or intensity     ║  Neither bold nor faint                                                 ║
║ 23       ║  Not italic, not Fraktur       ║                                                                         ║
║ 24       ║  Underline off                 ║  Not singly or doubly underlined                                        ║
║ 25       ║  Blink off                     ║                                                                         ║
║ 27       ║  Inverse off                   ║                                                                         ║
║ 28       ║  Reveal                        ║  conceal off                                                            ║
║ 29       ║  Not crossed out               ║                                                                         ║
║ 30–37    ║  Set foreground color          ║  See color table below                                                  ║
║ 38       ║  Set foreground color          ║  Next arguments are `5;n` or `2;r;g;b`, see below                       ║
║ 39       ║  Default foreground color      ║  implementation defined (according to standard)                         ║
║ 40–47    ║  Set background color          ║  See color table below                                                  ║
║ 48       ║  Set background color          ║  Next arguments are `5;n` or `2;r;g;b`, see below                       ║
║ 49       ║  Default background color      ║  implementation defined (according to standard)                         ║
║ 51       ║  Framed                        ║                                                                         ║
║ 52       ║  Encircled                     ║                                                                         ║
║ 53       ║  Overlined                     ║                                                                         ║
║ 54       ║  Not framed or encircled       ║                                                                         ║
║ 55       ║  Not overlined                 ║                                                                         ║
║ 60       ║  ideogram underline            ║  hardly ever supported                                                  ║
║ 61       ║  ideogram double underline     ║  hardly ever supported                                                  ║
║ 62       ║  ideogram overline             ║  hardly ever supported                                                  ║
║ 63       ║  ideogram double overline      ║  hardly ever supported                                                  ║
║ 64       ║  ideogram stress marking       ║  hardly ever supported                                                  ║
║ 65       ║  ideogram attributes off       ║  reset the effects of all of 60-64                                      ║
║ 90–97    ║  Set bright foreground color   ║  aixterm (not in standard)                                              ║
║ 100–107  ║  Set bright background color   ║  aixterm (not in standard)                                              ║
╚══════════╩════════════════════════════════╩═════════════════════════════════════════════════════════════════════════╝

╔══════════╦═════════╦═════════╗
║  Color   ║ Fg code ║ Bg code ║
╠══════════╬═════════╬═════════╣
║ Black    ║ 30      ║ 40      ║
║ Red      ║ 31      ║ 41      ║
║ Green    ║ 32      ║ 42      ║
║ Yellow   ║ 33      ║ 43      ║
║ Blue     ║ 34      ║ 44      ║
║ Magenta  ║ 35      ║ 45      ║
║ Cyan     ║ 36      ║ 46      ║
║ White    ║ 37      ║ 47      ║
║ B. Black ║ 30;1    ║ 100     ║
║ B. Red   ║ 31;1    ║ 101     ║
║ B. Green ║ 32;1    ║ 102     ║
║ B. Yellow║ 33;1    ║ 103     ║
║ B. Blue  ║ 34;1    ║ 104     ║
║ B. Magn. ║ 35;1    ║ 105     ║
║ B. Cyan  ║ 36;1    ║ 106     ║
║ B. White ║ 37;1    ║ 107     ║
╚══════════╩═════════╩═════════╝
*/

}

#endif
