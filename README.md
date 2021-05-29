# Benchmarking Suite for Range Tree Libraries

This project contains all of the code needed to replicate the experiments for the SC'21 paper: "Exploring Spatial Indexing for Accelerated Feature Retrieval in HPC." The code compares 20 different C/C++ spatial indexing libraries, and includes 1). correctness tests for the libraries and 2). the set of benchmarks used in the paper. These benchmarks include performance, strong and weak scaling, and memory usage tests.

## Prerequisities

The correctness tests have one core dependency: Boost (random)
The performance and scalability benchmark has 3 core dependencies: SEACASExo_format (required to read the mesh files), MPI (used for parallelization), and Boost (serialization, used for sharing timing results between MPI processes). The memory tests will also require Valgrind (we use the Massif heap profiler)

Additionally, if you actually wish to test any of the spatial indexing libraries, these will need to be installed separately. A complete list of the libraries and/or their code respositories can be found in the paper.

### Getting Started
The basic build instructions for this project are:
cd range_trees_benchmarking && mkdir build && cd build && cmake .. && make -j

Note: the spatial indexing libraries must be installed separately, and several of these libraries may have minor build issues. In addition, the there are a number of CMake variables users will have to set for some of the libraries such as the install directory of the library (e.g., ANN_DIR). CMake will tell you, using errors, if one or more of these variables are not set. For these reasons, we would highly recommend using the Docker image for this project, which comes packaged with all of these dependencies already built (without error) and which builds all of the code needed for this project (linking in all of the 20 librarie). It can be found at https://hub.docker.com/repository/docker/mlawsonca/benchmarking_suite_range_searching_libraries


#### CMake Variables

By default we build both the benchmarking code, and the correctness tests. Either of these can be disabled using:
-DBUILD_TESTS=false
-DBUILD_BENCHMARK=false

By default, testing and/or benchmarking is enabled for all of the libraries (apart from PCL, which we needed to use GCC 8.3.1 to build, rather than GCC 10.2.0 as was used for the rest of the project). Can disable any of the libraries using the following:
- -DTEST_3DTK=false
- -DTEST_ALGLIB=false
- -DTEST_ANN=false
- -DTEST_BOOST_RTREE=false
- -DTEST_CGAL=false
- -DTEST_FLANN=false
- -DTEST_KDTREE=false
- -DTEST_KDTREE2=false
- -DTEST_KDTREE3=false
- -DTEST_KDTREE4=false
- -DTEST_LIBKDTREE=false
- -DTEST_LIBKDTREE2=false
- -DTEST_LIBNABO=false
- -DTEST_LIBSPATIALINDEX=false
- -DTEST_NANOFLANN=false
- -DTEST_OCTREE=false
- -DTEST_PCL=false
- -DTEST_PICO_TREE=false
- -DTEST_RTREE_TEMPLATE=false
- -DTEST_SPATIAL=false


#### Usage
To generate the necessary job scripts, a few variables will need to be adjust in src/benchmark/make_job_scripts/make_job_scripts.cpp and src/benchmark/make_massif_commands.cpp: mesh_file_paths and mesh_file_names (indicating the filepath and name of the decomposed mesh files to use in testing), and project_folder (the folder where the executables, job scripts, and output will go for the project). For this project folder, the script expects it to contain the following subdirectories:
- source (where the range_query_benchmark exectuable(s) are copied)
- job_scripts (where the slurm/lsf job scripts should be stored)
- output (where the job output logs should be stored)

The user can also adjust LSF_SCHEDULER (a compiler directive, used to indicate if job scripts should be for an LSF scheduler or slurm scheduler)

In make_job_scripts.cpp, the user can also set the variables to determine if the job scripts are for the small scale runs (large_runs = false and scaling = false), for the large scale runs (large_runs = true) or for the strong and weak scaling tests (scaling = true). Similarly, for make_massif_commands.cpp, large_runs can be set (large_runs = false for small scale runs).

Then, the following is used to generate the job scripts:
g++ make_massif_commands.cpp -o make_massif_commands && ./make_massif_commands 0 && ./make_massif_commands 1


## Find Us

* [Docker](https://hub.docker.com/repository/docker/mlawsonca/benchmarking_suite_range_searching_libraries)

## Authors

* **Margaret Lawson**

## License

This project is licensed under the MIT License - see the [COPYING](COPYING) file for details.



