#ifndef LIBNABO_TEST_HH
#define LIBNABO_TEST_HH

#include "nabo/nabo.h"

using namespace std;

class TestLibnabo : public BboxIntersectionTest {
    private:
        Nabo::NNSearchD *tree;
        Eigen::MatrixXd *matrix;
        double tolerance = DEFAULT_TOLERANCE;
        bool is_linear_heap = true;


    public:

        bool intersections_exact() { return false; } //using circular radius is not exact

        TestLibnabo() {}

        ~TestLibnabo() {
            delete tree;
            delete matrix;
        }

        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, bool linear_heap, size_t bucket_size = NUM_ELEMS_PER_NODE) {
            //is column major
            bool keep_statistics = false;
            int num_pts = pts.size();
            int num_dims = 0;
            if(num_pts > 0) {
                num_dims = pts[0].size();
            }
            matrix = new Eigen::MatrixXd(num_dims, num_pts);
            for (int i = 0; i < pts.size(); i++) {
                //the matrix is column major
                matrix->col(i) = Eigen::VectorXd::Map(&pts[i][0],pts[i].size());
            }

            Nabo::Parameters additionalParameters("bucketSize", (unsigned)bucket_size);

            if(linear_heap) {
                tree = Nabo::NNSearchD::createKDTreeLinearHeap(*matrix, num_dims, keep_statistics, additionalParameters);
            }
            else {
                tree = Nabo::NNSearchD::createKDTreeTreeHeap(*matrix, num_dims, keep_statistics, additionalParameters);      
                is_linear_heap = false;
            }
        }

        void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
            build_tree(pts, indices, NUM_ELEMS_PER_NODE);
        }



        void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {
            // Look for the number of nearest neighbours=tree size (since theoretically every point could match)
            size_t k = matrix->cols();
            int num_queries = 1;
            double epsilon = 0.0; //don't want an approximate search
            Eigen::MatrixXi indices;
            Eigen::MatrixXd dists;
            Eigen::MatrixXd query;
            int num_dims = my_bbox.first.size();

            point mid_pt;
            double squared_radius_search_bound = 0;
            get_max_radius(my_bbox, mid_pt, squared_radius_search_bound);
            squared_radius_search_bound += tolerance;

            //is column major
            query.resize(num_dims,num_queries);
            for(int i = 0; i < mid_pt.size(); i++) {
                query(i,0) = mid_pt[i];
            }
            indices.resize(k, query.cols());
            dists.resize(k, query.cols());
            tree->knn(query, indices, dists, k, epsilon, Nabo::NNSearchD::ALLOW_SELF_MATCH , squared_radius_search_bound);
            int i = 0;
            intersections_indices.reserve(k);
            for(int i = 0; i < k; i++) {
                // means we've found fewer than the max possible number of intersections
                if(indices(i,0)  == -1) {
                // just have to make sure its not index 0 on a tree heap which will be -1 unless all points match the query
                    if((i > 0 || is_linear_heap)) {
                        return;
                    }
                }
                else {
                    intersections_indices.push_back(indices(i,0));
                }
            }
         
        }
};

#endif //LIBNABO_TEST_HH

