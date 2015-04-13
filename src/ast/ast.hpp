#pragma once

#include <map>
#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include <exception>
#include <z3++.h>
#include "symbolic/ControlFlowGraph.hpp"

/**
 * @brief Abstract Syntax Tree for representing a subset of C programs.
 *        Also provides utilities for CEGAR implementations.
 */
namespace ast {

	/******************************************************************************
		Classes
	 ******************************************************************************/

	class Program;
	class VarDef;
	class FunDef;

	class Statement;
	class TraceableStatement;
	class Assignment;
	class While;
	class Ite;
	class Call;
	class Return;
	class ParallelAssignment;
	class SimpleAssignment;
	class Assume;
	class Assert;
	class Skip;

	class Expr;
	class Conditional;
	class UnaryExpression;
	class BinaryExpression;
	class Literal;
	class VarName;
	class Unknown;

	class Predicate;
	class PredicateList;

	class Exception;
	class ValidationError;
	class UnsupportedOperationError;


	/******************************************************************************
		Types and Operators
	 ******************************************************************************/

	/**
	 * @brief Representation for types in the AST.
	 */
	class type_t {
		private:
			unsigned short _id;
			std::string _name;
			type_t(unsigned short id, std::string name) : _id(id), _name(name) {}
		public:
			std::string name() const { return _name; }
			static type_t bool_t() { return type_t(1, "bool"); }
			static type_t int_t() { return type_t(2, "int"); }
			static type_t unknown_t() { return type_t(77, "??"); }
			inline bool operator==(const type_t& rhs) const { return _id == rhs._id; }
			inline bool operator!=(const type_t& rhs) const { return !(*this == rhs); }
	};

	static type_t bool_t = type_t::bool_t();
	static type_t int_t = type_t::int_t();
	static type_t unknown_t = type_t::unknown_t();

	/**
	 * @brief Representation for unary operators in the AST.
	 */
	class unary_op {
		private:
			type_t _type;
			std::string _symbol;
			unsigned short _precedence;
			unary_op(type_t type, std::string symbol, unsigned short precedence) : _type(type), _symbol(symbol), _precedence(precedence) {}
		public:
			type_t type() const { return _type; }
			std::string symbol() const { return _symbol;}
			unsigned short precedence() const { return _precedence; }
			static unary_op Not() { return unary_op(bool_t, "!", 6); }
			static unary_op Neg() { return unary_op(int_t, "-", 6); }
			inline bool operator==(const unary_op& rhs) const { return _symbol == rhs._symbol; }
			inline bool operator!=(const unary_op& rhs) const { return !(*this == rhs); }
	};

	static unary_op ari_neg = unary_op::Neg();
	static unary_op log_not = unary_op::Not();


	/**
	 * @brief Representation for binar operators in the AST.
	 */
	class binary_op {
		private:
			type_t _type;
			type_t _subtype;
			std::string _symbol;
			unsigned short _precedence;
			unsigned short _op_type; // 1 = logic, 2 = comparision, 3 = arithmetic
			binary_op(type_t type, type_t subtype, std::string symbol, unsigned short precedence, unsigned short op_type)
			         : _type(type), _subtype(subtype), _symbol(symbol), _precedence(precedence), _op_type(op_type) {}
		public:
			type_t type() const { return _type; }
			type_t subtype() const { return _subtype; }
			std::string symbol() const { return _symbol; }
			unsigned short precedence() const { return _precedence; }
			bool is_logic_op() const { return _op_type == 1; }
			bool is_comparision_op() const { return _op_type == 2; }
			bool is_arithmetic_op() const { return _op_type == 3; }
			static binary_op Or() { return binary_op(bool_t, bool_t, "||", 1, 1); }
			static binary_op And() { return binary_op(bool_t, bool_t, "&&", 2, 1); }
			static binary_op Lt() { return binary_op(bool_t, int_t, "<", 3, 2); }
			static binary_op Lte() { return binary_op(bool_t, int_t, "<=", 3, 2); }
			static binary_op Gt() { return binary_op(bool_t, int_t, ">", 3, 2); }
			static binary_op Gte() { return binary_op(bool_t, int_t, ">=", 3, 2); }
			static binary_op Eq() { return binary_op(bool_t, int_t, "==", 3, 2); }
			static binary_op Neq() { return binary_op(bool_t, int_t, "!=", 3, 2); }
			static binary_op Plus() { return binary_op(int_t, int_t, "+", 4, 3); }
			static binary_op Minus() { return binary_op(int_t, int_t, "-", 4, 3); }
			static binary_op Mult() { return binary_op(int_t, int_t, "*", 5, 3); }
			static binary_op Div() { return binary_op(int_t, int_t, "/", 5, 3); }
			inline bool operator==(const binary_op& rhs) const { return _symbol == rhs._symbol; }
			inline bool operator!=(const binary_op& rhs) const { return !(*this == rhs); }
	};

