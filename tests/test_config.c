/**
 * @file test_config.c
 * @brief Unit tests for threshold_config_init.
 */

#include "threshold_eval.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================ Test framework ========================================== */

#define TEST_ASSERT(expr)                                                      \
        do {                                                                   \
                if (!(expr)) {                                                 \
                        fprintf(stderr, "FAIL  %s:%d  %s\n", __FILE__,         \
                                __LINE__, #expr);                              \
                        exit(EXIT_FAILURE);                                    \
                }                                                              \
        } while (0)

#define TEST_ASSERT_EQUAL(expected, actual) TEST_ASSERT((expected) == (actual))
#define TEST_ASSERT_TRUE(expr)              TEST_ASSERT(!!(expr))
#define TEST_ASSERT_FALSE(expr)             TEST_ASSERT(!(expr))
#define TEST_ASSERT_NOT_NULL(ptr)           TEST_ASSERT((ptr) != NULL)

#define TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual)                      \
        TEST_ASSERT(fabsf((float)(actual) - (float)(expected))                 \
                    <= (float)(delta))

#define TEST_PASS(name) fprintf(stdout, "PASS  %s\n", (name))

#define TEST_CASE(name)                                                        \
        static void name(void);                                                \
        static void name(void)

static void
run_test(void (*test_func)(void), const char *name)
{
        test_func();
        TEST_PASS(name);
}

/* ================ threshold_config_init tests ============================= */

TEST_CASE(test_config_init_sets_defaults)
{
        threshold_config_t cfg;
        memset(&cfg, 0xFF, sizeof(cfg));

        threshold_status_t status = threshold_config_init(&cfg);

        TEST_ASSERT_EQUAL(THRESHOLD_STATUS_OK, status);
        TEST_ASSERT_EQUAL(THRESHOLD_TYPE_NONE, cfg.type);
        TEST_ASSERT_EQUAL(THRESHOLD_POLICY_FAILSAFE, cfg.policy);
        TEST_ASSERT_FLOAT_WITHIN(1e-6f, THRESHOLD_EVAL_EPSILON, cfg.epsilon);
        TEST_ASSERT_TRUE(isnan(cfg.lolo));
        TEST_ASSERT_TRUE(isnan(cfg.lo));
        TEST_ASSERT_TRUE(isnan(cfg.hi));
        TEST_ASSERT_TRUE(isnan(cfg.hihi));
}

TEST_CASE(test_config_init_null_returns_error)
{
        threshold_status_t status = threshold_config_init(NULL);
        TEST_ASSERT_EQUAL(THRESHOLD_STATUS_INVALID_ARG, status);
}

/* ================ Main ==================================================== */

int
main(void)
{
        fprintf(stdout, "\n=== Running test_config unit tests ===\n\n");

        run_test(test_config_init_sets_defaults,
                 "test_config_init_sets_defaults");
        run_test(test_config_init_null_returns_error,
                 "test_config_init_null_returns_error");

        fprintf(stdout, "\n=== All tests passed ===\n\n");
        return EXIT_SUCCESS;
}
