#include <boost/random.hpp> //needed for random range generation
#include "common.hh"
#include "exodusII.h"
#include <thread>

#ifndef DOMAIN_LENGTH
    #error Your need to define DOMAIN_LENGTH in a common header file
#endif

using namespace std;

void get_queries_specific_feature_sizes(testing_config config, std::vector<std::vector<bbox>> &all_queries, std::vector<double> &queries_percent_data_covered
    ) 
{
    typedef boost::variate_generator<boost::mt19937&, boost::uniform_real<double>> generator;
    vector<double> domain_lengths;
    for(size_t i = 0; i < config.domain_lower_bounds.size(); i++) {
        domain_lengths.push_back(config.domain_upper_bounds[i]-config.domain_lower_bounds[i]);
        if(DEBUG) {
            cout << "domain_lengths[" << i << "]: " << domain_lengths[i] << endl;
            cout << "config.domain_upper_bounds[" << i << "]: " << config.domain_upper_bounds[i] << endl;
            cout << "config.domain_lower_bounds[" << i << "]: " << config.domain_lower_bounds[i] << endl;
        }
    }

    //feature = .001% of data -> .001% of domain space
    //value adjusted slightly so we find closer the correct amount of data
    double frac1 = cbrt(90100);
    double extra_extra_small_x_length = domain_lengths[0]/frac1;
    double extra_extra_small_y_length = domain_lengths[1]/frac1;
    double extra_extra_small_z_length = domain_lengths[2]/frac1;

    //feature = .1% of data -> .1% of domain space
    //value adjusted slightly so we find closer the correct amount of data
    double frac2 = cbrt(1050);
    double small_x_length = domain_lengths[0]/frac2;
    double small_y_length = domain_lengths[1]/frac2;
    double small_z_length = domain_lengths[2]/frac2;

    //feature = 1% of data -> 1% of domain space
    //value adjusted slightly so we find closer the correct amount of data
    double frac3 = cbrt(125);
    double med_x_length = domain_lengths[0]/frac3;
    double med_y_length = domain_lengths[1]/frac3;
    double med_z_length = domain_lengths[2]/frac3;

    //feature = 10% of data -> 10% of domain space
    //value adjusted significantly so we find closer the correct amount of data
    double frac4 = cbrt(19.2);
    double large_x_length = domain_lengths[0]/frac4;
    double large_y_length = domain_lengths[1]/frac4;
    double large_z_length = domain_lengths[2]/frac4;

    std::vector<std::vector<double>> all_query_sizes = {
        {extra_extra_small_x_length, extra_extra_small_y_length, extra_extra_small_z_length},
        {small_x_length, small_y_length, small_z_length},
        {med_x_length, med_y_length, med_z_length}
        ,{large_x_length, large_y_length, large_z_length}
    };
    all_queries.resize(all_query_sizes.size());
    std::vector<double> query_percent = {.001, .1, 1, 10};
    queries_percent_data_covered = query_percent;

    boost::mt19937 rng;
    //want it to be reproducible
    rng.seed(100);

    boost::uniform_real<double> range_x(config.domain_lower_bounds[0], config.domain_upper_bounds[0]);
    boost::uniform_real<double> range_y(config.domain_lower_bounds[1], config.domain_upper_bounds[1]);
    boost::uniform_real<double> range_z(config.domain_lower_bounds[2], config.domain_upper_bounds[2]);

    //separate them so the queries sets of a particular frequency are independent from the other frequencies
    std::vector<std::vector<generator>> generators = {
        {generator(rng, range_x), generator(rng, range_y), generator(rng, range_z)},
        {generator(rng, range_x), generator(rng, range_y), generator(rng, range_z)},
        {generator(rng, range_x), generator(rng, range_y), generator(rng, range_z)},
        {generator(rng, range_x), generator(rng, range_y), generator(rng, range_z)}
    };

    //only do 1/10 as many queries of the extra large size to save time
    std::vector<size_t> num_queries = {config.num_queries, config.num_queries, config.num_queries, config.num_queries/10};

    if(DEBUG) {
        std::cout << "all_query_sizes.size(): " << all_query_sizes.size() << std::endl;
        cout << "config.num_queries: " << config.num_queries << endl;
        cout << "config.domain_lower_bounds.size(): " << config.domain_lower_bounds.size() << endl;
    }
    for(size_t i = 0; i < all_query_sizes.size(); i++) {
        for(size_t j = 0; j < num_queries[i]; j++) {
            point query_lower_corner(config.domain_lower_bounds.size()), query_upper_corner(config.domain_lower_bounds.size());
            for(size_t k = 0; k < config.domain_lower_bounds.size(); k++) {
                query_lower_corner[k] = generators[i][k]();
                double len = all_query_sizes[i][k];
                while(query_lower_corner[k] + len > config.domain_upper_bounds[k]) {
                    if(DEBUG) {
                        std::cout << "found an instance where pt exceeds bounds" << std::endl;
                        std::cout << "query_lower_corner[" << k << "]: " << query_lower_corner[k] << ", len: " << len << ", domain_upper_bounds[" << k << "]: " << config.domain_upper_bounds[k] << std::endl;                        
                    }

                    query_lower_corner[k] = generators[i][k]();
                }
                query_upper_corner[k] = query_lower_corner[k] + len;
            }

            all_queries[i].push_back(bbox(query_lower_corner, query_upper_corner));
            if(DEBUG) {
                std::cout << "query: ";
                print_bbox(all_queries[i].back());
            }
        }
    }
    if(DEBUG) {
        std::cout << "done with get_queries_specific_feature_sizes" << std::endl;
    }
}