	static binary_op ari_plus = binary_op::Plus();
	static binary_op ari_minus = binary_op::Minus();
	static binary_op ari_mult = binary_op::Mult();
	static binary_op ari_div = binary_op::Div();
	static binary_op log_or = binary_op::Or();
	static binary_op log_and = binary_op::And();
	static binary_op cmp_lt = binary_op::Lt();
	static binary_op cmp_lte = binary_op::Lte();
	static binary_op cmp_gt = binary_op::Gt();
	static binary_op cmp_gte = binary_op::Gte();
	static binary_op cmp_eq = binary_op::Eq();
	static binary_op cmp_neq = binary_op::Neq();


	/******************************************************************************
		DECLARATIONS
	 ******************************************************************************/

	/**
	 * @brief Top-level AST node.
	 */
	class Program {
		private:
			std::vector<std::unique_ptr<VarDef>> _vars;
			std::vector<std::unique_ptr<FunDef>> _funs;
			std::map<std::string, VarDef*> _name2var;
			std::map<std::string, FunDef*> _name2fun;

		public:
			Program(std::vector<VarDef*> vars, std::vector<FunDef*> funs);
			/**
			 * @brief Gives a mapping from available variable names to their Declaration.
			 */
			const std::map<std::string, VarDef*> name2var() const { return _name2var; }
			/**
			 * @brief Gives a mapping from available function names to their Definition.
			 */
			const std::map<std::string, FunDef*> name2fun() const { return _name2fun; }
			/**
			 * @brief Checks whether all internal constraints are met.
			 * @details Mandatory to call before using any other method.
			 */
			void validate();
			/**
			 * @brief Prints the program to the standard output
			 * @details Assumes a validated program.
			 * @see Program::validate
			 */
			void prettyprint() const;
			/**
			 * @brief Writes the source code of this program to the given output.
			 * @details Assumes a validated program.
			 * @see Program::validate
			 * @param os a stream to write the source code to
			 */
			void prettyprint(std::ostream& os) const;
			/**
			 * @brief Adds assignments to the beginning of every function initializing variables with default values.
			 * @details Assumes a validated program.
			 * @see Program::validate
			 */
			void add_initializers();
			/**
			 * @brief Generates a predicate abstraction of the program.
			 * @details This is based on the predicate abstraction described by Thomas Ball (Microsoft Research)
			 *          in "Automatic Predicate Abstraction of C Programs" (2001)
			 *          
			 *          Assumes a validated program.
			 * 
			 * @see http://dl.acm.org/citation.cfm?id=378846
			 * @see http://www.cs.ucla.edu/~todd/research/pldi01.pdf
			 * @see Program::validate
			 * @param pl Predicates for the abstraction
			 * @return the abstracted program; ownership sould be claimed
			 */
			Program* abstract(const PredicateList& pl) const;
			/**
			 * @brief Translates this program into a ControlFlowGraph.
			 * @details Assumes that this program is the result of a predicate abstraction and that it is validated.
			 * @see Program::abstract
			 * @see Program::validate
			 * @return the corresponding ControlFlowGraph; ownership should be claimed
			 */
			symbolic::ControlFlowGraph* cfg();
			FunDef* entry2fun(symbolic::Node entry) const;
	};

