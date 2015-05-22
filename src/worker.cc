#include<assert.h>

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

void Worker::listen()
{
    while(true) {
        /* TODO: If data exists handle it */
        if (false) {
            Skvlr::request req; // TODO: pull from queue
            switch(req.type) {
                case Skvlr::GET:
                    handle_get(req);
                    req.sema.notify();
                    break;
                case Skvlr::PUT:
                    handle_put(req);
                    break;
                default: assert(false); // sanity check
            }
        }
    }
}

/**
 * @param req GET request to handle
 */
void Worker::handle_get(Skvlr::request &req)
{
    // TODO: handle gets to other cores.
    assert(req.type == Skvlr::GET);
    assert(req.status == Skvlr::PENDING);

    auto value = data.find(req.key);
    if (value == data.end()) {
        req.status = Skvlr::ERROR;
        return;
    }

    req.value = value->second;
    req.status = Skvlr::SUCCESS;
}

void Worker::handle_put(Skvlr::request &req)
{
    /* Empty */
    // TODO: implement me
    assert(req.type == Skvlr::PUT);
    assert(req.status == Skvlr::PENDING);

    req.status = Skvlr::ERROR;
}
