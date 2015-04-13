
#include <iostream>
#include <fstream>
#include <cassert>
#include <math.h>
#include "explizit/StateTransitionSystemLoader.hpp"
#include "explizit/Reachability.hpp"

using namespace std;
using namespace explizit;


int main(int argc, char **argv) {
	assert(argc == 4);

	// TODO: maybe we want to get them from prompt args
	int num_vars = stoi(argv[2]);
	int depth = stoi(argv[3]);

	ifstream stream(argv[1]);
	StateTransitionSystemLoader loader(stream);
	
	unique_ptr<StateTransitionSystem> sts = loader.make();
	State* si = sts->states()[0];

	VariableAssignment vars = VariableAssignment(num_vars);
	for (auto *var : sts->variables()) {
		vars.set(var, false);
	}

	ReachabilitySet rs = computeReachabilitySet(Configuration(si, move(vars)));

	size_t siex = (num_vars + 1) + pow(2, depth + 1) - 1;
	cout << endl << "SIZE expected: " << siex;
	size_t size = rs.size();
	cout << endl << "SIZE: " << size << endl;

	if (size - siex != 0) return 1;
	return 0;
}