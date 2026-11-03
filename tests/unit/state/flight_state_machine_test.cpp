#include <cstdint>
#include <set>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "flight_state.hpp"
#include "fakes/recording_state_guard.hpp"
#include "flight_state_support.hpp"

namespace {

using dfcs_test::forceState;
using dfcs_test::isPermitted;
using dfcs_test::kAllEvents;
using dfcs_test::kAllStates;
using dfcs_test::kDeniedReason;
using dfcs_test::oneAfterLastEvent;
using dfcs_test::wireAllGuards;

// Denied guard: outcome, reason, unchanged state, and one call with (from, event).
void expectGuardDeniedOnce(const DispatchResult& result, const FlightStateMachine& machine,
                           const RecordingStateGuard& guard, FlightState from,
                           FlightEvent event) {
  EXPECT_EQ(result.outcome, Outcome::RejectedGuardDenied);
  EXPECT_EQ(result.reason, kDeniedReason);
  EXPECT_EQ(machine.state(), from);
  ASSERT_EQ(guard.calls().size(), 1U);
  EXPECT_EQ(guard.calls()[0].state, from);
  EXPECT_EQ(guard.calls()[0].event, event);
}

}  // namespace

// TC-STATE-002
TEST(FlightStateMachine, TC_STATE_002_boot_reset_completed_moves_to_initializing) {
  FlightStateMachine machine;

  const DispatchResult result = machine.dispatch(FlightEvent::ResetCompleted);

  EXPECT_EQ(result.outcome, Outcome::Accepted);
  EXPECT_EQ(result.from, FlightState::Boot);
  EXPECT_EQ(result.to, FlightState::Initializing);
  EXPECT_EQ(result.behavior, Behavior::KeepInhibited);
  EXPECT_NE(result.row, -1);
  EXPECT_EQ(machine.state(), FlightState::Initializing);
}

// TC-STATE-003
TEST(FlightStateMachine,
     TC_STATE_003_non_permitted_pairs_rejected_without_change_or_guard_call) {
  int checked = 0;
  for (FlightState from : kAllStates) {
    for (FlightEvent event : kAllEvents) {
      if (isPermitted(from, event)) {
        continue;
      }
      SCOPED_TRACE(testing::Message() << "from=" << static_cast<int>(from)
                                      << " event=" << static_cast<int>(event));
      RecordingStateGuard allow(true, 0);
      FlightStateMachine machine;
      wireAllGuards(machine, allow);
      forceState(machine, from);
      allow.clearCalls();

      const DispatchResult result = machine.dispatch(event);

      EXPECT_EQ(result.outcome, Outcome::RejectedNotPermitted);
      EXPECT_EQ(result.row, -1);
      EXPECT_EQ(machine.state(), from);
      EXPECT_TRUE(allow.calls().empty());
      ++checked;
    }
  }
  EXPECT_EQ(checked, 45);
}

// TC-STATE-008
TEST(FlightStateMachine, TC_STATE_008_initializing_init_passed_moves_to_disarmed) {
  FlightStateMachine machine;
  forceState(machine, FlightState::Initializing);

  const DispatchResult result = machine.dispatch(FlightEvent::InitPassed);

  EXPECT_EQ(result.outcome, Outcome::Accepted);
  EXPECT_EQ(result.from, FlightState::Initializing);
  EXPECT_EQ(result.to, FlightState::Disarmed);
  EXPECT_EQ(result.behavior, Behavior::KeepInhibited);
  EXPECT_EQ(machine.state(), FlightState::Disarmed);
}

// TC-STATE-009
TEST(FlightStateMachine, TC_STATE_009_initializing_init_failed_moves_to_fault) {
  FlightStateMachine machine;
  forceState(machine, FlightState::Initializing);

  const DispatchResult result = machine.dispatch(FlightEvent::InitFailed);

  EXPECT_EQ(result.outcome, Outcome::Accepted);
  EXPECT_EQ(result.from, FlightState::Initializing);
  EXPECT_EQ(result.to, FlightState::Fault);
  EXPECT_EQ(result.behavior, Behavior::KeepInhibitedReportFault);
  EXPECT_EQ(machine.state(), FlightState::Fault);
}

