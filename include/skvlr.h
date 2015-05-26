#include <map>
#include <mutex>
#include <string>
#include <queue>

#include "semaphore.h"

#pragma once

#define CACHE_LINE_SIZE 64

class Skvlr {
    friend class Worker;

public:
    Skvlr(const std::string &name, int num_cores);
    ~Skvlr();

    // Blocking
    int db_get(const int key);

    // Non-blocking
    void db_put(const int key, const int value);

private:
    enum RequestType { GET, PUT };
    enum RequestStatus { PENDING, SUCCESS, ERROR };

    struct request {
        int key;
        int value;
        RequestType type;
        RequestStatus status;
        Semaphore sema;
    };

    const std::string name;
    const int num_cores;

    struct synch_queue {
      std::queue<request> queue;
      std::mutex queue_lock;
      char padding[2*CACHE_LINE_SIZE - sizeof(std::queue<request>) - sizeof(std::mutex)];
    };

    synch_queue **request_matrix;

    struct worker_info {
        std::string dir_name;
        int core_id;
    };

    std::vector<pthread_t> workers;

    static void *spawn_worker(void *aux);
};
