#include "unity.h"

// FND-03: Unity harness smoke test (NFR-010, FR-070).
void setUp(void) {}
void tearDown(void) {}

static void test_unity_runs(void) { TEST_ASSERT_TRUE(1); }

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_unity_runs);
  return UNITY_END();
}
