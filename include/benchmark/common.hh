#ifndef COMMON_HH
#define COMMON_HH

#include <stdlib.h>
#include <vector>
#include <iostream>
#include <chrono> /* high resolution clock */
#include <math.h>       /* pow */
#include "testing_configurations.hh"
#include <mpi.h>

#include <sstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>


#define NUM_DIMS 3
#define NUM_ELEMS_PER_NODE 20
#define LARGE_NUM_ELEMS_PER_NODE 200
#define DOMAIN_LENGTH 5
#define DEFAULT_TOLERANCE .00001
#define DEFAULT_LARGE_TOLERANCE .0001
#define RETRIEVE_NODES_FOR_BBOXES false

#ifndef DEBUG
    #define DEBUG false
#endif

extern bool USE_MPI;

typedef std::vector<double> point;
typedef std::pair<point, size_t> point_w_index;
typedef std::pair<point, point> bbox;
typedef std::pair<bbox, size_t> bbox_w_index;

typedef std::vector<float> point_f; // flann with CUDA only supports floats
typedef std::pair<point_f, size_t> point_w_index_f;
typedef std::pair<point_f, point_f> bbox_f;
typedef std::pair<bbox_f, point_f> bbox_w_index_f;


inline void print_results_header() {
    std::cout << "category, library option name, time elapsed (ns), avg perc data pts intersected, library, library option, num data pts written, num queries, x min, x max, y min, y max, z min, z max" << std::endl;
}

inline void print_config(const testing_config &config) {
    std::cout << ", " << config.library << ", " << config.library_option;
    std::cout << ", " << config.num_data_pts << ", " << config.num_queries;
    for(size_t i = 0; i < config.domain_lower_bounds.size(); i++) {
        std::cout <<  ", " << config.domain_lower_bounds[i] << ", " << config.domain_upper_bounds[i];
    }
    std::cout << std::endl << std::flush;
}



//asssumes the rank is 0
template <class T>
void combine_ser( int num_procs, int *each_proc_ser_values_size, int *displacement_for_each_proc, 
        char *serialized_c_str_all_ser_values, const T &my_val, std::vector<T> &all_vals)
{
    for(int i = 0; i < num_procs; i++) {
        int offset = displacement_for_each_proc[i];
        int count = each_proc_ser_values_size[i];
        if(count > 0) {
            T rec_value;

            //0 rank does not need to deserialize its own attrs
            if(i != 0) {

                char serialzed_vals_for_one_proc[count];

                memcpy ( serialzed_vals_for_one_proc, serialized_c_str_all_ser_values + offset, count);

                std::stringstream ss1;
                ss1.write(serialzed_vals_for_one_proc, count);
                boost::archive::text_iarchive ia(ss1);
                ia >> rec_value;
            }
            else { //i == rank
                rec_value = my_val;
            }
            all_vals.push_back(rec_value);
        }
    }
    free(serialized_c_str_all_ser_values);
}


template <class T>
void gatherv_ser_and_combine(const T &value, uint32_t num_procs, int rank, MPI_Comm comm,
    std::vector<T> &all_vals
    )
{
    int each_proc_ser_values_size[num_procs];
    int displacement_for_each_proc[num_procs];
    char *serialized_c_str_all_ser_values;

    int length_ser_c_str = 0;
    char *serialized_c_str;

    std::stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << value;
    std::string serialized_str = ss.str();
    length_ser_c_str = serialized_str.size() + 1;
    serialized_c_str = (char *) malloc(length_ser_c_str);
    serialized_str.copy(serialized_c_str, serialized_str.size());
    serialized_c_str[serialized_str.size()]='\0';

    MPI_Gather(&length_ser_c_str, 1, MPI_INT, each_proc_ser_values_size, 1, MPI_INT, 0, comm);

    int sum = 0;
    // int max_value = 0;
    if (rank == 0 ) {
        for(int i=0; i<num_procs; i++) {
            displacement_for_each_proc[i] = sum;
            sum += each_proc_ser_values_size[i];
        }
        serialized_c_str_all_ser_values = (char *) malloc(sum);
    }

    MPI_Gatherv(serialized_c_str, length_ser_c_str, MPI_CHAR,
           serialized_c_str_all_ser_values, each_proc_ser_values_size, displacement_for_each_proc,
           MPI_CHAR, 0, comm);

    free(serialized_c_str);

    if(rank == 0) {
        all_vals.reserve(num_procs);

        combine_ser( num_procs, each_proc_ser_values_size, displacement_for_each_proc, 
        serialized_c_str_all_ser_values, value, all_vals);
    }
}


inline void print_build_time(std::string test_name, std::chrono::high_resolution_clock::time_point build_start_time, testing_config config) {
    std::chrono::high_resolution_clock::time_point build_stop_time = std::chrono::high_resolution_clock::now();
    uint64_t build_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(build_stop_time - build_start_time).count();
    int num_procs, rank;

    if(USE_MPI) {
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);    
        MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
        std::vector<testing_config> all_configs;
        uint64_t all_build_times[num_procs];

        MPI_Gather(&build_time_ns, 1, MPI_UINT64_T, all_build_times, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);

        gatherv_ser_and_combine(config, num_procs, rank, MPI_COMM_WORLD, all_configs);
        if(rank == 0) {
            for(int i = 0; i < all_configs.size(); i++) {
                //cateogry: build time, library option name: test_name, time elapsed: all_build_times[i], avg num query points returned: 0 
                std::cout << "build time, " << test_name << ", " << all_build_times[i] << ", 0";
                print_config(all_configs[i]);    
            }
        }          
    }
    else {
        std::cout << "build time, " << test_name << ", " << build_time_ns << ", 0";
        print_config(config);           
    }

}

