
#ifndef COMMON_HH
#define COMMON_HH

#include <vector>
#include <iostream>
#include <chrono> /* high resolution clock */
#include <math.h>       /* pow */

#define NUM_DIMS 3
#define NUM_ELEMS_PER_NODE 20
#define DOMAIN_LENGTH 5
#define DEFAULT_TOLERANCE .00001
#define DEFAULT_LARGE_TOLERANCE .0001

typedef std::vector<double> point;
typedef std::pair<point, size_t> point_w_index;
typedef std::pair<point, point> bbox;
typedef std::pair<bbox, size_t> bbox_w_index;

typedef std::vector<float> point_f; // flann with CUDA only supports floats
typedef std::pair<point_f, size_t> point_w_index_f;
typedef std::pair<point_f, point_f> bbox_f;
typedef std::pair<bbox_f, point_f> bbox_w_index_f;


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
        virtual void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) = 0;
        virtual void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) = 0;
};


#endif //COMMON_HH