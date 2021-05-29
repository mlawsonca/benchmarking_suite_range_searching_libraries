
if(TEST_PCL)
    unset(Boost_INCLUDE_DIR CACHE)
    unset(Boost_LIBRARY_DIRS CACHE)
ednif()

#SEACAS enables us to read node and element coordinates from exodus files
find_package(SEACASExo_format REQUIRED)
find_package(MPI REQUIRED)
find_package(Boost COMPONENTS serialization REQUIRED)

set (ALL_SRCS 
    benchmark.cpp
    3d_points_tests.cpp
    3d_bboxes_tests.cpp
    data_and_query_generation.cpp
    perform_queries.cpp
)


set (ALL_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include/benchmark ${SEACASExo_format_INCLUDE_DIRS} ${SEACASExo_format_TPL_INCLUDE_DIRS} ${MPI_CXX_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
set (ALL_LIBS ${SEACASExo_format_LIBRARIES} ${SEACASExo_format_TPL_LIBRARIES} ${MPI_LIBRARIES} ${Boost_LIBRARIES})
set (ALL_BUILD_FLAGS "")
set (ALL_COMPILE_DEFINITIONS "")

if (TEST_3DTK)
    if(NOT DEFINED _3DTK_DIR) 
        message(FATAL_ERROR "The TEST_3DTK option requires _3DTK_DIR to be set")
    endif()

    find_library(LIB_SCAN NAMES scan PATHS ${_3DTK_DIR}/obj)
    if(NOT LIB_SCAN)
        message(FATAL_ERROR "3DTK build failed: scan library not found")
    else(NOT LIB_SCAN)
        message( STATUS "Found scan!")
    endif()

    list(APPEND ALL_INCLUDE_DIRS ${_3DTK_DIR}/include)
    list(APPEND ALL_LIBS ${LIB_SCAN})
    if(USE_OPEN_MP)
        find_package(OpenMP REQUIRED)
        list(APPEND ALL_BUILD_FLAGS "-DMAX_OPENMP_NUM_THREADS=${NUMBER_OF_CPUS} -DOPENMP_NUM_THREADS=${NUMBER_OF_CPUS} ${OpenMP_CXX_FLAGS} -DOPENMP")
    else()
        list(APPEND ALL_BUILD_FLAGS "-DMAX_OPENMP_NUM_THREADS=1 -DOPENMP_NUM_THREADS=1 ${OpenMP_CXX_FLAGS} -DOPENMP")
    endif()
    list(APPEND ALL_COMPILE_DEFINITIONS "TEST_3DTK")
endif()

if (TEST_ALGLIB)
    if(NOT DEFINED ALGLIB_DIR) 
        message(FATAL_ERROR "The TEST_ALGLIB option requires ALGLIB_DIR to be set")
    endif()

    FILE(GLOB ALGLIB_SRC ${ALGLIB_DIR}/src/*.cpp)

    list(APPEND ALL_SRCS ${ALGLIB_SRC})
    list(APPEND ALL_INCLUDE_DIRS ${ALGLIB_DIR}/src)
    list(APPEND ALL_COMPILE_DEFINITIONS "TEST_ALGLIB")
endif()



if (TEST_ANN)
    if(NOT DEFINED ANN_DIR) 
        message(FATAL_ERROR "The TEST_ANN option requires ANN_DIR to be set" )
    endif()

    find_library(ANN_LIB NAMES ann ANN Ann PATHS ${ANN_DIR}/lib)
    if(NOT ANN_LIB)
        message(FATAL_ERROR "ANN build failed: ann library not found")
    else(NOT ANN_LIB)
        message( STATUS "Found ann!")
    endif()

    list(APPEND ALL_INCLUDE_DIRS ${ANN_DIR}/include ${ANN_DIR}/test)
    list(APPEND ALL_LIBS ${ANN_LIB})
    list(APPEND ALL_COMPILE_DEFINITIONS "TEST_ANN")
endif()


if(TEST_BOOST_RTREE)
    find_package(Boost REQUIRED)   
    list(APPEND ALL_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    list(APPEND ALL_COMPILE_DEFINITIONS "TEST_BOOST")
endif()


if(TEST_CGAL)
    find_package(CGAL REQUIRED)    
    list(APPEND ALL_INCLUDE_DIRS ${CGAL_INCLUDE_DIRS} )
    list(APPEND ALL_COMPILE_DEFINITIONS "TEST_CGAL")
endif()

if (TEST_FLANN)
    if(NOT DEFINED FLANN_DIR) 
        message(FATAL_ERROR "The TEST_FLANN option requires FLANN_DIR to be set" )
    endif()
    find_package(Boost COMPONENTS mpi system serialization thread REQUIRED)
    if(NOT DEFINED LZ4_DIR) 
        message(FATAL_ERROR "The TEST_FLANN option requires LZ4_DIR to be set")
    endif()
    find_library(LZ4_LIB NAMES lz4 PATHS ${LZ4_DIR}/lib)
    if(NOT LZ4_LIB)
        message(FATAL_ERROR "flann build failed: lz4 library not found")
    else(NOT LZ4_LIB)
        message( STATUS "Found lz4!")
    endif(NOT LZ4_LIB)

    list(APPEND ALL_INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${FLANN_DIR}/src/cpp ${LZ4_DIR}/include)

    if(USE_GPU)
        env_module(load cuda/10.1.243)
        find_package(CUDA REQUIRED)
        list(APPEND ALL_LIBS -Wl,--push-state,--no-as-needed ${LZ4_LIB} -Wl,--pop-state ${FLANN_DIR}/install_debug/lib/libflann_s.a
            ${FLANN_DIR}/install_debug/lib/libflann_cuda_s.a ${CUDA_LIBRARIES})
        list(APPEND ALL_COMPILE_DEFINITIONS "TEST_FLANN" "USE_GPU" FLANN_USE_CUDA)
    else()
        message(STATUS "USE_GPU is turned off so am building without FLANN CUDA")
        list(APPEND ALL_LIBS -Wl,--push-state,--no-as-needed ${LZ4_LIB} -Wl,--pop-state ${FLANN_DIR}/install_debug/lib/libflann_s.a)
        list(APPEND ALL_COMPILE_DEFINITIONS "TEST_FLANN")
    endif()
endif()


if (TEST_KDTREE)
    if(NOT DEFINED KDTREE_DIR) 
        message(FATAL_ERROR "The TEST_KDTREE option requires KDTREE_DIR to be set" )
    endif()

    find_library(KDTREE_LIB NAMES KDTree kdtree KDTREE PATHS ${KDTREE_DIR}/build/lib)
    if(NOT KDTREE_LIB)
        message(FATAL_ERROR "kdtree build failed: kdtree library not found")
    else(NOT KDTREE_LIB)
        message( STATUS "Found kdtree!")
    endif(NOT KDTREE_LIB)

    list(APPEND ALL_INCLUDE_DIRS ${KDTREE_DIR})
    list(APPEND ALL_LIBS ${KDTREE_LIB})
    list(APPEND ALL_COMPILE_DEFINITIONS "TEST_KDTREE")
endif()




if (TEST_KDTREE2)
    if(NOT DEFINED KDTREE2_DIR) 
        message(FATAL_ERROR "The TEST_KDTREE2 option requires KDTREE2_DIR to be set" )
    endif()

    env_module(load boost-1.74.0-gcc-10.2.0-rdzugft)
    find_package(Boost COMPONENTS mpi system serialization thread REQUIRED)

    list(APPEND ALL_SRCS ${KDTREE2_DIR}/src-c++/kdtree2.cpp)
    list(APPEND ALL_INCLUDE_DIRS ${KDTREE2_DIR}/src-c++ ${Boost_INCLUDE_DIRS})
    list(APPEND ALL_COMPILE_DEFINITIONS "TEST_KDTREE2")
endif()


if (TEST_KDTREE3)
    if(NOT DEFINED CATKIN_SRC_DIR) 
        message(FATAL_ERROR "The TEST_KDTREE3 option requires CATKIN_SRC_DIR to be set to the catkin workstation src directory" )
    endif()
    if(NOT DEFINED CATKIN_INSTALL_DIR) 
        message(FATAL_ERROR "The TEST_KDTREE3 option requires CATKIN_INSTALL_DIR to be set to the catkin workstation install directory containing KdTree" )
    endif()

    find_package(PCL REQUIRED)
    find_package(Boost COMPONENTS system REQUIRED)
    find_package(Eigen3 3.3 REQUIRED NO_MODULE)

    find_library(ROSCPP_LIB NAMES roscpp PATHS ${CATKIN_INSTALL_DIR}/lib )
    find_library(CONSOLE_BRIDGE NAMES console_bridge PATHS ${CATKIN_SRC_DIR}/console_bridge/install/lib64 )
    find_library(ROSCONSOLE_BRIDGE NAMES rosconsole_bridge PATHS ${CATKIN_INSTALL_DIR}/lib )

    list(APPEND ALL_INCLUDE_DIRS ${CATKIN_SRC_DIR}/KdTree/include ${EIGEN3_INCLUDE_DIR})
    list(APPEND ALL_LIBS ${ROSCPP_LIB} ${ROSCONSOLE_BRIDGE} ${CONSOLE_BRIDGE} ${Boost_LIBRARIES})
    list(APPEND ALL_COMPILE_DEFINITIONS "TEST_KDTREE3")
endif()



if (TEST_KDTREE4)
    if(NOT DEFINED KDTREE4_DIR) 
        message(FATAL_ERROR "The TEST_KDTREE4 option requires KDTREE4_DIR to be set")
    endif()

    find_library(KDTREE4_LIB NAMES libKDTree.a libkdtree.a libKDTREE.a PATHS ${KDTREE4_DIR})
    if(NOT KDTREE4_LIB)
        message(FATAL_ERROR "kdtree4 build failed: kdtree4 library not found")
    else(NOT KDTREE4_LIB)
        message( STATUS "Found kdtree4!")
    endif(NOT KDTREE4_LIB)

    list(APPEND ALL_INCLUDE_DIRS ${KDTREE4_DIR})
    list(APPEND ALL_LIBS ${KDTREE4_LIB})
    list(APPEND ALL_COMPILE_DEFINITIONS "TEST_KDTREE4")
endif()



if (TEST_LIBKDTREE)
    if(NOT DEFINED LIBKDTREE_DIR) 
        message(FATAL_ERROR "The TEST_LIBKDTREE option requires LIBKDTREE_DIR to be set")
    endif()
    list(APPEND ALL_INCLUDE_DIRS ${LIBKDTREE_DIR})
    list(APPEND ALL_COMPILE_DEFINITIONS "TEST_LIBKDTREE")
endif()

if (TEST_LIBKDTREE2)
    if(NOT DEFINED LIBKDTREE2_DIR) 
        message(FATAL_ERROR "The TEST_LIBKDTREE2 option requires LIBKDTREE2_DIR to be set")
    endif()

    find_library(LIBKDTREE2_LIB NAMES libkdtree.a PATHS ${LIBKDTREE2_DIR}/install/lib)
    if(NOT LIBKDTREE2_LIB)
        message(FATAL_ERROR "libkdtree2 build failed: libkdtree2 library not found")
    else(NOT LIBKDTREE2_LIB)
        message( STATUS "Found libkdtree2!")
        message( STATUS "libkdtree2: ${LIBKDTREE2_LIB}")
    endif(NOT LIBKDTREE2_LIB)

    list(APPEND ALL_INCLUDE_DIRS ${LIBKDTREE2_DIR})
    list(APPEND ALL_LIBS ${LIBKDTREE2_LIB} -lpthread -lm)
    list(APPEND ALL_COMPILE_DEFINITIONS "TEST_LIBKDTREE2")
endif()


if (TEST_LIBNABO)
    find_package(Eigen3 3.3 REQUIRED NO_MODULE)

    if(NOT DEFINED LIBNABO_DIR) 
        message(FATAL_ERROR "The TEST_LIBNABO option requires LIBNABO_DIR to be set")
    endif()

    if(USE_OPEN_MP)
        find_package(OpenMP REQUIRED)
        find_library(LIBNABO_LIB NAMES libnabo.a PATHS ${LIBNABO_DIR}/install/lib)
    else()
        find_library(LIBNABO_LIB NAMES libnabo.a PATHS ${LIBNABO_DIR}/install_no_openmp/lib)
    endif()
    if(NOT LIBNABO_LIB)
        message(FATAL_ERROR "libnabo build failed: libnabo library not found")
    else(NOT LIBNABO_LIB)
        message( STATUS "Found libnabo!")
    endif(NOT LIBNABO_LIB)


    list(APPEND ALL_INCLUDE_DIRS ${LIBNABO_DIR} ${EIGEN3_INCLUDE_DIR})
    if(USE_OPEN_MP)
        list(APPEND ALL_LIBS -fopenmp)
    endif()
    list(APPEND ALL_LIBS ${LIBNABO_LIB})
    list(APPEND ALL_COMPILE_DEFINITIONS "TEST_LIBNABO")
endif()


if (TEST_LIBSPATIALINDEX)
    if(NOT DEFINED LIBSPATIALINDEX_DIR) 
        message(FATAL_ERROR "The TEST_LIBSPATIALINDEX option requires LIBSPATIALINDEX_DIR to be set")
    endif()
    list(APPEND ALL_INCLUDE_DIRS ${LIBSPATIALINDEX_DIR}/include)
    list(APPEND ALL_LIBS ${LIBSPATIALINDEX_DIR}/install/lib/libspatialindex.so)
    list(APPEND ALL_COMPILE_DEFINITIONS "TEST_LIBSPATIALINDEX")
endif()



if(TEST_NANOFLANN)
    if(NOT DEFINED NANOFLANN_DIR) 
        message(FATAL_ERROR "The TEST_NANOFLANN option requires NANOFLANN_DIR to be set")
    endif()
    list(APPEND ALL_INCLUDE_DIRS ${NANOFLANN_DIR}/include ${NANOFLANN_DIR}/examples)
    list(APPEND ALL_COMPILE_DEFINITIONS "TEST_NANOFLANN")
    if (USE_OPEN_MP)
        find_package(OpenMP REQUIRED)
        list(APPEND ALL_LIBS -fopenmp)
    endif()
endif()



if(TEST_OCTREE)
    if(NOT DEFINED OCTREE_DIR) 
        message(FATAL_ERROR "The TEST_OCTREE option requires OCTREE_DIR to be set")
    endif()
    list(APPEND ALL_INCLUDE_DIRS ${OCTREE_DIR})
    list(APPEND ALL_COMPILE_DEFINITIONS "TEST_OCTREE")

endif()


if (TEST_PCL)
    find_package(PCL REQUIRED)
    find_package(FLANN REQUIRED)
    if(NOT DEFINED LZ4_DIR) 
        message(FATAL_ERROR "The TEST_PCL option requires LZ4_DIR to be set")
    endif()
    find_library(LZ4_LIB NAMES lz4 PATHS ${LZ4_DIR}/lib)
    if(NOT LZ4_LIB)
        message(FATAL_ERROR "pcl build failed: lz4 library not found")
    else(NOT LZ4_LIB)
        message( STATUS "Found lz4!")
    endif(NOT LZ4_LIB)

    list(APPEND ALL_INCLUDE_DIRS ${PCL_INCLUDE_DIRS} ${FLANN_INCLUDE_DIR})
    list(APPEND ALL_LIBS ${LZ4_LIB})
    if(USE_GPU)
        list(APPEND ALL_LIBS ${PCL_LIBRARIES} )
        list(APPEND ALL_COMPILE_DEFINITIONS "TEST_PCL" "USE_GPU")
    else()
        list(APPEND ALL_LIBS ${PCL_KDTREE_LIBRARIES} ${PCL_OCTREE_LIBRARIES} ${FLANN_LIBRARY})
        list(APPEND ALL_COMPILE_DEFINITIONS "TEST_PCL")        
    endif()

endif()



if (TEST_PICO_TREE)
    if(NOT DEFINED PICO_TREE_DIR) 
        message(FATAL_ERROR "The TEST_PICO_TREE option requires PICO_TREE_DIR to be set")
    endif()
    list(APPEND ALL_INCLUDE_DIRS ${PICO_TREE_DIR}/src ${PICO_TREE_DIR}/examples/pico_common)
    list(APPEND ALL_COMPILE_DEFINITIONS "TEST_PICO_TREE")
endif()


if (TEST_RTREE_TEMPLATE)
    if(NOT DEFINED RTREE_TEMPLATE_DIR) 
        message(FATAL_ERROR "The TEST_RTREE_TEMPLATE option requires RTREE_TEMPLATE_DIR to be set")
    endif()
    list(APPEND ALL_INCLUDE_DIRS ${RTREE_TEMPLATE_DIR})
    list(APPEND ALL_COMPILE_DEFINITIONS "TEST_RTREE_TEMPLATE")
endif(TEST_RTREE_TEMPLATE)


if (TEST_SPATIAL)
    if(NOT DEFINED SPATIAL_DIR) 
        message(FATAL_ERROR "The TEST_SPATIAL option requires SPATIAL_DIR to be set")
    endif()
    list(APPEND ALL_INCLUDE_DIRS ${SPATIAL_DIR}/src)
    list(APPEND ALL_COMPILE_DEFINITIONS "TEST_SPATIAL")
endif()

# message(STATUS "ALL_SRCS: ${ALL_SRCS}")
# message(STATUS "ALL_INCLUDE_DIRS: ${ALL_INCLUDE_DIRS}")
# message(STATUS "ALL_LIBS: ${ALL_LIBS}")

add_executable(range_query_benchmark ${ALL_SRCS})
target_include_directories(range_query_benchmark PUBLIC ${ALL_INCLUDE_DIRS})

if(ALL_BUILD_FLAGS)
    set_target_properties(range_query_benchmark PROPERTIES COMPILE_FLAGS ${ALL_BUILD_FLAGS})
endif()
if(ALL_LIBS)
    target_link_libraries(range_query_benchmark ${ALL_LIBS})
endif()
if(ALL_COMPILE_DEFINITIONS)
    target_compile_definitions(range_query_benchmark PUBLIC ${ALL_COMPILE_DEFINITIONS})
endif()





