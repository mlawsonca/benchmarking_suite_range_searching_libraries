#ifndef LIBKDTREE2_TEST_HH
#define LIBKDTREE2_TEST_HH

extern "C" {
    #include "lib/kdtree.h"
    #include "lib/pqueue.h"
}
#include <float.h>

using namespace std;

static void * data_constr(void *data)
{
    size_t *a;

    a = (size_t*)malloc(sizeof(size_t));
    memcpy(a, (size_t*) data, sizeof(size_t));
    return a;
}

static void data_destr(void *data)
{
    free(data);
}

static struct kd_point *create_point_list(const vector<point> &pts, const vector<size_t> &indices, vector<float> &min_values, vector<float> &max_values)
{
    size_t num_dims = 0;
    if(pts.size() > 0) {
        num_dims = pts[0].size();
    }

    struct kd_point *point_list = (kd_point*)malloc(pts.size() * sizeof(struct kd_point));
    if (!point_list) {
        return NULL;
    }
    size_t index = 0;
    for(size_t i = 0; i < pts.size(); i++) {
        point_list[i].point = (float *)malloc(num_dims * sizeof(float));
        if (!point_list[i].point) {
            return NULL;
        }
        point_list[i].data = (size_t *) malloc(sizeof(size_t));
        memcpy(point_list[i].data, &indices[index++], sizeof(size_t));  
        for(size_t j = 0; j < num_dims; j++) {
            point_list[i].point[j] = pts[i][j];
            min_values[j] = std::min(min_values[j], (float)pts[i][j]);
            max_values[j] = std::max(max_values[j], (float)pts[i][j]);
        }
    }
    return point_list;
}



class TestLibkdtree2 : public BboxIntersectionTest {
    private:
        struct kdNode *tree;

    public:

        bool intersections_exact() { return true; } //bounding box search

        TestLibkdtree2() {}
        ~TestLibkdtree2() {
            delete tree;
        }


        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
            size_t num_dims = 0;
            size_t num_points = pts.size();
            if(num_points > 0) {
                num_dims = pts[0].size();
            }
            std::vector<float> min_values(num_dims, FLT_MAX);
            std::vector<float> max_values(num_dims, FLT_MIN);

            size_t num_threads = 1;
            struct kd_point *point_list = create_point_list(pts, indices, min_values, max_values);
            // assert(point_list != NULL);
            tree = kd_buildTree(point_list, num_points, data_constr, data_destr, &min_values[0], &max_values[0], num_dims, num_threads);
            // assert(tree != NULL);

            for(size_t i = 0; i < num_points; i++) {
                free(point_list[i].data);
                free(point_list[i].point);
            }
            free(point_list);
        }

        void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {

            float min_corner[my_bbox.first.size()];
            float max_corner[my_bbox.second.size()];
            for(int i = 0; i < my_bbox.first.size(); i++) {
                min_corner[i] = my_bbox.first[i];
                max_corner[i] = my_bbox.second[i];
            }
            size_t num_dims = my_bbox.first.size();
            struct pqueue *result = kd_ortRangeSearch(tree, min_corner, max_corner, num_dims);

            // assert(result != NULL);
            //results start at i (if you try to access index 0 you'll segfault)
            intersections_indices.reserve(result->size-1);
            for(size_t i = 1; i < result->size; i++) {
                intersections_indices.push_back( *(size_t*)(result->d[i]->node->data));
                // free the heap element
                free(result->d[i]);
            }
            // Free the heap 
            free(result->d);
            // Free the heap information structure 
            free(result);
        }

};

#endif //LIBKDTREE2_TEST_HH
