set(PARSER_SOURCES
	ast.cpp
	grammar.cpp
	parser.cpp
)

add_library(parser ${PARSER_SOURCES})
target_link_libraries(parser ast parserlib)