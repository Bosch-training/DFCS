#include "flight_state.hpp"

namespace {

bool isValidEvent(FlightEvent event) {
  const int value = static_cast<int>(event);
  return value >= static_cast<int>(FlightEvent::ResetCompleted) &&
         value <= static_cast<int>(FlightEvent::ServiceReset);
}

}  // namespace

void FlightStateMachine::setGuard(GuardId id, const IStateGuard* guard) {
  const std::size_t index = static_cast<std::size_t>(id);
  if (id != GuardId::None && index < kGuardSlots) {
    guards_[index] = guard;
  }
}

DispatchResult FlightStateMachine::dispatch(FlightEvent event) {
  DispatchResult result{Outcome::RejectedInvalidInput, state_, state_, -1, 0U,
                        Behavior::KeepInhibited};

  if (!isValidEvent(event)) {
    return result;
  }

  for (std::size_t i = 0; i < kTransitionRowCount; ++i) {
    const TransitionRow& row = kTransitionTable[i];
    if (row.from != state_ || row.event != event) {
      continue;
    }

    if (row.guard != GuardId::None) {
      const IStateGuard* guard = guards_[static_cast<std::size_t>(row.guard)];
      if (guard == nullptr) {
        result.outcome = Outcome::RejectedGuardDenied;
        return result;
      }
      const GuardResult verdict = guard->evaluate(state_, event);
      if (!verdict.allowed) {
        result.outcome = Outcome::RejectedGuardDenied;
        result.reason = verdict.reason;
        return result;
      }
    }

    state_ = row.to;
    result.outcome = Outcome::Accepted;
    result.to = row.to;
    result.row = static_cast<int8_t>(i);
    result.behavior = row.behavior;
    return result;
  }

  result.outcome = Outcome::RejectedNotPermitted;
  return result;
}
