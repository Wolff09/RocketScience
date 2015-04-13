#include "ast/ast.hpp"

using namespace ast;


Expr* Conditional::replace(const std::map<std::string, const Expr*>& repl) const {
	return new Conditional(_cond->copy(), _if->replace(repl), _else->replace(repl));
}

Expr* UnaryExpression::replace(const std::map<std::string, const Expr*>& repl) const {
	return new UnaryExpression(_op, _child->replace(repl));
}

Expr* BinaryExpression::replace(const std::map<std::string, const Expr*>& repl) const {
	return new BinaryExpression(_op, _left->replace(repl), _right->replace(repl));
}

Expr* Literal::replace(const std::map<std::string, const Expr*>& repl) const {
	return copy();
}

Expr* VarName::replace(const std::map<std::string, const Expr*>& repl) const {
	if (_ignore_replace == 0 && repl.count(name()) > 0) return repl.at(name())->copy();
	else return copy();
}

Expr* Unknown::replace(const std::map<std::string, const Expr*>& repl) const {
	return copy();
}

Expr* SymbolicConstant::replace(const std::map<std::string, const Expr*>& repl) const {
	assert(false);
	return copy();
}
