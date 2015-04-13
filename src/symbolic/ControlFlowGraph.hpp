/** @file
 * @brief This is the basic infrastructure for describing ControlFlowGraph s.
 * 
 * @details This file collects basic tools describing ControlFlowGraph s.
 * This includes several structs that describes nodes and procedures
 * for dealing with vectors of BDDs (such as concat and multiply )
 * and for generating frequently used BDDs (such as imply and equal ).
 */

#pragma once


#include <iostream>
#include <cmath>
#include <vector>
#include <cassert>
#include "cuddObj.hh"


namespace symbolic {


	/**
	 * @brief Collection of different Block types contained in
	 * ```ControlFlowGraph```s
	 */
	enum StateType { MAIN, BLOCK, CALL, RETURN, ENTRY, EXIT };

	/**
	 * @brief Immutable class capturing a node of a control flow graph.
	 * @details This class is an abstract, more verbose notation for
	 * dealing with nodes of control flow graphs which have an id and
	 * a type. It is also independent of an actual '''ControlFlowGraph'''
	 * instance.
	 */
	struct Node {
		const StateType type;
		const std::size_t id;
		Node(StateType type, std::size_t id) : type(type), id(id) {}
		bool is(StateType t) const { return type == t; }
		std::string tostr() const;
		inline bool operator==(const Node& rhs) const { return type == rhs.type && id == rhs.id; }
		inline bool operator!=(const Node& rhs) const { return !(*this == rhs); }
		inline bool operator<(const Node& rhs) const { return type == rhs.type ? id < rhs.id : type < rhs.type; }
	};

	template<StateType type>
	struct BaseBlock {
		const std::size_t id;
		BaseBlock(std::size_t id) : id(id) {}
		Node block() const { return Node(type, id); }
	};

	typedef BaseBlock<BLOCK> Block;
	typedef BaseBlock<MAIN> Main;

	struct Call {
		const std::size_t id;
		Call(std::size_t id) : id(id) {}
		Node call() const { return Node(CALL, id); }
		Node retrn() const { return Node(RETURN, id); }
	};

	struct Procedure {
		const std::size_t id;
		Procedure(std::size_t id) : id(id) {};
		Node entry() const { return Node(ENTRY, id); }
		Node exit() const { return Node(EXIT, id); }
	};


	/**
	 * @brief Concatenates the given vectors.
	 * @details This function is a shortcut for inserting the contents
	 * of each given vector into a new one.
	 * 
	 * The order of the given vectors is preserved in the output.
	 * 
	 * @param vectors list of vectors to concatenate
	 * @return new vector containing 
	 */
	static std::vector<BDD> concat(std::initializer_list<const std::vector<BDD>> vectors) {
		std::vector<BDD> ret;
		for (const auto &vec : vectors)
			ret.insert(ret.end(), vec.begin(), vec.end());
		return ret;
	}

	/**
	 * @brief Computes a BDD representing the conjuntion of an arbitrary number of BDDs.
	 * 
	 * @param init A base BDD, i.e. the BDD representing the constant ```one``` function.
	 *             *Note that this parameter is needed for the BDD elements coming from the
	 *             same manager and it is highly likly that you are not able to touch the
	 *             actual manager of the BDDs you are dealing with.*
	 * @param vectors A list of vectors that are conjuncted
	 * 
	 * @return the conjunction of all passed BDDs
	 */
	static BDD multiply(BDD init, std::initializer_list<std::vector<BDD>> vectors) {
		for (const auto &vec : vectors)
			for (const auto &e : vec) init *= e;
		return init;
	}

	/**
	 * @brief Implements the lazy boolean implication function.
	 * 
	 * @return ```l -> r```
	 */
	static bool imply(bool l, bool r) {
		return !l || r;
	}

	/**
	 * @brief Computes a BDD representing the equality function for the input BDDs.
	 * 
	 * @return ```l <-> r```
	 */
	static BDD equal(BDD l, BDD r) {
		return (!l + r) * (!r + l);
	}


