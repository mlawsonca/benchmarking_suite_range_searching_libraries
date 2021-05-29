#ifndef KDTREE3_TEST_HH
#define KDTREE3_TEST_HH

#include "kdtree/static_3d_tree.hpp"
#include "point_cloud/point_cloud.hpp"
#include "point_type/point_type.hpp"

using namespace std;



class TestKDTree3 : public BboxIntersectionTest {
    typedef kd_tree::Static3dTree<point_cloud::PointCloud<point_type::Point3d>> kdtree;

    private:
        kdtree *tree;

        void _build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size) {
            vector<point_type::Point3d> points_kdtree;
            for(int i = 0; i < pts.size(); i++) {
                points_kdtree.push_back(point_type::Point3d(pts[i][0], pts[i][1], pts[i][2]));
            }

            tree = new kdtree(bucket_size);
            tree->build(points_kdtree);
        }
    public:

        bool intersections_exact() { return false; } //radius of circle is not exact

        TestKDTree3() {}
        ~TestKDTree3() {
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
            point mid_pt;
            double radius_search_bound = 0;   
            get_max_radius(my_bbox, mid_pt, radius_search_bound);
            radius_search_bound += DEFAULT_TOLERANCE;
            point_type::Point3d query = point_type::Point3d(mid_pt[0], mid_pt[1], mid_pt[2]);

            tree->radiusSearch(query, radius_search_bound, intersections_indices);
        }

};

#endif //KDTREE3_TEST_HH
