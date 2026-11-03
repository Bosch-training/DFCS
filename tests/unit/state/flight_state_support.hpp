#pragma once

#include <array>
#include <cstdint>

#include <gtest/gtest.h>

#include "flight_state.hpp"

namespace dfcs_test {

inline constexpr std::array<FlightState, 6> kAllStates = {
    FlightState::Boot,     FlightState::Initializing, FlightState::Disarmed,
    FlightState::Armed,    FlightState::Failsafe,     FlightState::Fault};

inline constexpr std::array<FlightEvent, 9> kAllEvents = {
    FlightEvent::ResetCompleted,         FlightEvent::InitPassed,
    FlightEvent::InitFailed,             FlightEvent::ArmRequest,
    FlightEvent::DisarmRequest,          FlightEvent::CriticalFaultToFailsafe,
    FlightEvent::CriticalFaultToFault,   FlightEvent::RecoveryAccepted,
    FlightEvent::ServiceReset};

// Opaque guard reason chosen by the test (code set owned by STA-02, OQ-5).
inline constexpr uint16_t kDeniedReason = 0x2A5U;

struct PermittedPair {
  FlightState from;
  FlightEvent event;
};

// Hand-written from SRS 10; not read from kTransitionTable.
inline constexpr std::array<PermittedPair, 9> kPermittedPairs = {{
    {FlightState::Boot, FlightEvent::ResetCompleted},
    {FlightState::Initializing, FlightEvent::InitPassed},
    {FlightState::Initializing, FlightEvent::InitFailed},
    {FlightState::Disarmed, FlightEvent::ArmRequest},
    {FlightState::Armed, FlightEvent::DisarmRequest},
    {FlightState::Armed, FlightEvent::CriticalFaultToFailsafe},
    {FlightState::Armed, FlightEvent::CriticalFaultToFault},
    {FlightState::Failsafe, FlightEvent::RecoveryAccepted},
    {FlightState::Fault, FlightEvent::ServiceReset},
}};

inline bool isPermitted(FlightState from, FlightEvent event) {
  for (const PermittedPair& pair : kPermittedPairs) {
    if (pair.from == from && pair.event == event) {
      return true;
    }
  }
  return false;
}

// Event value one past the last FlightEvent enumerator.
inline FlightEvent oneAfterLastEvent() {
  return static_cast<FlightEvent>(static_cast<int>(FlightEvent::ServiceReset) + 1);
}

inline void wireAllGuards(FlightStateMachine& machine, const IStateGuard& guard) {
  machine.setGuard(GuardId::Arm, &guard);
  machine.setGuard(GuardId::Recovery, &guard);
  machine.setGuard(GuardId::ServiceReset, &guard);
}

inline void expectAccepted(FlightStateMachine& machine, FlightEvent event) {
  const DispatchResult result = machine.dispatch(event);
  ASSERT_EQ(result.outcome, Outcome::Accepted);
}

// Drives the machine to target through permitted events. Arm guard must allow before ARMED.
inline void forceState(FlightStateMachine& machine, FlightState target) {
  if (target == FlightState::Boot) {
    return;
  }
  expectAccepted(machine, FlightEvent::ResetCompleted);
  if (target == FlightState::Initializing) {
    return;
  }
  if (target == FlightState::Fault) {
    expectAccepted(machine, FlightEvent::InitFailed);
    return;
  }
  expectAccepted(machine, FlightEvent::InitPassed);
  if (target == FlightState::Disarmed) {
    return;
  }
  expectAccepted(machine, FlightEvent::ArmRequest);
  if (target == FlightState::Armed) {
    return;
  }
  expectAccepted(machine, FlightEvent::CriticalFaultToFailsafe);
}

}  // namespace dfcs_test
