#include "range_tree_libraries.hh"
#include "perform_queries.hh"

#ifndef NUM_ELEMS_PER_NODE
    #error Your need to define NUM_ELEMS_PER_NODE in a common header file
#endif 

#ifndef LARGE_NUM_ELEMS_PER_NODE
    #error Your need to define LARGE_NUM_ELEMS_PER_NODE in a common header file
#endif

void test_brute_force_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "Brute Force";
            TestBruteForce *test_brute_force = new TestBruteForce();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_brute_force->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_brute_force, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_brute_force_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}   
#ifdef TEST_3DTK
void test_3dtk_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "3DTK";
            Test3DTK *test_3dtk = new Test3DTK();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_3dtk->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_3dtk, test_name, pts, indices, config);
            break;
        }
        case 1: {
            string test_name = "3DTK Bucket Size = " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            Test3DTK *test_3dtk = new Test3DTK();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_3dtk->build_tree(pts, indices, LARGE_NUM_ELEMS_PER_NODE);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_3dtk, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_3dtk_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;
        }
    }
}
#endif 

#ifdef TEST_ALGLIB
void test_alglib_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "ALGLIB";
            TestAlglib *test_alglib = new TestAlglib();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_alglib->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_alglib, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_alglib_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}    
#endif

#ifdef TEST_ANN
void test_ann_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "ANN";
            TestANN *test_ann = new TestANN();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_ann->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_ann, test_name, pts, indices, config);
            break;
        }
        case 1: {
            string test_name = "ANN Bucket Size = " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            TestANN *test_ann = new TestANN();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_ann->build_tree(pts, indices, LARGE_NUM_ELEMS_PER_NODE);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_ann, test_name, pts, indices, config);
            break;
        }
        case 2 : {
            string test_name = "ANN BD Tree";
            TestANN *test_ann_bd_tree = new TestANN();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_ann_bd_tree->build_tree_bd(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_ann_bd_tree, test_name, pts, indices, config);
            break;
        }
        case 3 : {
            string test_name = "ANN BD Tree Bucket Size =  " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            TestANN *test_ann_bd_tree = new TestANN();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_ann_bd_tree->build_tree_bd(pts, indices, LARGE_NUM_ELEMS_PER_NODE);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_ann_bd_tree, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_ann_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif 

#ifdef TEST_BOOST
void test_boost_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "Boost";
           auto test_boost0 = new TestBoost<boost::geometry::index::linear<NUM_ELEMS_PER_NODE>>();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_boost0->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_boost0, test_name, pts, indices, config);
            break;
        }
        case 1: {
            string test_name = "Boost Bucket Size =  " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            auto test_boost1 = new TestBoost<boost::geometry::index::linear<LARGE_NUM_ELEMS_PER_NODE>>();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_boost1->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_boost1, test_name, pts, indices, config);
            break;
        }
        case 2 : {
            string test_name = "Boost Quadratic";
            auto test_boost2 = new TestBoost<boost::geometry::index::quadratic<NUM_ELEMS_PER_NODE>>();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_boost2->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_boost2, test_name, pts, indices, config);
            break;
        }
        case 3 : {
            string test_name = "Boost Rstar";
            auto test_boost3 = new TestBoost<boost::geometry::index::rstar<NUM_ELEMS_PER_NODE>>();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_boost3->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_boost3, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_boost_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif 

#ifdef TEST_CGAL
void test_cgal_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "CGAL Kdtree";
            TestCGAL::KDTree *test_cgal_kdtree = new TestCGAL::KDTree();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_cgal_kdtree->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_cgal_kdtree, test_name, pts, indices, config);
            break;
        }
        case 1: {
            string test_name = "CGAL Kdtree Bucket Size =  " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            TestCGAL::KDTree *test_cgal_kdtree2 = new TestCGAL::KDTree();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_cgal_kdtree2->build_tree(pts, indices, LARGE_NUM_ELEMS_PER_NODE);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_cgal_kdtree2, test_name, pts, indices, config);
            break;
        }
        case 2 : {
            string test_name = "CGAL Range Tree";
            TestCGAL::RangeTree *test_cgal_range_tree = new TestCGAL::RangeTree();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_cgal_range_tree->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_cgal_range_tree, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_cgal_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif

#ifdef TEST_FLANN
void test_flann_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "FLANN Kdtree";
            TestFLANN::KDTree *test_flann_kdtree = new TestFLANN::KDTree();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_flann_kdtree->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_flann_kdtree, test_name, pts, indices, config);
            break;
        }
        case 1: {
            string test_name = "FLANN Kdtree Bucket Size =  " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            TestFLANN::KDTree *test_flann_kdtree = new TestFLANN::KDTree();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_flann_kdtree->build_tree(pts, indices, LARGE_NUM_ELEMS_PER_NODE);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_flann_kdtree, test_name, pts, indices, config);
            break;
        }
        case 2 : {
            string test_name = "FLANN Kdtree CUDA";
            TestFLANN::CUDA *test_flann_kdtree_cuda = new TestFLANN::CUDA();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_flann_kdtree_cuda->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_flann_kdtree_cuda, test_name, pts, indices, config);
            break;
        }
        case 3 : {
            string test_name = "FLANN Kdtree CUDA Bucket Size =  " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            TestFLANN::CUDA *test_flann_kdtree_cuda = new TestFLANN::CUDA();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_flann_kdtree_cuda->build_tree(pts, indices, LARGE_NUM_ELEMS_PER_NODE);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_flann_kdtree_cuda, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_flann_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif 

#ifdef TEST_KDTREE
void test_kdtree_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "KDTree";
            TestKDTree *test_ktree = new TestKDTree();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_ktree->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_ktree, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_kdtree_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif

#ifdef TEST_KDTREE2
void test_kdtree2_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "KDTree2";
            TestKDTree2 *test_ktree2;
            test_ktree2 = new TestKDTree2();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_ktree2->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_ktree2, test_name, pts, indices, config);
            break;
        }
        case 1: {
            string test_name = "KDTree2 Bucket Size =  " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            TestKDTree2 *test_ktree2 = new TestKDTree2();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_ktree2->build_tree(pts, indices, LARGE_NUM_ELEMS_PER_NODE);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_ktree2, test_name, pts, indices, config);

            break;
        }
        case 2 : {
            string test_name = "KDTree2 rearranged points";
            bool rearrange_pts = true;
            TestKDTree2 *test_ktree2 = new TestKDTree2();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_ktree2->build_tree(pts, indices, NUM_ELEMS_PER_NODE, rearrange_pts);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_ktree2, test_name, pts, indices, config);

            break;
        }
        case 3 : {
            string test_name = "KDTree2 rearranged points Bucket Size =  " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            bool rearrange_pts = true;
            TestKDTree2 *test_ktree2 = new TestKDTree2();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_ktree2->build_tree(pts, indices, LARGE_NUM_ELEMS_PER_NODE, rearrange_pts);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_ktree2, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_kdtree2_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif

#ifdef TEST_KDTREE3
void test_kdtree3_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "KDTree3";
            TestKDTree3 *test_ktree3;
            test_ktree3 = new TestKDTree3();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_ktree3->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_ktree3, test_name, pts, indices, config);
            break;
        }
        case 1: {
            string test_name = "KDTree3 Bucket Size =  " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            TestKDTree3 *test_ktree3 = new TestKDTree3();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_ktree3->build_tree(pts, indices, LARGE_NUM_ELEMS_PER_NODE);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_ktree3, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_kdtree3_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif

#ifdef TEST_KDTREE4
void test_kdtree4_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "KDTree4";
            TestKDTree4 *test_ktree4 = new TestKDTree4();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_ktree4->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_ktree4, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_kdtree4_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif

#ifdef TEST_LIBKDTREE
void test_libkdtree_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "Libkdtree";
            TestLibkdtree *test_libkdtree = new TestLibkdtree();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_libkdtree->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_libkdtree, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_libkdtree_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif

#ifdef TEST_LIBKDTREE2
void test_libkdtree2_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "Libkdtree2";
            TestLibkdtree2 *test_libkdtree2 = new TestLibkdtree2();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_libkdtree2->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_libkdtree2, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_libkdtree2_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif

#ifdef TEST_LIBNABO
void test_libnabo_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "Libnabo";
            TestLibnabo *test_libnabo = new TestLibnabo();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_libnabo->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_libnabo, test_name, pts, indices, config);
            break;
        }
        case 1: {
            string test_name = "Libnabo Bucket Size =  " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            TestLibnabo *test_libnabo = new TestLibnabo();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_libnabo->build_tree(pts, indices, true, LARGE_NUM_ELEMS_PER_NODE);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_libnabo, test_name, pts, indices, config);
            break;
        }
        case 2 : {
            string test_name = "Libnabo Tree Heap";
            TestLibnabo *test_libnabo = new TestLibnabo();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_libnabo->build_tree(pts, indices, false);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_libnabo, test_name, pts, indices, config);
            break;
        }
        case 3 : {
            string test_name = "Libnabo Tree Heap Bucket Size =  " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            TestLibnabo *test_libnabo = new TestLibnabo();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_libnabo->build_tree(pts, indices, false, LARGE_NUM_ELEMS_PER_NODE);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_libnabo, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_libnabo_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif

#ifdef TEST_LIBSPATIALINDEX
void test_libspatialindex_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "Libspatialindex";
            TestLibspatialindex *test_libspatialindex;
            test_libspatialindex = new TestLibspatialindex();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_libspatialindex, test_name, pts, indices, config);
            break;
        }
        case 1: {
            string test_name = "Libspatialindex Bucket Size =  " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            TestLibspatialindex *test_libspatialindex = new TestLibspatialindex();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree(pts, indices, LARGE_NUM_ELEMS_PER_NODE);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_libspatialindex, test_name, pts, indices, config);
            break;
        }
        case 2 : {
            string test_name = "Libspatialindex linear";
            TestLibspatialindex *test_libspatialindex = new TestLibspatialindex();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree_linear(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_libspatialindex, test_name, pts, indices, config);
            break;
        }
        case 3 : {
            string test_name = "Libspatialindex quadratic";
            TestLibspatialindex *test_libspatialindex = new TestLibspatialindex();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree_quadratic(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_libspatialindex, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_libspatialindex_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif

#ifdef TEST_NANOFLANN
void test_nanoflann_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "Nanoflann";
            TestNanoflann *test_nanoflann = new TestNanoflann();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_nanoflann->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_nanoflann, test_name, pts, indices, config);
            break;
        }
        case 1: {
            string test_name = "Nanoflann Bucket Size =  " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            TestNanoflann *test_nanoflann = new TestNanoflann();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_nanoflann->build_tree(pts, indices, LARGE_NUM_ELEMS_PER_NODE);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_nanoflann, test_name, pts, indices, config);
            break;
        }
        case 2: {
            string test_name = "Nanoflann Duplicated Storage";
            TestNanoflann *test_nanoflann = new TestNanoflann();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_nanoflann->build_tree(pts, indices, NUM_ELEMS_PER_NODE, true);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_nanoflann, test_name, pts, indices, config);
            break;
        }
        case 3: {
            string test_name = "Nanoflann Duplicated Storage Bucket Size =  " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            TestNanoflann *test_nanoflann = new TestNanoflann();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_nanoflann->build_tree(pts, indices, LARGE_NUM_ELEMS_PER_NODE, true);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_nanoflann, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_nanoflann_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif 

#ifdef TEST_OCTREE
void test_octree_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "Octree";
            TestOctree *test_octree = new TestOctree();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_octree->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_octree, test_name, pts, indices, config);
            break;
        }
        case 1: {
            string test_name = "Octree Bucket Size =  " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            TestOctree *test_octree = new TestOctree();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_octree->build_tree(pts, indices, LARGE_NUM_ELEMS_PER_NODE);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_octree, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_octree_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif

#ifdef TEST_PCL
void test_pcl_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    double total_domain_volume = 1;
    for(size_t i = 0; i < config.domain_lower_bounds.size(); i++) {
        total_domain_volume *= (config.domain_upper_bounds[i]-config.domain_lower_bounds[i]);
    }

    size_t num_data_pts = pts.size();
    double avg_voxel_volume_per_data_pt = 0;
    if(num_data_pts > 0) {
        avg_voxel_volume_per_data_pt = total_domain_volume/(double)num_data_pts;
    }
    avg_voxel_volume_per_data_pt = 1;

    if(DEBUG) {
        cout << "avg_voxel_volume_per_data_pt: " << avg_voxel_volume_per_data_pt << endl;
    }
    switch(config.library_option) {
        case 0: {
            string test_name = "PCL Octree Resolution for Bucket size approx=  " + std::to_string(NUM_ELEMS_PER_NODE);
            //set resolution/voxel length to cbrt(default bucket size*avg voxel volume per data point) to get num data points per leaf approx=large bucket size
            TestPCL::Octree *test_pcl = new TestPCL::Octree();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_pcl->build_tree(pts, indices, NUM_ELEMS_PER_NODE*avg_voxel_volume_per_data_pt);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_pcl, test_name, pts, indices, config);
            break;
        }
        case 1: {
            string test_name = "PCL Octree Resolution for Bucket Size approx=  " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            //set resolution/voxel length to cbrt(large bucket size*avg voxel volume per data point) to get num data points per leaf approx=large bucket size
            TestPCL::Octree *test_pcl = new TestPCL::Octree();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_pcl->build_tree(pts, indices, LARGE_NUM_ELEMS_PER_NODE*avg_voxel_volume_per_data_pt);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_pcl, test_name, pts, indices, config);

            break;
        }
        case 2 : {
            string test_name = "PCL KDTree";
            //KDtree is just a wrapper around FLANN, no added functionality. shouldn't expect it to be faster
            //library doesn't support setting setting bucket size for kd tree, default is 15
            TestPCL::KDTree *test_pcl_kdtree = new TestPCL::KDTree();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_pcl_kdtree->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_pcl_kdtree, test_name, pts, indices, config);
            break;
        }
        case 3 : {
            string test_name = "PCL OctreeGPU";
            TestPCL::OctreeGPU *test_pcl_octree_gpu = new TestPCL::OctreeGPU();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_pcl_octree_gpu->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_pcl_octree_gpu, test_name, pts, indices, config);
            break;
        }
        case 4 : {
            string test_name = "PCL OctreeGPU query domain decomposition";
            TestPCL::OctreeGPU *test_pcl_octree_gpu = new TestPCL::OctreeGPU();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_pcl_octree_gpu->build_tree(pts, indices);
            //doesn't affect how the tree builds just how you query it
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_pcl_octree_gpu, test_name, pts, indices, config, GPU_DOMAIN_DECOMP);
            break;
        }
        default : {
            cout << "error. test_pcl_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif

#ifdef TEST_PICO_TREE
void test_pico_tree_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "Pico tree";
            TestPicoTree *test_pico_tree;
            test_pico_tree = new TestPicoTree();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_pico_tree->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_pico_tree, test_name, pts, indices, config);
            break;
        }
        case 1: {
            string test_name = "Pico tree Bucket Size =  " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            TestPicoTree *test_pico_tree = new TestPicoTree();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_pico_tree->build_tree(pts, indices, LARGE_NUM_ELEMS_PER_NODE);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_pico_tree, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_pico_tree_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif

#ifdef TEST_RTREE_TEMPLATE
void test_rtree_template_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "Rtree template";
            TestRtreeTemplate::Points<NUM_ELEMS_PER_NODE> *test_rtree_template;
            test_rtree_template = new TestRtreeTemplate::Points<NUM_ELEMS_PER_NODE>();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_rtree_template->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_rtree_template, test_name, pts, indices, config);
            break;
        }
        case 1: {
            string test_name = "Rtree template Bucket Size =  " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);
            TestRtreeTemplate::Points<LARGE_NUM_ELEMS_PER_NODE> *test_rtree_template_LARGE_NUM_ELEMS_PER_NODE;
            test_rtree_template_LARGE_NUM_ELEMS_PER_NODE = new TestRtreeTemplate::Points<LARGE_NUM_ELEMS_PER_NODE>();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_rtree_template_LARGE_NUM_ELEMS_PER_NODE->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_rtree_template_LARGE_NUM_ELEMS_PER_NODE, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_rtree_template_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif

#ifdef TEST_SPATIAL
void test_spatial_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config) {
    switch(config.library_option) {
        case 0: {
            string test_name = "Spatial";
            TestSpatial::Points *test_spatial;
            test_spatial = new TestSpatial::Points();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_spatial->build_tree(pts, indices);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_spatial, test_name, pts, indices, config);
            break;
        }
        default : {
            cout << "error. test_spatial_points was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif



