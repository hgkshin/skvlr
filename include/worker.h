#include <unistd.h>
#include <fstream>
#include <map>

#include "skvlr.h"
#include "skvlr_internal.h"

#pragma once

class Worker {
public:
    Worker(const worker_init_data init_data);
    ~Worker();

    void listen();

    void handle_get(request *req);
    void handle_put(request *req);
    int persist(const int key, const int value);

private:
    std::map<int, int> data;
    worker_init_data worker_data;
    std::ofstream outputLog;
    unsigned int total_gets;
    unsigned int total_puts;
};
