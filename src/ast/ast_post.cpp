#include "ast/ast.hpp"

#include <boost/algorithm/string.hpp>

using namespace ast;


Expr* Conditional::postprocess_interpolant(const Program& prog) const {
	auto c = _cond->postprocess_interpolant(prog);
	auto i = _if->postprocess_interpolant(prog);
	auto e = _else->postprocess_interpolant(prog);
	return new Conditional(c, i, e);
}

Expr* UnaryExpression::postprocess_interpolant(const Program& prog) const {
	auto c = _child->postprocess_interpolant(prog);
	return new UnaryExpression(_op, c);
}

Expr* BinaryExpression::postprocess_interpolant(const Program& prog) const {
	auto l = _left->postprocess_interpolant(prog);
	auto r = _right->postprocess_interpolant(prog);
	return new BinaryExpression(_op, l, r);
}

Expr* Literal::postprocess_interpolant(const Program& prog) const {
	return copy();
}

Expr* VarName::postprocess_interpolant(const Program& prog) const {
	assert(std::count(_value.begin(), _value.end(), '%') == 1);
	assert(std::count(_value.begin(), _value.end(), '$') <= 1);
	
	std::vector<std::string> split;
	boost::split(split, _value, boost::is_any_of("%$"));
	assert(split.size() >= 2);

	std::string scope = split[0];
	std::string name = split[1];

	const VarDef* decl;
	if (scope == "global") decl = prog.name2var().at(name);
	else decl = prog.name2fun().at(scope)->name2var().at(name);
	
	return new VarName(decl);
}

Expr* Unknown::postprocess_interpolant(const Program& prog) const {
	return copy();
}

Expr* SymbolicConstant::postprocess_interpolant(const Program& prog) const {
	assert(false);
}