// TC-STATE-010
TEST(FlightStateMachine, TC_STATE_010_disarmed_arm_request_with_allowing_guard_arms) {
  RecordingStateGuard allowArm(true, 0);
  FlightStateMachine machine;
  machine.setGuard(GuardId::Arm, &allowArm);
  forceState(machine, FlightState::Disarmed);

  const DispatchResult result = machine.dispatch(FlightEvent::ArmRequest);

  EXPECT_EQ(result.outcome, Outcome::Accepted);
  EXPECT_EQ(result.from, FlightState::Disarmed);
  EXPECT_EQ(result.to, FlightState::Armed);
  EXPECT_EQ(result.behavior, Behavior::EnableControlPathUnderLimits);
  EXPECT_EQ(machine.state(), FlightState::Armed);
  ASSERT_EQ(allowArm.calls().size(), 1U);
  EXPECT_EQ(allowArm.calls()[0].state, FlightState::Disarmed);
  EXPECT_EQ(allowArm.calls()[0].event, FlightEvent::ArmRequest);
}

// TC-STATE-011
TEST(FlightStateMachine, TC_STATE_011_armed_disarm_request_moves_to_disarmed_unguarded) {
  RecordingStateGuard allowArm(true, 0);
  FlightStateMachine machine;
  machine.setGuard(GuardId::Arm, &allowArm);
  forceState(machine, FlightState::Armed);
  allowArm.clearCalls();

  const DispatchResult result = machine.dispatch(FlightEvent::DisarmRequest);

  EXPECT_EQ(result.outcome, Outcome::Accepted);
  EXPECT_EQ(result.from, FlightState::Armed);
  EXPECT_EQ(result.to, FlightState::Disarmed);
  EXPECT_EQ(result.behavior, Behavior::InhibitNormalCommands);
  EXPECT_EQ(machine.state(), FlightState::Disarmed);
  EXPECT_TRUE(allowArm.calls().empty());
}

// TC-STATE-012
TEST(FlightStateMachine, TC_STATE_012_armed_critical_fault_to_failsafe) {
  RecordingStateGuard allowArm(true, 0);
  FlightStateMachine machine;
  machine.setGuard(GuardId::Arm, &allowArm);
  forceState(machine, FlightState::Armed);

  const DispatchResult result = machine.dispatch(FlightEvent::CriticalFaultToFailsafe);

  EXPECT_EQ(result.outcome, Outcome::Accepted);
  EXPECT_EQ(result.from, FlightState::Armed);
  EXPECT_EQ(result.to, FlightState::Failsafe);
  EXPECT_EQ(result.behavior, Behavior::ApplyDefinedSafetyResponse);
  EXPECT_EQ(machine.state(), FlightState::Failsafe);
}

// TC-STATE-013
TEST(FlightStateMachine, TC_STATE_013_armed_critical_fault_to_fault) {
  RecordingStateGuard allowArm(true, 0);
  FlightStateMachine machine;
  machine.setGuard(GuardId::Arm, &allowArm);
  forceState(machine, FlightState::Armed);

  const DispatchResult result = machine.dispatch(FlightEvent::CriticalFaultToFault);

  EXPECT_EQ(result.outcome, Outcome::Accepted);
  EXPECT_EQ(result.from, FlightState::Armed);
  EXPECT_EQ(result.to, FlightState::Fault);
  EXPECT_EQ(result.behavior, Behavior::ApplyDefinedSafetyResponse);
  EXPECT_EQ(machine.state(), FlightState::Fault);
}

