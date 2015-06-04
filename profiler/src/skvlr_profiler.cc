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
    total_cores(total_cores), kd(kd), SLEEP_TIME(0)
{
}

KVProfiler::~KVProfiler() {}

double KVProfiler::run_profiler() {
    // Actual Scalability Data
    double throughput_trials_sum = 0;
   
    // Statistics to help explain 
    double avg_duration_sum = 0;  
    double max_duration_sum = 0;
    double min_duration_sum = 0;
    double real_duration_sum = 0;
    double start_offset_sum = 0;

    // Run Profiler 
    std::vector<std::vector<std::pair<int, int>>> per_client_ops(this->total_cores);
    generate_per_client_ops(per_client_ops);
    this->start_times.resize(total_cores); 
    this->end_times.resize(total_cores);
    this->durations.resize(total_cores);

    for (size_t trial = 0; trial < num_trials; trial++) {
        // Several key distributions are dependent on number of cores
        // Concern: is it a problem that keys change from experiment to experiment?
        std::vector<std::thread> threads(this->total_cores);
        size_t total_num_ops = 0;
        
        for (size_t client_id = 0; client_id < this->total_cores; client_id++) {
          total_num_ops += per_client_ops[client_id].size();
          threads[client_id] = std::thread(&KVProfiler::run_client, this, client_id,
                                           per_client_ops[client_id]);
        }
        
        for (auto &thread : threads) {
          thread.join();
        }
       
        // Calculate avg thread duration 
        double avg_duration = 0;
        for (size_t i = 0; i < this->total_cores; i++) {
          avg_duration += durations[i];
        }
        avg_duration /= this->total_cores; 
        avg_duration_sum += avg_duration;
        
        // Calculate minimum/max thread duration
        double min_duration = *min_element(durations.begin(), durations.end());
        double max_duration = *max_element(durations.begin(), durations.end());
        min_duration_sum += min_duration;
        max_duration_sum += max_duration;

        // Calculate real thread duration
        double min_start_time = *min_element(start_times.begin(), start_times.end());
        double max_end_time = *max_element(end_times.begin(), end_times.end());
        double real_duration = max_end_time - min_start_time; 
        real_duration_sum += real_duration;
        
        // Calculate start offset from max and min start
        start_offset_sum += *max_element(start_times.begin(), start_times.end()) - min_start_time;
        
        // Real Value
        // Calculate throughput with either real or average
        //throughput_trials_avg += (total_num_ops / average_thread_duration);
        throughput_trials_sum += (total_num_ops / real_duration);
        


        //sleep(SLEEP_TIME);
    }
    DEBUG_PROFILER("Average thread duration across trials: " <<
                    avg_duration_sum/num_trials << std::endl); 
    DEBUG_PROFILER("Real thread duration across trials: " <<
                    real_duration_sum/num_trials << std::endl);
    DEBUG_PROFILER("Min thread duration across trials: " <<
                    min_duration_sum/num_trials << std::endl);
    DEBUG_PROFILER("Max thread duration across trials: " <<
                    max_duration_sum/num_trials << std::endl);
    DEBUG_PROFILER("Start time offset across trials: " <<
                    start_offset_sum/num_trials << std::endl);

    return throughput_trials_sum / num_trials;
}

void KVProfiler::generate_per_client_ops(
      std::vector<std::vector<std::pair<int, int>>> &per_client_ops) {
    for (size_t client_id = 0; client_id < per_client_ops.size(); client_id++) {
        if (this->kd == PARTITION_GET_HEAVY) {
            per_client_ops[client_id] = generate_partitioned_ops(this->ops_per_trial, 0.95, 20,
                                                                 client_id);
        } else if (this->kd == PARTITION_PUT_HEAVY) {
            per_client_ops[client_id] = generate_partitioned_ops(this->ops_per_trial, 0.10, 20,
                                                                 client_id);
        } else if (this->kd == BASIC_GET_HEAVY) {
            per_client_ops[client_id] = generate_basic_ops(this->ops_per_trial, 0.95);
        } else if (this->kd == BASIC_PUT_HEAVY) {
            per_client_ops[client_id] = generate_basic_ops(this->ops_per_trial, 0.10);
        } else if (this->kd == HOT_KEYS_GET_HEAVY) {
            per_client_ops[client_id] = generate_hot_key_ops(this->ops_per_trial, 0.95);
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
    
    double start_time = get_wall_time();
    for (size_t i = 0; i < ops.size(); i++) {
        if (ops[i].second == -1) {
            int val;
            this->kv_store->db_get(ops[i].first, &val, curr_cpu);
        } else {
            this->kv_store->db_put(ops[i].first, ops[i].second, curr_cpu);
        }
    }
    double end_time = get_wall_time();
    
    // False sharing? Does it matter?
    this->start_times[client_core] = start_time;
    this->end_times[client_core] = end_time;
    this->durations[client_core] = end_time - start_time;
}

double KVProfiler::get_wall_time() {
    struct timeval time;
    if (gettimeofday(&time, NULL)){
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
