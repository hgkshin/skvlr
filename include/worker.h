#include <unistd.h>
#include <fstream>
#include <map>

#include "skvlr.h"
#include "skvlr_internal.h"

#pragma once

class Worker {
public:
    Worker(const worker_init_data init_data, struct global_state *global_state);
    ~Worker();

    void listen();

    int persist(const std::map<int, int>& puts);

private:
    struct global_state *global_state;
    worker_init_data worker_data;
    unsigned int total_gets;
    unsigned int total_puts;
};
