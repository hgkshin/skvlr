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
  std::vector<double> running_average(total_cores);
  for (size_t kv_cores = 1; kv_cores <= total_cores; kv_cores++) {
        KVStore *kv = NULL;
        if (type == SKVLR) {
          kv = new Skvlr(kv_file, kv_cores);
        } else if (type == UNSKVLR) {
          kv = new Unskvlr();
        } else if (type == EMPTYSKVLR) {
          kv = new EmptySkvlr();
        }
        /* Ideally we'd also be able to try our previous implementation here */

        KVProfiler profiler(kv, trials_to_avg, ops, kv_cores, kd);
        double ops = profiler.run_profiler();  
        running_average[kv_cores - 1] = ops;
        double scale = ops / running_average[0];
        DEBUG_PROFILER(ops << " ops/sec for core " << kv_cores << " (" << scale << "x)" << std::endl);
  }
}


int main() {
    size_t TOTAL_CORES = 8; //sysconf(_SC_NPROCESSORS_ONLN);
    size_t NUM_TRIALS = 5;
    size_t NUM_OPS = 1000000;

    clean_up_dir();

    DEBUG_PROFILER("Running Profiler on Empty Skvlr....." << std::endl);
    run_experiment(EMPTYSKVLR, PROFILER_DUMP_DIR + "profiler_empty_get_db", PARTITION_GET_HEAVY,
                   TOTAL_CORES, NUM_TRIALS, NUM_OPS);
    DEBUG_PROFILER(std::endl);
    
    DEBUG_PROFILER("Running Profiler on Skvlr (Get Heavy)....." << std::endl);
    run_experiment(SKVLR, PROFILER_DUMP_DIR + "profiler_skvlr_get_db", PARTITION_GET_HEAVY,
                   TOTAL_CORES, NUM_TRIALS, NUM_OPS);

    DEBUG_PROFILER(std::endl);
    /*DEBUG_PROFILER("Running Profiler on Skvlr (Put Heavy)....." << std::endl);
    run_experiment(SKVLR, PROFILER_DUMP_DIR + "profiler_skvlr_put_db", PARTITION_PUT_HEAVY,
                   TOTAL_CORES, NUM_TRIALS, NUM_OPS);*/

    DEBUG_PROFILER("Running Profiler on Unskvlr (Hash Map)....." << std::endl);
    run_experiment(UNSKVLR, PROFILER_DUMP_DIR + "profiler_unskvlr_db", PARTITION_GET_HEAVY,
                   TOTAL_CORES, NUM_TRIALS, NUM_OPS); 
    
    return 0;
}
