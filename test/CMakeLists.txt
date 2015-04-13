
# add_subdirectory(explizit)
# add_subdirectory(symbolic)

# add_executable(Playground Playground.cpp)
# add_test(Playground Playground)

add_executable(RocketScience RocketScience.cpp)
target_link_libraries(RocketScience parser ast cegar)