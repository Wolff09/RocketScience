cmake_minimum_required(VERSION 2.6)
project(RocketScience C CXX)


# $config
set(CMAKE_VERBOSE_MAKEFILE OFF) # ON

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif(NOT CMAKE_BUILD_TYPE)


set(CMAKE_BUILD_TYPE Debug) # don't get rid of assertions

# use c++11
if(CMAKE_COMPILER_IS_GNUCXX)
	add_definitions(-fno-strict-aliasing)
	add_definitions(-Wall -Wextra)
	add_definitions(-fshow-column)
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11)
endif(CMAKE_COMPILER_IS_GNUCXX)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11") #  -ftemplate-depth=512
endif()


# Boost
# FIND_PACKAGE( Boost 1.55 COMPONENTS regex REQUIRED )
# INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )


# adds test target
enable_testing()


# includes
 # TODO: this could be cooler (why is the include in lib/cudd/CMakeLists.txt) ignored?
include_directories(lib/cudd/cudd)
include_directories(lib/cudd/dddmp)
include_directories(lib/cudd/epd)
include_directories(lib/cudd/mtr)
include_directories(lib/cudd/obj)
include_directories(lib/cudd/st)
include_directories(lib/cudd/util)
include_directories(lib/cudd/obj)
include_directories(lib/parse)
include_directories(lib/parserlib)
include_directories(src)


# sources
add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(test)
