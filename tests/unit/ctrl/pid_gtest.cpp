// DC-50 (CTL-01) unit tests for dfcs::core::Pid (FR-040, FR-002, FR-061,
// FR-067). Expected values come from
// docs/test-specifications/DC-50-unit-test-spec.csv and the update() algorithm
// in docs/design/DC-50-pid-controller.md (4.2).

#include <gtest/gtest.h>

#include <limits>

#include "pid.hpp"

using dfcs::core::Pid;
using dfcs::core::PidGains;
using dfcs::core::PidResult;
using dfcs::core::PidStatus;

namespace {

constexpr float kNaN = std::numeric_limits<float>::quiet_NaN();
constexpr float kInf = std::numeric_limits<float>::infinity();
constexpr float kMax = std::numeric_limits<float>::max();
constexpr float kMinNormal = std::numeric_limits<float>::min();

Pid MakeConfigured(float kp, float ki, float kd) {
  Pid pid;
  EXPECT_EQ(pid.configure(PidGains{kp, ki, kd}), PidStatus::Ok);
  return pid;
}

void ExpectResult(const PidResult& result, PidStatus status, float output) {
  EXPECT_EQ(result.status, status);
  EXPECT_EQ(result.output, output);
}

void ExpectGains(const Pid& pid, float kp, float ki, float kd) {
  EXPECT_EQ(pid.gains().kp, kp);
  EXPECT_EQ(pid.gains().ki, ki);
  EXPECT_EQ(pid.gains().kd, kd);
}

}  // namespace

// TC-CTRL-002
TEST(Pid, TC_CTRL_002_p_term_only_output_is_kp_times_error) {
  Pid pid = MakeConfigured(2.0f, 0.0f, 0.0f);

  ExpectResult(pid.update(3.0f, 0.5f), PidStatus::Ok, 6.0f);
  EXPECT_EQ(pid.integral(), 0.0f);

  ExpectResult(pid.update(-1.5f, 0.5f), PidStatus::Ok, -3.0f);
  EXPECT_EQ(pid.integral(), 0.0f);

  ExpectResult(pid.update(0.0f, 0.5f), PidStatus::Ok, 0.0f);
  EXPECT_EQ(pid.integral(), 0.0f);
}

// TC-CTRL-007
TEST(Pid, TC_CTRL_007_i_term_only_ramps_by_ki_error_dt) {
  Pid pid = MakeConfigured(0.0f, 2.0f, 0.0f);

  ExpectResult(pid.update(3.0f, 0.5f), PidStatus::Ok, 3.0f);
  ExpectResult(pid.update(3.0f, 0.5f), PidStatus::Ok, 6.0f);
  ExpectResult(pid.update(3.0f, 0.5f), PidStatus::Ok, 9.0f);
  ExpectResult(pid.update(3.0f, 0.5f), PidStatus::Ok, 12.0f);
  EXPECT_EQ(pid.integral(), 12.0f);
}

// TC-CTRL-008
TEST(Pid, TC_CTRL_008_i_term_error_sign_flip_reverses_ramp) {
  Pid pid = MakeConfigured(0.0f, 2.0f, 0.0f);
  for (int i = 0; i < 4; ++i) {
    pid.update(3.0f, 0.5f);
  }
  ASSERT_EQ(pid.integral(), 12.0f);

  ExpectResult(pid.update(-3.0f, 0.5f), PidStatus::Ok, 9.0f);
  ExpectResult(pid.update(-3.0f, 0.5f), PidStatus::Ok, 6.0f);
  EXPECT_EQ(pid.integral(), 6.0f);
}

// TC-CTRL-009
TEST(Pid, TC_CTRL_009_d_term_first_step_after_configure_is_zero) {
  Pid pid = MakeConfigured(0.0f, 0.0f, 2.0f);

  ExpectResult(pid.update(5.0f, 0.5f), PidStatus::Ok, 0.0f);
}

// TC-CTRL-010
TEST(Pid, TC_CTRL_010_d_term_constant_error_after_first_step_is_zero) {
  Pid pid = MakeConfigured(0.0f, 0.0f, 2.0f);

  ExpectResult(pid.update(3.0f, 0.5f), PidStatus::Ok, 0.0f);
  ExpectResult(pid.update(3.0f, 0.5f), PidStatus::Ok, 0.0f);
  ExpectResult(pid.update(3.0f, 0.5f), PidStatus::Ok, 0.0f);
}

// TC-CTRL-011
TEST(Pid, TC_CTRL_011_d_term_ramp_error_equals_kd_slope) {
  Pid pid = MakeConfigured(0.0f, 0.0f, 2.0f);

  ExpectResult(pid.update(0.0f, 0.5f), PidStatus::Ok, 0.0f);
  ExpectResult(pid.update(1.0f, 0.5f), PidStatus::Ok, 4.0f);
  ExpectResult(pid.update(2.0f, 0.5f), PidStatus::Ok, 4.0f);
  ExpectResult(pid.update(3.0f, 0.5f), PidStatus::Ok, 4.0f);
}

