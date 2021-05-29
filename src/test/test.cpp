#include <stdlib.h>
#include <iostream>
#include "range_tree_libraries.hh"
#include <algorithm> /* sort */
#include <chrono> /* std::chrono::high_resolution_clock */

using namespace std;

#define IS_BBOX true
#define OUTPUT_TIMING_RESULTS true

#ifndef NUM_ELEMS_PER_NODE
    #error Your need to define NUM_ELEMS_PER_NODE in a common header file
#endif 

#ifndef DOMAIN_LENGTH
    #error Your need to define DOMAIN_LENGTH in a common header file
#endif

#ifndef DEBUG
    #define DEBUG false
#endif

#ifdef LARGE_TEST
    #include <boost/random.hpp> //needed for random range generation
#endif

void run_tests(BboxIntersectionTest *test, const string &test_name, const std::vector<bbox> &query_bboxes,
    const std::vector<point> &pts, const std::vector<size_t> &indices, const vector<vector<size_t>> &correct_results,
    bool is_bboxes = false, bool delete_tree = true
    );



bool check_intersection(const bbox &bounding_box, const point &pt) {
    return(
        bounding_box.first[0] <= pt[0] && 
        bounding_box.second[0] >= pt[0] && 
        bounding_box.first[1] <= pt[1] && 
        bounding_box.second[1] >= pt[1] && 
        bounding_box.first[2] <= pt[2] && 
        bounding_box.second[2] >= pt[2]
    );
}

