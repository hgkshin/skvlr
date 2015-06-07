#include "skvlr_internal.h"

#pragma once

class KVStore {
 public:
    KVStore() { /* Empty */ };
    ~KVStore() { /* Empty */ };

    virtual void db_get(const int key, int *value, int curr_cpu = -1) = 0;
    virtual void db_put(const int key, int value,  int curr_cpu = -1) = 0;
    virtual void db_sync(int curr_cpu = -1) {
        UNUSED(curr_cpu);
        /* Empty */
    };
    void db_watch(const int key, const std::function<void(const int)> &callback, int curr_cpu = -1)
    {
        UNUSED(key);
        UNUSED(callback);
        UNUSED(curr_cpu);
    }
};
