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
  : name(name), num_workers(num_workers), num_cores(sysconf(_SC_NPROCESSORS_ONLN))
{
    assert(num_workers <= num_cores);
    /* Alternatively, can find number of cores using:
     int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    */
    DIR *dir = opendir(name.c_str());
    if(!dir) {
      if(!mkdir(name.c_str(), 0 /* what mode do we want? */)) {
	/* throw some sort of error and maybe exit? At least throw an exception.*/
	exit(1);
      }
    }
    closedir(dir);

    request_matrix = new synch_queue*[num_cores];
    for(int i = 0; i < num_cores; i++) {
      request_matrix[i] = new synch_queue[num_cores];
    }

    for(int i = 0; i < num_workers; i++) {
      worker_info *info = new worker_info;
      info->core_id = i;
      info->dir_name = name;
      info->num_queues = num_cores;
      info->queues = request_matrix[i];
      pthread_t worker_thread;
      pthread_create(&worker_thread, NULL, &spawn_worker, info);
      workers.push_back(worker_thread);
    }
}

Skvlr::~Skvlr()
{
    for(int i = 0; i < num_workers; i++) {
      delete[] request_matrix[i];
    }
    delete[] request_matrix;

    //TODO: use a shared bool to tell workers when to shut down.
    /*
    for (pthread_t& worker : workers) {
      pthread_join(worker, NULL);
    }
    */
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
    UNUSED(value); // TODO: remove

    std::cout << "db_get: " << key << std::endl;
    uint32_t out;

    MurmurHash3_x86_32(&key, sizeof(int), 0, &out);
 
    int curr_cpu = sched_getcpu();
    if(curr_cpu < 0)
      return -1;

    //synch_queue synch_queue = request_matrix[out % num_workers][curr_cpu];
    /* Construct request, enqueue request, down the semaphore. Return
       -1 or value based on response. */
    
    //TODO: safely insert new request into proper queue
    //TODO: wait until request semaphore wakes you up
    //TODO: if request fails (req.STATUS == Skvlr::ERROR) return -1; else 0

    return -1;
}

/**
 * Put data into the key-value store asynchronously.
 * @param key Key to insert data into.
 * @param value Value to insert
 */
void Skvlr::db_put(const int key, const int value)
{
    std::cout << "db_put: " << key << ": " << value << std::endl;
    /* Empty */

    // TODO: safely insert new request into proper queue, return immediately
}

void *Skvlr::spawn_worker(void *aux) {
    worker_info *info = (struct worker_info *) aux;

    /* Set up processor affinity. */
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(info->core_id, &cpuset);
    assert(!pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset));

    /* Initialize worker. */
    Worker worker(*info);
    delete info;

    /* Now worker loops infinitely. TODO: need a way for worker to exit. */
    worker.listen();
    return NULL;
}
