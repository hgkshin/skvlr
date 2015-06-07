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
    std::map<int, int> *old_state = NULL;
    // Sleep for a random offset ala Raft
    unsigned int millisecond_offset = rand() % 100;
    usleep(millisecond_offset * 1000);
    
    while(!*this->worker_data.should_exit) {
        unsigned int millisecond_sleep = 100;
        usleep(millisecond_sleep * 1000); // This is in microseconds

        double start_time = get_wall_time();
        // We figure most gets on the old map are done by the time
        // we're swapping out the map.
        if(old_state != NULL) {
            delete old_state;
        }

        std::map<int, int> core_local_puts;
        pthread_spin_lock(&this->worker_data.maps->puts_lock);
        core_local_puts.swap(this->worker_data.maps->local_puts);
        pthread_spin_unlock(&this->worker_data.maps->puts_lock);

        global_state->lock();
        global_state->global_data.insert(core_local_puts.begin(), core_local_puts.end());

        std::map<int, int> *new_state = new std::map<int, int>(
            global_state->global_data.begin(), global_state->global_data.end());                                                              

        old_state = this->worker_data.maps->local_state;
        this->worker_data.maps->local_state = new_state;

        // If we're okay with a looser persistence model, we
        // can move this operation to after the unlock.
        persist(core_local_puts);
        global_state->unlock();
        
        double end_time = get_wall_time();
        UNUSED(start_time);
        UNUSED(end_time);
        DEBUG_WORKER("Duration of worker: " << end_time - start_time << std::endl);
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
