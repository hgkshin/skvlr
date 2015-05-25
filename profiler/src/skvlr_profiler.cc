#include <thread>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <set>
#include <vector>
#include <random>
#include <tuple>
#include <assert.h>

#include "skvlr.h"

const int NUM_CORES = 2;
const int NUM_OPS = 10000;

// Varies proportion of gets to puts
const double GET_FRAC = 0.9;
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

// Generate operations per client thread
void generate_ops(std::vector<std::pair<int, int>> &ops) {
  std::vector<int> get_keys;
  std::vector<int> put_keys;
  generate_keys(NUM_GETS, 0, 1, get_keys);
  generate_keys(NUM_PUTS, 0, 1, put_keys);
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
void run_ops(Skvlr *kv, const std::vector<std::pair<int, int>> ops) {
  for (size_t i = 0; i < ops.size(); i++) {
    if (ops[i].second == -1) {
      kv->db_get(ops[i].first);
    } else {
      kv->db_put(ops[i].first, ops[i].second);
    }
  } 
}

int main() {
  std::vector<std::vector<std::pair<int, int>>> client_thread_ops;
  
  for (int i = 0; i < NUM_CORES; i++) {
    std::vector<std::pair<int, int>> ops;
    generate_ops(ops);
    client_thread_ops.push_back(ops);
  }
  // Runs for: 1, 2, 4 ... NUM_CORES
  for (int kv_cores = 1; kv_cores <= NUM_CORES; kv_cores *= 2) {
    std::cout << "Core speeds for " << kv_cores << " processors: " << std::endl;
    Skvlr kv = Skvlr("profiler_db", kv_cores); 
    std::vector<std::thread> threads(NUM_CORES);
  
    std::clock_t start = std::clock();
    for (int client_thread = 0; client_thread < NUM_CORES; client_thread++) {
      threads[client_thread] = std::thread(run_ops, &kv, client_thread_ops[client_thread]);
      // TODO (kevinshin): Set processor affinity. Also, should kv be passed by reference?
    } 
    for (auto &thread : threads) {
      thread.join();
    }

    double duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
    std::cout << "Duration: " << duration << std::endl; 
    std::cout << "Number of operations: " << NUM_OPS << std::endl;
  }
  return 0;
}
