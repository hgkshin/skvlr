#include <iostream>
#include <map>
#include <sys/dir.h>


#include "skvlr.h"
#include "test.h"

const std::string PARENT_DIR = "test/test_dump/";

int main(int argc, const char *argv[]) {
  return begin_testing(argc, argv);
}

static bool test_db_open() {
  std::string db_path = PARENT_DIR + "test_open_db";
  Skvlr test_kv(db_path, 1);
  std::cout << "Attempting to open DB" << std::endl;
  DIR *dir = opendir(db_path.c_str());
  std::cout << "Opened DB" << std::endl;
  
  if (!dir) {
    return false;
  }
  closedir(dir); 
  return true;
}

// put() random values, asserts that values are with consistent get() on a single core
static bool test_valid_values() {
  Skvlr test_kv(PARENT_DIR + "test_valid_values_db", 1);
  for (int i = 0; i < 10; i++) {
    test_kv.db_put(i, i);
  }
  //sleep(5); // TODO: address with synchronous put()
  for (int i = 0; i < 10; i++) {
    int value;
    test_kv.db_get(i, &value);
    check_eq(value, i);
  }
  return true;
}


/* Notes:
 *   1. make test removes and remakes the test_dump directory
 *   2. time out value is in test_utils.cc and is set to 20 sec
 */
BEGIN_TEST_SUITE(sanity_checks) {
  run_test(test_db_open);
  run_test(test_valid_values);

}

BEGIN_TESTING {
  run_suite(sanity_checks);
}
