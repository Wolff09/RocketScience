#include "ast/ast.hpp"

using namespace ast;


bool Conditional::contains(std::string varname) const {
	return _cond->contains(varname) || _if->contains(varname) || _else->contains(varname);
}

bool UnaryExpression::contains(std::string varname) const {
	return _child->contains(varname);
}

bool BinaryExpression::contains(std::string varname) const {
	return _left->contains(varname) || _right->contains(varname);
}

bool Literal::contains(std::string varname) const {
	return false;
}

bool VarName::contains(std::string varname) const {
	return varname == _value;
}

bool Unknown::contains(std::string varname) const {
	return false;
}

bool SymbolicConstant::contains(std::string varname) const {
	assert(false);
	return false;
}




bool Conditional::contains_any_var() const {
	return _cond->contains_any_var() || _if->contains_any_var() || _else->contains_any_var();
}

bool UnaryExpression::contains_any_var() const {
	return _child->contains_any_var();
}

bool BinaryExpression::contains_any_var() const {
	return _left->contains_any_var() || _right->contains_any_var();
}

bool Literal::contains_any_var() const {
	return false;
}

bool VarName::contains_any_var() const {
	return true;
}

bool Unknown::contains_any_var() const {
	return false;
}

bool SymbolicConstant::contains_any_var() const {
	assert(false);
	return false;
}




bool Conditional::contains_ignored_var() const {
	return _cond->contains_ignored_var() || _if->contains_ignored_var() || _else->contains_ignored_var();
}

bool UnaryExpression::contains_ignored_var() const {
	return _child->contains_ignored_var();
}

bool BinaryExpression::contains_ignored_var() const {
	return _left->contains_ignored_var() || _right->contains_ignored_var();
}

bool Literal::contains_ignored_var() const {
	return false;
}

bool VarName::contains_ignored_var() const {
	return _ignore_replace != 0;
}

bool Unknown::contains_ignored_var() const {
	return false;
}

bool SymbolicConstant::contains_ignored_var() const {
	assert(false);
	return false;
}