	class VarDef {
		private:
			std::string _name;
			type_t _type;
			std::size_t _cfg_id;
			const Program* _prog = NULL;
			const FunDef* _fun = NULL;

		public:
			VarDef(std::string name, type_t type);
			std::string name() const { return _name; }
			type_t type() const { return _type; }
			bool is_global() const { return _prog != NULL; }
			bool is_local() const { return _fun != NULL; }
			const FunDef* function() const { return _fun; }
			void validate() const;
			void prettyprint(std::ostream& os, int indent) const;
			std::size_t cfgid() const { return _cfg_id; }
			BDD cfg(const symbolic::ControlFlowGraph& cfg, bool primed=false) const;
			void cfg_pass_one(std::size_t& index);

		friend class Program;
		friend class FunDef;
	};

	class FunDef {
		private:
			std::string _name;
			std::vector<std::unique_ptr<VarDef>> _vars;
			std::vector<std::unique_ptr<Statement>> _stmts;
			std::map<std::string, VarDef*> _name2var;
			std::unique_ptr<symbolic::Procedure> _cfg_proc;
			std::unique_ptr<symbolic::Node> _cfg_last;

		public:
			FunDef(std::string name, std::vector<VarDef*> vars, std::vector<Statement*> stmts);
			const std::map<std::string, VarDef*> name2var() const { return _name2var; }
			std::string name() const { return _name; }
			void validate(const Program& prog);
			void prettyprint(std::ostream& os) const;
			void add_initializers(const Program& prog);
			FunDef* abstract(const PredicateList& pl, z3::solver& solver, z3::context& context) const;
			void cfg_pass_one(std::size_t& numVars, std::size_t& numBlocks, std::size_t& numProcs, std::size_t& numCalls);
			void cfg_pass_two(symbolic::ControlFlowGraph& cfg) const;
			void collect_cfg_transitions(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection) const;
			const symbolic::Procedure* cfg_procedure() const { return _cfg_proc.get(); }
			std::vector<const TraceableStatement*> flat_trace(const Program& abstract, const symbolic::ControlFlowGraph& cfg, const BDD callconf, const BDD returnconf, const BDD bounds, const BDD ignored_edges) const;
			// std::vector<Expr*> cfg_wp_proof(const Expr* phi, const Program& prog, const symbolic::ControlFlowGraph& cfg, const BDD preconf, const BDD postconf, const BDD bounds, const BDD ignored_edges) const;
	};


	/******************************************************************************
		STATEMENTS
	 ******************************************************************************/

	class Statement {
		protected:
			std::unique_ptr<symbolic::Node> _cfg_pre;

		public:
			virtual ~Statement() = default;
			virtual void validate(const Program& prog, const FunDef& fun) = 0;
			virtual void prettyprint(std::ostream& os, int indent) const = 0;
			virtual std::vector<Statement*> abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const = 0;
			virtual symbolic::Node cfg_pass_one(std::size_t& numNodes, std::size_t& numCalls, std::size_t& numProcs, symbolic::Node& pre);
			virtual void cfg_pass_two(symbolic::ControlFlowGraph& cfg) = 0;
			virtual void collect_cfg_transitions(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection) const = 0;
	};

	class TraceableStatement : public Statement {
		// TODO: they should have a wp
		public:
			virtual ~TraceableStatement() = default;
			virtual std::vector<const TraceableStatement*> flat_trace(const Program& abstract, const symbolic::ControlFlowGraph& cfg, const BDD preconf, const BDD postconf, const BDD bounds, const BDD ignored_edges) const = 0;
			virtual Expr* wp(const Expr& phi) const = 0;
			virtual Expr* con(std::map<const VarDef*, std::size_t>& lvalmap) const = 0;
	};

	class While : public Statement {
		private:
			std::unique_ptr<Expr> _cond;
			std::vector<std::unique_ptr<Statement>> _stmts;
			std::unique_ptr<symbolic::Node> _cfg_body_post, _cfg_post;

