#include "pid.hpp"

#include <cmath>

namespace dfcs::core {

PidStatus Pid::configure(const PidGains& gains) {
  if (!std::isfinite(gains.kp) || !std::isfinite(gains.ki) ||
      !std::isfinite(gains.kd)) {
    return PidStatus::InvalidConfig;
  }
  gains_ = gains;
  reset();
  configured_ = true;
  return PidStatus::Ok;
}

PidResult Pid::update(float error, float dt_s) {
  if (!configured_) {
    return {PidStatus::NotConfigured, 0.0f};
  }
  if (!std::isfinite(dt_s) || dt_s <= 0.0f) {
    return {PidStatus::InvalidDt, 0.0f};
  }
  if (!std::isfinite(error)) {
    return {PidStatus::InvalidInput, 0.0f};
  }

  const float p = gains_.kp * error;
  const float i_new = integral_ + gains_.ki * error * dt_s;
  const float d =
      has_prev_ ? gains_.kd * (error - prev_error_) / dt_s : 0.0f;
  const float sum = p + i_new + d;
  if (!std::isfinite(p) || !std::isfinite(i_new) || !std::isfinite(d) ||
      !std::isfinite(sum)) {
    return {PidStatus::NumericFault, 0.0f};
  }

  integral_ = i_new;
  prev_error_ = error;
  has_prev_ = true;
  return {PidStatus::Ok, sum};
}

void Pid::reset() {
  integral_ = 0.0f;
  prev_error_ = 0.0f;
  has_prev_ = false;
}

}  // namespace dfcs::core
