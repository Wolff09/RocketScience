#include "ast/ast.hpp"
#include "ast/trace.hpp"

using namespace ast;



Expr* Call::wp(const Expr& phi) const {
	return phi.pop_ignore();
}

Expr* Return::wp(const Expr& phi) const {
	return phi.push_ignore();
}

Expr* ParallelAssignment::wp(const Expr& phi) const {
	std::map<std::string, const Expr*> repl;
	for (std::size_t i = 0; i < _vars.size(); i++)
		repl[_vars.at(i)->name()] = _exprs.at(i).get();
	return phi.replace(repl);
}

Expr* SimpleAssignment::wp(const Expr& phi) const {
	std::map<std::string, const Expr*> repl;
	repl[_var->name()] = _expr.get();
	return phi.replace(repl);
}

Expr* Assume::wp(const Expr& phi) const {
	// wp(assume(cond), phi) = phi || !cond
	return new BinaryExpression(log_or, phi.copy(), new UnaryExpression(log_not, _expr->copy()));
}

Expr* Assert::wp(const Expr& phi) const {
	// assumption: _cond == false
	assert(dynamic_cast<Literal*>(_expr.get()) != NULL);
	assert(dynamic_cast<Literal*>(_expr.get())->bool_value() == false);
	return { new Literal(false) };
}

Expr* Skip::wp(const Expr& phi) const {
	return phi.copy();
}
