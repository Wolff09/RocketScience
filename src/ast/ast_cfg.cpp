#include "ast/ast.hpp"

using namespace ast;


/******************************************************************************
	HELPERS
 ******************************************************************************/

#define ASSERT_FAIL_BLOCK symbolic::Node(symbolic::BLOCK, 0)

class TemporaryNode {
	// owns an object which can be set, referenced and released (claim ownership)
	private:
		std::unique_ptr<symbolic::Node> _node;
	public:
		TemporaryNode() {}
		TemporaryNode(symbolic::Node&& n) { _node.reset(new symbolic::Node(std::move(n))); }
		TemporaryNode(symbolic::Node& n) { _node.reset(new symbolic::Node(n)); }
		TemporaryNode& operator=(symbolic::Node&& n) { _node.reset(new symbolic::Node(std::move(n))); return *this; }
		TemporaryNode& operator=(symbolic::Node& n) { _node.reset(new symbolic::Node(n)); return *this; }
		symbolic::Node& operator*() { return *_node; }
		symbolic::Node* operator&() { return _node.release(); }
};

symbolic::Node* mk_Block(std::size_t id) {
	return new symbolic::Node(symbolic::BLOCK, id);
}

bool contains_bdd(const std::vector<BDD>& vars, const BDD b) {
	for (const auto v : vars)
		if (v == b) return true;
	return false;
}

BDD keep_all_vars_but(const symbolic::ControlFlowGraph& cfg, const std::vector<BDD>& vars) {
	//std::cout << "KEEPALL_VARS_BUT(cfg, set): " << vars.size() << std::endl;
	BDD res = cfg.one();
	for (std::size_t i = 0; i < cfg.programVariables().size(); i++) {
		BDD v = cfg.programVariables().at(i);
		BDD p = cfg.programVariablesPrime().at(i);
		if (contains_bdd(vars, v)) continue;
		res &= symbolic::equal(v, p);
	}
	return res;
}

BDD keep_all_vars(const symbolic::ControlFlowGraph& cfg) {
	std::vector<BDD> empty;
	return keep_all_vars_but(cfg, empty);
}

#define KEEP_ALL keep_all_vars(cfg)

BDD assignment2bdd(const symbolic::ControlFlowGraph& cfg, const VarName& v, const Expr& e) {
	const Conditional* ec = dynamic_cast<const Conditional*>(&e);
	assert(ec != NULL);
	assert(dynamic_cast<const Unknown*>(ec->low()) != NULL);

	BDD var = v.decl()->cfg(cfg, false);
	BDD pri = v.decl()->cfg(cfg, true);
	BDD cnd = ec->guard()->cfg(cfg);
	BDD act = ec->high()->cfg(cfg);
	BDD ass = symbolic::equal(pri, act);
	return ass | !cnd; // <==> (cnd & ass) | !cnd
}


/******************************************************************************
	CFG
 ******************************************************************************/

symbolic::ControlFlowGraph* Program::cfg() {
	std::size_t numVars = 0;
	std::size_t numGlob , numLoc;
	// block0 = blockAssertFail
	// main0 = start
	// main1 = stop
	// call0 = call to main procedure/function -> treat main as regular function
	std::size_t numMains = 2, numBlocks = 1;
	std::size_t numProcs = 0, numCalls = 1;

	// pass 1: preparation -> create nodes
	for (auto& v : _vars)
		v->cfg_pass_one(numVars);
	numGlob = numVars;

	// TODO: local variables --> momentan hat jede lokale variable eine eigenen variable im CFG --> muss nicht sein, wir brauchen lediglich genügend im cfg
	for (auto& f : _funs)
		f->cfg_pass_one(numVars, numBlocks, numProcs, numCalls);
	numLoc = numVars - numGlob;

	// create cfg
	symbolic::ControlFlowGraph* cfg = new symbolic::ControlFlowGraph(numMains, numBlocks, numProcs, numCalls, numGlob, numLoc);

	// connect main nodes of cfg to main function of program
	BDD keep_globals = keep_all_vars_but(*cfg, cfg->localVariables());
	cfg->addTransition(symbolic::Main(0).block(), symbolic::Call(0).call(), keep_globals);
	cfg->addCall(symbolic::Call(0), *(_name2fun.at("main")->cfg_procedure()));
	cfg->addTransition(symbolic::Call(0).retrn(), symbolic::Main(1).block(), keep_globals);

	// pass 2: translation -> add transitions
	for (auto& f : _funs)
		f->cfg_pass_two(*cfg);

	return cfg;
}

