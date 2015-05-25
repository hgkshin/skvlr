#include <iostream>
#include "worker.h"

Worker::Worker(const int fd, const int worker_id, std::map<int, int> data)
  : fd(fd), worker_id(worker_id), data(data)
{
    for (int i = 0; i < 5; ++i) {
        std::cout << "I am worker " << worker_id << std::endl;
        sleep(1);
    }
    UNUSED(this->fd);
    UNUSED(this->worker_id);
}

Worker::~Worker()
{
    /* Empty */
}


void Worker::handle_get(Skvlr::request &req __attribute__((unused)))
{
    /* Empty */
}

void Worker::handle_put(Skvlr::request &req __attribute__((unused)))
{
    /* Empty */
}
