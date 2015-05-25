/**
 * @file
 *
 * The test utility header file.
 *
 * @author Sergio Benitez
 * @date 09/03/2014
 */

#ifndef TEST_H
#define TEST_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/*
 * Typed versions of min() and max().
 */
#ifndef max
#define max(a,b) \
  ({ \
     __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; \
   })
#endif

#ifndef min
#define min(a,b) \
  ({ \
     __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; \
   })
#endif

/**
 * Checks that the first and second strings are equivalent.
 *
 * This macro can be invoked like a function. The calling function will be
 * returned from (with return value false) if the check fails.
 *
 * @param first The first string.
 * @param second The second string.
 */
#define check_eq_str(first, second) \
  do { \
    if (strcmp((first), (second))) \
    { \
      printf(":: %s:%d: Check '%s == %s' (\"%s\" == \"%s\") failed\n", \
          __FILE__, __LINE__, #first, #second, \
          first, second); \
      return false;  \
    } \
  } while (0)

/**
 * Checks that the first and second numbers are equivalent.
 *
 * This macro can be invoked like a function. The calling function will be
 * returned from (with return value false) if the check fails.
 *
 * @param first The first number.
 * @param second The second number.
 */
#define check_eq(first, second) \
  do { \
    if (!((first) == (second))) \
    { \
      printf(":: %s:%d: Check '%s == %s' (%ld == %ld) failed\n", \
          __FILE__, __LINE__, #first, #second, \
          (long) (first), (long) (second)); \
      return false;  \
    } \
  } while (0)

/**
 * Checks that the first and second numbers are not equivalent.
 *
 * This macro can be invoked like a function. The calling function will be
 * returned from (with return value false) if the check fails.
 *
 * @param first The first number.
 * @param second The second number.
 */
#define check_neq(first, second) \
  do { \
    if (!((first) != (second))) \
    { \
      printf(":: %s:%d: Check '%s != %s' (%ld != %ld) failed\n", \
          __FILE__, __LINE__, #first, #second, \
          (long) (first), (long) (second)); \
      return false;  \
    } \
  } while (0)

/**
 * Checks that some condition evaluates to true.
 *
 * This macro can be invoked like a function. The calling function will be
 * returned from (with return value false) if the check fails.
 *
 * @param condition The condition to check.
 */
#define check(condition) \
  do { \
    if (!(condition)) \
    { \
      printf(":: %s:%d: Check '%s' failed\n", \
          __FILE__, __LINE__, #condition); \
      return false;  \
    } \
  } while (0)

/*
 * The testing library BEGIN_TESTING macro. Sets up the _main_test_function.
 */
#define BEGIN_TESTING \
  void _main_test_function(bool *_result)

/**
 * The testing library run_suite macro. This should only be called inside of the
 * main testing function. This macro will run a given suite, setting the result
 * suite parameter appropriately.
 *
 * @param test_suite The test suite to run.
 */
#define run_suite(test_suite) \
  do { \
    bool success = _run_suite(#test_suite, test_suite); \
    *_result = *_result && success; \
  } while (0)

/*
 * The testing library BEGIN_TEST_SUITE macro. Sets up a test suite function.
 *
 * @param name The name of the test suite function.
 */
#define BEGIN_TEST_SUITE(name) \
  void name(bool *_result, int *_num_tests, int *_num_passed)

/**
 * The testing library run_test macro. This should only be called inside of a
 * test suite function. This macro will run a given test, setting the result
 * parameter appropriately.
 *
 * @param test_fn The test function to run.
 */
#define clean_run_test(test_fn, cleanup) \
  do { \
    bool success = _run_test(#test_fn, test_fn, _num_tests, _num_passed, cleanup); \
    *_result = *_result && success; \
  } while (0)

#define run_test(test_fn) \
  do { \
    bool success = _run_test(#test_fn, test_fn, _num_tests, _num_passed, NULL); \
    *_result = *_result && success; \
  } while (0)

/*
 * Internal functions. These shouldn't be called directly.
 */
void _main_test_function(bool *);
bool _run_test(const char *name, bool (*fn)(void), int *ntests, int *npassed,
    void (*cleanup)(void));
bool _run_suite(const char *name, void (*fn)(bool *result, int *_num_tests,
      int *_num_passed));

int begin_testing(int, const char *argv[]);
#endif
