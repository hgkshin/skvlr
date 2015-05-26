
#include <iostream>
#include <assert.h>

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

// TODO: Change fd, worker_id values
Worker::Worker(const Skvlr::worker_init_data init_data)
  : fd(5), worker_id(5)
{
  UNUSED(init_data);
}

Worker::~Worker()
{
    /* Empty */
}

/**
 * Listen for incoming requests by busy waiting and inspecting the queue of
 * requests for new PENDING requests to service.
 */
void Worker::listen()
{
    while(true) {
        /* TODO: If data exists handle it */
        if (false) {
            Skvlr::request req; // TODO: pull from queue
            switch(req.type) {
                case Skvlr::GET:
                    handle_get(req);
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
 * Handle a get request. Retreives data corresponding to req.key from memory,
 * since all data is assumed to fit in memory.  Synchronous, so this releases
 * req's semaphore before exiting.  Success/failure of the request is stored in
 * req.status.
 *
 * @param req PENDING GET request to handle
 */
void Worker::handle_get(Skvlr::request &req)
{
    // TODO: handle gets to other cores.
    assert(req.type == Skvlr::GET);
    assert(req.status == Skvlr::PENDING);

    auto value = data.find(req.key);
    if (value == data.end()) {
        req.status = Skvlr::ERROR;
        goto release_sema;
    }

    *req.value = value->second;
    req.status = Skvlr::SUCCESS;

release_sema:
    req.sema.notify();
}

/**
 * Handle an incoming put request. Asynchronous, so it attempts to persist the
 * key and stores the new value in the worker's internal map if persisting
 * succeeds. Inspect req.status to determine if the persistence has completed
 * successfully or not.
 *
 * @param req PENDING PUT request to handle
 */
void Worker::handle_put(Skvlr::request &req)
{
    /* Empty */
    assert(req.type == Skvlr::PUT);
    assert(req.status == Skvlr::PENDING);

    // TODO: implement persisting the data, add to map if persistence succeeds
    int success = persist(req.key, *req.value);
    if (success != 0) {
      req.status = Skvlr::ERROR;
      return;
    }

    data.insert(std::pair<int, int>(req.key, *req.value));
    req.status = Skvlr::SUCCESS;
}

/**
 * Persist a key-value pair in this worker's disk.
 *
 * @return 0 on success, or a negative value on failure.
 */
int Worker::persist(const int key, const int value)
{
    (void) key;
    (void) value;
    // TODO: implement me!
    return -1;
}
