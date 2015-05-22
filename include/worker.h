#include <map>

#include "skvlr.h"

#pragma once

class Worker {

public:
    Worker(const int fd, const int worker_id, std::map<int, int> data);
    ~Worker();

private:
    void handle_get(Skvlr::request &req);
    void handle_put(Skvlr::request &req);
    void listen();
    int persist(const int key, const int value);

    const int fd;
    const int worker_id;
    std::map<int, int> data;
};