BDD VarDef::cfg(const symbolic::ControlFlowGraph& cfg, bool primed) const {
	if (primed) return cfg.programVariablesPrime().at(cfgid());
	else return cfg.programVariables().at(cfgid());
}


/******************************************************************************
	CFG_PASS_ONE
 ******************************************************************************/

void VarDef::cfg_pass_one(std::size_t& index) {
	_cfg_id = index++;
}

void FunDef::cfg_pass_one(std::size_t& numVars, std::size_t& numBlocks, std::size_t& numProcs, std::size_t& numCalls) {
	_cfg_proc.reset(new symbolic::Procedure(numProcs++));

	for (auto& v : _vars)
		v->cfg_pass_one(numVars);

	TemporaryNode pre = _cfg_proc->entry();
	for (auto& s : _stmts)
		pre = s->cfg_pass_one(numBlocks, numCalls, numProcs, *pre);
	_cfg_last.reset(&pre);
}

symbolic::Node Statement::cfg_pass_one(std::size_t& numNodes, std::size_t& numCalls, std::size_t& numProcs, symbolic::Node& pre) {
	_cfg_pre.reset(new symbolic::Node(pre));
	return pre;
}

symbolic::Node While::cfg_pass_one(std::size_t& numNodes, std::size_t& numCalls, std::size_t& numProcs, symbolic::Node& pre) {
	assert(dynamic_cast<Unknown*>(_cond.get()) != NULL);
	Statement::cfg_pass_one(numNodes, numCalls, numProcs, pre);

	TemporaryNode node = pre;
	for (auto& s : _stmts)
		node = s->cfg_pass_one(numNodes, numCalls, numProcs, *node);
	_cfg_body_post.reset(&node);

	_cfg_post.reset(mk_Block(numNodes++));
	return *_cfg_post;
}

symbolic::Node Ite::cfg_pass_one(std::size_t& numNodes, std::size_t& numCalls, std::size_t& numProcs, symbolic::Node& pre) {
	assert(dynamic_cast<Unknown*>(_cond.get()) != NULL);
	Statement::cfg_pass_one(numNodes, numCalls, numProcs, pre);

	TemporaryNode node = pre;
	for (auto& s : _if)
		node = s->cfg_pass_one(numNodes, numCalls, numProcs, *node);
	_cfg_if_post.reset(&node);

	node = pre;
	for (auto& s : _else)
		node = s->cfg_pass_one(numNodes, numCalls, numProcs, *node);
	_cfg_else_post.reset(&node);

	_cfg_post.reset(mk_Block(numNodes++));
	return *_cfg_post;
}

symbolic::Node Call::cfg_pass_one(std::size_t& numNodes, std::size_t& numCalls, std::size_t& numProcs, symbolic::Node& pre) {
	Statement::cfg_pass_one(numNodes, numCalls, numProcs, pre);
	_cfg_call.reset(new symbolic::Call(numCalls++));
	return _cfg_call->retrn();
}

symbolic::Node Assignment::cfg_pass_one(std::size_t& numNodes, std::size_t& numCalls, std::size_t& numProcs, symbolic::Node& pre) {
	Statement::cfg_pass_one(numNodes, numCalls, numProcs, pre);
	_cfg_post.reset(mk_Block(numNodes++));
	return *_cfg_post;
}

symbolic::Node AssBase::cfg_pass_one(std::size_t& numNodes, std::size_t& numCalls, std::size_t& numProcs, symbolic::Node& pre) {
	Statement::cfg_pass_one(numNodes, numCalls, numProcs, pre);
	_cfg_post.reset(mk_Block(numNodes++));
	/*output*///std::cout << "AssBase::cfg_pass_one" << "##post=" << _cfg_post->tostr() << std::endl;
	return *_cfg_post;
}

/******************************************************************************
	CFG_PASS_TWO
 ******************************************************************************/

void FunDef::cfg_pass_two(symbolic::ControlFlowGraph& cfg) const {
	for (const auto& s : _stmts)
		s->cfg_pass_two(cfg);
	
	cfg.addTransition(*_cfg_last, _cfg_proc->exit(), KEEP_ALL);
}

void While::cfg_pass_two(symbolic::ControlFlowGraph& cfg) {
	assert(dynamic_cast<Unknown*>(_cond.get()) != NULL);
	cfg.addTransition(*_cfg_pre, *_cfg_post, KEEP_ALL);
	cfg.addTransition(*_cfg_body_post, *_cfg_pre, KEEP_ALL);
	cfg.addTransition(*_cfg_body_post, *_cfg_post, KEEP_ALL);

	// substatements connect to _cfg_pre
	for (const auto& s : _stmts)
		s->cfg_pass_two(cfg);
}

