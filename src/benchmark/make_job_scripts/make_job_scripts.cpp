#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <algorithm>    // std::min
#include "make_job_scripts.hh"
#include <math.h> /* ceil */

#define LOCAL_DEBUG_RUN false
#define REMOTE_DEBUG_RUN false
#define LSF_SCHEDULER true /* if the scheduler is LSF vs slurm */

using namespace std;


bool debug_logging = true;
bool extreme_debug_logging = true;
debugLog debug_log = debugLog(debug_logging);
debugLog extreme_debug_log = debugLog(extreme_debug_logging);


void create_txn_files(const vector<uint32_t> &clusters_procs_per_node, const vector<vector<string>> &cluster_names_for_procs_per_node);
void generate_txn_file(bool repeat_jobs, string cluster, uint32_t job_num, uint32_t iteration, uint32_t time, 
                        uint32_t num_nodes, uint32_t num_procs, uint32_t num_procs_per_node,
                        const string &mesh_file_path, const string &mesh_file_name, uint32_t num_completed_iterations, uint32_t num_exec_copies, const run_config &config
                        , uint32_t data_decomposition_size
                        );
vector<run_config> get_run_configs();


int main(int argc, char **argv) {

    uint32_t num_procs_per_node_vortex = 160; //technically 176 but they recommend 160 to leave 2 procs per socket (4/node *4 threads) available for managing system resources
    vector<uint32_t> clusters_procs_per_node = {num_procs_per_node_vortex};
    vector<vector<string>> cluster_names_for_procs_per_node = {{"vortex"}};

    create_txn_files(clusters_procs_per_node, cluster_names_for_procs_per_node);

}