// TC-CTRL-012
TEST(Pid, TC_CTRL_012_d_term_uses_dt_of_current_call) {
  Pid pid = MakeConfigured(0.0f, 0.0f, 2.0f);

  ExpectResult(pid.update(0.0f, 0.5f), PidStatus::Ok, 0.0f);
  ExpectResult(pid.update(1.0f, 0.5f), PidStatus::Ok, 4.0f);
  ExpectResult(pid.update(2.0f, 0.25f), PidStatus::Ok, 8.0f);
}

// TC-CTRL-013
TEST(Pid, TC_CTRL_013_combined_terms_sum_p_i_and_d) {
  Pid pid = MakeConfigured(2.0f, 4.0f, 1.0f);

  ExpectResult(pid.update(1.0f, 0.5f), PidStatus::Ok, 4.0f);
  EXPECT_EQ(pid.integral(), 2.0f);

  ExpectResult(pid.update(3.0f, 0.5f), PidStatus::Ok, 18.0f);
  EXPECT_EQ(pid.integral(), 8.0f);

  ExpectResult(pid.update(3.0f, 0.5f), PidStatus::Ok, 20.0f);
  EXPECT_EQ(pid.integral(), 14.0f);
}

// TC-CTRL-014
TEST(Pid, TC_CTRL_014_all_gains_zero_outputs_zero_and_no_integral) {
  Pid pid = MakeConfigured(0.0f, 0.0f, 0.0f);

  ExpectResult(pid.update(5.0f, 0.5f), PidStatus::Ok, 0.0f);
  ExpectResult(pid.update(-5.0f, 0.5f), PidStatus::Ok, 0.0f);
  EXPECT_EQ(pid.integral(), 0.0f);
}

// TC-CTRL-015
TEST(Pid, TC_CTRL_015_reset_clears_state_and_keeps_gains) {
  Pid pid = MakeConfigured(2.0f, 4.0f, 1.0f);
  pid.update(1.0f, 0.5f);
  pid.update(3.0f, 0.5f);

  pid.reset();

  EXPECT_EQ(pid.integral(), 0.0f);
  ExpectGains(pid, 2.0f, 4.0f, 1.0f);
  EXPECT_TRUE(pid.configured());
  ExpectResult(pid.update(3.0f, 0.5f), PidStatus::Ok, 12.0f);
}

// TC-CTRL-016
TEST(Pid, TC_CTRL_016_reset_on_unconfigured_stays_unconfigured) {
  Pid pid;

  pid.reset();

  EXPECT_FALSE(pid.configured());
  ExpectResult(pid.update(1.0f, 0.5f), PidStatus::NotConfigured, 0.0f);
}

// TC-CTRL-031
TEST(Pid, TC_CTRL_031_reset_after_rejected_configure_stays_unconfigured) {
  Pid pid;
  EXPECT_EQ(pid.configure(PidGains{kNaN, 0.0f, 0.0f}),
            PidStatus::InvalidConfig);

  pid.reset();

  EXPECT_FALSE(pid.configured());
  ExpectResult(pid.update(1.0f, 0.5f), PidStatus::NotConfigured, 0.0f);
}

// TC-CTRL-017
TEST(Pid, TC_CTRL_017_configure_fresh_stores_gains_and_configures) {
  Pid pid;

  EXPECT_EQ(pid.configure(PidGains{2.0f, 4.0f, 1.0f}), PidStatus::Ok);

  EXPECT_TRUE(pid.configured());
  ExpectGains(pid, 2.0f, 4.0f, 1.0f);
  EXPECT_EQ(pid.integral(), 0.0f);
}

// TC-CTRL-018
TEST(Pid, TC_CTRL_018_configure_replaces_gains_and_clears_history) {
  Pid pid = MakeConfigured(2.0f, 4.0f, 1.0f);
  pid.update(1.0f, 0.5f);
  pid.update(3.0f, 0.5f);

  EXPECT_EQ(pid.configure(PidGains{1.0f, 0.0f, 2.0f}), PidStatus::Ok);

  ExpectGains(pid, 1.0f, 0.0f, 2.0f);
  EXPECT_EQ(pid.integral(), 0.0f);
  ExpectResult(pid.update(5.0f, 0.5f), PidStatus::Ok, 5.0f);
}

// TC-CTRL-019
TEST(Pid, TC_CTRL_019_configure_nonfinite_kp_on_fresh_pid_is_rejected) {
  Pid pid;

  EXPECT_EQ(pid.configure(PidGains{kNaN, 0.0f, 0.0f}),
            PidStatus::InvalidConfig);

  EXPECT_FALSE(pid.configured());
  ExpectResult(pid.update(1.0f, 0.5f), PidStatus::NotConfigured, 0.0f);
}

