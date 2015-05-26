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

Skvlr::Skvlr(const std::string &name, int num_cores)
    : name(name), num_cores(num_cores)
{
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

    for(int i = 0; i < num_cores; i++) {
      worker_info *info = (worker_info *) malloc(sizeof(worker_info));
      info->core_id = i;
      info->dir_name = name;
      pthread_t worker_thread;
      pthread_create(&worker_thread, NULL, &spawn_worker, info);
      workers.push_back(worker_thread);
    }

    /*
    std::vector<std::thread> threads(num_cores);
    for (int i = 0; i < num_cores; ++i) {
        threads[i] =
            std::thread([](int id) {
                std::map<int, int> data;
                Worker w(0, id, data);
                }, i);
    }
    for (auto &thread : threads) {
        thread.join();
    }
    */
}

Skvlr::~Skvlr()
{
    for(int i = 0; i < num_cores; i++) {
      delete[] request_matrix[i];
    }
    delete[] request_matrix;

    for (pthread_t& worker : workers) {
      pthread_join(worker, NULL);
    }
}

int Skvlr::db_get(const int key)
{
    std::cout << "db_get: " << key << std::endl;
    uint32_t out;

    MurmurHash3_x86_32(&key, sizeof(int), 0, &out);
    
    unsigned cpu;
    if(!getcpu(&cpu, NULL, NULL)) {
      return -1;
    }
    return -1;
}

void Skvlr::db_put(const int key, const int value)
{
    std::cout << "db_put: " << key << ": " << value << std::endl;
    /* Empty */
}

void *Skvlr::spawn_worker(void *aux) {
    struct worker_info *info = (struct worker_info *) aux;

    /* Set up processor affinity. */
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(info->core_id, &cpuset);
    assert(!pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset));

    /* Initialize worker. */
    Worker worker(*info);
    free(info);

    /* Now worker loops infinitely. TODO: need a way for worker to exit. */
    worker.listen();
    return NULL;
}
