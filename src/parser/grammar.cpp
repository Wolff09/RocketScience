#include <iostream>
#include "parser/grammar.hpp"

using namespace parserlib;
using namespace parser;


// additional rules


/******************************************************************************
	PROGRAM GRAMMAR
 ******************************************************************************/


/**** TERMINALS ****/


// newline
#define NEWLINE              nl(expr("\r\n") | "\n\r" | '\n' | '\r')


// any character
#define ANY_CHAR             range(0, 255)


//block comments
#define BLOCK_COMMENTS       ("/*" >> *(!expr("*/") >> (NEWLINE | ANY_CHAR)) >> "*/")


// line comments
#define COMMENT              ("//" >> *(!(NEWLINE | eof()) >> ANY_CHAR) >> (NEWLINE | eof()))


// letter
#define CHAR                 (range('a', 'z') | range('A', 'Z'))


// digit
#define DIGIT                range('0', '9')


// whitespace
rule parser::whitespace	= *(BLOCK_COMMENTS | COMMENT | NEWLINE | range(0, 32));


// integer literal
rule parser::boolean		= expr("true") | "false";


// integer literal
rule parser::number		= term(+DIGIT);


// unknown
rule parser::unknown	= expr("unknown");


// identifier
rule parser::identifier	=  &!(boolean | unknown) >> term((CHAR | '_') >> *(CHAR | DIGIT | '_'));


// varname + funname
rule parser::varname	= identifier;
rule parser::funname	= identifier;


/**** NEW EXPRESSIONS ****/


//value
rule parser::value	= varname | number | boolean | '(' >> expression >> ')';


// negation
extern rule parser::unary_expr;
rule parser::logical_not_expr	= '!' >> unary_expr;
rule parser::negative_expr   	= '-' >> unary_expr;


// unary
rule parser::unary_expr = logical_not_expr | negative_expr | value;


// multiplication
rule parser::mul_op  	= '*' >> unary_expr;
rule parser::div_op  	= '/' >> unary_expr;
rule parser::mul_expr	= unary_expr >> *(mul_op | div_op);


// add
rule parser::add_op  	= '+' >> mul_expr;
rule parser::sub_op  	= '-' >> mul_expr;
rule parser::add_expr	= mul_expr >> *(add_op | sub_op);


// comparision expression
rule parser::lt_op   	= "<"  >> add_expr;
rule parser::lte_op  	= "<=" >> add_expr;
rule parser::gt_op   	= ">"  >> add_expr;
rule parser::gte_op  	= ">=" >> add_expr;
rule parser::eq_op   	= "==" >> add_expr;
rule parser::neq_op  	= "!=" >> add_expr;
rule parser::cmp_expr	= add_expr >> *(eq_op | neq_op | lt_op | lte_op | gt_op | gte_op);


// logical and
rule parser::log_and_op      	= "&&" >> cmp_expr;
rule parser::logical_and_expr	= cmp_expr >> *log_and_op;


// logical or
rule parser::log_or_op      	= "||" >> logical_and_expr;
rule parser::logical_or_expr	= logical_and_expr >> *log_or_op;


// expression
rule parser::expression	= unknown | logical_or_expr;


/**** STATEMENTS ****/


extern rule parser::statement;


// if-then-else
rule parser::elz	= "else";
rule parser::it 	= expr("if") >> "(" >> expression >> ")" >> "{" >> *statement >> "}";
rule parser::ite	= expr("if") >> "(" >> expression >> ")" >> "{" >> *statement >> "}" >> elz >> "{" >> *statement >> "}";


// while
rule parser::whl	= expr("while") >> "(" >> expression >> ")" >> "{" >> *statement >> "}";


// function call
rule parser::fun	= funname >> "();";


// assignment
// rule parser::ass	= varname >> "=" >> expression >> ";";
rule parser::eqsep 	= "=";
rule parser::ass   	= varname >> *("," >> varname) >> eqsep >> expression >> *("," >> expression) >> ";";

// skip
rule parser::skipp	= ";";

// assume, assert
rule parser::azzume	= expr("assume") >> "(" >> expression >> ")" >> ";";
rule parser::azzert	= expr("assert") >> "(" >> expression >> ")" >> ";";


// statement
rule parser::statement = fun | ite | it | whl | ass | azzume | azzert | skipp;


/**** DECLARATIONS ****/


// types
rule parser::type_int 		= "int";
rule parser::type_bool		= "bool";


// variable declaration
rule parser::vardef			= (type_bool | type_int) >> identifier >> ";";


// function declaration
rule parser::fundef			= "void" >> identifier >> "(" >> ")" >> "{" >> *vardef >> *statement >> "}";


// translation unit
rule parser::program		= *vardef >> *fundef;


/******************************************************************************
	PREDICATE GRAMMAR
 ******************************************************************************/


rule parser::pred         	= expression;
rule parser::predblock    	= identifier >> ":" >> *(pred >> ";");
rule parser::predicatelist	= *predblock;


/******************************************************************************
	CONNECT GRAMMAR AND AST
 ******************************************************************************/

#include "parser/ast.hpp"

ast<Boolean>         	ast_boolean      	(boolean);
ast<Number>          	ast_number       	(number);
ast<Identifier>      	ast_identifier   	(identifier);
ast<VarName>         	ast_varname      	(varname);
ast<FunName>         	ast_funname      	(funname);
ast<Unknown>         	ast_unknown      	(unknown);
                     	                 									
ast<LogicalNot>      	log_not_expr_ast 	(logical_not_expr);
ast<ArithmeticalNeg> 	negative_expr_ast	(negative_expr);
ast<ArithmeticalMult>	mul_expr_ast     	(mul_op);
ast<ArithmeticalMult>	div_expr_ast     	(div_op);
ast<ArithmeticalAdd> 	add_expr_ast     	(add_op);
ast<ArithmeticalSub> 	sub_expr_ast     	(sub_op);
ast<ComparisonEq>    	eq_op_expr_ast   	(eq_op);
ast<ComparisonNeq>   	neq_op_expr_ast  	(neq_op);
ast<ComparisonLt>    	lt_op_expr_ast   	(lt_op);
ast<ComparisonLte>   	lte_op_expr_ast  	(lte_op);
ast<ComparisonGt>    	gt_op_expr_ast   	(gt_op);
ast<ComparisonGte>   	gte_op_expr_ast  	(gte_op);
ast<LogicalAnd>      	log_and_expr_ast 	(log_and_op);
ast<LogicalOr>       	log_or_expr_ast  	(log_or_op);
                     	                 																								
ast<IfThen>          	ast_it           	(it);
ast<IfThenElse>      	ast_ite          	(ite);
ast<ElseSeparator>   	ast_elz          	(elz);
ast<While>           	ast_whl          	(whl);
ast<Call>            	ast_fun          	(fun);
ast<AssignSeparator> 	ast_eqsep        	(eqsep);
ast<Assignment>      	ast_ass          	(ass);
ast<Assume>          	ast_azzume       	(azzume);
ast<Assert>          	ast_azzert       	(azzert);
ast<Skip>            	ast_skipp        	(skipp);
                     	                 																								
ast<IntType>         	ast_type_int     	(type_int);
ast<BoolType>        	ast_type_bool    	(type_bool);
                     	                 																								
ast<VarDef>          	ast_vardef       	(vardef);
ast<FunDef>          	ast_fundef       	(fundef);
ast<Program>         	ast_program      	(program);
                     	                 	
ast<Predicate>       	ast_pred         	(pred);
ast<PredicateBlock>  	ast_predblock    	(predblock);
ast<PredicateList>   	ast_predicatelist	(predicatelist);
