#pragma once

#include <cstdint>
#include <vector>

#include "flight_state.hpp"

// Hand-written fake for IStateGuard: fixed verdict, records every call.
class RecordingStateGuard final : public IStateGuard {
 public:
  struct Call {
    FlightState state;
    FlightEvent event;
  };

  RecordingStateGuard(bool allowed, uint16_t reason) : allowed_(allowed), reason_(reason) {}

  GuardResult evaluate(FlightState state, FlightEvent event) const override {
    calls_.push_back(Call{state, event});
    return GuardResult{allowed_, reason_};
  }

  const std::vector<Call>& calls() const { return calls_; }
  void clearCalls() { calls_.clear(); }

 private:
  bool allowed_;
  uint16_t reason_;
  mutable std::vector<Call> calls_;
};