// TC-STATE-014
TEST(FlightStateMachine, TC_STATE_014_failsafe_recovery_accepted_disarms_never_arms) {
  RecordingStateGuard allow(true, 0);
  FlightStateMachine machine;
  machine.setGuard(GuardId::Arm, &allow);
  machine.setGuard(GuardId::Recovery, &allow);
  forceState(machine, FlightState::Failsafe);
  allow.clearCalls();

  const DispatchResult result = machine.dispatch(FlightEvent::RecoveryAccepted);

  EXPECT_EQ(result.outcome, Outcome::Accepted);
  EXPECT_EQ(result.from, FlightState::Failsafe);
  EXPECT_EQ(result.to, FlightState::Disarmed);
  EXPECT_EQ(result.behavior, Behavior::NeverAutoRearm);
  EXPECT_EQ(machine.state(), FlightState::Disarmed);
  EXPECT_NE(machine.state(), FlightState::Armed);
  ASSERT_EQ(allow.calls().size(), 1U);
  EXPECT_EQ(allow.calls()[0].state, FlightState::Failsafe);
  EXPECT_EQ(allow.calls()[0].event, FlightEvent::RecoveryAccepted);
}

// TC-STATE-015
TEST(FlightStateMachine, TC_STATE_015_fault_service_reset_moves_to_boot) {
  RecordingStateGuard allow(true, 0);
  FlightStateMachine machine;
  machine.setGuard(GuardId::ServiceReset, &allow);
  forceState(machine, FlightState::Fault);
  allow.clearCalls();

  const DispatchResult result = machine.dispatch(FlightEvent::ServiceReset);

  EXPECT_EQ(result.outcome, Outcome::Accepted);
  EXPECT_EQ(result.from, FlightState::Fault);
  EXPECT_EQ(result.to, FlightState::Boot);
  EXPECT_EQ(result.behavior, Behavior::KeepInhibited);
  EXPECT_EQ(machine.state(), FlightState::Boot);
  ASSERT_EQ(allow.calls().size(), 1U);
  EXPECT_EQ(allow.calls()[0].state, FlightState::Fault);
  EXPECT_EQ(allow.calls()[0].event, FlightEvent::ServiceReset);
}

// TC-STATE-016
TEST(FlightStateMachine, TC_STATE_016_new_machine_in_boot_rejects_arm_request) {
  FlightStateMachine machine;
  EXPECT_EQ(machine.state(), FlightState::Boot);

  const DispatchResult result = machine.dispatch(FlightEvent::ArmRequest);

  EXPECT_EQ(result.outcome, Outcome::RejectedNotPermitted);
  EXPECT_EQ(machine.state(), FlightState::Boot);
}

// TC-STATE-017
TEST(FlightStateMachine, TC_STATE_017_only_permitted_pairs_accepted_with_distinct_rows) {
  RecordingStateGuard allow(true, 0);
  std::set<int> rows;
  int acceptedCount = 0;
  for (FlightState from : kAllStates) {
    for (FlightEvent event : kAllEvents) {
      SCOPED_TRACE(testing::Message() << "from=" << static_cast<int>(from)
                                      << " event=" << static_cast<int>(event));
      FlightStateMachine machine;
      wireAllGuards(machine, allow);
      forceState(machine, from);

      const DispatchResult result = machine.dispatch(event);

      if (isPermitted(from, event)) {
        EXPECT_EQ(result.outcome, Outcome::Accepted);
        EXPECT_EQ(result.from, from);
        EXPECT_GE(result.row, 0);
        rows.insert(static_cast<int>(result.row));
        ++acceptedCount;
      } else {
        EXPECT_EQ(result.outcome, Outcome::RejectedNotPermitted);
      }
    }
  }
  EXPECT_EQ(acceptedCount, 9);
  EXPECT_EQ(rows.size(), 9U);
}

// TC-STATE-018
TEST(FlightStateMachine, TC_STATE_018_only_disarmed_arm_request_enters_armed) {
  RecordingStateGuard allow(true, 0);
  std::vector<std::pair<FlightState, FlightEvent>> armedEntries;
  for (FlightState from : kAllStates) {
    for (FlightEvent event : kAllEvents) {
      SCOPED_TRACE(testing::Message() << "from=" << static_cast<int>(from)
                                      << " event=" << static_cast<int>(event));
      FlightStateMachine machine;
      wireAllGuards(machine, allow);
      forceState(machine, from);

      const DispatchResult result = machine.dispatch(event);
      const FlightState after = machine.state();

      if (from != FlightState::Armed && after == FlightState::Armed) {
        armedEntries.emplace_back(from, event);
        EXPECT_EQ(result.outcome, Outcome::Accepted);
      }
      if (from == FlightState::Failsafe || from == FlightState::Fault) {
        EXPECT_NE(after, FlightState::Armed);
      }
    }
  }
  ASSERT_EQ(armedEntries.size(), 1U);
  EXPECT_EQ(armedEntries[0].first, FlightState::Disarmed);
  EXPECT_EQ(armedEntries[0].second, FlightEvent::ArmRequest);
}

