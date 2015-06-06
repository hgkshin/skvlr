#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <sys/dir.h>

#include "skvlr.h"
#include "worker.h"
#include "skvlr_test.h"

const std::string TEST_OPEN = TEST_DUMP_DIR + "test_open_db";
const std::string TEST_DESTRUCTOR = TEST_DUMP_DIR + "test_destructor_db";
const std::string TEST_VALID_VALUES = TEST_DUMP_DIR + "test_valid_values_db";
const std::string TEST_WORKER = TEST_DUMP_DIR + "worker";

// put() random values, asserts that values are with consistent get() on a single core
static bool test_valid_values() {
  Skvlr test_kv(TEST_VALID_VALUES, 1);
    for (int i = 0; i < 1000; i++) {
        test_kv.db_put(i, i);
    }
    test_kv.db_sync();
    for (int i = 0; i < 1000; i++) {
        int value;
        test_kv.db_get(i, &value);
        check_eq(value, i);
    }

    std::cout << "complete!" << std::endl;
    return true;
}

static bool test_worker_persistence_single() {
    if (!prepare_for_next_suite(TEST_WORKER)) return false;

    update_maps q;
    bool terminate = false;
    worker_init_data data(TEST_WORKER.c_str(), 0, &q, 1, &terminate);

    struct global_state global_state("blah");//TODO: update this
    Worker w(data, &global_state);

    w.persist(1, 2);

    std::fstream outputFile(data.dataFilePath());
    check(outputFile.good());

    int key, value;
    outputFile >> key >> value;
    check_eq(key,   1);
    check_eq(value, 2);
    outputFile.close();

    return true;
}

// Write a bunch of values and make sure that we can reconstruct them.
static bool test_worker_persistence_map() {
    if (!prepare_for_next_suite(TEST_WORKER)) return false;

    update_maps q;
    bool terminate = false;
    worker_init_data data(TEST_WORKER.c_str(), 0, &q, 1, &terminate);
    struct global_state global_state("blah"); // TODO: update this to correct file name
    Worker w(data, &global_state);
    std::map<int, int> values;
    for (int i = 0; i < 1000; ++i) {
        values[i] = 2 * i;
        w.persist(i, values[i]);
    }

    std::fstream outputFile(data.dataFilePath());
    check(outputFile.good());

    int key, value;
    size_t num_values_seen = 0;
    while (outputFile >> key >> value) {
        check_eq(value, values[key]);
        num_values_seen++;
    }
    check_eq(num_values_seen, values.size());
    outputFile.close();

    return true;
}

static bool test_worker_loads_from_file() {
    if (!prepare_for_next_suite(TEST_WORKER)) return false;

    // Write values to one worker.
    update_maps q;
    bool terminate = false;
    worker_init_data data(TEST_WORKER.c_str(), 0, &q, 1, &terminate);
    struct global_state global_state("blah");//TODO: update this to correct name
    {
        // Create a separate scope to invoke the worker destructor.
        Worker w(data, &global_state);
        for (int i = 0; i < 1000; ++i) {
            w.persist(i, 2 * i);
        }
    }
    sleep(1);

    // Start a second worker in the same directory.
    Worker secondWorker(data, &global_state);
    for (int i = 0; i < 1000; ++i) {
        check_eq(global_state.global_data[i], 2 * i);
    }

    return true;
}

BEGIN_TEST_SUITE(single_thread_tests) {
    run_test(test_valid_values);
    run_test(test_worker_persistence_single);
    run_test(test_worker_persistence_map);
    run_test(test_worker_loads_from_file);
}
