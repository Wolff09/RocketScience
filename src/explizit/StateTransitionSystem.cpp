
#include "explizit/StateTransitionSystem.hpp"

using namespace std;
using namespace explizit;


// StateTransitionSystem

const vector<State*>& StateTransitionSystem::states() const {
	return _statePointers;
}

const vector<Variable*>& StateTransitionSystem::variables() const {
	return _variablePointers;
}

State* StateTransitionSystem::makeState(const string& name) {
	State* newState = new State(name);
	_states.push_back(unique_ptr<State>(newState));
	_statePointers.push_back(newState);
	return newState;
}

Variable* StateTransitionSystem::makeVariable(const string& name) {
	Variable* newVariable = new Variable(name, _variables.size());
	_variables.push_back(unique_ptr<Variable>(newVariable));
	_variablePointers.push_back(newVariable);
	return newVariable;
}


// State
State::State(const string& name) : _name(name) {}

const string& State::name() const {
	return _name;
}

const vector<Transition*>& State::transitions() const {
	return _transitionPointers;
}

Transition* State::makeTransition(State* dst) {
	Transition* newTransition = new Transition(dst);
	_transitions.push_back(unique_ptr<Transition>(newTransition));
	_transitionPointers.push_back(newTransition);
	return newTransition;
}


// Transition

Transition::Transition(State* destination) : _dst(destination) {}

State* Transition::destination() const {
	return _dst;
}

Guard* Transition::guard() const {
	return _guard.get();
}

const vector<Action*> Transition::actions() const {
	return _actionPointers;
}

Guard* Transition::makeGuard(unique_ptr<Formula> formula) {
	Guard* newGuard = new Guard(move(formula));
	_guard.reset(newGuard);
	return newGuard;
}

Action* Transition::makeAction(Variable* assignedVariable, unique_ptr<Formula> formula) {
	Action* newAction = new Action(assignedVariable, move(formula));
	_actions.push_back(unique_ptr<Action>(newAction));
	_actionPointers.push_back(newAction);
	return newAction;
}


// Guard

Guard::Guard(unique_ptr<Formula> guard) {
	_formula = move(guard);
}

bool Guard::isEnabled(const VariableAssignment& assignment) const {
	bool isEnabled = _formula->evaluate(move(assignment));
	return isEnabled;
}


// Action

Action::Action(Variable* assignedVariable, unique_ptr<Formula> action) {
	_assignedVariable = assignedVariable;
	_formula = move(action);
}

Variable* Action::getVariable() const {
	return _assignedVariable;
}

bool Action::computeResult(const VariableAssignment& assignment) const {
	return _formula->evaluate(assignment);
}
