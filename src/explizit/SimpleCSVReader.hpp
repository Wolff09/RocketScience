
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <cassert>
#include <boost/algorithm/string.hpp>
// #include <boost/algorithm/string/regex.hpp>

/*
 * Available classes in here:
 *   1. SimpleCSVReader
 */


namespace explizit {


	class SimpleCSVReader {
		private:
			std::istream& _inputStream;
			std::string _line;
			bool _hasNext;

			void readLine() {
				assert(false);
				// for some reason this stopped working :(
				// if(std::getline(_inputStream, _line)) _hasNext = true;
				// else _hasNext = false;
			}
			void splitLine() {
				// splitting with regex avoids trimming later on -> but not efficient
				// split_regex(*(_cells.get()), _line, regex("( )*,( )*") );
				boost::split(*(_cells.get()), _line, boost::is_any_of(","));
			}

		protected:
			std::unique_ptr<std::vector<std::string> > _cells;

		public:
			SimpleCSVReader(std::istream& input) : _inputStream(input) {
				assert(false);
				// for some reason this stopped working :(
				// assert(input.good());
				_cells.reset(new std::vector<std::string>());
				readLine();
			}
			bool hasNext() const {
				return _hasNext;
			}
			std::vector<std::string>* getNext() {
				assert(hasNext());
				splitLine();
				readLine();
				return _cells.get();
			}
	};


}