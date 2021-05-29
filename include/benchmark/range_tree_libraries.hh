#ifndef RANGE_TREE_LIBRARIES_HH
#define RANGE_TREE_LIBRARIES_HH

#include "common.hh"

#include "all_libraries/brute_force_test.hh"

#ifdef TEST_3DTK
    #include "all_libraries/3dtk_test.hh"
#endif

#ifdef TEST_ALGLIB
    #include "all_libraries/alglib_test.hh"
#endif

#ifdef TEST_ANN
    #include "all_libraries/ann_test.hh"
#endif

#ifdef TEST_BOOST
    #include "all_libraries/boost_test.hh"
#endif

#ifdef TEST_CGAL
    #include "all_libraries/cgal_test.hh"
#endif

#ifdef TEST_FLANN
    #include "all_libraries/flann_test.hh"
#endif

#ifdef TEST_KDTREE
    #include "all_libraries/kdtree_test.hh"
#endif

#ifdef TEST_KDTREE2
    #include "all_libraries/kdtree2_test.hh"
#endif

#ifdef TEST_KDTREE3
    #include "all_libraries/kdtree3_test.hh"
#endif

#ifdef TEST_KDTREE4
    #include "all_libraries/kdtree4_test.hh"
#endif

#ifdef TEST_LIBKDTREE
    #include "all_libraries/libkdtree_test.hh"
#endif

#ifdef TEST_LIBKDTREE2
    #include "all_libraries/libkdtree2_test.hh"
#endif

#ifdef TEST_LIBNABO
    #include "all_libraries/libnabo_test.hh"
#endif

#ifdef TEST_LIBSPATIALINDEX
    #include "all_libraries/libspatialindex_test.hh"
#endif

#ifdef TEST_NANOFLANN
    #include "all_libraries/nanoflann_test.hh"
#endif

#ifdef TEST_OCTREE
    #include "all_libraries/octree_test.hh"
#endif

#ifdef TEST_PCL
    #include "all_libraries/pcl_test.hh"
#endif

#ifdef TEST_PICO_TREE
    #include "all_libraries/pico_tree_test.hh"
#endif

#ifdef TEST_RTREE_TEMPLATE
    #include "all_libraries/rtree_template_test.hh"
#endif

#ifdef TEST_SPATIAL
    #include "all_libraries/spatial_test.hh"
#endif

#endif //RANGE_TREE_LIBRARIES_HH