void get_queries_random_feature_sizes(testing_config config, std::vector<bbox> &queries) {
    boost::mt19937 rng;
    //want it to be reproducible
    rng.seed(100);
    vector<double> domain_lengths;
    for(size_t i = 0; i < config.domain_lower_bounds.size(); i++) {
        domain_lengths.push_back(config.domain_upper_bounds[i]-config.domain_upper_bounds[i]);
    }

    boost::uniform_real<double> range_x(config.domain_lower_bounds[0], config.domain_upper_bounds[0]);
    boost::uniform_real<double> range_y(config.domain_lower_bounds[1], config.domain_upper_bounds[1]);
    boost::uniform_real<double> range_z(config.domain_lower_bounds[2], config.domain_upper_bounds[2]);
    boost::uniform_real<double> range_x_length(domain_lengths[0]/1000, domain_lengths[0]/10);
    boost::uniform_real<double> range_y_length(domain_lengths[1]/1000, domain_lengths[1]/10);
    boost::uniform_real<double> range_z_length(domain_lengths[2]/1000, domain_lengths[2]/10);

    boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd_x(rng, range_x);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd_y(rng, range_y);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd_z(rng, range_z);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd_length_x(rng, range_x_length);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd_length_y(rng, range_y_length);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd_length_z(rng, range_z_length);

    for(size_t i = 0; i < config.num_queries; i++) {
        double x = rnd_x();
        double y = rnd_y();
        double z = rnd_z();
        double x_len = rnd_length_x();
        double y_len = rnd_length_y();
        double z_len = rnd_length_z();
        queries.push_back(bbox(point({x, y, z}),point({x+x_len, y+y_len, z+z_len})));
        if(DEBUG) {
            std::cout << "query: ";
            print_bbox(queries.back());
        }
    }
}

void get_small_queries(std::vector<bbox> &queries) {
    queries.push_back(std::make_pair(point({0,0,0}),point({1,1,1})));
    queries.push_back(std::make_pair(point({.5,.5,.5}),point({1,1,1})));
}

