/** @file
 * @brief Symbolic StateTransitionSystem for modeling boolean while programs.
 */

#pragma once

#include <iostream>
#include <cmath>
#include <vector>
#include <cassert>
#include "cuddObj.hh"


/**
 * @brief Basic stuff for symbolic model checking boolean while programs.
 */
namespace symbolic {


	/**
	 * @brief Symbolic representation of a state transition system for modeling boolean while programs.
	 * @details
	 * # StateTransitionSystem
	 * A StateTransitionSystem is a symbolic representation of boolean while program encoded as state
	 * transition system. To encode such a boolean while program we need states and program variables.
	 * Both are encoded via boolean variables in the StateTransitionSystem. The number of variables
	 * held by a StateTransitionSystem is fixed, so you need to specify how many states and program
	 * variables you need upon creation.
	 * 
	 * ### Variables
	 * A StateTransitionSystem holds variables for encoding states and program variables. Beyond this
	 * classification, there are two types of variables (independent of states and program variables):
	 * *current* and *next* variables. These two types are used to model a configuration transition
	 * of system.
	 * 
	 * Each variable comes with a unique immutable id (aka. label) which can be accessed. This id is
	 * simultaneously the index of the variable in the vector you can access via variables().
	 * 
	 * In fact, each variable is internally followed by its next counterpart and the vector returned
	 * by variables() does contain variables for encoding states at the beginning and variables for
	 * the program variables at the end. However, one should not rely on this ordering. There are
	 * member methods for accessing states and program variables, for checking the type of variable
	 * and for converting *current* variables to *next* variables and vice versa.
	 * 
	 * Note that the index of a variable does not necessarily correspond to its position in the
	 * variable ordering!
	 * 
	 * Furthermore, be aware of the terminology. There are *variables* and *program variables*.
	 * The later one come from a program and are annotated to transitions. The former are BDD
	 * variables of the internal symbolic representation. The type (as in type system) of a variable
	 * is ```BDD```. I.e. a variable is a BDD with exactly one node containing a variable.
	 * Furthermore, both variables have an index. *program variable* have an continuous index
	 * between ```0``` and ```number_of_variables``` identify it uniquely. The internal
	 * *variables* do also have an index identifying them uniquely, however, there are no
	 * guarantees on the shape/range of this index.
	 * 
	 * 
	 * ### Transitions
	 * Transitions are annotated with guards and actions, which are also represented symbolically.
	 * The guard formula states whether transition is enabled.
	 * The action formula describes the actions upon variables, i.e. how variables change their
	 * values by taking a transition. An action is supposed to only use program variables.
	 * Actions are usually of the following form:
	 * 
	 * 	x1' = !x1 & x2, x2' = x2 | x3
	 * 
	 * Note, that one can not directly feed this formula into the StateTransitionSystem since
	 * 	1. The '=' operator is not supported by Cudd. You have to circumvent this by expanding
	 * 	   the definition of equivalence, i.e.
	 * 	   ``` a <-> b |=| (!a | b) & (!b | a) ```
	 * 	2. Multiple actions are combined via conjunction.
	 * 
	 * So the above actions, for example, would look like the following:
	 * 
	 * 	(!x1' | (!x1 & x2) & (x1' | !(!x1 & x2)) & (!x2' & (x2 | x3)) & (x2' | & !(x2 | x3))
	 * 
	 * Note: The actions are *explicit*. Id est, variables that do not covered by an action
	 * are interpreted as "don't care". Thus you have to provide actions like ```x'=x``` for
	 * all variables which should not change.
	 * 
	 * ### BDDs
	 * All the symbolic representations are stored as BDDs. The used BDD library is Cudd.
	 */
	class StateTransitionSystem {
		private:
			const Cudd _manager;

			const std::size_t _numStates;
			const std::size_t _numVariables;
			const std::size_t _numStateVariables;

			BDD _transitionRelation;
			BDD _protoCurrState;
			BDD _protoNextState;

			std::vector<BDD> _vars;

