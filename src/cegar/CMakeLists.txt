set(CEGAR_SOURCES
	cegar.cpp
	constraints.cpp
	interpolate.cpp
)

add_library(cegar ${CEGAR_SOURCES})
target_link_libraries(cegar z3 symbolic ast)