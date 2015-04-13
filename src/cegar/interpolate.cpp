#include "cegar/interpolate.hpp"

#include "ast/abstraction_utils.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <z3++.h>

using namespace cegar;


std::pair<bool, z3::expr> cegar::interpolate(z3::context& ctx, const z3::expr minus, const z3::expr plus) {
	// z3::solver bla(ctx);
	// bla.push();
	// auto hae = minus && plus;
	// std::cout<<"testing: "<<hae<<std::endl;
	// bla.add(hae);
	// auto res = bla.check();
	// bla.pop();
	// assert(res == z3::unsat);

	Z3_ast patParts[] = {Z3_mk_interpolant(ctx, minus), plus};
	// Z3_ast patParts[] = {A, Z3_mk_interpolant(ctx, B)};
	Z3_ast pat = Z3_mk_and(ctx, 2, patParts);

	Z3_params params = 0;
	Z3_ast_vector interpolants;
	Z3_model model = 0;
	Z3_lbool status;

	status = Z3_compute_interpolant(ctx, pat, params, &interpolants, &model);
	
	if (status == Z3_L_FALSE) {
		assert(Z3_ast_vector_size(ctx, interpolants) == 1);
		Z3_ast ast = Z3_ast_vector_get(ctx, interpolants, 0);
		z3::expr interpolant(ctx, ast);

		// test interpolante
		z3::solver solver(ctx);
		assert(ast::implies(solver, minus, interpolant));
		assert(!ast::is_taut(solver, interpolant&&plus));

		return std::make_pair(true, interpolant);
	} else {
		if (model) {
			std::cout << "MODEL FOUND: " << std::endl << Z3_model_to_string(ctx, model) << std::endl;
			Z3_del_model(ctx, model);
		}
		assert(false);
		return std::make_pair(false, z3::expr(ctx));
	}
}

std::vector<ast::Expr*> cegar::compute_interpolants(const ast::Program& prog, const std::vector<const ast::TraceableStatement*>& trace, const std::vector<ast::Expr*>& constraints) {
	// constraints might contain variables with the same name which however stem from different function declarations
	// -> properly translate variables to prefixed (with scope name) z3 variables and undo this in the end

	// TODO: programs must not have duplicate variable declarations -> oryl?
	z3::context ctx;

	std::vector<z3::expr> z3c;
	for (const ast::Expr* e : constraints)
		z3c.push_back(e->z3(ctx));

	std::vector<ast::Expr*> interpolants;
	std::stack<std::size_t> funbodies;
	funbodies.push(0);
	for (std::size_t i = 0; i < z3c.size() - 1; i++) {
		// auto stmt = trace.at(i);
		// if (dynamic_cast<const ast::Call*>(stmt)) {
		//	funbodies.push(i+1);
		//	continue; // TODO: correct?
		// } else if (dynamic_cast<const ast::Return*>(stmt)) {
		//	funbodies.pop();
		//	continue; // TODO: correct?
		// }
		// auto stack_pointer = funbodies.top();
		auto stack_pointer = 0;
		// minus = contraints[0] && ... && contraints[i]
		z3::expr minus = z3c.at(stack_pointer);
		for (std::size_t j = stack_pointer+1; j <= i; j++) minus = minus && z3c.at(j);
		// plus  = contraints[i+1] && ... && contraints[constraints.size()-1]
		z3::expr plus = z3c.at(i+1);
		for (std::size_t j = i+2; j <= z3c.size()-1; j++) plus = plus && z3c.at(j);
		// make interpolant
		auto res = interpolate(ctx, minus, plus);
		assert(res.first);
		interpolants.push_back(ast::z3expr2expr(res.second));
	}

	return interpolants;
}