		public:
			While(Expr* cond, std::vector<Statement*> stmts);
			const Expr* condition() const { return _cond.get(); }
			void validate(const Program& prog, const FunDef& fun);
			virtual void prettyprint(std::ostream& os, int indent) const;
			virtual std::vector<Statement*> abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const;
			virtual symbolic::Node cfg_pass_one(std::size_t& numNodes, std::size_t& numCalls, std::size_t& numProcs, symbolic::Node& pre);
			virtual void cfg_pass_two(symbolic::ControlFlowGraph& cfg);
			virtual void collect_cfg_transitions(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection) const;
	};

	class Ite : public Statement {
		private:
			bool _has_else_branch;
			std::unique_ptr<Expr> _cond;
			std::vector<std::unique_ptr<Statement>> _if;
			std::vector<std::unique_ptr<Statement>> _else;
			std::unique_ptr<symbolic::Node> _cfg_if_post, _cfg_else_post, _cfg_post;

		public:
			Ite(Expr* cond, std::vector<Statement*> ifStmts);
			Ite(Expr* cond, std::vector<Statement*> ifStmts, std::vector<Statement*> elseStmts);
			const Expr* condition() const { return _cond.get(); }
			void validate(const Program& prog, const FunDef& fun);
			virtual void prettyprint(std::ostream& os, int indent) const;
			virtual std::vector<Statement*> abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const;
			virtual symbolic::Node cfg_pass_one(std::size_t& numNodes, std::size_t& numCalls, std::size_t& numProcs, symbolic::Node& pre);
			virtual void cfg_pass_two(symbolic::ControlFlowGraph& cfg);
			virtual void collect_cfg_transitions(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection) const;
	};

	class Call : public TraceableStatement {
		private:
			std::string _funname;
			FunDef* _decl;
			std::unique_ptr<symbolic::Call> _cfg_call;
			std::unique_ptr<Return> _trace_return;

		public:
			Call(std::string name);
			Call(std::string name, const Call& trace_father);
			void validate(const Program& prog, const FunDef& fun);
			virtual void prettyprint(std::ostream& os, int indent) const;
			virtual std::vector<Statement*> abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const;
			virtual symbolic::Node cfg_pass_one(std::size_t& numNodes, std::size_t& numCalls, std::size_t& numProcs, symbolic::Node& pre);
			virtual void cfg_pass_two(symbolic::ControlFlowGraph& cfg);
			virtual void collect_cfg_transitions(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection) const;
			virtual std::vector<const TraceableStatement*> flat_trace(const Program& abstract, const symbolic::ControlFlowGraph& cfg, const BDD preconf, const BDD postconf, const BDD bounds, const BDD ignored_edges) const;
			virtual Expr* wp(const Expr& phi) const;
			virtual Expr* con(std::map<const VarDef*, std::size_t>& lvalmap) const;
	};

	class Return : public TraceableStatement {
		// this is the counterpart for calls in traces; it must not occure in program code!
		public:
			virtual void validate(const Program& prog, const FunDef& fun) { assert(false); }
			virtual void prettyprint(std::ostream& os, int indent) const;
			virtual std::vector<Statement*> abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const { assert(false); }
			virtual symbolic::Node cfg_pass_one(std::size_t& numNodes, std::size_t& numCalls, std::size_t& numProcs, symbolic::Node& pre) { assert(false); }
			virtual void cfg_pass_two(symbolic::ControlFlowGraph& cfg) { assert(false); }
			virtual void collect_cfg_transitions(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection) const { assert(false); }
			virtual std::vector<const TraceableStatement*> flat_trace(const Program& abstract, const symbolic::ControlFlowGraph& cfg, const BDD preconf, const BDD postconf, const BDD bounds, const BDD ignored_edges) const { assert(false); }
			virtual Expr* wp(const Expr& phi) const;
			virtual Expr* con(std::map<const VarDef*, std::size_t>& lvalmap) const;
	};

