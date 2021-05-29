#ifndef OCTREE_TEST_HH
#define OCTREE_TEST_HH

#include "Octree.hpp"

template <>
struct unibn::traits::access<point, 0>
{
  static double get(const point& p)
  {
    return p[0];
  }
};

template <>
struct unibn::traits::access<point, 1>
{
  static double get(const point& p)
  {
    return p[1];
  }
};

template <>
struct unibn::traits::access<point, 2>
{
  static double get(const point& p)
  {
    return p[2];
  }
};



class TestOctree : public BboxIntersectionTest {        

    private:

        unibn::Octree<point> *tree;

        void _build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size = NUM_ELEMS_PER_NODE)
        {
            //params: uint32_t bucketSize = 32, bool copyPoints = false, float minExtent = 0.0f
            unibn::OctreeParams params(bucket_size);
            tree = new unibn::Octree<point>();
            tree ->initialize(pts);
        }

    public:

        bool intersections_exact() { return false; } //circular radius is not exact

        TestOctree() {}

        ~TestOctree() {
            delete tree;
        }


        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
            _build_tree(pts, indices);
        }

        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size) {
            //set both interior node capacity and leaf capacity to bucket size
            _build_tree(pts, indices, bucket_size);
        }

        void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
            point mid_pt;
            double squared_radius_search_bound = 0;
            get_max_radius(my_bbox, mid_pt, squared_radius_search_bound);
            squared_radius_search_bound += DEFAULT_TOLERANCE;

            //requires using uint32_t instead of size_t
            std::vector<uint32_t> indices;  
            tree->radiusNeighbors<unibn::L2Distance<point>>(mid_pt, squared_radius_search_bound, indices);
            std::copy(indices.begin(), indices.end(), std::back_inserter(intersections_indices));
        }

};

#endif //OCTREE_TEST_HH
