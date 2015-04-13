
add_executable(CFG_devl CFG_devl.cpp)
target_link_libraries(CFG_devl cudd++)

add_executable(Fib Fib.cpp)
target_link_libraries(Fib cudd++)



add_executable(SimpleTest SimpleTest.cpp)
target_link_libraries(SimpleTest cudd++)
add_test(SimpleSymbolicTest SimpleTest)

add_executable(LongTest LongTest.cpp)
target_link_libraries(LongTest cudd++)
add_test(LongSymbolicTest:10 LongTest 10)
add_test(LongSymbolicTest:20 LongTest 20)
add_test(LongSymbolicTest:50 LongTest 50)
add_test(LongSymbolicTest:100 LongTest 100)
add_test(LongSymbolicTest:200 LongTest 200)
add_test(LongSymbolicTest:500 LongTest 500)

add_executable(PetriNet PetriNet.cpp)
target_link_libraries(PetriNet cudd++)
add_test(PetriNetTest PetriNet 10)

add_executable(PetriNetUglyFormula PetriNetUglyFormula.cpp)
target_link_libraries(PetriNetUglyFormula cudd++)
add_test(PetriNetTestUglyFormula PetriNetUglyFormula 10)
