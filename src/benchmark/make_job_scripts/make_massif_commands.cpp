#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <algorithm>    // std::min
#include "make_job_scripts.hh"
#include <math.h> /* ceil */
#include <unistd.h> //needed for gethostname
#include <cstring> //needed for strstr

#define LOCAL_DEBUG_RUN false
#define REMOTE_DEBUG_RUN false
#define LSF_SCHEDULER true /* if the scheduler is LSF vs slurm */

using namespace std;


bool debug_logging = true;
bool extreme_debug_logging = true;
debugLog debug_log = debugLog(debug_logging);
debugLog extreme_debug_log = debugLog(extreme_debug_logging);

void create_massif_commands(const string &mesh_file_path, const string &mesh_file_name, const run_config &config, int simulated_num_procs, bool first_job, bool vortex=true);
vector<run_config> get_run_configs();


int main(int argc, char **argv) {
    bool vortex = false;
    char hostname[100];
    gethostname(hostname, sizeof(hostname));
    char *output = strstr (hostname,"vortex");
    if(output) {
        vortex = true;
    }
    bool large_runs = true;
    bool timeout = true;

    vector<uint32_t> all_num_procs;
    vector<string> mesh_file_paths;
    vector<string> mesh_file_names;
    string pfs_name;
    if(vortex) {
        pfs_name = "/pfsA";
    }
    else {
        pfs_name = "/pfsB";        
    }


    if(timeout) {
        all_num_procs = {160, 16};
         mesh_file_paths =  {
                "/path/to/meshes",
                "/path/to/meshes/with/large/amt/data/per/proc"
            };   
            //I did my decomposition using nem_split with the linear algorithm
            mesh_file_names = {
                "mesh-name-m160-l.par.160",
                "mesh-name-m16-l.par.16"
            };

    }
    else {
        if(large_runs) {
            all_num_procs = {16};
            mesh_file_paths =  {
                pfs_name + "path/to/meshes/with/large/amt/data/per/proc"
            };
            mesh_file_names = {
                "mesh-name-m16-l.par.16"
            };
        }
        else {
            if(vortex) {
                all_num_procs = {160};   
                mesh_file_names = {
                    "mesh-name-m160-l.par.160"
                };         
            }
            else {
                all_num_procs = {72};   
                mesh_file_names = {
                    "mesh-name-m72-l.par.72"
                };    
            }

            mesh_file_paths =  {
                pfs_name + "path/to/meshes"
            };

        }
    }

  

    for(int i = 0; i < all_num_procs.size(); i++) {
        cout << "num procs: " << all_num_procs[i] << ", file path: " << mesh_file_paths[i] << ", file name: " << mesh_file_names[i] << endl;
    }

/*-----------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------*/
    vector<run_config> run_configs;
    if(!timeout) {
        if(large_runs) {
            //use whichever is needed
            // run_configs = get_large_run_configs();
            run_configs = get_best_large_run_configs();
        }
        else {
            //don't need to collect memory results for jobs that have run error
            run_configs = get_configs_for_libraries_without_errors();
        }    
    }


    extreme_debug_log << "about to start generating massif commands" << endl;

    bool first_job = true;
    for(size_t i = 0; i < all_num_procs.size(); i++) {
        if(timeout && i == 0) {
            run_configs = get_timeout_small_run_configs();
        }
        else {
            run_configs = get_timeout_large_run_configs();            
        }
        for(const run_config &config : run_configs) {
            create_massif_commands(mesh_file_paths[i], mesh_file_names[i], config, all_num_procs[i], first_job, vortex);
            first_job = false;
        }
    }
}


