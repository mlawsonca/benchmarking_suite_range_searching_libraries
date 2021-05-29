cmake_minimum_required(VERSION 3.8)

project(range_tree_benchmark)

set(CMAKE_BUILD_TYPE "Release")

option(BUILD_TESTS "Build code to check the correctness of the various range tree libraries" ON)
option(BUILD_BENCHMARK "Build code used for benchmarking" ON)

if(BUILD_TESTS)
    message(STATUS "Building the correctness test")
endif()
if(BUILD_BENCHMARK)
    message(STATUS "Building the benchmark")
endif()


option(USE_OPEN_MP "Enable the libraries' OpenMP abilities" OFF)
if(NOT USE_OPEN_MP)
    message(STATUS "Compiling programs without OpenMP")
endif()

option(USE_GPU "Enable the libraries to use the node's GPU" OFF)
if(USE_GPU)
    message(STATUS "Compiling programs WITH GPU")   
else()
    message(STATUS "Compiling programs without GPU")
endif()

option(LARGE_TEST "Perform a large test rather than a small one" ON)
if(LARGE_TEST AND BUILD_TESTS)
    message(STATUS "Am performing a large correctness test")
endif()


option(TEST_3DTK "Evaluate 3DTK" ON) 
option(TEST_ALGLIB "Evaluate alglib" ON) 
option(TEST_ANN "Evaluate ANN" ON) 
option(TEST_BOOST_RTREE "Evaluate boost rtree" ON) 
option(TEST_CGAL "evaluate cgal" ON) 
option(TEST_FLANN "Evaluate FLANN" ON) 
option(TEST_KDTREE "Evaluate kdtree" ON) 
option(TEST_KDTREE2 "Evaluate kdtree2" ON) 
option(TEST_KDTREE3 "Evaluate kdtree3" ON) #finsihed
option(TEST_KDTREE4 "Evaluate kdtree4" ON) 
option(TEST_LIBKDTREE "Evaluate libkdtree" ON) 
option(TEST_LIBKDTREE2 "Evaluate libkdtree2" ON) 
option(TEST_LIBNABO "Evaluate libnabo" ON) 
option(TEST_LIBSPATIALINDEX "Evaluate libspatialindex" ON) 
option(TEST_NANOFLANN "Evaluate nanoflann" ON) 
option(TEST_OCTREE "Evaluate octree" ON) 
option(TEST_PCL "Evaluate pcl" OFF)  #REMINDER: NEEDS GCC 8
option(TEST_PICO_TREE "Evaluate pico_tree" ON) 
option(TEST_RTREE_TEMPLATE "Evaluate rtree_template" ON) 
option(TEST_SPATIAL "Evaluate the spatial library" ON) 

find_package(EnvModules REQUIRED)
find_package(PkgConfig REQUIRED) 

#PCL only compiles with GCC8.3.1, 10.2.0 doesn't work
if(TEST_PCL)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 8.4)
        message(FATAL_ERROR "Can only use PCL with GCC <= 8. We did testing with 8.3.1")
    endif()
endif()

set( CMAKE_MODULE_PATH "$ENV{HOME}/bin/spack/share/spack/modules/linux-rhel7-power9le" ${CMAKE_MODULE_PATH}  )

include(ProcessorCount)
ProcessorCount(PROCESSOR_COUNT)
if(NOT PROCESSOR_COUNT EQUAL 0)
  set(NUMBER_OF_CPUS "${PROCESSOR_COUNT}" CACHE STRING "The number of processors to use (default: ${PROCESSOR_COUNT})" )
else()
  set(NUMBER_OF_CPUS "1" CACHE STRING "The number of processors to use (default: 1)" )
endif()
message(STATUS "The number of processors to use for OpenMP (default: ${NUMBER_OF_CPUS})")


if(BUILD_BENCHMARK)
    add_subdirectory(src/benchmark)
endif()
if(BUILD_TESTS)
    add_subdirectory(src/test)
endif()

