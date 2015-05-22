#include <iostream>
#include <fstream>
#include <pthread.h>
#include <sys/dir.h>
#include <sys/stat.h>

#include "skvlr.h"
#include "murmurhash3.h"

Skvlr::Skvlr(const std::string &name, int num_cores)
    : name(name), num_cores(num_cores)
{
    DIR *dir = opendir(name.c_str());
    if(!dir) {
      if(!mkdir(name.c_str(), 0 /* what mode do we want? */)) {
	/* throw some sort of error and maybe exit? At least throw an exception.*/
	exit(1);
      }
    }

    request_matrix = new std::queue<struct request>*[num_cores];
    request_matrix_locks = new std::mutex*[num_cores];
    for(int i = 0; i < num_cores; i++) {
      request_matrix[i] = new std::queue<struct request>[num_cores];
      request_matrix_locks[i] = new std::mutex[num_cores];
    }

    for(int i = 0; i < num_cores; i++) {
      spawn_and_pin_thread(name, i);
    }
    // Spawn threads, pin to cores

    closedir(dir);
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
