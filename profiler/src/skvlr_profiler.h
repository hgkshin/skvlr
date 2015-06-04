#include <vector>
#include <mutex>
#include <condition_variable>
#include "kvstore.h"

#pragma once

const std::string PROFILER_DUMP_DIR = "profiler/profiler_dump/";

enum KeyDistribution {PARTITION_GET_HEAVY, PARTITION_PUT_HEAVY,
                      BASIC_GET_HEAVY, BASIC_PUT_HEAVY,
                      HOT_KEYS_GET_HEAVY, HOT_KEYS_PUT_HEAVY};

class KVProfiler {
 public:
    KVProfiler(KVStore *kv_store, size_t num_trials, size_t ops_per_trial, size_t total_cores,
               KeyDistribution kd);
    ~KVProfiler();  
   
    double run_profiler(); 

 private:
    KVStore *kv_store;   
    size_t num_trials;
    size_t ops_per_trial;
    size_t total_cores;
    KeyDistribution kd;
    const double SLEEP_TIME;
    
    std::vector<double> start_times;
    std::vector<double> end_times;
    std::vector<double> durations;
    
    std::mutex ready_threads_m;
    std::condition_variable ready_threads_cv;
    size_t ready_threads_count; 
    double time_to_start;

    void generate_per_client_ops(std::vector<std::vector<std::pair<int, int>>> &per_client_ops);
    std::vector<std::pair<int, int>> generate_partitioned_ops(size_t num_ops,
                                                              double get_prop,
                                                              size_t num_distinct_keys,
                                                              size_t client_core);

    std::vector<std::pair<int, int>> generate_basic_ops(size_t num_ops, double get_prop);
    std::vector<std::pair<int, int>> generate_hot_key_ops(size_t num_ops, double get_prop);
    void run_client(const size_t client_core, const std::vector<std::pair<int, int>> ops); 
    double get_wall_time();
};
