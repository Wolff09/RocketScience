#include <sstream>
#include "parser/grammar.hpp"
#include "parser/ast.hpp"

using namespace parser;


/******************************************************************************
	CONSTRUCTION
 ******************************************************************************/

std::string construction(parserlib::ast_stack &st, AstNode& node) {
	std::string val;
	std::stringstream stream;
	for(parserlib::input::iterator it = node.m_begin.m_it; it != node.m_end.m_it; ++it) {
		stream << (char)*it;
	}
	stream >> val;
	return val;
}

void Identifier::construct(parserlib::ast_stack &st) {
	_name = construction(st, *this);
}

void Literal::construct(parserlib::ast_stack &st) {
	_value = construction(st, *this);
}


/******************************************************************************
	toAST
 ******************************************************************************/

#define TC(X) X == bool_t ? ::ast::bool_t : X == int_t ? ::ast::int_t : ::ast::unknown_t

::ast::Program* Program::toAST() {
	std::vector<::ast::VarDef*> vars;
	for (VarDef* v : _vars.objects()) vars.push_back(v->toAST());
	std::vector<::ast::FunDef*> funs;
	for (FunDef* f : _funs.objects()) funs.push_back(f->toAST());
	return new ::ast::Program(vars, funs);
}

::ast::FunDef* FunDef::toAST() {
	std::vector<::ast::VarDef*> vars;
	for (VarDef* v : _vars.objects()) vars.push_back(v->toAST());
	std::vector<::ast::Statement*> stmts;
	for (Statement* s : _stmts.objects()) stmts.push_back(s->toAST());
	return new ::ast::FunDef(_ident->name(), vars, stmts);
}

::ast::VarDef* VarDef::toAST() {
	return new ::ast::VarDef(_ident->name(), TC(_type->type()));
}


::ast::Statement* IfThen::toAST() {
	std::vector<::ast::Statement*> ifstmts;
	for (Statement* s : _stmts.objects()) ifstmts.push_back(s->toAST());
	return new ::ast::Ite(_cond->toAST(), ifstmts);
}

::ast::Statement* IfThenElse::toAST() {
	std::vector<::ast::Statement*> ifstmts;
	for (Statement* s : _else.objects()) ifstmts.push_back(s->toAST());
	std::vector<::ast::Statement*> elsestmts;
	for (Statement* s : _if.objects()) elsestmts.push_back(s->toAST());
	return new ::ast::Ite(_cond->toAST(), ifstmts, elsestmts);
}

::ast::Statement* While::toAST() {
	std::vector<::ast::Statement*> stmts;
	for (Statement* s : _stmts.objects()) stmts.push_back(s->toAST());
	return new ::ast::While(_cond->toAST(), stmts);
}

::ast::Statement* Call::toAST() {
	return new ::ast::Call(_fun->name());
}

::ast::Statement* Assignment::toAST() {
	auto lhs = _lhs.objects();
	auto rhs = _rhs.objects();
	if (lhs.size() == rhs.size() && lhs.size() == 1) {
		// simple assignment
		return new ::ast::SimpleAssignment(new ::ast::VarName(lhs.front()->name()), rhs.front()->toAST());
	} else {
		// parallel assignment
		std::vector<::ast::VarName*> vars;
		for (VarName* v : lhs) vars.push_back(new ::ast::VarName(v->name()));
		std::vector<::ast::Expr*> exprs;
		for (Expr* e : rhs) exprs.push_back(e->toAST());
		return new ::ast::ParallelAssignment(vars, exprs);
	}
}

::ast::Statement* Assume::toAST() {
	return new ::ast::Assume(_expr->toAST());
}

::ast::Statement* Assert::toAST() {
	return new ::ast::Assert(_expr->toAST());
}

::ast::Statement* Skip::toAST() {
	return new ::ast::Skip();
}


::ast::Expr* UnaryExpr::toAST() {
	return new ::ast::UnaryExpression(toASTop(), _child->toAST());
}

::ast::Expr* BinaryExpr::toAST() {
	return new ::ast::BinaryExpression(toASTop(), _left->toAST(), _right->toAST());
}

::ast::Expr* Number::toAST() {
	return new ::ast::Literal(std::stoi(_value));
}

::ast::Expr* Boolean::toAST() {
	return new ::ast::Literal(_value == "true" ? true : false);
}

::ast::Expr* VarName::toAST() {
	return new ::ast::VarName(name());
}

::ast::Expr* Unknown::toAST() {
	return new ::ast::Unknown();
}


::ast::PredicateList* PredicateList::toAST() {
	std::vector<std::pair<std::string, ::ast::Predicate*>> predlist;
	for (PredicateBlock* b : _blocks.objects())
		for (Predicate* p : b->preds())
			predlist.push_back(std::pair<std::string, ::ast::Predicate*>(b->name(), p->toAST()));
	return new ::ast::PredicateList(predlist);
}

::ast::Predicate* Predicate::toAST() {
	return new ::ast::Predicate(_expr->toAST());
}