void get_random_data(testing_config config, std::vector<point> &pts, std::vector<size_t> &indices) {
    boost::mt19937 rng;
    //want it to be reproducible
    rng.seed(100);

    vector<double> domain_lengths;
    for(size_t i = 0; i < config.domain_lower_bounds.size(); i++) {
        domain_lengths.push_back(config.domain_upper_bounds[i]-config.domain_upper_bounds[i]);
    }
    boost::uniform_real<double> range_x(config.domain_lower_bounds[0], config.domain_upper_bounds[0]);
    boost::uniform_real<double> range_y(config.domain_lower_bounds[1], config.domain_upper_bounds[1]);
    boost::uniform_real<double> range_z(config.domain_lower_bounds[2], config.domain_upper_bounds[2]);
    boost::uniform_real<double> range_x_length(domain_lengths[0]/1000, domain_lengths[0]/10);
    boost::uniform_real<double> range_y_length(domain_lengths[1]/1000, domain_lengths[1]/10);
    boost::uniform_real<double> range_z_length(domain_lengths[2]/1000, domain_lengths[2]/10);

    boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd_x(rng, range_x);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd_y(rng, range_y);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd_z(rng, range_z);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd_length_x(rng, range_x_length);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd_length_y(rng, range_y_length);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > rnd_length_z(rng, range_z_length);

    if(DEBUG) {
        std::cout << "about to make points" << std::endl;
    }

    if(config.data_type == POINTS) {
        for(size_t i = 0; i < config.num_data_pts; i++) {
            double x = rnd_x();
            double y = rnd_y();
            double z = rnd_z();
            pts.push_back(point({x, y, z}));
            indices.push_back(i);

            if(DEBUG) {
                if(i < 100) {
                    std::cout << "pt: ";
                    print_point(pts.back());
                }
            }
        }        
    }
    else if(config.data_type == BBOXES) {
        for(size_t i = 0; i < config.num_data_pts; i++) {
            double x = rnd_x();
            double y = rnd_y();
            double z = rnd_z();
            double x_len = rnd_length_x();
            double y_len = rnd_length_y();
            double z_len = rnd_length_z();
            pts.push_back(point({x, y, z}));
            pts.push_back(point({x+x_len, y+y_len, z+z_len}));
            indices.push_back(i);
            if(DEBUG) {
                if(i < 100) {
                    std::cout << "box: ";
                    print_bbox(bbox(pts[pts.size()-2], pts.back()));
                }
            }
        }        
    }
    else if(config.data_type == TRIANGLES) {
        for(size_t i = 0; i < config.num_data_pts; i++) {
            double x = rnd_x();
            double y = rnd_y();
            double z = rnd_z();
            double x_len = rnd_length_x();
            double y_len = rnd_length_y();
            double z_len = rnd_length_z();
            double x_len2 = rnd_length_x();
            double y_len2 = rnd_length_y();
            double z_len2 = rnd_length_z();
            pts.push_back(point({x, y, z}));
            pts.push_back(point({x+x_len, y+y_len, z+z_len}));
            pts.push_back(point({x+x_len2, y+y_len2, z+z_len2}));
            indices.push_back(i);
            if(DEBUG) {
                if(i < 100) {
                    std::cout << "triangle: " << std::endl;
                    print_point(pts[pts.size()-3]);
                    print_point(pts[pts.size()-2]);
                    print_point(pts[pts.size()-1]);
                }
            }
        }
    }
    else {
        std::cerr << "Error in get_random_data. The config.data_type of " << config.data_type << " did not match one of the expected categories" << std::endl;
    }
    if(DEBUG) {
        std::cout << "done with make points" << std::endl;        
    }
}

