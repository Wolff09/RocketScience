#include <iostream>

#include "symbolic/Reachability.hpp"
#include "symbolic/ControlFlowGraph.hpp"

using namespace std;
using namespace symbolic;


ControlFlowGraph cfg(2,0,1,3,10,10); // main, block, procedures, calls, global, local
BDD one = cfg.one(), zero = cfg.zero();

vector<BDD> n,np,f,fp,x,xp,y,yp;

Node minit = Main(0).block();
Node mfin = Main(1).block();
Procedure FIB = Procedure(0);
Node FIBin = FIB.entry();
Node FIBout = FIB.exit();
Call fm = Call(0);
Node fmc = fm.call();
Node fmr = fm.retrn();
Call f1 = Call(1);
Node f1c = f1.call();
Node f1r = f1.retrn();
Call f2 = Call(2);
Node f2c = f2.call();
Node f2r = f2.retrn();


unsigned int fib(unsigned int x) {
	if (x == 0) return 0;
	else if (x == 1) return 1;
	else return fib(x-1)+fib(x-2);
}

BDD eq(BDD x, BDD y) {
	return (!x + y) * (!y + x);
}

BDD eq(vector<BDD> v, unsigned int i) {
	BDD result = one;
	for (int j = 0; j < v.size(); j++) {
		if (i&1) result *= v[j];
		else result *= !v[j];
		i >>= 1;
	}
	return result;
}

BDD eq(vector<BDD> v1, vector<BDD> v2) {
	BDD result = one;
	for (int i = 0; i < v1.size(); i++)
		result *= eq(v1[i], v2[i]);
	return result;
}


void init_vars() {
	n.push_back(cfg.globalVariables()[0]);
	n.push_back(cfg.globalVariables()[1]);
	n.push_back(cfg.globalVariables()[2]);
	n.push_back(cfg.globalVariables()[3]);
	np.push_back(cfg.globalVariablesPrime()[0]);
	np.push_back(cfg.globalVariablesPrime()[1]);
	np.push_back(cfg.globalVariablesPrime()[2]);
	np.push_back(cfg.globalVariablesPrime()[3]);

	x.push_back(cfg.localVariables()[0]);
	x.push_back(cfg.localVariables()[1]);
	x.push_back(cfg.localVariables()[2]);
	x.push_back(cfg.localVariables()[3]);
	xp.push_back(cfg.localVariablesPrime()[0]);
	xp.push_back(cfg.localVariablesPrime()[1]);
	xp.push_back(cfg.localVariablesPrime()[2]);
	xp.push_back(cfg.localVariablesPrime()[3]);

	f.push_back(cfg.globalVariables()[4]);
	f.push_back(cfg.globalVariables()[5]);
	f.push_back(cfg.globalVariables()[6]);
	f.push_back(cfg.globalVariables()[7]);
	f.push_back(cfg.globalVariables()[8]);
	f.push_back(cfg.globalVariables()[9]);
	fp.push_back(cfg.globalVariablesPrime()[4]);
	fp.push_back(cfg.globalVariablesPrime()[5]);
	fp.push_back(cfg.globalVariablesPrime()[6]);
	fp.push_back(cfg.globalVariablesPrime()[7]);
	fp.push_back(cfg.globalVariablesPrime()[8]);
	fp.push_back(cfg.globalVariablesPrime()[9]);

	y.push_back(cfg.localVariables()[4]);
	y.push_back(cfg.localVariables()[5]);
	y.push_back(cfg.localVariables()[6]);
	y.push_back(cfg.localVariables()[7]);
	y.push_back(cfg.localVariables()[8]);
	y.push_back(cfg.localVariables()[9]);
	yp.push_back(cfg.localVariablesPrime()[4]);
	yp.push_back(cfg.localVariablesPrime()[5]);
	yp.push_back(cfg.localVariablesPrime()[6]);
	yp.push_back(cfg.localVariablesPrime()[7]);
	yp.push_back(cfg.localVariablesPrime()[8]);
	yp.push_back(cfg.localVariablesPrime()[9]);
}

void init_main() {
	cfg.addTransition(minit, fmc, eq(np, n));
	cfg.addTransition(fmr, mfin, eq(np, n)*eq(fp, f));
	cfg.addCall(fm, FIB);
}

void init_FIB() {
	// base cases
	cfg.addTransition(FIBin, FIBout, eq(n, 0)*eq(np, 0)*eq(fp, 0));
	cfg.addTransition(FIBin, FIBout, eq(n, 1)*eq(np, 1)*eq(fp, 1));

	// formula describing: "eq(np, n-1)" aka subtracting 1 from n (inplace in n)
	BDD subN1 = zero;
	for (int i = 1; i < 16; i++)
		subN1 += eq(n, i) * eq(np, i-1);

	// formula describing: "eq(fp, f+y)" aka adding f and y (inplace in f)
	BDD plusFY = zero;
	for (int i = 0; i < 64; i++)
		for (int j = 0; j < 64; j++)
			if (i+j < 64)
				plusFY += eq(f, i) * eq(y, j) * eq(fp, i+j);

	// inductive steps
	cfg.addTransition(FIBin, f1c, !eq(n,0) * !eq(n,1) * eq(xp, n) * subN1);
	cfg.addCall(f1, FIB);
	cfg.addTransition(f1r, f2c, eq(yp, f)*eq(xp, x)*subN1);
	cfg.addCall(f2, FIB);
	cfg.addTransition(f2r, FIBout, eq(np, x) * plusFY);
}

int main() {
	// std::cout << "ssssggggggglllllllssssggggggglllllllggggggg" << std::endl;
	init_vars();
	init_main();
	init_FIB();


	BDD init = cfg.encode(minit) * eq(n, 10);
	cout << "Initial Config:" << endl;
	init.PrintCover();

	cout << "Computing reachable..." << endl;
	BDD reach = reachable(cfg, init);
	
	cout << "Configurations reaching mfin:" << endl;
	BDD reach_last = reach * cfg.encode(mfin);
	if (reach_last.IsZero()) cout << "<none>" << endl;
	else {
		std::cout << "ssssnnnffff--------------------------------" << std::endl;
		reach_last.PrintCover();
		if ((reach_last * eq(n, 10) * eq(f, fib(10))).IsZero()) cout << "FIB FAILED" << endl;
		else cout << "FIB SUCCESSFUL" << endl;
	}


	// for (int i = 0; i < 8; i++) {
	// 	cout << "Running test: fib(" << i << ")==" << fib(i) << "...";
	// 	BDD test_init = cfg.encode(minit) * eq(n, i);
	// 	BDD test_reach = reachable(cfg, test_init);
	// 	BDD test_check = test_reach * cfg.encode(mfin) * eq(n, i) * eq(f, fib(i));
	// 	assert(!test_check.IsZero());
	// 	cout << " successful" << endl;
	// }

	cout << endl << endl << "Fin" << endl;
	return 0;
}
