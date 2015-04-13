
#include <iostream>
#include <fstream>
#include <cassert>
#include "explizit/StateTransitionSystemLoader.hpp"

using namespace std;
using namespace explizit;


int main(int argc, char **argv) {
	//assert(argc == 2);
	cout << "testing CSV" << endl;
	string filename(argv[1]);
	cout << "Got filename: " << filename  << " from cmd-parameter  "  << endl;
	ifstream stream(filename.c_str());
	cout << "loaded the file" << endl;
	StateTransitionSystemLoader loader(stream);
	cout << "initialized the StateTransitionSystemLoader" << endl;
	loader.make();
	cout << "Made the state transition system" << endl;
    return 0;
}