		public:
			/**
			 * @brief Constructs a new StateTransitionSystem.
			 * @param numStates the number of states that should be supported
			 * @param numVars the number of variables that should be supported
			 */
			StateTransitionSystem(const std::size_t numStates, const std::size_t numVars) : _numStates(numStates), _numVariables(numVars), _manager(Cudd(2*(numVars+numStates), 0)), _numStateVariables(2*ceil(log2(numStates))) {
				for (size_t i = 0; i < _numStateVariables + 2*numVars; i += 2) {
					_vars.push_back(_manager.bddVar(i));
					_vars.push_back(_manager.bddVar(i+1));
				}

				assert(_vars.size() == _numStateVariables + 2*numVars);

				_protoCurrState = one();
				_protoNextState = one();
				for (size_t i = 0; i < _numStateVariables; i += 2) {
					_protoCurrState *= !_vars[i];
					_protoNextState *= !_vars[i+1];
				}

				_transitionRelation = zero();
			}

			/**
			 * @brief Adds a transition to the system.
			 * @details Transitions are 4-tuples containing source and destination states as well as
			 * a guard and actions. Both, guard and actions, are given symbolically as boolean functions
			 * implemented by a BDD.
			 * 
			 * The guard should only use *current* variables corresponding to *program variables*. This is,
			 * however, a soft constraint and is never checked.
			 * 
			 * The actions must be combined by conjunction. The Actions should also only contain variables
			 * corresponding to *program variables*. Again, this is a soft constraint and is never checked.
			 * So it might be possible to change variables used for encoding states with an action. The
			 * result is unpredictable.
			 * 
			 * Note: The actions are *explicit*. Id est, variables that do not covered by an action
			 * are interpreted as "don't care". Thus you have to provide actions like ```x'=x``` for
			 * all variables which should not change.
			 * 
			 * @param src index of the source state (outgoing)
			 * @param dst index of the destination state (ingoing)
			 * @param guard BDD implementing the guard function
			 * @param action BDD implementing the action function
			 */
			void addTransition(std::size_t src, std::size_t dst, BDD guard, BDD action) {
				assert(0 <= src && src < _numStates);
				assert(0 <= dst && dst < _numStates);
				_transitionRelation += stateCurrent(src) * guard * stateNext(dst) * action;
			}


			/**
			 * @brief Gives a BDD implementing the transition relation of the StateTransitionSystem.
			 * @details The transition relation contains all needed information of the StateTransitionSystem.
			 * The connection between states are implement just as the guards and actions annotated to
			 * the transitions.
			 * @return BDD implementing the transition relation with guards and actions
			 */
			const BDD& transitionRelation() const { return _transitionRelation; }


			/**
			 * @brief Gives the number of states that are supported by this StateTransitionSystem.
			 * @return maximal number of states
			 */
			std::size_t numberOfStates() const { return _numStates; }

			/**
			 * @brief Gives the number of variables that are supported by this StateTransitionSystem.
			 * @return maximal number of variables
			 */
			std::size_t numberOfVariables() const { return _numVariables; }


			/**
			 * @brief Returns a BDD representing the static 1 function.
			 * @return BDD consisting solely of the terminal 1 node
			 */
			BDD one() const { return _manager.bddOne(); }

			/**
			 * @brief Returns a BDD representing the static 0 function.
			 * @return BDD consisting solely of the terminal 0 node
			 */
			BDD zero() const { return _manager.bddZero(); }


			/**
			 * @brief Computes the index of the topmost variable of the given BDD.
			 * @param b a BDD to compute the label of
			 * @return variable index of the topmost node of the given BDD
			 */
			unsigned int label(BDD b) const { return b.NodeReadIndex(); }


			/**
			 * @brief Grants access to all variables contained in the StateTransitionSystem.
			 * @details The returned vector contains the variables for encoding states at the
			 * beginning and the variables for *program variables* at the end of the vector.
			 * The *current* and *next* variables are interleaved: each *current* variable is
			 * followed directly by its corresponding *next* variable.
			 * @return immutable vector containing all variables
			 */
			const std::vector<BDD> &variables() const { return _vars; }


			/**
			 * @brief Tests whether a given variable is a *current* variable.
			 * @throws AssertionError if there is no variable with the given index
			 * @param index the index (label) of a variable
			 * @return true iff. the given index belongs to a *current* variable
			 */
			bool isCurrentVariable(unsigned int index) const {
				assert(0 <= index && index < _vars.size());
				return index % 2 == 0;
			}

