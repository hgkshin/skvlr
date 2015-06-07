#include <map>
#include <mutex>
#include <string>
#include <queue>
#include <thread>
#include <sstream>
#include "semaphore.h"

#include "skvlr_internal.h"
#include "kvstore.h"

#pragma once

class Skvlr : public KVStore {
    friend class Worker;

public:
    Skvlr(const std::string &name, int num_cores);
    ~Skvlr();

    // Prevent assignment and copy constructors.
    Skvlr & operator=(const Skvlr&) = delete;
    Skvlr(const Skvlr&) = delete;

    // Synchronous if status != NULL, Asynchronous if status == NULL
    void db_get(const int key, int *value, int curr_cpu = -1);
    void db_put(const int key, int value,  int curr_cpu = -1);
    void db_sync(int curr_cpu = -1);

    void db_watch(const int key, const std::function<void(const int)> &callback, int curr_cpu = -1);
    // Public only for testing purposes
    std::vector<std::thread> workers;

 private:
    const std::string name;
    const int num_workers;
    const int num_cores;

    bool should_stop __attribute__((aligned(CACHE_LINE_SIZE)));;
    struct global_state global_state __attribute__((aligned(CACHE_LINE_SIZE)));;
    update_maps *data __attribute__((aligned(CACHE_LINE_SIZE)));;

    /* Access using [worker cpu][client cpu]. */
    static void spawn_worker(worker_init_data init_data,
                             struct global_state *global_state);
};
