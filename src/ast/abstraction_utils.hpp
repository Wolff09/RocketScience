#pragma once

#include <set>
#include <queue>
#include "ast/ast.hpp"


namespace ast {


	/**
	 * @brief Traverses the Z3 representation of an expression an converts it to an internal representation.
	 * @param expr the expression to convert
	 * @return semantically equal internal representation
	 */
	Expr* z3expr2expr(z3::expr expr);


	/**
	 * @brief Checks whether the formula ```lhs``` implies ```rhs``` logically.
	 * @param solver a solver with adequate context
	 * @param lhs left-hand-side of the implication
	 * @param rhs right-hand-side of the implication
	 * @return ```true``` iff. ```|= lhs -> rhs```.
	 */
	bool implies(z3::solver& solver, const z3::expr& lhs, const z3::expr& rhs);


	/**
	 * @brief Checks whether the formula ```lhs``` equals ```rhs``` logically.
	 * @param solver a solver with adequate context
	 * @param lhs left-hand-side of the equality
	 * @param rhs right-hand-side of the equality
	 * @return ```true``` iff. ```|= lhs <-> rhs```.
	 */
	bool equals(z3::solver& solver, const z3::expr& lhs, const z3::expr& rhs);

	/**
	 * @brief Checks whether the given formula is a tautology
	 * @param solver solver with adequate context
	 * @param expr the expression to check
	 * @return ```true``` iff. ```|= expr```
	 */
	bool is_taut(z3::solver& solver, const z3::expr& expr);


	/**
	 * @brief Computes the largest disjunction ```c``` over the given Predicates ```preds``` such that ```c``` implies ```phi```.
	 * @details This is based on the predicate abstraction described by Thomas Ball (Microsoft Research)
	 *          in "Automatic Predicate Abstraction of C Programs" (2001)
	 * 
	 * @see http://dl.acm.org/citation.cfm?id=378846
	 * @see http://www.cs.ucla.edu/~todd/research/pldi01.pdf
	 * @see Program::abstract
	 * @param preds Predicates used for the predicate abstraction
	 * @param phi some formula, usually some kind of weakest precondition
	 * @param solver solver with adequate context
	 * @return an expression ```res``` with ```res |= phi``` and ```f.a. res': (res' -> phi) -> (res -> res')```
	 */
	Expr* weakest_whatsoever(const std::vector<Predicate*>& preds, z3::expr& phi, z3::solver& solver);

	/**
	 * @brief Shortcut for ```!weakest_whatsoever(preds, !phi, solver)```
	 * @details This is based on the predicate abstraction described by Thomas Ball (Microsoft Research)
	 *          in "Automatic Predicate Abstraction of C Programs" (2001)
	 * 
	 * @see ast::weakest_whatsoever
	 * @see http://dl.acm.org/citation.cfm?id=378846
	 * @see http://www.cs.ucla.edu/~todd/research/pldi01.pdf
	 * @see Program::abstract
	 * @param preds Predicates used for the predicate abstraction
	 * @param phi some formula, usually some kind of weakest precondition
	 * @param solver solver with adequate context
	 * @return ```!weakest_whatsoever(preds, !phi, solver)```
	 */
	Expr* strongest_whatsoever(const std::vector<Predicate*>& preds, z3::expr& phi, z3::solver& solver);


}
