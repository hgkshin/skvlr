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

    struct request {
        int key;
        union {
            int value_to_store; // PUT requests only
            int *return_value;  // GET requests only
        };
        RequestType type;
        RequestStatus status;
        Semaphore sema;
    };

    struct synch_queue {
        std::queue<request*> queue;
        std::mutex queue_lock;
        char padding[2*CACHE_LINE_SIZE - sizeof(std::queue<request>) - sizeof(std::mutex)];
    };

    struct worker_init_data {
        const std::string dir_name;
        const int core_id;
        synch_queue *queues;
        const int num_queues;
        const bool *should_exit;

        worker_init_data(const std::string dir_name, const int core_id,
                         synch_queue *queues, const int num_queues, const bool *should_exit)
        :  dir_name(dir_name), core_id(core_id), queues(queues), num_queues(num_queues),
            should_exit(should_exit)
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
    bool should_stop;

    /* Access using [worker cpu][client cpu]. */
    synch_queue **request_matrix;

    std::vector<std::thread> workers;

    static void spawn_worker(worker_init_data init_data);
};
