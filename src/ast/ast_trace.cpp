#include "ast/ast.hpp"
#include "ast/trace.hpp"

using namespace ast;



std::vector<const TraceableStatement*> Assignment::flat_trace(const Program& abstract, const symbolic::ControlFlowGraph& cfg, const BDD preconf, const BDD postconf, const BDD bounds, const BDD ignored_edges) const {
	assert(_trace_stmt != NULL);
	return { _trace_stmt };
}

std::vector<const TraceableStatement*> AssBase::flat_trace(const Program& abstract, const symbolic::ControlFlowGraph& cfg, const BDD preconf, const BDD postconf, const BDD bounds, const BDD ignored_edges) const {
	assert(_trace_stmt.get() != NULL);
	return { _trace_stmt.get() };
}

std::vector<const TraceableStatement*> Call::flat_trace(const Program& abstract, const symbolic::ControlFlowGraph& cfg, const BDD preconf, const BDD postconf, const BDD bounds, const BDD ignored_edges) const {
	assert(cfg.decode(preconf) == _cfg_call->call());
	assert(cfg.decode(postconf) == _cfg_call->retrn());
	// TODO: add call/return statements to trace!
	auto result = _decl->flat_trace(abstract, cfg, preconf, postconf, bounds, ignored_edges);
	result.insert(result.begin(), this);
	result.push_back(_trace_return.get());
	return result;
}

std::vector<const TraceableStatement*> FunDef::flat_trace(const Program& abstract, const symbolic::ControlFlowGraph& cfg, const BDD callconf, const BDD returnconf, const BDD bounds, const BDD edgeignore) const {
	// remove summary edge taken for the path being currently handled
	// -> this prevents looping/infinite Hoare Proofs
	// this is the only sufficient solution, i.e. just pruning some configuration is not sufficient,
	// since procedures of the abstracted program are potentially non-deterministically and thus
	// only the call-return relation (summary edge) properly characterises the function sequence
	const auto np2p = symbolic::concat({cfg.stateVariablesPrime(), cfg.programVariablesPrime(), cfg.stateVariablesPrime(), cfg.programVariablesPrime(), cfg.globalVariablesRel()});
	auto edge_taken = callconf * returnconf.VectorCompose(np2p);
	assert((cfg.transitionRelation() & edge_taken) != cfg.zero());
	auto ignored_edges = edgeignore + edge_taken;

	// prepare variable removal to avoid overhead
	auto nonglobvars = symbolic::multiply(cfg.one(), {cfg.stateVariables(), cfg.localVariables()});
	auto globalval = [&] (const BDD bdd) -> BDD {
		return bdd.ExistAbstract(nonglobvars);
	};

	// extract global pre/post state of call
	BDD val_in = globalval(callconf);
	BDD val_out = globalval(returnconf);

	// compute procedure entry/exit points according to summary edge for callconf/returnconf
	BDD src = cfg.encode(_cfg_proc->entry()) * val_in;
	BDD dst = cfg.encode(_cfg_proc->exit()) * val_out;
	assert((src & bounds) != cfg.zero());
	assert((dst & bounds) != cfg.zero());

	// generate flat "sub" trace
	auto trace = ast::flat_trace(abstract, cfg, src, dst, bounds, ignored_edges);
	assert(trace.size() > 0);
	return trace;
}
