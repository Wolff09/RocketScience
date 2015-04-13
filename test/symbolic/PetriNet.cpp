#include <iostream>
#include <vector>
#include "symbolic/StateTransitionSystem.hpp"
#include "symbolic/Reachability.hpp"

using namespace std;
using namespace symbolic;


vector<BDD> xp, xq, np, nq;
int loops;


BDD eq(BDD x, BDD y) {
	return (!x + y) * (!y + x);
}

BDD id_all_but(int i, int j, BDD id) {
	for (int k = 0; k < loops; k++) {
		if (k == i || k == j) continue;
		id *= eq(np[k], xp[k]) * eq(nq[k], xq[k]);
	}
	return id;
}


int main(int argc, char **argv)
{
	assert(argc == 2);
	loops = stoi(argv[1]);
	assert(loops >= 1);
	
	StateTransitionSystem sts = StateTransitionSystem(1, 2*loops);
	BDD one = sts.one();
	BDD zero = sts.zero();

	for (int i = 0; i < loops; i++) {
		xp.push_back(sts.programVariableCurrent(i));
		xq.push_back(sts.programVariableCurrent(loops+i));
		np.push_back(sts.programVariableNext(i));
		nq.push_back(sts.programVariableNext(loops+i));
	}

	// first petri transitions
	sts.addTransition(0, 0,
		xq[0],
		eq(np[0], one) * eq(nq[0], zero) * id_all_but(0, 0, one)
	);

	// inner petri transitions
	for (int i = 1; i < loops; i++)
		sts.addTransition(0, 0,
			xp[i-1] * xq[i],
			eq(np[i-1], zero) * eq(nq[i], zero) * eq(np[i], one) * eq(nq[i-1], one) * id_all_but(i-1, i, one)
		);

	// last petri transitions
	sts.addTransition(0, 0,
		xp[loops-1],
		eq(np[loops-1], zero) * eq(nq[loops-1], one) * id_all_but(loops-1, loops-1, one)
	);


	BDD init = sts.stateCurrent(0);
	for (int i = 0; i < loops; i++) init *= eq(xp[i], one) * eq(xq[i], zero);

	cout << "Computing Reachability..." << endl << flush;
	BDD r = reachable(sts, init);

	BDD w = sts.stateCurrent(0);
	for (int i = 0; i < loops; i++) init *= eq(xp[i], zero) * eq(xq[i], one);

	cout << endl << "Reachability: ";
	if ((r * !w).IsZero()) cout << "Gut :)" << endl;
	else cout << "Kaputt :(" << endl;
	cout << endl;


	xp.clear();
	np.clear();
	xq.clear();
	nq.clear();
	return 0;
}
