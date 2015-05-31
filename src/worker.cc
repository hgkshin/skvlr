#include <iostream>
#include <assert.h>

#include "worker.h"

Worker::Worker(const worker_init_data init_data, std::map<int, int> *global_state)
    : global_state(global_state), worker_data(init_data),
      outputLog(init_data.dataFilePath(), std::ios::app),
      total_gets(0), total_puts(0)
{
    // Read the contents of the data file if any and store it in `data`.
    std::ifstream f(init_data.dataFilePath());
    int key, value;
    while (f >> key >> value) {
        this->worker_data.maps->local_state[key] = value;
    }
    f.close();
}

Worker::~Worker()
{
    DEBUG_WORKER("Total gets for core #" << worker_data.core_id << ": "
                 << total_gets << std::endl);
    DEBUG_WORKER("Total puts for core #" << worker_data.core_id << ": "
                 << total_puts << std::endl);
    outputLog.close();
}

/**
 * Listen for incoming requests by busy waiting and inspecting the queue of
 * requests for new PENDING requests to service.
 */
void Worker::listen()
{
    // check queues round-robin
    while(!*this->worker_data.should_exit) {
        int time = rand() % 10;
        sleep(0.5 + time / 10.0);
        std::map<int, int> empty_puts;
        pthread_spin_lock(&this->worker_data.maps->puts_lock);
        empty_puts.swap(this->worker_data.maps->local_puts);
        pthread_spin_unlock(&this->worker_data.maps->puts_lock);

        global_state->insert(empty_puts.begin(), empty_puts.end());

        this->worker_data.maps->local_state.insert(global_state->begin(), global_state->end());

        for (auto kv : empty_puts) {
            persist(kv.first, kv.second);
        }
        // synch_queue *queue = worker_data.queues + (curQueue);
        // curQueue = (curQueue + 1) % worker_data.num_queues;
        // queue->queue_lock.lock();

        // if (queue->queue.empty()) {
        //     queue->queue_lock.unlock();
        //     continue;
        // }

        // request *req = queue->queue.front();
        // queue->queue.pop();
        // queue->queue_lock.unlock();

        // switch(req->type) {
        //     case GET:
        //         handle_get(req);
        //         total_gets++;
        //         break;
        //     case PUT:
        //         handle_put(req);
        //         total_puts++;
        //         break;
        //     default: assert(false); // sanity check
        // }


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
void Worker::handle_get(request *req)
{
    UNUSED(req);
    // TODO: handle gets to other cores.
    // assert(req->type == GET);
//     assert(req->status == PENDING);

//     auto value = data.find(req->key);
//     if (value == data.end()) {
//         //DEBUG_WORKER("Get not found!" << std::endl);
//         req->status = ERROR;
//         goto release_sema;
//     }

//     *req->return_value = value->second;
//     req->status = SUCCESS;

// release_sema:
//     req->sema.notify();
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
void Worker::handle_put(request *req)
{
    UNUSED(req);
    // /* Empty */
//     assert(req->type == PUT);
//     assert(req->status == PENDING);

//     int success = persist(req->key, req->value_to_store);
//     if (success != 0) {
//       DEBUG_WORKER("handle_put failed\n");
//       req->status = ERROR;
//       goto release_sema;
//     }

//     data.insert(std::pair<int, int>(req->key, req->value_to_store));
//     req->status = SUCCESS;

// release_sema:
//     if (req->synchronous) {
//         req->sema.notify();
//     } else {
//         delete req;
//     }
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
