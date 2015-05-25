#include <iostream>
<<<<<<< HEAD
#include <fstream>
#include <pthread.h>
#include <sys/dir.h>
#include <sys/stat.h>
=======
#include <map>
#include <thread>
#include <vector>
>>>>>>> 23d0843c5cc32ad6088c5bbab7e9ef13aea075c4

#include "skvlr.h"
#include "murmurhash3.h"
#include "worker.h"

Skvlr::Skvlr(const std::string &name, int num_cores)
    : name(name), num_cores(num_cores)
{
<<<<<<< HEAD
    DIR *dir = opendir(name.c_str());
    if(!dir) {
      if(!mkdir(name.c_str(), 0 /* what mode do we want? */)) {
	/* throw some sort of error and maybe exit? At least throw an exception.*/
	exit(1);
      }
    }
=======
    std::cout << "Hello" << std::endl;
    // Create KV store if no directory exists.
    //
    // Open files
>>>>>>> 23d0843c5cc32ad6088c5bbab7e9ef13aea075c4

    request_matrix = new std::queue<struct request>*[num_cores];
    request_matrix_locks = new std::mutex*[num_cores];
    for(int i = 0; i < num_cores; i++) {
      request_matrix[i] = new std::queue<struct request>[num_cores];
      request_matrix_locks[i] = new std::mutex[num_cores];
    }

<<<<<<< HEAD
    for(int i = 0; i < num_cores; i++) {
      worker_info *info = (worker_info *) malloc(sizeof(worker_info));
      info->core_id = i;
      info->dir_name = name;
      pthread_t worker_thread;
      pthread_create(&worker_thread, NULL, &spawn_worker, info);
      
    }

    closedir(dir);
=======
    std::vector<std::thread> threads(num_cores);
    for (int i = 0; i < num_cores; ++i) {
        threads[i] =
            std::thread([](int id) {
                std::map<int, int> data;
                Worker w(0, id, data);
                }, i);
    }
    sleep (10);
    for (auto &thread : threads) {
        thread.join();
    }
>>>>>>> 23d0843c5cc32ad6088c5bbab7e9ef13aea075c4
}

Skvlr::~Skvlr()
{
    for(int i = 0; i < num_cores; i++) {
      delete[] request_matrix[i];
      delete[] request_matrix_locks[i];
    }
    delete[] request_matrix;
    delete[] request_matrix_locks;
}

int Skvlr::db_get(const int key)
{
  std::cout << "db_get: " << key << std::endl;
  uint32_t out;

  MurmurHash3_x86_32(&key, sizeof(int), 0, &out);
    return -1;
}

void Skvlr::db_put(const int key, const int value)
{
  std::cout << "db_put: " << key << ": " << value << std::endl;
    /* Empty */
}

void *Skvlr::spawn_worker(void *aux) {
    return NULL;
}
