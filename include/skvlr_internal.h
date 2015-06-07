#include <iostream>
#include <string>
#include <mutex>
#include <map>
#include <thread>
#include <queue>
#include <sstream>
#include <fstream>
#include <vector>
#include <condition_variable>

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

struct update_maps {
    // The local read-only state and the RW lock on it.
    pthread_rwlock_t local_state_lock;
    std::map<int, int> local_state;
    pthread_spinlock_t puts_lock;

    // Can't this just be a vector? That'll be less overhead;
    // it's not like we need O(1) random access.
    std::map<int, int> local_puts;

    std::mutex sync_mutex;
    std::condition_variable sync_cv;
    int num_updates;

    std::map<int, std::vector<std::function<void(const int)>>> watches;
} __attribute__((aligned(CACHE_LINE_SIZE)));

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

struct global_state {
    std::map<int, int> global_data;
    std::ofstream outputLog;
    pthread_spinlock_t global_state_lock;

    global_state(const std::string& file_name)
    : outputLog(file_name) {
        pthread_spin_init(&global_state_lock, PTHREAD_PROCESS_SHARED);
        // Read the contents of the data file if any and store it in `data`.
        std::ifstream f(file_name);
        int key, value;
        if (f.fail()) return;
        while (f >> key >> value) {
            global_data[key] = value;
        }
        f.close();
    }

    void lock() {
      pthread_spin_lock(&global_state_lock);
    }

    void unlock() {
      pthread_spin_unlock(&global_state_lock);
    }
};
