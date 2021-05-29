#ifndef TEST_CONFIGURATIONS_HH
#define TEST_CONFIGURATIONS_HH

#include <vector>

enum Library : unsigned short {
    ALGLIB = 0,
    ANN = 1,
    BOOST_RTREE = 2,
    BRUTE_FORCE = 3,
    CGAL_LIBRARY = 4,
    FLANN = 5,
    KDTREE = 6,
    KDTREE2 = 7,
    KDTREE3 = 8,
    KDTREE4 = 9,
    LIB3DTK = 10,
    LIBKDTREE = 11,
    LIBKDTREE2 = 12,
    LIBNABO = 13,
    LIBSPATIALINDEX = 14,
    NANOFLANN = 15,
    OCTREE = 16,
    PCL = 17,
    PICO_TREE = 18,
    RTREE_TEMPLATE = 19,
    SPATIAL = 20 
};

enum DataType : unsigned short {
    POINTS,
    BBOXES,
    TRIANGLES   
};

enum QueryType : unsigned short {
    STANDARD,
    GPU_DOMAIN_DECOMP
};

struct testing_config {
    std::vector<double> domain_lower_bounds;
    std::vector<double> domain_upper_bounds;
    size_t num_data_pts; 
    size_t num_queries; 
    Library library;
    DataType data_type;
    short unsigned library_option;

    testing_config(const std::vector<double> &domain_lower_bnds, const std::vector<double> &domain_upper_bnds, 
        size_t n_data_pts, size_t n_queries, Library lib, DataType d_type, short unsigned lib_option
        ) 
    {
        domain_lower_bounds = domain_lower_bnds;
        domain_upper_bounds = domain_upper_bnds;
        num_data_pts = n_data_pts; 
        num_queries = n_queries; 
        library = lib;
        data_type = d_type;
        library_option = lib_option;
    }

    testing_config() { 
    }


    template <typename Archive>
    void serialize(Archive& ar, const unsigned int ver)
    {
        ar & domain_lower_bounds;
        ar & domain_upper_bounds;
        ar & num_data_pts;
        ar & num_queries;
        ar & library;
        ar & data_type;
        ar & library_option;
    }
};


#endif //TEST_CONFIGURATIONS_HH