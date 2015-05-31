#include <thread>
#include <algorithm>
#include <sys/time.h>
#include <ctime>
#include <iostream>
#include <set>
#include <vector>
#include <sys/dir.h>
#include <sched.h>
#include <sys/stat.h>
#include <random>
#include <tuple>
#include <assert.h>

#include "skvlr.h"
#include "unskvlr.h"
#include "kvstore.h"
#include "murmurhash3.h"
#include "worker.h"

const std::string PROFILER_DUMP_DIR = "profiler/profiler_dump/";
const std::string PROFILER_BASIC = PROFILER_DUMP_DIR + "profiler_basic_db";

const int TOTAL_CORES = 8; //sysconf(_SC_NPROCESSORS_ONLN);
const int NUM_OPS = 1000000;

const static bool TEST_BASELINE = false;

// Varies proportion of gets to puts
const double GET_FRAC = 0.95;
const int NUM_GETS = GET_FRAC * NUM_OPS;
const int NUM_PUTS = (1 - GET_FRAC) * NUM_OPS;

// Varies key distribution (clustered, far apart, etc)
void generate_keys(int num_keys, double mean, double stddev, std::vector<int> &keys) {
    std::default_random_engine generator;
    // TODO (kevinshin): make various distributions
    std::normal_distribution<double> distribution(mean, stddev);
    int key;
    for (int i = 0; i < num_keys; i++) {
        key = (int) distribution(generator);
        keys.push_back(key);
    }
}

void generate_partitioned_keys(int num_keys, uint32_t ops_id, std::vector<int> &keys) {
  int key = 0;
  uint32_t out;
  MurmurHash3_x86_32(&key, sizeof(int), 0, &out);
  while(out % TOTAL_CORES != ops_id) {
    key++;
    MurmurHash3_x86_32(&key, sizeof(int), 0, &out);
  }

  for (int i = 0; i < num_keys; i++) {
    keys.push_back(key);
  }
}

// Generate operations per client thread
void generate_ops(std::vector<std::pair<int, int>> &ops, uint32_t ops_id) {
    UNUSED(ops_id);
    std::vector<int> get_keys;
    std::vector<int> put_keys;
    generate_partitioned_keys(NUM_GETS, ops_id, get_keys);
    generate_partitioned_keys(NUM_PUTS, ops_id, put_keys);
    //generate_keys(NUM_GETS, 0, 10, get_keys);
    //generate_keys(NUM_PUTS, 0, 10, put_keys);
    // TODO (kevinshin): Make get/put distinguishing cleaner
    // TODO (kevinshin): Initialize all gets with values first (call put)
    for(size_t i = 0; i < get_keys.size(); i++) {
        std::pair<int, int> pair(get_keys[i], -1);
        ops.push_back(pair);
    }
    for(size_t i = 0; i < put_keys.size(); i++) {
        int random_value = rand();
        assert(random_value >= 0);
        std::pair<int, int> pair(put_keys[i], random_value);
        ops.push_back(pair);
    }
    std::random_shuffle(ops.begin(), ops.end());
}

// Main function to apply get/put operations
void run_ops(KVStore *kv, int core_id, const std::vector<std::pair<int, int>> ops) {
    /* Set up processor affinity. */
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    assert(!pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset));


    for (size_t i = 0; i < ops.size(); i++) {
        if (ops[i].second == -1) {
            int val;
            kv->db_get(ops[i].first, &val);
        } else {
            kv->db_put(ops[i].first, ops[i].second);
        }
    }
}

double get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

int main() {
    std::string remove_command = "rm -rf ";
    remove_command += PROFILER_DUMP_DIR;
    std::string make_command = "mkdir ";
    make_command += PROFILER_DUMP_DIR;
    assert(system(remove_command.c_str()) == 0);
    assert(system(make_command.c_str()) == 0);

    std::vector<std::vector<std::pair<int, int>>> client_thread_ops;

    for (uint32_t i = 0; i < TOTAL_CORES; i++) {
        std::vector<std::pair<int, int>> ops;
        generate_ops(ops, i);
        client_thread_ops.push_back(ops);
    }
    // Runs for: 1, 2, 4 ... TOTAL_CORES
    for (int kv_cores = 1; kv_cores <= TOTAL_CORES; kv_cores++) {
        KVStore *kv = NULL;
        if (TEST_BASELINE) {
            kv = new Unskvlr();
        } else {
            kv = new Skvlr(PROFILER_BASIC, kv_cores);
        }
        std::vector<std::thread> threads(kv_cores);

        double start_time = get_wall_time();
        for (int client_thread = 0; client_thread < kv_cores; client_thread++) {
            threads[client_thread] = std::thread(run_ops, kv,
                                                 client_thread, client_thread_ops[client_thread]);
            // TODO (kevinshin): Should kv be passed by reference?
        }
        for (auto &thread : threads) {
            thread.join();
        }
        double end_time = get_wall_time();
        double duration = end_time - start_time;

        /*DEBUG_PROFILER("Core speeds for " << kv_cores << " worker cores: " << std::endl);
        DEBUG_PROFILER("Duration: " << duration << std::endl);
        DEBUG_PROFILER("Number of client threads: " << TOTAL_CORES << std::endl);
        DEBUG_PROFILER("Number of operations per client thread: " << NUM_OPS << std::endl);
        DEBUG_PROFILER("Total operations / second: " << NUM_OPS / duration << std::endl);
        */
        DEBUG_PROFILER(NUM_OPS * kv_cores/duration << std::endl);
    }
    return 0;
}
