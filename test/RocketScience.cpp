
#include <iostream>
#include "cegar/cegar.hpp"


int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cout << std::endl << "ERROR!" << std::endl << "Usage: " << argv[0] << " <program file name>" << std::endl;
		return 1;
	}

	std::string progfile = argv[1];
	bool correct = cegar::prove(progfile);
}
