#include "skvlr_internal.h"

#pragma once

class KVStore {
 public:
    KVStore() { /* Empty */ };
    ~KVStore() { /* Empty */ };

    virtual void db_get(const int key, int *value, int curr_cpu = -1) = 0;
    virtual void db_put(const int key, int value,  int curr_cpu = -1) = 0;
};
