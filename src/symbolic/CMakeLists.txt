
set(SYMBOLIC_SOURCES
	ControlFlowGraph.cpp
	Reachability.cpp
)

add_library(symbolic ${SYMBOLIC_SOURCES})
target_link_libraries(symbolic cudd++)