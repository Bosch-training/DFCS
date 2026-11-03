// CTL-01 single-axis PID (FR-040). Demand source only; no limits (CTL-02),
// gating (CTL-03) or gain validation beyond finiteness (CTL-04).
#ifndef DFCS_CORE_PID_HPP
#define DFCS_CORE_PID_HPP

namespace dfcs::core {

struct PidGains {
  float kp;
  float ki;
  float kd;
};

enum class PidStatus {
  Ok,
  NotConfigured,
  InvalidConfig,
  InvalidDt,
  InvalidInput,
  NumericFault
};

// output is 0.0f unless status == Ok.
struct PidResult {
  PidStatus status;
  float output;
};

// One owning caller per instance; no internal locking.
class Pid {
 public:
  PidStatus configure(const PidGains& gains);
  PidResult update(float error, float dt_s);
  void reset();
  bool configured() const { return configured_; }
  float integral() const { return integral_; }
  const PidGains& gains() const { return gains_; }

 private:
  PidGains gains_{0.0f, 0.0f, 0.0f};
  float integral_{0.0f};
  float prev_error_{0.0f};
  bool has_prev_{false};
  bool configured_{false};
};

}  // namespace dfcs::core

#endif  // DFCS_CORE_PID_HPP