			/**
			 * @brief Tests whether a given variable is a *current* variable.
			 * @details This method considers the label of the given BDD only,
			 * i.e. it tests whether the topmost variable of the given BDD is
			 * a *current* variable.
			 * 
			 * This method is a shorthand for writing
			 * ```isCurrentVariable(label(b))```.
			 * 
			 * @see isCurrentVariable()
			 * @throws AssertionError if the given BDD does not contain any variables
			 * @param b a BDD whose topmost variable should be checked
			 * @return true iff. the topmost variable of the given BDD is a *current* variable
			 */
			bool isCurrentVariable(BDD b) const { return isCurrentVariable(label(b)); };

			/**
			 * @brief Tests whether a given variable is a *next* variable.
			 * @details This method is a shorthand for writing
			 * ```!isCurrentVariable(index)```.
			 * 
			 * @see isCurrentVariable()
			 * @param index the index (label) of a variable
			 * @return true iff. the given index belongs to a *next* variable
			 */
			bool isNextVariable(unsigned int index) const { return !isCurrentVariable(index); }

			/**
			 * @brief Tests whether a given variable is a *next* variable.
			 * @details This method considers the topmost variable of the given BDD only.
			 * It is a shorthand for writing ```!isCurrentVariable(b)```.
			 * 
			 * @see isCurrentVariable()
			 * @throws AssertionError if the given BDD does not contain any variables
			 * @param b a BDD whose topmost variable should be checked
			 * @return true iff. the topmost variable of the given BDD is a *next* variable
			 */
			bool isNextVariable(BDD b) const { return !isCurrentVariable(b); }

			/**
			 * @brief Tests whether a given variable is used for encoding states.
			 * @details The result does not imply anything about the variable beeing a
			 * *next* or a *current* variable.
			 * 
			 * @throws AssertionError if there is no variable with the given index
			 * @param index the index (label) of a variable
			 * @return true iff. the given index belongs to a variable used to encoding states
			 */
			bool isStateVariable(unsigned int index) const {
				assert(0 <= index && index < _vars.size());
				return index < _numStateVariables;
			}

			/**
			 * @brief Tests whether a given variable is used for encoding states.
			 * @details This method checks whether the topmost variable of a given BDD
			 * is used for encoding states. It is a shorthand for writing
			 * ```isStateVariable(label(b))```
			 * 
			 * @see isStateVariable()
			 * @throws AssertionError if the given DD does not contain any variables
			 * @param b a BDD whose topmost variable should be checked
			 * @return true iff. the topmost variable of the given BDD is used for encoding states
			 */
			bool isStateVariable(BDD b) const { return isStateVariable(label(b)); }

			/**
			 * @brief Tests whether a given variable represents a program variable.
			 * @details The result does not imply anything about the variable being a
			 * *next* or a *current* variable.
			 * 
			 * Furthermore, this method is a shorthand for writing
			 * ```!isStateVariable(index)```.
			 * 
			 * @see isStateVariable()
			 * @param index the index (label) of a variable
			 * @return true iff. the given index belongs to a variable representing a program variable
			 */
			bool isProgramVariable(unsigned int index) const { return !isStateVariable(index); }

			/**
			 * @brief Tests whether a given variable represents a program variable.
			 * @details This method checks whether the topmost variable of a given BDD
			 * represents a program variable. It is a shorthand for writing
			 * ```!isStateVariable(b)```
			 * 
			 * @see  isStateVariable()
			 * @throws AssertionError if the given DD does not contain any variables
			 * @param b a BDD whose topmost variable should be checked
			 * @return true iff. the topmost variable of the given BDD represents a program variable
			 */
			bool isProgramVariable(BDD b) const { return !isStateVariable(b); }

			/**
			 * @brief Returns the corresponding *next* counterpart of a given variable.
			 * @details If the given index refers to a *next* variable, simply the variable with the
			 * given index is returned.
			 * 
			 * @throws AssertionError if there is no variable with the given index
			 * @param index the index (label) of a variable
			 * @return a BDD containing exactly one variable, namely the *next* version of the
			 * variable with the given index
			 */
			BDD nextOf(unsigned int index) const {
				assert(0 <= index && index < _vars.size());
				if (isCurrentVariable(index)) return _vars[index+1];
				else return _vars[index];
			}

