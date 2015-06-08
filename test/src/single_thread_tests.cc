#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <sys/dir.h>
#include <sys/stat.h>

#include "skvlr.h"
#include "worker.h"
#include "skvlr_test.h"

const std::string TEST_OPEN = TEST_DUMP_DIR + "test_open_db";
const std::string TEST_DESTRUCTOR = TEST_DUMP_DIR + "test_destructor_db";
const std::string TEST_VALID_VALUES = TEST_DUMP_DIR + "test_valid_values_db";
const std::string TEST_WORKER = TEST_DUMP_DIR + "worker";

/**
 * Removes file if it exists. Return 0 on success (file didn't exist or
 * existent file successfully deleted), nonzero value otherwise
 */
static int remove_if_exists (std::string file_name) {
    struct stat buffer;
    if (stat(file_name.c_str(), &buffer) != 0) {
        return 0;
    }

    return std::remove(file_name.c_str());
}

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

    return true;
}

static bool test_worker_persistence_single() {
    if (!prepare_for_next_suite(TEST_WORKER)) return false;

    // set up test state
    update_maps q;
    bool terminate = false;
    worker_init_data data(0, &q, &terminate);

    std::string file_name = "test/test_dump/test_worker_persistence_single";
    check(remove_if_exists(file_name) == 0);

    struct global_state global_state(file_name);
    Worker w(data, &global_state);

    // construct test data
    std::map<int, int> puts_map;
    puts_map[1] = 2;

    w.persist(puts_map);  // persist!

    // check persisted values

    std::ifstream output_file(file_name);
    int key, value;
    output_file >> key >> value;
    check_eq(key,   1);
    check_eq(value, 2);
    output_file.close();

    std::remove(file_name.c_str());

    return true;
}

// Write a bunch of values and make sure that we can reconstruct them.
static bool test_worker_persistence_map() {
    if (!prepare_for_next_suite(TEST_WORKER)) return false;

    // set up test state
    update_maps q;
    bool terminate = false;
    worker_init_data data(0, &q, &terminate);

    std::string file_name = "test/test_dump/test_worker_persistence_map";
    check(remove_if_exists(file_name) == 0);

    struct global_state global_state(file_name);
    Worker w(data, &global_state);

    // construct test data
    std::map<int, int> values;
    for (int i = 0; i < 1000; ++i) {
        values[i] = 2 * i;
    }

    w.persist(values); // persist!

    // check persisted values
    std::ifstream output_file(file_name);
    int key, value;
    size_t num_values_seen = 0;
    while (output_file >> key >> value) {
        check_eq(value, values[key]);
        num_values_seen++;
    }

    check_eq(num_values_seen, values.size());
    return true;
}

static bool test_global_state_loads_from_file() {
    if (!prepare_for_next_suite(TEST_WORKER)) return false;

    // set up test state
    update_maps q;
    bool terminate = false;
    worker_init_data data(0, &q, &terminate);

    std::string file_name = "test/test_dump/test_global_state_loads_from_file";
    check(remove_if_exists(file_name) == 0);

    struct global_state global_state(file_name);
    Worker w(data, &global_state);

    // construct test data
    std::map<int, int> values;
    for (int i = 0; i < 1000; ++i) {
        values[i] = 2 * i;
    }

    w.persist(values); // persist!


    // check that when loaded data is still there
    struct global_state retrieved_state(file_name);
    for (int i = 0; i < 1000; ++i) {
        check_eq(retrieved_state.global_data[i], 2 * i);
    }

    return true;
}

BEGIN_TEST_SUITE(single_thread_tests) {
    run_test(test_valid_values);
    run_test(test_worker_persistence_single);
    run_test(test_worker_persistence_map);
    run_test(test_global_state_loads_from_file);
}