	class Assignment : public TraceableStatement {
		// TODO: remove SimpleAssignment,ParallelAssignment,Skip in favour of a super-duper Assignment
		protected:
			std::unique_ptr<symbolic::Node> _cfg_post;
			const Assignment* _trace_stmt = NULL; // points to the assignment statement which created "this" during abstraction

		public:
			Assignment();
			Assignment(const Assignment* trace_father);
			virtual symbolic::Node cfg_pass_one(std::size_t& numNodes, std::size_t& numCalls, std::size_t& numProcs, symbolic::Node& pre);
			virtual void collect_cfg_transitions(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection) const;
			virtual std::vector<const TraceableStatement*> flat_trace(const Program& abstract, const symbolic::ControlFlowGraph& cfg, const BDD preconf, const BDD postconf, const BDD bounds, const BDD ignored_edges) const;
	};

	class ParallelAssignment : public Assignment {
		private:
			std::vector<std::unique_ptr<VarName>> _vars;
			std::vector<std::unique_ptr<Expr>> _exprs;

		public:
			ParallelAssignment(std::vector<VarName*> vars, std::vector<Expr*> exprs);
			ParallelAssignment(std::vector<VarName*> vars, std::vector<Expr*> exprs, const Assignment* trace_father);
			void validate(const Program& prog, const FunDef& fun);
			virtual void prettyprint(std::ostream& os, int indent) const;
			virtual std::vector<Statement*> abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const;
			virtual void cfg_pass_two(symbolic::ControlFlowGraph& cfg);
			virtual Expr* wp(const Expr& phi) const;
			virtual Expr* con(std::map<const VarDef*, std::size_t>& lvalmap) const;
	};

	class SimpleAssignment : public Assignment {
		private:
			std::unique_ptr<VarName> _var;
			std::unique_ptr<Expr> _expr;

		public:
			SimpleAssignment(VarName* var, Expr* expr);
			SimpleAssignment(VarName* var, Expr* expr, const Assignment* trace_father);
			void validate(const Program& prog, const FunDef& fun);
			virtual void prettyprint(std::ostream& os, int indent) const;
			virtual std::vector<Statement*> abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const;
			virtual void cfg_pass_two(symbolic::ControlFlowGraph& cfg);
			virtual Expr* wp(const Expr& phi) const;
			virtual Expr* con(std::map<const VarDef*, std::size_t>& lvalmap) const;
	};

	class AssBase : public TraceableStatement {
		protected:
			std::unique_ptr<Expr> _expr;
			std::string _name;
			std::unique_ptr<symbolic::Node> _cfg_post;
			std::unique_ptr<AssBase> _trace_stmt; // assume from an if/while abstraction

		public:
			AssBase(Expr* expr, std::string name);
			AssBase(Expr* expr, std::string name, AssBase* trace_father);
			virtual void validate(const Program& prog, const FunDef& fun);
			virtual void prettyprint(std::ostream& os, int indent) const;
			virtual std::vector<Statement*> abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const = 0;
			virtual symbolic::Node cfg_pass_one(std::size_t& numNodes, std::size_t& numCalls, std::size_t& numProcs, symbolic::Node& pre);
			virtual void cfg_pass_two(symbolic::ControlFlowGraph& cfg);
			virtual void collect_cfg_transitions(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection) const;
			virtual std::vector<const TraceableStatement*> flat_trace(const Program& abstract, const symbolic::ControlFlowGraph& cfg, const BDD preconf, const BDD postconf, const BDD bounds, const BDD ignored_edges) const;
	};

	class Assume : public AssBase {
		public:
			Assume(Expr* expr);
			Assume(Expr* expr, Expr* trace_father_expr);
			virtual std::vector<Statement*> abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const;
			virtual Expr* wp(const Expr& phi) const;
			virtual Expr* con(std::map<const VarDef*, std::size_t>& lvalmap) const;
	};

	class Assert : public AssBase {
		public:
			Assert(Expr* expr);
			Assert(Expr* expr, const Assert& trace_father);
			virtual std::vector<Statement*> abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const;
			virtual void cfg_pass_two(symbolic::ControlFlowGraph& cfg);
			virtual void collect_cfg_transitions(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection) const;
			virtual Expr* wp(const Expr& phi) const;
			virtual Expr* con(std::map<const VarDef*, std::size_t>& lvalmap) const;
	};