void create_massif_commands(const string &mesh_file_path, const string &mesh_file_name, const run_config &config, int simulated_num_procs,  bool first_job, bool vortex) {
        
    int num_procs_to_test_at_a_time = 1;

    string testing_file_name = "range_query_benchmark";
    string executable_file_full_path;

    string project_folder;
    if(LOCAL_DEBUG_RUN) {
        project_folder = "/path/to/local/folder";
    }
    else if(REMOTE_DEBUG_RUN) {
        project_folder = "/path/to/project/build_dir";            
    }
    else {
        if(vortex) {
            project_folder = "/pfsA/path/to/proj/results";

        }
        else {
            project_folder = "/psfC/path/to/proj/results";
        }            
    }

    if(config.library == PCL) {
        executable_file_full_path = project_folder + "/source_pcl/" + testing_file_name;            
    }
    else {
        if(REMOTE_DEBUG_RUN) {
            executable_file_full_path = project_folder + "/src/benchmark/" + testing_file_name;
        }
        else {
            executable_file_full_path =  project_folder + "/source/" + testing_file_name;
        }
    }
    string output_path = project_folder + "/memory_results";

    string massif_combined_analysis_results_file_full_path = output_path + "/results_summary.log";
    string massif_commands_file_full_path = output_path + "/massif_commands.bash";

    ofstream file;
    if(first_job) {
        file.open(massif_commands_file_full_path);
        if(!file.is_open()) {
            cerr << "error opening massif_commands_file_full_path: " << massif_commands_file_full_path << endl;
            exit(-1);
        };

        file << "echo 'library name, library, data type, library option, num procs, memory tree (B), max memory when building tee (B)' > " << massif_combined_analysis_results_file_full_path << endl;
    }
    else {
        file.open(massif_commands_file_full_path, std::ofstream::app);
        if(!file.is_open()) {
            cerr << "error opening massif_commands_file_full_path: " << massif_commands_file_full_path << endl;
            exit(-1);
        }; 
    }

    bool use_mpi = false;
    bool valgrind = true;

    //just use 1 representative proc
    for(int rank = 1; rank < 2; rank++) {
        char cmd_line_args[512];
        snprintf(cmd_line_args, sizeof(cmd_line_args), "%s %s %hu %hu %hu %lu %d %d %d %d %d", 
                mesh_file_path.c_str(), mesh_file_name.c_str(), config.library, config.data_type, config.library_option, config.num_queries, 
                use_mpi, num_procs_to_test_at_a_time, simulated_num_procs, rank, valgrind
            );

        extreme_debug_log << "cmd_line_args: " << cmd_line_args << endl;

        string testing_params = get_library_name(config.library) + "_" + get_data_type_name(config.data_type) + "_" + to_string(simulated_num_procs) + 
                                "_" +  to_string(config.library_option) + "_" + to_string(rank);

        string results_file_name = "memory_results_" + testing_params + ".log";
        debug_log << "results_file_name: " << results_file_name << endl;
        string results_full_file_path = output_path + "/" + results_file_name;

        //capture heap information
        file << "valgrind --tool=massif --threshold=10.0 \\" << endl;
        if(config.data_type == POINTS) {
            file << "--ignore-fn='exodus_read_vertex_coordinates(int, std::vector<std::vector<double, std::allocator<double> >, std::allocator<std::vector<double, std::allocator<double> > > >&, std::pair<std::vector<double, std::allocator<double> >, std::vector<double, std::allocator<double> > >&, unsigned int&)' \\" << endl;
        } 
        else if(config.data_type == BBOXES) {
            file << "--ignore-fn='exodus_read_element_bboxes(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::vector<std::vector<double, std::allocator<double> >, std::allocator<std::vector<double, std::allocator<double> > > >&, std::pair<std::vector<double, std::allocator<double> >, std::vector<double, std::allocator<double> > >&, unsigned int&, std::vector<std::vector<unsigned long, std::allocator<unsigned long> >, std::allocator<std::vector<unsigned long, std::allocator<unsigned long> > > >&)' \\" << endl;
        }
        file << "--ignore-fn='std::vector<unsigned int, std::allocator<unsigned int> >::_M_default_append(unsigned long)' \\" << endl;
        file << "--ignore-fn='void std::vector<std::vector<unsigned long, std::allocator<unsigned long> >, std::allocator<std::vector<unsigned long, std::allocator<unsigned long> > > >::_M_realloc_insert<std::vector<unsigned long, std::allocator<unsigned long> > const&>(__gnu_cxx::__normal_iterator<std::vector<unsigned long, std::allocator<unsigned long> >*, std::vector<std::vector<unsigned long, std::allocator<unsigned long> >, std::allocator<std::vector<unsigned long, std::allocator<unsigned long> > > > >, std::vector<unsigned long, std::allocator<unsigned long> > const&)' \\" << endl;
        file << "--ignore-fn='void std::vector<std::vector<double, std::allocator<double> >, std::allocator<std::vector<double, std::allocator<double> > > >::_M_realloc_insert<std::vector<double, std::allocator<double> > >(__gnu_cxx::__normal_iterator<std::vector<double, std::allocator<double> >*, std::vector<std::vector<double, std::allocator<double> >, std::allocator<std::vector<double, std::allocator<double> > > > >, std::vector<double, std::allocator<double> >&&)' \\" << endl;
        file << "--ignore-fn='std::vector<double, std::allocator<double> >::_M_default_append(unsigned long)' \\" << endl;
        file << "--massif-out-file=" << results_full_file_path << " " << executable_file_full_path << " " << cmd_line_args;

        //collect tree size, the last size found is the most stable
        file << " && memory_tree=`ms_print " << results_full_file_path << " | grep 'heap allocation functions' | awk -F'[()]' '{print $2}' | tail -1 | sed 's/.$//'`";

        //collect the maximum memory required when building the tree
        file << " && max_memory_tree_build=`ms_print " << results_full_file_path << " | grep 'heap allocation functions' | awk -F'[()]' '{print $2}' | sort -rn | head -1 | sed 's/.$//'`"; 

        file << " && echo " << get_library_name(config.library) << ", " << config.library << ", " << config.data_type << ", " << config.library_option << ", ";
        file << simulated_num_procs << ", $memory_tree, $max_memory_tree_build >> " <<  massif_combined_analysis_results_file_full_path;

        file << endl;
    }
    file.close();    

}



