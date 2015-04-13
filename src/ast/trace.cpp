#include <set>
#include "ast/trace.hpp"
#include "symbolic/Reachability.hpp"


using namespace ast;



std::vector<std::pair<const TraceableStatement*, std::pair<BDD, BDD>>> ast::extract_trace(const Program& abstract, const symbolic::ControlFlowGraph& cfg, const BDD src, const BDD dst, const BDD bounds, const BDD ignored_edges) {
	std::vector<BDD> bdd_path = symbolic::find_path(cfg, src, dst, bounds, ignored_edges);
	if (bdd_path.size() == 0) return {};
	
	// translate bdds to symbolic::Nodes
	std::vector<symbolic::Node> node_path;
	for (BDD b : bdd_path)
		node_path.push_back(cfg.decode(b));

	// find all distince entry nodes appearing in node_path
	std::set<symbolic::Node> entries;
	for (auto node : node_path)
		if (node.type == symbolic::ENTRY)
			entries.insert(node);

	// collect mapping for all statemtents that possibly appear in trace
	std::map<std::pair<symbolic::Node, symbolic::Node>, const TraceableStatement*> srcdst2stmt;
	for (auto node : entries)
		abstract.entry2fun(node)->collect_cfg_transitions(srcdst2stmt);

	// translate node path to statement sequence
	std::vector<std::pair<const TraceableStatement*, std::pair<BDD, BDD>>> result;
	for (std::size_t i = 0; i < node_path.size()-1; i++) {
		auto key = std::make_pair(node_path[i], node_path[i+1]);
		const TraceableStatement* stmt = srcdst2stmt[key]; // don't use map::at(); we want NULL as default value
		auto trans = std::make_pair(bdd_path[i], bdd_path[i+1]);
		if (stmt != NULL) result.push_back(std::make_pair(stmt, trans));
	}

	return result;
}



std::vector<const TraceableStatement*> ast::flat_trace(const Program& abstract, const symbolic::ControlFlowGraph& cfg, const BDD src, const BDD dst, const BDD bounds, const BDD ignored_edges) {
	// TODO: inline ast::extract_trace
	std::vector<const TraceableStatement*> result;
	auto trace = extract_trace(abstract, cfg, src, dst, bounds, ignored_edges);
	for (const auto& e : trace) {
		const TraceableStatement* stmt = e.first;
		const auto preconf = e.second.first;
		const auto postconf = e.second.second;
		auto subtrace = stmt->flat_trace(abstract, cfg, preconf, postconf, bounds, ignored_edges);
		std::copy(subtrace.begin(), subtrace.end(), std::back_inserter(result));
	}
	return result;
}