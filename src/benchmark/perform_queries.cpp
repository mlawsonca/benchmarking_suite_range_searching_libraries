//whether we are performing element-based computation using the nodes (meaning we need to retrieve the node ids) or using the elements
#if RETRIEVE_NODES_FOR_BBOXES
    #include <set>
#endif
#include "data_and_query_generation.hh"
#include "range_tree_libraries.hh"

extern bool VALGRIND;

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

void perform_queries(BboxIntersectionTest *test, const std::string &test_name,
    const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config, QueryType query_type,
    const std::vector<std::vector<size_t>> &element_node_ids) 
{   

    if(!VALGRIND) {

        //contains queries for each category (those that cover a very small portion of the domain, a small portion, a moderate portion, and a large portion)
        std::vector<std::vector<bbox>> all_queries; 
        std::vector<double> queries_percent_data_covered;
        get_queries_specific_feature_sizes(config, all_queries, queries_percent_data_covered);


        for(size_t i = 0; i < all_queries.size(); i++) {
            std::chrono::high_resolution_clock::time_point query_start_time = std::chrono::high_resolution_clock::now();
            size_t num_intersected_data_points = 0;

            for(int j = 0; j < all_queries[i].size(); j++) {
                bbox query = all_queries[i][j];
                std::vector<size_t> query_result_indices;
                if(DEBUG) {
                    std::cout << "query: ";
                    print_bbox(query);
                }

                if(query_type == GPU_DOMAIN_DECOMP) {
                    #ifdef TEST_PCL
                        size_t num_sub_queries = 24;
                        size_t num_x_queries = 2;
                        size_t num_y_queries = 3;
                        size_t num_z_queries = 4;
                        static_cast<TestPCL::OctreeGPU*>(test)->get_intersections(query, query_result_indices, num_sub_queries, num_x_queries, num_y_queries, num_z_queries);
                    #else
                        std::cerr << "Error. Used query type: GPU_DOMAIN_DECOMP without testing PCL, which is the only library that uses this query type" << std::endl;
                        exit(-1);
                    #endif
                }
                else {
                    test->get_intersections(query, query_result_indices);
                }

                //point queries may be inexact (e.g., because they use a circular radius). Box and triangle queries will always be exact
                if(!test->intersections_exact()) {
                    std::vector<size_t> exact_intersections;
                    exact_intersections.reserve(query_result_indices.size());
                    for(auto index : query_result_indices) {
                        if(check_intersection(query, pts[index])) {
                            exact_intersections.push_back(index);
                            if(DEBUG) {
                                print_data(pts, index, config.data_type);
                            }
                        }
                    }
                    num_intersected_data_points += exact_intersections.size();
                }
                else {
                    if(config.data_type == POINTS) {
                        num_intersected_data_points += query_result_indices.size();   
                    }
                    else {
                        #if RETRIEVE_NODES_FOR_BBOXES
                            std::set<size_t> node_indices;
                            for(size_t element_index : query_result_indices) {
                                for(size_t i = 0; i < element_node_ids[element_index].size(); i++) {
                                    node_indices.insert(element_node_ids[element_index][i]);
                                }
                            }
                            num_intersected_data_points += node_indices.size();
                        #else
                            //assumption: we are performing element-based computation, using centroid (or something other than nodes)
                            num_intersected_data_points += query_result_indices.size();
                        #endif
                    }
                    if(DEBUG) {
                        std::cout << "query_result_indices.size(): "  << query_result_indices.size() << std::endl;    
                    }        
                }
                if(DEBUG && test->intersections_exact()) {
                    for(auto index : query_result_indices) {
                        print_data(pts, index, config.data_type);
                    }
                }
            }

            //if data_type==BBOXES not RETRIEVE_NODES_FOR_BBOXES, num data pts will be set to num elements
            double avg_perc_data_pts_intersected = (num_intersected_data_points / (double)all_queries[i].size()) / config.num_data_pts * 100;
            print_query_time(queries_percent_data_covered[i], test_name, query_start_time, avg_perc_data_pts_intersected, config);
        }

    }
    else {
        //helps make sure there is time for massif to detect the decline in memory usage after tree building has completed
        for(size_t i = 0; i < config.num_queries; i++) {
            print_query_time(0.0, "dummy", std::chrono::high_resolution_clock::now(), 0, config);
        }

    }


    delete test;
}