			/**
			 * @brief Returns the corresponding *next* counterpart of a given variable.
			 * @details This method considers the topmost variable of the given BDD only.
			 * It is a shorthand for writing ```nextOf(label(b))```.
			 * 
			 * @see nextOf()
			 * @throws AssertionError if the given DD does not contain any variables
			 * @param b a BDD where the *next* version of topmost variable should be computed
			 * @return a BDD containing exactly one variable, namely the *next* version of the
			 * topmost variable of the given BDD
			 */
			BDD nextOf(BDD b) const { return nextOf(label(b)); }

			/**
			 * @brief Returns the corresponding *current* counterpart of a given variable.
			 * @details If the given index refers to a *current* variable, simply the variable with the
			 * given index is returned.
			 * 
			 * @throws AssertionError if there is no variable with the given index
			 * @param index the index (label) of a variable
			 * @return a BDD containing exactly one variable, namely the *current* version of the
			 * variable with the given index
			 */
			BDD currentOf(unsigned int index) const {
				assert(0 <= index && index < _vars.size());
				if (isNextVariable(index)) return _vars[index-1];
				else return _vars[index];
			}

			/**
			 * @brief Returns the corresponding *current* counterpart of a given variable.
			 * @details This method considers the topmost variable of the given BDD only.
			 * It is a shorthand for writing ```currentOf(label(b))```.
			 * 
			 * @see currentOf()
			 * @throws AssertionError if the given DD does not contain any variables
			 * @param b a BDD where the *current* version of topmost variable should be computed
			 * @return a BDD containing exactly one variable, namely the *current* version of the
			 * topmost variable of the given BDD
			 */
			BDD currentOf(BDD b) const { return currentOf(label(b)); }


			/**
			 * @brief Returns the *current* variable which corresponds to the given program variable.
			 * @details The given index is the index of a *program variable*, i.e. it must be in the
			 * range between ```0``` and ```numberOfVariables()```.
			 * 
			 * @see numberOfVariables()
			 * @throws AssertionError if the given index is out of bounds
			 * @param index the index of a *program variable*
			 * @return a BDD containing exactly one variable, namely the *current* variable corresponding
			 * to the given *program variable*
			 */
			BDD programVariableCurrent(size_t index) const {
				assert(0 <= index && index < _numVariables);
				return _vars[_numStateVariables + 2*index];
			}

			/**
			 * @brief Returns the *next* variable which corresponds to the given program variable.
			 * @details The given index is the index of a *program variable*, i.e. it must be in the
			 * range between ```0``` and ```numberOfVariables()```.
			 * 
			 * @see numberOfVariables()
			 * @throws AssertionError if the given index is out of bounds
			 * @param index the index of a *program variable*
			 * @return a BDD containing exactly one variable, namely the *next* variable corresponding
			 * to the given *program variable*
			 */
			BDD programVariableNext(size_t index) const {
				assert(0 <= index && index < _numVariables);
				return _vars[_numStateVariables + 2*index + 1];
			}

			/**
			 * @brief Computes a characteristic formula for a state with *current* variables.
			 * @details The given index is the index of a state, i.e. it must be in the range
			 * between ```0``` and ```numberOfStates()```.
			 * 
			 * @see numberOfStates()
			 * @throws AssertionError if the given index is out of bounds
			 * @param index the index of a state
			 * @return a BDD implementing the characteristic formula of the given state, using only
			 * *current* variables
			 */
			BDD stateCurrent(size_t index) const {
				assert(index >= 0 && index < _numStates);
				BDD state = _protoCurrState;
				for (size_t pos = 0; index; pos += 2) {
					if (index & 1) state = state.Compose(!_vars[pos], pos);
					index >>= 1;
				}
				return state;
			}

			/**
			 * @brief Computes a characteristic formula for a state with *next* variables.
			 * @details The given index is the index of a state, i.e. it must be in the range
			 * between ```0``` and ```numberOfStates()```.
			 * 
			 * @see numberOfStates()
			 * @throws AssertionError if the given index is out of bounds
			 * @param index the index of a state
			 * @return a BDD implementing the characteristic formula of the given state, using only
			 * *next* variables
			 */
			BDD stateNext(size_t index) const {
				assert(index >= 0 && index < _numStates);
				BDD state = _protoNextState;
				for (size_t pos = 1; index; pos += 2) {
					if (index & 1) state = state.Compose(!_vars[pos], pos);
					index >>= 1;
				}
				return state;
			}
	};


}
