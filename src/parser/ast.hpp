#pragma once

#include <map>
#include <list>
#include <string>
#include <iostream>
#include "parserlib.hpp"
#include "ast/ast.hpp"


namespace parser {

	using type_t = const unsigned short;
	static type_t bool_t = 1;
	static type_t int_t = 2;

}


/******************************************************************************
	AVAILABLE AST CLASSES
 ******************************************************************************/

namespace parser {

	// basics
	class AstNode;
	class Identifier;
	class FunName;

	// expression classes
	class Expr;
	class Number;
	class Boolean;
	class VarName;
	class ArithmeticalNeg;
	class ArithmeticalMult;
	class ArithmeticalDiv;
	class ArithmeticalAdd;
	class ArithmeticalSub;
	class ComparisonEq;
	class ComparisonNeq;
	class ComparisonLt;
	class ComparisonLte;
	class ComparisonGt;
	class ComparisonGte;
	class LogicalNot;
	class LogicalOr;
	class LogicalAnd;

	// type classes
	class Type;
	class IntType;
	class BoolType;

	// statement classes
	class Statement;
	class IfThen;
	class ElseSeparator;
	class IfThenElse;
	class While;
	class Call;
	struct assign_t;
	class Assignment;
	class Assume;
	class Assert;
	class Skip;

	// declaration classes
	class VarDef;
	class FunDef;
	class Program;

	// predicates
	class Predicate;
	class PredicateBlock;
	class PredicateList;

}


/******************************************************************************
	BASICS
 ******************************************************************************/

namespace parser {

	class AstNode : public parserlib::ast_container {};

	class Identifier : public AstNode {
		private:
			std::string _name;

		public:
			virtual void construct(parserlib::ast_stack &st);
			std::string name() const { return _name; }
	};

	class FunName : public AstNode {
		private:
			parserlib::ast_ptr<Identifier> _ident;

		public:
			std::string name() const { return _ident->name(); }
	};

}


/******************************************************************************
	TYPES
 ******************************************************************************/

namespace parser {

	class Type : public AstNode {
		public:
			virtual type_t type() const = 0;
			virtual std::string name() const = 0;
	};

	class IntType : public Type {
		public:
			virtual type_t type() const { return int_t; }
			virtual std::string name() const { return "int"; }
	};

	class BoolType : public Type {
		public:
			virtual type_t type() const { return bool_t; }
			virtual std::string name() const { return "bool"; }
	};

}


/******************************************************************************
	EXPRESSIONS + STATEMENTS
 ******************************************************************************/

namespace parser {

	class Statement : public AstNode {
		public:
			virtual ::ast::Statement* toAST() = 0;
	};

	class IfThen : public Statement {
		private:
			parserlib::ast_ptr<Expr> _cond;
			parserlib::ast_list<Statement> _stmts;

		public:
			::ast::Statement* toAST();
	};

	class ElseSeparator : public AstNode {};

	class IfThenElse : public Statement {
		private:
			parserlib::ast_ptr<Expr> _cond;
			parserlib::ast_list<Statement> _else;
			parserlib::ast_ptr<ElseSeparator> _dummy;
			parserlib::ast_list<Statement> _if;

		public:
			::ast::Statement* toAST();
	};

	class While : public Statement {
		private:
			parserlib::ast_ptr<Expr> _cond;
			parserlib::ast_list<Statement> _stmts;

		public:
			::ast::Statement* toAST();
	};

	class Call : public Statement {
		private:
			parserlib::ast_ptr<FunName> _fun;

		public:
			::ast::Statement* toAST();
	};

	class AssignSeparator : public AstNode {};

	class Assignment : public Statement {
		private:
			parserlib::ast_list<VarName> _lhs;
			parserlib::ast_ptr<AssignSeparator> _dummy;
			parserlib::ast_list<Expr> _rhs;

		public:
			::ast::Statement* toAST();
	};

	class Assume : public Statement {
		private:
			parserlib::ast_ptr<Expr> _expr;

		public:
			::ast::Statement* toAST();
	};

	class Assert : public Statement {
		private:
			parserlib::ast_ptr<Expr> _expr;

		public:
			::ast::Statement* toAST();
	};

	class Skip : public Statement {
		public:
			::ast::Statement* toAST();
	};

}


/******************************************************************************
	EXPRESSIONS + STATEMENTS
 ******************************************************************************/
	
namespace parser {

	class Expr : public AstNode {
		public:
			virtual int precedence() const = 0;
			virtual ::ast::Expr* toAST() = 0;
	};

	class UnaryExpr : public Expr {
		protected:
			parserlib::ast_ptr<Expr> _child;

		public:
			virtual std::string op() const = 0;
			virtual type_t type() const = 0;
			virtual ::ast::Expr* toAST();
			virtual ::ast::unary_op toASTop() const = 0;
	};

	class BinaryExpr : public Expr {
		protected:
			parserlib::ast_ptr<Expr> _left;
			parserlib::ast_ptr<Expr> _right;

