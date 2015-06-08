#include "skvlr_internal.h"
#include "kvstore.h"
#include <assert.h>
#include <map>
#include <mutex>

#pragma once

class EmptySkvlr : public KVStore {
public:
    EmptySkvlr();
    ~EmptySkvlr();

    // Prevent assignment and copy constructors.
    EmptySkvlr & operator=(const EmptySkvlr&) = delete;
    EmptySkvlr(const EmptySkvlr&) = delete;

    void db_get(const int key, int *value, int curr_cpu = -1);
    void db_put(const int key, int value,  int curr_cpu = -1);

 private:
    struct aligned_map {
        std::map<int, int>map;
        std::mutex map_lock;
    }__attribute__((aligned(64)));
    aligned_map maps[8];
};
