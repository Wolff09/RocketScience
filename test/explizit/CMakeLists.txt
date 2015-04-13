
set(CSV_PATH ${CMAKE_SOURCE_DIR}/test/explizit/csv/)

############
## SIMPLE ##
############
add_executable(ExplicitTest ExplicitTest.cpp)
target_link_libraries(ExplicitTest explizit)
add_test(ExplicitTest ExplicitTest)


################
## COMPARATOR ##
################
add_executable(ComparatorTest ComparatorTest.cpp)
target_link_libraries(ComparatorTest explizit)
add_test(ConfigurationComparator ComparatorTest)


################
## CSV IMPORT ##
################
add_executable(CSVTest CSVTest.cpp)
target_link_libraries(CSVTest explizit)

# generate a test target for every .csv test file
file(GLOB files ${CSV_PATH}*.csv)
foreach(file ${files})
	get_filename_component(fname ${file} NAME)
	add_test(CSVImport:${fname} CSVTest ${file})
endforeach()


############################
## Baeume-CSV-IMPORT-TEST ##
############################
add_executable(BaeumeBlatt2Aufgabe3 BaeumeBlatt2Aufgabe3.cpp)
target_link_libraries(BaeumeBlatt2Aufgabe3 explizit)
add_test(BOA2.3 BaeumeBlatt2Aufgabe3 ${CSV_PATH}BaeumeBlatt2Aufgabe3.csv)


##################
## REACHABILITY ##
##################
add_executable(SimpleReachabilityTest SimpleReachabilityTest.cpp)
target_link_libraries(SimpleReachabilityTest explizit)
add_test(SimpleReachabilityTest SimpleReachabilityTest)

add_executable(BigReachability BigReachability.cpp)
target_link_libraries(BigReachability explizit)

add_test(BigReachability:v3_d2 BigReachability ${CSV_PATH}big_v3_d2.csv 3 2)
add_test(BigReachability:v10_d5 BigReachability ${CSV_PATH}big_v10_d5.csv 10 5)
add_test(BigReachability:v20_d10 BigReachability ${CSV_PATH}big_v20_d10.csv 20 10)
add_test(BigReachability:v20_d13 BigReachability ${CSV_PATH}big_v20_d13.csv 20 13)