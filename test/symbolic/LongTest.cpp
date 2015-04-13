#include <iostream>
#include <string>

#include "symbolic/StateTransitionSystem.hpp"
#include "symbolic/Reachability.hpp"

using namespace std;
using namespace symbolic;

BDD eq(BDD x, BDD y) {
	return (!x + y) * (!y + x);
}


void debug(string s, bool lb) {
	if (lb) cout << s << endl;
	else cout << s;
	cout << flush;
}
void debug(string s) { debug(s, true); }

int main(int argc, char **argv)
{
	assert(argc == 2);

	size_t numStates = stoi(argv[1]);
	size_t numVars = 35;

	debug("Creating StateTransitionSystem with " + to_string(numStates) + " states and " + to_string(numVars) + " variables...");

	StateTransitionSystem sts = StateTransitionSystem(numStates,numVars);
	debug("StateTransitionSystem created");

	BDD one = sts.one();
	BDD zero = sts.zero();

	BDD init = sts.stateCurrent(0);
	for (size_t i = 0; i < numVars; i++) init *= eq(sts.programVariableCurrent(i), zero);
	debug("Initial Configuration created");

	debug("Creating action...", false);
	BDD action = one;
	for (size_t i = 0; i < numVars; i++) {
		debug(" " + to_string(i), false);
		action *= eq(sts.programVariableCurrent(i), i < numVars/2 ? sts.programVariableNext(i) : zero);
	}
	debug("\nCreating guard...");
	BDD guard = one;
	for (size_t i = 0; i < numVars; i++) guard *= eq(sts.programVariableCurrent(i), zero);
	debug("Creating Transitions...", false);
	for (size_t i=0; i < numStates-1; i++) {
		sts.addTransition(i, i+1, guard, action);
		if (i%50==0) debug(" " + to_string(i), false);
	}
	debug("\nAll Transitions (" + to_string(numStates) + ") created");

	debug("Starting Reachability Analysis");
	BDD r = reachable(move(sts), move(init)); 
	debug("Finished Reachability Analysis");
	// r.PrintCover();

	return 0;
}
