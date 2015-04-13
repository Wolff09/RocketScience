#include "ast/ast.hpp"

using namespace ast;



Expr* Conditional::push_ignore() const {
	return new Conditional(_cond->push_ignore(), _if->push_ignore(), _else->push_ignore());
}

Expr* UnaryExpression::push_ignore() const {
	return new UnaryExpression(_op, _child->push_ignore());
}

Expr* BinaryExpression::push_ignore() const {
	return new BinaryExpression(_op, _left->push_ignore(), _right->push_ignore());
}

Expr* Literal::push_ignore() const {
	return copy();
}

Expr* VarName::push_ignore() const {
	VarName* result = copy();
	if (_decl->is_local())
		result->_ignore_replace++;
	return result;
}

Expr* Unknown::push_ignore() const {
	return copy();
}

Expr* SymbolicConstant::push_ignore() const {
	assert(false);
	return copy();
}



Expr* Conditional::pop_ignore() const {
	return new Conditional(_cond->pop_ignore(), _if->pop_ignore(), _else->pop_ignore());
}

Expr* UnaryExpression::pop_ignore() const {
	return new UnaryExpression(_op, _child->pop_ignore());
}

Expr* BinaryExpression::pop_ignore() const {
	return new BinaryExpression(_op, _left->pop_ignore(), _right->pop_ignore());
}

Expr* Literal::pop_ignore() const {
	return new Literal(_type, _value);
}

Expr* VarName::pop_ignore() const {
	VarName* result = new VarName(this);
	if (_decl->is_local()) {
		assert(_ignore_replace > 0);
		result->_ignore_replace--;
	}
	return result;
}

Expr* Unknown::pop_ignore() const {
	return new Unknown();
}

Expr* SymbolicConstant::pop_ignore() const {
	assert(false);
	return copy();
}
