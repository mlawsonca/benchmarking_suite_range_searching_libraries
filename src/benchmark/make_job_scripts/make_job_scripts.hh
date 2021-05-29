#ifndef MAKE_JOB_SCRIPTS_HH
#define MAKE_JOB_SCRIPTS_HH

#include "../../../include/benchmark/testing_configurations.hh"

struct debugLog {
  private:
    bool on;
    bool zero_rank_logging;
    int my_rank;

  public:
  debugLog(bool turn_on, bool zero_rank_log=false, int rank=-1) {
    on = turn_on;
    zero_rank_logging = zero_rank_log;
    my_rank = rank;
  }
  void set_rank(int rank) {
    my_rank = rank;
  }
  void turn_on_zero_rank_logging() {
    zero_rank_logging = true;
  }
  void turn_off_zero_rank_logging() {
    zero_rank_logging = false;
  }
  void turn_on_logging() {
    on = true;
  }
  void turn_off_logging() {
    on = false;
  }
  template<typename T> debugLog& operator << (const T& x) {
   if(on || (zero_rank_logging && (my_rank == 0) )) {
      std::cout << x;
    }
    return *this;
  }
  debugLog& operator<<(std::ostream& (*manipulator)(std::ostream&)) {
   if(on || (zero_rank_logging && (my_rank == 0) )) {
      std::cout << manipulator;
    }
    return *this;
  }
};




std::string get_library_name(Library lib) {
    switch(lib) {
        case ALGLIB : {
            return "ALGLIB";
        }
        case ANN : {
            return "ANN";
        }
        case BOOST_RTREE : {
            return "BOOST_RTREE";
        }
        case BRUTE_FORCE : {
            return "BRUTE_FORCE";
        }
        case CGAL_LIBRARY : {
            return "CGAL";
        }
        case FLANN : {
            return "FLANN";
        }        
        case KDTREE : {
            return "KDTREE";
        }
        case KDTREE2 : {
            return "KDTREE2";
        }
        case KDTREE3 : {
            return "KDTREE3";
        }
        case KDTREE4 : {
            return "KDTREE4";
        }
        case LIB3DTK : {
            return "3DTK";
        }
        case LIBKDTREE : {
            return "LIBKDTREE";
        }
        case LIBKDTREE2 : {
            return "LIBKDTREE2";
        }
        case LIBNABO : {
            return "LIBNABO";
        }
        case LIBSPATIALINDEX : {
            return "LIBSPATIALINDEX";
        }
        case NANOFLANN : {
            return "NANOFLANN";
        }
        case OCTREE : {
            return "OCTREE";
        }
        case PCL : {
            return "PCL";
        }
        case PICO_TREE : {
            return "PICO_TREE";
        }
        case RTREE_TEMPLATE : {
            return "RTREE_TEMPLATE";
        }
        case SPATIAL : {
            return "SPATIAL";
        }
        default : {
            return "ERROR";
        }
    }
}

std::string get_data_type_name(DataType data_type) {
    switch(data_type) {
        case POINTS : { 
            return "POINTS";
        }
        case(BBOXES) : {
            return "BBOXES";
        }
        case(TRIANGLES) : {
            return "TRIANGLES";
        }
        default : {
            return "ERROR";
        }
    }
}

struct run_configs {
    Library library;
    DataType data_type;
    size_t num_queries;
    std::vector<unsigned short> library_options;

    run_configs(Library lib, const std::vector<unsigned short> &lib_options, DataType d_type=POINTS, size_t n_queries=10000) {
        library = lib;
        library_options = lib_options;
        data_type = d_type;
        num_queries = n_queries;
    }

    run_configs(Library lib, size_t num_lib_options, DataType d_type=POINTS, size_t n_queries=10000) {
        library = lib;
        data_type = d_type;
        num_queries = n_queries;
        for(size_t i = 0; i < num_lib_options; i++) {
            library_options.push_back(i);
        }
    }
};


struct run_config {
    Library library;
    DataType data_type;
    size_t num_queries;
    unsigned short num_library_options;
    unsigned short library_option;

