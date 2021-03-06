#include <string>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <map>
#include <sys/dir.h>
#include <sys/time.h>
#include <sched.h>
#include <map>
#include <thread>
#include <vector>

#include "skvlr.h"
#include "worker.h"
#include "skvlr_test.h"

const int NUM_CORES = 4;
const int SECS_IN_TEXT = 6;
const int TIMEOUT_LENGTH = 1;

const std::string TEST_SINGLE_SYNC = TEST_DUMP_DIR + "test_single_sync";
const std::string TEST_SINGLE_SEQUENTIAL_SYNC =
    TEST_DUMP_DIR + "test_single_sequential_sync";
const std::string TEST_MULTIPLE_SYNC = TEST_DUMP_DIR + "test_multiple_sync";
const std::string TEST_SINGLE = TEST_DUMP_DIR + "test_single";
const std::string TEST_MULTIPLE = TEST_DUMP_DIR + "test_multiple";
const std::string TEST_WATCH = TEST_DUMP_DIR + "test_watch";

//static double get_wall_time() {
    //struct timeval time;
    //if (gettimeofday(&time, NULL)){
        //return 0;
    //}
    //return (double)time.tv_sec + (double)time.tv_usec * .000001;
//}

static int generate_val(int key) {
    return key * 2;
}

static bool test_single_client_sync() {
    Skvlr kv(TEST_SINGLE_SYNC, 1);

    int start = -1000;
    int end = 1000;

    for (int key = start; key < end; ++key) {
        int val;
        kv.db_get(key, &val);
        check_eq(val, 0);
        val = generate_val(key);
        kv.db_put(key, val);
    }

    kv.db_sync();

    for (int key = start; key < end; ++key) {
        int val;
        kv.db_get(key, &val);
        check_eq(val, generate_val(key));
    }
    return true;
}

static bool test_single_client_sequential_sync() {
    Skvlr kv(TEST_SINGLE_SEQUENTIAL_SYNC, 1);

    int start = -5;
    int end = 5;

    for (int key = start; key < end; ++key) {
        int val;
        kv.db_get(key, &val);

        check_eq(val, 0);
        val = generate_val(key);
        kv.db_put(key, val);
        kv.db_sync();

        // check all previously put keys, and this one
        for (int check_key = start; check_key <= key; ++check_key) {
            kv.db_get(check_key, &val);
            check_eq(val, generate_val(check_key));
        }
    }
    return true;
}

/**
 * Adds data to kv from start_key to end_key generated by generate_val,
 * keeping track of the data added in a shared map (data). Gets mtx before
 * each insert and releases it after each insert so that the world can be
 * stopped by the parent thread periodically to test Skvlr state. Pins itself
 * to core_num.
 */
static void put_data(int start_key, int end_key, Skvlr *kv,
        std::map<int, int> *data, std::mutex *mtx, int core_num) {
    /* Set up processor affinity. */
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_num, &cpuset);
    assert(!pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset));

    for (int key = start_key; key < end_key; ++key) {
        mtx->lock();
        int val = generate_val(key);
        kv->db_put(key, val, core_num);
        (*data)[key] = val;
        mtx->unlock();
    }
    kv->db_sync(core_num);
}

static bool test_multiple_clients_sync() {
    Skvlr kv(TEST_MULTIPLE_SYNC, NUM_CORES);
    std::vector<std::thread> workers(NUM_CORES);
    std::map<int, int> data;
    std::mutex mtx;

    int num_total_inserts_per_worker = 10000;
    int num_rounds = 10;
    int num_inserts_per_round = num_total_inserts_per_worker / num_rounds;

    for (int round = 0; round < num_rounds; ++round) {
        // spawn put worker threads
        for (int core_num = 0; core_num < NUM_CORES; ++core_num) {
            int start = core_num * num_total_inserts_per_worker +
                round * num_inserts_per_round;
            int end = start + num_inserts_per_round;

            workers[core_num] = std::thread(&put_data, start, end, &kv, &data, &mtx,
                    core_num);
        }

        // wait for completion
        for (int core_num = 0; core_num < NUM_CORES; ++core_num) {
            workers[core_num].join();
        }

        // sync to ensure flush to skvlr
        kv.db_sync(0);

        // check expected number of elems inserted locally
        check_eq((int) data.size(),
                NUM_CORES * ((round + 1) * num_inserts_per_round));

        // verify all values stored locally are in skvlr
        for (auto iter = data.begin(); iter != data.end(); ++iter) {
            int key = iter->first;
            int val = iter->second;
            int retrieved_val;
            kv.db_get(key, &retrieved_val, 0);
            check_eq(retrieved_val, val);
        }
    }

    return true;
}

static bool test_single_client() {
    Skvlr kv(TEST_SINGLE, 1);

    int start = -1000;
    int end = 1000;

    for (int key = start; key < end; ++key) {
        int val;
        kv.db_get(key, &val);
        check_eq(val, 0);
        val = generate_val(key);
        kv.db_put(key, val);
    }

    sleep(2);

    for (int key = start; key < end; ++key) {
        int val;
        kv.db_get(key, &val);
        check_eq(val, generate_val(key));
    }

    return true;
}

static bool test_multiple_clients() {
    Skvlr kv(TEST_MULTIPLE, NUM_CORES);
    std::vector<std::thread> workers(NUM_CORES);
    std::map<int, int> data;
    std::mutex mtx;

    int num_total_inserts_per_worker = 10000;
    int num_rounds = 4;
    int num_inserts_per_round = num_total_inserts_per_worker / num_rounds;

    for (int round = 0; round < num_rounds; ++round) {
        // spawn put worker threads
        for (int core_num = 0; core_num < NUM_CORES; ++core_num) {
            int start = core_num * num_total_inserts_per_worker +
                round * num_inserts_per_round;
            int end = start + num_inserts_per_round;

            workers[core_num] = std::thread(&put_data, start, end, &kv, &data, &mtx,
                    core_num);
        }

        // wait for completion
        for (int core_num = 0; core_num < NUM_CORES; ++core_num) {
            workers[core_num].join();
        }

        // sleep long enough to ensure flush to skvlr
        sleep(2);

        // check expected number of elems inserted locally
        check_eq((int) data.size(),
                NUM_CORES * ((round + 1) * num_inserts_per_round));

        // verify all values stored locally are in skvlr
        for (auto iter = data.begin(); iter != data.end(); ++iter) {
            int key = iter->first;
            int val = iter->second;
            int retrieved_val;
            kv.db_get(key, &retrieved_val);
            check_eq(retrieved_val, val);
        }
    }

    return true;
}

int global_watch_callback_value = 0;

void callback_fn(const int test) {
    UNUSED(test);
    global_watch_callback_value++;
}

static bool test_watch() {
    const int kKeyToWatch = 42;
    Skvlr kv(TEST_WATCH, 1);
    kv.db_put(kKeyToWatch, -1);
    kv.db_watch(kKeyToWatch, callback_fn, 1);
    kv.db_put(kKeyToWatch, 2);
    kv.db_sync();
    check_eq(global_watch_callback_value, 1);
    return true;
}

BEGIN_TEST_SUITE(test_consistency) {
    run_test(test_single_client_sync);
    run_test(test_single_client_sequential_sync);
    run_test(test_multiple_clients_sync);
    run_test(test_single_client);
    run_test(test_multiple_clients);
    run_test(test_watch);
}
