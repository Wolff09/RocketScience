
#pragma once

#include <vector>
#include <string>
#include <memory>
#include "explizit/Common.hpp"
#include "explizit/Formula.hpp"

/*
 * Available classes in here:
 *   1. StateTransitionSystem
 *   2. State
 *   3. Transition
 *   4. Guard
 *   5. Action
 */


namespace explizit {


	// forward declarations due to circular dependencies
	class StateTransitionSystem;
	class State;
	class Transition;
	class Guard;
	class Action;

 	class StateTransitionSystem {
		private:
			std::vector<std::unique_ptr<State>> _states;
			std::vector<std::unique_ptr<Variable>> _variables;
			std::vector<State*> _statePointers;
			std::vector<Variable*> _variablePointers;

		public:
			const std::vector<State*>& states() const;
			const std::vector<Variable*>& variables() const;
			// TODO: delete copy constructor?

			State* makeState(const std::string& name);
			Variable* makeVariable(const std::string& name);
	};

	class State {
		private:
			const std::string _name;
			std::vector<std::unique_ptr<Transition>> _transitions;
			std::vector<Transition*> _transitionPointers;

		public:
			State(const std::string& name);

			const std::string& name() const;
			const std::vector<Transition*>& transitions() const;
			Transition* makeTransition(State* dst);
	};

	class Transition {
		private:
			State* _dst;
			std::unique_ptr<Guard> _guard;
			std::vector<std::unique_ptr<Action>> _actions;
			std::vector<Action*> _actionPointers;

		public:
			Transition(State* destination);

			State* destination() const;
			Guard* guard() const;
			const std::vector<Action*> actions() const;

			Guard* makeGuard(std::unique_ptr<Formula> guard);
			Action* makeAction(Variable* variable, std::unique_ptr<Formula> action);
	};

	class Guard {
		private:
			std::unique_ptr<Formula> _formula;

		public:
			Guard(std::unique_ptr<Formula> guard);

			bool isEnabled(const VariableAssignment& /*TODO:const?*/ assignment) const;
	};

	class Action {
		private:
			Variable* _assignedVariable;
			std::unique_ptr<Formula> _formula;

		public:
			Action(Variable* assignedVariable, std::unique_ptr<Formula> action);
			
			Variable* getVariable() const;
			bool computeResult(const VariableAssignment& /*TODO:const?*/ assignment) const;
	};

}
