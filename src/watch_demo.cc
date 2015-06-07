#include <iostream>
#include <vector>
#include <thread>
#include <unistd.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <condition_variable>
#include <mutex>

#include "skvlr.h"




int main() {
    const int kNumCores = 4;
    const int kKeyToWatch = 42;

    Skvlr kv("watch", kNumCores);
    int num_threads_launched = 0;
    std::mutex wait_for_threads;
    std::condition_variable wait_for_threads_cv;

    printf("Main thread: Storing initial value.\n");
    kv.db_put(kKeyToWatch, -1);
    kv.db_sync();

    std::vector<std::thread> threads(kNumCores);
    for (int i = 0; i < kNumCores; ++i) {
        // Spawn a thread pretending to be on each core.
        const int core = i;
        threads[core] = std::thread([&, core]()  {
            kv.db_sync(core);
            bool can_exit = false;
            kv.db_watch(kKeyToWatch, [&](const int value) -> void {
                printf("Thread %d: %d => %d\n", core, kKeyToWatch, value);
                can_exit = true;
            }, core);

            // Notify that a thread launched.
            wait_for_threads.lock();
            num_threads_launched++;
            wait_for_threads_cv.notify_all();
            wait_for_threads.unlock();

            while (!can_exit) sleep(0.5);
        });
    }

    printf("Main thread: Waiting for threads to launch\n");
    std::unique_lock<std::mutex> lk(wait_for_threads);
    wait_for_threads_cv.wait(lk, [&num_threads_launched]() {
            return num_threads_launched == 4;
    });
    lk.unlock();

    printf("Main thread: Setting %d => %d\n", kKeyToWatch, 137);
    kv.db_put(kKeyToWatch, 137);
    kv.db_sync();

    for (std::thread &thread : threads) {
        thread.join();
    }
    printf("Main thread: All done\n");
    return 0;
}
