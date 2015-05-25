#include <unistd.h>
#include <map>

#include "skvlr.h"

#pragma once

class Worker {
<<<<<<< HEAD

public:
=======
 public:
>>>>>>> 23d0843c5cc32ad6088c5bbab7e9ef13aea075c4
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
