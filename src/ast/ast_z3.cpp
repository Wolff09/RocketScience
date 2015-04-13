#include "ast/ast.hpp"
#include "ast/abstraction_utils.hpp"

using namespace ast;


/**** Expressions ****/

z3::expr Conditional::z3(z3::context& context) const {
	return z3::to_expr(context, Z3_mk_ite(context, _cond->z3(context), _if->z3(context), _else->z3(context)));
}

z3::expr UnaryExpression::z3(z3::context& context) const {
	if (_op == log_not) return !_child->z3(context);
	else if (_op == ari_neg) return -_child->z3(context);
	else assert(false);
}

z3::expr BinaryExpression::z3(z3::context& context) const {
	auto lhs = _left->z3(context);
	auto rhs = _right->z3(context);
	// TODO: check whether we used proper operator overloads
	if (_op == ari_plus) return lhs + rhs;
	else if (_op == ari_minus) return lhs - rhs;
	else if (_op == ari_mult) return lhs * rhs;
	else if (_op == ari_div) return lhs / rhs;
	else if (_op == log_or) return lhs || rhs;
	else if (_op == log_and) return lhs && rhs;
	else if (_op == cmp_lt) return lhs < rhs;
	else if (_op == cmp_lte) return lhs <= rhs;
	else if (_op == cmp_gt) return lhs > rhs;
	else if (_op == cmp_gte) return lhs >= rhs;
	else if (_op == cmp_eq) return lhs == rhs;
	else if (_op == cmp_neq) return lhs != rhs;
	else assert(false);
}

z3::expr Literal::z3(z3::context& context) const {
	if (_type == bool_t) return context.bool_val(_value == "true" ? true : false);
	else if (_type == int_t) return context.int_val(std::stoi(_value));
	else assert(false);
}

z3::expr VarName::z3(z3::context& context) const {
	assert(_decl != NULL);
	auto name = _value.c_str();
	auto type = _decl->type();
	if (type == bool_t) return context.bool_const(name);
	else if (type == int_t) return context.int_const(name);
	else assert(false);
}

z3::expr Unknown::z3(z3::context& context) const {
	throw UnsupportedOperationError("Value of type 'unknown' cannot be converted to a Z3 expression.");
}

z3::expr SymbolicConstant::z3(z3::context& context) const {
	// std::string postfix = "$" + std::to_string(_num);
	// return mk_z3var(context, _var->decl(), "", postfix);
	// TODO: this breaks in case of boolean variables
	assert(_decl != NULL);
	auto fun = _decl->function();
	auto prefix = fun == NULL ? "global" : fun->name();
	auto varname = _decl->name();
	auto postfix = std::to_string(_num);
	auto name = prefix + "%" + varname + "$" + postfix;
	auto type = _decl->type();
	if (type == bool_t) return context.bool_const(name.c_str());
	else if (type == int_t) return context.int_const(name.c_str());
	else assert(false);
	// return context.int_const(name.c_str());
}

/**** Predicates ****/

const z3::expr& Predicate::z3() const {
	assert(_z3repr != NULL);
	return *_z3repr.get();
}

void Predicate::precompute_z3(z3::context& context) {
	_z3repr.reset(new z3::expr(_expr->z3(context)));
}

void PredicateList::clear_z3() const {
	for (auto& p : _ownership)
		p->clear_z3();
}

void Predicate::clear_z3() {
	_z3repr.reset();
}