void Ite::cfg_pass_two(symbolic::ControlFlowGraph& cfg) {
	assert(dynamic_cast<Unknown*>(_cond.get()) != NULL);
	/*output*///std::cout << "Ite::cfg_pass_two" << std::endl;

	cfg.addTransition(*_cfg_if_post, *_cfg_post, KEEP_ALL);
	cfg.addTransition(*_cfg_else_post, *_cfg_post, KEEP_ALL);
	
	// substatements connect to _cfg_pre
	for (const auto& s : _if)
		s->cfg_pass_two(cfg);
	for (const auto& s : _else)
		s->cfg_pass_two(cfg);
}

void Call::cfg_pass_two(symbolic::ControlFlowGraph& cfg) {
	/*output*///std::cout << "Call::cfg_pass_two" << std::endl;
	cfg.addTransition(*_cfg_pre, _cfg_call->call(), KEEP_ALL); // one more transition for the sake of simplicity
	cfg.addCall(*_cfg_call, *(_decl->cfg_procedure()));
}

void ParallelAssignment::cfg_pass_two(symbolic::ControlFlowGraph& cfg) {
	/*output*///std::cout << "ParallelAssignment::cfg_pass_two: " << _vars.size() << std::endl;

	std::vector<BDD> but;
	for (const auto& v : _vars) but.push_back(v->cfg(cfg));
	BDD keep_remaining = keep_all_vars_but(cfg, but);

	BDD assignment = cfg.one();
	for (std::size_t i = 0; i < _vars.size(); i++)
		assignment &= assignment2bdd(cfg, *_vars.at(i), *_exprs.at(i));

	cfg.addTransition(*_cfg_pre, *_cfg_post, assignment & keep_remaining);
}

void SimpleAssignment::cfg_pass_two(symbolic::ControlFlowGraph& cfg) {
	/*output*///std::cout << "SimpleAssignment::cfg_pass_two" << std::endl;
	BDD assignment = assignment2bdd(cfg, *_var, *_expr);
	BDD keep_remaining = keep_all_vars_but(cfg, { _var->cfg(cfg) });
	cfg.addTransition(*_cfg_pre, *_cfg_post, assignment & keep_remaining);
}

void AssBase::cfg_pass_two(symbolic::ControlFlowGraph& cfg) {
	/*output*///std::cout << "AssBase::cfg_pass_two" << std::endl;
	BDD cond = _expr->cfg(cfg);
	cfg.addTransition(*_cfg_pre, *_cfg_post, cond & KEEP_ALL);
}

void Assert::cfg_pass_two(symbolic::ControlFlowGraph& cfg) {
	AssBase::cfg_pass_two(cfg);
	/*output*///std::cout << "Assert::cfg_pass_two" << std::endl;
	BDD cond = _expr->cfg(cfg);
	cfg.addTransition(*_cfg_pre, ASSERT_FAIL_BLOCK, !cond & KEEP_ALL);
}

void Skip::cfg_pass_two(symbolic::ControlFlowGraph& cfg) {
	cfg.addTransition(*_cfg_pre, *_cfg_post, KEEP_ALL);
}

void DocString::cfg_pass_two(symbolic::ControlFlowGraph& cfg) {}


/******************************************************************************
	EXPRESSIONS
 ******************************************************************************/

BDD Conditional::cfg(const symbolic::ControlFlowGraph& cfg) const {
	BDD condbdd = _cond->cfg(cfg);
	BDD ifbdd = _if->cfg(cfg);
	BDD elsebdd = _else->cfg(cfg);
	// condbdd ? ifbdd : elsebdd
	// <==> (condbdd -> ifbdd) & (!condbdd -> elsebdd)
	// <==> (!condbdd | ifbdd) & (condbdd | elsebdd)
	BDD res = (!condbdd | ifbdd) & (condbdd | elsebdd);
	return res;
}

BDD UnaryExpression::cfg(const symbolic::ControlFlowGraph& cfg) const {
	if (_op == ari_neg) throw UnsupportedOperationError("Only boolean expressions are allowed.");
	else if (_op == log_not) return !_child->cfg(cfg);
	else assert(false);
}

