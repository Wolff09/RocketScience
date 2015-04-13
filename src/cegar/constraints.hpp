#pragma once

#include "ast/ast.hpp"


namespace cegar {


	std::vector<ast::Expr*> compute_constraints(const std::vector<const ast::TraceableStatement*>& trace);


}