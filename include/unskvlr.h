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

class Unskvlr : public KVStore {
public:
    Unskvlr();
    ~Unskvlr();

    // Prevent assignment and copy constructors.
    Unskvlr & operator=(const Unskvlr&) = delete;
    Unskvlr(const Unskvlr&) = delete;

    void db_get(const int key, int *value, int curr_cpu = -1);
    void db_put(const int key, int value,  int curr_cpu = -1);

 private:
    std::map<int, int> data;
    std::mutex data_lock;
};
