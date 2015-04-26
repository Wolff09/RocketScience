#include "cegar/cegar.hpp"

#include <iostream>
#include <boost/range/adaptor/reversed.hpp>
#include "ast/ast.hpp"
#include "ast/trace.hpp"
#include "parser/parser.hpp"
#include "ast/abstraction_utils.hpp"
#include "symbolic/Reachability.hpp"
#include "cegar/constraints.hpp"
#include "cegar/interpolate.hpp"
#include <sys/time.h>

using namespace cegar;


#define INIT symbolic::Main(0).block()
#define FAIL symbolic::Node(symbolic::BLOCK, 0)
#define HOARE_GOAL_FALSE new ast::Literal(false)


/******************************************************************************
	Helpers
 ******************************************************************************/

ast::Program* load_program(const std::string filename) {
	auto progstream = parser::open_file(filename);
	ast::Program* prog = parser::parse_program(progstream);
	prog->add_initializers();
	prog->validate();
	return prog;
}


bool is_spurious(ast::Program& prog, const std::vector<const ast::TraceableStatement*>& trace) {
	auto wp = std::unique_ptr<ast::Expr>(new ast::Literal(false));
	for (auto stmt : boost::adaptors::reverse(trace))
		wp.reset(stmt->wp(*wp));

	z3::context context;
	z3::solver solver(context);

	return ast::is_taut(solver, wp->z3(context));
}


void refine_predicates(ast::PredicateList& preds, const ast::Program& prog, const std::vector<const ast::TraceableStatement*>& trace) {
	std::vector<ast::Expr*> constraints = compute_constraints(trace);
	assert(constraints.size() == trace.size());

	/*output*/std::cout << "/**************** BEGIN TRACE ****************/" << std::endl;
	for (std::size_t i = 0; i < trace.size(); i++) {
		/*output*/trace.at(i)->prettyprint(std::cout, 1);
		/*output*///std::cout << "          * ";
		/*output*///constraints.at(i)->prettyprint(std::cout);
		/*output*///std::cout << std::endl;
	}
	/*output*/std::cout << "/***************** END TRACE *****************/" << std::endl;
	/*output*/std::cout << std::endl;

	std::vector<ast::Expr*> interpolants = compute_interpolants(prog, trace, constraints);
	assert(constraints.size() == interpolants.size() + 1);

	std::vector<ast::Expr*> newones;
	for (ast::Expr* e : interpolants) {
		ast::Expr* post = e->postprocess_interpolant(prog);
		delete e;
		post->collect_potential_predicates(newones);
		delete post;
	}
	interpolants.clear();

	/*output*/std::cout << "/************** BEGIN REFINEMENT *************/" << std::endl;
	for (ast::Expr* e : newones) {
		assert(e->is_well_scoped());
		const ast::FunDef* scope = e->scope();
		std::string scopename = scope == NULL ? "global" : scope->name();

		bool added = preds.extend(new ast::Predicate(e), scopename, true);
		if (added) {
			std::cout << "-- new predicate [" << scopename << "] ";
			e->prettyprint(std::cout);
			std::cout << std::endl;
		}
	}
	/*output*/std::cout << "/*************** END REFINEMENT **************/" << std::endl;

	preds.validate(prog);
}


/******************************************************************************
	CEGAR Loop
 ******************************************************************************/

bool cegar::prove(std::string filename) {
	auto clk_begin = clock();

	std::unique_ptr<ast::Program> program, abstract;
	std::unique_ptr<ast::PredicateList> predicates;
	std::unique_ptr<symbolic::ControlFlowGraph> cfg; 

	program.reset(load_program(filename));
	predicates.reset(new ast::PredicateList({}));

	program->prettyprint(std::cout);

	// CEGAR loop
	std::size_t loop_count = 1;
	while (true) {
		/*output*/std::cout << std::endl << std::endl << "======================================================================";
		/*output*/std::cout << std::endl << "============================= Loop No. " << loop_count << " =============================";
		/*output*/std::cout << std::endl << "======================================================================" << std::endl << std::endl;

		// STEP 1: abstract
		abstract.reset(program->abstract(*predicates));
		cfg.reset(abstract->cfg());

		/*output*/predicates->prettyprint(std::cout);
		/*output*/std::cout << std::endl;
		/*output*/abstract->prettyprint(std::cout);
		/*output*/std::cout << std::endl;

		// STEP 2: reachability analysis; search for counterexample
		BDD init = cfg->encode(INIT);
		BDD bad = cfg->encode(FAIL);
		BDD reachset = symbolic::reachable(*cfg, init, bad);
		if ((reachset & bad) == cfg->zero()) {
			/*output*/std::cout << std::endl;
			/*output*/std::cout << "   +---------------------------+" << std::endl;
			/*output*/std::cout << "   | Your programm is CORRECT! |" << std::endl;
			/*output*/std::cout << "   +---------------------------+" << std::endl;
			/*output*/std::cout << std::endl;
			/*output*/std::cout << "Total Time Taken: " << (clock()-clk_begin)/1000/1000.0 << "s" << std::endl;
			return true; // if no bad state is reachable, we are done
		}

		// STEP 3: check if counterexample is spurious
		auto trace = ast::flat_trace(*abstract, *cfg, init, bad, reachset);
		if (!is_spurious(*program, trace)) {
			/*output*/std::cout << std::endl;
			/*output*/std::cout << "   +-------------------------+" << std::endl;
			/*output*/std::cout << "   | Your programm is WRONG! |" << std::endl;
			/*output*/std::cout << "   +-------------------------+" << std::endl;
			/*output*/std::cout << std::endl;
			/*output*/std::cout << "Total Time Taken: " << (clock()-clk_begin)/1000/1000.0 << "s" << std::endl;
			return false;
		}

		// STEP 4: refine abstraction
		trace.pop_back(); // no need to delete pointer -> owned by abstract program
		auto num_preds = predicates->size();
		refine_predicates(*predicates, *program, trace);
		if (predicates->size() == num_preds) {
			/*output*/std::cout << std::endl;
			/*output*/std::cout << "   +----------------------+" << std::endl;
			/*output*/std::cout << "   | Sorry, CEGAR failed! |" << std::endl;
			/*output*/std::cout << "   +----------------------+" << std::endl;
			/*output*/std::cout << std::endl;
			/*output*/std::cout << "Total Time Taken: " << (clock()-clk_begin)/1000/1000.0 << "s" << std::endl;
			assert(false);
		}

		loop_count++;
		if (loop_count > 20) break; // don't let it run forever
	}
	
	cfg.reset();
	abstract.reset();
	predicates.reset();
	program.reset();
	assert(false);
}
