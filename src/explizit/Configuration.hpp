
#pragma once

#include <boost/functional/hash.hpp>
#include "explizit/Common.hpp"
#include "explizit/StateTransitionSystem.hpp"

/*
 * Available classes in here:
 *   1. Configuration
 *   2. ConfigurationComparator
 */


namespace explizit {


	class Configuration {
		private:
			State* _state;
			VariableAssignment _assignment;

		public:
			Configuration(State* state, VariableAssignment assignment) : _state(state), _assignment(assignment) {}

			State* state() const { return _state; }
			const VariableAssignment& variableAssignment() const { return _assignment; }
			std::string toString() const {
				std::string out;
				out = "[" + _state->name() + ", " + _assignment.toString() + "]";
				return out;
			}


		friend class std::hash<Configuration>;
		friend inline bool operator== (const Configuration& lhs, const Configuration& rhs) {
			return lhs._state == rhs._state && lhs._assignment == rhs._assignment;
		}
	};


}


namespace std {


	template <> struct hash<explizit::Configuration> {
		size_t operator()(const explizit::Configuration& config) const {
			std::size_t seed = 0;
			boost::hash_combine(seed, hash<string>()(config._state->name())); // TODO: use id, since name is not unique -> or hash<state>
			boost::hash_combine(seed, hash<explizit::VariableAssignment>()(config._assignment));
			return seed;
		}
	};


}