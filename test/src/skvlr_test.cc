#include <iostream>
#include <map>

#include "skvlr.h"
#include "test.h"

int main(int argc, const char *argv[]) {
  return begin_testing(argc, argv);
}

// put() random values, asserts that values are with consistent get() on a single core
static bool test_valid_values() {
  Skvlr test_kv = Skvlr("test_db", 1);
  for (int i = 0; i < 1000; i++) {
    test_kv.db_put(i, i);
  }
  for (int i = 0; i < 1000; i++) {
    int value;
    test_kv.db_get(i, &value);
    check_eq(value, i);
  }
  return true;
}

// Time out value is set in test_utils.cc
BEGIN_TEST_SUITE(sanity_checks) {
  run_test(test_valid_values);
}

BEGIN_TESTING {
  run_suite(sanity_checks);
}
