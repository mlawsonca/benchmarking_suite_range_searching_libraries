#ifndef RANGE_TREE_LIBRARIES_HH
#define RANGE_TREE_LIBRARIES_HH

#include "common.hh"
#include "../benchmark/all_libraries/brute_force_test.hh"

#ifdef TEST_3DTK
    #include "../benchmark/all_libraries/3dtk_test.hh"
#endif

#ifdef TEST_ALGLIB
    #include "../benchmark/all_libraries/alglib_test.hh"
#endif

#ifdef TEST_ANN
    #include "../benchmark/all_libraries/ann_test.hh"
#endif

#ifdef TEST_BOOST
    #include "../benchmark/all_libraries/boost_test.hh"
#endif

#ifdef TEST_CGAL
    #include "../benchmark/all_libraries/cgal_test.hh"
#endif

#ifdef TEST_FLANN
    #include "../benchmark/all_libraries/flann_test.hh"
#endif

#ifdef TEST_KDTREE
    #include "../benchmark/all_libraries/kdtree_test.hh"
#endif

#ifdef TEST_KDTREE2
    #include "../benchmark/all_libraries/kdtree2_test.hh"
#endif

#ifdef TEST_KDTREE3
    #include "../benchmark/all_libraries/kdtree3_test.hh"
#endif

#ifdef TEST_KDTREE4
    #include "../benchmark/all_libraries/kdtree4_test.hh"
#endif

#ifdef TEST_LIBKDTREE
    #include "../benchmark/all_libraries/libkdtree_test.hh"
#endif

#ifdef TEST_LIBKDTREE2
    #include "../benchmark/all_libraries/libkdtree2_test.hh"
#endif

#ifdef TEST_LIBNABO
    #include "../benchmark/all_libraries/libnabo_test.hh"
#endif

#ifdef TEST_LIBSPATIALINDEX
    #include "../benchmark/all_libraries/libspatialindex_test.hh"
#endif

#ifdef TEST_NANOFLANN
    #include "../benchmark/all_libraries/nanoflann_test.hh"
#endif

#ifdef TEST_OCTREE
    #include "../benchmark/all_libraries/octree_test.hh"
#endif

#ifdef TEST_PCL
    #include "../benchmark/all_libraries/pcl_test.hh"
#endif

#ifdef TEST_PICO_TREE
    #include "../benchmark/all_libraries/pico_tree_test.hh"
#endif

#ifdef TEST_RTREE_TEMPLATE
    #include "../benchmark/all_libraries/rtree_template_test.hh"
#endif

#ifdef TEST_SPATIAL
    #include "../benchmark/all_libraries/spatial_test.hh"
#endif

#endif //RANGE_TREE_LIBRARIES_HH