// TC-STATE-019
TEST(FlightStateMachine, TC_STATE_019_arm_guard_denial_rejects_and_keeps_disarmed) {
  RecordingStateGuard denyArm(false, kDeniedReason);
  FlightStateMachine machine;
  machine.setGuard(GuardId::Arm, &denyArm);
  forceState(machine, FlightState::Disarmed);

  const DispatchResult result = machine.dispatch(FlightEvent::ArmRequest);

  expectGuardDeniedOnce(result, machine, denyArm, FlightState::Disarmed, FlightEvent::ArmRequest);
}

// TC-STATE-020
TEST(FlightStateMachine, TC_STATE_020_guard_denial_on_d4_d8_d9_keeps_state) {
  // D4: DISARMED + ArmRequest.
  RecordingStateGuard denyArm(false, kDeniedReason);
  FlightStateMachine d4;
  d4.setGuard(GuardId::Arm, &denyArm);
  forceState(d4, FlightState::Disarmed);
  const DispatchResult resultD4 = d4.dispatch(FlightEvent::ArmRequest);
  expectGuardDeniedOnce(resultD4, d4, denyArm, FlightState::Disarmed, FlightEvent::ArmRequest);

  // D8: FAILSAFE + RecoveryAccepted.
  RecordingStateGuard allowArm(true, 0);
  RecordingStateGuard denyRecovery(false, kDeniedReason);
  FlightStateMachine d8;
  d8.setGuard(GuardId::Arm, &allowArm);
  d8.setGuard(GuardId::Recovery, &denyRecovery);
  forceState(d8, FlightState::Failsafe);
  denyRecovery.clearCalls();
  const DispatchResult resultD8 = d8.dispatch(FlightEvent::RecoveryAccepted);
  expectGuardDeniedOnce(resultD8, d8, denyRecovery, FlightState::Failsafe,
                        FlightEvent::RecoveryAccepted);

  // D9: FAULT + ServiceReset.
  RecordingStateGuard denyService(false, kDeniedReason);
  FlightStateMachine d9;
  d9.setGuard(GuardId::ServiceReset, &denyService);
  forceState(d9, FlightState::Fault);
  const DispatchResult resultD9 = d9.dispatch(FlightEvent::ServiceReset);
  expectGuardDeniedOnce(resultD9, d9, denyService, FlightState::Fault, FlightEvent::ServiceReset);
}

// TC-STATE-021
TEST(FlightStateMachine, TC_STATE_021_unwired_guards_fail_closed) {
  // D4: no guard wired.
  FlightStateMachine d4;
  forceState(d4, FlightState::Disarmed);
  const DispatchResult resultD4 = d4.dispatch(FlightEvent::ArmRequest);
  EXPECT_EQ(resultD4.outcome, Outcome::RejectedGuardDenied);
  EXPECT_EQ(resultD4.reason, 0U);
  EXPECT_EQ(d4.state(), FlightState::Disarmed);

  // D8: Arm allowed, Recovery left unwired.
  RecordingStateGuard allowArm(true, 0);
  FlightStateMachine d8;
  d8.setGuard(GuardId::Arm, &allowArm);
  forceState(d8, FlightState::Failsafe);
  allowArm.clearCalls();
  const DispatchResult resultD8 = d8.dispatch(FlightEvent::RecoveryAccepted);
  EXPECT_EQ(resultD8.outcome, Outcome::RejectedGuardDenied);
  EXPECT_EQ(resultD8.reason, 0U);
  EXPECT_EQ(d8.state(), FlightState::Failsafe);
  EXPECT_TRUE(allowArm.calls().empty());

  // D9: no guard wired.
  FlightStateMachine d9;
  forceState(d9, FlightState::Fault);
  const DispatchResult resultD9 = d9.dispatch(FlightEvent::ServiceReset);
  EXPECT_EQ(resultD9.outcome, Outcome::RejectedGuardDenied);
  EXPECT_EQ(resultD9.reason, 0U);
  EXPECT_EQ(d9.state(), FlightState::Fault);
}

