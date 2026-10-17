#pragma once
#include <Arduino.h>

// 連続回転サーボ1個を LEDC (50Hz PWM) で駆動する
class ContinuousServo {
 public:
  ContinuousServo(uint8_t pin, uint8_t ledcChannel, bool invert, int16_t trimUs);
  bool begin();
  // speed: -1.0 (後退最大) .. 0 (停止) .. +1.0 (前進最大)
  void write(float speed);
  void release();  // パルス出力を止める

 private:
  void writeMicroseconds(uint16_t us);

  uint8_t pin_;
  uint8_t channel_;
  bool invert_;
  int16_t trimUs_;
  bool released_ = true;
};

// 左右2輪の差動駆動 (加減速制限つき)
class DifferentialDrive {
 public:
  DifferentialDrive(ContinuousServo& left, ContinuousServo& right);
  bool begin();
  // throttle: 前後 -1..1, turn: 右旋回が正 -1..1
  void setTarget(float throttle, float turn);
  void stop();          // 目標を0にする (減速して停止)
  void emergencyStop(); // 即時停止
  void update();        // loop() から定期的に呼ぶ

 private:
  ContinuousServo& left_;
  ContinuousServo& right_;
  float targetL_ = 0, targetR_ = 0;
  float curL_ = 0, curR_ = 0;
  uint32_t lastUpdateUs_ = 0;
};
