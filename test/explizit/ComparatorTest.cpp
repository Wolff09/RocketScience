
#include <set>
#include <vector>//
#include <iostream>
#include "explizit/Common.hpp"
#include "explizit/Reachability.hpp"
#include "explizit/Configuration.hpp"
#include "explizit/StateTransitionSystem.hpp"

using namespace std;
using namespace explizit;

class Test {
	private:
		ReachabilitySet _set;

	public:
		bool add(State* state, VariableAssignment assignment, bool expectedSizeIncreased) {
			const Configuration& config = Configuration(state, assignment);
			return add(config, expectedSizeIncreased);
		}
		bool add(const Configuration& c, bool expectedSizeIncreased) {
			// return true if the test failed, i.e. the expectation was not met
			size_t oldSize = _set.size();
			_set.insert(c);
			size_t newSize = _set.size();
			bool sizeIncreased = oldSize != newSize;
			return sizeIncreased != expectedSizeIncreased;
		}

};

int main(int argc, char **argv) {
	StateTransitionSystem sts;
	State* s1 = sts.makeState("s1");
	State* s2 = sts.makeState("s2");
	State* s3 = sts.makeState("s3");
	Variable* v1 = sts.makeVariable("v1");
	Variable* v2 = sts.makeVariable("v2");
	Variable* v3 = sts.makeVariable("v3");


	VariableAssignment a000(3);
	a000.set(v1, false);
	a000.set(v2, false);
	a000.set(v3, false);

	VariableAssignment a001(a000);
	a001.set(v1, true);
	VariableAssignment a010(a000);
	a010.set(v2, true);
	VariableAssignment a100(a000);
	a100.set(v3, true);

	VariableAssignment a011(a001);
	a011.set(v2, true);
	VariableAssignment a110(a100);
	a110.set(v2, true);
	VariableAssignment a101(a100);
	a101.set(v1, true);

	VariableAssignment a111(a110);
	a111.set(v1, true);

	Test test;
	bool failed = false;
	failed = failed || test.add(s1, a000, true);
	failed = failed || test.add(s1, a000, false);
	failed = failed || test.add(s1, a001, true);
	failed = failed || test.add(s2, a000, true);
	failed = failed || test.add(s3, a001, true);
	failed = failed || test.add(s2, a001, true);
	failed = failed || test.add(s3, a001, false);
	failed = failed || test.add(s3, a010, true);
	failed = failed || test.add(s3, a100, true);
	failed = failed || test.add(s2, a100, true);
	failed = failed || test.add(s1, a100, true);
	failed = failed || test.add(s2, a010, true);

    return failed ? 1 : 0;
}