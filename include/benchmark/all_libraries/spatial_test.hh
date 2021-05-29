#ifndef SPATIAL_TEST_HH
#define SPATIAL_TEST_HH

#include "point_multimap.hpp"
#include "idle_point_multimap.hpp"
#include "box_multimap.hpp"
#include "idle_box_multimap.hpp"
#include "region_iterator.hpp"

using namespace std;



namespace TestSpatial {

    class Points : public BboxIntersectionTest { 
        typedef spatial::idle_point_multimap<NUM_DIMS, point, size_t> kdtree;
        typedef spatial::point_multimap<NUM_DIMS, point, size_t> kdtree_self_balancing;

        private:

            kdtree *tree;
            kdtree_self_balancing *tree_self_balancing;
            bool self_balancing = false;

        public:

            bool intersections_exact() { return true; } //bounding box (region) search

            Points() {}
            ~Points() {
                delete tree;
                delete tree_self_balancing;
            }


            void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
                vector<point_w_index> pts_w_index;
                for(int i = 0; i < pts.size(); i++) {
                    pts_w_index.push_back(point_w_index(pts[i],indices[i]));
                }

                if(self_balancing) {
                    tree_self_balancing = new kdtree_self_balancing(); 
                    tree_self_balancing->insert(pts_w_index.begin(), pts_w_index.end());
                }
                else {
                    tree = new kdtree(); 
                    tree->insert_rebalance(pts_w_index.begin(), pts_w_index.end());
                }        
            }

            //closed region -> includes points that fall on the boundary
            void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
                typedef spatial::closed_region_iterator<kdtree> iterator;
                typedef spatial::closed_region_iterator<kdtree_self_balancing> iterator_balanced;

                if(self_balancing) {
                    for (iterator_balanced i = spatial::closed_region_begin(*tree_self_balancing, my_bbox.first, my_bbox.second); i != spatial::closed_region_end(*tree_self_balancing, my_bbox.first, my_bbox.second); ++i) {
                        intersections_indices.push_back((*i).second);
                    }
                }
                else {
                    for (iterator i = spatial::closed_region_begin(*tree, my_bbox.first, my_bbox.second); i != spatial::closed_region_end(*tree, my_bbox.first, my_bbox.second); ++i) {
                        intersections_indices.push_back((*i).second);
                    }
                }
            }  
    };

    class Bboxes : public BboxIntersectionTest { 
        typedef spatial::idle_box_multimap<NUM_DIMS*2, point, size_t> kdtree;
        typedef spatial::box_multimap<NUM_DIMS*2, point, size_t> kdtree_self_balancing;

        private:
            kdtree *tree;
            kdtree_self_balancing *tree_self_balancing;
            bool self_balancing = false;

        public:

            bool intersections_exact() { return true; } //intesection/overlapping region search

            Bboxes() {}
            ~Bboxes() {
                delete tree;
                delete tree_self_balancing;
            }


            void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
                if((pts.size() % 2) !=0) {
                    std::cerr << "error. your point list size has to be even to insert bounding boxes" << std::endl;
                    return;
                }
                vector<point_w_index> pts_w_index;
                for(int i = 0; i < pts.size(); i += 2) {
                    //express box as a 2*n dimensional point with all of the lower vertex's coordinates appearing first then all of the uppoer vertex's
                    point bbox_as_pt;
                    bbox_as_pt.reserve(2*NUM_DIMS);
                    bbox_as_pt.insert(bbox_as_pt.end(), pts[i].begin(), pts[i].end());
                    bbox_as_pt.insert(bbox_as_pt.end(), pts[i+1].begin(), pts[i+1].end());
                    pts_w_index.push_back(point_w_index(bbox_as_pt, i/2));
                }

                if(self_balancing) {
                    tree_self_balancing = new kdtree_self_balancing(); 
                    tree_self_balancing->insert(pts_w_index.begin(), pts_w_index.end());
                }
                else {
                    tree = new kdtree(); 
                    tree->insert_rebalance(pts_w_index.begin(), pts_w_index.end());
                }        
            }


            //closed region -> includes points that fall on the boundary
            void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
                typedef spatial::overlap_region_iterator<kdtree> iterator;
                typedef spatial::overlap_region_iterator<kdtree_self_balancing> iterator_balanced;

                //express box as a 2*n dimensional point with all of the lower vertex's coordinates appearing first then all of the uppoer vertex's
                point flattened_bbox;
                flattened_bbox.reserve(2*NUM_DIMS);
                flattened_bbox.insert(flattened_bbox.end(), my_bbox.first.begin(),  my_bbox.first.end());
                flattened_bbox.insert(flattened_bbox.end(), my_bbox.second.begin(),  my_bbox.second.end());
                //makes sure we could intersections on the boundary only as intersections
                for(int i = 0; i < flattened_bbox.size(); i++) {
                    if(i < flattened_bbox.size()/2) {
                        flattened_bbox[i] -= DEFAULT_TOLERANCE; 
                    }
                    else {
                        flattened_bbox[i] += DEFAULT_TOLERANCE; 
                    }
                }

                if(self_balancing) {
                    for (iterator_balanced i = spatial::overlap_region_begin(*tree_self_balancing, flattened_bbox); i != spatial::overlap_region_end(*tree_self_balancing, flattened_bbox); ++i) {
                        intersections_indices.push_back((*i).second);
                    }
                }
                else {
                    for (iterator i = spatial::overlap_region_begin(*tree, flattened_bbox); i != spatial::overlap_region_end(*tree, flattened_bbox); ++i) {
                        intersections_indices.push_back((*i).second);
                    }
                }
            }  
        };

};

#endif //SPATIAL_TEST_HH
