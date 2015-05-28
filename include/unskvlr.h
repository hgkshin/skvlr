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

    // Synchronous if status != NULL, Asynchronous if status == NULL
    void db_get(const int key, int *value, RequestStatus *status=NULL);
    void db_put(const int key, int value, RequestStatus *status=NULL);

 private:
    std::map<int, int> data;
    std::mutex data_lock;
};
