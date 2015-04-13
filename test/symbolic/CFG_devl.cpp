#include <iostream>

#include "symbolic/Reachability.hpp"
// #include "symbolic/CFGFactory.hpp"
#include "symbolic/ControlFlowGraph.hpp"

using namespace std;
using namespace symbolic;


BDD eq(BDD x, BDD y) {
	return (!x + y) * (!y + x);
}

void test() {
	ControlFlowGraph cfg(2,0,1,2,2,0);

	BDD one = cfg.one();
	BDD zero = cfg.zero();
	BDD x = cfg.programVariables()[0];
	BDD xp = cfg.programVariablesPrime()[0];
	BDD y = cfg.programVariables()[1];
	BDD yp = cfg.programVariablesPrime()[1];

	cfg.addTransition(Main(0).block(), Call(0).call(), eq(xp,zero)*eq(yp,zero));
	cfg.addCall(Call(0), Procedure(0));
	cfg.addTransition(Call(0).retrn(), Main(1).block(), eq(xp,x)*eq(yp,y));

	// recursive function
	cfg.addTransition(Procedure(0).entry(), Procedure(0).exit(), x&y&eq(xp,x)*eq(yp,y));
	cfg.addTransition(Procedure(0).entry(), Call(1).call(), !x*!y * eq(xp, one)*eq(yp,y));
	cfg.addTransition(Procedure(0).entry(), Call(1).call(), x*!y * eq(xp,x)*eq(yp,one));
	cfg.addTransition(Procedure(0).entry(), Call(1).call(), !x*y * eq(xp,one)*eq(yp,y));
	cfg.addCall(Call(1), Procedure(0)); // brace for recursion
	cfg.addTransition(Call(1).retrn(), Procedure(0).exit(), eq(xp,x)*eq(yp,y));

	BDD init = cfg.encode(Main(0).block());
	cout << "Initial Config:" << endl;
	init.PrintCover();

	cout << "Computing reachable..." << endl;
	BDD reach = reachable(cfg, init);
	
	cout << "Configurations reaching Main(1):" << endl;
	BDD reach_last = reach * cfg.encode(Main(1).block());
	if (reach_last.IsZero()) cout << "<none>" << endl;
	else reach_last.PrintCover();
}


int main() {
	test();
	cout << endl << endl << "Fin" << endl;
	return 0;
}
