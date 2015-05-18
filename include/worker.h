#include <map>

#include "skvlr.h"

#pragma once

class Worker {

    Worker(const int fd, const int worker_id, std::map<int, int> data);
    ~Worker();

 private:

    void handle_get(Skvlr::request &req);
    void handle_put(Skvlr::request &req);

    const int fd;
    const int worker_id;
    std::map<int, int> data;
};
