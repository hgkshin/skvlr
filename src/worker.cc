#include <iostream>
#include <assert.h>
#include <sys/time.h>
#include <ctime>

#include "worker.h"

Worker::Worker(const worker_init_data init_data, struct global_state *global_state)
    : global_state(global_state), worker_data(init_data),
      total_gets(0), total_puts(0)
{
    // empty
}

Worker::~Worker()
{
    DEBUG_WORKER("Total gets for core #" << worker_data.core_id << ": "
                 << total_gets << std::endl);
    DEBUG_WORKER("Total puts for core #" << worker_data.core_id << ": "
                 << total_puts << std::endl);
}

double Worker::get_wall_time() {
    struct timeval time;
    if (gettimeofday(&time, NULL)){
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

/**
 * Listen for incoming requests by busy waiting and inspecting the queue of
 * requests for new PENDING requests to service.
 */
void Worker::listen()
{
    // Sleep for a random offset ala Raft
    unsigned int millisecond_offset = rand() % 1000;
    usleep(millisecond_offset * 1000);

    while(!*this->worker_data.should_exit) {
        unsigned int millisecond_sleep = 500;
        usleep(millisecond_sleep * 1000); // This is in microseconds

        double start_time = get_wall_time();

        // Get a snapshot of the local state and all watches on this core.
        update_maps *core_data = this->worker_data.maps;

        std::map<int, int> core_local_puts;

        std::map<int, std::vector<std::function<void(const int)>>> watches;
        std::map<int, int> updated_values;

        // Lock down puts while we apply them.
        pthread_spin_lock(&core_data->puts_lock);
        core_local_puts.swap(core_data->local_puts);
        watches = core_data->watches;
        pthread_spin_unlock(&core_data->puts_lock);

        global_state->lock();
        // Update the global state with all puts from this core.
        for (auto kv : core_local_puts) {
            global_state->global_data[kv.first] = kv.second;
        }

        // Update the local read-only state to reflect the global state.  This pulls in updates from
        // different cores as well as our own puts.  Note that we must hold the global lock here
        // since we're reading what other cores could be writing.
        // TODO: RW lock
        pthread_rwlock_wrlock(&this->worker_data.maps->local_state_lock);
        for (auto &kv : global_state->global_data) {
            // Collect the values that changed on our core so we can notify watches.
            if (core_data->local_state.find(kv.first) == core_data->local_state.end() ||
                core_data->local_state[kv.first] != kv.second) {
                updated_values[kv.first] = kv.second;
            }

            core_data->local_state[kv.first] = kv.second;
        }
        pthread_rwlock_unlock(&this->worker_data.maps->local_state_lock);

        // If we're okay with a looser persistence model, we
        // can move this operation to after the unlock.
        persist(core_local_puts);
        global_state->unlock();

        // Notify all threads watching a key/value that got updated.  It's important we do this
        // after releasing all locks to avoid calling outside functions when we hold
        // performance-critical locks.
        for (auto &kv : updated_values) {
            for (auto &fn : watches[kv.first]) fn(kv.second);
        }

        double end_time = get_wall_time();
        UNUSED(start_time);
        UNUSED(end_time);
        DEBUG_WORKER("Duration of worker: " << end_time - start_time << std::endl);

        std::unique_lock<std::mutex> lock(core_data->sync_mutex);
        core_data->num_updates++;
        core_data->sync_cv.notify_all();
    }
}

/**
 * Persist a map of puts.
 *
 * @return 0 on success, or a negative value on failure.
 */
int Worker::persist(const std::map<int, int>& puts)
{
    for (auto kv: puts) {
        global_state->outputLog << kv.first << '\t' << kv.second << '\n';
    }
    global_state->outputLog << std::flush;

    return 0;
}
