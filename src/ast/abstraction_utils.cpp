#include "ast/abstraction_utils.hpp"

using namespace ast;


/******************************************************************************
	Z3 Helpers
 ******************************************************************************/


Expr* ast::z3expr2expr(z3::expr expr) {
	assert(expr.is_app());
	std::string op = expr.decl().name().str();

	if (expr.is_const()) {
		if (expr.is_bool()) return new Literal(op == "true");
		else if (!expr.is_numeral()) return new VarName(op);
		else if (expr.is_int()) {
			int val;
			Z3_get_numeral_int(expr.ctx(), expr, &val);
			return new Literal(val);
		}
		else assert(false);
	}

	if (expr.num_args() == 1) {
		auto sub = z3expr2expr(expr.arg(0));
		// TODO: verify operators
		if (op == "not") return new UnaryExpression(log_not, sub);
		else if (op == "-") return new UnaryExpression(ari_neg, sub);
		else assert(false);
	}

	if (expr.num_args() == 2) {
		auto lhs = z3expr2expr(expr.arg(0));
		auto rhs = z3expr2expr(expr.arg(1));
		// TODO: verify operators
		if (op == "or") return new BinaryExpression(log_or, lhs, rhs);
		else if (op == "and") return new BinaryExpression(log_and, lhs, rhs);
		else if (op == "=") return new BinaryExpression(cmp_eq, lhs, rhs);
		else if (op == "!=") return new BinaryExpression(cmp_neq, lhs, rhs);
		else if (op == "<") return new BinaryExpression(cmp_lt, lhs, rhs);
		else if (op == "<=") return new BinaryExpression(cmp_lte, lhs, rhs);
		else if (op == ">") return new BinaryExpression(cmp_gt, lhs, rhs);
		else if (op == ">=") return new BinaryExpression(cmp_gte, lhs, rhs);
		else if (op == "+") return new BinaryExpression(ari_plus, lhs, rhs);
		else if (op == "-") return new BinaryExpression(ari_minus, lhs, rhs);
		else if (op == "*") return new BinaryExpression(ari_mult, lhs, rhs);
		else if (op == "/") return new BinaryExpression(ari_div, lhs, rhs);
		else assert(false);
	}

	if (expr.num_args() > 2) {
		auto astop = log_or;
		if (op == "or") astop = log_or;
		else if (op == "and") astop = log_and;
		else if (op == "+") astop = ari_plus;
		else if (op == "-") astop = ari_minus;
		else if (op == "*") astop = ari_mult;
		else if (op == "/") astop = ari_div;
		else assert(false);
		auto result = z3expr2expr(expr.arg(0));
		for (auto i = 1; i < expr.num_args(); i++)
			result = new BinaryExpression(astop, result, z3expr2expr(expr.arg(i)));
		return result;
	}

	// std::cout << std::endl << "FAIL: " << expr << std::endl; 
	assert(false);
}


bool ast::implies(z3::solver& solver, const z3::expr& lhs, const z3::expr& rhs) {
	return is_taut(solver, !lhs || rhs);
}


bool ast::equals(z3::solver& solver, const z3::expr& lhs, const z3::expr& rhs) {
	return is_taut(solver, (!lhs && !rhs) || (lhs && rhs));
}


bool ast::is_taut(z3::solver& solver, const z3::expr& expr) {
	solver.push();
	solver.add(!expr);
	auto res = solver.check();
	solver.pop();
	return res == z3::unsat;
}


/******************************************************************************
	Helpers for Cube handling
 ******************************************************************************/

struct Cube {
	std::set<std::pair<int, bool>> literals;
	int max;
	z3::expr repr;

	Cube(z3::expr dummy) : repr(dummy) {}
	Cube(int literal_index, bool negated, z3::expr literal) : max(literal_index), repr(negated ? !literal : literal) {
		literals.insert(std::make_pair(literal_index, negated));
	}
	Cube(const Cube& c, int literal_index, bool negated, z3::expr literal) : literals(c.literals), max(literal_index), repr(c.repr) {
		assert(literal_index > c.max); // check if init of max was correct
		repr = repr && (negated ? !literal : literal);
		literals.insert(std::make_pair(literal_index, negated));
	}
};


bool subcube(const Cube& sub, const Cube& super) {
	return std::includes(super.literals.begin(), super.literals.end(), sub.literals.begin(), sub.literals.end());
}


bool no_prime_implicant(const std::vector<Cube>& cube_list, const Cube& prime) {
	// checks whether prime is no prime implicant, i.e. whether it is a subcube of some cube in cube_list
	for (const Cube& c : cube_list)
		if (subcube(c, prime)) // if(c subcube of prime)
			return true;
	return false;
}


Expr* lit2expr(const std::vector<Predicate*>& preds, const std::pair<int, bool>& lit) {
	auto varname = new VarName(preds.at(lit.first)->varname());
	if (!lit.second) return varname;
	else return new UnaryExpression(log_not, varname);
}


Expr* cube2expr(const std::vector<Predicate*>& preds, const Cube& cube) {
	assert(cube.literals.size() > 0);

	// TODO: balance result expression?
	Expr* result = lit2expr(preds, *cube.literals.begin());
	for (auto i = ++cube.literals.begin(); i != cube.literals.end(); i++)
		result = new BinaryExpression(log_and, result, lit2expr(preds, *i));

	return result;
}


Expr* cubes2expr(const std::vector<Predicate*>& preds, const std::vector<Cube>& cube_list) {
	if (cube_list.size() == 0)
		return new Literal(false);
	
	// TODO: balance result expression?
	Expr* result = cube2expr(preds, cube_list.front());
	for (auto it = ++cube_list.begin(); it != cube_list.end(); it++)
		result = new BinaryExpression(log_or, result, cube2expr(preds, *it));

	return result;
}


/******************************************************************************
	Magic
 ******************************************************************************/

Expr* ast::weakest_whatsoever(const std::vector<Predicate*>& preds, z3::expr& phi, z3::solver& solver) {
	std::queue<Cube> work_list; // contains all cubes to explore
	std::vector<Cube> cube_list; // contains all prime implicants of phi

	if (is_taut(solver, phi)) return new Literal(true);
	if (is_taut(solver, !phi)) return new Literal(false);

	for (int i = 0; i < preds.size(); i++) {
		work_list.push(Cube(i, false, preds.at(i)->z3()));
		work_list.push(Cube(i, true, preds.at(i)->z3()));
	}

	// explore all cubes, prune if possible
	while (!work_list.empty()) {
		Cube curr = work_list.front();
		work_list.pop();

		if (no_prime_implicant(cube_list, curr)) continue;
		else if (implies(solver, curr.repr, !phi)) continue;
		else if (implies(solver, curr.repr, phi)) cube_list.push_back(curr);
		else {
			for (int i = curr.max + 1; i < preds.size(); i++) {
				auto lit = preds.at(i)->z3();
				work_list.push(Cube(curr, i, false, lit));
				work_list.push(Cube(curr, i, true, lit));
			}
		}
	}

	auto result = cubes2expr(preds, cube_list);
	return result;
}

Expr* ast::strongest_whatsoever(const std::vector<Predicate*>& preds, z3::expr& phi, z3::solver& solver) {
	auto notphi = !phi;
	auto weakest = weakest_whatsoever(preds, notphi, solver);
	return new UnaryExpression(log_not, weakest);
}