	class Skip : public Assignment {
		public:
			Skip();
			Skip(const Assignment* trace_father);
			void validate(const Program& prog, const FunDef& fun);
			virtual void prettyprint(std::ostream& os, int indent) const;
			virtual std::vector<Statement*> abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const;
			virtual void cfg_pass_two(symbolic::ControlFlowGraph& cfg);
			virtual Expr* wp(const Expr& phi) const;
			virtual Expr* con(std::map<const VarDef*, std::size_t>& lvalmap) const;
	};

	class DocString : public Statement {
		private:
			std::string _doc;

		public:
			DocString(std::string docstring);
			DocString(std::stringstream& docstring);
			virtual void validate(const Program& prog, const FunDef& fun);
			virtual void prettyprint(std::ostream& os, int indent) const;
			virtual std::vector<Statement*> abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const;
			virtual void cfg_pass_two(symbolic::ControlFlowGraph& cfg);
			virtual void collect_cfg_transitions(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection) const;
	};


	/******************************************************************************
		EXPRESSIONS
	 ******************************************************************************/

	class Expr {
		protected:
			type_t _type;
			int _precedence;

		public:
			Expr(type_t type, int precedence);
			virtual ~Expr() = default;
			type_t type() const { return _type; }
			int precedence() const { return _precedence; }
			virtual void validate(const Program& prog, const FunDef& fun) = 0;
			virtual void prettyprint(std::ostream& os) const = 0;
			virtual Expr* copy() const = 0;
			virtual Expr* replace(const std::map<std::string, const Expr*>& repl) const = 0;
			virtual bool contains(std::string varname) const = 0;
			virtual bool contains_any_var() const = 0;
			virtual bool contains_ignored_var() const = 0;
			virtual const FunDef* scope() const = 0;
			virtual z3::expr z3(z3::context& context) const = 0;
			virtual BDD cfg(const symbolic::ControlFlowGraph& cfg) const = 0;
			virtual void collect_potential_predicates(std::vector<Expr*>& collection) const = 0; // TODO: remove
			virtual Expr* push_ignore() const = 0;
			virtual Expr* pop_ignore() const = 0;
			virtual bool is_well_scoped() const = 0;
			virtual Expr* postprocess_interpolant(const Program& prog) const = 0;
	};

	class Conditional : public Expr {
		private:
			std::unique_ptr<Expr> _cond;
			std::unique_ptr<Expr> _if;
			std::unique_ptr<Expr> _else;

		public:
			Conditional(Expr* cond, Expr* yes, Expr* no);
			void validate(const Program& prog, const FunDef& fun);
			virtual void prettyprint(std::ostream& os) const;
			virtual Conditional* copy() const;
			virtual Expr* replace(const std::map<std::string, const Expr*>& repl) const;
			virtual bool contains(std::string varname) const;
			virtual bool contains_any_var() const;
			virtual bool contains_ignored_var() const;
			virtual const FunDef* scope() const;
			virtual z3::expr z3(z3::context& context) const;
			virtual BDD cfg(const symbolic::ControlFlowGraph& cfg) const;
			const Expr* guard() const { return _cond.get(); }
			const Expr* high() const { return _if.get(); }
			const Expr* low() const { return _else.get(); }
			virtual void collect_potential_predicates(std::vector<Expr*>& collection) const;
			virtual Expr* push_ignore() const;
			virtual Expr* pop_ignore() const;
			virtual bool is_well_scoped() const;
			virtual Expr* postprocess_interpolant(const Program& prog) const;
	};

	class UnaryExpression : public Expr {
		private:
			unary_op _op;
			std::unique_ptr<Expr> _child;