void create_txn_files(const vector<uint32_t> &clusters_procs_per_node, const vector<vector<string>> &cluster_names_for_procs_per_node)
{
    bool large_runs = false;
    bool scaling = true;

    bool repeat_jobs = false;
    int num_completed_iterations = 0;
    uint32_t num_exec_copies = 1;

    vector<uint32_t> all_num_nodes;
    vector<vector<uint32_t>> all_num_procs;
    //specify for both points and bboxes
    vector<vector<uint32_t>> all_num_procs_per_node;
    vector<uint32_t> all_data_decomposition_sizes;
    vector<uint32_t> times;
    vector<string> mesh_file_paths;
    vector<string> mesh_file_names;

    const uint32_t LARGE_DATA_DECOMPOSITION_SIZE = 16;


    if(scaling) {
       // testing procs per node
        times =  {7200, 7200, 7200, 7200, 7200, 7200, 7200}; 
        all_num_nodes = {8, 4, 2, 1, 1, 1, 1};
        all_num_procs_per_node = { {20, 20}, {40, 40}, {80, 80}, {32, 32}, {48, 48}, {64, 64}, {80, 80}};
        all_data_decomposition_sizes = {160, 160, 160, 16, 16, 16, 16};
         mesh_file_paths =  {
                "/path/to/meshes",
                "/path/to/meshes",
                "/path/to/meshes",
                "/path/to/meshes/with/large/amt/data/per/proc",
                "/path/to/meshes/with/large/amt/data/per/proc",
                "/path/to/meshes/with/large/amt/data/per/proc",
                "/path/to/meshes/with/large/amt/data/per/proc",
            };   
            mesh_file_names = {
                "mesh-name-m160-l.par.160",
                "mesh-name-m160-l.par.160",
                "mesh-name-m160-l.par.160",
                "mesh-name-m16-l.par.16",
                "mesh-name-m16-l.par.16",
                "mesh-name-m16-l.par.16",
                "mesh-name-m16-l.par.16",
            };         
    }
    else {
        if(large_runs) {
            all_num_procs = {{16, 16}};
            all_num_nodes = {1};
            mesh_file_paths =  {
                "/path/to/meshes/with/large/amt/data/per/proc"
            };
            mesh_file_names = {
                "mesh-name-m16-l.par.16"
            };
        }
        else {
            all_num_procs = {{160, 160}};
            all_num_nodes = {1};
            times =  {3600};
            mesh_file_paths =  {
                "/path/to/meshes"
            };
            mesh_file_names = {
                "mesh-name-m160-l.par.160"
            };
        }        
    }


    cout << "writing to: " << endl;
    for(int i = 0; i < cluster_names_for_procs_per_node.size(); i++) {
        for(int j = 0; j < cluster_names_for_procs_per_node[i].size(); j++) {
            cout << cluster_names_for_procs_per_node[i][j] << endl;
        }
    }
    cout << endl;

    uint32_t num_iterations = 3;

/*-----------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------------*/
    vector<run_config> run_configs;
    if(!scaling) {
        if(large_runs) {
            run_configs = get_best_large_run_configs();
        }
        else {
            run_configs = get_run_configs();
        }        
    }
    
    if (all_num_nodes.size() > 0) {
        if(all_num_procs.size() > 0) {
            if(all_num_procs_per_node.size() == 0) {
                for (size_t i = 0; i < all_num_procs.size(); i++) {
                    vector<uint32_t> num_procs_per_node; 
                    for(size_t j = 0; j < all_num_procs[i].size(); j++) {
                        num_procs_per_node.push_back(all_num_procs[i][j] / all_num_nodes[i]);
                    }
                    all_num_procs_per_node.push_back(num_procs_per_node);
                }
            }                            
        }
        else if(all_num_procs_per_node.size() > 0) {
            if(all_num_procs.size() == 0) {
                for (size_t i = 0; i < all_num_procs_per_node.size(); i++) {
                    vector<uint32_t> num_procs; 
                    for(size_t j = 0; j < all_num_procs_per_node[i].size(); j++) {
                        num_procs.push_back(all_num_procs_per_node[i][j] * all_num_nodes[i]);
                    }
                    all_num_procs.push_back(num_procs);
                }
            }
        }
        else {
            cerr << "error. must specify 2 of: all_num_nodes, all_num_procs, all_num_procs_per_node" << endl;
            exit(-1);
        }
    }
    else {
        if(all_num_procs.size() > 0 &&  all_num_procs_per_node.size() > 0) {
            if(all_num_nodes.size() == 0) {
                for (size_t i = 0; i < all_num_procs.size(); i++) {
                    for(size_t j = 0; j < all_num_procs[i].size(); j++) {
                        all_num_nodes.push_back(ceil(all_num_procs[i][j] / (float)all_num_procs_per_node[i][j]));
                    }
                }
            }
        }
        else {
            cerr << "error. must specify 2 of: all_num_nodes, all_num_procs, all_num_procs_per_node" << endl;
            exit(-1);
        }
    }


    extreme_debug_log << "about to start generating txn files" << endl;

    vector<vector<uint32_t>> job_nums(cluster_names_for_procs_per_node.size());
    for(int i = 0; i < job_nums.size(); i++) {
        //set the job_num for each cluster to 0
        uint32_t length = cluster_names_for_procs_per_node[i].size();
        std::vector<uint32_t> temp(length, 0);
        job_nums[i] = temp;
    }


    for(int i = 0; i < clusters_procs_per_node.size(); i++) {
        for(int j = 0; j < all_num_procs.size(); j++) {
            uint32_t time = times[j];
            string mesh_file_path = mesh_file_paths[j];
            string mesh_file_name = mesh_file_names[j];

            for(int k = 0; k < cluster_names_for_procs_per_node[i].size(); k++) {
                string cluster = cluster_names_for_procs_per_node[i][k];
                if(scaling) {
                    if (mesh_file_path.find("large") != std::string::npos) {
                        run_configs = get_scaling_scaling_large_run_configs(); 
                    }
                    else {
                        run_configs = get_scaling_scaling_small_run_configs();                   
                    }
                }

                for(const run_config &config : run_configs) {
           
                    uint32_t num_nodes = all_num_nodes[j];
                    uint32_t num_procs_per_node = all_num_procs_per_node[j][config.data_type];
                    uint32_t num_procs = all_num_procs[j][config.data_type];
                 
                    uint32_t data_decomposition_size;
                    if(scaling) {
                        data_decomposition_size = all_data_decomposition_sizes[j];
                    }
                    else {
                        data_decomposition_size = num_procs;
                    }

                    debug_log << "num_procs: " << num_procs << ", num_nodes: " << num_nodes << endl;

                    uint32_t requested_procs_per_node = min ( num_procs_per_node, num_procs);

                    for(uint32_t iteration=0; iteration< num_iterations; iteration++) {
                        extreme_debug_log << "iteration: " << iteration << endl;
                        generate_txn_file(repeat_jobs, cluster, job_nums[i][k], iteration, time, 
                        num_nodes, num_procs, requested_procs_per_node,
                        mesh_file_path, mesh_file_name, num_completed_iterations, num_exec_copies, config,
                        data_decomposition_size
                        );
                        job_nums[i][k]++;
                    }                        
                }

            }
            extreme_debug_log << "finished for clusters" << endl;
        }
    }
}


