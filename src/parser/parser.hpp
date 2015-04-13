#pragma once


#include <fstream>
#include <iostream>
#include <exception>
#include "ast/ast.hpp"


namespace parser {

	::std::ifstream open_file(std::string filename);

	::ast::Program* parse_program(std::string src);
	::ast::Program* parse_program(std::istream& is);

	::ast::PredicateList* parse_predicates(std::string src);
	::ast::PredicateList* parse_predicates(std::istream& is);

	class ParserException : public std::exception {
		private:
			const char* _msg;
			virtual const char* what() const throw() { return _msg; }

		public:
			ParserException(const char* msg) : _msg(msg) {}
			ParserException(std::string msg) : _msg(msg.c_str()) {}
	};
}