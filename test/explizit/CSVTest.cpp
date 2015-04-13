
#include <iostream>
#include <fstream>
#include <cassert>
#include "explizit/StateTransitionSystemLoader.hpp"

using namespace std;
using namespace explizit;


int main(int argc, char **argv) {
	assert(argc == 2);
	string filename(argv[1]);
	ifstream stream(filename.c_str());
	StateTransitionSystemLoader loader(stream);
	loader.make();
    return 0;
}