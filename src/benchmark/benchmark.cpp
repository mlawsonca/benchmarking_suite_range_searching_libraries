#include <numeric>
#include "range_tree_libraries.hh"
#include "benchmark.hh"

using namespace std;

bool USE_MPI = true;
bool VALGRIND = false;

int main(int argc, char **argv) {
    if(argc < 7) {
        cerr << "Error. " << argv[0] << " expects 6 arguments: the base mesh file path (to be adjusted by the number of folders the files are spread across), the mesh file name";
        cerr << ", the tree library, the type of data to write (point, bbox, triangle)";
        cerr << ", which of the library's options to use, and the number of queries to issue" << endl;
        return -1;
    }
    string mesh_file_path = argv[1];
    string mesh_file_name = argv[2];
    Library library = (Library)stoul(argv[3],nullptr,0);
    DataType data_type = (DataType)stoul(argv[4],nullptr,0);
    unsigned short library_option = stoul(argv[5],nullptr,0);
    size_t num_queries = stoull(argv[6],nullptr,0);
    size_t num_procs_to_test = 1;
    int comm_size;
    int rank = 0;
    bool valgrind = false;


    if(argc >= 8) {
        USE_MPI = stoi(argv[7]);
    }

    if(USE_MPI) {
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
        if(argc >= 9) {
            comm_size = stoi(argv[8]);
        }
        else {
            MPI_Comm_size(MPI_COMM_WORLD, &comm_size);  
        }
    }
    else {
        if(argc >= 9) {
            num_procs_to_test = stoull(argv[8],nullptr,0);
            if(argc >= 10) {
                comm_size = stoull(argv[9],nullptr,0);
                if(argc >= 11) {
                    rank = stoi(argv[10]);
                    if(argc >= 12) {
                        VALGRIND = stoi(argv[11]);
                    }
                }
            }
            else {
                comm_size = num_procs_to_test;
            }
        }
    }

    if(DEBUG) {
        cout << "USE_MPI: " << USE_MPI << endl;
        cout << "num_procs_to_test: " << num_procs_to_test << endl;
        cout << "comm_size: " << comm_size << endl;
        cout << "rank: " << rank << endl;
        cout << "valgrind: " << valgrind << endl;
    }

    for(size_t i = 0; i < num_procs_to_test; i++) {
        if(num_procs_to_test > 1) {
            rank = i;
        }
        string my_mesh_file = get_full_path_for_mesh(mesh_file_path, mesh_file_name, rank, comm_size);

        vector<point> mesh_coordinates;
        bbox domain_bounds;
        uint32_t num_data_pts;
        std::vector<std::vector<size_t>> element_node_ids;
        if(data_type == POINTS) {
            get_data_from_exodus_file(data_type, my_mesh_file, mesh_coordinates, domain_bounds, num_data_pts);
        }
        else {
            get_data_from_exodus_file(data_type, my_mesh_file, mesh_coordinates, domain_bounds, num_data_pts, element_node_ids);
        }
        
        if(DEBUG) {
            cout << "domain lower bound: " << endl;
            print_point(domain_bounds.first);
            cout << "domain upper bound: " << endl;
            print_point(domain_bounds.second);
        }

        std::vector<size_t> indices; // vector with 100 ints.
        if(data_type == POINTS) {
            indices.resize(mesh_coordinates.size());
        }
        else if(data_type == BBOXES) {
            //only have 1 index for every box (every 2 points)
            indices.resize(mesh_coordinates.size()/2);        
        }
        std::iota (std::begin(indices), std::end(indices), 0); // Fill with 0, 1, ..., indices.size()-1

        testing_config config(domain_bounds.first, domain_bounds.second, num_data_pts, num_queries, library, data_type, library_option);
        if(rank == 0) {
            print_results_header();        
        }

        perform_test(mesh_coordinates, indices, config, element_node_ids);        
    }
    

    if(USE_MPI) {
        MPI_Barrier (MPI_COMM_WORLD);
        MPI_Finalize();        
    }
}

