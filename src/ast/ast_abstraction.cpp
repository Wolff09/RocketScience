#include <set>
#include <queue>
#include "ast/ast.hpp"
#include "ast/abstraction_utils.hpp"

using namespace ast;


Program* Program::abstract(const PredicateList& pl) const {
	std::vector<VarDef*> vars;
	std::vector<FunDef*> funs;

	z3::context context;
	z3::solver solver(context);

	for (Predicate* p : pl.preds_for("global")) {
		vars.push_back(new VarDef(p->varname(), bool_t));
		p->precompute_z3(context);
	}

	for (auto& f : _funs)
		funs.push_back(f->abstract(pl, solver, context));

	// remove every 'z3::expr' from the predicates as they need 'cntxt' upon deletion
	pl.clear_z3();

	Program* prog = new Program(vars, funs);
	prog->validate();
	return prog;
}

void append(std::vector<Statement*>& vec, std::vector<Statement*> val) {
	for (auto* s : val) vec.push_back(s);
}

FunDef* FunDef::abstract(const PredicateList& pl, z3::solver& solver, z3::context& context) const {
	std::vector<VarDef*> vars;
	std::vector<Statement*> stmts;
	solver.push();

	for (Predicate* p : pl.preds_for(_name)) {
		vars.push_back(new VarDef(p->varname(), bool_t));
		p->precompute_z3(context);
	}

	auto preds = std::vector<Predicate*>();
	auto global_preds = pl.preds_for("global");
	auto local_preds = pl.preds_for(_name);
	preds.insert(preds.end(), global_preds.begin(), global_preds.end());
	preds.insert(preds.end(), local_preds.begin(), local_preds.end());

	for (auto& s : _stmts)
		append(stmts, s->abstract(preds, solver, context));

	// add a dummy value to make the following pop never fail -> weird
	solver.add(context.bool_val(true));
	solver.pop();
	return new FunDef(_name, vars, stmts);
}

/**** Statements ****/

DocString* mk_doc(std::string ctrl, const Expr& expr) {
	std::stringstream com;
	com << ctrl << "(";
	expr.prettyprint(com);
	com << ")" << std::endl;
	return new DocString(com);
}

DocString* mk_doc(const SimpleAssignment& ass) {
	std::stringstream com;
	ass.prettyprint(com, 0);
	return new DocString(com);
}

std::vector<Statement*> While::abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const {
	std::vector<Statement*> stmts;

	auto posz3cond = _cond->z3(context);
	auto negz3cond = !posz3cond;

	auto pos = strongest_whatsoever(preds, posz3cond, solver);
	auto neg = strongest_whatsoever(preds, negz3cond, solver);

	stmts.push_back(new Assume(pos, _cond->copy()));

	for (auto& s : _stmts)
		append(stmts, s->abstract(preds, solver, context));

	auto whl = new While(new Unknown(), stmts);
	auto asu = new Assume(neg, new UnaryExpression(log_not, _cond->copy()));
	auto doc = mk_doc("while", *_cond);
	return { doc, whl, asu };
}

std::vector<Statement*> Ite::abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const {
	std::vector<Statement*> if_stmts;
	std::vector<Statement*> else_stmts;

	auto posz3cond = _cond->z3(context);
	auto negz3cond = !posz3cond;

	auto pos = strongest_whatsoever(preds, posz3cond, solver);
	auto neg = strongest_whatsoever(preds, negz3cond, solver);

	if_stmts.push_back(new Assume(pos, _cond->copy()));
	else_stmts.push_back(new Assume(neg, new UnaryExpression(log_not, _cond->copy())));

	for (auto& s : _if)
		append(if_stmts, s->abstract(preds, solver, context));
	if (_has_else_branch)
		for (auto& s : _else)
			append(else_stmts, s->abstract(preds, solver, context));

	// always add else branch with assume
	auto ite = new Ite(new Unknown(), if_stmts, else_stmts);
	auto doc = mk_doc("if", *_cond);
	return { doc, ite };
}

std::vector<Statement*> Call::abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const {
	Call* c = new Call(_funname, *this);
	c->_decl = _decl;
	return { c };
}

