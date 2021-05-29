#ifndef DATA_AND_QUERY_GENERATION_HH
#define DATA_AND_QUERY_GENERATION_HH

#include "common.hh"

void get_queries_specific_feature_sizes(testing_config config, std::vector<std::vector<bbox>> &all_queries, std::vector<double> &queries_percent_data_covered);
void get_queries_random_feature_sizes(testing_config config, std::vector<bbox> &queries);
void get_small_queries(std::vector<bbox> &queries);
void get_random_data(testing_config config, std::vector<point> &pts, std::vector<size_t> &indices);
void get_regular_mesh_data(testing_config config);
void get_data_from_exodus_file(DataType data_type, const std::string &full_file_path, std::vector<point> &mesh_coords, bbox &domain_bounds, 
    uint32_t &num_data_pts, std::vector<std::vector<size_t>> &node_ids_per_elem);
void get_data_from_exodus_file(DataType data_type, const std::string &full_file_path, std::vector<point> &mesh_coords, bbox &domain_bounds, 
    uint32_t &num_data_pts);
#endif //DATA_AND_QUERY_GENERATION_HH