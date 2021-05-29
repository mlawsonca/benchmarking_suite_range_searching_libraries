#include "range_tree_libraries.hh"
#include "perform_queries.hh"

#ifndef NUM_ELEMS_PER_NODE
    #error Your need to define NUM_ELEMS_PER_NODE in a common header file
#endif 

#ifndef LARGE_NUM_ELEMS_PER_NODE
    #error Your need to define LARGE_NUM_ELEMS_PER_NODE in a common header file
#endif


void test_brute_force_bboxes(const std::vector<point> &pts_bbox, const std::vector<size_t> &indices_bbox, testing_config config, 
    const std::vector<std::vector<size_t>> &element_node_ids) 
{
    switch(config.library_option) {
        case 0: {
            string test_name = "Brute Force Bboxes";
            TestBruteForce *test_brute_force_bboxes = new TestBruteForce();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_brute_force_bboxes->build_tree(pts_bbox, indices_bbox);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_brute_force_bboxes, test_name, pts_bbox, indices_bbox, config, STANDARD, element_node_ids);
            break;
        }
        default : {
            cout << "error. test_brute_force_bboxes was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}

#ifdef TEST_BOOST
void test_boost_bboxes(const std::vector<point> &pts_bbox, const std::vector<size_t> &indices_bbox, testing_config config, 
    const std::vector<std::vector<size_t>> &element_node_ids) 
{
    switch(config.library_option) {
        case 0: {
            string test_name = "Boost Bboxes";
            auto test_boost4 = new TestBoost<boost::geometry::index::linear<NUM_ELEMS_PER_NODE>>();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_boost4->build_tree_bbox(pts_bbox, indices_bbox);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_boost4, test_name, pts_bbox, indices_bbox, config, STANDARD, element_node_ids);
            break;
        }
        case 1: {
            string test_name = "Boost Bboxes Bucket Size = " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);;
            auto test_boost4 = new TestBoost<boost::geometry::index::linear<LARGE_NUM_ELEMS_PER_NODE>>();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_boost4->build_tree_bbox(pts_bbox, indices_bbox);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_boost4, test_name, pts_bbox, indices_bbox, config, STANDARD, element_node_ids);
            break;
        }
        default : {
            cout << "error. test_boost_bboxes was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif

#ifdef TEST_CGAL
void test_cgal_bboxes(const std::vector<point> &pts_bbox, const std::vector<size_t> &indices_bbox, testing_config config, 
    const std::vector<std::vector<size_t>> &element_node_ids) 
{
    switch(config.library_option) {
        case 0 : {
            string test_name = "CGAL Segment Tree Bboxes";
            TestCGAL::SegmentTree *test_cgal_segment_tree = new TestCGAL::SegmentTree();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_cgal_segment_tree->build_tree(pts_bbox, indices_bbox);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_cgal_segment_tree, test_name, pts_bbox, indices_bbox, config, STANDARD, element_node_ids);
            break;
        }
        case 1 : {
            string test_name = "CGAL AABBTree Bboxes";
            TestCGAL::AABBTree::Bboxes *test_cgal_aabb_tree_bboxes = new TestCGAL::AABBTree::Bboxes();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_cgal_aabb_tree_bboxes->build_tree(pts_bbox, indices_bbox);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_cgal_aabb_tree_bboxes, test_name, pts_bbox, indices_bbox, config, STANDARD, element_node_ids);
            break;
        }
        default : {
            cout << "error. test_cgal_bboxes was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif

#ifdef TEST_LIBSPATIALINDEX
void test_libspatialindex_bboxes(const std::vector<point> &pts_bbox, const std::vector<size_t> &indices_bbox, testing_config config, 
    const std::vector<std::vector<size_t>> &element_node_ids) 
{
    switch(config.library_option) {
        case 0 : {
            string test_name = "Libspatialindex Bboxes";
            TestLibspatialindex *test_libspatialindex = new TestLibspatialindex();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree_bbox(pts_bbox, indices_bbox);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_libspatialindex, test_name, pts_bbox, indices_bbox, config, STANDARD, element_node_ids);
            break;
        }
        case 1 : {
            string test_name = "Libspatialindex Bboxes Bucket Size = " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);;
            TestLibspatialindex *test_libspatialindex = new TestLibspatialindex();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree_bbox(pts_bbox, indices_bbox, LARGE_NUM_ELEMS_PER_NODE);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_libspatialindex, test_name, pts_bbox, indices_bbox, config, STANDARD, element_node_ids);
            break;
        }
        case 2 : {
            string test_name = "Libspatialindex Bboxes linear";
            TestLibspatialindex *test_libspatialindex = new TestLibspatialindex();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree_bbox_linear(pts_bbox, indices_bbox);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_libspatialindex, test_name, pts_bbox, indices_bbox, config, STANDARD, element_node_ids);
            break;
        }
        case 3 : {
            string test_name = "Libspatialindex Bboxes quadratic";
            TestLibspatialindex *test_libspatialindex = new TestLibspatialindex();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree_bbox_quadratic(pts_bbox, indices_bbox);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_libspatialindex, test_name, pts_bbox, indices_bbox, config, STANDARD, element_node_ids);
            break;
        }
        default : {
            cout << "error. test_libspatialindex_bboxes was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif

#ifdef TEST_RTREE_TEMPLATE
void test_rtree_template_bboxes(const std::vector<point> &pts_bbox, const std::vector<size_t> &indices_bbox, testing_config config, 
    const std::vector<std::vector<size_t>> &element_node_ids) 
{
    switch(config.library_option) {
        case 0 : {
            string test_name = "Rtree Template Bboxes";
            TestRtreeTemplate::Bboxes<NUM_ELEMS_PER_NODE> *test_rtree_template_bboxes;
            test_rtree_template_bboxes = new TestRtreeTemplate::Bboxes<NUM_ELEMS_PER_NODE>();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_rtree_template_bboxes->build_tree(pts_bbox, indices_bbox);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_rtree_template_bboxes, test_name, pts_bbox, indices_bbox, config, STANDARD, element_node_ids);
            break;
        }
        case 1 : {
            string test_name = "Rtree Template Bboxes Bucket Size = " + std::to_string(LARGE_NUM_ELEMS_PER_NODE);;
            TestRtreeTemplate::Bboxes<NUM_ELEMS_PER_NODE> *test_rtree_template_bboxes_LARGE_NUM_ELEMS_PER_NODE;
            test_rtree_template_bboxes_LARGE_NUM_ELEMS_PER_NODE = new TestRtreeTemplate::Bboxes<NUM_ELEMS_PER_NODE>();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_rtree_template_bboxes_LARGE_NUM_ELEMS_PER_NODE->build_tree(pts_bbox, indices_bbox);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_rtree_template_bboxes_LARGE_NUM_ELEMS_PER_NODE, test_name, pts_bbox, indices_bbox, config, STANDARD, element_node_ids);
            break;
        }
        default : {
            cout << "error. test_rtree_template_bboxes was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif

#ifdef TEST_SPATIAL
void test_spatial_bboxes(const std::vector<point> &pts_bbox, const std::vector<size_t> &indices_bbox, testing_config config, 
    const std::vector<std::vector<size_t>> &element_node_ids) 
{
    switch(config.library_option) {
        case 0: {
            string test_name = "Spatial Bboxes";
            //note - could test, for both points and bboxes if the self-balancing version is faster, but it seems unlikely
            TestSpatial::Bboxes *test_spatial_bboxes;
            test_spatial_bboxes = new TestSpatial::Bboxes();
            std::chrono::high_resolution_clock::time_point build_start_time = std::chrono::high_resolution_clock::now();
            test_spatial_bboxes->build_tree(pts_bbox, indices_bbox);
            print_build_time(test_name, build_start_time, config);
            perform_queries(test_spatial_bboxes, test_name, pts_bbox, indices_bbox, config, STANDARD, element_node_ids);
            break;
        }
        default : {
            cout << "error. test_spatial_bboxes was run with option: " << config.library_option << ", which exceeds the maximum expected value" << endl;            
        }
    }
}
#endif



