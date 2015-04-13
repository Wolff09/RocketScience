#include "symbolic/Reachability.hpp"

using namespace symbolic;


BDD symbolic::reachable(const StateTransitionSystem& sts, BDD reachable) {
	const BDD& trans = sts.transitionRelation();
	const std::vector<BDD> vars = sts.variables();

	// create BDD for quantification over current variables and substitution
	BDD quantify = sts.one();
	std::vector<BDD> compose;
	for (const auto &v : vars) {
		if (sts.isCurrentVariable(v)) quantify *= v;
		compose.push_back(sts.currentOf(v));
	}

	int counter = 0;
	// fixed point iteration
	BDD next = reachable;
	do {
		reachable = std::move(next);
		next += (trans * reachable).ExistAbstract(quantify).VectorCompose(compose);
		// next += trans.Constrain(reachable).ExistAbstract(quantify).VectorCompose(compose);
		counter++;
	}
	while (reachable != next);
	std::cout << "Iterations: " << counter << std::endl;

	return reachable;
}



BDD symbolic::reachable(ControlFlowGraph& cfg, const BDD init, const BDD bad, const bool init_call_frame) {
	/* TODO: Rel0
	 * Currently we use Rel0 := id * val.
	 * Maybe we should use Rel0 := id * Cofactor(val) to
	 * allow better reuse.
	 * The care set should be user defined. Hence there would
	 * also be a possibility for the user to define the laziness
	 * of the procedure summary analysis.
	 */

	BDD reach = init, prevreach;
	BDD sum = cfg.zero(), prevsum;
	BDD trans = cfg._trans, prevtrans;

	// precompute initial relation: identity on globals, i.e val(g) <-> mem(g)
	BDD rel0 = cfg.one();
	for (std::size_t i = 0; i < cfg._numGlobVars; i++)
		rel0 *= equal(cfg._globalVars[i], cfg._globalVarsRel[i]);
	if (init_call_frame)
		for (const auto &v : cfg._localVars)
			rel0 *= !v;

	BDD locId = cfg.one();
	for (std::size_t i = 0; i < cfg._numTempVars; i++)
		locId *= equal(cfg._localVars[i], cfg._localVarsPrimed[i]);

	// precompute formulas for quantification
	const BDD state = multiply(cfg.one(), {cfg._stateVars});
	const BDD state_and_pvar = multiply(cfg.one(), {cfg._stateVars, cfg._programVars});
	const BDD state_and_loc = multiply(cfg.one(), {cfg._stateVars, cfg._localVars});
	const BDD state_and_loc_and_rel = multiply(cfg.one(), {cfg._stateVars, cfg._localVars, cfg._globalVarsRel});

	// a priori compute vectors for composition
	const std::vector<BDD> unprime_state_and_pvar = concat({cfg._stateVars, cfg._programVars, cfg._stateVars, cfg._programVars, cfg._globalVarsRel});
	const std::vector<BDD> unprime_state_and_memorize_glob = concat({cfg._stateVars, cfg._globalVarsRel, cfg._localVars, cfg._stateVars, cfg._programVarsPrimed, cfg._globalVarsRel}); // s' -> s, g -> g''
	const std::vector<BDD> unprime_state_and_rel_to_guardedaction = concat({cfg._stateVars, cfg._programVarsPrimed, cfg._stateVars, cfg._programVarsPrimed, cfg._globalVars});

	assert(unprime_state_and_pvar.size() == cfg._vars.size());
	assert(unprime_state_and_memorize_glob.size() == cfg._vars.size());
	assert(unprime_state_and_rel_to_guardedaction.size() == cfg._vars.size());

	// shortcut for returning
	auto reach_bad = [&] () -> bool { return ((reach & bad) != cfg.zero()); };
	auto sum_bad = [&] () -> bool { return ((sum & bad) != cfg.zero()); };
	auto mk_return = [&] () -> BDD {
		cfg._trans = trans;
		return (reach | sum).ExistAbstract(multiply(cfg.one(), {cfg._globalVarsRel}));
	};

	// fixed point iteration(s)
	bool summaryEdgeAdded;
	do {
		/*output*///std::cout << "-- Iteration --" << std::endl;

		// regular reachability analysis: find all configuration reachable
		do {
			/*output*///std::cout << "\t- Reach" << std::endl;
			prevreach = reach;
			// follow transition relation (successor computation)
			reach += (reach * trans).ExistAbstract(state_and_pvar).VectorCompose(unprime_state_and_pvar);
			if (reach_bad()) return mk_return();
		} while (reach != prevreach);

		// follow calls from reachability analysis (calls from summary procedures are dealt with below)
		// initial relation is identiy guarded with source valuation (src val: x; rel val: x'')
		sum += (reach * cfg._calls).ExistAbstract(state_and_loc).VectorCompose(unprime_state_and_memorize_glob) * rel0;
		if (sum_bad()) return mk_return();

		// procedure summary analysis: compute input/output relation and add summary edges
		summaryEdgeAdded = false;
		do {
			/*output*///std::cout << "\t- Edge" << std::endl;
			do {
				/*output*///std::cout << "\t\t- Proc" << std::endl;
				prevsum = sum;
				// follow transition relation (successor computation)
				sum += (sum * trans).ExistAbstract(state_and_pvar).VectorCompose(unprime_state_and_pvar);
				if (sum_bad()) return mk_return();
				if (sum != prevsum) continue;
				// follow procedure calls (call valuation: x')
				sum += (sum * cfg._calls).ExistAbstract(state_and_loc_and_rel).VectorCompose(unprime_state_and_memorize_glob) * rel0;
				if (sum_bad()) return mk_return();
			} while (sum != prevsum);

			// add summary edges
			prevtrans = trans;
			trans += (sum * cfg._exits).ExistAbstract(state_and_loc).VectorCompose(unprime_state_and_rel_to_guardedaction) * cfg._returns * locId;
			summaryEdgeAdded = summaryEdgeAdded || (trans != prevtrans);
		} while (trans != prevtrans);

	} while (summaryEdgeAdded);
	// end: fixed point iteration(s)

	return mk_return();
}



std::vector<BDD> symbolic::find_path(const ControlFlowGraph& cfg, const BDD src, const BDD dst, const BDD reach, const BDD ignored_edges) {
	// TODO: revisit code documentation
	assert(src & reach != cfg.zero());
	assert(dst & reach != cfg.zero());

	// precompute some stuff
	const BDD trans = (cfg.transitionRelation() | cfg.callRelation()) & !ignored_edges;
	const BDD pre_quantifier = multiply(cfg.one(), {cfg.stateVariablesPrime(), cfg.programVariablesPrime()});
	const BDD post_quantifier = multiply(cfg.one(), {cfg.stateVariables(), cfg.programVariables()});
	const std::vector<BDD> pre_replace = concat({cfg.stateVariablesPrime(), cfg.programVariablesPrime(), cfg.stateVariablesPrime(), cfg.programVariablesPrime(), cfg.globalVariablesRel()});;
	const std::vector<BDD> post_replace = concat({cfg.stateVariables(), cfg.programVariables(), cfg.stateVariables(), cfg.programVariables(), cfg.globalVariablesRel()});
	const std::vector<BDD> mintermvars = concat({cfg.stateVariables(), cfg.programVariables()});

	// inline functions to allow reuse of precomputed enteties
	auto preimage = [&] (BDD bdd) -> BDD {
		return (bdd.VectorCompose(pre_replace) * trans).ExistAbstract(pre_quantifier);
	};
	auto postimage = [&] (BDD bdd) -> BDD {
		return (bdd * trans).ExistAbstract(post_quantifier).VectorCompose(post_replace);
	};
	auto single = [&] (BDD bdd) -> BDD {
		return bdd.PickOneMinterm(mintermvars);
	};

	// backward search to explore all k-step reachable states
	// bouded by k being the shortest path from src -> dst in reach
	// remember all explored states to avoid infinite search
	BDD explored = cfg.zero();
	BDD preexplored = cfg.zero();
	std::vector<BDD> ksteps2dst;
	ksteps2dst.push_back(dst);
	while ((ksteps2dst.back() & src) == cfg.zero()) {
		BDD pre = preimage(ksteps2dst.back());
		pre &= reach;
		ksteps2dst.push_back(pre);

		// if stabalized, there is no path
		preexplored = explored;
		explored += pre;
		if (preexplored == explored) return {};
	}

	// the shortest path src->dst has length k
	std::size_t k = ksteps2dst.size() - 1;

	// forward search from src throuhg k-step reachable states to identify (single) path of length k
	std::vector<BDD> path;
	path.push_back(single(src & ksteps2dst[k]));
	for (std::size_t i = 1; i <= k; i++) {
		BDD post = postimage(path[i-1]);
		post &= ksteps2dst[k-i]; // post can reach dst in k-i steps
		post = single(post);
		path.push_back(post);
	}

	return path;
}
