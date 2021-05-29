#ifndef KDTREE4_TEST_HH
#define KDTREE4_TEST_HH

#include "kdtree.h"

using namespace std;


class TestKDTree4 : public BboxIntersectionTest {
    private:
        kdtree *tree;
        vector<void *>all_data;

    public:

        bool intersections_exact() { return false; } //circular radius isnt exact

        TestKDTree4() {}
        ~TestKDTree4() {
           for(int i = 0; i < all_data.size(); i++) {
                if(all_data[i] != NULL) {
                    free(all_data[i]);
                }
            }
            kd_free(tree);
        }

        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
            size_t num_dims = 0;
            if(pts.size() > 0) {
                num_dims = pts[0].size();
            }
            tree = kd_create(num_dims);
            for(int i = 0; i < pts.size(); i++) {
                void *data = malloc(sizeof(size_t));
                memcpy(data, &indices[i], sizeof(size_t));  
                kd_insert(tree, &pts[i][0], data);
                all_data.push_back(data);
            }


        }

        void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
            point mid_pt;
            double radius_search_bound = 0;
            get_max_radius(my_bbox, mid_pt, radius_search_bound);
            radius_search_bound+=DEFAULT_TOLERANCE;

            struct kdres *results = kd4_nearest_range(tree, &mid_pt[0], radius_search_bound);

            intersections_indices.reserve(kd_res_size(results));
            double location[3];
            while( !kd_res_end( results ) ) {
                size_t *res_index = (size_t*)kd_res_item( results, location );
                intersections_indices.push_back(*res_index);

                kd_res_next( results );
            }

            kd_res_free( results );
        }

};

#endif //KDTREE4_TEST_HH
