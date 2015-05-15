#include "worker.h"


Worker::Worker(const int fd, const int worker_id)
    : fd(fd), worker_id(worker_id)
{
    /* Empty */
}

Worker::~Worker()
{
    /* Empty */
}


void worker::handle_get(request &req)
{
    /* Empty */
}

void Worker::handle_put(request &req)
{
    /* Empty */
}
