/**
 * @file test_policies.c
 * @brief Unit tests for policy flag combinations and invalid sample handling.
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

/* ================ Policy flag combination tests ============================
 */

TEST_CASE(test_policy_flags_combined_failsafe_trip)
{
        threshold_config_t cfg;
        threshold_plan_t plan;
        threshold_config_init(&cfg);
        cfg.type = THRESHOLD_TYPE_RANGE;
        cfg.lolo = 10.0f;
        cfg.lo = 20.0f;
        cfg.hi = 80.0f;
        cfg.hihi = 90.0f;
        cfg.policy = THRESHOLD_POLICY_FAILSAFE;

        threshold_status_t status = threshold_plan_build(&plan, &cfg);
        TEST_ASSERT_EQUAL(THRESHOLD_STATUS_OK, status);
        TEST_ASSERT_TRUE(plan.valid);
        TEST_ASSERT_EQUAL(THRESHOLD_SEV_TRIP_HIGH,
                          threshold_plan_eval(&plan, NAN));
        TEST_ASSERT_EQUAL(THRESHOLD_SEV_TRIP_HIGH,
                          threshold_plan_eval(&plan, INFINITY));
}

TEST_CASE(test_policy_flags_combined_deescalate_warn)
{
        threshold_config_t cfg;
        threshold_plan_t plan;
        threshold_config_init(&cfg);
        cfg.type = THRESHOLD_TYPE_RANGE;
        cfg.lolo = 10.0f;
        cfg.lo = 20.0f;
        cfg.hi = 80.0f;
        cfg.hihi = 90.0f;
        cfg.policy = THRESHOLD_POLICY_DEESCALATE_WARN;

        threshold_status_t status = threshold_plan_build(&plan, &cfg);
        TEST_ASSERT_EQUAL(THRESHOLD_STATUS_OK, status);
        TEST_ASSERT_TRUE(plan.valid);
        TEST_ASSERT_EQUAL(THRESHOLD_SEV_WARN_HIGH,
                          threshold_plan_eval(&plan, NAN));
        TEST_ASSERT_EQUAL(THRESHOLD_SEV_WARN_HIGH,
                          threshold_plan_eval(&plan, INFINITY));
}

TEST_CASE(test_policy_flags_combined_ignore_invalid)
{
        threshold_config_t cfg;
        threshold_plan_t plan;
        threshold_config_init(&cfg);
        cfg.type = THRESHOLD_TYPE_RANGE;
        cfg.lolo = 10.0f;
        cfg.lo = 20.0f;
        cfg.hi = 80.0f;
        cfg.hihi = 90.0f;
        cfg.policy = THRESHOLD_POLICY_IGNORE_INVALID;

        threshold_status_t status = threshold_plan_build(&plan, &cfg);
        TEST_ASSERT_EQUAL(THRESHOLD_STATUS_OK, status);
        TEST_ASSERT_TRUE(plan.valid);
        TEST_ASSERT_EQUAL(THRESHOLD_SEV_OK, threshold_plan_eval(&plan, NAN));
        TEST_ASSERT_EQUAL(THRESHOLD_SEV_OK,
                          threshold_plan_eval(&plan, INFINITY));
}

TEST_CASE(test_policy_flags_combined_multiple_invalid)
{
        threshold_config_t cfg;
        threshold_plan_t plan;
        threshold_config_init(&cfg);
        cfg.type = THRESHOLD_TYPE_UPPER;
        cfg.hi = 80.0f;
        cfg.hihi = 90.0f;
        cfg.policy = THRESHOLD_POLICY_FAILSAFE_TRIP
                     | THRESHOLD_POLICY_DEESCALATE_WARN
                     | THRESHOLD_POLICY_IGNORE_INVALID;

        threshold_status_t status = threshold_plan_build(&plan, &cfg);
        TEST_ASSERT_EQUAL(THRESHOLD_STATUS_OK, status);
        TEST_ASSERT_TRUE(plan.valid);

        /* FAILSAFE_TRIP has highest priority */
        TEST_ASSERT_EQUAL(THRESHOLD_SEV_TRIP_HIGH,
                          threshold_plan_eval(&plan, NAN));
        TEST_ASSERT_EQUAL(THRESHOLD_SEV_TRIP_HIGH,
                          threshold_plan_eval(&plan, INFINITY));
}

TEST_CASE(test_policy_flags_combined_with_reorder)
{
        threshold_config_t cfg;
        threshold_plan_t plan;
        threshold_config_init(&cfg);
        cfg.type = THRESHOLD_TYPE_RANGE;
        cfg.lolo = 90.0f;
        cfg.lo = 80.0f;
        cfg.hi = 20.0f;
        cfg.hihi = 10.0f;
        cfg.policy = THRESHOLD_POLICY_ALLOW_REORDER;

        threshold_status_t status = threshold_plan_build(&plan, &cfg);
        TEST_ASSERT_EQUAL(THRESHOLD_STATUS_OK, status);
        TEST_ASSERT_TRUE(plan.valid);
        /* Thresholds should be reordered */
        TEST_ASSERT_FLOAT_WITHIN(1e-6f, 10.0f, plan.lolo);
        TEST_ASSERT_FLOAT_WITHIN(1e-6f, 20.0f, plan.lo);
        TEST_ASSERT_FLOAT_WITHIN(1e-6f, 80.0f, plan.hi);
        TEST_ASSERT_FLOAT_WITHIN(1e-6f, 90.0f, plan.hihi);
}

/* ================ Main ==================================================== */

int
main(void)
{
        fprintf(stdout, "\n=== Running test_policies unit tests ===\n\n");

        run_test(test_policy_flags_combined_failsafe_trip,
                 "test_policy_flags_combined_failsafe_trip");
        run_test(test_policy_flags_combined_deescalate_warn,
                 "test_policy_flags_combined_deescalate_warn");
        run_test(test_policy_flags_combined_ignore_invalid,
                 "test_policy_flags_combined_ignore_invalid");
        run_test(test_policy_flags_combined_multiple_invalid,
                 "test_policy_flags_combined_multiple_invalid");
        run_test(test_policy_flags_combined_with_reorder,
                 "test_policy_flags_combined_with_reorder");

        fprintf(stdout, "\n=== All tests passed ===\n\n");
        return EXIT_SUCCESS;
}
