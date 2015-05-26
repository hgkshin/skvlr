#include <map>
#include <mutex>
#include <string>
#include <queue>
#include <thread>

#include "semaphore.h"

#pragma once

#define CACHE_LINE_SIZE 64

class Skvlr {
    friend class Worker;

public:
    Skvlr(const std::string &name, int num_cores);
    ~Skvlr();

    // Prevent assignment and copy constructors.
    Skvlr & operator=(const Skvlr&) = delete;
    Skvlr(const Skvlr&) = delete;

    // Blocking
    int db_get(const int key, int *value);

    // Non-blocking
    void db_put(const int key, const int value);

private:
    enum RequestType { GET, PUT };
    enum RequestStatus { PENDING, SUCCESS, ERROR };

    /**
     * For PUT requests, value points to the value to store.
     * For GET requests, value points to where to store the retrieved value.
     */
    struct request {
        int key;
        int *value;
        RequestType type;
        RequestStatus status;
        Semaphore sema;
    };

    const std::string name;
    const int num_workers;
    const int num_cores;

    struct synch_queue {
      std::queue<request*> queue;
      std::mutex queue_lock;
      char padding[2*CACHE_LINE_SIZE - sizeof(std::queue<request>) - sizeof(std::mutex)];
    };

    /* Access using [worker cpu][client cpu]. */
    synch_queue **request_matrix;

    struct worker_init_data {
        std::string dir_name;
        int core_id;
        synch_queue *queues;
        int num_queues;
    };

    std::vector<std::thread> workers;

    static void spawn_worker(worker_init_data init_data);
};