void get_regular_mesh_data(testing_config config, std::vector<point> &pts, std::vector<size_t> &indices) {
    if(DEBUG) {
        std::cout << "about to make points" << std::endl;
    }
    if(config.data_type == POINTS) {
        size_t index = 0;
        for(double i = 0; i < DOMAIN_LENGTH; i++) {
            for(double j = 0; j < DOMAIN_LENGTH; j++) {
                for(double k = 0; k < DOMAIN_LENGTH; k++) {
                    pts.push_back({i, j, k});
                    indices.push_back(index);
                    index += 1;   
                }
            }
        }
    }
    else if(config.data_type == BBOXES) {
        size_t index = 0;
        for(double i = 0; i < std::floor(DOMAIN_LENGTH/2); i++) {
            for(double j = 0; j < std::floor(DOMAIN_LENGTH/2); j++) {
                for(double k = 0; k < std::floor(DOMAIN_LENGTH/2); k++) {
                    pts.push_back({i, j, k});
                    pts.push_back({i+1, j+1, k+1});
                    indices.push_back(index);   
                    index += 1;
                }
            }
        }
    }
    else if(config.data_type == TRIANGLES) {
        std::vector<point> temp;
        for(double i = 0; i < DOMAIN_LENGTH; i++) {
            for(double j = 0; j < DOMAIN_LENGTH; j++) {
                for(double k = 0; k < DOMAIN_LENGTH; k++) {
                     temp.push_back({i, j, k});        
                }
            }
        } 
        for(int i = 0; i < (temp.size()/3); i++) {
            pts.push_back(temp[i]);
            pts.push_back(temp[i+DOMAIN_LENGTH]);
            pts.push_back(temp[i+DOMAIN_LENGTH*DOMAIN_LENGTH]);
            indices.push_back(i);
        }                
    }
    if(DEBUG) {
        std::cout << "done with make points" << std::endl;
    }
}

int exodus_open_file(std::string file_name) {
    int error, exodus_id = -1;
    //size of the reals as used by the cpu and as stored
    int cpu_word_size = 8;
    int io_word_size = 0;
    float database_version;
    int retry_count = 0;
    while(exodus_id < 0 && retry_count < 100) {
        exodus_id = ex_open (file_name.c_str(), EX_READ, &cpu_word_size, &io_word_size, &database_version);
        retry_count += 1;
        //sleep just in case there is file system contention
        std::this_thread::sleep_for(1000ms);
    }
    if(exodus_id < 0) {
        std::cerr << "error in ex_open" << std::endl;
        exit(-1);
    }

    return exodus_id;

}

void exodus_read_vertex_coordinates(int exodus_id, vector<double> &x_coords, vector<double> &y_coords, vector<double> &z_coords) {
    int num_dim , num_nodes, num_elem, num_elem_blocks, num_node_sets, num_side_sets;
    char  db_title[MAX_STR_LENGTH];
    if(ex_get_init (exodus_id, db_title, &num_dim, &num_nodes, &num_elem, &num_elem_blocks, &num_node_sets, &num_side_sets)) {
        std::cerr << "error in ex_get_init" << std::endl;
        exit(-1);        
    }

    x_coords.resize(num_nodes);
    y_coords.resize(num_nodes);
    z_coords.resize(num_nodes); 
    if(ex_get_coord(exodus_id , &x_coords[0], &y_coords[0], &z_coords[0])) {
        std::cerr << "error in ex_get_coord" << std::endl;
        exit(-1);          
    }
}


