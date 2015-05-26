#include <unistd.h>
#include <map>

#include "skvlr.h"

#pragma once

#define UNUSED(x) (void)x;

class Worker {
public:
    Worker(const int fd, const int worker_id, std::map<int, int> data);
    Worker(const Skvlr::worker_init_data init_data);
    ~Worker();

    void listen();

private:
    void handle_get(Skvlr::request *req);
    void handle_put(Skvlr::request *req);
    int persist(const int key, const int value);

    const int fd;
    const int worker_id;
    std::map<int, int> data;
};
