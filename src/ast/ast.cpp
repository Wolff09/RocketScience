#include "ast/ast.hpp"

using namespace ast;


// this file contains some functions that were not split up into different .cpp files


/*** Predicate stuff ***/


const std::vector<Predicate*> PredicateList::preds_for(std::string scopename) const {
	if (_name2pred.count(scopename) > 0) return _name2pred.at(scopename);
	else return std::vector<Predicate*>();
}


/*** Literal stuff ***/

bool Literal::bool_value() const {
	assert(type() == bool_t);
	return _value == "true";
}

int Literal::int_value() const {
	assert(type() == int_t);
	return std::stoi(_value);
}


/*** entry2fun ***/

FunDef* Program::entry2fun(symbolic::Node entry) const {
	assert(entry.type == symbolic::ENTRY);
	FunDef* result = _funs.at(entry.id).get();
	assert(result->cfg_procedure()->entry() == entry);
	return result;
}


/*** add_initializers ***/

void Program::add_initializers() {
	for (auto& f : _funs)
		f->add_initializers(*this);
}

void add_init(std::vector<VarName*>& vars, std::vector<Expr*>& exprs, const VarDef& var) {
	vars.push_back(new VarName(var.name()));
	if (var.type() == bool_t) exprs.push_back(new Literal(false));
	else if (var.type() == int_t) exprs.push_back(new Literal(0));
	else assert(false);
}

void FunDef::add_initializers(const Program& prog) {
	std::vector<VarName*> vars;
	std::vector<Expr*> exprs;

	if (_name == "main")
		for (const auto& e : prog.name2var())
			add_init(vars, exprs, *(e.second));
	for (const auto& v : _vars)
		add_init(vars, exprs, *v);

	Statement* initializer = NULL;
	if (vars.size() == 0) return;
	if (vars.size() == 1) initializer = new SimpleAssignment(vars[0], exprs[0]);
	else if (vars.size() > 1) initializer = new ParallelAssignment(vars, exprs);

	// add initializer to the front of _stmts
	// -> create temporary vector; move all statements into it; then move them back
	// TODO: this is nasty, improve?
	std::vector<std::unique_ptr<Statement>> new_stmts;
	new_stmts.push_back(std::unique_ptr<Statement>(initializer));
	std::move(_stmts.begin(), _stmts.end(), std::back_inserter(new_stmts));
	_stmts.clear();
	std::move(new_stmts.begin(), new_stmts.end(), std::back_inserter(_stmts));
}


/*** collect_potential_predicates ***/
// TODO: delete this function

void Conditional::collect_potential_predicates(std::vector<Expr*>& collection) const {
	_cond->collect_potential_predicates(collection);
	_if->collect_potential_predicates(collection);
	_else->collect_potential_predicates(collection);
}

void UnaryExpression::collect_potential_predicates(std::vector<Expr*>& collection) const {
	_child->collect_potential_predicates(collection);
}

void BinaryExpression::collect_potential_predicates(std::vector<Expr*>& collection) const {
	if (_op == log_and || _op == log_or) {
		_left->collect_potential_predicates(collection);
		_right->collect_potential_predicates(collection);
	}
	if (_op.is_comparision_op() && contains_any_var()) collection.push_back(copy());
}

void Literal::collect_potential_predicates(std::vector<Expr*>& collection) const {}

void VarName::collect_potential_predicates(std::vector<Expr*>& collection) const {
	if (_decl != NULL && _decl->type() == bool_t)
		collection.push_back(copy());
}

void Unknown::collect_potential_predicates(std::vector<Expr*>& collection) const {}

void SymbolicConstant::collect_potential_predicates(std::vector<Expr*>& collection) const {}






// // TODO: remove this... its debug stuff
// void Statement::print_father() const { std::cout << "-*- something is missing -*-" << std::endl; }
// void Call::print_father() const { std::cout << "        " << _decl->name() << "();" << std::endl; }
// void ParallelAssignment::print_father() const {
//	if (_simple_father != NULL) _simple_father->prettyprint(std::cout, 2);
//	else if (_parallel_father != NULL) _parallel_father->prettyprint(std::cout, 2);
//	else assert(false);
// }
// void SimpleAssignment::print_father() const {
//	if (_simple_father != NULL) _simple_father->prettyprint(std::cout, 2);
//	else if (_parallel_father != NULL) _parallel_father->prettyprint(std::cout, 2);
//	else assert(false);
// }
// void Assume::print_father() const {
//	std::cout << "        assume(";
//	if (!_father_positive) UnaryExpression(log_not, _father_expr->copy()).prettyprint(std::cout);
//	else _father_expr->prettyprint(std::cout);
//	std::cout << ")" << ";" << std::endl;
// }
// void Assert::print_father() const {
//	// if (_originates_from != NULL) _originates_from->prettyprint(std::cout, 2);
//	// else assert(false);
//	std::cout << "        assert(false)" << std::endl;
// }
// void Skip::print_father() const {
//	if (_simple_father != NULL) _simple_father->prettyprint(std::cout, 2);
//	else if (_parallel_father != NULL) _parallel_father->prettyprint(std::cout, 2);
//	else assert(false);
// }