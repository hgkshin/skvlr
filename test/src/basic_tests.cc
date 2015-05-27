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
const std::string TEST_PINNING = TEST_DUMP_DIR + "test_pinning_db";

static bool test_db_open() {
  Skvlr test_kv(TEST_OPEN, 1);
  DIR *dir = opendir(TEST_OPEN.c_str());
  if (!dir) {
    return false;
  }
  closedir(dir); 
  return true;
}

static bool test_skvlr_destructor() {
    // The destructor should wait for threads to finish.  When test_kv goes out of scope, we
    // will either thrown an exception if a thread gets abandoned or hang indefinitely if
    // workers aren't joined properly.
    DEBUG_TEST("Total cores available: " << total_cores() << std::endl);
    Skvlr test_kv(TEST_DESTRUCTOR, total_cores());
    return true;
}

static bool test_pinning() {
    Skvlr test_kv(TEST_PINNING, total_cores());
    sleep(5); // Allow workers to spawn
    int index = 0;
    for (auto& worker : test_kv.workers) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        check_neq(pthread_getaffinity_np(worker.native_handle(), sizeof(cpu_set_t), &cpuset), 1);
        for (int i = 0; i < total_cores(); i++) {
            if (CPU_ISSET(i, &cpuset)) {
                check_eq(index, i);
            }
        }
        index++;
    }
    return true;
} 

BEGIN_TEST_SUITE(basic_tests) {
    run_test(test_db_open);
    run_test(test_skvlr_destructor);
    run_test(test_pinning);
}




