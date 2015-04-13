
#pragma once

#include <map>
#include <vector>
#include <string>
#include <algorithm>

/*
 * Available classes in here:
 *   1. Variable
 *   2. VariableAssignment
 *   3. VariableAssignmentComparator
 */


namespace explizit {


	class Variable {
		private:
			const int _id;
			const std::string _name;

		public:
			Variable(const Variable& variable) = delete;
			Variable(const std::string& name, const int& id) : _name(name), _id(id) {}

			const std::string& name() const { return _name; }
			const int& id() const { return _id; }
	};

	class VariableAssignment{
		private:
			std::vector<bool> _assignment;

		public:
			VariableAssignment(size_t size) { _assignment.assign(size, false); }

			bool evaluate(Variable* variable) const { return _assignment[variable->id()]; }
			void set(Variable* variable, bool value) { _assignment[variable->id()] = value; }
			std::size_t size() const { return _assignment.size(); }
			std::string toString() const {
				std::string out;
				for (const auto &e : _assignment) {
					if (e) out.append("1");
					else out.append("0");
				}
				return out;
			}


		friend class std::hash<VariableAssignment>;
		friend inline bool operator== (const VariableAssignment& lhs, const VariableAssignment& rhs) {
			return lhs._assignment.size() == rhs._assignment.size() &&
			       std::equal(lhs._assignment.begin(), lhs._assignment.end(), rhs._assignment.begin());
		}
	};


}


namespace std {


	template <> struct hash<explizit::VariableAssignment> {
		size_t operator()(const explizit::VariableAssignment& assignment) const {
			return hash<vector<bool>>()(assignment._assignment);
		}
	};


}
