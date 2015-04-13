
#pragma once

#include <map>
#include <memory>
#include <string>
#include <boost/algorithm/string.hpp>
#include "explizit/SimpleCSVReader.hpp"
#include "explizit/StateTransitionSystem.hpp"

/*
 * Available classes in here:
 *   1. StateTransitionSystemLoader
 */


namespace explizit {


	enum TokenType { NOT, AND, OR, IMP, XOR, EQUIV, ITE, FTRUE, FFALSE, VARIABLE };

	class StateTransitionSystemLoader {

		private:
			SimpleCSVReader _csv;
			// structures for building a StateTransitionSystem
			std::map<std::string, State*> _name2state;
			std::map<std::string, Variable*> _name2variable;
			std::unique_ptr<StateTransitionSystem> _sys;
			// structures for parsing formulae
			std::size_t _position;
			std::vector<std::string> _tokens;

			bool existsState(const std::string& name) const {
				return _name2state.find(name) != _name2state.end();
			}
			State* getState(const std::string& name) {
				if (!existsState(name)) _name2state[name] = _sys->makeState(name);
				return _name2state[name];
			}

			bool existsVariable(const std::string& name) const {
				return _name2variable.find(name) != _name2variable.end();
			}
			Variable* getVariable(const std::string& name) {
				if (!existsVariable(name)) _name2variable[name] = _sys->makeVariable(name);
				return _name2variable[name];
			}

			Formula* getFormula(const std::string& formula) {
				_position = 0;
				boost::split(_tokens, formula, boost::is_any_of(" "), boost::token_compress_on);
				return parseFormula();
			}
			Formula* parseFormula();
			Formula* parseNextFormula() {
				_position++;
				return parseFormula();
			}
			TokenType getCurrentTokenType() const;

	public:
		StateTransitionSystemLoader(std::istream& input) : _csv(SimpleCSVReader(input)) {
			_sys.reset(new StateTransitionSystem());
		}
		std::unique_ptr<StateTransitionSystem> make();
	};


}