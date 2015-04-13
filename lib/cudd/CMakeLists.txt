
####################
## CUDD C LIBRARY ##
####################
include_directories(cudd dddmp epd mtr obj st util)

AUX_SOURCE_DIRECTORY(cudd SRC_CUDD)
set(SRC_EPD
	epd/epd.c
)
set(SRC_MTR
	mtr/mtrBasic.c
	mtr/mtrGroup.c
)
set(SRC_ST
	st/st.c
)
set(SRC_UTIL
	util/cpu_time.c
	util/cpu_stats.c
	util/safe_mem.c
	util/strsav.c
	util/texpand.c
	util/ptime.c
	util/prtime.c
	util/pipefork.c
	util/pathsearch.c
	util/stub.c
	util/datalimit.c
)
set(SRC_DDDMP
	dddmp/dddmpStoreBdd.c
	dddmp/dddmpStoreAdd.c
	dddmp/dddmpStoreCnf.c
	dddmp/dddmpLoad.c
	dddmp/dddmpLoadCnf.c
	dddmp/dddmpNodeBdd.c
	dddmp/dddmpNodeAdd.c
	dddmp/dddmpNodeCnf.c
	dddmp/dddmpStoreMisc.c
	dddmp/dddmpUtil.c
	dddmp/dddmpBinary.c
	dddmp/dddmpConvert.c
	dddmp/dddmpDbg.c 
)
set(CUDD_SOURCES
	${SRC_CUDD}
	${SRC_EPD}
	${SRC_MTR}
	${SRC_ST}
	${SRC_UTIL}
	${SRC_DDDMP}
)

add_library(cudd ${CUDD_SOURCES})


if(${UNIX})
	set_target_properties("cudd" "PROPERTIES" "COMPILE_FLAGS" "-w -O3 -mtune=native -DHAVE_IEEE_754 -DBSD -DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8")
endif()

if(${APPLE})
	set_target_properties("cudd" "PROPERTIES" "COMPILE_FLAGS" "-w -O3 -mtune=native -DHAVE_IEEE_754 -DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8")
endif()


###################
## CUDD TEST (C) ##
###################
include_directories(nanotrav)

AUX_SOURCE_DIRECTORY(nanotrav NANOTRAV_SOURCES)
add_executable(nanotrav ${NANOTRAV_SOURCES})
target_link_libraries(nanotrav cudd m)

# testing a la nanotrav/README
set(NANOTRAV_PATH ${CMAKE_SOURCE_DIR}/lib/cudd/nanotrav/)
add_test(nanotrav:C17.blif nanotrav -p 1 -cover ${NANOTRAV_PATH}C17.blif)
add_test(nanotrav:C880.blif nanotrav -p 1 -ordering dfs -autodyn -automethod sifting -reordering sifting -drop ${NANOTRAV_PATH}C880.blif)
add_test(nanotrav:s27.blif nanotrav -p 1 -trav ${NANOTRAV_PATH}s27.blif)
add_test(nanotrav:mult32a.blif nanotrav -p 1 -autodyn -reordering sifting -trav ${NANOTRAV_PATH}mult32a.blif)
# add_test(nanotrav:rcn25.blif nanotrav -p 1 -envelope ${NANOTRAV_PATH}rcn25.blif) # takes approx. 1 minute


#######################
## CUDD C++ BINDINGS ##
#######################
include_directories(obj)

add_library(cudd++ obj/cuddObj.cc)
target_link_libraries(cudd++ cudd) # TODO: when linking cudd++ to executable, do i need to link cudd, too?


#####################
## CUDD TEST (C++) ##
#####################
add_executable(testcudd++ obj/testobj.cc)
target_link_libraries(testcudd++ cudd++)
add_test(testcudd++ testcudd++)