// TC-CTRL-032
TEST(Pid, TC_CTRL_032_configure_nonfinite_ki_on_fresh_pid_is_rejected) {
  Pid pid;

  EXPECT_EQ(pid.configure(PidGains{0.0f, kInf, 0.0f}),
            PidStatus::InvalidConfig);

  EXPECT_FALSE(pid.configured());
  ExpectResult(pid.update(1.0f, 0.5f), PidStatus::NotConfigured, 0.0f);
}

// TC-CTRL-033
TEST(Pid, TC_CTRL_033_configure_nonfinite_kd_on_fresh_pid_is_rejected) {
  Pid pid;

  EXPECT_EQ(pid.configure(PidGains{0.0f, 0.0f, -kInf}),
            PidStatus::InvalidConfig);

  EXPECT_FALSE(pid.configured());
  ExpectResult(pid.update(1.0f, 0.5f), PidStatus::NotConfigured, 0.0f);
}

// TC-CTRL-020
TEST(Pid, TC_CTRL_020_configure_nonfinite_keeps_gains_and_state) {
  Pid pid = MakeConfigured(2.0f, 4.0f, 1.0f);
  pid.update(1.0f, 0.5f);
  pid.update(3.0f, 0.5f);

  EXPECT_EQ(pid.configure(PidGains{kNaN, 4.0f, 1.0f}),
            PidStatus::InvalidConfig);

  ExpectGains(pid, 2.0f, 4.0f, 1.0f);
  EXPECT_EQ(pid.integral(), 8.0f);
  EXPECT_TRUE(pid.configured());
  ExpectResult(pid.update(5.0f, 0.5f), PidStatus::Ok, 32.0f);
}

// TC-CTRL-021
TEST(Pid, TC_CTRL_021_update_before_configure_is_not_configured) {
  Pid pid;
  EXPECT_FALSE(pid.configured());

  ExpectResult(pid.update(1.0f, 0.5f), PidStatus::NotConfigured, 0.0f);

  EXPECT_FALSE(pid.configured());
}

// TC-CTRL-022
TEST(Pid, TC_CTRL_022_update_zero_or_negative_zero_dt_is_invalid_dt) {
  Pid pid = MakeConfigured(2.0f, 4.0f, 1.0f);

  ExpectResult(pid.update(1.0f, 0.0f), PidStatus::InvalidDt, 0.0f);
  ExpectResult(pid.update(1.0f, -0.0f), PidStatus::InvalidDt, 0.0f);
  EXPECT_EQ(pid.integral(), 0.0f);
}

// TC-CTRL-023
TEST(Pid, TC_CTRL_023_update_negative_dt_is_invalid_dt) {
  Pid pid = MakeConfigured(2.0f, 4.0f, 1.0f);

  ExpectResult(pid.update(1.0f, -0.5f), PidStatus::InvalidDt, 0.0f);
  EXPECT_EQ(pid.integral(), 0.0f);
}

// TC-CTRL-024
TEST(Pid, TC_CTRL_024_update_nonfinite_dt_is_invalid_dt) {
  Pid pid = MakeConfigured(2.0f, 4.0f, 1.0f);

  ExpectResult(pid.update(1.0f, kNaN), PidStatus::InvalidDt, 0.0f);
  ExpectResult(pid.update(1.0f, kInf), PidStatus::InvalidDt, 0.0f);
  ExpectResult(pid.update(1.0f, -kInf), PidStatus::InvalidDt, 0.0f);
  EXPECT_EQ(pid.integral(), 0.0f);
}

// TC-CTRL-025
TEST(Pid, TC_CTRL_025_update_smallest_positive_normal_dt_is_accepted) {
  Pid pid = MakeConfigured(2.0f, 0.0f, 0.0f);

  ExpectResult(pid.update(3.0f, kMinNormal), PidStatus::Ok, 6.0f);
}

// TC-CTRL-026
TEST(Pid, TC_CTRL_026_update_nonfinite_error_is_invalid_input) {
  Pid pid = MakeConfigured(2.0f, 4.0f, 1.0f);

  ExpectResult(pid.update(kNaN, 0.5f), PidStatus::InvalidInput, 0.0f);
  ExpectResult(pid.update(kInf, 0.5f), PidStatus::InvalidInput, 0.0f);
  ExpectResult(pid.update(-kInf, 0.5f), PidStatus::InvalidInput, 0.0f);
  EXPECT_EQ(pid.integral(), 0.0f);
}

