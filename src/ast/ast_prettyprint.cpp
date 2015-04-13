#include "ast/ast.hpp"

using namespace ast;


#define WS ' '
#define NL '\n'
#define SEMI ';'
#define INDENT std::string(4*indent, ' ')
#define OUTPUT_DOCSTRING true

/**** Declarations ****/

void Program::prettyprint() const { prettyprint(std::cout); }

void Program::prettyprint(std::ostream& os) const {
	os << "/*************** BEGIN PROGRAM ***************/" << NL;
	for (auto& v : _vars) v->prettyprint(os, 0);
	for (auto& f : _funs) f->prettyprint(os);
	os << "/**************** END PROGRAM ****************/" << NL << std::flush;
}

void VarDef::prettyprint(std::ostream& os, int indent) const {
	os << INDENT << _type.name() << WS << _name << SEMI << NL;
}

void FunDef::prettyprint(std::ostream& os) const {
	os << NL << "void " << _name << "() {" << NL;
	for (auto& v : _vars) v->prettyprint(os, 1);
	for (auto& s : _stmts) s->prettyprint(os, 1);
	os << "}" << NL;
}

/**** Statements ****/

void While::prettyprint(std::ostream& os, int indent) const {
	os << INDENT << "while (";
	_cond->prettyprint(os);
	os << ") {" << NL;
	for (auto& s : _stmts) s->prettyprint(os, indent+1);
	os << INDENT << "}" << NL;
}

void Ite::prettyprint(std::ostream& os, int indent) const {
	os << INDENT << "if (";
	_cond->prettyprint(os);
	os << ") {" << NL;
	for (auto& s : _if) s->prettyprint(os, indent+1);
	if (_has_else_branch) {
		os << INDENT << "} else {" << NL;
		for (auto& s : _else) s->prettyprint(os, indent+1);
	}
	os << INDENT << "}" << NL;
}

void Call::prettyprint(std::ostream& os, int indent) const {
	os << INDENT << _funname << "();" << NL;
}

void Return::prettyprint(std::ostream& os, int indent) const {
	os << INDENT << "return;" << NL;
}

void SimpleAssignment::prettyprint(std::ostream& os, int indent) const {
	os << INDENT << _var->name() << " = ";
	_expr->prettyprint(os);
	os << SEMI << NL;
}

void ParallelAssignment::prettyprint(std::ostream& os, int indent) const {
	os << INDENT << _vars.front()->name();
	for (std::size_t i = 1; i < _vars.size(); i++) os << ", " << _vars.at(i)->name();
	os << " = " << NL;
	indent += 2;
	os << INDENT;
	_exprs.front()->prettyprint(os);
	for (std::size_t i = 1; i < _exprs.size(); i++) {
		os << ", " << NL << INDENT;
		_exprs.at(i)->prettyprint(os);
	}
	os << SEMI << NL;
}

void AssBase::prettyprint(std::ostream& os, int indent) const {
	os << INDENT << _name << "(";
	_expr->prettyprint(os);
	os << ")" << SEMI << NL;
}

void Skip::prettyprint(std::ostream& os, int indent) const {
	os << INDENT << ";" << NL;
}

void DocString::prettyprint(std::ostream& os, int indent) const {
	if (OUTPUT_DOCSTRING) os << INDENT << "// " << _doc;
}

/**** Expressions ****/

void Conditional::prettyprint(std::ostream& os) const {
	_cond->prettyprint(os);
	os << " ? ";
	_if->prettyprint(os);
	os << " : ";
	_else->prettyprint(os);
}

void UnaryExpression::prettyprint(std::ostream& os) const {
	bool nbr = precedence() > _child->precedence();
	os << _op.symbol() << (nbr ? "(" : "");
	_child->prettyprint(os);
	os << (nbr ? ")" : "");
}

void BinaryExpression::prettyprint(std::ostream& os) const {
	bool nbr_lhs = precedence() > _left->precedence();
	bool nbr_rhs = precedence() > _right->precedence();
	os << (nbr_lhs ? "(" : "");
	_left->prettyprint(os);
	os << (nbr_lhs ? ")" : "");
	os << " " << _op.symbol() << " ";
	os << (nbr_rhs ? "(" : "");
	_right->prettyprint(os);
	os << (nbr_rhs ? ")" : "");
}

void Literal::prettyprint(std::ostream& os) const {
	os << _value;
}

void VarName::prettyprint(std::ostream& os) const {
	// this allows debugging the value of _ignore_replace
	os << _value;
	os << "^" << _ignore_replace;
}

void SymbolicConstant::prettyprint(std::ostream& os) const {
	os << "⟨" << _decl->name() << "," << _num << "⟩";
}

// void Expression::prettyprint(std::ostream& os) const {
//	_expr->prettyprint(os);
// }

/**** Predicates ****/

void Predicate::prettyprint(std::ostream& os, int indent) const {
	os << INDENT;
	_expr->prettyprint(os);
	os << SEMI << NL;
}

void PredicateList::prettyprint() const { prettyprint(std::cout); }

void PredicateList::prettyprint(std::ostream& os) const {
	os << "/*********** BEGIN PREDICATE LIST ************/";
	for (auto& kvp : _name2pred) {
		os << NL << kvp.first << ":" << NL;
		for (Predicate* p : kvp.second)
			p->prettyprint(os, 1);
	}
	if (size() == 0) os << NL;
	os << "/************ END PREDICATE LIST *************/" << NL << std::flush;
}