// TC-STATE-022
TEST(FlightStateMachine, TC_STATE_022_out_of_range_event_rejected_invalid_input) {
  RecordingStateGuard allowArm(true, 0);
  FlightStateMachine machine;
  machine.setGuard(GuardId::Arm, &allowArm);
  forceState(machine, FlightState::Disarmed);
  allowArm.clearCalls();

  const DispatchResult result = machine.dispatch(oneAfterLastEvent());

  EXPECT_EQ(result.outcome, Outcome::RejectedInvalidInput);
  EXPECT_EQ(machine.state(), FlightState::Disarmed);
  EXPECT_TRUE(allowArm.calls().empty());
}

// TC-STATE-023
TEST(FlightStateMachine, TC_STATE_023_event_range_upper_boundary) {
  FlightStateMachine machine;
  forceState(machine, FlightState::Disarmed);

  const DispatchResult lastEnumerator = machine.dispatch(FlightEvent::ServiceReset);
  EXPECT_EQ(lastEnumerator.outcome, Outcome::RejectedNotPermitted);
  EXPECT_EQ(machine.state(), FlightState::Disarmed);

  const DispatchResult oneAfter = machine.dispatch(oneAfterLastEvent());
  EXPECT_EQ(oneAfter.outcome, Outcome::RejectedInvalidInput);
  EXPECT_EQ(machine.state(), FlightState::Disarmed);
}

// TC-STATE-024
TEST(FlightStateMachine,
     TC_STATE_024_failsafe_only_recovery_leaves_and_never_reaches_armed) {
  int checked = 0;
  for (FlightEvent event : kAllEvents) {
    if (event == FlightEvent::RecoveryAccepted) {
      continue;
    }
    SCOPED_TRACE(testing::Message() << "event=" << static_cast<int>(event));
    RecordingStateGuard allow(true, 0);
    FlightStateMachine machine;
    wireAllGuards(machine, allow);
    forceState(machine, FlightState::Failsafe);
    allow.clearCalls();

    const DispatchResult result = machine.dispatch(event);

    EXPECT_EQ(result.outcome, Outcome::RejectedNotPermitted);
    EXPECT_EQ(machine.state(), FlightState::Failsafe);
    EXPECT_TRUE(allow.calls().empty());
    ++checked;
  }
  EXPECT_EQ(checked, 8);
}

// TC-STATE-025
TEST(FlightStateMachine, TC_STATE_025_fault_only_service_reset_leaves_and_never_reaches_armed) {
  int checked = 0;
  for (FlightEvent event : kAllEvents) {
    if (event == FlightEvent::ServiceReset) {
      continue;
    }
    SCOPED_TRACE(testing::Message() << "event=" << static_cast<int>(event));
    RecordingStateGuard allow(true, 0);
    FlightStateMachine machine;
    wireAllGuards(machine, allow);
    forceState(machine, FlightState::Fault);
    allow.clearCalls();

    const DispatchResult result = machine.dispatch(event);

    EXPECT_EQ(result.outcome, Outcome::RejectedNotPermitted);
    EXPECT_EQ(machine.state(), FlightState::Fault);
    EXPECT_TRUE(allow.calls().empty());
    ++checked;
  }
  EXPECT_EQ(checked, 8);
}

