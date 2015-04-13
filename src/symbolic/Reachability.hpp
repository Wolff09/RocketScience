/** @file
 * @brief Reachability Engine for symbolic StateTransitionSystem s and ControlFlowGraph s.
 */

#pragma once

#include <vector>
#include <cassert>
#include <utility>
#include "cuddObj.hh"
#include "symbolic/StateTransitionSystem.hpp"
#include "symbolic/ControlFlowGraph.hpp"


namespace symbolic {

	/**
	 * @brief Computes all reachable configuration from a given initial one.
	 * @details Given a StateTransitionSystem and an initial configuration,
	 * this method does a reachability analysis.
	 * 
	 * A configuration is a tuple (state, variable_assignment).
	 * Configurations are also encoded symbolically by a formula of the
	 * following shape:
	 * 
	 *	characteristicFormula(state) & x_1=val(x_1) & ... & x_n=val(x_n)
	 * 
	 * The computation itself is a fixed point computing successor
	 * consecutively. The successors of a configuration can be computed
	 * by the following:
	 * 
	 *	(exists x_1...x_n: transition & current_configs)[x_i'\x_i]
	 *	
	 * A more verbose description of the above:
	 *	1. compute the conjunction of the transition relation with
	 *	   the currently reachable configurations
	 *		  - the transition relation is symbolically encoded containing
	 *		    transitions with guards and actions
	 *		  - the current reachable configurations is a disjunction of
	 *		    symbolically encoded reachable configurations
	 *	2. compute the existential abstraction quantifying over all
	 *	   *current* variable
	 *	3. replacing all *next* variables with their *current* counterpart
	 *	4. union the computed successors with the currently reachable
	 *	   configurations via disjunction (towards fixed point iteration)
	 * 
	 * The successor are computed via the BDD algorithms
	 *	(1) Apply (AND)
	 *	(2) Exists
	 *	(3) Compose
	 *	(4) Apply (OR).
	 * There is a more efficient way to do this in a single "run" over the BDDs
	 * by using the ```RelProdS``` algorithm [1]. However, this algorithm is not
	 * implemented in the Cudd BDD library and implementing it on top of the
	 * C++ wrapper is less efficient than applying the four algorithms mentioned
	 * above.
	 * 
	 * To get a performance boost one might implement the ```RelProdS```
	 * algorithm directly at Cudd C library bypassing the unique table [1].
	 * This is considered as future work.
	 * 
	 * [1] http://essay.utwente.nl/61650/1/thesis_Tom_van_Dijk.pdf
	 *     especially chapter 3
	 * 
	 * @param sts a StateTransitionSystem
	 * @param reachable an initial configuration
	 * @return a BDD implementing the characteristic formula of the reachable
	 *         configurations
	 */
	BDD reachable(const StateTransitionSystem& sts, BDD reachable);



	/**
	 * @brief Computes reachable configurations in ```ControlFlowGraph```s.
	 *
	 * @details Configurations are given as BDDs that describe both a state and
	 * the contents of all variables. Note that changes of local variables are not
	 * inside of procedure calls.
	 *
	 * The result is a BDD that described all configurations (states and variables)
	 * Than can be reached from the configuration space described by the BDD init.
	 *
	 * This algorithm supports procedures, meaning that the control flow graph May
	 * contain procedure calls. Whenever the reachability engine discovers new reachable
	 * calls of procedures, a relation between input and output variables will be
	 * computed. This relation is used to insert new edges that bypass procedure calls.
	 * These edges will emulate the behavior of a certain procedure call.
	 * 
	 * // TODO: change documentation -> returned BDD contains all explored configs (reach + sum)
	 *
	 * @param cfg the ControlFlowGraph
	 * @param init the initial configuration
	 * @param init_call_frame flag determining wheter locla variables are initialized
	 *        to ```false```
	 * @return a BDD implementing the characteristic formula of the reachable
	 *         configurations
	 */
	BDD reachable(ControlFlowGraph& cfg, const BDD init, const BDD bad, const bool init_call_frame=false);



	/**
	 * @brief Computes a path from ```src``` to ```dst``` within the bounds of ```reach```
	 * @details The used algorithm first performs a backward search starting in ```dst```.
	 *          It explores all states in ```reach``` which can reach ```dst``` in exactly
	 *          ```k``` steps where ```k``` is the length of a shortest path from ```src```
	 *          to ```dst```.
	 *          Then, a forward search thru the ```k```-step reachable states is performed.
	 *          This search generates a single path. That is, the resulting BDDs do not
	 *          contain "don't cares" variables among BDD-variables representing the state
	 *          and the current valuation.
	 *          
	 *          TODO: revisit documentation
	 * 
	 * @see ControlFlowGraph
	 * @see ControlFlowGraph::stateVariables()
	 * @see ControlFlowGraph::programVariables()
	 * @param cfg The ControlFlowGraph to be searched in
	 * @param src Set of ```cfg``` states.
	 * @param dst Set of ```cfg``` states.
	 * @param reach Set of reachable states which are used as bound. That is, only states
	 *              contained in this set are explored.
	 * @param ignored_edges Set of edges that will be ignored during path construction
	 * @return A path encoded in BDDs which won't contain "don't cares"
	 */
	std::vector<BDD> find_path(const ControlFlowGraph& cfg, const BDD src, const BDD dst, const BDD reach, const BDD ignored_edges);


}
