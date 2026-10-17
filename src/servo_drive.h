#pragma once
#include <Arduino.h>

// Drives one continuous-rotation servo with LEDC (50Hz PWM)
class ContinuousServo {
 public:
  ContinuousServo(uint8_t pin, uint8_t ledcChannel, bool invert, int16_t trimUs);
  bool begin();
  // speed: -1.0 (full reverse) .. 0 (stop) .. +1.0 (full forward)
  void write(float speed);
  void release();  // Stop pulse output

 private:
  void writeMicroseconds(uint16_t us);

  uint8_t pin_;
  uint8_t channel_;
  bool invert_;
  int16_t trimUs_;
  bool released_ = true;
};

// Two-wheel differential drive (with acceleration limiting)
class DifferentialDrive {
 public:
  DifferentialDrive(ContinuousServo& left, ContinuousServo& right);
  bool begin();
  // throttle: forward/backward -1..1, turn: right turn positive -1..1
  void setTarget(float throttle, float turn);
  void stop();          // Set target to 0 (decelerate to a stop)
  void emergencyStop(); // Stop immediately
  void update();        // Call periodically from loop()

 private:
  ContinuousServo& left_;
  ContinuousServo& right_;
  float targetL_ = 0, targetR_ = 0;
  float curL_ = 0, curR_ = 0;
  uint32_t lastUpdateUs_ = 0;
};
