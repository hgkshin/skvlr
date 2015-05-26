#include <map>
#include <mutex>
#include <string>
#include <queue>
#include <thread>
#include <sstream>

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
    void db_put(const int key, int value);

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

    struct synch_queue {
        std::queue<request> queue;
        std::mutex queue_lock;
        char padding[2*CACHE_LINE_SIZE - sizeof(std::queue<request>) - sizeof(std::mutex)];
    };

    struct worker_init_data {
        const std::string dir_name;
        const int core_id;
        const synch_queue *queues;
        const int num_queues;

        worker_init_data(const std::string dir_name, const int core_id,
                         const synch_queue *queues, const int num_queues)
        :  dir_name(dir_name), core_id(core_id), queues(queues), num_queues(num_queues)
        {
            /* Empty */
        }

        // Returns the name of the data file that this worker stores its data in.
        std::string dataFileName() const {
            std::stringstream ss;
            ss << this->core_id << ".data";
            return ss.str();
        }

        // Returns the path at which this worker stores its data.
        std::string dataFilePath() const {
            std::stringstream ss;
            ss << this->dir_name << "/" << dataFileName();
            return ss.str();
        }
    };

 private:
    const std::string name;
    const int num_workers;
    const int num_cores;

    /* Access using [worker cpu][client cpu]. */
    synch_queue **request_matrix;

    std::vector<std::thread> workers;

    static void spawn_worker(worker_init_data init_data);
};