std::vector<Statement*> SimpleAssignment::abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const {
	std::vector<VarName*> vars;
	std::vector<Expr*> exprs;

	// create vectors for _var and _expr repr -> allows using the replace function from z3
	auto z3var = z3::expr_vector(context);
	z3var.push_back(_var->z3(context));
	auto z3expr = z3::expr_vector(context);
	z3expr.push_back(_expr->z3(context));

	for (Predicate* p : preds) {
		const Expr* pex = p->expr();
		if (!pex->contains(_var->name())) continue;

		z3::expr z3p = p->z3();
		z3::expr poswp = z3p.substitute(z3var, z3expr);
		z3::expr negwp = !poswp;

		Expr* pos = weakest_whatsoever(preds, poswp, solver);
		Expr* neg = weakest_whatsoever(preds, negwp, solver);

		// TODO: one could add an simplification step here; but: it is done on the BDD level anyway
		Expr* guard = new BinaryExpression(log_or, pos, neg);
		Expr* newval = new UnaryExpression(log_not, neg->copy());

		// note: the resulting assignment must be a Conditional, i.e. of the form 'guard ? value : unkown'
		vars.push_back(new VarName(p->varname()));
		exprs.push_back(new Conditional(guard, newval, new Unknown()));
	}
	
	auto doc = mk_doc(*this);
	if (vars.size() > 1) return { doc, new ParallelAssignment(vars, exprs, this) };
	else if (vars.size() == 1) return { doc, new SimpleAssignment(vars[0], exprs[0], this) };
	else return { doc, new Skip(this) };
}

std::vector<Statement*> ParallelAssignment::abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const {
	std::vector<VarName*> vars;
	std::vector<Expr*> exprs;

	// create vectors for _var and _expr repr -> allows using the replace function from z3
	auto z3vars = z3::expr_vector(context);
	auto z3exprs = z3::expr_vector(context);
	for (const auto& v : _vars) z3vars.push_back(v->z3(context));
	for (const auto& e : _exprs) z3exprs.push_back(e->z3(context));

	for (Predicate* p : preds) {
		const Expr* pex = p->expr();
		bool contains_any = false;
		for (const auto& v : _vars)
			contains_any = contains_any || pex->contains(v->name());
		if (!contains_any) continue;

		z3::expr z3p = p->z3();
		z3::expr poswp = z3p.substitute(z3vars, z3exprs);
		z3::expr negwp = !poswp;

		Expr* pos = weakest_whatsoever(preds, poswp, solver);
		Expr* neg = weakest_whatsoever(preds, negwp, solver);

		// TODO: one could add an simplification step here; but: it is done on the BDD level anyway
		Expr* guard = new BinaryExpression(log_or, pos, neg);
		Expr* newval = new UnaryExpression(log_not, neg->copy());

		// note: the resulting assignment must be a Conditional, i.e. of the form 'guard ? value : unkown'
		vars.push_back(new VarName(p->varname()));
		exprs.push_back(new Conditional(guard, newval, new Unknown()));
	}
	
	auto doc = new DocString("ParallelAssignment\n"); // mk_doc(*this);
	if (vars.size() > 1) return { doc, new ParallelAssignment(vars, exprs, this) };
	else if (vars.size() == 1) return { doc, new SimpleAssignment(vars[0], exprs[0], this) };
	else return { doc, new Skip(this) };
}

std::vector<Statement*> Assume::abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const {
	throw UnsupportedOperationError("Abstraction of assume statements is not supported.");
}

std::vector<Statement*> Assert::abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const {
	// assert(cond) <==> if (cond) { } else { assert(false); }
	// --> handle assert like an if
	auto posz3cond = _expr->z3(context);
	auto negz3cond = !posz3cond;

	auto pos = strongest_whatsoever(preds, posz3cond, solver);
	auto neg = strongest_whatsoever(preds, negz3cond, solver);

	auto asu_pos = new Assume(pos, _expr->copy());
	auto asu_neg = new Assume(neg, new UnaryExpression(log_not, _expr->copy()));
	auto abort = new Assert(new Literal(false), *this);
	auto ite = new Ite(new Unknown(), { asu_pos }, { asu_neg, abort });
	auto doc = mk_doc("assert", *_expr);
	return { doc, ite };
}

std::vector<Statement*> Skip::abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const {
	return {};
}

std::vector<Statement*> DocString::abstract(const std::vector<Predicate*> preds, z3::solver& solver, z3::context& context) const {
	return {};
}