void exodus_read_vertex_coordinates(int exodus_id, std::vector<point> &coords, bbox &domain_bounds, uint32_t &num_nodes) {
    double domain_lower_bounds[3] = {DBL_MAX, DBL_MAX, DBL_MAX};
    double domain_upper_bounds[3] = {-DBL_MAX, -DBL_MAX, -DBL_MAX};

    int num_dim, num_elem, num_elem_blocks, num_node_sets, num_side_sets;
    char  db_title[MAX_STR_LENGTH];
    if(ex_get_init (exodus_id, db_title, &num_dim, &num_nodes, &num_elem, &num_elem_blocks, &num_node_sets, &num_side_sets)) {
        std::cerr << "error in ex_get_init" << std::endl;
        exit(-1);             
    }

    vector<double> x_coords, y_coords, z_coords;
    x_coords.resize(num_nodes);
    y_coords.resize(num_nodes);
    z_coords.resize(num_nodes); 
    if(ex_get_coord(exodus_id , &x_coords[0], &y_coords[0], &z_coords[0])) {
        std::cerr << "error in ex_get_coord" << std::endl;
        exit(-1);     
    }

    for(int i = 0; i < num_nodes; i++) {
        coords.push_back(point({x_coords[i], y_coords[i], z_coords[i]}));

        domain_lower_bounds[0] = std::min(domain_lower_bounds[0], x_coords[i]);
        domain_upper_bounds[0] = std::max(domain_upper_bounds[0], x_coords[i]);
        domain_lower_bounds[1] = std::min(domain_lower_bounds[1], y_coords[i]);
        domain_upper_bounds[1] = std::max(domain_upper_bounds[1], y_coords[i]);
        domain_lower_bounds[2] = std::min(domain_lower_bounds[2], z_coords[i]);
        domain_upper_bounds[2] = std::max(domain_upper_bounds[2], z_coords[i]);
    }


    domain_bounds = bbox(point({domain_lower_bounds[0], domain_lower_bounds[1], domain_lower_bounds[2]}), 
                         point({domain_upper_bounds[0], domain_upper_bounds[1], domain_upper_bounds[2]})
                    );
}


void exodus_read_vertex_coordinates(const std::string &full_file_path, std::vector<point> &coords, bbox &domain_bounds, uint32_t &num_nodes) {
    int exodus_id = exodus_open_file(full_file_path);
    exodus_read_vertex_coordinates(exodus_id, coords, domain_bounds, num_nodes);
    ex_close (exodus_id);
}


void exodus_get_element_connectivity(int exodus_id, int elem_block_id, vector<uint32_t> &node_connectivity, uint32_t &num_nodes_per_elem) {
    int error, num_elem_in_block, num_edges_per_elem, num_faces_per_elem, num_attr_per_elem;
    char elem_description[MAX_STR_LENGTH +1];

    if(ex_get_block(exodus_id, EX_ELEM_BLOCK, elem_block_id, elem_description, &num_elem_in_block, &num_nodes_per_elem, &num_edges_per_elem, &num_faces_per_elem, &num_attr_per_elem)) {
        cerr << "error with ex_get_block" << endl;
        exit(-1);
    }
    else {
        if(num_elem_in_block > 0) {
            /* read  element  connectivity  */
            int *edge_connectivity = (int *)  calloc(num_edges_per_elem*num_elem_in_block ,sizeof(int ));
            int *face_connectivity = (int *)  calloc(num_faces_per_elem*num_elem_in_block ,sizeof(int ));
            node_connectivity.resize(num_nodes_per_elem*num_elem_in_block);

            if(ex_get_conn(exodus_id, EX_ELEM_BLOCK, elem_block_id, &node_connectivity[0], edge_connectivity, face_connectivity)) {
                cerr << "error with ex_get_conn, error: " << error << endl;
                exit(-1);
            }

            free(edge_connectivity);
            free(face_connectivity);    
        }  
    }
}

