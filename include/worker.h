#include <unistd.h>
#include <fstream>
#include <map>

#include "skvlr.h"

#pragma once

#define UNUSED(x) (void)x;

class Worker {
public:
    Worker(const Skvlr::worker_init_data init_data);
    ~Worker();

    void listen();

    // TODO (RR): Make these methods private but testable.
    void handle_get(Skvlr::request *req);
    void handle_put(Skvlr::request *req);
    int persist(const int key, const int value);

private:
    std::map<int, int> data;
    const Skvlr::worker_init_data worker_data;
    std::ofstream outputLog;
};
