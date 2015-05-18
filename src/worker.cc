#include "worker.h"


Worker::Worker(const int fd, const int worker_id, std::map<int, int> data)
  : fd(fd), worker_id(worker_id), data(data)
{
    /* Empty */
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
