#include "ast/ast.hpp"

using namespace ast;


/**** Declarations ****/

Program::Program(std::vector<VarDef*> vars, std::vector<FunDef*> funs) {
	for (VarDef* v : vars) {
		_vars.push_back(std::unique_ptr<VarDef>(v));
		_vars.back()->_prog = this;
		_name2var[v->name()] = v;
	}
	for (FunDef* f : funs) {
		_funs.push_back(std::unique_ptr<FunDef>(f));
		_name2fun[f->name()] = f;
	}
}

VarDef::VarDef(std::string name, type_t type) : _type(type) {
	_name = name;
}

FunDef::FunDef(std::string name, std::vector<VarDef*> vars, std::vector<Statement*> stmts) {
	_name = name;
	for (VarDef* v : vars) {
		_vars.push_back(std::unique_ptr<VarDef>(v));
		_vars.back()->_fun = this;
		_name2var[v->name()] = v;
	}
	for (Statement* s : stmts) _stmts.push_back(std::unique_ptr<Statement>(s));
}

/**** Statements ****/

While::While(Expr* cond, std::vector<Statement*> stmts) {
	_cond.reset(cond);
	for (Statement* s : stmts) _stmts.push_back(std::unique_ptr<Statement>(s));
}

Ite::Ite(Expr* cond, std::vector<Statement*> ifStmts) : Ite(cond, ifStmts, std::vector<Statement*>()) {
	_has_else_branch = false;
}

Ite::Ite(Expr* cond, std::vector<Statement*> ifStmts, std::vector<Statement*> elseStmts) {
	_cond.reset(cond);
	for (Statement* s : ifStmts) _if.push_back(std::unique_ptr<Statement>(s));
	for (Statement* s : elseStmts) _else.push_back(std::unique_ptr<Statement>(s));
	_has_else_branch = true;
}

Call::Call(std::string name) : _funname(name) {}

Call::Call(std::string name, const Call& trace_father) : _funname(name) {
	_trace_return.reset(new Return());
}

Assignment::Assignment() {}

Assignment::Assignment(const Assignment* trace_father) : _trace_stmt(trace_father) {}

SimpleAssignment::SimpleAssignment(VarName* var, Expr* expr) : Assignment() {
	_var.reset(var);
	_expr.reset(expr);
}

SimpleAssignment::SimpleAssignment(VarName* var, Expr* expr, const Assignment* trace_father) : Assignment(trace_father) {
	_var.reset(var);
	_expr.reset(expr);
}

ParallelAssignment::ParallelAssignment(std::vector<VarName*> vars, std::vector<Expr*> exprs) : Assignment() {
	for (VarName* v : vars) _vars.push_back(std::unique_ptr<VarName>(v));
	for (Expr* e : exprs) _exprs.push_back(std::unique_ptr<Expr>(e));
}

ParallelAssignment::ParallelAssignment(std::vector<VarName*> vars, std::vector<Expr*> exprs, const Assignment* trace_father) : Assignment(trace_father) {
	for (VarName* v : vars) _vars.push_back(std::unique_ptr<VarName>(v));
	for (Expr* e : exprs) _exprs.push_back(std::unique_ptr<Expr>(e));
}

AssBase::AssBase(Expr* expr, std::string name) : _name(name) {
	_expr.reset(expr);
}

AssBase::AssBase(Expr* expr, std::string name, AssBase* trace_father) : _name(name) {
	_expr.reset(expr);
	_trace_stmt.reset(trace_father);
}

Assume::Assume(Expr* expr) : AssBase(expr, "assume") {}

Assume::Assume(Expr* expr, Expr* trace_father_expr) : AssBase(expr, "assume", new Assume(trace_father_expr)) {}

Assert::Assert(Expr* expr) : AssBase(expr, "assert") {}

Assert::Assert(Expr* expr, const Assert& trace_father) : AssBase(expr, "assert", new Assert(new Literal(false))) {}

Skip::Skip() {}

Skip::Skip(const Assignment* trace_father) : Assignment(trace_father) {}

DocString::DocString(std::string docstring) : _doc(docstring) {}

DocString::DocString(std::stringstream& docstring) : _doc(docstring.str()) {}

/**** Expressions ****/

Expr::Expr(type_t type, int precedence) : _type(type) {
	_precedence = precedence;
}

Conditional::Conditional(Expr* cond, Expr* yes, Expr* no) : Expr(bool_t, 0) {
	_cond.reset(cond);
	_if.reset(yes);
	_else.reset(no);
}

UnaryExpression::UnaryExpression(const unary_op& op, Expr* child) : Expr(op.type(), op.precedence()), _op(op) {
	_child.reset(child);
}

BinaryExpression::BinaryExpression(const binary_op& op, Expr* left, Expr* right) : Expr(op.type(), op.precedence()), _op(op) {
	_left.reset(left);
	_right.reset(right);
}

Literal::Literal(type_t type, std::string value) : Expr(type, 10) {
	_value = value;
}

Literal::Literal(bool val) : Expr(bool_t, 10), _value(val ? "true" : "false") {}
Literal::Literal(int val) : Expr(int_t, 10), _value(std::to_string(val)) {}

VarName::VarName(std::string varname) : Literal(unknown_t, varname), _ignore_replace(0) {}

VarName::VarName(const VarName& cp) : Literal(cp._type, cp._value), _decl(cp._decl), _ignore_replace(cp._ignore_replace) {}

VarName::VarName(const VarName* cp) : Literal(cp->_type, cp->_value), _decl(cp->_decl), _ignore_replace(cp->_ignore_replace) {}

VarName::VarName(const VarDef* def) : Literal(def->type(), def->name()), _ignore_replace(0), _decl(def) {}

Unknown::Unknown() : Literal(bool_t, "unknown") {}

SymbolicConstant::SymbolicConstant(const VarDef* var, std::size_t num) : _num(num), Literal("<>") {
	_decl = var;
}

// Expression::Expression(Expr* expr) {
//	_expr.reset(expr);
// }

/**** Predicates ****/

Predicate::Predicate(Expr* expr) {
	_expr.reset(expr);
}

PredicateList::PredicateList(std::vector<std::pair<std::string, Predicate*>> predlist) {
	for (auto& np : predlist) {
		_ownership.push_back(std::unique_ptr<Predicate>(np.second));
		if (_name2pred.count(np.first) == 0)
			_name2pred[np.first] = std::vector<Predicate*>();
	}

	for (auto&np : predlist)
		_name2pred[np.first].push_back(np.second);
}
