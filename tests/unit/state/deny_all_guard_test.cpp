#include <gtest/gtest.h>

#include "flight_state.hpp"
#include "flight_state_support.hpp"

// TC-STATE-028
TEST(DenyAllGuard, TC_STATE_028_evaluate_denies_every_pair_with_reason_zero) {
  DenyAllGuard guard;
  const IStateGuard& asInterface = guard;

  for (FlightState state : dfcs_test::kAllStates) {
    for (FlightEvent event : dfcs_test::kAllEvents) {
      SCOPED_TRACE(testing::Message() << "state=" << static_cast<int>(state)
                                      << " event=" << static_cast<int>(event));
      const GuardResult result = asInterface.evaluate(state, event);
      EXPECT_FALSE(result.allowed);
      EXPECT_EQ(result.reason, 0U);
    }
  }
}
