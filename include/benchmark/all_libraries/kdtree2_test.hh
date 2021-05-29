#ifndef KDTREE2_TEST_HH
#define KDTREE2_TEST_HH
  
#include "kdtree2.hpp"
#include <boost/multi_array.hpp>
// #include <boost/random.hpp>

using namespace std;



class TestKDTree2 : public BboxIntersectionTest {
    typedef boost::multi_array<float,2> KDTreeArray;

    private:
        kdtree2::KDTree* tree;
        KDTreeArray *data;

  
        //rearrange points is designed to make leaves more contiguous in memory/help ensure that topologically close nodes are contiguous
        void _build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size, bool rearrange_points) {
            data = new KDTreeArray;
            data->resize(boost::extents[pts.size()][NUM_DIMS]);

            for (int i=0; i<pts.size(); i++) {
                for (int j=0; j<NUM_DIMS; j++) {
                    (*data)[i][j] = pts[i][j];
                }
            }
            tree = new kdtree2::KDTree(*data,rearrange_points,bucket_size); 
        }
        struct kdtree2_result_iterator : kdtree2::KDTreeResultVector::const_iterator
        {
            using base_class = kdtree2::KDTreeResultVector::const_iterator;
            using value_type = size_t;

            using base_class::base_class;

            value_type operator*() const {
                auto const& src = base_class::operator*();
                return (src.idx);
            }
        };
    public:

        bool intersections_exact() { return false; } //circular radius is not exact

        TestKDTree2() {}
        ~TestKDTree2() {
            delete tree;
            delete data;
        }


        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
            _build_tree(pts, indices, NUM_ELEMS_PER_NODE, false);
        }

        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size, bool rearrange_points=false) {
            _build_tree(pts, indices, bucket_size, rearrange_points);
        }

        //closed region -> includes Points that fall on the boundary
        void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
            point_f mid_pt;
            float squared_radius_search_bound = 0;
            get_max_squared_radius(my_bbox, mid_pt, squared_radius_search_bound);

            kdtree2::KDTreeResultVector result; 

            tree->r_nearest(mid_pt, squared_radius_search_bound, result);
            intersections_indices.assign(kdtree2_result_iterator(result.begin()),
                                       kdtree2_result_iterator(result.end()));
        }

};

#endif //KDTREE2_TEST_HH