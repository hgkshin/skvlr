#include <assert.h>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sched.h>
#include <map>
#include <thread>
#include <vector>
#include <stdexcept>

#include "skvlr.h"
#include "murmurhash3.h"
#include "worker.h"

Skvlr::Skvlr(const std::string &name, int num_workers)
  : workers(num_workers), name(name), num_workers(num_workers),
    num_cores(sysconf(_SC_NPROCESSORS_ONLN)), should_stop(false),
    global_state(name)
{
    assert(num_workers <= num_cores);

    data = new update_maps[num_cores];
    for (int i = 0; i < num_cores; ++i) {
        pthread_spin_init(&data[i].puts_lock, 0);
        data[i].local_state = new std::map<int, int>();
    }
    for(int i = 0; i < num_workers; i++) {
        worker_init_data init_data(name, i, &data[i], num_cores, &should_stop);
        workers[i] = std::thread(&spawn_worker, init_data, &global_state);
    }
}

Skvlr::~Skvlr()
{
    this->should_stop = true;
    for(auto& worker : workers) {
        worker.join();
    }
}

/**
 * Get data from the key-value store synchronously.
 * All data is stored in memory.
 * TODO: details about performance characteristics of this?
 * @param key Key to search for
 * @param value Pointer to where value should be stored.
 */
void Skvlr::db_get(const int key, int *value, int curr_cpu)
{
    if (curr_cpu == -1) curr_cpu = sched_getcpu();
    if(curr_cpu < 0) {
        DEBUG_SKVLR("Current CPU < 0 on db_get\n");
        return;
    }

    try {
        *value = this->data[curr_cpu % num_workers].local_state->at(key);
    } catch (const std::out_of_range& err) {
        *value = 0;
    }
}

/**
 * Put data into the key-value store asynchronously or synchronously
 * depending on whether client passes in status.
 * @param key Key to insert data into.
 * @param value Value to insert
 * @curr_cpu optional The CPU the caller is running on.
 */
void Skvlr::db_put(const int key, int value, int curr_cpu)
{
    if (curr_cpu == -1) curr_cpu = sched_getcpu();
    if (curr_cpu < 0) {
        DEBUG_SKVLR("Current CPU < 0 on db_put\n");
        return;
    }

    pthread_spin_lock(&this->data[curr_cpu % num_workers].puts_lock);
    this->data[curr_cpu % num_workers].local_puts[key]  = value;
    pthread_spin_unlock(&this->data[curr_cpu % num_workers].puts_lock);
}

void Skvlr::db_sync()
{
    /* Empty */
}

void Skvlr::spawn_worker(worker_init_data init_data, struct global_state *global_state) {
    /* Set up processor affinity. */
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(init_data.core_id, &cpuset);
    assert(!pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset));

    /* Initialize worker. */
    Worker worker(init_data, global_state);

    /* Now worker loops infinitely. */
    worker.listen();
}