int main(int argc, char **argv) {
    std::vector<point> pts;
    std::vector<size_t> indices;
    std::vector<point> triangle_pts;
    std::vector<size_t> triangle_indices;
    std::vector<point> bbox_pts;
    std::vector<size_t> bbox_indices;
    std::vector<bbox> query_bboxes;
    const size_t large_bucket_size = 50;
    vector<vector<size_t>> brute_force_results;
    vector<vector<size_t>> brute_force_results_bboxes;
    double total_domain_volume;
    size_t num_data_pts;

    #if OUTPUT_TIMING_RESULTS
        std::chrono::high_resolution_clock::time_point build_start_time;
        std::chrono::high_resolution_clock::time_point build_stop_time;
    #endif

    #ifdef LARGE_TEST
        double domain_length_x = 25.0;
        double domain_length_y = 50.0;
        double domain_length_z = 100.0;
        if(argc > 1) {
            num_data_pts = stoi(argv[1]);
            cout << "setting num_data_pts=" << num_data_pts << endl;
        }
        else {
            num_data_pts = 100000; 
        }
        size_t num_queries = 100;

        boost::mt19937 rng;
        rng.seed(100);

        total_domain_volume = domain_length_x*domain_length_y*domain_length_z;

        boost::uniform_real<double> range_x(-(domain_length_x/2), domain_length_x/2);
        boost::uniform_real<double> range_y(-(domain_length_y/2), domain_length_y/2);
        boost::uniform_real<double> range_z(-(domain_length_z/2), domain_length_z/2);
        boost::uniform_real<double> range_x_length(domain_length_x/1000, domain_length_x/10);
        boost::uniform_real<double> range_y_length(domain_length_y/1000, domain_length_y/10);
        boost::uniform_real<double> range_z_length(domain_length_z/1000, domain_length_z/10);

        boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd_x(rng, range_x);
        boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd_y(rng, range_y);
        boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd_z(rng, range_z);
        boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd_length_x(rng, range_x_length);
        boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd_length_y(rng, range_y_length);
        boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd_length_z(rng, range_z_length);

        cout << "about to make points" << endl;
        //do these separately so that if, for a given library, you don't need to test the bboxes or triangles option, 
        //not generating the points for these options won't change the values for the points test
        for(size_t i = 0; i < num_data_pts; i++) {
            double x = rnd_x();
            double y = rnd_y();
            double z = rnd_z();
            pts.push_back(point({x, y, z}));
            indices.push_back(i);

            if(DEBUG) {
                if(i < 100) {
                    cout << "pt: ";
                    print_point(pts.back());
                }
            }
        }
        for(size_t i = 0; i < num_data_pts; i++) {
            double x = rnd_x();
            double y = rnd_y();
            double z = rnd_z();
            double x_len = rnd_length_x();
            double y_len = rnd_length_y();
            double z_len = rnd_length_z();
            bbox_pts.push_back(point({x, y, z}));
            bbox_pts.push_back(point({x+x_len, y+y_len, z+z_len}));
            bbox_indices.push_back(i);
            if(DEBUG) {
                if(i < 100) {
                    cout << "box: ";
                    print_bbox(bbox(bbox_pts[bbox_pts.size()-2], bbox_pts.back()));
                }
            }
        }
        for(size_t i = 0; i < num_data_pts; i++) {
            double x = rnd_x();
            double y = rnd_y();
            double z = rnd_z();
            double x_len = rnd_length_x();
            double y_len = rnd_length_y();
            double z_len = rnd_length_z();
            double x_len2 = rnd_length_x();
            double y_len2 = rnd_length_y();
            double z_len2 = rnd_length_z();
            triangle_pts.push_back(point({x, y, z}));
            triangle_pts.push_back(point({x+x_len, y+y_len, z+z_len}));
            triangle_pts.push_back(point({x+x_len2, y+y_len2, z+z_len2}));
            triangle_indices.push_back(i);
            if(DEBUG) {
                if(i < 100) {
                    cout << "triangle: " << endl;
                    print_point(triangle_pts[triangle_pts.size()-3]);
                    print_point(triangle_pts[triangle_pts.size()-2]);
                    print_point(triangle_pts[triangle_pts.size()-1]);
                }
            }
        }

        for(size_t i = 0; i < num_queries; i++) {
            double x = rnd_x();
            double y = rnd_y();
            double z = rnd_z();
            double x_len = rnd_length_x();
            double y_len = rnd_length_y();
            double z_len = rnd_length_z();
            query_bboxes.push_back(bbox(point({x, y, z}),point({x+x_len, y+y_len, z+z_len})));
            if(DEBUG) {
                cout << "query: ";
                print_bbox(query_bboxes.back());
            }
        }
        cout << "done with make points" << endl;
    #else
        if(DEBUG) {
            cout << "about to create points" << endl;
        }
        total_domain_volume = DOMAIN_LENGTH*DOMAIN_LENGTH*DOMAIN_LENGTH;
        num_data_pts = DOMAIN_LENGTH*DOMAIN_LENGTH*DOMAIN_LENGTH;

        int index = 0;
        int bbox_index = 0;
        for(double i = 0; i < DOMAIN_LENGTH; i++) {
            for(double j = 0; j < DOMAIN_LENGTH; j++) {
                for(double k = 0; k < DOMAIN_LENGTH; k++) {
                    pts.push_back({i, j, k});
                    indices.push_back(index);
                    index += 1;
                    if(i < 2 && j < 2 && k < 2) {
                        bbox_pts.push_back({i, j, k});
                        bbox_pts.push_back({i+1, j+1, k+1});
                        bbox_indices.push_back(bbox_index);   
                        bbox_index += 1;
                    }
                }
            }
        }
        for(int i = 0; i < (pts.size()/3); i++) {
            triangle_pts.push_back(pts[i]);
            triangle_pts.push_back(pts[i+DOMAIN_LENGTH]);
            triangle_pts.push_back(pts[i+DOMAIN_LENGTH*DOMAIN_LENGTH]);

            triangle_indices.push_back(i);
        } 

        if(DEBUG) {
            cout << "about to create query boxes" << endl;
        }

        query_bboxes.push_back(std::make_pair(point({0,0,0}),point({1,1,1})));
        query_bboxes.push_back(std::make_pair(point({.5,.5,.5}),point({1,1,1})));
    #endif

    cout << "about to test libraries" << endl;

    TestBruteForce *test_brute_force = new TestBruteForce();
    brute_force_results.resize(query_bboxes.size());
    test_brute_force->build_tree(pts, indices);
    for(size_t i = 0; i < query_bboxes.size(); i++) {
        test_brute_force->get_intersections(query_bboxes[i], brute_force_results[i]);
        //sort the results so that later it is easier to check if the tests equal the brute force results
        std::sort(brute_force_results[i].begin(), brute_force_results[i].end());
    }

    TestBruteForce *test_brute_force_bboxes = new TestBruteForce();
    brute_force_results_bboxes.resize(query_bboxes.size());
    test_brute_force_bboxes->build_tree(bbox_pts, bbox_indices);
    for(size_t i = 0; i < query_bboxes.size(); i++) {
        test_brute_force_bboxes->get_intersections_bboxes(query_bboxes[i], brute_force_results_bboxes[i]);
        //sort the results so that later it is easier to check if the tests equal the brute force results
        std::sort(brute_force_results_bboxes[i].begin(), brute_force_results_bboxes[i].end());
    }
    cout << "done with brute force solution" << endl;

    if(DEBUG) {
        cout << endl << "results: points" << endl;
        for(size_t i = 0; i < brute_force_results.size(); i++) {
            cout << "results for query " << i << endl;
            for(size_t j = 0; j < brute_force_results[i].size(); j++) {
                size_t index = brute_force_results[i][j];
                cout << "index: " << index << ", point: ";
                print_point(pts[index]);
            }
            cout << endl;
        }
        cout << endl;

        cout << "results: bboxes" << endl;
        for(size_t i = 0; i < brute_force_results_bboxes.size(); i++) {
            cout << "results for query " << i << endl;
            for(size_t j = 0; j < brute_force_results_bboxes[i].size(); j++) {
                size_t index = brute_force_results_bboxes[i][j];
                cout << "index: " << 2*index << ", bbox: ";
                print_bbox(bbox(bbox_pts[2*index], bbox_pts[2*index+1]));
            }
            cout << endl;
        }
        cout << endl;
    }


    #ifdef TEST_ALGLIB
        TestAlglib *test_alglib = new TestAlglib();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_alglib->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "ALGLIB build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_alglib->build_tree(pts, indices);
        #endif
        run_tests(test_alglib, "ALGLIB", query_bboxes, pts, indices, brute_force_results);

    #endif

    #ifdef TEST_ANN
        TestANN *test_ann = new TestANN();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_ann->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "ANN build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_ann->build_tree(pts, indices);
        #endif
        run_tests(test_ann, "ANN", query_bboxes, pts, indices, brute_force_results);



        test_ann = new TestANN();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_ann->build_tree(pts, indices, large_bucket_size);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "ANN Bucket size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_ann->build_tree(pts, indices, large_bucket_size);
        #endif
        run_tests(test_ann, "ANN Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);

        TestANN *test_ann_bd_tree = new TestANN();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_ann_bd_tree->build_tree_bd(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "ANN BD tree build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_ann_bd_tree->build_tree_bd(pts, indices);
        #endif
        run_tests(test_ann_bd_tree, "ANN BD Tree", query_bboxes, pts, indices, brute_force_results);

        test_ann_bd_tree = new TestANN();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_ann_bd_tree->build_tree_bd(pts, indices, large_bucket_size);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "ANN BD tree Bucket Size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_ann_bd_tree->build_tree_bd(pts, indices, large_bucket_size);
        #endif
        run_tests(test_ann_bd_tree, "ANN BD Tree Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);

    #endif

    #ifdef TEST_BOOST
        auto test_boost = new TestBoost<boost::geometry::index::linear<NUM_ELEMS_PER_NODE>>();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_boost->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Boost build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_boost->build_tree(pts, indices);
        #endif
        run_tests(test_boost, "Boost", query_bboxes, pts, indices, brute_force_results);

        auto test_boost2 = new TestBoost<boost::geometry::index::linear<large_bucket_size>>();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_boost2->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Boost Bucket Size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_boost2->build_tree(pts, indices);
        #endif
        run_tests(test_boost2, "Boost Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);

        auto test_boost3 = new TestBoost<boost::geometry::index::quadratic<NUM_ELEMS_PER_NODE>>();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_boost3->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Boost Quadratic build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_boost3->build_tree(pts, indices);
        #endif
        run_tests(test_boost3, "Boost Quadratic", query_bboxes, pts, indices, brute_force_results);

        auto test_boost4 = new TestBoost<boost::geometry::index::rstar<NUM_ELEMS_PER_NODE>>();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_boost4->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Boost Rstar build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_boost4->build_tree(pts, indices);
        #endif
        run_tests(test_boost4, "Boost Rstar", query_bboxes, pts, indices, brute_force_results);

        auto test_boost5 = new TestBoost<boost::geometry::index::linear<NUM_ELEMS_PER_NODE>>();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_boost5->build_tree_bbox(bbox_pts, bbox_indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Boost Boxes build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_boost5->build_tree_bbox(bbox_pts, bbox_indices);
        #endif
        run_tests(test_boost5, "Boost Boxes", query_bboxes, bbox_pts, bbox_indices, brute_force_results_bboxes, IS_BBOX);

        auto test_boost6 = new TestBoost<boost::geometry::index::linear<large_bucket_size>>();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_boost6->build_tree_bbox(bbox_pts, bbox_indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Boost Boxes Bucket Size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_boost6->build_tree_bbox(bbox_pts, bbox_indices);
        #endif
        run_tests(test_boost6, "Boost Boxes Bucket Size = 50", query_bboxes, bbox_pts, bbox_indices, brute_force_results_bboxes, IS_BBOX);

    #endif 

    #ifdef TEST_CGAL
        TestCGAL::KDTree *test_cgal_kdtree = new TestCGAL::KDTree();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_cgal_kdtree->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "CGAL Kdtree build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_cgal_kdtree->build_tree(pts, indices);
        #endif
        run_tests(test_cgal_kdtree, "CGAL Kdtree", query_bboxes, pts, indices, brute_force_results);

        TestCGAL::KDTree *test_cgal_kdtree2 = new TestCGAL::KDTree();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_cgal_kdtree2->build_tree(pts, indices, large_bucket_size);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "CGAL Kdtree Bucket Size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_cgal_kdtree2->build_tree(pts, indices, large_bucket_size);
        #endif
        run_tests(test_cgal_kdtree2, "CGAL Kdtree Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);

        TestCGAL::RangeTree *test_cgal_range_tree = new TestCGAL::RangeTree();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_cgal_range_tree->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "CGAL Range tree build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_cgal_range_tree->build_tree(pts, indices);
        #endif
        run_tests(test_cgal_range_tree, "CGAL Range Tree", query_bboxes, pts, indices, brute_force_results);

        TestCGAL::SegmentTree *test_cgal_segment_tree = new TestCGAL::SegmentTree();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_cgal_segment_tree->build_tree(bbox_pts, bbox_indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "CGAL Segment tree build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_cgal_segment_tree->build_tree(bbox_pts, bbox_indices);
        #endif
        run_tests(test_cgal_segment_tree, "CGAL Segment Tree", query_bboxes, bbox_pts, bbox_indices, brute_force_results_bboxes, IS_BBOX);
 
        TestCGAL::AABBTree::Triangles *test_cgal_aabb_tree = new TestCGAL::AABBTree::Triangles();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_cgal_aabb_tree->build_tree(triangle_pts, triangle_indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "CGAL AABBtree Triangles build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_cgal_aabb_tree->build_tree(triangle_pts, triangle_indices);
        #endif
        run_tests(test_cgal_aabb_tree, "CGAL AABBTree Triangles", query_bboxes, pts, indices, brute_force_results);

        TestCGAL::AABBTree::Bboxes *test_cgal_aabb_tree_bboxes = new TestCGAL::AABBTree::Bboxes();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_cgal_aabb_tree_bboxes->build_tree(bbox_pts, bbox_indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "CGAL AABBtree Bboxes build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_cgal_aabb_tree_bboxes->build_tree(bbox_pts, bbox_indices);
        #endif
        run_tests(test_cgal_aabb_tree_bboxes, "CGAL AABBTree Bboxes", query_bboxes, bbox_pts, bbox_indices, brute_force_results_bboxes, IS_BBOX);

    #endif

    #if TEST_FLANN
        TestFLANN::KDTree *test_flann_kdtree = new TestFLANN::KDTree();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_flann_kdtree->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "FLANN Kdtree build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_flann_kdtree->build_tree(pts, indices);
        #endif
        run_tests(test_flann_kdtree, "FLANN Kdtree", query_bboxes, pts, indices, brute_force_results);

        test_flann_kdtree = new TestFLANN::KDTree();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_flann_kdtree->build_tree(pts, indices, large_bucket_size);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "FLANN Kdtree Bucket size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_flann_kdtree->build_tree(pts, indices, large_bucket_size);
        #endif
        run_tests(test_flann_kdtree, "FLANN Kdtree Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);

        #if USE_GPU
            TestFLANN::CUDA *test_flann_kdtree_cuda = new TestFLANN::CUDA();
            #if OUTPUT_TIMING_RESULTS
                build_start_time = std::chrono::high_resolution_clock::now();
                test_flann_kdtree_cuda->build_tree(pts, indices);
                build_stop_time = std::chrono::high_resolution_clock::now();
                cout << "FLANN Kdtree CUDA build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
            #else
                test_flann_kdtree_cuda->build_tree(pts, indices);
            #endif
            run_tests(test_flann_kdtree_cuda, "FLANN Kdtree CUDA", query_bboxes, pts, indices, brute_force_results);

            test_flann_kdtree_cuda = new TestFLANN::CUDA();
            #if OUTPUT_TIMING_RESULTS
                build_start_time = std::chrono::high_resolution_clock::now();
                test_flann_kdtree_cuda->build_tree(pts, indices, large_bucket_size);
                build_stop_time = std::chrono::high_resolution_clock::now();
                cout << "FLANN Kdtree CUDA Bucket size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
            #else
                test_flann_kdtree_cuda->build_tree(pts, indices, large_bucket_size);
            #endif
            run_tests(test_flann_kdtree_cuda, "FLANN Kdtree CUDA Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);
        #endif
    #endif

    #ifdef TEST_KDTREE
        TestKDTree *test_ktree;
        test_ktree = new TestKDTree();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_ktree->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Kdtree build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_ktree->build_tree(pts, indices);
        #endif
        run_tests(test_ktree, "KDTree", query_bboxes, pts, indices, brute_force_results);
    #endif 

    #ifdef TEST_KDTREE2
        TestKDTree2 *test_ktree2;
        test_ktree2 = new TestKDTree2();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_ktree2->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Kdtree2 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_ktree2->build_tree(pts, indices);
        #endif
        run_tests(test_ktree2, "KDTree2", query_bboxes, pts, indices, brute_force_results);

        test_ktree2 = new TestKDTree2();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_ktree2->build_tree(pts, indices, large_bucket_size);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Kdtree2 Bucket size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_ktree2->build_tree(pts, indices, large_bucket_size);
        #endif
        run_tests(test_ktree2, "KDTree2 Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);

        bool rearrange_pts = true;
        test_ktree2 = new TestKDTree2();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_ktree2->build_tree(pts, indices, NUM_ELEMS_PER_NODE, rearrange_pts);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Kdtree2 rearranged points build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_ktree2->build_tree(pts, indices, NUM_ELEMS_PER_NODE, rearrange_pts);
        #endif
        run_tests(test_ktree2, "KDTree2 rearranged points", query_bboxes, pts, indices, brute_force_results);

        test_ktree2 = new TestKDTree2();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_ktree2->build_tree(pts, indices, large_bucket_size, rearrange_pts);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Kdtree2 rearranged points bucket size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_ktree2->build_tree(pts, indices, large_bucket_size, rearrange_pts);
        #endif
        run_tests(test_ktree2, "KDTree2 rearranged points Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);

    #endif 

    #ifdef TEST_KDTREE3
        TestKDTree3 *test_ktree3;
        test_ktree3 = new TestKDTree3();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_ktree3->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Kdtree3 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_ktree3->build_tree(pts, indices);
        #endif
        run_tests(test_ktree3, "KDTree3", query_bboxes, pts, indices, brute_force_results);

        test_ktree3 = new TestKDTree3();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_ktree3->build_tree(pts, indices, large_bucket_size);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Kdtree3 bucket size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_ktree3->build_tree(pts, indices, large_bucket_size);
        #endif
        run_tests(test_ktree3, "KDTree3 Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);

    #endif 
        
    #ifdef TEST_KDTREE4
        TestKDTree4 *test_ktree4;
        test_ktree4 = new TestKDTree4();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_ktree4->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Kdtree4 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_ktree4->build_tree(pts, indices);
        #endif
        run_tests(test_ktree4, "KDTree4", query_bboxes, pts, indices, brute_force_results);
    #endif 

    #ifdef TEST_LIBKDTREE
        TestLibkdtree *test_libkdtree;
        test_libkdtree = new TestLibkdtree();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_libkdtree->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Libkdtree build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_libkdtree->build_tree(pts, indices);
        #endif
        run_tests(test_libkdtree, "Libkdtree", query_bboxes, pts, indices, brute_force_results);
    #endif

    #ifdef TEST_3DTK
        Test3DTK *test_3dtk = new Test3DTK();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_3dtk->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "3DTK build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_3dtk->build_tree(pts, indices);
        #endif
        run_tests(test_3dtk, "3DTK", query_bboxes, pts, indices, brute_force_results);

        test_3dtk = new Test3DTK();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_3dtk->build_tree(pts, indices, large_bucket_size);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "3DTK Bucket size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_3dtk->build_tree(pts, indices, large_bucket_size);
        #endif
        run_tests(test_3dtk, "3DTK Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);

    #endif

    #ifdef TEST_LIBKDTREE2
        TestLibkdtree2 *test_libkdtree2;
        test_libkdtree2 = new TestLibkdtree2();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_libkdtree2->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Libkdtree2 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_libkdtree2->build_tree(pts, indices);
        #endif
        run_tests(test_libkdtree2, "Libkdtree2", query_bboxes, pts, indices, brute_force_results);
    #endif

    #ifdef TEST_LIBNABO
        TestLibnabo *test_libnabo;
        test_libnabo = new TestLibnabo();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_libnabo->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Libnabo build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_libnabo->build_tree(pts, indices);
        #endif
        run_tests(test_libnabo, "Libnabo", query_bboxes, pts, indices, brute_force_results);

        test_libnabo = new TestLibnabo();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_libnabo->build_tree(pts, indices, true, large_bucket_size);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Libnabo Bucket Size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_libnabo->build_tree(pts, indices, true, large_bucket_size);
        #endif
        run_tests(test_libnabo, "Libnabo Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);

        test_libnabo = new TestLibnabo();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_libnabo->build_tree(pts, indices, false);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Libnabo Tree heap build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_libnabo->build_tree(pts, indices, false);
        #endif
        run_tests(test_libnabo, "Libnabo Tree Heap", query_bboxes, pts, indices, brute_force_results);

        test_libnabo = new TestLibnabo();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_libnabo->build_tree(pts, indices, false, large_bucket_size);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Libnabo Tree heap Bucket Size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_libnabo->build_tree(pts, indices, false, large_bucket_size);
        #endif
        run_tests(test_libnabo, "Libnabo Tree Heap Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);
    #endif

    #ifdef TEST_LIBSPATIALINDEX
        TestLibspatialindex *test_libspatialindex;
        test_libspatialindex = new TestLibspatialindex();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Libspatialindex build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_libspatialindex->build_tree(pts, indices);
        #endif
        run_tests(test_libspatialindex, "Libspatialindex", query_bboxes, pts, indices, brute_force_results);

        test_libspatialindex = new TestLibspatialindex();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree(pts, indices, large_bucket_size);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Libspatialindex Bucket Size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_libspatialindex->build_tree(pts, indices, large_bucket_size);
        #endif
        run_tests(test_libspatialindex, "Libspatialindex Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);

        test_libspatialindex = new TestLibspatialindex();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree_linear(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Libspatialindex Linear build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_libspatialindex->build_tree_linear(pts, indices);
        #endif
        run_tests(test_libspatialindex, "Libspatialindex linear", query_bboxes, pts, indices, brute_force_results);

        test_libspatialindex = new TestLibspatialindex();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree_linear(pts, indices, large_bucket_size);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Libspatialindex Linear Bucket Size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_libspatialindex->build_tree_linear(pts, indices, large_bucket_size);
        #endif
        run_tests(test_libspatialindex, "Libspatialindex linear Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);

        test_libspatialindex = new TestLibspatialindex();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree_quadratic(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Libspatialindex quadratic build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_libspatialindex->build_tree_quadratic(pts, indices);
        #endif
        run_tests(test_libspatialindex, "Libspatialindex quadratic", query_bboxes, pts, indices, brute_force_results);

        test_libspatialindex = new TestLibspatialindex();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree_quadratic(pts, indices, large_bucket_size);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Libspatialindex quadratic Bucket Size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_libspatialindex->build_tree_quadratic(pts, indices, large_bucket_size);
        #endif
        run_tests(test_libspatialindex, "Libspatialindex quadratic Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);


        test_libspatialindex = new TestLibspatialindex();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree_bbox(bbox_pts, bbox_indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Libspatialindex faces build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_libspatialindex->build_tree_bbox(bbox_pts, bbox_indices);
        #endif
        run_tests(test_libspatialindex, "Libspatialindex faces", query_bboxes, bbox_pts, bbox_indices, brute_force_results_bboxes, IS_BBOX);

        test_libspatialindex = new TestLibspatialindex();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree_bbox(bbox_pts, bbox_indices, large_bucket_size);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Libspatialindex faces Bucket Size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_libspatialindex->build_tree_bbox(bbox_pts, bbox_indices, large_bucket_size);
        #endif
        run_tests(test_libspatialindex, "Libspatialindex faces Bucket Size = 50", query_bboxes, bbox_pts, bbox_indices, brute_force_results_bboxes, IS_BBOX);

        test_libspatialindex = new TestLibspatialindex();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree_bbox_linear(bbox_pts, bbox_indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Libspatialindex faces linear build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_libspatialindex->build_tree_bbox_linear(bbox_pts, bbox_indices);
        #endif
        run_tests(test_libspatialindex, "Libspatialindex faces linear", query_bboxes, bbox_pts, bbox_indices, brute_force_results_bboxes, IS_BBOX);

        test_libspatialindex = new TestLibspatialindex();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree_bbox_linear(bbox_pts, bbox_indices, large_bucket_size);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Libspatialindex faces linear Bucket Size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_libspatialindex->build_tree_bbox_linear(bbox_pts, bbox_indices, large_bucket_size);
        #endif
        run_tests(test_libspatialindex, "Libspatialindex faces linear Bucket Size = 50", query_bboxes, bbox_pts, bbox_indices, brute_force_results_bboxes, IS_BBOX);

        test_libspatialindex = new TestLibspatialindex();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree_bbox_quadratic(bbox_pts, bbox_indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Libspatialindex faces quadratic build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_libspatialindex->build_tree_bbox_quadratic(bbox_pts, bbox_indices);
        #endif
        run_tests(test_libspatialindex, "Libspatialindex faces quadratic", query_bboxes, bbox_pts, bbox_indices, brute_force_results_bboxes, IS_BBOX);

        test_libspatialindex = new TestLibspatialindex();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_libspatialindex->build_tree_bbox_quadratic(bbox_pts, bbox_indices, large_bucket_size);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Libspatialindex faces quadratic Bucket Size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_libspatialindex->build_tree_bbox_quadratic(bbox_pts, bbox_indices, large_bucket_size);
        #endif
        run_tests(test_libspatialindex, "Libspatialindex faces quadratic Bucket Size = 50", query_bboxes, bbox_pts, bbox_indices, brute_force_results_bboxes, IS_BBOX);


    #endif

    #ifdef TEST_NANOFLANN
        TestNanoflann *test_nanoflann;
        test_nanoflann = new TestNanoflann();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_nanoflann->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Nanoflann build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_nanoflann->build_tree(pts, indices);
        #endif
        run_tests(test_nanoflann, "Nanoflann", query_bboxes, pts, indices, brute_force_results);

        test_nanoflann = new TestNanoflann();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_nanoflann->build_tree(pts, indices, large_bucket_size);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Nanoflann Bucket Size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_nanoflann->build_tree(pts, indices, large_bucket_size);
        #endif
        run_tests(test_nanoflann, "Nanoflann Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);

        test_nanoflann = new TestNanoflann();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_nanoflann->build_tree(pts, indices, NUM_ELEMS_PER_NODE, true);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Nanoflann  Duplicated Storage  build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_nanoflann->build_tree(pts, indices);
        #endif
        run_tests(test_nanoflann, "Nanoflann Duplicated Storage ", query_bboxes, pts, indices, brute_force_results);

        test_nanoflann = new TestNanoflann();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_nanoflann->build_tree(pts, indices, large_bucket_size, true);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Nanoflann  Duplicated Storage Bucket Size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_nanoflann->build_tree(pts, indices, large_bucket_size);
        #endif
        run_tests(test_nanoflann, "Nanoflann Duplicated Storage Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);

    #endif

    #ifdef TEST_OCTREE
        TestOctree *test_octree;
        test_octree = new TestOctree();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_octree->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Octree build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_octree->build_tree(pts, indices);
        #endif
        run_tests(test_octree, "Octree", query_bboxes, pts, indices, brute_force_results);

        test_octree = new TestOctree();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_octree->build_tree(pts, indices, large_bucket_size);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Octree Bucket Size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_octree->build_tree(pts, indices, large_bucket_size);
        #endif
        run_tests(test_octree, "Octree Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);

    #endif 

    #ifdef TEST_PCL
        TestPCL::Octree *test_pcl;
        double avg_voxel_volume_per_data_pt = total_domain_volume/(double)num_data_pts;
        //set resolution/voxel length to cbrt(default bucket size*avg voxel volume per data point) to get num data points per leaf approx=large bucket size
        test_pcl = new TestPCL::Octree();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_pcl->build_tree(pts, indices, NUM_ELEMS_PER_NODE*avg_voxel_volume_per_data_pt);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "PCL Octree Resolution for Bucket size approx=20 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_pcl->build_tree(pts, indices);
        #endif
        run_tests(test_pcl, "PCL Octree Resolution for Bucket size approx=20", query_bboxes, pts, indices, brute_force_results);

        //set resolution/voxel length to cbrt(large bucket size*avg voxel volume per data point) to get num data points per leaf approx=large bucket size
        test_pcl = new TestPCL::Octree();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_pcl->build_tree(pts, indices, large_bucket_size*avg_voxel_volume_per_data_pt);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "PCL Octree Resolution for Bucket size approx=50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_pcl->build_tree(pts, indices, large_bucket_size);
        #endif
        run_tests(test_pcl, "PCL Octree Resolution for Bucket Size approx= 50", query_bboxes, pts, indices, brute_force_results);

        //KDtree is just a wrapper around FLANN, no added functionality. shouldn't expect it to be faster
        //library doesn't support setting setting bucket size for kd tree, default is 15
        TestPCL::KDTree *test_pcl_kdtree = new TestPCL::KDTree();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_pcl_kdtree->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "PCL KDTree build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_pcl_kdtree->build_tree(pts, indices);
        #endif
        run_tests(test_pcl_kdtree, "PCL KDTree", query_bboxes, pts, indices, brute_force_results);

        #if USE_GPU
            TestPCL::OctreeGPU *test_pcl_octree_gpu = new TestPCL::OctreeGPU();
            #if OUTPUT_TIMING_RESULTS
                build_start_time = std::chrono::high_resolution_clock::now();
                test_pcl_octree_gpu->build_tree(pts, indices);
                build_stop_time = std::chrono::high_resolution_clock::now();
                cout << "PCL OctreeGPU build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
            #else
                test_pcl_octree_gpu->build_tree(pts, indices);
            #endif
            run_tests(test_pcl_octree_gpu, "PCL OctreeGPU", query_bboxes, pts, indices, brute_force_results, !IS_BBOX, false);

            //doesn't affect how the tree builds just how you query it
            run_tests(test_pcl_octree_gpu, "PCL OctreeGPU query domain decomposition", query_bboxes, pts, indices, brute_force_results);
        #endif
    #endif

    #ifdef TEST_PICO_TREE
        TestPicoTree *test_pico_tree;
        test_pico_tree = new TestPicoTree();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_pico_tree->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Pico tree build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_pico_tree->build_tree(pts, indices);
        #endif
        run_tests(test_pico_tree, "Pico tree", query_bboxes, pts, indices, brute_force_results);

        test_pico_tree = new TestPicoTree();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_pico_tree->build_tree(pts, indices, large_bucket_size);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Pico tree Bucket Size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_pico_tree->build_tree(pts, indices, large_bucket_size);
        #endif
        run_tests(test_pico_tree, "Pico tree Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);
    #endif 

    #ifdef TEST_RTREE_TEMPLATE
        TestRtreeTemplate::Points<NUM_ELEMS_PER_NODE> *test_rtree_template;
        test_rtree_template = new TestRtreeTemplate::Points<NUM_ELEMS_PER_NODE>();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_rtree_template->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Rtree template build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_rtree_template->build_tree(pts, indices);
        #endif
        run_tests(test_rtree_template, "Rtree template", query_bboxes, pts, indices, brute_force_results);

        TestRtreeTemplate::Points<large_bucket_size> *test_rtree_template_large_bucket_size;
        test_rtree_template_large_bucket_size = new TestRtreeTemplate::Points<large_bucket_size>();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_rtree_template_large_bucket_size->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Rtree template Bucket Size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_rtree_template_large_bucket_size->build_tree(pts, indices);
        #endif
        run_tests(test_rtree_template_large_bucket_size, "Rtree template Bucket Size = 50", query_bboxes, pts, indices, brute_force_results);

        TestRtreeTemplate::Bboxes<NUM_ELEMS_PER_NODE> *test_rtree_template_bboxes;
        test_rtree_template_bboxes = new TestRtreeTemplate::Bboxes<NUM_ELEMS_PER_NODE>();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_rtree_template_bboxes->build_tree(bbox_pts, bbox_indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Rtree Template Bboxes build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_rtree_template_bboxes->build_tree(bbox_pts, bbox_indices);
        #endif
        run_tests(test_rtree_template_bboxes, "Rtree Template Bboxes", query_bboxes, bbox_pts, bbox_indices, brute_force_results_bboxes, IS_BBOX);

        TestRtreeTemplate::Bboxes<NUM_ELEMS_PER_NODE> *test_rtree_template_bboxes_large_bucket_size;
        test_rtree_template_bboxes_large_bucket_size = new TestRtreeTemplate::Bboxes<NUM_ELEMS_PER_NODE>();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_rtree_template_bboxes_large_bucket_size->build_tree(bbox_pts, bbox_indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Rtree Template Bboxes Bucket Size = 50 build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_rtree_template_bboxes_large_bucket_size->build_tree(bbox_pts, bbox_indices);
        #endif
        run_tests(test_rtree_template_bboxes_large_bucket_size, "Rtree Template Bboxes Bucket Size = 50", query_bboxes, bbox_pts, bbox_indices, brute_force_results_bboxes, IS_BBOX);
    #endif 

    #if TEST_SPATIAL
        TestSpatial::Points *test_spatial;
        test_spatial = new TestSpatial::Points();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_spatial->build_tree(pts, indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Spatial build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_spatial->build_tree(pts, indices);
        #endif
        run_tests(test_spatial, "Spatial", query_bboxes, pts, indices, brute_force_results);

        //note - could test, for both points and bboxes if the self-balancing version is faster, but it seems unlikely
        TestSpatial::Bboxes *test_spatial_bboxes;
        test_spatial_bboxes = new TestSpatial::Bboxes();
        #if OUTPUT_TIMING_RESULTS
            build_start_time = std::chrono::high_resolution_clock::now();
            test_spatial_bboxes->build_tree(bbox_pts, bbox_indices);
            build_stop_time = std::chrono::high_resolution_clock::now();
            cout << "Spatial Bboxes build time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count() << " ns" << endl;
        #else
            test_spatial_bboxes->build_tree(bbox_pts, bbox_indices);
        #endif
        run_tests(test_spatial_bboxes, "Spatial Bboxes", query_bboxes, bbox_pts, bbox_indices, brute_force_results_bboxes, IS_BBOX);
    #endif

}

void run_tests(BboxIntersectionTest *test, const string &test_name, const std::vector<bbox> &query_bboxes,
    const std::vector<point> &pts, const std::vector<size_t> &indices, const std::vector<std::vector<size_t>> &correct_results,
     bool is_bboxes, bool delete_tree
    ) 
{

    #if OUTPUT_TIMING_RESULTS
        std::chrono::high_resolution_clock::time_point query_start_time = std::chrono::high_resolution_clock::now();
        std::chrono::high_resolution_clock::time_point query_stop_time;
    #endif
    #ifdef MEM_TRACK
        cout << "memory before issuing queries" << endl;
        MemTrack::TrackListMemoryUsage();
    #endif
    cout << "testing " << test_name << endl;

    for(size_t i = 0; i < query_bboxes.size(); i++) {
        std::vector<size_t> query_result_indices;
        if(DEBUG) {
            cout << "query: ";
            print_bbox(query_bboxes[i]);
        }

        if(test_name == "PCL OctreeGPU query domain decomposition") {
            #if defined TEST_PCL && defined USE_GPU
                size_t num_sub_queries = 24;
                size_t num_x_queries = 2;
                size_t num_y_queries = 3;
                size_t num_z_queries = 4;
                static_cast<TestPCL::OctreeGPU*>(test)->get_intersections(query_bboxes[i], query_result_indices, num_sub_queries, num_x_queries, num_y_queries, num_z_queries);
            #endif
        }
        else {
            test->get_intersections(query_bboxes[i], query_result_indices);
        }

        //we don't have a good way to do correctness checking for triangle intersection
        if (test_name.find("Triangles") == std::string::npos) { 
            if(test->intersections_exact()) {
                std::sort(query_result_indices.begin(), query_result_indices.end());
                if(query_result_indices != correct_results[i]) {
                    cout << "query " << i << ": ";
                    print_bbox(query_bboxes[i]);
                    cout << "error. results do not match. the library finds " <<  query_result_indices.size() << " results when it should find " << correct_results[i].size() << endl;
                    cout << "query found: " << endl;
                    for(int j = 0; j < query_result_indices.size(); j++) {
                        cout << "index: " << query_result_indices[j] << ", ";
                        print_point(pts[query_result_indices[j]]);
                    }
                    cout << "correct result: " << endl;
                    for(int j = 0; j < correct_results[i].size(); j++) {
                        cout << "index: " << correct_results[i][j] << ", ";
                        print_point(pts[correct_results[i][j]]);
                    }
                    cout << "the results were expected to be exact" << endl;               
                }
                else {
                    if(DEBUG) {
                        cout << "query " << i << ": correct results" << endl;
                    }
                }            
            }
            else {
                vector<size_t> exact_intersections;
                exact_intersections.reserve(query_result_indices.size());
                for(auto index : query_result_indices) {
                    if(check_intersection(query_bboxes[i], pts[index])) {
                        exact_intersections.push_back(index);
                    }
                }
                std::sort(exact_intersections.begin(), exact_intersections.end());
                if(exact_intersections != correct_results[i]) {
                    cout << "query " << i << ": ";
                    print_bbox(query_bboxes[i]);
                    cout << "error. results do not match. the library finds " <<  query_result_indices.size() << " results when it should find " << correct_results[i].size() << endl;
                    cout << "query found: " << endl;
                    for(int j = 0; j < exact_intersections.size(); j++) {
                        print_point(pts[exact_intersections[j]]);
                    }
                    cout << "correct result: " << endl;
                    for(int j = 0; j < correct_results[i].size(); j++) {
                        print_point(pts[correct_results[i][j]]);
                    }
                    cout << "the results were NOT expected to be exact" << endl; 
                }
                else {
                    if(DEBUG) {
                        cout << "query " << i << ": correct results" << endl;
                    }
                }        
            }
        }



        if(DEBUG) {
            cout << "num intersections: " << query_result_indices.size() << endl;
            cout << "intersections: " << endl;

            for(int j = 0; j < query_result_indices.size(); j++) {
                size_t query_result_index = query_result_indices[j];

                if(!test->intersections_exact()) {
                    if(check_intersection(query_bboxes[i], pts[query_result_index])) {
                        print_point(pts[query_result_index]);
                    }
                    else {
                        cout << "found a point that is outside our search area: ";
                        print_point(pts[query_result_index]);
                    }
                }
                else {
                    //print the triangle
                    if (test_name.find("Triangles") != std::string::npos) { 
                        cout << "triangle: " << endl; 
                        print_point(pts[query_result_index]);
                        print_point(pts[query_result_index+DOMAIN_LENGTH]);
                        print_point(pts[query_result_index+DOMAIN_LENGTH*DOMAIN_LENGTH]);
                    }
                    else if(is_bboxes) {
                        cout << "bbox: " << endl; 
                        print_point(pts[2*query_result_index]);
                        print_point(pts[2*query_result_index+1]);
                    }
                    else {
                        print_point(pts[query_result_index]);
                    }
                }
            }
        }
    } //end for all queries
    #ifdef OUTPUT_TIMING_RESULTS
        query_stop_time = std::chrono::high_resolution_clock::now();
        cout << test_name << " query time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(query_stop_time - query_start_time).count() << " ns" << endl;
    #endif
    #ifdef MEM_TRACK
        cout << "memory after issuing queries" << endl;
        MemTrack::TrackListMemoryUsage();
    #endif
    if(delete_tree) {
        delete test; 
    }

}



void test_feature_generation() {
    typedef boost::variate_generator<boost::mt19937&, boost::uniform_real<double>> generator;

    double domain_length_x = 50.0;
    double domain_length_y = 100.0;
    double domain_length_z = 200.0;
    size_t num_data_pts = 1000000; 
    size_t num_queries = 10000;

    //feature = .001% of data
    double extra_extra_small_x_length = domain_length_x/40;
    double extra_extra_small_y_length = domain_length_y/50;
    double extra_extra_small_z_length = domain_length_z/50;

    //feature = .1% of data
    double small_x_length = domain_length_x/10;
    double small_y_length = domain_length_y/10;
    double small_z_length = domain_length_z/10;

    //feature = 1% of data
    double med_x_length = domain_length_x/4;
    double med_y_length = domain_length_y/5;
    double med_z_length = domain_length_z/5;

    //feature = 10% of data
    double large_x_length = domain_length_x/2;
    double large_y_length = domain_length_y/2;
    double large_z_length = domain_length_z/2.5;

    vector<double> domain_lower_bounds = {-(domain_length_x/2), -(domain_length_y/2), -(domain_length_z/2)};
    vector<double> domain_upper_bounds = {(domain_length_x/2), (domain_length_y/2), (domain_length_z/2)};

    vector<vector<double>> all_query_sizes = {
        {extra_extra_small_x_length, extra_extra_small_y_length, extra_extra_small_z_length},
        {small_x_length, small_y_length, small_z_length},
        {med_x_length, med_y_length, med_z_length},
        {large_x_length, large_y_length, large_z_length}
    };
    vector<vector<bbox>> all_queries(all_query_sizes.size());

    boost::mt19937 rng;
    rng.seed(100);

    boost::uniform_real<double> range_x(domain_lower_bounds[0], domain_upper_bounds[0]);
    boost::uniform_real<double> range_y(domain_lower_bounds[1], domain_upper_bounds[1]);
    boost::uniform_real<double> range_z(domain_lower_bounds[2], domain_upper_bounds[2]);

    vector<generator> generators = {generator(rng, range_x), generator(rng, range_y), generator(rng, range_z)};

    for(size_t i = 0; i < all_query_sizes.size(); i++) {
       for(size_t j = 0; j < num_queries; j++) {
            point query_lower_corner(domain_lower_bounds.size()), query_upper_corner(domain_lower_bounds.size());
            for(size_t k = 0; k < domain_lower_bounds.size(); k++) {
                query_lower_corner[k] = generators[k]();
                double len = all_query_sizes[i][k];
                while(query_lower_corner[k] + len > domain_upper_bounds[k]) {
                    cout << "found an instance where pt exceeds bounds" << endl;
                    cout << "query_lower_corner[" << k << "]: " << query_lower_corner[k] << ", len: " << len << ", domain_upper_bounds[" << k << "]: " << domain_upper_bounds[k] << endl;
                    query_lower_corner[k] = generators[k]();
                }
                query_upper_corner[k] = query_lower_corner[k] + len;
            }

            all_queries[i].push_back(bbox(query_lower_corner, query_upper_corner));
            if(DEBUG) {
                cout << "query: ";
                print_bbox(all_queries[i].back());
            }
        }
    }

}