BDD BinaryExpression::cfg(const symbolic::ControlFlowGraph& cfg) const {
	if (_op == ari_plus) throw UnsupportedOperationError("Only boolean expressions are allowed.");
	else if (_op == ari_minus) throw UnsupportedOperationError("Only boolean expressions are allowed.");
	else if (_op == ari_mult) throw UnsupportedOperationError("Only boolean expressions are allowed.");
	else if (_op == ari_div) throw UnsupportedOperationError("Only boolean expressions are allowed.");
	else if (_op == log_or) return _left->cfg(cfg) | _right->cfg(cfg);
	else if (_op == log_and) return _left->cfg(cfg) & _right->cfg(cfg);
	else if (_op == cmp_lt) throw UnsupportedOperationError("Only boolean expressions are allowed.");
	else if (_op == cmp_lte) throw UnsupportedOperationError("Only boolean expressions are allowed.");
	else if (_op == cmp_gt) throw UnsupportedOperationError("Only boolean expressions are allowed.");
	else if (_op == cmp_gte) throw UnsupportedOperationError("Only boolean expressions are allowed.");
	else if (_op == cmp_eq) throw UnsupportedOperationError("Only boolean expressions are allowed.");
	else if (_op == cmp_neq) throw UnsupportedOperationError("Only boolean expressions are allowed.");
	else assert(false);
}

BDD Literal::cfg(const symbolic::ControlFlowGraph& cfg) const {
	if (_type != bool_t) throw UnsupportedOperationError("Only boolean literals are allowed.");
	else if (_value == "true") return cfg.one();
	else if (_value == "false") return cfg.zero();
	else assert(false);
}

BDD VarName::cfg(const symbolic::ControlFlowGraph& cfg) const {
	return _decl->cfg(cfg);
}

BDD Unknown::cfg(const symbolic::ControlFlowGraph& cfg) const {
	throw UnsupportedOperationError("The 'unknown' expression cannot be translated into a BDD.");
}

BDD SymbolicConstant::cfg(const symbolic::ControlFlowGraph& cfg) const {
	assert(false);
}


/******************************************************************************
	TRANSITION TO STATEMENT
 ******************************************************************************/

void FunDef::collect_cfg_transitions(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection) const {
	for (const auto& s : _stmts)
		s->collect_cfg_transitions(collection);
}

void While::collect_cfg_transitions(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection) const {
	// cfg.addTransition(*pre, *_cfg_body_pre, c & keep_all);
	// cfg.addTransition(*pre, *_cfg_body_post, !c & keep_all);
	// cfg.addTransition(*_cfg_body_post, *pre, keep_all);
	// just collect the sub-statements; when a trace is inspected the commands from the body will appear anyway...
	for (const auto& s : _stmts)
		s->collect_cfg_transitions(collection);
}

void Ite::collect_cfg_transitions(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection) const {
	// cfg.addTransition(*pre, *_cfg_if_pre, c & keep_all);
	// cfg.addTransition(*pre, *_cfg_else_pre, !c & keep_all);
	// cfg.addTransition(*_cfg_if_post, *_cfg_post, keep_all);
	// cfg.addTransition(*_cfg_else_post, *_cfg_post, keep_all);
	// just collection the sub-statements; when a trace is inspected the commands from the corresponding branch will appear anyway...
	for (const auto& s : _if)
		s->collect_cfg_transitions(collection);
	for (const auto& s : _else)
		s->collect_cfg_transitions(collection);
}

void exc(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection, symbolic::Node src, symbolic::Node dst, const TraceableStatement* stmt) {
	auto key = std::make_pair(src, dst);
	assert(collection.count(key) == 0);
	collection[key] = stmt;
}

void Call::collect_cfg_transitions(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection) const {
	// cfg.addCall(*_cfg_call, *_cfg_proc);
	// the call needs to be expanded into an parallel assignment representing the procedure summary edge taken
	exc(collection, _cfg_call->call(), _cfg_call->retrn(), this);
}

void Assignment::collect_cfg_transitions(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection) const {
	exc(collection, *_cfg_pre, *_cfg_post, this);
}

void AssBase::collect_cfg_transitions(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection) const {
	// cfg.addTransition(*pre, *_cfg_post, cond & keep_all);
	exc(collection, *_cfg_pre, *_cfg_post, this);
}

void Assert::collect_cfg_transitions(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection) const {
	// cfg.addTransition(*pre, ASSERT_FAIL_BLOCK, !cond & keep_all);
	exc(collection, *_cfg_pre, ASSERT_FAIL_BLOCK, this);
}

void DocString::collect_cfg_transitions(std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*>& collection) const {}
