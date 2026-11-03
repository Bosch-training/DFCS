#pragma once

#include <cstddef>
#include <cstdint>

// STA-01 (DC-17): flight states and transition table (FR-030, FR-031).

enum class FlightState { Boot, Initializing, Disarmed, Armed, Failsafe, Fault };

enum class FlightEvent {
  ResetCompleted,
  InitPassed,
  InitFailed,
  ArmRequest,
  DisarmRequest,
  CriticalFaultToFailsafe,
  CriticalFaultToFault,
  RecoveryAccepted,
  ServiceReset
};

enum class GuardId { None, Arm, Recovery, ServiceReset };

// SRS 10 "Required behavior" column, carried as data for downstream consumers.
enum class Behavior {
  KeepInhibited,
  KeepInhibitedReportFault,
  EnableControlPathUnderLimits,
  InhibitNormalCommands,
  ApplyDefinedSafetyResponse,
  NeverAutoRearm
};

enum class Outcome { Accepted, RejectedNotPermitted, RejectedGuardDenied, RejectedInvalidInput };

struct TransitionRow {
  FlightState from;
  FlightEvent event;
  GuardId guard;
  FlightState to;
  Behavior behavior;
};

struct GuardResult {
  bool allowed;
  uint16_t reason;  // opaque; code set owned by STA-02 (OQ-5)
};

class IStateGuard {
 public:
  virtual ~IStateGuard() = default;
  virtual GuardResult evaluate(FlightState state, FlightEvent event) const = 0;
};

// Deny-by-default guard (FR-067).
class DenyAllGuard final : public IStateGuard {
 public:
  GuardResult evaluate(FlightState, FlightEvent) const override { return GuardResult{false, 0U}; }
};

struct DispatchResult {
  Outcome outcome;
  FlightState from;
  FlightState to;
  int8_t row;         // table row index, -1 if none
  uint16_t reason;    // guard reason, 0 if none
  Behavior behavior;  // meaningful only when outcome == Accepted
};

inline constexpr std::size_t kTransitionRowCount = 9;
extern const TransitionRow kTransitionTable[kTransitionRowCount];

class FlightStateMachine {
 public:
  FlightStateMachine() = default;

  FlightState state() const { return state_; }

  // Wiring only; not a state setter. A null or unwired guard denies.
  void setGuard(GuardId id, const IStateGuard* guard);

  // Sole writer of state_.
  DispatchResult dispatch(FlightEvent event);

 private:
  static constexpr std::size_t kGuardSlots = 4;

  FlightState state_ = FlightState::Boot;
  const IStateGuard* guards_[kGuardSlots] = {};
};
