#ifndef FLANN_TEST_HH
#define FLANN_TEST_HH

#include <flann/flann.h>

using namespace std;

namespace TestFLANN {
    class KDTree : public BboxIntersectionTest {
        private:

            flann::KDTreeSingleIndex<flann::L2_Simple<double>> *tree;
            //produces an error if the data is not kept accessible
            double *flattened;

            void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, flann::IndexParams build_params) {
                int num_rows = pts.size();
                if(num_rows == 0) {
                    return;
                }
                int num_cols = pts[0].size();
                flattened = (double*)malloc(sizeof(double)*num_rows*num_cols);
                for (int i = 0; i < num_rows; i++ ) {
                    for (int j = 0; j < num_cols; j++ ) {
                        flattened[i*num_cols+j] = pts[i][j];
                   }
                }
  
                tree = new flann::KDTreeSingleIndex<flann::L2_Simple<double>>(flann::Matrix<double>(flattened,num_rows,num_cols), build_params);
                tree -> buildIndex();
            }

        public:
            bool intersections_exact() { return false ;} //using a circular radius is not exact


            KDTree() {}
            ~KDTree() {
                delete tree;
                free(flattened);
            }


            /*** note: FLANN supports many other index types but they aren't suitable for our problem
                //linear - don't want. is brute force
                //LSH - for matching binary features using hamming distances
                //KDTree - doesn't make sense to use more than one tree for an exact search
                //Kmeans, CompositeIndex, HierarchicalClustering, Autotuned - is designed for high dimensional data
            ***/
            void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
                bool reorder = false;
                build_tree(pts, indices, flann::KDTreeSingleIndexParams(NUM_ELEMS_PER_NODE,reorder));

            }

            void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size) {
                bool reorder = false;
                build_tree(pts, indices, flann::KDTreeSingleIndexParams(bucket_size,reorder));
            }

            void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {

                size_t num_rows = 1; //are only issuing one query
                int num_dims = my_bbox.first.size(); 

                point mid_pt;
                double squared_radius_search_bound = 0;
                get_max_squared_radius(my_bbox, mid_pt, squared_radius_search_bound);
                squared_radius_search_bound += DEFAULT_TOLERANCE; //doens't include things right on the border so we add a tolerance

                Matrix<double> query(&mid_pt[0], num_rows, num_dims);
                 
                vector<vector<double>> dists;
                vector<vector<int>> indices;

                size_t num_leaves_to_check = FLANN_CHECKS_UNLIMITED;
                bool search_for_approx_neighbors = false;
                bool sorted = false;

                size_t num_results = tree->radiusSearch( query, indices, dists, squared_radius_search_bound, flann::SearchParams(num_leaves_to_check, search_for_approx_neighbors, sorted) );
                std::copy(indices[0].begin(), indices[0].begin()+num_results, std::back_inserter(intersections_indices));
            }
    };

    #if USE_GPU
        class CUDA : public BboxIntersectionTest {
            private:
                //note - could use other distance functions
                flann::KDTreeCuda3dIndex<flann::L2_Simple<float>> *tree;

                void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, flann::IndexParams build_params) {
                    int num_rows = pts.size();
                    if(num_rows == 0) {
                        return;
                    }
                    int num_cols = pts[0].size();

                    float *flattened = new float[num_rows*num_cols];
                    for (int i = 0; i < num_rows; i++ ) {
                        for (int j = 0; j < num_cols; j++ ) {
                            flattened[i*num_cols+j] = pts[i][j];
                       }
                    } 
                    tree = new flann::KDTreeCuda3dIndex<flann::L2_Simple<float>>(flann::Matrix<float>(flattened,num_rows,num_cols), build_params);
                    tree -> buildIndex(); 

                    delete[] flattened;
                }

            public:
                bool intersections_exact() { return false ;} //using a circular radius is not exact

                CUDA() {}
                ~CUDA() {
                    delete tree;
                }


                /*** note: FLANN supports many other index types but they aren't suitable for our problem
                    //linear - don't want. is brute force
                    //LSH - for matching binary features using hamming distances
                    //KDTree - doesn't make sense to use more than one tree for an exact search
                    //Kmeans, CompositeIndex, HierarchicalClustering, Autotuned - is designed for high dimensional data and produced very inexact results in low dimensions
                    //Kmeans and Composite Index add too many points, Hierarchical clustering misses them
                ***/



                void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices) {
                    build_tree(pts, indices, flann::KDTreeCuda3dIndexParams(NUM_ELEMS_PER_NODE));
                }

                void build_tree(const std::vector<point> &pts, const std::vector<size_t> &indices, size_t bucket_size) {
                    build_tree(pts, indices, flann::KDTreeCuda3dIndexParams(bucket_size));
                }

                void get_intersections(const bbox &my_bbox, std::vector<size_t> &intersections_indices) {

                    size_t num_rows = 1; //are only issuing one query
                    int num_dims = my_bbox.first.size(); 

                    point_f mid_pt;
                    float squared_radius_search_bound = 0;
                    get_max_squared_radius(my_bbox, mid_pt, squared_radius_search_bound);

                    squared_radius_search_bound += DEFAULT_TOLERANCE; //doens't include things right on the border so we add a tolerance

                    Matrix<float> query(&mid_pt[0], num_rows, num_dims);
                    
                    //has to be ints and floats
                    vector<vector<int>> indices;
                    vector<vector<float>> dists;

                    size_t num_leaves_to_check = FLANN_CHECKS_UNLIMITED;
                    bool search_for_approx_neighbors = false;
                    bool sorted = false;

                    size_t num_results = tree->radiusSearch( query, indices, dists, squared_radius_search_bound, flann::SearchParams(num_leaves_to_check, search_for_approx_neighbors, sorted) );
                    std::copy(indices[0].begin(), indices[0].begin() + num_results,  std::back_inserter(intersections_indices));
                }
        };
    #endif
}

#endif //FLANN_TEST_HH

