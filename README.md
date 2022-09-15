# term tools
Small collection of terminal functions, for my own amusement.

# when to use

Under no circumstances I do endorse the use of these when you can use ncurses instead.

The terminal input function keys work with derivatives of xterm (echo $TERM) and should also work with linux tty (control alt f1 and so on...)

The main.cpp file contains a small demonstration of how this thing is used. The buffer.cpp file will allow you to print the bytes coming in from the keyboard, in case there is need to add more terminal types (each one handles function keys their own way...)
