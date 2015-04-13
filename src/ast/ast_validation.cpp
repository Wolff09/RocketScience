#include "ast/ast.hpp"

using namespace ast;

#define VERBOSE false


/**** Declarations ****/

void Program::validate() {
	if (_name2var.size() != _vars.size())
		throw ValidationError("Duplicate global variable declaration.");
	if (_name2fun.size() != _funs.size())
		throw ValidationError("Duplicate function declaration.");
	if (_name2fun.count("main") == 0)
		throw ValidationError("No 'main()' function found");
	for (const auto& v : _vars)
		v->validate();
	for (auto& f : _funs)
		f->validate(*this);
}

void VarDef::validate() const {
	// TODO: variables must neither contain '%' nor '$'
	if (_name.at(0) == '$')
		throw ValidationError("Variables must not start with character '$'.");
}

void FunDef::validate(const Program& prog) {
	if (_name2var.size() != _vars.size())
		throw ValidationError("Duplicate local variable declaration in function '" + _name + "()'.");
	for (const auto& v : _vars)
		v->validate();
	for (auto& s : _stmts)
		s->validate(prog, *this);
}

/**** Statements ****/

void While::validate(const Program& prog, const FunDef& fun) {
	_cond->validate(prog, fun);
	if (_cond->type() != bool_t)
		throw ValidationError("Condition for 'while' statement in '" + fun.name() + "()' not of type 'bool'.");
	for (auto& s : _stmts)
		s->validate(prog, fun);
}

void Ite::validate(const Program& prog, const FunDef& fun) {
	_cond->validate(prog, fun);
	if (_cond->type() != bool_t)
		throw ValidationError("Condition for 'if' statement in '" + fun.name() + "()' not of type 'bool'.");
	for (auto& s : _if)
		s->validate(prog, fun);
	for (auto& s : _else)
		s->validate(prog, fun);
}

void Call::validate(const Program& prog, const FunDef& fun) {
	if (prog.name2fun().count(_funname) == 0)
		throw ValidationError("Call to unknown function '" + _funname + "()' in '" + fun.name() + "()'.");
	else
		_decl = prog.name2fun().at(_funname);
}

void SimpleAssignment::validate(const Program& prog, const FunDef& fun) {
	_var->validate(prog, fun);
	_expr->validate(prog, fun);
	if (_var->type() != _expr->type())
		throw ValidationError("Assignment to variable '" + _var->name() + "' in function '" + fun.name() + "()' not viable.");
}

void ParallelAssignment::validate(const Program& prog, const FunDef& fun) {
	if (_vars.size() != _exprs.size())
		throw ValidationError("Parallel Assignment in function '" + fun.name() + "()' is unbalanced.");
	if (_vars.size() == 0)
		throw ValidationError("Parallel Assignment in function '" + fun.name() + "()' is empty.");
	if (_vars.size() == 1)
		throw ValidationError("Parallel Assignment in function '" + fun.name() + "()' is simple.");
	for (auto& v : _vars)
		v->validate(prog, fun);
	for (auto& e : _exprs)
		e->validate(prog, fun);
	for (std::size_t i = 0; i < _vars.size(); i++)
		if (_vars.at(i)->type() != _exprs.at(i)->type())
			throw ValidationError("Parallel Assignment to variable '" + _vars.at(i)->name() + "' in function '" + fun.name() + "()' not viable.");
	for (std::size_t i = 0; i < _vars.size(); i++)
		for (std::size_t j = i+1; j < _vars.size(); j++)
			if (_vars[i]->name() == _vars[j]->name())
				throw ValidationError("Parallel Assignment in function '" + fun.name() + "()' hast multiple assignments to variable '" + _vars[i]->name() + "'.");
}

void AssBase::validate(const Program& prog, const FunDef& fun) {
	_expr->validate(prog, fun);
	if (_expr->type() != bool_t)
		throw ValidationError("Expression for '" + _name + "' statement in function '" + fun.name() + "()' must be of type 'bool'.");
}

void Skip::validate(const Program& prog, const FunDef& fun) {}

/**** Expressions ****/

void Conditional::validate(const Program& prog, const FunDef& fun) {
	_cond->validate(prog, fun);
	_if->validate(prog, fun);
	_else->validate(prog, fun);
	if (_cond->type() != bool_t)
		throw ValidationError("Condition of conditional expression in '" + fun.name() + "()' not of type 'bool'.");
	if (_if->type() != bool_t)
		throw ValidationError("If-branch of conditional expression in '" + fun.name() + "()' not of type 'bool'.");
	if (_else->type() != bool_t)
		throw ValidationError("Else-branch of conditional expression in '" + fun.name() + "()' not of type 'bool'.");
}

void UnaryExpression::validate(const Program& prog, const FunDef& fun) {
	_child->validate(prog, fun);
	if (_child->type() != type())
		throw ValidationError("Unary expression in function '" + fun.name() + "()' wrongly typed.");
}

void BinaryExpression::validate(const Program& prog, const FunDef& fun) {
	_left->validate(prog, fun);
	_right->validate(prog, fun);
	if (_left->type() != _op.subtype() || _right->type() != _op.subtype())
		throw ValidationError("Binary expression in function '" + fun.name() + "()' wrongly typed.");
}

void Literal::validate(const Program& prog, const FunDef& fun) {}

void VarName::validate(const Program& prog, const FunDef& fun) {
	if (prog.name2var().count(_value) > 0)
		_decl = prog.name2var().at(_value);
	else if (fun.name2var().count(_value) > 0)
		_decl = fun.name2var().at(_value);
	else
		throw ValidationError("Undeclared variable '" + _value + "' in function '" + fun.name() + "()'.");
	_type = _decl->type();
}

void Unknown::validate(const Program& prog, const FunDef& fun) {}

void DocString::validate(const Program& prog, const FunDef& fun) {}

/**** Predicates ****/

void Predicate::validate(const Program& prog, const FunDef& fun, std::string name) {
	_name = name;
	if (VERBOSE) {
		// verbose naming
		std::stringstream s;
		s << "${";
		_expr->prettyprint(s);
		s << "}";
		_name = s.str();
	}
	_expr->validate(prog, fun);
	if (_expr->type() != bool_t)
		throw ValidationError("Type of predicate must be 'bool'");
}

#define DUMMYFUNC FunDef("__global_scope__", std::vector<VarDef*>(), std::vector<Statement*>())
#define GLOBAL_PREFIX "g"
#define LOCAL_PREFIX "l"

void PredicateList::validate(const Program& prog) {
	int gc = 0, lc = 0;
	for (auto& kvp : _name2pred)
		if (kvp.first != "global" && prog.name2fun().count(kvp.first) == 0)
			throw ValidationError("Predicate refers to undeclared function '" + kvp.first + "()'.");
		else
			for (Predicate* p : kvp.second)
				if (kvp.first == "global") p->validate(prog, DUMMYFUNC, GLOBAL_PREFIX + std::to_string(gc++));
				else p->validate(prog, *prog.name2fun().at(kvp.first), LOCAL_PREFIX + std::to_string(lc++));
}
