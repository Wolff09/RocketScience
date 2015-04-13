
#include <queue>
#include <iostream>
#include "explizit/Reachability.hpp"

using namespace std;
using namespace explizit;


ReachabilitySet explizit::computeReachabilitySet(const Configuration& init) {
	ReachabilitySet reachabilitySet;
	queue<Configuration> worklist;

	worklist.push(init);
	while (!worklist.empty()) {
		Configuration& head = worklist.front();
		worklist.pop();

		// config already reached => no need to explore successors again
		auto setinsert = reachabilitySet.insert(move(head));
		bool inserted = setinsert.second;
		if (!inserted) continue;
		const Configuration* config = setinsert.first;

		State* currentState = config->state();
		const VariableAssignment& currentAssignment = config->variableAssignment();
		for (const auto *transition : currentState->transitions()) {
			Guard* guard = transition->guard();

			if (guard->isEnabled(currentAssignment)) {
				// copy old assignment and update all variables affected by actions
				VariableAssignment newAssignment = VariableAssignment(currentAssignment);
				for (const auto *action : transition->actions()) {
					newAssignment.set(action->getVariable(), action->computeResult(currentAssignment));
				}

				worklist.push(Configuration(transition->destination(), newAssignment));
			}
		}
	}

	return move(reachabilitySet);
}