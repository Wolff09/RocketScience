#include "ast/ast.hpp"

#include "ast/abstraction_utils.hpp"

using namespace ast;


bool PredicateList::contains(const Predicate& pred, std::string scope) const {
	z3::context ctx;
	z3::solver solver(ctx);
	z3::expr p3 = pred.expr()->z3(ctx);

	if (is_taut(solver, p3)) return true;
	if (is_taut(solver, !p3)) return true;

	for (const Predicate* c : preds_for("global"))
		if (equals(solver, c->expr()->z3(ctx), p3))
			return true;

	for (const Predicate* c : preds_for(scope))
		if (equals(solver, c->expr()->z3(ctx), p3))
			return true;

	return false;
}

bool PredicateList::extend(Predicate* pred, std::string scope, bool check_for_duplicate) {
	if (check_for_duplicate && contains(*pred, scope)) {
		delete pred;
		return false;
	} else {
		_ownership.push_back(std::unique_ptr<Predicate>(pred));
		_name2pred[scope].push_back(_ownership.back().get());
		return true;
	}
}