		public:
			UnaryExpression(const unary_op& op, Expr* child);
			void validate(const Program& prog, const FunDef& fun);
			virtual void prettyprint(std::ostream& os) const;
			virtual UnaryExpression* copy() const;
			virtual Expr* replace(const std::map<std::string, const Expr*>& repl) const;
			virtual bool contains(std::string varname) const;
			virtual bool contains_any_var() const;
			virtual bool contains_ignored_var() const;
			virtual const FunDef* scope() const;
			virtual z3::expr z3(z3::context& context) const;
			virtual BDD cfg(const symbolic::ControlFlowGraph& cfg) const;
			virtual void collect_potential_predicates(std::vector<Expr*>& collection) const;
			virtual Expr* push_ignore() const;
			virtual Expr* pop_ignore() const;
			virtual bool is_well_scoped() const;
			virtual Expr* postprocess_interpolant(const Program& prog) const;
	};

	class BinaryExpression : public Expr {
		private:
			binary_op _op;
			std::unique_ptr<Expr> _left;
			std::unique_ptr<Expr> _right;

		public:
			BinaryExpression(const binary_op& op, Expr* left, Expr* right);
			void validate(const Program& prog, const FunDef& fun);
			virtual void prettyprint(std::ostream& os) const;
			virtual BinaryExpression* copy() const;
			virtual Expr* replace(const std::map<std::string, const Expr*>& repl) const;
			virtual bool contains(std::string varname) const;
			virtual bool contains_any_var() const;
			virtual bool contains_ignored_var() const;
			virtual const FunDef* scope() const;
			virtual z3::expr z3(z3::context& context) const;
			virtual BDD cfg(const symbolic::ControlFlowGraph& cfg) const;
			virtual void collect_potential_predicates(std::vector<Expr*>& collection) const;
			virtual Expr* push_ignore() const;
			virtual Expr* pop_ignore() const;
			virtual bool is_well_scoped() const;
			virtual Expr* postprocess_interpolant(const Program& prog) const;
	};

	class Literal : public Expr {
		protected:
			std::string _value;
			Literal(type_t type, std::string value);

		public:
			Literal(bool val);
			Literal(int val);
			bool bool_value() const;
			int int_value() const;
			void validate(const Program& prog, const FunDef& fun);
			virtual void prettyprint(std::ostream& os) const;
			virtual Literal* copy() const;
			virtual Expr* replace(const std::map<std::string, const Expr*>& repl) const;
			virtual bool contains(std::string varname) const;
			virtual bool contains_any_var() const;
			virtual bool contains_ignored_var() const;
			virtual const FunDef* scope() const;
			virtual z3::expr z3(z3::context& context) const;
			virtual BDD cfg(const symbolic::ControlFlowGraph& cfg) const;
			virtual void collect_potential_predicates(std::vector<Expr*>& collection) const;
			virtual Expr* push_ignore() const;
			virtual Expr* pop_ignore() const;
			virtual bool is_well_scoped() const;
			virtual Expr* postprocess_interpolant(const Program& prog) const;
	};

	class VarName : public Literal {
		private:
			const /*TODO:change to const correct?*/ VarDef* _decl;
			std::size_t _ignore_replace; // replace() has an effect iff. _ignore_replace == 0

		public:
			VarName(const VarName& cp);
			VarName(const VarName* cp);
			VarName(std::string varname);
			VarName(const VarDef* def);
			std::string name() const { return _value; }
			const VarDef* decl() const { return _decl; }
			void validate(const Program& prog, const FunDef& fun);
			virtual VarName* copy() const;
			virtual Expr* replace(const std::map<std::string, const Expr*>& repl) const;
			virtual bool contains(std::string varname) const;
			virtual bool contains_any_var() const;
			virtual bool contains_ignored_var() const;
			virtual const FunDef* scope() const;
			virtual z3::expr z3(z3::context& context) const;
			virtual BDD cfg(const symbolic::ControlFlowGraph& cfg) const;
			virtual void collect_potential_predicates(std::vector<Expr*>& collection) const;
			virtual Expr* push_ignore() const;
			virtual Expr* pop_ignore() const;
			virtual bool is_well_scoped() const;
			virtual void prettyprint(std::ostream& os) const;
			virtual Expr* postprocess_interpolant(const Program& prog) const;
	};

