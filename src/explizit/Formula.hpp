// TODO: override keyword (cfg. Javas @Override)
// TODO: remove toString() output stuff?
#pragma once

#include <memory>
#include "explizit/Common.hpp"

/*
 * Available classes in here:
 *    1. Formula (abstract)
 *    2. NotFormula
 *    3. BinaryFormula (abstract)
 *    4. AndFormula
 *    5. OrFormula
 *    6. ImpFormula
 *    7. XOrFormula
 *    8. EquivFormula
 *    9. ITEFormula
 *   10. Literal
 *   11. TrueFormula
 *   11. FalseFormula
 *
 * Note: all clases listed above inherit from Formula.
 */

using namespace std;
namespace explizit {


	class Formula {
		public:
			virtual ~Formula() = default;
			virtual bool evaluate(const VariableAssignment& assignment) const = 0;
			virtual std::string toString() const = 0;
	};

	class NotFormula : public Formula {
		private:
			std::unique_ptr<Formula> _subformula;

		public:
			NotFormula(Formula* subformula) { _subformula.reset(subformula); };
			bool evaluate(const VariableAssignment& assignment) const {
				return !_subformula->evaluate(assignment);
			}
			std::string toString() const {
				return "!(" + _subformula->toString() + ")";
			}
	};

	class BinaryFormula : public Formula {
		protected:
			std::unique_ptr<Formula> _left;
			std::unique_ptr<Formula> _right;

		public:
			BinaryFormula(Formula* left, Formula* right) { _left.reset(left); _right.reset(right); };
			virtual ~BinaryFormula() = default;
			virtual bool evaluate(const VariableAssignment& assignment) const = 0;
			std::string toString() const {
				return "(" + _left->toString() + ") " + getOperatorSymbol() + " (" + _right->toString() + ")";
			}
			virtual std::string getOperatorSymbol() const = 0;
	};

	class AndFormula : public BinaryFormula {
		public:
			AndFormula(Formula* left, Formula* right) : BinaryFormula(left, right) {};
			bool evaluate(const VariableAssignment& assignment) const {
				bool evalLeft = _left->evaluate(assignment);
				bool evalRight = _right->evaluate(assignment);
				return evalLeft & evalRight;
			}
			std::string getOperatorSymbol() const { return "&"; };
	};

	class OrFormula : public BinaryFormula {
		public:
			OrFormula(Formula* left, Formula* right) : BinaryFormula(left, right) {};
			bool evaluate(const VariableAssignment& assignment) const {
				return _left->evaluate(assignment) | _right->evaluate(assignment);
			}
			std::string getOperatorSymbol() const { return "|"; };
	};

	class ImpFormula : public BinaryFormula {
		public:
			ImpFormula(Formula* left, Formula* right) : BinaryFormula(left, right) {};
			bool evaluate(const VariableAssignment& assignment) const {
				return !_left->evaluate(assignment) || _right->evaluate(assignment);
			}
			std::string getOperatorSymbol() const { return "->"; };
	};

	class XOrFormula : public BinaryFormula {
		public:
			XOrFormula(Formula* left, Formula* right) : BinaryFormula(left, right) {};
			bool evaluate(const VariableAssignment& assignment) const {
				return _left->evaluate(assignment) != _right->evaluate(assignment);
			}
			std::string getOperatorSymbol() const { return "^"; };
	};

	class EquivFormula : public BinaryFormula {
		public:
			EquivFormula(Formula* left, Formula* right) : BinaryFormula(left, right) {};
			bool evaluate(const VariableAssignment& assignment) const {
				return _left->evaluate(assignment) == _right->evaluate(assignment);
			}
			std::string getOperatorSymbol() const { return "<->"; };
	};

	class ITEFormula : public Formula {
		private:
			std::unique_ptr<Formula> _condition;
			std::unique_ptr<Formula> _then;
			std::unique_ptr<Formula> _otherwise;

		public:
			ITEFormula(Formula* condition, Formula* then_case, Formula* else_case) {
				_condition.reset(condition);
				_then.reset(then_case);
				_otherwise.reset(else_case);
			};
			bool evaluate(const VariableAssignment& assignment) const {
				return _condition->evaluate(assignment) ? _then->evaluate(assignment) : _otherwise->evaluate(assignment);
			}
			std::string toString() const {
				return "(" + _condition->toString() + ") ? (" + _then->toString() + ") : (" + _otherwise->toString() + ")";
			}
	};

	class Literal : public Formula {
		private:
			Variable* _variable;

		public:
			Literal(Variable* variable) { _variable = variable; }
			bool evaluate(const VariableAssignment& assignment) const {
				return assignment.evaluate(_variable);
			}
			std::string toString() const {
				return "<" + _variable->name() + ">";
			}
	};

	class TrueFormula : public Formula {
		public:
			bool evaluate(const VariableAssignment& assignment) const { return true; }
			std::string toString() const { return "true"; }
	};

	class FalseFormula : public Formula {
		public:
			bool evaluate(const VariableAssignment& assignment) const { return false; }
			std::string toString() const { return "false"; }
	};


}
