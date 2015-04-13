#pragma once

#include "ast/ast.hpp"



namespace cegar {

	std::pair<bool, z3::expr> interpolate(z3::context& ctx, const z3::expr minus, const z3::expr plus);

	std::vector<ast::Expr*> compute_interpolants(const ast::Program& prog, const std::vector<const ast::TraceableStatement*>& trace, const std::vector<ast::Expr*>& constraints);
	
	// static void intertest() {
	// z3::context ctx;
	// std::map<std::string, z3::expr> var2z3;
	// var2z3.insert(std::make_pair("a2", ctx.int_const("a2")));
	// var2z3.insert(std::make_pair("a3", ctx.int_const("a3")));
	// var2z3.insert(std::make_pair("b0", ctx.int_const("b0")));
	// var2z3.insert(std::make_pair("c1", ctx.int_const("c1")));

	// z3::expr z3minus = minus.z3(ctx, var2z3);
	// z3::expr z3plus = plus.z3(ctx, var2z3);

	//	auto l1 = new ast::BinaryExpression(ast::cmp_gt, new ast::VarName("b0"), new ast::Literal(0));
	//	auto l2 = new ast::BinaryExpression(ast::cmp_eq, new ast::VarName("c1"), new ast::BinaryExpression(ast::ari_mult, new ast::Literal(2), new ast::VarName("b0")));
	//	auto l3 = new ast::BinaryExpression(ast::cmp_eq, new ast::VarName("a2"), new ast::VarName("b0"));
	//	auto l4 = new ast::BinaryExpression(ast::cmp_eq, new ast::VarName("a3"), new ast::BinaryExpression(ast::ari_minus, new ast::VarName("a2"), new ast::Literal(1)));
	  	
	//	auto l5 = new ast::BinaryExpression(ast::cmp_lt, new ast::VarName("a3"), new ast::VarName("b0"));
	//	auto l6 = new ast::BinaryExpression(ast::cmp_eq, new ast::VarName("a3"), new ast::VarName("c1"));

	//	auto apre = new ast::BinaryExpression(ast::log_and, l1, l2);
	//	apre = new ast::BinaryExpression(ast::log_and, apre, l3);
	//	auto a = ast::BinaryExpression(ast::log_and, apre, l4);
	//	auto b = ast::BinaryExpression(ast::log_and, l5, l6);
	//	auto res = interpolate(a, b);
	//	if (res) delete res;
	// }

}
