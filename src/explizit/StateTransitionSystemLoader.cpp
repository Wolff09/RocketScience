
#include <vector>
#include "explizit/StateTransitionSystemLoader.hpp"

using namespace std;
using namespace boost;
using namespace explizit;


// Bulding structure aspect

unique_ptr<StateTransitionSystem> StateTransitionSystemLoader::make() {
	vector<string>* cells;
	auto cell = [&] (size_t index) -> const string& {
		string& s = (*cells)[index];
		trim(s);
		return s;
	};
	while (_csv.hasNext()) {
		cells = _csv.getNext();

		State* src = getState(cell(0));
		State* dst = getState(cell(1));

		Transition* trans = src->makeTransition(dst);

		Formula* guard = getFormula(cell(2));
		trans->makeGuard(unique_ptr<Formula>(guard));

		for (size_t i = 3; i < cells->size(); i+=2) {
			Variable* var = getVariable(cell(i));
			Formula* action = getFormula(cell(i+1));
			trans->makeAction(var, unique_ptr<Formula>(action));
		}
	}
	return move(_sys);
}


// Parsing aspect

TokenType StateTransitionSystemLoader::getCurrentTokenType() const {
	if (_tokens[_position].front() == '(' && _tokens[_position].back() == ')') {
		if (_tokens[_position] == "(!)") return TokenType::NOT;
		else if (_tokens[_position] == "(&)") return TokenType::AND;
		else if (_tokens[_position] == "(|)") return TokenType::OR;
		else if (_tokens[_position] == "(->)") return TokenType::IMP;
		else if (_tokens[_position] == "(^)") return TokenType::XOR;
		else if (_tokens[_position] == "(<->)") return TokenType::EQUIV;
		else if (_tokens[_position] == "(=)") return TokenType::EQUIV;
		else if (_tokens[_position] == "(==)") return TokenType::EQUIV;
		else if (_tokens[_position] == "(?)") return TokenType::ITE;
		else if (_tokens[_position] == "(true)") return TokenType::FTRUE;
		else if (_tokens[_position] == "(T)") return TokenType::FTRUE;
		else if (_tokens[_position] == "(false)") return TokenType::FFALSE;
		else if (_tokens[_position] == "(F)") return TokenType::FFALSE;
		else assert(false);
	}
	else return TokenType::VARIABLE;
}

Formula* StateTransitionSystemLoader::parseFormula() {
	switch(getCurrentTokenType()) {
		case TokenType::NOT: return new NotFormula(parseNextFormula());
		case TokenType::AND: return new AndFormula(parseNextFormula(), parseNextFormula());
		case TokenType::OR: return new OrFormula(parseNextFormula(), parseNextFormula());
		case TokenType::IMP: return new ImpFormula(parseNextFormula(), parseNextFormula());
		case TokenType::XOR: return new XOrFormula(parseNextFormula(), parseNextFormula());
		case TokenType::EQUIV: return new EquivFormula(parseNextFormula(), parseNextFormula());
		case TokenType::ITE: return new ITEFormula(parseNextFormula(), parseNextFormula(), parseNextFormula());
		case TokenType::FTRUE: return new TrueFormula();
		case TokenType::FFALSE: return new FalseFormula();
		case TokenType::VARIABLE: return new Literal(getVariable(_tokens[_position]));
	}
	assert(false);
}