	class Unknown : public Literal {
		public:
			Unknown();
			void validate(const Program& prog, const FunDef& fun);
			virtual Unknown* copy() const;
			virtual Expr* replace(const std::map<std::string, const Expr*>& repl) const;
			virtual bool contains(std::string varname) const;
			virtual bool contains_any_var() const;
			virtual bool contains_ignored_var() const;
			virtual const FunDef* scope() const;
			virtual z3::expr z3(z3::context& context) const;
			virtual BDD cfg(const symbolic::ControlFlowGraph& cfg) const;
			virtual void collect_potential_predicates(std::vector<Expr*>& collection) const;
			virtual Expr* push_ignore() const;
			virtual Expr* pop_ignore() const;
			virtual bool is_well_scoped() const;
			virtual Expr* postprocess_interpolant(const Program& prog) const;
	};

	class SymbolicConstant : public Literal {
		private:
			// std::unique_ptr<VarName> _var;
			std::size_t _num;
			const VarDef* _decl;

		public:
			SymbolicConstant(const VarDef* var, std::size_t num);
			// const VarName* var() const { return _var.get(); }
			std::size_t num() const { return _num; }
			virtual void prettyprint(std::ostream& os) const;
			virtual SymbolicConstant* copy() const;
			virtual Expr* replace(const std::map<std::string, const Expr*>& repl) const;
			virtual bool contains(std::string varname) const;
			virtual bool contains_any_var() const;
			virtual bool contains_ignored_var() const;
			virtual const FunDef* scope() const;
			virtual z3::expr z3(z3::context& context) const;
			virtual BDD cfg(const symbolic::ControlFlowGraph& cfg) const;
			virtual void collect_potential_predicates(std::vector<Expr*>& collection) const;
			virtual Expr* push_ignore() const;
			virtual Expr* pop_ignore() const;
			virtual bool is_well_scoped() const;
			virtual Expr* postprocess_interpolant(const Program& prog) const;
	};


	/******************************************************************************
		PREDICATES
	 ******************************************************************************/

	class Predicate {
		private:
			std::unique_ptr<Expr> _expr;
			std::string _name;
			std::unique_ptr<z3::expr> _z3repr;
			void clear_z3();

		public:
			Predicate(Expr* expr);
			std::string varname() const { assert(_name != ""); return _name; }
			const Expr* expr() const { return _expr.get(); }
			void validate(const Program& prog, const FunDef& fun, std::string name);
			void prettyprint(std::ostream& os, int indent) const;
			const z3::expr& z3() const;
			void precompute_z3(z3::context& context);

		friend class PredicateList;
	};

	class PredicateList {
		private:
			std::vector<std::unique_ptr<Predicate>> _ownership;
			std::map<std::string, std::vector<Predicate*>> _name2pred;
			void clear_z3() const;
			bool contains(const Predicate& pred, std::string scope) const;

		public:
			PredicateList(std::vector<std::pair<std::string, Predicate*>> predlist);
			const std::map<std::string, std::vector<Predicate*>> name2preds() const { return _name2pred; }
			void validate(const Program& prog);
			void prettyprint() const;
			void prettyprint(std::ostream& os) const;
			bool extend(Predicate* pred, std::string scope, bool check_for_duplicate=false);
			const std::vector<Predicate*> preds_for(std::string scopename) const;
			std::size_t size() const { return _ownership.size(); }

		friend class Program;
	};


	/******************************************************************************
		EXCEPTIONS
	 ******************************************************************************/

	class ValidationError: public std::exception {
		private:
			const char* _msg;
			const char* what() const throw() { return _msg; }

		public:
			ValidationError(std::string msg) : _msg(msg.c_str()) {}
	};

	class UnsupportedOperationError: public std::exception {
		private:
			const char* _msg;
			const char* what() const throw() { return _msg; }

		public:
			UnsupportedOperationError(std::string msg) : _msg(msg.c_str()) {}
	};

}
