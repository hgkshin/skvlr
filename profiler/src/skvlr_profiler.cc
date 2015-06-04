#include <thread>
#include <algorithm>
#include <sys/time.h>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <sys/dir.h>
#include <random>
#include <tuple>
#include <assert.h>

#include "kvstore.h"
#include "skvlr_profiler.h"
#include "murmurhash3.h"

KVProfiler::KVProfiler(KVStore *kv_store, size_t num_trials, size_t ops_per_trial,
                       size_t total_cores, KeyDistribution kd)
  : kv_store(kv_store), num_trials(num_trials), ops_per_trial(ops_per_trial),
    total_cores(total_cores), kd(kd), SLEEP_TIME(2)
{}

KVProfiler::~KVProfiler() {}


void KVProfiler::no_op() {

}

double KVProfiler::run_profiler() {
    // Initial data structures
    double running_average = 0;

    // Run Profiler 
    for (size_t trial = 0; trial < num_trials; trial++) {
        // Several key distributions are dependent on number of cores
        // Concern: is it a problem that keys change from experiment to experiment?
        std::vector<std::vector<std::pair<int, int>>> per_client_ops(this->total_cores);
        generate_per_client_ops(per_client_ops);

        std::vector<std::thread> threads(this->total_cores);
        double start_time = get_wall_time(); 
        size_t total_num_ops = 0;
        for (size_t client_id = 0; client_id < this->total_cores; client_id++) {
          total_num_ops += per_client_ops[client_id].size();
          threads[client_id] = std::thread(&KVProfiler::run_client, this, client_id,
                                           per_client_ops[client_id]);
        }
        for (auto &thread : threads) {
          thread.join();
        }
        double end_time = get_wall_time();
        double duration = end_time - start_time;
        double total_ops_per_sec = total_num_ops / duration;
        running_average += total_ops_per_sec;
        sleep(SLEEP_TIME);
    }
    running_average /= num_trials;
    
    return running_average;
}

void KVProfiler::generate_per_client_ops(
      std::vector<std::vector<std::pair<int, int>>> &per_client_ops) {
    for (size_t client_id = 0; client_id < per_client_ops.size(); client_id++) {
        if (this->kd == PARTITION_GET_HEAVY) {
            per_client_ops[client_id] = generate_partitioned_ops(this->ops_per_trial, 0.90, 20,
                                                                 client_id);
        } else if (this->kd == PARTITION_PUT_HEAVY) {
            per_client_ops[client_id] = generate_partitioned_ops(this->ops_per_trial, 0.10, 20,
                                                                 client_id);
        } else if (this->kd == BASIC_GET_HEAVY) {
            per_client_ops[client_id] = generate_basic_ops(this->ops_per_trial, 0.90);
        } else if (this->kd == BASIC_PUT_HEAVY) {
            per_client_ops[client_id] = generate_basic_ops(this->ops_per_trial, 0.10);
        } else if (this->kd == HOT_KEYS_GET_HEAVY) {
            per_client_ops[client_id] = generate_hot_key_ops(this->ops_per_trial, 0.10);
        } else if (this->kd == HOT_KEYS_PUT_HEAVY) {
            per_client_ops[client_id] = generate_hot_key_ops(this->ops_per_trial, 0.10);
        }
    } 
}

/* Generates operations that are based on keys that hash to a client's core.
 * Has additional nice property that each client now has disjoint keys. */
std::vector<std::pair<int, int>> KVProfiler::generate_partitioned_ops(size_t num_ops,
                                                                      double get_prop,
                                                                      size_t num_distinct_keys,
                                                                      size_t client_core) {
    std::vector<std::pair<int, int>> client_ops;
    // Generates num_distinct_keys keys
    std::vector<int> keys(num_distinct_keys);
    int key = 0;
    uint32_t out;
    MurmurHash3_x86_32(&key, sizeof(int), 0, &out);
    for (size_t i = 0; i < keys.size(); i++) {
        while(out % this->total_cores != client_core) {
            key++;
            MurmurHash3_x86_32(&key, sizeof(int), 0, &out);
        }
        keys[i] = key;
    }

    size_t num_gets = num_ops * get_prop;
    size_t num_puts = num_ops * (1 - get_prop);

    for(size_t get = 0; get < num_gets; get++) {
        int rand_index = rand() % num_distinct_keys;
        std::pair<int, int> op(keys[rand_index], -1);
        client_ops.push_back(op);
    }
    
    for (size_t put = 0; put < num_puts; put++) {
        int rand_index = rand() % num_distinct_keys;
        int rand_value = rand();
        assert(rand_value >= 0);
        std::pair<int, int> op(keys[rand_index], rand_value);
        client_ops.push_back(op);
    }
    std::random_shuffle(client_ops.begin(), client_ops.end());
    return client_ops;
}

std::vector<std::pair<int, int>> KVProfiler::generate_basic_ops(size_t num_ops, double get_prop) {
  std::vector<std::pair<int, int>> client_ops;
  
  size_t num_gets = num_ops * get_prop;
  size_t num_puts = num_ops * (1 - get_prop);

  for(size_t get = 0; get < num_gets; get++) {
    std::pair<int, int> op(1, -1);
    client_ops.push_back(op);
  }
  
  for (size_t put = 0; put < num_puts; put++) {
    int rand_value = rand();
    assert(rand_value >= 0);
    std::pair<int, int> op(1, rand_value);
    client_ops.push_back(op);
  }
  std::random_shuffle(client_ops.begin(), client_ops.end());
  return client_ops;
}

std::vector<std::pair<int, int>> KVProfiler::generate_hot_key_ops(size_t num_ops,
                                                                  double get_prop) {
  std::vector<std::pair<int, int>> client_ops;
   
  size_t num_gets = num_ops * get_prop;
  size_t num_puts = num_ops * (1 - get_prop);
  
  std::default_random_engine generator;
  // Adjustable mean/std dev
  std::normal_distribution<double> distribution(0, 7);
  
  for(size_t get = 0; get < num_gets; get++) {
    std::pair<int, int> op((int) distribution(generator), -1);
    client_ops.push_back(op);
  }
  
  for (size_t put = 0; put < num_puts; put++) {
    int rand_value = rand();
    assert(rand_value >= 0);
    std::pair<int, int> op((int) distribution(generator), rand_value);
    client_ops.push_back(op);
  }
  std::random_shuffle(client_ops.begin(), client_ops.end());
  return client_ops;
}

void KVProfiler::run_client(const size_t client_core, const std::vector<std::pair<int, int>> ops) {
    /* Set up processor affinity. */
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(client_core, &cpuset);
    assert(!pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset));

    int curr_cpu = sched_getcpu();
    for (size_t i = 0; i < ops.size(); i++) {
        if (ops[i].second == -1) {
            int val;
            this->kv_store->db_get(ops[i].first, &val, curr_cpu);
        } else {
            this->kv_store->db_put(ops[i].first, ops[i].second, curr_cpu);
        }
    }
}

double KVProfiler::get_wall_time() {
    struct timeval time;
    if (gettimeofday(&time, NULL)){
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