// TC-CTRL-027
TEST(Pid, TC_CTRL_027_p_term_overflow_is_numeric_fault) {
  Pid pid = MakeConfigured(kMax, 0.0f, 0.0f);

  ExpectResult(pid.update(2.0f, 0.5f), PidStatus::NumericFault, 0.0f);
  EXPECT_EQ(pid.integral(), 0.0f);
}

// TC-CTRL-034
TEST(Pid, TC_CTRL_034_i_term_overflow_is_numeric_fault) {
  Pid pid = MakeConfigured(0.0f, kMax, 0.0f);

  ExpectResult(pid.update(4.0f, 0.5f), PidStatus::NumericFault, 0.0f);
  EXPECT_EQ(pid.integral(), 0.0f);
}

// TC-CTRL-035
TEST(Pid, TC_CTRL_035_d_term_overflow_does_not_advance_previous_error) {
  Pid pid = MakeConfigured(0.0f, 0.0f, kMax);

  ExpectResult(pid.update(0.0f, 0.5f), PidStatus::Ok, 0.0f);
  ExpectResult(pid.update(1.0f, 0.5f), PidStatus::NumericFault, 0.0f);
  ExpectResult(pid.update(1.0f, 1.0f), PidStatus::Ok, kMax);
  EXPECT_EQ(pid.integral(), 0.0f);
}

// TC-CTRL-028
TEST(Pid, TC_CTRL_028_sum_overflow_with_finite_terms_is_numeric_fault) {
  Pid pid = MakeConfigured(kMax, kMax, 0.0f);

  ExpectResult(pid.update(1.0f, 1.0f), PidStatus::NumericFault, 0.0f);
  EXPECT_EQ(pid.integral(), 0.0f);
}

// TC-CTRL-029
TEST(Pid, TC_CTRL_029_rejected_dt_call_leaves_state_unchanged) {
  Pid pid = MakeConfigured(2.0f, 4.0f, 1.0f);
  pid.update(1.0f, 0.5f);

  ExpectResult(pid.update(1.0f, 0.0f), PidStatus::InvalidDt, 0.0f);
  EXPECT_EQ(pid.integral(), 2.0f);

  ExpectResult(pid.update(3.0f, 0.5f), PidStatus::Ok, 18.0f);
}

// TC-CTRL-036
TEST(Pid, TC_CTRL_036_rejected_input_call_leaves_state_unchanged) {
  Pid pid = MakeConfigured(2.0f, 4.0f, 1.0f);
  pid.update(1.0f, 0.5f);

  ExpectResult(pid.update(kNaN, 0.5f), PidStatus::InvalidInput, 0.0f);
  EXPECT_EQ(pid.integral(), 2.0f);

  ExpectResult(pid.update(3.0f, 0.5f), PidStatus::Ok, 18.0f);
}

// TC-CTRL-037
TEST(Pid, TC_CTRL_037_rejected_numeric_fault_call_leaves_state_unchanged) {
  Pid pid = MakeConfigured(2.0f, 4.0f, 1.0f);
  pid.update(1.0f, 0.5f);

  ExpectResult(pid.update(kMax, 0.5f), PidStatus::NumericFault, 0.0f);
  EXPECT_EQ(pid.integral(), 2.0f);

  ExpectResult(pid.update(3.0f, 0.5f), PidStatus::Ok, 18.0f);
}

// TC-CTRL-038
TEST(Pid, TC_CTRL_038_rejected_dt_before_first_step_keeps_d_term_zero) {
  Pid pid = MakeConfigured(2.0f, 4.0f, 1.0f);

  ExpectResult(pid.update(1.0f, 0.0f), PidStatus::InvalidDt, 0.0f);

  ExpectResult(pid.update(3.0f, 0.5f), PidStatus::Ok, 12.0f);
  EXPECT_EQ(pid.integral(), 6.0f);
}

// TC-CTRL-039
TEST(Pid, TC_CTRL_039_rejected_input_before_first_step_keeps_d_term_zero) {
  Pid pid = MakeConfigured(2.0f, 4.0f, 1.0f);

  ExpectResult(pid.update(kNaN, 0.5f), PidStatus::InvalidInput, 0.0f);

  ExpectResult(pid.update(3.0f, 0.5f), PidStatus::Ok, 12.0f);
  EXPECT_EQ(pid.integral(), 6.0f);
}

// TC-CTRL-030
TEST(Pid, TC_CTRL_030_unconfigured_reports_not_configured_before_dt_and_error) {
  Pid pid;

  ExpectResult(pid.update(kNaN, 0.0f), PidStatus::NotConfigured, 0.0f);
}

// TC-CTRL-040
TEST(Pid, TC_CTRL_040_configured_reports_invalid_dt_before_invalid_input) {
  Pid pid = MakeConfigured(2.0f, 4.0f, 1.0f);

  ExpectResult(pid.update(kNaN, 0.0f), PidStatus::InvalidDt, 0.0f);
}
