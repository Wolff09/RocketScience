#pragma once

#include <iostream>
#include "parser/parser.hpp" // TODO: why do we need to include this here?? -> runtime error if removed...


/**
 * @brief Toolkit for model checking.
 */
namespace cegar {


	/**
	 * @brief Performs a CEGAR loop to check C-like integer
	 *        program for assertions errors.
	 * @details The CEGAR loop contains of the following steps:
	 *          1. the input Program is abstracted with the current
	 *             set of predicates (initally empty)
	 *          2. the abstracted Program is translated into a
	 *             ControlFlowGraph and a reachability analysis
	 *             is conducted
	 *          3. if no bad states (assertion errors) are reachable
	 *             the input program is proven correct, otherwise
	 *             a counterexample trace in the abstract program
	 *             is extracted
	 *          4. a Hoare proof is generated to check whether the
	 *             counterexample is present in the input program, too
	 *          5. if the counterexample is not spurious the input
	 *             program is proven incorrect, otherwise
	 *             the set of predicates is refined based on the
	 *             Hoare proof
	 * 
	 * @see ast::Program::abstract
	 * @see ast::Program::cfg
	 * @see ast::extract_trace
	 * @see ast::hoare_proof
	 * @param filename path to a file containing the program to check
	 * @return ```true``` if the program is proven correct,
	 *         ```false``` if the program is proven incorrect.
	 */
	bool prove(std::string filename);


}