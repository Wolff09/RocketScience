#pragma once

#include "parserlib.hpp"


namespace parser {

	extern parserlib::rule whitespace;
	extern parserlib::rule expression;
	extern parserlib::rule program;
	extern parserlib::rule predicatelist;

	extern parserlib::rule boolean;
	extern parserlib::rule number;
	extern parserlib::rule unknown;
	extern parserlib::rule identifier;
	extern parserlib::rule varname;
	extern parserlib::rule funname;
	extern parserlib::rule value;
	extern parserlib::rule unary_expr;
	extern parserlib::rule logical_not_expr;
	extern parserlib::rule negative_expr;
	extern parserlib::rule unary_expr;
	extern parserlib::rule mul_op;
	extern parserlib::rule div_op;
	extern parserlib::rule mul_expr;
	extern parserlib::rule add_op;
	extern parserlib::rule sub_op;
	extern parserlib::rule add_expr;
	extern parserlib::rule lt_op;
	extern parserlib::rule lte_op;
	extern parserlib::rule gt_op;
	extern parserlib::rule gte_op;
	extern parserlib::rule eq_op;
	extern parserlib::rule neq_op;
	extern parserlib::rule cmp_expr;
	extern parserlib::rule log_and_op;
	extern parserlib::rule logical_and_expr;
	extern parserlib::rule log_or_op;
	extern parserlib::rule logical_or_expr;
	extern parserlib::rule statement;
	extern parserlib::rule elz;
	extern parserlib::rule it;
	extern parserlib::rule ite;
	extern parserlib::rule whl;
	extern parserlib::rule fun;
	extern parserlib::rule ass;
	extern parserlib::rule azzume;
	extern parserlib::rule azzert;
	extern parserlib::rule skipp;
	extern parserlib::rule eqsep;
	extern parserlib::rule statement;
	extern parserlib::rule type_int;
	extern parserlib::rule type_bool;
	extern parserlib::rule vardef;
	extern parserlib::rule fundef;
	extern parserlib::rule pred;
	extern parserlib::rule predblock;

}