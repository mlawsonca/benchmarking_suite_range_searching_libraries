#ifndef BENCHMARK_HH
#define BENCHMARK_HH

#define NUM_MESH_FOLDERS 4

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// 3d_bboxes /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_brute_force_bboxes(const std::vector<point> &pts_bbox, const std::vector<size_t> &indices_bbox, testing_config config, const std::vector<std::vector<size_t>> &element_node_ids) ;
void test_boost_bboxes(const std::vector<point> &pts_bbox, const std::vector<size_t> &indices_bbox, testing_config config, const std::vector<std::vector<size_t>> &element_node_ids);
void test_cgal_bboxes(const std::vector<point> &pts_bbox, const std::vector<size_t> &indices_bbox, testing_config config, const std::vector<std::vector<size_t>> &element_node_ids);
void test_libspatialindex_bboxes(const std::vector<point> &pts_bbox, const std::vector<size_t> &indices_bbox, testing_config config, const std::vector<std::vector<size_t>> &element_node_ids);
void test_rtree_template_bboxes(const std::vector<point> &pts_bbox, const std::vector<size_t> &indices_bbox, testing_config config, const std::vector<std::vector<size_t>> &element_node_ids);
void test_spatial_bboxes(const std::vector<point> &pts_bbox, const std::vector<size_t> &indices_bbox, testing_config config, const std::vector<std::vector<size_t>> &element_node_ids);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// 3d_faces //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_cgal_faces(const std::vector<point> &pts_triangle, const std::vector<size_t> &indices_triangle, testing_config config);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// 3d_points /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void test_brute_force_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_3dtk_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_alglib_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);    
void test_ann_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_boost_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_cgal_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_flann_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_kdtree_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_kdtree2_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_kdtree3_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_kdtree4_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_libkdtree_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_libkdtree2_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_libnabo_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_libspatialindex_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_nanoflann_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_octree_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_pcl_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_pico_tree_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_rtree_template_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);
void test_spatial_points(const std::vector<point> &pts, const std::vector<size_t> &indices, testing_config config);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// data_and_query_generation /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "data_and_query_generation.hh"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// perform_queries ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "perform_queries.hh"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
string get_full_path_for_mesh(const string &mesh_file_path, const string &mesh_file_name, int mpi_rank, int data_decomposition_size) {
    char mpi_rank_padded[31];
    if(data_decomposition_size < 10) {
        sprintf( mpi_rank_padded, "%d", (mpi_rank%data_decomposition_size) );
    }
    else if(data_decomposition_size < 100)
        sprintf( mpi_rank_padded, "%02d", (mpi_rank%data_decomposition_size) );
    else if(data_decomposition_size < 1000) {
        sprintf( mpi_rank_padded, "%03d", (mpi_rank%data_decomposition_size) );
    }
    else if(data_decomposition_size < 10000) {
        sprintf( mpi_rank_padded, "%04d", (mpi_rank%data_decomposition_size) );
    }
    else {
        std::cerr << "error. maxed out num procs for get_full_path_for_mesh" << std::endl;
        exit(-1);
    }

    int mesh_folder = (mpi_rank % NUM_MESH_FOLDERS) + 1;
    string mesh_path = mesh_file_path + "_" + std::to_string(mesh_folder) + "/" + mesh_file_name + "." + mpi_rank_padded;
    return mesh_path;
}

void perform_test(const std::vector<point> &mesh_coordinates, const std::vector<size_t> &indices, const testing_config &config,
    const std::vector<std::vector<size_t>> &element_node_ids);

#endif //BENCHMARK_HH