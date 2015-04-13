#include "ast/ast.hpp"

using namespace ast;


Conditional* Conditional::copy() const {
	return new Conditional(_cond->copy(), _if->copy(), _else->copy());
}

UnaryExpression* UnaryExpression::copy() const {
	return new UnaryExpression(_op, _child->copy());
}

BinaryExpression* BinaryExpression::copy() const {
	return new BinaryExpression(_op, _left->copy(), _right->copy());
}

Literal* Literal::copy() const {
	return new Literal(_type, _value);
}

VarName* VarName::copy() const {
	return new VarName(this);
}

Unknown* Unknown::copy() const {
	return new Unknown();
}

SymbolicConstant* SymbolicConstant::copy() const {
	return new SymbolicConstant(_decl, _num);
}

