#ifndef ANN_TEST_HH
#define ANN_TEST_HH

#include <ANN/ANN.h>                    // ANN declarations

using namespace std;

class TestANN : public BboxIntersectionTest {
    private:
        ANNbd_tree* tree;
        ANNsplitRule split_rule = ANN_KD_SUGGEST; //author's suggestion, sliding midpoint
        ANNshrinkRule shrink_rule = ANN_BD_NONE; //kdtree
        int num_data_pts;

    public:
        bool intersections_exact() { return false; } //using a circular radius is not exact

        TestANN() {}
        ~TestANN() {
            delete tree;
        }


        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &pt_indices, size_t bucket_size) {
            num_data_pts = pts.size();
            size_t num_dims = pts[0].size();
            ANNpointArray data_pts = annAllocPts(num_data_pts, num_dims);
            for (int i = 0; i < num_data_pts; i++) { 
                for (int d = 0; d < num_dims; d++) {
                    data_pts[i][d] = pts[i][d];
                }
            }
            tree = new ANNbd_tree( data_pts, num_data_pts, num_dims, bucket_size, split_rule, shrink_rule); 
        }

        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &pt_indices) {
            build_tree(pts, pt_indices, NUM_ELEMS_PER_NODE);
        }


        void build_tree_bd(const std::vector<point> &pts, const std::vector<size_t> &pt_indices, size_t bucket_size) {
            num_data_pts = pts.size();
            size_t num_dims = pts[0].size();
            shrink_rule = ANN_BD_SUGGEST;

            ANNpointArray data_pts = annAllocPts(num_data_pts, num_dims);
            for (int i = 0; i < num_data_pts; i++) { 
                for (int d = 0; d < num_dims; d++) {
                    data_pts[i][d] = pts[i][d];
                }
            }

            tree = new ANNbd_tree( data_pts, num_data_pts, num_dims, bucket_size, split_rule, shrink_rule); 
        }

        void build_tree_bd(const std::vector<point> &pts, const std::vector<size_t> &pt_indices) {
            build_tree_bd(pts, pt_indices, NUM_ELEMS_PER_NODE);
        }

        //awkward since it only supports radius search for KNN, so we have to set K to num_data_pts, and the radius is circular
        void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {

            point mid_pt;
            double squared_radius_search_bound = 0;
            size_t num_nearest_neighbors_to_find = num_data_pts; //don't want to do KNN, want all in radius
            double epsilon = 0.0; //don't want an approximate search

            vector<int> intersections_indices_vect(num_data_pts);
            vector<double> distances(num_data_pts);
            get_max_squared_radius(my_bbox, mid_pt, squared_radius_search_bound);

            size_t num_intersected_pts = tree->annkFRSearch(&mid_pt[0], squared_radius_search_bound, num_data_pts, &intersections_indices_vect[0], &distances[0], epsilon);
            std::copy(intersections_indices_vect.begin(), intersections_indices_vect.begin()+num_intersected_pts, std::back_inserter(intersections_indices));
        }
};

#endif //ANN_TEST_HH