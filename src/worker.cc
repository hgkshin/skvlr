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
        // TODO: Acquire global lock here.
        // global_state->insert(std::make_pair(key, value));
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
    while(!*this->worker_data.should_exit) {
        int time = this->worker_data.core_id % 10;
        sleep(10 + time / 10.0);

        std::map<int, int> core_local_puts;
        std::map<int, std::vector<std::function<void(const int)>>> watches;
        pthread_spin_lock(&this->worker_data.maps->puts_lock);
        core_local_puts.swap(this->worker_data.maps->local_puts);
        watches = this->worker_data.maps->watches;
        pthread_spin_unlock(&this->worker_data.maps->puts_lock);

        for (auto kv : core_local_puts) {
            (*global_state)[kv.first] = kv.second;
        }

        auto local_state = this->worker_data.maps->local_state;
        for (auto kv : *global_state) {
            if (local_state.find(kv.first) == local_state.end() ||
                local_state[kv.first] != kv.second) {
                for (auto &fn : watches[kv.first]) {
                    fn(kv.second);
                }
            }
        }

        this->worker_data.maps->local_state.insert(global_state->begin(), global_state->end());

        for (auto kv : core_local_puts) {
            persist(kv.first, kv.second);
        }
    }
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
