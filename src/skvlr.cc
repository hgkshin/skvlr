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

#include "skvlr.h"
#include "murmurhash3.h"
#include "worker.h"

Skvlr::Skvlr(const std::string &name, int num_workers)
  : name(name), num_workers(num_workers), num_cores(sysconf(_SC_NPROCESSORS_ONLN)),
    workers(num_workers)
{
    assert(num_workers <= num_cores);
    std::cout << "Commencing initialization of " << name << "." << std::endl;

    DIR *dir = opendir(name.c_str());
    if(!dir) {
        std::cout << "Directory " << name << " doesn't exist, so we create it."
		  << std::endl;
        assert(mkdir(name.c_str(), 777));
    }
    closedir(dir);

    std::cout << "Initializing queue matrix." << std::endl;
    request_matrix = new synch_queue*[num_cores];
    for(int i = 0; i < num_cores; i++) {
        request_matrix[i] = new synch_queue[num_cores];
    }

    std::cout << "Spawning workers." << std::endl;
    for(int i = 0; i < num_workers; i++) {
        worker_init_data init_data = {name, i, request_matrix[i], num_cores};
        workers[i] = std::thread(&spawn_worker, init_data);
    }
}

Skvlr::~Skvlr()
{
    for(auto& worker : workers) {
        worker.join();
    }

    for(int i = 0; i < num_workers; i++) {
        delete[] request_matrix[i];
    }
    delete[] request_matrix;
}

/**
 * Get data from the key-value store synchronously. All data is stored in
 * memory.
 * TODO: details about performance characteristics of this?
 * @param key Key to search for
 * @param value Pointer to where value should be stored.
 * @return 0 on success, negative number on failure.
 */
int Skvlr::db_get(const int key, int *value)
{
    std::cout << "db_get: " << key << std::endl;
    uint32_t out;

    MurmurHash3_x86_32(&key, sizeof(int), 0, &out);

    int curr_cpu = sched_getcpu();
    if(curr_cpu < 0)
        return -1;

    request req;
    req.key = key;
    req.value = value;
    req.type = GET;
    req.status = PENDING;

    synch_queue &synch_queue = request_matrix[out % num_workers][curr_cpu];
    synch_queue.queue_lock.lock();
    synch_queue.queue.push(&req);
    synch_queue.queue_lock.unlock();

    req.sema.wait();
    std::cout << "db_get " << key << " returned ";
    if(req.status == SUCCESS) {
        std::cout << *value << std::endl;
	return 0;
    } else {
        std::cout << "error" << std::endl;
	return -1;
    }
}

/**
 * Put data into the key-value store asynchronously.
 * @param key Key to insert data into.
 * @param value Value to insert
 */
void Skvlr::db_put(const int key, int value)
{
    std::cout << "db_put: " << key << ": " << value << std::endl;

    uint32_t out;
    MurmurHash3_x86_32(&key, sizeof(int), 0, &out);

    int curr_cpu = sched_getcpu();
    if(curr_cpu < 0)
        return;
    
    request *req = new request;
    req->key = key;
    req->value = &value;
    req->type = PUT;
    req->status = PENDING;
    
    /* Note: worker is responsible for freeing req's memory. */
    synch_queue &synch_queue = request_matrix[out % num_workers][curr_cpu];
    synch_queue.queue_lock.lock();
    synch_queue.queue.push(req);
    synch_queue.queue_lock.unlock();

    /* TODO: how do we tell the user if there are errors? */
}

void Skvlr::spawn_worker(worker_init_data init_data) {
    /* Set up processor affinity. */
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(init_data.core_id, &cpuset);
    assert(!pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset));

    /* Initialize worker. */
    Worker worker(init_data);

    /* Now worker loops infinitely. TODO: need a way for worker to exit. */
    worker.listen();
}
