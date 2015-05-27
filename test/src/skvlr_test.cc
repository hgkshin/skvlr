#include <iostream>
#include <fstream>
#include <map>
#include <sys/dir.h>
#include <assert.h>

#include "skvlr.h"
#include "worker.h"
#include "skvlr_test.h"

int total_cores() {
 return sysconf(_SC_NPROCESSORS_ONLN);
}

bool prepare_for_next_suite(const std::string directory) {
    std::string remove_command = "rm -rf ";
    remove_command += directory;
    std::string make_command = "mkdir ";
    make_command += directory;
    bool success = system(remove_command.c_str()) == 0;
    success = success && system(make_command.c_str()) == 0;
    return success;
}

int main(int argc, const char *argv[]) {
    return begin_testing(argc, argv);
}

// Note: time out value is in test_utils.cc and is set to 20 sec
BEGIN_TESTING {
    prepare_for_next_suite(TEST_DUMP_DIR);
    run_suite(basic_tests);
    prepare_for_next_suite(TEST_DUMP_DIR);
    run_suite(single_thread_tests);
    //run_suite(many_thread_tests);
}
