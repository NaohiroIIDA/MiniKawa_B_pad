#include "servo_drive.h"

#include "config.h"

namespace {
constexpr uint32_t PWM_FREQ_HZ = 50;
constexpr uint8_t PWM_RESOLUTION_BITS = 16;
constexpr uint32_t PWM_PERIOD_US = 1000000 / PWM_FREQ_HZ;  // 20000us
constexpr float STOP_THRESHOLD = 0.01f;

float approach(float current, float target, float maxStep) {
  if (target > current + maxStep) return current + maxStep;
  if (target < current - maxStep) return current - maxStep;
  return target;
}
}  // namespace

// ---------------- ContinuousServo ----------------

ContinuousServo::ContinuousServo(uint8_t pin, uint8_t ledcChannel, bool invert, int16_t trimUs)
    : pin_(pin), channel_(ledcChannel), invert_(invert), trimUs_(trimUs) {}

bool ContinuousServo::begin() {
  if (ledcSetup(channel_, PWM_FREQ_HZ, PWM_RESOLUTION_BITS) == 0) return false;
  ledcAttachPin(pin_, channel_);
  release();
  return true;
}

void ContinuousServo::writeMicroseconds(uint16_t us) {
  uint32_t duty = (uint32_t)us * ((1u << PWM_RESOLUTION_BITS) - 1) / PWM_PERIOD_US;
  ledcWrite(channel_, duty);
  released_ = false;
}

void ContinuousServo::release() {
  ledcWrite(channel_, 0);
  released_ = true;
}

void ContinuousServo::write(float speed) {
  speed = constrain(speed, -1.0f, 1.0f);
  if (SERVO_RELEASE_ON_STOP && fabsf(speed) < STOP_THRESHOLD) {
    if (!released_) release();
    return;
  }
  if (invert_) speed = -speed;
  int us = SERVO_STOP_US + trimUs_ + (int)lroundf(speed * SERVO_RANGE_US);
  writeMicroseconds((uint16_t)constrain(us, 500, 2500));
}

// ---------------- DifferentialDrive ----------------

DifferentialDrive::DifferentialDrive(ContinuousServo& left, ContinuousServo& right)
    : left_(left), right_(right) {}

bool DifferentialDrive::begin() {
  lastUpdateUs_ = micros();
  return left_.begin() && right_.begin();
}

void DifferentialDrive::setTarget(float throttle, float turn) {
  float l = throttle + turn;
  float r = throttle - turn;
  // どちらかが ±1 を超えたら比率を保って正規化
  float m = max(fabsf(l), fabsf(r));
  if (m > 1.0f) {
    l /= m;
    r /= m;
  }
  targetL_ = l;
  targetR_ = r;
}

void DifferentialDrive::stop() { targetL_ = targetR_ = 0; }

void DifferentialDrive::emergencyStop() {
  targetL_ = targetR_ = curL_ = curR_ = 0;
  left_.write(0);
  right_.write(0);
}

void DifferentialDrive::update() {
  uint32_t now = micros();
  float dt = (now - lastUpdateUs_) * 1e-6f;
  lastUpdateUs_ = now;
  float step = ACCEL_PER_SEC * dt;
  curL_ = approach(curL_, targetL_, step);
  curR_ = approach(curR_, targetR_, step);
  left_.write(curL_);
  right_.write(curR_);
}