void perform_test(const vector<point> &mesh_coordinates, const vector<size_t> &indices, const testing_config &config, 
    const std::vector<std::vector<size_t>> &element_node_ids) 
{

    switch(config.library) {
        #ifdef TEST_ALGLIB
            case ALGLIB : {
                    if(config.data_type == POINTS) {
                        test_alglib_points(mesh_coordinates, indices, config);
                    }
                    else {
                        cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                        exit(-1);
                    }
                break;
            }
        #endif
        #ifdef TEST_ANN
            case ANN : {
                if(config.data_type == POINTS) {
                    test_ann_points(mesh_coordinates, indices, config);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif
        #ifdef TEST_BOOST
            case BOOST_RTREE : {
                if(config.data_type == POINTS) {
                    test_boost_points(mesh_coordinates, indices, config);
                }
                else if(config.data_type == BBOXES) {
                    test_boost_bboxes(mesh_coordinates, indices, config, element_node_ids);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif
            case BRUTE_FORCE : {
                if(config.data_type == POINTS) {
                    test_brute_force_points(mesh_coordinates, indices, config);
                }
                else if(config.data_type == BBOXES) {
                    test_brute_force_bboxes(mesh_coordinates, indices, config, element_node_ids);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #ifdef TEST_CGAL
            case CGAL_LIBRARY : {
                if(config.data_type == POINTS) {
                    test_cgal_points(mesh_coordinates, indices, config);
                }
                else if(config.data_type == BBOXES) {
                    test_cgal_bboxes(mesh_coordinates, indices, config, element_node_ids);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif
        #ifdef TEST_FLANN

            case FLANN : {
                if(config.data_type == POINTS) {
                    test_flann_points(mesh_coordinates, indices, config);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif

        #ifdef TEST_KDTREE
            case KDTREE : {
                if(config.data_type == POINTS) {
                    test_kdtree_points(mesh_coordinates, indices, config);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif

        #ifdef TEST_KDTREE2
            case KDTREE2 : {
                if(config.data_type == POINTS) {
                    test_kdtree2_points(mesh_coordinates, indices, config);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif

        #ifdef TEST_KDTREE3
            case KDTREE3 : {
                if(config.data_type == POINTS) {
                    test_kdtree3_points(mesh_coordinates, indices, config);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif

        #ifdef TEST_KDTREE4
            case KDTREE4 : {
                if(config.data_type == POINTS) {
                    test_kdtree4_points(mesh_coordinates, indices, config);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif

        #ifdef TEST_3DTK
            case LIB3DTK : {
                if(config.data_type == POINTS) {
                    test_3dtk_points(mesh_coordinates, indices, config);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif

        #ifdef TEST_LIBKDTREE
            case LIBKDTREE : {
                if(config.data_type == POINTS) {
                    test_libkdtree_points(mesh_coordinates, indices, config);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif

        #ifdef TEST_LIBKDTREE2
            case LIBKDTREE2 : {
                if(config.data_type == POINTS) {
                    test_libkdtree2_points(mesh_coordinates, indices, config);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif

        #ifdef TEST_LIBNABO
            case LIBNABO : {
                if(config.data_type == POINTS) {
                    test_libnabo_points(mesh_coordinates, indices, config);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif

        #ifdef TEST_LIBSPATIALINDEX
            case LIBSPATIALINDEX : {
                if(config.data_type == POINTS) {
                    test_libspatialindex_points(mesh_coordinates, indices, config);
                }
                else if(config.data_type == BBOXES) {
                    test_libspatialindex_bboxes(mesh_coordinates, indices, config, element_node_ids);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif

        #ifdef TEST_NANOFLANN
            case NANOFLANN : {
                if(config.data_type == POINTS) {
                    test_nanoflann_points(mesh_coordinates, indices, config);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif

        #ifdef TEST_OCTREE
            case OCTREE : {
                if(config.data_type == POINTS) {
                    test_octree_points(mesh_coordinates, indices, config);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif

        #ifdef TEST_PCL
            case PCL : {
                if(config.data_type == POINTS) {
                    test_pcl_points(mesh_coordinates, indices, config);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif

        #ifdef TEST_PICO_TREE
            case PICO_TREE : {
                if(config.data_type == POINTS) {
                    test_pico_tree_points(mesh_coordinates, indices, config);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif

        #ifdef TEST_RTREE_TEMPLATE
            case RTREE_TEMPLATE : {
                if(config.data_type == POINTS) {
                    test_rtree_template_points(mesh_coordinates, indices, config);
                }
                else if(config.data_type == BBOXES) {
                    test_rtree_template_bboxes(mesh_coordinates, indices, config, element_node_ids);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif

        #ifdef TEST_SPATIAL
            case SPATIAL : {
                if(config.data_type == POINTS) {
                    test_spatial_points(mesh_coordinates, indices, config);
                }   
                else if(config.data_type == BBOXES) {
                    test_spatial_bboxes(mesh_coordinates, indices, config, element_node_ids);
                }
                else {
                    cerr << "error. data_type " << config.data_type << " not defined for library " << config.library << endl;
                    exit(-1);
                }
                break;
            }
        #endif
        default : {
                cerr << "error. library " << config.library << " not defined" << endl;
                exit(-1);        
        }
    }
}