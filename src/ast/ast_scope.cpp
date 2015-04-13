#include "ast/ast.hpp"

using namespace ast;


/* scope() */

const FunDef* Conditional::scope() const {
	auto rc = _cond->scope();
	if (rc) return rc;
	auto ri = _if->scope();
	if (ri) return ri;
	return _else->scope();
}

const FunDef* UnaryExpression::scope() const {
	return _child->scope();
}

const FunDef* BinaryExpression::scope() const {
	auto r = _left->scope();
	if (r) return r;
	return _right->scope();
}

const FunDef* Literal::scope() const {
	return NULL;
}

const FunDef* VarName::scope() const {
	return _decl->function();
}

const FunDef* Unknown::scope() const {
	return NULL;
}

const FunDef* SymbolicConstant::scope() const {
	assert(false);
	return _decl->function();
}


/* is_well_scoped() */

bool Conditional::is_well_scoped() const {
	if (!_cond->is_well_scoped()) return false;
	if (!_if->is_well_scoped()) return false;
	if (!_else->is_well_scoped()) return false;
	auto cs = _cond->scope();
	auto is = _if->scope();
	auto es = _else->scope();
	if (cs == NULL) return is == NULL || es == NULL || is == es;
	else return (cs == is || is == NULL) && (cs == es || es == NULL);
}

bool UnaryExpression::is_well_scoped() const {
	return _child->is_well_scoped();
}

bool BinaryExpression::is_well_scoped() const {
	if (!_left->is_well_scoped()) return false;
	if (!_right->is_well_scoped()) return false;
	auto ls = _left->scope();
	auto rs = _right->scope();
	return ls == NULL || rs == NULL || ls == rs;
}

bool Literal::is_well_scoped() const {
	return true;
}

bool VarName::is_well_scoped() const {
	return true;
}

bool Unknown::is_well_scoped() const {
	assert(false);
	return true;
}

bool SymbolicConstant::is_well_scoped() const {
	assert(false);
	return true;
}
