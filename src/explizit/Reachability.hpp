
#pragma once

#include <utility>
#include <unordered_set>
#include "explizit/Common.hpp"
#include "explizit/Configuration.hpp"

/*
 * Available classes in here:
 *   1. ReachabilitySet
 * 
 * Available methods in here:
 *   1. computeReachabilitySet
 */


namespace explizit {


	class ReachabilitySet {
		private:
			std::unordered_set<Configuration> _configs;
			std::vector<const Configuration*> _configPointers;

		public:
			const std::vector<const Configuration*> configs() const {
				return _configPointers;
			}
			std::size_t size() const {
				return _configs.size();
			}
			std::pair<const Configuration*, bool> insert(const Configuration& config) {
				auto res = _configs.insert(config);
				if (res.second) _configPointers.push_back(&(*res.first));
				return std::move(std::make_pair(&(*res.first), res.second));
			}
	};

	ReachabilitySet computeReachabilitySet(const Configuration& init);


}