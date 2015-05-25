#include <unistd.h>
#include <map>

#include "skvlr.h"

#pragma once

#define UNUSED(x) (void)x;

class Worker {
 public:
    Worker(const int fd, const int worker_id, std::map<int, int> data);
    ~Worker();

 private:

    void handle_get(Skvlr::request &req);
    void handle_put(Skvlr::request &req);

    const int fd;
    const int worker_id;
    std::map<int, int> data;
};
