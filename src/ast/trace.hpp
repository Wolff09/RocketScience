#pragma once

#include "ast/ast.hpp"


namespace ast {
	// TODO: rework doc


	/**
	 * @brief Computes some trace of program statements that are executed while
	 *        following a path from ```src``` to ```dst``` in the given ```cfg```.
	 * @details Computes a shortest path from ```src``` to ```dst``` in the given ```cfg```.
	 *          The search space is bound by the given ```bounds```.
	 *          This path is then mapped to statements of the given (abstract) program.
	 * 
	 * @see Program::cfg
	 * @see Program::abstract
	 * @see Statement#_cfg_pre
	 * @param abstract a boolean program, e.g. an abstraction of a integer program
	 * @param cfg the ```ControlFlowGraph``` corresponding with the ```abstract``` program
	 * @param src source node in ```cfg```
	 * @param dst destination node in ```cfg```
	 * @param bounds a set of configurations to consider when searching for a path thru the ```cfg```
	 * @param ignored_edges a set of edges in the given ```cfg``` which are not considered when searching for a path from ```src``` to ```dst```
	 * @return a trace of statements that are executed on a path thru the ```cfg```;
	 *         also provides the ```pre``` and ```post``` configuration of every statement encoded as BDD
	 */
	std::vector<std::pair<const TraceableStatement*, std::pair<BDD, BDD>>> extract_trace(const Program& abstract, const symbolic::ControlFlowGraph& cfg, const BDD src, const BDD dst, const BDD bounds, const BDD ignored_edges);


	/**
	 * @brief Shorthand for ```extract_trace(abstract, cfg, src, dst, bounds, cfg.zero())```
	 * @details Assumes that no edges are to be ignored for the trace.
	 * @see ast::extract_trace
	 */
	static std::vector<std::pair<const TraceableStatement*, std::pair<BDD, BDD>>> extract_trace(const Program& abstract, const symbolic::ControlFlowGraph& cfg, const BDD src, const BDD dst, const BDD bounds) {
		return extract_trace(abstract, cfg, src, dst, bounds, cfg.zero());
	}


	
	std::vector<const TraceableStatement*> flat_trace(const Program& abstract, const symbolic::ControlFlowGraph& cfg, const BDD src, const BDD dst, const BDD bounds, const BDD ignored_edges);
	

	/**
	 * @brief Shorthand for ```flat_trace(abstract, cfg, src, dst, bounds, cfg.zero())```
	 * @details Assumes that no edges are to be ignored for the trace.
	 * @see ast::flat_trace
	 */
	static std::vector<const TraceableStatement*> flat_trace(const Program& abstract, const symbolic::ControlFlowGraph& cfg, const BDD src, const BDD dst, const BDD bounds) {
		return flat_trace(abstract, cfg, src, dst, bounds, cfg.zero());
	}

}