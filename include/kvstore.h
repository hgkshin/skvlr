#include "skvlr_internal.h"

#pragma once

class KVStore {
 public:
    KVStore() { /* Empty */ };
    ~KVStore() { /* Empty */ };

    // Synchronous if status != NULL, Asynchronous if status == NULL
    virtual void db_get(const int key, int *value, RequestStatus *status=NULL) = 0;
    virtual void db_put(const int key, int value, RequestStatus *status=NULL) = 0;

};
