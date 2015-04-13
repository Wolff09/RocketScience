#include <sstream>
#include "parserlib.hpp"
#include "parser/ast.hpp"
#include "parser/grammar.hpp"
#include "parser/parser.hpp"

using namespace parserlib;
using namespace parser;


std::ifstream parser::open_file(std::string filename) {
	return std::ifstream(filename);
}


template<class T>
T* parse_input(input in, rule start) {
	error_list el;
	T* root = 0;
	parse(in, start, ::whitespace, el, root);

	if (!root) {
		std::stringstream ss;
		ss << "Error: " << std::endl;
		for(error_list::iterator it = el.begin(); it != el.end(); ++it) {
			error &err = *it;
			ss << "line " << err.m_begin.m_line << ", col " << err.m_begin.m_col << ": ";
			ss << "syntax error" << std::endl;
		}
		throw ParserException(ss.str());
	}
	return root;
}

input mk_input(std::string src) {
	return input(src.begin(), src.end());
}

input mk_input(std::istream& is) {
	std::string src;
	src.assign(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>());
	return input(src.begin(), src.end());
}


::ast::Program* parser::parse_program(std::string src) {
	std::unique_ptr<Program> prog(parse_input<Program>(mk_input(src), ::program));
	return prog->toAST();
}

::ast::Program* parser::parse_program(std::istream& is) {
	std::unique_ptr<Program> prog(parse_input<Program>(mk_input(is), ::program));
	return prog->toAST();
}


::ast::PredicateList* parser::parse_predicates(std::string src) {
	std::unique_ptr<PredicateList> preds(parse_input<PredicateList>(mk_input(src), ::predicatelist));
	return preds->toAST();
}

::ast::PredicateList* parser::parse_predicates(std::istream& is) {
	std::unique_ptr<PredicateList> preds(parse_input<PredicateList>(mk_input(is), ::predicatelist));
	return preds->toAST();
}
