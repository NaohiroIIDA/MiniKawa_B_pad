// M5Atom Lite Bluetooth ゲームパッド操縦ロボット
//
// 操作 (Xbox 配置。PS/Switch パッドでも同じ位置のボタン):
//   左スティック上下 : 前進 / 後退
//   右スティック左右 : 旋回 (左スティック左右でも可)
//   十字キー         : 前後左右 (デジタル操作)
//   A (下ボタン)     : 速度レベル切替 (低 → 中 → 高)
//   B (右ボタン)     : 押している間 非常停止
//
// 本体ボタン:
//   短押し : 走行ロック (ON の間は動かない) の切替
//   長押し : ペアリング情報を消去 (別のパッドと組み直すとき)
//
// 本体LED:
//   青点滅 = ゲームパッド待ち / 緑 = 接続中 / 赤 = 停止中(ロック・非常停止)
#include <Arduino.h>

#include "config.h"
#include "gamepad.h"
#include "servo_drive.h"

namespace {

ContinuousServo leftServo(PIN_SERVO_LEFT, LEDC_CH_LEFT, SERVO_INVERT_LEFT, SERVO_TRIM_LEFT_US);
ContinuousServo rightServo(PIN_SERVO_RIGHT, LEDC_CH_RIGHT, SERVO_INVERT_RIGHT, SERVO_TRIM_RIGHT_US);
DifferentialDrive drive(leftServo, rightServo);

constexpr uint8_t NUM_SPEED_LEVELS = sizeof(SPEED_LEVELS) / sizeof(SPEED_LEVELS[0]);
uint8_t speedLevel = SPEED_LEVEL_DEFAULT;
uint32_t prevButtons = 0;
bool locked = false;    // 本体ボタンでの走行ロック
bool stopping = false;  // B ボタンでの非常停止中

// ---------------- LED ----------------

void setLed(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(PIN_RGB_LED, r * LED_BRIGHTNESS / 255, g * LED_BRIGHTNESS / 255,
                b * LED_BRIGHTNESS / 255);
}

void updateLed(bool connected) {
  static uint32_t lastMs = 0;
  if (millis() - lastMs < 50) return;  // 書き込み頻度を抑える
  lastMs = millis();

  if (!connected) {
    bool on = (millis() / 300) % 2;
    setLed(0, 0, on ? 255 : 0);
  } else if (locked || stopping) {
    setLed(255, 0, 0);
  } else {
    setLed(0, 255, 0);
  }
}

// ---------------- 本体ボタン ----------------

void handleButton() {
  static bool wasPressed = false;
  static uint32_t pressedAt = 0;
  static bool longHandled = false;

  bool pressed = digitalRead(PIN_BUTTON) == LOW;
  uint32_t now = millis();

  if (pressed && !wasPressed) {
    pressedAt = now;
    longHandled = false;
  } else if (pressed && !longHandled && now - pressedAt >= FORGET_KEYS_HOLD_MS) {
    longHandled = true;
    drive.emergencyStop();
    setLed(255, 255, 255);
    Pad::forgetPairings();
    delay(500);
  } else if (!pressed && wasPressed && !longHandled && now - pressedAt > 30) {
    locked = !locked;
    Serial.printf("[DRIVE] lock %s\n", locked ? "ON" : "OFF");
  }
  wasPressed = pressed;
}

// ---------------- 操縦 ----------------

float applyDeadzone(float v) {
  if (fabsf(v) < STICK_DEADZONE) return 0;
  // 不感帯の外側を 0..1 に再マップ
  float sign = v > 0 ? 1.0f : -1.0f;
  return sign * (fabsf(v) - STICK_DEADZONE) / (1.0f - STICK_DEADZONE);
}

void handleGamepad() {
  if (!Pad::isConnected()) {
    drive.emergencyStop();
    prevButtons = 0;
    stopping = false;
    return;
  }

  GamepadState gp = Pad::state();
  uint32_t pressed = gp.buttons & ~prevButtons;
  prevButtons = gp.buttons;

  if (pressed & GP_BTN_A) {
    speedLevel = (speedLevel + 1) % NUM_SPEED_LEVELS;
    Serial.printf("[DRIVE] speed level %u (%.0f%%)\n", speedLevel + 1,
                  SPEED_LEVELS[speedLevel] * 100);
    Pad::rumble(80 * (speedLevel + 1));  // レベルが高いほど長く振動
  }

  stopping = gp.buttons & GP_BTN_B;
  if (locked || stopping) {
    drive.emergencyStop();
    return;
  }

  float throttle = -applyDeadzone(gp.ly);  // スティックは上が負なので反転
  float turn = applyDeadzone(gp.rx);
  if (turn == 0) turn = applyDeadzone(gp.lx);

  // 十字キーが押されていればスティックより優先
  if (gp.dpad) {
    throttle = 0;
    turn = 0;
    if (gp.dpad & GP_DPAD_UP) throttle += 1;
    if (gp.dpad & GP_DPAD_DOWN) throttle -= 1;
    if (gp.dpad & GP_DPAD_RIGHT) turn += 1;
    if (gp.dpad & GP_DPAD_LEFT) turn -= 1;
    if (throttle != 0) turn *= 0.5f;  // 斜めは緩やかにカーブ
  }

  float scale = SPEED_LEVELS[speedLevel];
  drive.setTarget(throttle * scale, turn * TURN_GAIN * scale);
}

}  // namespace

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUTTON, INPUT);  // G39 は内部プルアップ無し (基板上でプルアップ済み)
  setLed(0, 0, 0);

  if (!drive.begin()) {
    Serial.println("[DRIVE] servo PWM init failed");
  }
  Pad::begin();
  Serial.println("M5Atom Lite robot ready");
}

void loop() {
  Pad::update();
  handleButton();
  handleGamepad();
  drive.update();
  updateLed(Pad::isConnected());
  // Bluepad32 のタスクに CPU を譲る (ウォッチドッグ対策)
  delay(10);
}
