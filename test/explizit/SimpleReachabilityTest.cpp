
#include <set>
#include <vector>
#include <iostream>
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
	// TransitionSystem:
	// s1 --[v1/v2=true]--> s2
	// s2 --[v2/v1=false]--> s3
	// s2 --[!v2/v1=false]--> s4
	// s3 --[true/v1=v2,v2=v1]--> s3
	StateTransitionSystem s;
	
	State* s1 = s.makeState("s1");
	State* s2 = s.makeState("s2");
	State* s3 = s.makeState("s3");
	State* s4 = s.makeState("s4");

	Variable* v1 = s.makeVariable("v1");
	Variable* v2 = s.makeVariable("v2");

	Transition* t12 = s1->makeTransition(s2);
	Transition* t23 = s2->makeTransition(s3);
	Transition* t24 = s2->makeTransition(s4);
	Transition* t33 = s3->makeTransition(s3);

	t12->makeGuard(unique_ptr<Formula>(new Literal(v1)));
	t12->makeAction(v2, unique_ptr<Formula>(new TrueFormula()));

	t23->makeGuard(unique_ptr<Formula>(new Literal(v2)));
	t23->makeAction(v1, unique_ptr<Formula>(new FalseFormula()));

	t24->makeGuard(unique_ptr<Formula>(new NotFormula(new Literal(v2))));
	t24->makeAction(v1, unique_ptr<Formula>(new FalseFormula()));

	t33->makeGuard(unique_ptr<Formula>(new TrueFormula()));
	t33->makeAction(v1, unique_ptr<Formula>(new Literal(v2)));
	t33->makeAction(v2, unique_ptr<Formula>(new Literal(v1)));

	VariableAssignment vars = VariableAssignment(2);
	vars.set(v1, true);
	vars.set(v2, false);
	Configuration init = Configuration(s1, vars);

	cout << endl;
	cout << "Starting computeReachabilitySet..." << endl;
	ReachabilitySet rs = computeReachabilitySet(init);

	cout << endl << endl << endl;
	cout << "initial configuration: " << init.state()->name() << "\t[" << init.variableAssignment().evaluate(v1) << ", " << init.variableAssignment().evaluate(v2) << "]";
	cout << endl <<"Size of ReachabilitySet: " << rs.size() << endl;
	for (const auto *elem : rs.configs()) {
		cout << elem->state()->name();
		cout << "\t[";
		cout << elem->variableAssignment().evaluate(v1);
		cout << ", ";
		cout << elem->variableAssignment().evaluate(v2);
		cout << "]" << endl;
	}
	cout << endl;

    return 0;
}