	/**
     * @brief class for generating and manipulating control flow graphs.
     *
     * @details This class is used by the reachability engine to 
     * read from the control flow graph and to manipulate it
     * (i.e. adding summary edges).
     * Guards, actions and variables are modeled as BDDs.
     * Procedures are supported. Their parameters and return values
     * should be encoded in global variables.
     *  
     *
	 * Variable Layout:
	 * 1. state variables
	 * 2. program variables
	 * 3. primed variables
	 * 4. primed program variables
	 * 5. doubly primed program variables ("memory" for summary relation)
	 */
	class ControlFlowGraph {
		private:
			const std::size_t _numNodeVariables;
			const std::size_t _numMainBlocks;
			const std::size_t _numBlocks;
			const std::size_t _numProcedures;
			const std::size_t _numCalls;
			const std::size_t _numGlobVars;
			const std::size_t _numTempVars;
			const std::size_t _numPVars;

			const std::size_t _offsetPrime;
			const std::size_t _offsetRel;
			
			const Cudd _mgr;

			BDD _trans;
			BDD _calls;
			BDD _exits;
			BDD _returns;

			const std::vector<BDD> _vars;
			const std::vector<BDD> _stateVars;
			const std::vector<BDD> _stateVarsPrimed;
			const std::vector<BDD> _programVars;
			const std::vector<BDD> _programVarsPrimed;
			const std::vector<BDD> _globalVars;
			const std::vector<BDD> _globalVarsPrimed;
			const std::vector<BDD> _globalVarsRel;
			const std::vector<BDD> _localVars;
			const std::vector<BDD> _localVarsPrimed;
			
			BDD _stateProto;
			BDD _stateProtoPrimed;

			static bool checkTransitionConstraints(StateType src, StateType dst);
			static std::vector<BDD> init_vars(const Cudd& mgr, size_t numVars);
			static std::vector<BDD> init_varSubset(std::vector<BDD> vars, std::size_t begin, std::size_t range);

			std::size_t offsetOf(StateType type) const;
			std::size_t indexOf(Node node) const;
			BDD encode(Node node, bool primed) const;

		public:
			ControlFlowGraph(std::size_t numMainBlocks, std::size_t numBlocks, std::size_t numProcedures, std::size_t numCalls, std::size_t numGlobalVariables, std::size_t numLocalVariables);

			BDD one() const { return _mgr.bddOne(); }
			BDD zero() const { return _mgr.bddZero(); }
			BDD transitionRelation() const { return _trans; }
			BDD callRelation() const { return _calls; }

			void addTransition(Node src, Node dst, BDD guardedaction);
			void addCall(Call call, Procedure proc);

			const std::vector<BDD>& variables() const { return _vars; }
			const std::vector<BDD>& stateVariables() const { return _stateVars; }
			const std::vector<BDD>& stateVariablesPrime() const { return _stateVarsPrimed; }
			const std::vector<BDD>& programVariables() const { return _programVars; }
			const std::vector<BDD>& programVariablesPrime() const { return _programVarsPrimed; }
			const std::vector<BDD>& globalVariables() const { return _globalVars; }
			const std::vector<BDD>& globalVariablesPrime() const { return _globalVarsPrimed; }
			const std::vector<BDD>& globalVariablesRel() const { return _globalVarsRel; }
			const std::vector<BDD>& localVariables() const { return _localVars; }
			const std::vector<BDD>& localVariablesPrime() const { return _localVarsPrimed; }

			std::size_t number_of_mains() const { return _numMainBlocks; }
			std::size_t number_of_blocks() const { return _numBlocks; }
			std::size_t number_of_procedures() const { return _numProcedures; }
			std::size_t number_of_calls() const { return _numCalls; }

			BDD encode(Node node) const { return encode(node, false); }
			symbolic::Node decode(BDD state) const;


			friend BDD reachable(ControlFlowGraph& cfg, BDD init, BDD bad, bool init_call_frame);
	};


}