// TC-STATE-026
TEST(FlightStateMachine, TC_STATE_026_rejection_outcomes_are_distinct) {
  RecordingStateGuard denyArm(false, kDeniedReason);
  FlightStateMachine machine;
  machine.setGuard(GuardId::Arm, &denyArm);
  forceState(machine, FlightState::Disarmed);

  const DispatchResult notPermitted = machine.dispatch(FlightEvent::DisarmRequest);
  const DispatchResult guardDenied = machine.dispatch(FlightEvent::ArmRequest);
  const DispatchResult invalidInput = machine.dispatch(oneAfterLastEvent());

  EXPECT_EQ(notPermitted.outcome, Outcome::RejectedNotPermitted);
  EXPECT_EQ(guardDenied.outcome, Outcome::RejectedGuardDenied);
  EXPECT_EQ(invalidInput.outcome, Outcome::RejectedInvalidInput);
  EXPECT_NE(notPermitted.outcome, guardDenied.outcome);
  EXPECT_NE(notPermitted.outcome, invalidInput.outcome);
  EXPECT_NE(guardDenied.outcome, invalidInput.outcome);
  EXPECT_NE(notPermitted.outcome, Outcome::Accepted);
  EXPECT_NE(guardDenied.outcome, Outcome::Accepted);
  EXPECT_NE(invalidInput.outcome, Outcome::Accepted);
  EXPECT_EQ(machine.state(), FlightState::Disarmed);
}

// TC-STATE-027
TEST(FlightStateMachine, TC_STATE_027_arm_rejected_before_init_completes) {
  RecordingStateGuard allowArm(true, 0);
  FlightStateMachine machine;
  machine.setGuard(GuardId::Arm, &allowArm);

  const DispatchResult inBoot = machine.dispatch(FlightEvent::ArmRequest);
  EXPECT_EQ(inBoot.outcome, Outcome::RejectedNotPermitted);
  EXPECT_EQ(machine.state(), FlightState::Boot);

  const DispatchResult reset = machine.dispatch(FlightEvent::ResetCompleted);
  ASSERT_EQ(reset.outcome, Outcome::Accepted);
  EXPECT_EQ(machine.state(), FlightState::Initializing);

  const DispatchResult inInitializing = machine.dispatch(FlightEvent::ArmRequest);
  EXPECT_EQ(inInitializing.outcome, Outcome::RejectedNotPermitted);
  EXPECT_EQ(machine.state(), FlightState::Initializing);
  EXPECT_TRUE(allowArm.calls().empty());
}

// TC-STATE-029
TEST(FlightStateMachine, TC_STATE_029_deny_all_guard_on_guarded_rows_rejects) {
  // D4: DISARMED + ArmRequest.
  DenyAllGuard denyArm;
  FlightStateMachine d4;
  d4.setGuard(GuardId::Arm, &denyArm);
  forceState(d4, FlightState::Disarmed);
  const DispatchResult resultD4 = d4.dispatch(FlightEvent::ArmRequest);
  EXPECT_EQ(resultD4.outcome, Outcome::RejectedGuardDenied);
  EXPECT_EQ(resultD4.reason, 0U);
  EXPECT_EQ(d4.state(), FlightState::Disarmed);

  // D8: FAILSAFE + RecoveryAccepted, Arm allowed so FAILSAFE is reachable.
  RecordingStateGuard allowArm(true, 0);
  DenyAllGuard denyRecovery;
  FlightStateMachine d8;
  d8.setGuard(GuardId::Arm, &allowArm);
  d8.setGuard(GuardId::Recovery, &denyRecovery);
  forceState(d8, FlightState::Failsafe);
  const DispatchResult resultD8 = d8.dispatch(FlightEvent::RecoveryAccepted);
  EXPECT_EQ(resultD8.outcome, Outcome::RejectedGuardDenied);
  EXPECT_EQ(resultD8.reason, 0U);
  EXPECT_EQ(d8.state(), FlightState::Failsafe);

  // D9: FAULT + ServiceReset.
  DenyAllGuard denyService;
  FlightStateMachine d9;
  d9.setGuard(GuardId::ServiceReset, &denyService);
  forceState(d9, FlightState::Fault);
  const DispatchResult resultD9 = d9.dispatch(FlightEvent::ServiceReset);
  EXPECT_EQ(resultD9.outcome, Outcome::RejectedGuardDenied);
  EXPECT_EQ(resultD9.reason, 0U);
  EXPECT_EQ(d9.state(), FlightState::Fault);
}
