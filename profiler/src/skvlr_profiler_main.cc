#include <vector>
#include <random>
#include <tuple>
#include <assert.h>
#include <iostream>

#include "skvlr_profiler.h"

#include "skvlr_internal.h"
#include "skvlr.h"
#include "unskvlr.h"
#include "empty_skvlr.h"
#include "kvstore.h"
#include "worker.h"

enum ExperimentType {SKVLR, UNSKVLR, EMPTYSKVLR};

void clean_up_dir() {
    // Prepare Directory
    std::string remove_command = "rm -rf " + PROFILER_DUMP_DIR;
    std::string make_command = "mkdir " + PROFILER_DUMP_DIR;
    assert(system(remove_command.c_str()) == 0);
    assert(system(make_command.c_str()) == 0);
}

void run_experiment(ExperimentType type,
                    std::string kv_file,
                    KeyDistribution kd,                    
                    size_t total_cores,
                    size_t trials_to_avg,
                    size_t ops) {
  double first_trial = 0.0;
  for (size_t kv_cores = 1; kv_cores <= total_cores; kv_cores++) {
        KVStore *kv = NULL;
        if (type == SKVLR) {
          kv = new Skvlr(kv_file, kv_cores);
        } else if (type == UNSKVLR) {
          kv = new Unskvlr();
        } else if (type == EMPTYSKVLR) {
          kv = new EmptySkvlr();
        }
        
        KVProfiler profiler(kv, trials_to_avg, ops, kv_cores, kd);
        double ops_per_sec = profiler.run_profiler();
        if (kv_cores == 1) {
          first_trial = ops_per_sec;
        } 
        double scale = ops_per_sec / first_trial;
        DEBUG_PROFILER(ops_per_sec << " ops/sec for core " << kv_cores << " (" << scale << "x)" << std::endl);
  }
}

int main() {
    size_t TOTAL_CORES = sysconf(_SC_NPROCESSORS_ONLN);
    size_t NUM_TRIALS = 3;
    // Memory limited, so we run this number of operations by MULTIPLIER in our run_client code.
    // Currently MULTIPLIER is 1 since we didn't need to enhance ops on AWS, but if memory
    // is an issue you can lower NUM_OPS and increase multiplier
    size_t NUM_OPS = 30000000;

    clean_up_dir();

    DEBUG_PROFILER("Running Profiler for 95% Gets, Normally Distributed with hot keys from " <<
                   "N(0, 5)" << std::endl); 
    DEBUG_PROFILER("Running Profiler on Skvlr....." << std::endl);
    run_experiment(SKVLR, PROFILER_DUMP_DIR + "profiler_skvlr_get_db", HOT_KEYS_GET_HEAVY,
                   TOTAL_CORES, NUM_TRIALS, NUM_OPS);
    DEBUG_PROFILER(std::endl);

    /*DEBUG_PROFILER("Running Profiler on Empty Skvlr....." << std::endl);
    run_experiment(EMPTYSKVLR, PROFILER_DUMP_DIR + "profiler_empty_get_db",HOT_KEYS_GET_HEAVY,
                   TOTAL_CORES, NUM_TRIALS, NUM_OPS);
    DEBUG_PROFILER(std::endl);

    DEBUG_PROFILER("Running Profiler on Unskvlr (Hash Map)....." << std::endl);
    run_experiment(UNSKVLR, PROFILER_DUMP_DIR + "profiler_unskvlr_db", HOT_KEYS_GET_HEAVY,
                   TOTAL_CORES, NUM_TRIALS, NUM_OPS); 
    DEBUG_PROFILER(std::endl); */

    DEBUG_PROFILER("Running Profiler for 95% Gets, 1 Key" << std::endl);
    DEBUG_PROFILER("Running Profiler on Skvlr....." << std::endl);
    run_experiment(SKVLR, PROFILER_DUMP_DIR + "profiler_skvlr_get_db", SINGLE_KEY_GET_HEAVY,
                   TOTAL_CORES, NUM_TRIALS, NUM_OPS);
    DEBUG_PROFILER(std::endl);

    DEBUG_PROFILER("Running Profiler for 95% Gets, Puts batched together with a sync call with "
                   << "N(0, 5)" << std::endl);
    DEBUG_PROFILER("Running Profiler on Skvlr....." << std::endl);
    run_experiment(SKVLR, PROFILER_DUMP_DIR + "profiler_skvlr_get_db", SYNC_GET_HEAVY,
                   TOTAL_CORES, NUM_TRIALS, NUM_OPS);
    DEBUG_PROFILER(std::endl);


    return 0;
}
