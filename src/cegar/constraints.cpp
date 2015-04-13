#include <stack>
#include "cegar/constraints.hpp"


using namespace cegar;


std::vector<ast::Expr*> cegar::compute_constraints(const std::vector<const ast::TraceableStatement*>& trace) {
	// push/pop current map when call/return found
	std::vector<ast::Expr*> result;

	std::stack<std::map<const ast::VarDef*, std::size_t>> maps;
	maps.push(std::map<const ast::VarDef*, std::size_t>());

	for (const ast::TraceableStatement* stmt : trace) {
		auto& lvm = maps.top();

		// pop/push lvalue maps when handling call/return to properly handle function calls
		if (dynamic_cast<const ast::Call*>(stmt)) maps.push(std::map<const ast::VarDef*, std::size_t>(lvm));
		else if (dynamic_cast<const ast::Return*>(stmt)) {
			auto top = maps.top();
			maps.pop();
			// global variables should remain in the stack
			for (const auto& e : top)
				if (e.first->function() == NULL)
					maps.top()[e.first] = e.second;
		}

		result.push_back(stmt->con(lvm));
	}

	return result;
}