void generate_txn_file(bool repeat_jobs, string cluster, uint32_t job_num, uint32_t iteration, uint32_t time, 
                        uint32_t num_nodes, uint32_t num_procs, uint32_t num_procs_per_node,
                        const string &mesh_file_path, const string &mesh_file_name, uint32_t num_completed_iterations, uint32_t num_exec_copies, const run_config &config
                        , uint32_t data_decomposition_size
                        )
{   
        if(iteration == 0) {
            extreme_debug_log << "about to create txn for " << cluster << " num nodes: " << num_nodes << "\n";  
        }

        bool multiple_executables = (num_exec_copies > 1);
        uint32_t exec_count = job_num % num_exec_copies;

        char cmd_line_args[512];

        if(data_decomposition_size != num_procs) {
            bool USE_MPI = true;
            snprintf(cmd_line_args, sizeof(cmd_line_args), "%s %s %hu %hu %hu %lu %d %lu", 
                mesh_file_path.c_str(), mesh_file_name.c_str(), config.library, config.data_type, config.library_option, config.num_queries,
                USE_MPI, data_decomposition_size);
        }
        else {
            snprintf(cmd_line_args, sizeof(cmd_line_args), "%s %s %hu %hu %hu %lu", 
                mesh_file_path.c_str(), mesh_file_name.c_str(), config.library, config.data_type, config.library_option, config.num_queries );
        }

        extreme_debug_log << "cmd_line_args: " << cmd_line_args << endl;

        string account;
        string pfs_name;
        string sbatch_path;
        string project_folder;

        string testing_file_name;

        testing_file_name = "range_query_benchmark";

        string itr_to_print;
        if(repeat_jobs) {
            itr_to_print = to_string(iteration + num_completed_iterations);
        }
        else {
            itr_to_print = to_string(iteration);
        }
        
        string testing_params = get_library_name(config.library) + "_" + get_data_type_name(config.data_type) + "_" + to_string(num_nodes) + "_" + to_string(num_procs) + "_" + to_string(data_decomposition_size) + "_" +  to_string(config.library_option) + "_" + itr_to_print;
        
        if(LOCAL_DEBUG_RUN) {
            project_folder = "/path/to/local/proj/folder";
        }
        else if(REMOTE_DEBUG_RUN) {
            project_folder = "/path/to/proj/build";            
        }
        else {
            if(vortex) {
                pfs_name = "pfsA";
                project_folder = "/" + pfs_name + "/path/to/proj/results";

            }
            else {
                pfs_name = "pfsC";
                project_folder = "/" + pfs_name + "/path/to/proj/results/" + cluster;
            }            
        }

        sbatch_path = project_folder + "/job_scripts";
        string sbatch_file_name = testing_params + ".sl";
        string sbatch_file_full_path = sbatch_path + "/" + sbatch_file_name;
        debug_log << " sbatch_file_full_path: " << sbatch_file_full_path << endl;
        string results_file_name = testing_params + ".log";
        string sbatch_script_path = sbatch_path + "/sbatch_script";            

        debug_log << "results_file_name: " << results_file_name << endl;

        ofstream file;
        file.open(sbatch_file_full_path);
        if(!file.is_open()) {
            cerr << "error opening job script: " << sbatch_file_full_path << endl;
            exit(-1);
        };
        file << "#!/bin/bash" << endl << endl;

        uint32_t hrs = time / 3600; 
        uint32_t mins = (time - 3600*hrs) / 60;
        uint32_t secs = time - 3600 * hrs - 60*mins; 
        char time_str[30];


        #if LSF_SCHEDULER
            file << "#BSUB -nnodes " << num_nodes << endl;

            //means each node will have 160 processes and 16 processes used for managing the system's services (2 cores/socket * 4 HWT * 2 sockets)
            file << "#BSUB -core_isolation 2" << endl;
            file << "#BSUB -Jd " << testing_params << endl;
            sprintf(time_str, "#BSUB -W %.2d:%.2d", hrs, mins);
        #else
            file << "#SBATCH --account=MY_WCID" << endl;
            file << "#SBATCH --nodes=" << num_nodes << endl;
            sprintf(time_str, "#SBATCH --time=%.2d:%.2d:%.2d", hrs, mins, secs);
        #endif
        file << time_str << endl;
        file << endl;
        file << endl;

        if(config.library == PCL) {
            file << "SOURCE_DIR=" << project_folder << "/source_pcl" << "\n";            
        }
        else {
            file << "SOURCE_DIR=" << project_folder << "/source" << "\n";
        }
        file << "OUTPUT_DIR=" << project_folder << "/output" << "\n";    

        file << "NUM_PROCS=" << num_procs <<endl;
        file << "NUM_PROCS_PER_NODE=" << num_procs_per_node << endl;
        file << endl;

        if(multiple_executables) {
            file << "TESTING_FILE_NAME=" << testing_file_name + to_string(exec_count) << endl;
        }
        else {
            file << "TESTING_FILE_NAME=" << testing_file_name << "\n";   
        }
        file << endl;
        file << "TESTING_LOG_FILE=${OUTPUT_DIR}/" << testing_file_name << "_"<< results_file_name << endl;
        file << endl;

        #if LSF_SCHEDULER            
            file << "cat $LSB_DJOB_HOSTFILE | sort | uniq | > ${SOURCE_DIR}/hostfile$$.txt" << endl;


            file << "mpiexec --prefix /path/to/my/openmpi/installation -nooversubscribe --use-hwthread-cpus -bind-to hwthread -np ${NUM_PROCS} ";

            if(num_procs > num_procs_per_node) {
                file << "--npernode ${NUM_PROCS_PER_NODE} --hostfile ${SOURCE_DIR}/hostfile$$.txt " ;
            }
        #else 
            file << "NODES=`scontrol show hostname $SLURM_HOSTNAME | head -n " << num_nodes << " | awk -vORS=, '{ print $1 }' | sed 's/,$//'`" << endl;
            file << endl;

            file << "ALLNODES=`scontrol show hostname $SLURM_HOSTNAME`" << endl;
            file << "echo \"$ALLNODES\" " << endl;
            file << "echo \"mpiexec -nooversubscribe --host $NODES -np ${NUM_PROCS} --npernode ${NUM_PROCS_PER_NODE} ${SOURCE_DIR}/${TESTING_FILE_NAME} " << 
                  cmd_line_args << " &> ${TESTING_LOG_FILE} \" " << endl;
            file << endl;

            file << "mpiexec -nooversubscribe --host $NODES -np ${NUM_PROCS} --npernode ${NUM_PROCS_PER_NODE} ";
        #endif

        file << "${SOURCE_DIR}/${TESTING_FILE_NAME} ";
        file << cmd_line_args << " &> ${TESTING_LOG_FILE} " << endl;
        file << endl;

        #if LSF_SCHEDULER
            file << "rm -f ${SOURCE_DIR}/hostfile$$.txt" << endl;
            file << endl;
        #endif

        file.close();

        ofstream sbatch_script;
        //use \n instead of endl to make sure the output isn't buffered (could instead disable this buffering and use endl)
        sbatch_script << "#! /bin/bash \n\n";

        string sbatch_full_file_name = sbatch_path + "/" + sbatch_file_name;
        #if LSF_SCHEDULER
            if(job_num == 0) {
                sbatch_script.open(sbatch_script_path);
                if(!sbatch_script.is_open()) {
                    cerr << "error opening sbatch_script: " << sbatch_script_path << endl;
                    exit(-1);
                }   
                else {
                    cout << "writing to sbatch_script: " << sbatch_script_path << endl;
                }
                if(num_completed_iterations > 0) {
                    sbatch_script << "bsub -J \"job0repeat" << num_completed_iterations << "\" " << sbatch_full_file_name << "\n"; 
                }
                else {
                    sbatch_script << "bsub -J \"job0\" " << sbatch_full_file_name << "\n"; 
                }
            }
            else if(multiple_executables) {
                sbatch_script.open(sbatch_script_path, std::ofstream::app);    
                if(job_num < num_exec_copies) {
                    if(num_completed_iterations > 0) {
                        sbatch_script << "bsub -J \"job" << job_num << "repeat" << num_completed_iterations << "\" " << sbatch_full_file_name << "\n"; 
                    }
                    else {
                        sbatch_script << "bsub -J \"job" << job_num << "\" " << sbatch_full_file_name << "\n"; 
                    }
                }
                else {
                    if(num_completed_iterations > 0) {
                        sbatch_script << "bsub -J \"job" << job_num << "repeat" << num_completed_iterations << "\" -w \"ended(job" << (job_num-num_exec_copies) <<  "repeat" << num_completed_iterations << ")\" "  << sbatch_full_file_name << "\n"; 
                    }
                    else {
                        sbatch_script << "bsub -J \"job" << job_num << "\" -w \"ended(job" << (job_num-num_exec_copies) << ")\" "  << sbatch_full_file_name << "\n"; 
                    }
                }
            }
            else {
                sbatch_script.open(sbatch_script_path, std::ofstream::app);  
                    if(num_completed_iterations > 0) {
                        sbatch_script << "bsub -J \"job" << job_num << "repeat" << num_completed_iterations << "\" -w \"ended(job" << (job_num-1) <<  "repeat" << num_completed_iterations << ")\" "  << sbatch_full_file_name << "\n";                         
                    }
                    else {
                        sbatch_script << "bsub -J \"job" << job_num << "\" -w \"ended(job" << (job_num-1) << ")\" "  << sbatch_full_file_name << "\n";                         
                    }
            }        

        #else
            if(job_num == 0) {
                sbatch_script << "jid0=$(sbatch " << sbatch_full_file_name << " | awk '{print $4}')\n"; 
            }
            else if(multiple_executables) {
                sbatch_script.open(sbatch_script_path, std::ofstream::app);    
                if(job_num < num_exec_copies) {
                    sbatch_script << "jid"  << job_num << "=$(sbatch " << sbatch_full_file_name << " | awk '{print $4}')\n"; 
                }
                else {
                    sbatch_script << "jid"  << job_num << "=$(sbatch --dependency=afterany:$jid" << job_num-num_exec_copies << " " << sbatch_full_file_name << " | awk '{print $4}')\n"; 
                }
            }
            else {
                sbatch_script.open(sbatch_script_path, std::ofstream::app);  
                sbatch_script << "jid"  << job_num << "=$(sbatch --dependency=afterany:$jid" << job_num-1 << " " << sbatch_full_file_name << " | awk '{print $4}')\n"; 
            }
        #endif 
        sbatch_script.close();    
        exec_count += 1;
        exec_count = exec_count % num_exec_copies;

}

