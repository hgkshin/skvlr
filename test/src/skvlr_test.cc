#include <iostream>
#include <fstream>
#include <map>

#include "skvlr.h"
#include "worker.h"
#include "test.h"

int main(int argc, const char *argv[]) {
  return begin_testing(argc, argv);
}

// put() random values, asserts that values are with consistent get() on a single core
static bool test_valid_values() {
    Skvlr test_kv("test_db", 1);
    for (int i = 0; i < 1000; i++) {
        test_kv.db_put(i, i);
    }

    sleep(1);

    for (int i = 0; i < 1000; i++) {
        int value;
        test_kv.db_get(i, &value);
        check_eq(value, i);
    }

    std::cout << "complete!" << std::endl;
    return true;
}

static bool prepare_worker_directory()
{
    check_eq(system("rm -rf worker"), 0);
    check_eq(system("mkdir worker"), 0);
    return true;
}

static bool test_worker_persistence_single() {
    if (!prepare_worker_directory()) return false;

    Skvlr::synch_queue q;
    bool terminate = false;
    Skvlr::worker_init_data data("worker", 0, &q, 1, &terminate);

    Worker w(data);

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
    if (!prepare_worker_directory()) return false;

    Skvlr::synch_queue q;
    bool terminate = false;
    Skvlr::worker_init_data data("worker", 0, &q, 1, &terminate);
    Worker w(data);
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
    if (!prepare_worker_directory()) return false;

    // Write values to one worker.
    Skvlr::synch_queue q;
    bool terminate = false;
    Skvlr::worker_init_data data("worker", 0, &q, 1, &terminate);
    {
        // Create a separate scope to invoke the worker destructor.
        Worker w(data);
        for (int i = 0; i < 1000; ++i) {
            w.persist(i, 2 * i);
        }
    }

    // Start a second worker in the same directory.
    Worker secondWorker(data);
    for (int i = 0; i < 1000; ++i) {
        int value;
        Skvlr::request req;
        req.key = i;
        req.return_value = &value;
        req.type = Skvlr::RequestType::GET;
        req.status = Skvlr::RequestStatus::PENDING;
        secondWorker.handle_get(&req);
        check_eq(value, 2 * i);
    }

    return true;
}

static bool test_skvlr_destructor() {
    // The destructor should wait for threads to finish.  When test_kv goes out of scope, we
    // will either thrown an exception if a thread gets abandoned or hang indefinitely if
    // workers aren't joined properly.
    Skvlr test_kv("test_db", 4);
    return true;
}

// Time out value is set in test_utils.cc
BEGIN_TEST_SUITE(sanity_checks) {
    run_test(test_valid_values);
    run_test(test_worker_persistence_single);
    run_test(test_worker_persistence_map);
    run_test(test_worker_loads_from_file);
    run_test(test_skvlr_destructor);
}

BEGIN_TESTING {
  run_suite(sanity_checks);
}
