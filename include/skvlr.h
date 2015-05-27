#include <map>
#include <mutex>
#include <string>
#include <queue>
#include <thread>
#include <sstream>
#include "semaphore.h"

#include "skvlr_internal.h"

#pragma once

class Skvlr {
    friend class Worker;

public:
    Skvlr(const std::string &name, int num_cores);
    ~Skvlr();

    // Prevent assignment and copy constructors.
    Skvlr & operator=(const Skvlr&) = delete;
    Skvlr(const Skvlr&) = delete;

    // Synchronous if status != NULL, Asynchronous if status == NULL
    void db_get(const int key, int *value, RequestStatus *status=NULL);
    void db_put(const int key, int value, RequestStatus *status=NULL);

    // Public only for testing purposes
    std::vector<std::thread> workers;

 private:
    const std::string name;
    const int num_workers;
    const int num_cores;
    bool should_stop;
    
    /* Access using [worker cpu][client cpu]. */
    synch_queue **request_matrix;

    static void spawn_worker(worker_init_data init_data);
};
