set(AST_SOURCES
	ast.cpp
	ast_abstraction.cpp
	ast_cfg.cpp
	ast_con.cpp
	ast_construction.cpp
	ast_contains.cpp
	ast_copy.cpp
	ast_extend.cpp
	ast_ignore.cpp
	ast_post.cpp
	ast_prettyprint.cpp
	ast_replace.cpp
	ast_scope.cpp
	ast_trace.cpp
	ast_validation.cpp
	ast_wp.cpp
	ast_z3.cpp
	abstraction_utils.cpp
	trace.cpp
)

add_library(ast ${AST_SOURCES})
target_link_libraries(ast z3 symbolic)