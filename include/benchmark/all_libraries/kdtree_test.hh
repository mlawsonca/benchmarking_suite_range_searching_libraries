#ifndef KDTREE_TEST_HH
#define KDTREE_TEST_HH

#include "KDTree.hpp"

using namespace std;


class TestKDTree : public BboxIntersectionTest {
    private:
        KDTree *tree;

    public:

        bool intersections_exact() { return false; } //circular radius isnt exact

        TestKDTree() {}
        ~TestKDTree() {
            free(tree);
        }
   

        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
            tree = new KDTree(pts);
        }

        void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
            point mid_pt;
            double radius_search_bound = 0;
            get_max_radius(my_bbox, mid_pt, radius_search_bound);
            radius_search_bound+=DEFAULT_TOLERANCE;

            intersections_indices = tree->neighborhood_indices(mid_pt, radius_search_bound);
        }

};

#endif //KDTREE_TEST_HH
