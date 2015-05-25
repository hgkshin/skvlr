#include <iostream>
#include <map>
#include <thread>
#include <vector>

#include "skvlr.h"
#include "murmurhash3.h"
#include "worker.h"

Skvlr::Skvlr(const std::string &name, int num_cores)
    : name(name), num_cores(num_cores)
{
    std::cout << "Hello" << std::endl;
    // Create KV store if no directory exists.
    //
    // Open files

    // Initialize matrix

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
}

Skvlr::~Skvlr()
{
    /* Empty */
}
int Skvlr::db_get(const int key)
{
  //std::cout << "db_get: " << key << std::endl;
  uint32_t out;

  MurmurHash3_x86_32(&key, sizeof(int), 0, &out);
  return -1;
}

void Skvlr::db_put(const int key, const int value)
{
  UNUSED(key);
  UNUSED(value);
  //std::cout << "db_put: " << key << ": " << value << std::endl;
  
  /* Empty */
}