inline void print_query_time(double query_percent_data_covered, std::string test_name, 
        std::chrono::high_resolution_clock::time_point query_start_time, double avg_perc_features_intersected, testing_config config) {

    std::chrono::high_resolution_clock::time_point query_stop_time = std::chrono::high_resolution_clock::now();
    uint64_t query_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(query_stop_time - query_start_time).count();
    int num_procs, rank;

    if(USE_MPI) {
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);    
        MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
        std::vector<testing_config> all_configs;
        uint64_t all_query_times[num_procs];
        double all_avg_perc_features_intersected[num_procs];

        MPI_Gather(&query_time_ns, 1, MPI_UINT64_T, all_query_times, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
        MPI_Gather(&avg_perc_features_intersected, 1, MPI_DOUBLE, all_avg_perc_features_intersected, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        gatherv_ser_and_combine(config, num_procs, rank, MPI_COMM_WORLD, all_configs);
        if(rank == 0) {
            for(int i = 0; i < all_configs.size(); i++) {
                //cateogry: query time + %data covered, library option name: test_name, time elapsed: query_time_ns, avg num query points returned: avg_num_intersected_data_pts
                std::cout << "query time " << std::to_string(query_percent_data_covered) << ", " << test_name << ", " << all_query_times[i];
                std::cout << ", " << all_avg_perc_features_intersected[i];  
                print_config(all_configs[i]);    
            }
        }        
    }
    else {
        std::cout << "query time " << std::to_string(query_percent_data_covered) << ", " << test_name << ", " << query_time_ns;
        std::cout << ", " << avg_perc_features_intersected;
        print_config(config);         
    }
}


template <class T>
void print_point(T x, T y, T z, bool suppress_newline = false) {
    std::cout << "pt: (" << x << ", " << y << ", " << z << ")"; 
    if(!suppress_newline) {
        std::cout << std::endl; 
    }
}

template <class T>
void print_point(const std::vector<T> &pt, bool suppress_newline = false) {
    std::cout << "pt: (" << pt[0] << ", " << pt[1] << ", " << pt[2] << ")";
    if(!suppress_newline) {
        std::cout << std::endl; 
    }
}

template <class T>
void print_bbox(const std::pair<std::vector<T>,std::vector<T>> &box, bool suppress_newline = false) {
    std::cout << "bbox: (" << box.first[0] << ", " << box.first[1] << ", " << box.first[2] << ")-(" << 
                         box.second[0] << ", " << box.second[1] << ", " << box.second[2] << ")";

    if(!suppress_newline) {
        std::cout << std::endl; 
    }
}

static void print_data(const std::vector<point> &pts, size_t index, DataType data_type) {
    if(data_type == POINTS) {
        print_point(pts[index]);
    }
    else if(data_type == BBOXES) {
        std::cout << "bbox: " << std::endl; 
        print_point(pts[2*index]);
        print_point(pts[2*index+1]);
    }
    else if(data_type == TRIANGLES) {
        std::cout << "triangle: " << std::endl; 
        print_point(pts[3*index]);
        print_point(pts[3*index+1]);
        print_point(pts[3*index+2]);
    }
}



template <class T>
void get_max_squared_radius(const bbox &my_bbox, std::vector<T> &mid_pt, T &squared_radius_search_bound) {
    squared_radius_search_bound = 0;
    for(int i = 0; i < my_bbox.first.size(); i++) {
        mid_pt.push_back((my_bbox.first[i] + my_bbox.second[i]) / 2);
        squared_radius_search_bound += ( pow((mid_pt.back()-my_bbox.first[i]),2));
    }

}

template <class T>
void get_max_radius(const bbox &my_bbox, std::vector<T> &mid_pt, T &squared_radius_search_bound) {
    get_max_squared_radius(my_bbox, mid_pt, squared_radius_search_bound);
    squared_radius_search_bound = sqrt(squared_radius_search_bound);
}

template <class T>
void get_manhattan_radius(const bbox &my_bbox, std::vector<T> &mid_pt, T &radius_search_bound) {
    radius_search_bound = 0;
    for(int i = 0; i < my_bbox.first.size(); i++) {
        mid_pt.push_back((my_bbox.first[i] + my_bbox.second[i]) / 2);
        double diff = (my_bbox.second[i]-mid_pt.back());
        if( diff > radius_search_bound) {
            radius_search_bound = diff;
        }
    }
}


class BboxIntersectionTest {
    public:
        virtual bool intersections_exact() = 0;

        //can't templatize pure virtual functions
        virtual void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) = 0;
        virtual void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) = 0;
};


#endif //COMMON_HH