void exodus_read_element_bboxes(const std::string &full_file_path, std::vector<point> &element_bboxes_as_pts, bbox &domain_bounds,
    uint32_t &num_nodes, vector<vector<size_t>> &node_ids_per_elem) 
{
    double domain_lower_bounds[3] = {DBL_MAX, DBL_MAX, DBL_MAX};
    double domain_upper_bounds[3] = {-DBL_MAX, -DBL_MAX, -DBL_MAX};

    int exodus_id = exodus_open_file(full_file_path);

    int num_dim, num_elem, num_elem_blocks, num_node_sets, num_side_sets;
    char  db_title[MAX_STR_LENGTH];
    if(ex_get_init (exodus_id, db_title, &num_dim, &num_nodes, &num_elem, &num_elem_blocks, &num_node_sets, &num_side_sets)) {
        std::cerr << "Error with ex_get_init" << std::endl;
        exit(-1);
    }
    if(!RETRIEVE_NODES_FOR_BBOXES) {
        num_nodes = num_elem; //want to keep track of the number of bounding boxes, not the number of nodes
    }

    vector<vector<uint32_t>> connectivity_lists(num_elem_blocks);
    uint32_t num_nodes_per_elem[num_elem_blocks];

    /* read in the ids for the element blocks */
    vector<int> elem_block_ids(num_elem_blocks);

    if(ex_get_ids (exodus_id, EX_ELEM_BLOCK, &elem_block_ids[0] )) {
        std::cerr << "error with ex_get_ids" << std::endl;
        exit(-1);
    }

    for(size_t i = 0; i < num_elem_blocks; i++) {
        int elem_block_id = elem_block_ids[i];
        exodus_get_element_connectivity(exodus_id, elem_block_id, connectivity_lists[i], num_nodes_per_elem[i]);
    }

    element_bboxes_as_pts.reserve(num_elem);
    vector<double> x_coords, y_coords, z_coords;
    exodus_read_vertex_coordinates(exodus_id, x_coords, y_coords, z_coords);
    ex_close (exodus_id);

    for(size_t i = 0; i < connectivity_lists.size(); i++) {
        size_t elem_size = num_nodes_per_elem[i];
        // cout << "new elem" << endl;
        for(size_t j = 0; j < connectivity_lists[i].size(); j += elem_size) {
            double mins[3] = {DBL_MAX, DBL_MAX, DBL_MAX};
            double maxes[3] = {-DBL_MAX, -DBL_MAX, -DBL_MAX};
            vector<size_t> node_ids;
            for(size_t k = 0; k < elem_size; k++) {
                size_t node_id = connectivity_lists[i][j+k]-1; //node_ids start at 1 instead of 0
                node_ids.push_back(node_id);
                mins[0] = std::min(mins[0], x_coords[node_id]);
                maxes[0] = std::max(maxes[0], x_coords[node_id]);
                mins[1] = std::min(mins[1], y_coords[node_id]);
                maxes[1] = std::max(maxes[1], y_coords[node_id]);
                mins[2] = std::min(mins[2], z_coords[node_id]);
                maxes[2] = std::max(maxes[2], z_coords[node_id]);
            }
            node_ids_per_elem.push_back(node_ids);
            element_bboxes_as_pts.push_back(point({mins[0], mins[1], mins[2]}));
            element_bboxes_as_pts.push_back(point({maxes[0], maxes[1], maxes[2]}));
            domain_lower_bounds[0] = std::min(domain_lower_bounds[0], mins[0]);
            domain_upper_bounds[0] = std::max(domain_upper_bounds[0], maxes[0]);
            domain_lower_bounds[1] = std::min(domain_lower_bounds[1], mins[1]);
            domain_upper_bounds[1] = std::max(domain_upper_bounds[1], maxes[1]);
            domain_lower_bounds[2] = std::min(domain_lower_bounds[2], mins[2]);
            domain_upper_bounds[2] = std::max(domain_upper_bounds[2], maxes[2]);
        }
    }    
    domain_bounds = bbox(point({domain_lower_bounds[0], domain_lower_bounds[1], domain_lower_bounds[2]}), 
                         point({domain_upper_bounds[0], domain_upper_bounds[1], domain_upper_bounds[2]})
                    );

}


void get_data_from_exodus_file(DataType data_type, const std::string &full_file_path, std::vector<point> &mesh_coords, 
    bbox &domain_bounds, uint32_t &num_data_pts) 
{
    exodus_read_vertex_coordinates(full_file_path, mesh_coords, domain_bounds, num_data_pts);
}

void get_data_from_exodus_file(DataType data_type, const std::string &full_file_path, std::vector<point> &mesh_coords, bbox &domain_bounds,
    uint32_t &num_data_pts, vector<vector<size_t>> &node_ids_per_elem) 
{
    exodus_read_element_bboxes(full_file_path, mesh_coords, domain_bounds, num_data_pts, node_ids_per_elem);
}


