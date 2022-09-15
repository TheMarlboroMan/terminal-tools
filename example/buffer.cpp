#include <iostream>
#include <tools/terminal_in.h>

int main(int, char **) {

	tools::terminal_in in(false);

	while(true) {

		auto input=in.get();

		if(!input) {

			continue;
		}

		if(input.type==input.types::control && input.control==input.controls::escape) {

			break;
		}

		std::cout<<"read "<<input.read<<" bytes..."<<std::endl;
		for(std::size_t i=0; i<input.read; i++) {

			std::cout<<"\t["<<i<<"] => "<<(int)input.buffer[i]<<std::endl;
		}
	}

	return 0;
}


