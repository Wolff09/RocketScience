
#include <set>
#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>
#include "explizit/StateTransitionSystemLoader.hpp"
#include "explizit/Common.hpp"
#include "explizit/Formula.hpp"
#include "explizit/Configuration.hpp"
#include "explizit/StateTransitionSystem.hpp"
#include "explizit/SimpleCSVReader.hpp"
#include "explizit/StateTransitionSystemLoader.hpp"
#include "explizit/Reachability.hpp"

using namespace std;
using namespace explizit;

int main(int argc, char **argv) {
	assert(argc == 2);
	string filename(argv[1]);
	ifstream stream(filename.c_str());
	StateTransitionSystemLoader csvloader(stream);
	std::unique_ptr<explizit::StateTransitionSystem> progtrans = csvloader.make();


	VariableAssignment vars = VariableAssignment(progtrans->variables().size());
	for (auto *variable : progtrans->variables()) {
		vars.set(variable, false);
	}
	// whitebox-Test: Assume that the initial state is the first one that is considered by StateTransitionSystemLoader
	Configuration initconf = Configuration(progtrans->states()[0], vars);
	ReachabilitySet reach = computeReachabilitySet(initconf);

	for (auto *conf : reach.configs()){
		cout << endl << conf->state()->name();
		cout << "(";
		for (auto *variable : progtrans->variables()) {
			cout << conf->variableAssignment().evaluate(variable);
		}
		cout << ")" << endl;
	}

	return 0; // TODO: do some real testing
}