    run_config(Library lib, size_t num_lib_options, DataType d_type=POINTS, size_t lib_option=0, size_t n_queries=10000) {
        library = lib;
        num_library_options = num_lib_options;
        data_type = d_type;
        num_queries = n_queries;
        library_option = lib_option;
    }
};


void print_config(run_config config) {
    std::cout << "library=" << get_library_name(config.library) << ", data_type=" << get_data_type_name(config.data_type) << 
        ", option: " << int(config.library_option) << std::endl;
}

std::vector<run_config> get_run_configs() {
    std::vector<run_config> configs = {
        run_config(ALGLIB, 1),
        run_config(ANN, 4),
        run_config(BOOST_RTREE, 4),
        run_config(BRUTE_FORCE, 1),
        run_config(CGAL_LIBRARY, 3),
        run_config(FLANN, 4),
        run_config(KDTREE, 1),
        run_config(KDTREE2, 4),
        run_config(KDTREE3, 2),
        run_config(KDTREE4, 1),
        run_config(LIB3DTK, 2),
        run_config(LIBKDTREE, 1),
        run_config(LIBKDTREE2, 1),
        run_config(LIBNABO, 4),
        run_config(LIBSPATIALINDEX, 4),
        run_config(NANOFLANN, 4),
        run_config(OCTREE, 2),
        run_config(PCL, 5),
        run_config(PICO_TREE, 2),
        run_config(RTREE_TEMPLATE, 2),
        run_config(SPATIAL, 1),
        run_config(BOOST_RTREE, 2, BBOXES),
        run_config(BRUTE_FORCE, 1, BBOXES),
        run_config(CGAL_LIBRARY, 1, BBOXES),
        run_config(LIBSPATIALINDEX, 4, BBOXES),
        run_config(RTREE_TEMPLATE, 2, BBOXES),
        run_config(SPATIAL, 1, BBOXES)
    };

    std::vector<run_config> configs_adjusted;
    for(run_config config :  configs) {
        for(size_t i = 0; i < config.num_library_options; i++) {
            run_config copy = config;
            copy.library_option = i;
            configs_adjusted.push_back(copy);
        }
    }

    for(const run_config &config :  configs_adjusted) {
        print_config(config);
    }
    return configs_adjusted;

}


std::vector<run_config> get_best_large_run_configs() {
    std::vector<run_configs> configs = {
        run_configs(ALGLIB, 1),
        //performance almost exactly the same for linear, quadratic, and rstar
        run_configs(BOOST_RTREE, 2),
        run_configs(BRUTE_FORCE, 1),
        run_configs(CGAL_LIBRARY, 2), //eliminated range tree
        run_configs(KDTREE2, 4),
        run_configs(LIBKDTREE2, 1),
        run_configs(OCTREE, 2),
        run_configs(PICO_TREE, 2),
        run_configs(RTREE_TEMPLATE, 2),
        //these have yet to run, so cannot confirm
        run_configs(BOOST_RTREE, 2, BBOXES),
        run_configs(BRUTE_FORCE, 1, BBOXES),
        run_configs(CGAL_LIBRARY, std::vector<unsigned short>({1}), BBOXES),
        run_configs(RTREE_TEMPLATE, 2, BBOXES),
    };

    std::vector<run_config> configs_adjusted;
    for(run_configs config :  configs) {
        for(size_t i = 0; i < config.library_options.size(); i++) {
            run_config copy(config.library, config.library_options.size(), config.data_type, config.library_options[i], config.num_queries);
            configs_adjusted.push_back(copy);
        }
    }

    for(const run_config &config :  configs_adjusted) {
        print_config(config);
    }
    return configs_adjusted;

}

