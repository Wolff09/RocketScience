#include <iostream>

#include "symbolic/StateTransitionSystem.hpp"
#include "symbolic/Reachability.hpp"

using namespace std;
using namespace symbolic;

BDD eq(BDD x, BDD y) {
	return (!x + y) * (!y + x);
}

int main()
{
	StateTransitionSystem sts = StateTransitionSystem(5,1);
	BDD x = sts.programVariableCurrent(0);
	BDD xp = sts.programVariableNext(0);
	BDD one = sts.one();
	BDD zero = sts.zero();
	BDD g1 = eq(x, one);
	BDD a1 = eq(xp, one);
	BDD a2 = eq(xp, zero);
	BDD a3 = eq(xp, x);
	BDD a4 = eq(xp, zero);
	BDD a5 = eq(xp, x);
	sts.addTransition(0, 1, one, a1);
	sts.addTransition(1, 2, zero, a2);
	sts.addTransition(1, 3, g1, a3);
	sts.addTransition(2, 3, one, a4);
	sts.addTransition(3, 4, one, a5);

	cout << "Number of Variables held by StateTransitionSystem: " << sts.variables().size() << endl << endl;

	cout << "Transitions:" << endl;
	sts.transitionRelation().PrintCover();
	/*
	011100 -0
	010000 -1
	110100 11
	101001 11
	101001 00
	*/

	BDD init = sts.stateCurrent(0) * eq(x, zero);
	cout << "Initial Configuratioin:" << endl;
	init.PrintCover();
	/*
	0-0-0- 0-
	*/

	BDD r = reachable(sts, init);
	cout << "Reachable Stuff:" << endl;
	r.PrintCover();
	/*
	1--- 0-  s=1,3, x=1
	0-0- 1-  s=4, x=1
	0-0- 0-  s=0, x=0
	*/

	return 0;
}
