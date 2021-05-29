#ifndef PICO_TREE_TEST_HH
#define PICO_TREE_TEST_HH

#include <pico_tree/kd_tree.hpp>
#include <pico_adaptor.hpp>

using namespace std;



class TestPicoTree : public BboxIntersectionTest {

    class PicoPoint {
        private:
            point my_pt;
        public:
            using ScalarType = double;
            static constexpr int Dim = NUM_DIMS;

            PicoPoint(const point &pt) {
                my_pt = pt;
            }

            inline double const& operator()(int const i) const { return my_pt[i]; }
            inline double& operator()(int const i) { return my_pt[i]; }

    };

    typedef pico_tree::KdTree<size_t, double, NUM_DIMS, PicoAdaptor<size_t, PicoPoint>> kdtree;

    private:
        kdtree *tree;
        //have to keep this in scope
        vector<PicoPoint> pico_pts;

        void _build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size) {
            for(int i = 0; i < pts.size(); i++) {
                pico_pts.push_back(PicoPoint(pts[i]));
            }
            PicoAdaptor<size_t, PicoPoint> adaptor = PicoAdaptor<size_t, PicoPoint>(pico_pts);
            tree = new kdtree(adaptor, bucket_size);
        }
    public:

        bool intersections_exact() { return true; } //bounding box search

        TestPicoTree() {}
        ~TestPicoTree() {
            delete tree;
        }


        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
            _build_tree(pts, indices, NUM_ELEMS_PER_NODE);
        }

        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size) {
            _build_tree(pts, indices, bucket_size);
        }

        //closed region -> includes Points that fall on the boundary
        void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
            tree->SearchBox(PicoPoint(my_bbox.first), PicoPoint(my_bbox.second), &intersections_indices);
        }

};

#endif //PICO_TREE_TEST_HH