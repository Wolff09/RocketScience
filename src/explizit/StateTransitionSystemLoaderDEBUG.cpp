
#include <vector>
#include "explizit/StateTransitionSystemLoader.hpp"

using namespace std;
using namespace boost;
using namespace explizit;


// Bulding structure aspect

unique_ptr<StateTransitionSystem> StateTransitionSystemLoader::make() {
	cout << "StateTransitionSystem.make()" << endl;
	vector<string>* cells;
	cout << "StateTransitionSystem.make() created cells-vector" << endl;
	auto cell = [&] (size_t index) -> const string& {
		string& s = (*cells)[index];
		cout << "StateTransitionSystem.make() lambda at cell " << index << " with string s: " << s << endl;
		trim(s);
		cout << "StateTransitionSystem.make() lambda at cell " << index <<" with trimmed string s: " << s << endl;
		return s;
	};
	cout << "StateTransitionSystem.make() defined lambda" << endl;
	while (_csv.hasNext()) {
		cout << "StateTransitionSystem.make() while()" << endl;
		cells = _csv.getNext();
		cout << "StateTransitionSystem.make() got next CSV-Line" << endl;
		State* src = getState(cell(0));
		cout << "StateTransitionSystem.make() got new src" << src->name() << endl;
		State* dst = getState(cell(1));
		cout << "StateTransitionSystem.make() got new dst" << dst->name() << endl;

		Transition* trans = src->makeTransition(dst);
		cout << "StateTransitionSystem.make() got new transition" << endl;

		Formula* guard = getFormula(cell(2));
		cout << "StateTransitionSystem.make() got new guard" << endl;
		trans->makeGuard(unique_ptr<Formula>(guard));
		cout << "StateTransitionSystem.make() set the new guard" << endl;

		cout << "StateTransitionSystem.make() while() for() start loop with size " << cells->size() << endl;
		for (size_t i = 3; i < cells->size(); i+=2) {
			cout << "StateTransitionSystem.make() while() for()" << endl;
			Variable* var = getVariable(cell(i));
			cout << "StateTransitionSystem.make() got new variable for action" << endl;
			Formula* action = getFormula(cell(i+1));
			cout << "StateTransitionSystem.make() got new formula for action" << endl;
			trans->makeAction(var, unique_ptr<Formula>(action));
			cout << "StateTransitionSystem.make() set the new action" << endl;
		}
		cout << "StateTransitionSystem.make() while() for() END" << endl;
	}
	cout << "StateTransitionSystem.make() while() END" << endl;
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