std::vector<run_config> get_configs_for_libraries_without_errors() {
    std::vector<run_configs> configs = {
        run_configs(ALGLIB, 1),
        // run_configs(ANN, 4), none of the small jobs finsihed
        run_configs(BOOST_RTREE, 2), //performance almost exactly the same for linear, quadratic, and rstar so only test one algorithm
        run_configs(BRUTE_FORCE, 1),
        run_configs(CGAL_LIBRARY, 2), //eliminated range tree
        run_configs(FLANN, 2), //gpu out of memory error, so gpu jobs omitted
        run_configs(KDTREE, 1),
        run_configs(KDTREE2, 4),
        run_configs(KDTREE3, 2),
        run_configs(KDTREE4, 1),
        run_configs(LIB3DTK, 2),
        run_configs(LIBKDTREE, 1),
        run_configs(LIBKDTREE2, 1),
        run_configs(LIBNABO, std::vector<unsigned short>({2,3})), //dont need to test the linear time heap
        run_configs(LIBSPATIALINDEX, 4),
        run_configs(NANOFLANN, 4),
        run_configs(OCTREE, 2),
        run_configs(PCL, 3), //gpu out of memory error, so gpu jobs omitted
        run_configs(PICO_TREE, 2),
        run_configs(RTREE_TEMPLATE, 2),
        run_configs(SPATIAL, 1),
        run_configs(BOOST_RTREE, 2, BBOXES),
        run_configs(BRUTE_FORCE, 1, BBOXES),
        run_configs(CGAL_LIBRARY, std::vector<unsigned short>({1}), BBOXES),
        run_configs(LIBSPATIALINDEX, 4, BBOXES),
        run_configs(RTREE_TEMPLATE, 2, BBOXES),
        run_configs(SPATIAL, 1, BBOXES)
    };

    std::vector<run_config> configs_adjusted;
    for(run_configs config :  configs) {
        for(size_t i = 0; i < config.library_options.size(); i++) {
            run_config copy(config.library, config.library_options.size(), config.data_type, config.library_options[i], config.num_queries);
            configs_adjusted.push_back(copy);
        }
    }

    for(const run_config &config :  configs_adjusted) {
        print_config(config);
    }
    return configs_adjusted;

}


std::vector<run_config> get_multinode_scaling_small_run_configs() {
    uint32_t num_queries = 1000;

    std::vector<run_configs> configs = {
        run_configs(CGAL_LIBRARY, 1, POINTS, num_queries),
        run_configs(KDTREE, 1, POINTS, num_queries),
        run_configs(KDTREE3, std::vector<unsigned short>({1}), POINTS, num_queries),

        run_configs(BOOST_RTREE, std::vector<unsigned short>({1}), BBOXES, num_queries),
        run_configs(SPATIAL, 1, BBOXES, num_queries)

    };

    std::vector<run_config> configs_adjusted;
    for(run_configs config :  configs) {
        for(size_t i = 0; i < config.library_options.size(); i++) {
            run_config copy(config.library, config.library_options.size(), config.data_type, config.library_options[i], config.num_queries);
            configs_adjusted.push_back(copy);
        }
    }

    for(const run_config &config :  configs_adjusted) {
        print_config(config);
    }
    return configs_adjusted;
}


std::vector<run_config> get_multinode_scaling_large_run_configs() {
    uint32_t num_queries = 1000;

    std::vector<run_configs> configs = {
        run_configs(ALGLIB, 1, POINTS, num_queries),
        run_configs(KDTREE2, std::vector<unsigned short>({3}), POINTS, num_queries),

        run_configs(BOOST_RTREE, std::vector<unsigned short>({1}), BBOXES, num_queries),
        run_configs(RTREE_TEMPLATE, std::vector<unsigned short>({1}), BBOXES, num_queries),
    };

    std::vector<run_config> configs_adjusted;
    for(run_configs config :  configs) {
        for(size_t i = 0; i < config.library_options.size(); i++) {
            run_config copy(config.library, config.library_options.size(), config.data_type, config.library_options[i], config.num_queries);
            configs_adjusted.push_back(copy);
        }
    }

    for(const run_config &config :  configs_adjusted) {
        print_config(config);
    }
    return configs_adjusted;
}
#endif //MAKE_JOB_SCRIPTS_HH