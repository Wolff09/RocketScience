#include "ast/ast.hpp"

using namespace ast;


Expr* sub(const std::map<const VarDef*, std::size_t>& lvmap, const Expr& e) {
	// TODO: this is definetly not performant
	std::map<std::string, const Expr*> repl;
	for (const auto& e : lvmap)
		repl[e.first->name()] = new SymbolicConstant(e.first, e.second);
	auto result = e.replace(repl);
	for (const auto& e : repl)
		delete e.second;
	return result;
}


Expr* Call::con(std::map<const VarDef*, std::size_t>& lvalmap) const {
	// locals löschen bzw. auf komische konstanten setzten?
	// assert(false);
	return new Literal(true);
}

Expr* Return::con(std::map<const VarDef*, std::size_t>& lvalmap) const {
	// assert(false);
	return new Literal(true);
}

Expr* Assume::con(std::map<const VarDef*, std::size_t>& lvalmap) const {
	return sub(lvalmap, *_expr);
}

Expr* Assert::con(std::map<const VarDef*, std::size_t>& lvalmap) const {
	// TODO: what to do here?
	return new Literal(true);
}

Expr* SimpleAssignment::con(std::map<const VarDef*, std::size_t>& lvalmap) const {
	auto rhs = sub(lvalmap, *_expr);
	assert(_var->decl() != NULL);
	lvalmap[_var->decl()]++;
	auto lhs = sub(lvalmap, *_var);
	return new BinaryExpression(cmp_eq, lhs, rhs);
}

Expr* ParallelAssignment::con(std::map<const VarDef*, std::size_t>& lvalmap) const {
	std::vector<Expr*> rhss, lhss;
	for (const auto& e : _exprs) rhss.push_back(sub(lvalmap, *e));
	for (const auto& v : _vars) assert(v->decl() != NULL);
	for (const auto& v : _vars) lvalmap[v->decl()]++;
	for (const auto& v : _vars) lhss.push_back(sub(lvalmap, *v));
	Expr* result = new BinaryExpression(cmp_eq, lhss.at(0), rhss.at(0));
	for	(std::size_t i = 1; i < rhss.size(); i++) {
	   	auto cmp = new BinaryExpression(cmp_eq, lhss.at(i), rhss.at(i));
	   	result = new BinaryExpression(log_and, result, cmp);
	}
	return result;
}

Expr* Skip::con(std::map<const VarDef*, std::size_t>& lvalmap) const {
	return new Literal(true);
}