		public:
			virtual std::string op() const = 0;
			virtual type_t type() const = 0;
			virtual type_t subtype() const = 0;
			virtual ::ast::Expr* toAST();
			virtual ::ast::binary_op toASTop() const = 0;
	};


	/**** Values/Variables ****/


	class Literal : public Expr {
		protected:
			std::string _value;

		public:
			virtual int precedence() const { return 7; }
			virtual void construct(parserlib::ast_stack &st);
	};

	class Number : public Literal {
		public:
			virtual ::ast::Expr* toAST();
	};

	class Boolean : public Literal {
		public:
			virtual ::ast::Expr* toAST();
	};


	class VarName : public Expr {
		private:
			parserlib::ast_ptr<Identifier> _ident;

		public:
			virtual int precedence() const { return 7; }
			std::string name() const { return _ident->name(); }
			virtual ::ast::Expr* toAST();
	};

	class Unknown : public Expr {
		public:
			virtual int precedence() const { return 7; }
			virtual ::ast::Expr* toAST();
	};


	/**** Black Magic ****/

	// TODO: redundant information: name, type, precedence, op <=> astop


	#define UNARY_EXPR(NAME, TYPE, PRECEDENCE, OP, ASTOP) class NAME : public UnaryExpr {\
		public: \
			virtual type_t type() const { return TYPE; } \
			virtual std::string op() const { return OP; } \
			virtual int precedence() const { return PRECEDENCE; } \
			virtual ::ast::unary_op toASTop() const { return ASTOP; } \
	};

	#define BINARY_EXPR(NAME, TYPE, SUBTYPE, PRECEDENCE, OP, ASTOP) class NAME : public BinaryExpr {\
		public: \
			virtual type_t type() const { return TYPE; } \
			virtual std::string op() const { return OP; } \
			virtual type_t subtype() const { return SUBTYPE; } \
			virtual int precedence() const { return PRECEDENCE; } \
			virtual ::ast::binary_op toASTop() const { return ASTOP; } \
	};


	/**** Arithmetic ****/


	UNARY_EXPR(ArithmeticalNeg, int_t, 6, "-", ::ast::ari_neg);
	BINARY_EXPR(ArithmeticalMult, int_t, int_t, 5, "*", ::ast::ari_mult);
	BINARY_EXPR(ArithmeticalDiv, int_t, int_t, 5, "/", ::ast::ari_div);
	BINARY_EXPR(ArithmeticalAdd, int_t, int_t, 4, "+", ::ast::ari_plus);
	BINARY_EXPR(ArithmeticalSub, int_t, int_t, 4, "-", ::ast::ari_minus);


	/**** Comparisons ****/


	BINARY_EXPR(ComparisonEq, bool_t, int_t, 3, "==", ::ast::cmp_eq);
	BINARY_EXPR(ComparisonNeq, bool_t, int_t, 3, "!=", ::ast::cmp_neq);
	BINARY_EXPR(ComparisonLt, bool_t, int_t, 3, "<", ::ast::cmp_lt);
	BINARY_EXPR(ComparisonLte, bool_t, int_t, 3, "<=", ::ast::cmp_lte);
	BINARY_EXPR(ComparisonGt, bool_t, int_t, 3, ">", ::ast::cmp_gt);
	BINARY_EXPR(ComparisonGte, bool_t, int_t, 3, ">=", ::ast::cmp_gte);


	/**** Logics ****/


	UNARY_EXPR(LogicalNot, bool_t, 6, "!", ::ast::log_not);
	BINARY_EXPR(LogicalAnd, bool_t, bool_t, 2, "&&", ::ast::log_and);
	BINARY_EXPR(LogicalOr, bool_t, bool_t, 1, "||", ::ast::log_or);

}


/******************************************************************************
	DECLARATIONS
 ******************************************************************************/

namespace parser {

	class VarDef : public AstNode {
		private:
			parserlib::ast_ptr<Type> _type;
			parserlib::ast_ptr<Identifier> _ident;

		public:
			::ast::VarDef* toAST();
	};

	class FunDef : public AstNode {
		private:
			parserlib::ast_ptr<Identifier> _ident;
			parserlib::ast_list<VarDef> _vars;
			parserlib::ast_list<Statement> _stmts;

		public:
			::ast::FunDef* toAST();
	};

	class Program : public AstNode {
		private:
			parserlib::ast_list<VarDef> _vars;
			parserlib::ast_list<FunDef> _funs;

		public:
			::ast::Program* toAST();
	};

}


/******************************************************************************
	PREDICATES
 ******************************************************************************/

namespace parser {

	class Predicate : public AstNode {
		private:
			parserlib::ast_ptr<Expr> _expr;
			std::string _varname;

		public:
			::ast::Predicate* toAST();
	};

	class PredicateBlock : public AstNode {
		private:
			parserlib::ast_ptr<Identifier> _ident;
			parserlib::ast_list<Predicate> _preds;

		public:
			std::string name() const { return _ident->name(); }
			std::list<Predicate*> preds() const { return _preds.objects(); }
	};

	class PredicateList : public AstNode {
		private:
			parserlib::ast_list<PredicateBlock> _blocks;

		public:
			::ast::PredicateList* toAST();
	};

}
