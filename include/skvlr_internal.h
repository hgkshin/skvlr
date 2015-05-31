#include <string>
#include <mutex>
#include <map>
#include <thread>
#include <queue>
#include <sstream>

#pragma once

#define UNUSED(x) (void)x;

/* Debug macros per file type */
// #define DEBUG_WORKER_FLAG true
#define DEBUG_SKVLR_FLAG true
#define DEBUG_TEST_FLAG true
#define DEBUG_PROFILER_FLAG true

#ifdef DEBUG_WORKER_FLAG
#  define DEBUG_WORKER(x) std::cerr << "\033[1;34mWORKER LOG: " << x << "\033[0m"
#else
#  define DEBUG_WORKER(x) do {} while (0)
#endif

#ifdef DEBUG_SKVLR_FLAG
#  define DEBUG_SKVLR(x) std::cerr << "\033[1;31mSKVLR LOG: " << x << "\033[0m"
#else
#  define DEBUG_SKVLR(x) do {} while (0)
#endif

#ifdef DEBUG_TEST_FLAG
#  define DEBUG_TEST(x) std::cerr << "\033[1;33mTEST LOG: " << x << "\033[0m"
#else
#  define DEBUG_TEST(x) do {} while (0)
#endif

#ifdef DEBUG_PROFILER_FLAG
#  define DEBUG_PROFILER(x) std::cerr << "\033[1;32mPROFILER LOG: " << x << "\033[0m"
#else
#  define DEBUG_PROFILER(x) do {} while (0)
#endif

#define CACHE_LINE_SIZE 64

enum RequestType { GET, PUT };
enum RequestStatus { PENDING, SUCCESS, ERROR };

struct request {
    int key;
    union {
        int value_to_store; // PUT requests only
        int *return_value;  // GET requests only
    };
    bool synchronous;
    RequestType type;
    RequestStatus status;
    Semaphore sema;
};

struct synch_queue {
    std::queue<request*> queue;
    std::mutex queue_lock;
    char padding[2*CACHE_LINE_SIZE - sizeof(std::queue<request>) - sizeof(std::mutex)];
};

struct update_maps {
    char foo[64];
    std::map<int, int> local_state;
    pthread_spinlock_t puts_lock;
    std::map<int, int> local_puts;
    char bar[64];
};

struct worker_init_data {
    const std::string dir_name;
    const int core_id;
    update_maps *maps;
    const int num_queues;
    const bool *should_exit;

    worker_init_data(const std::string dir_name, const int core_id,
                     update_maps *maps, const int num_queues, const bool *should_exit)
    :  dir_name(dir_name), core_id(core_id), maps(maps), num_queues(num_queues),
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
