#ifndef LIBKDTREE_TEST_HH
#define LIBKDTREE_TEST_HH

#include <kdtree++/kdtree.hpp>


using namespace std;


class TestLibkdtree : public BboxIntersectionTest {

    struct my_point {
        typedef double value_type;
        point_w_index pt;

        my_point(const point_w_index &p) {
            pt = p;
        }
        inline value_type operator[](size_t const i) const { return pt.first[i]; }
        inline size_t index() {
            return pt.second;
        }
    };


    typedef LibKDTree::KDTree<NUM_DIMS,my_point> kdtree;
    private:

        kdtree *tree;
    public:
        bool intersections_exact() { return false; } //manhattan distance is not exact

        TestLibkdtree() {}
        ~TestLibkdtree() {
            delete tree;
        }


        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
            vector<my_point> pts_w_index;
            for(int i = 0; i < pts.size(); i++) {
                pts_w_index.push_back(my_point(point_w_index(pts[i],indices[i])));
            }
            tree = new kdtree();
            tree->insert(pts_w_index.begin(), pts_w_index.end());
            //triggers rebalance, recommended to do after insertions are finished and after every deletion (which also requires searching the tree)
            tree->optimize();
        }

        //closed region -> includes points that fall on the boundary
        void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
            std::vector<my_point> matching_pts;

            point mid_pt;
            double radius_search_bound = 0;
            get_manhattan_radius(my_bbox, mid_pt, radius_search_bound);

            vector<my_point> results;
            my_point mid_pt_w_index = my_point(point_w_index(mid_pt, 0));
            tree->find_within_range(mid_pt_w_index, radius_search_bound, std::back_inserter(results));
            intersections_indices.reserve(results.size());
            for(int i = 0; i < results.size(); i++) {
                intersections_indices.push_back(results[i].index());
            }
        }

};

#endif //LIBKDTREE_TEST_HH

