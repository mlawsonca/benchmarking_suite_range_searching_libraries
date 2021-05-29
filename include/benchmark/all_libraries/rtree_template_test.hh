#ifndef RTREE_TEMPLATE_TEST_HH
#define RTREE_TEMPLATE_TEST_HH

#include "RTree.h"
#include <math.h>       /* floor */
#include <iostream>

using namespace std;

namespace TestRtreeTemplate {
    template<int bucket_size=NUM_ELEMS_PER_NODE>
    class Points: public BboxIntersectionTest {
        //index data type, element data type, num dims, element data type, max nodes, min nodes
        typedef RTree<size_t, double, NUM_DIMS, double, bucket_size, (const int)(bucket_size*.3)> rtree;
        private:
            rtree *tree;

        public:

            bool intersections_exact() { return true; } //bounding box search

            Points() {}
            ~Points() {
                free(tree);
            }

            void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
                tree = new rtree();
                for(size_t i=0; i < pts.size(); i++)
                {
                    //doesn't support points but we can insert a "bounding box" where min=max
                    tree->Insert(&pts[i][0], &pts[i][0], indices[i]);
                }
            }

            void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
                tree->Search(&my_bbox.first[0], &my_bbox.second[0], intersections_indices);
            }
    };

    template<int bucket_size=NUM_ELEMS_PER_NODE>
    class Bboxes: public BboxIntersectionTest {
        //index data type, element data type, num dims, element data type, max nodes, min nodes
        typedef RTree<size_t, double, NUM_DIMS, double, bucket_size, (const int)(bucket_size*.3)> rtree;
        private:
            rtree *tree;

        public:

            bool intersections_exact() { return true; } //bounding box search

            Bboxes() {}
            ~Bboxes() {
                free(tree);
            }

            void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
                if((pts.size() % 2) !=0) {
                    std::cerr << "error. your point list size has to be even to insert bounding boxes" << std::endl;
                    return;
                }
                tree = new rtree();
                for(size_t i=0; i < pts.size(); i+=2)
                {
                    //doesn't support points but we can insert a "bounding box" where min=max
                    tree->Insert(&pts[i][0], &pts[i+1][0], indices[i/2]);
                }
            }

            void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
                tree->Search(&my_bbox.first[0], &my_bbox.second[0], intersections_indices);
            }
    };
}

#endif //RTREE_TEMPLATE_TEST_HH

