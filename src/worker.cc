#include <iostream>
#include <assert.h>

#include "worker.h"

// TODO: Change fd, worker_id values
Worker::Worker(const Skvlr::worker_init_data init_data)
    : worker_data(init_data), outputLog(init_data.dataFilePath(), std::ios::app)
{
    // Read the contents of the data file if any and store it in `data`.
    std::ifstream f(init_data.dataFilePath());
    int key, value;
    while (f >> key >> value) {
        data[key] = value;
    }
    f.close();
}

Worker::~Worker()
{
    outputLog.close();
}

/**
 * Listen for incoming requests by busy waiting and inspecting the queue of
 * requests for new PENDING requests to service.
 */
void Worker::listen()
{
    // check queues round-robin
    unsigned int curQueue = 0;
    while(!*this->worker_data.should_exit) {
        Skvlr::synch_queue *queue = worker_data.queues + (curQueue);
        curQueue = (curQueue + 1) % worker_data.num_queues;
        queue->queue_lock.lock();

        if (queue->queue.empty()) {
            queue->queue_lock.unlock();
            continue;
        }

        Skvlr::request *req = queue->queue.front();
        queue->queue.pop();
        queue->queue_lock.unlock();

        switch(req->type) {
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

/**
 * Handle a get request. Retreives data corresponding to req.key from memory,
 * since all data is assumed to fit in memory.  Synchronous, so this releases
 * req's semaphore before exiting.  Success/failure of the request is stored in
 * req.status.
 *
 * @param req PENDING GET request to handle
 */
void Worker::handle_get(Skvlr::request *req)
{
    // TODO: handle gets to other cores.
    assert(req->type == Skvlr::GET);
    assert(req->status == Skvlr::PENDING);

    std::cout << "handle_get: Getting key " << req->key << ", ";

    auto value = data.find(req->key);
    if (value == data.end()) {
        std::cout << "not found!" << std::endl;
        req->status = Skvlr::ERROR;
        goto release_sema;
    }

    *req->return_value = value->second;
    req->status = Skvlr::SUCCESS;
    std::cout << "value: " << *req->return_value << std::endl;

release_sema:
    req->sema.notify();
}

/**
 * Handle an incoming put request. Asynchronous, so it attempts to persist the
 * key and stores the new value in the worker's internal map if persisting
 * succeeds. Inspect req.status to determine if the persistence has completed
 * successfully or not.
 *
 * @param req PENDING PUT request to handle
 *
 * NOTE: we need to add a callback parameter to req if we wish to make the
 * status of a request visible, as per the documentation above.  Right now, it's
 * merely discarded immediately by deleting the request object.
 */
void Worker::handle_put(Skvlr::request *req)
{
    /* Empty */
    assert(req->type == Skvlr::PUT);
    assert(req->status == Skvlr::PENDING);

    std::cout << "handle_put: putting (" << req->key << ", " <<
        req->value_to_store << ")" << std::endl;

    int success = persist(req->key, req->value_to_store);
    if (success != 0) {
      req->status = Skvlr::ERROR;
      return;
    }

    data.insert(std::pair<int, int>(req->key, req->value_to_store));
    req->status = Skvlr::SUCCESS;

    delete req;
}

/**
 * Persist a key-value pair in this worker's disk.
 *
 * @return 0 on success, or a negative value on failure.
 */
int Worker::persist(const int key, const int value)
{
    outputLog << key << '\t' << value << '\n' << std::flush;

    return 0;
}
