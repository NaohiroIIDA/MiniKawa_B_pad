// M5Atom Lite Bluetooth gamepad-controlled robot
//
// Controls (Xbox layout; same button positions on PS/Switch gamepads):
//   Left stick up/down    : forward / backward
//   Right stick left/right: turn (left stick left/right also works)
//   D-pad                 : forward/backward/left/right (digital)
//   A (bottom button)     : cycle speed level (low -> mid -> high)
//   B (right button)      : emergency stop while held
//
// On-board button:
//   Short press: toggle drive lock (no movement while ON)
//   Long press : clear pairing information (to pair a different gamepad)
//
// On-board LED:
//   Blinking blue = waiting for gamepad / green = connected / red = stopped (lock or emergency stop)
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
bool locked = false;    // Drive lock toggled by the on-board button
bool stopping = false;  // Emergency stop active (B button held)

// ---------------- LED ----------------

void setLed(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(PIN_RGB_LED, r * LED_BRIGHTNESS / 255, g * LED_BRIGHTNESS / 255,
                b * LED_BRIGHTNESS / 255);
}

void updateLed(bool connected) {
  static uint32_t lastMs = 0;
  if (millis() - lastMs < 50) return;  // Limit the update rate
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

// ---------------- On-board button ----------------

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

// ---------------- Driving ----------------

float applyDeadzone(float v) {
  if (fabsf(v) < STICK_DEADZONE) return 0;
  // Remap the range outside the dead zone to 0..1
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
    Pad::rumble(80 * (speedLevel + 1));  // Longer rumble at higher levels
  }

  stopping = gp.buttons & GP_BTN_B;
  if (locked || stopping) {
    drive.emergencyStop();
    return;
  }

  float throttle = -applyDeadzone(gp.ly);  // Stick up is negative, so invert
  float turn = applyDeadzone(gp.rx);
  if (turn == 0) turn = applyDeadzone(gp.lx);

  // The D-pad takes priority over the sticks when pressed
  if (gp.dpad) {
    throttle = 0;
    turn = 0;
    if (gp.dpad & GP_DPAD_UP) throttle += 1;
    if (gp.dpad & GP_DPAD_DOWN) throttle -= 1;
    if (gp.dpad & GP_DPAD_RIGHT) turn += 1;
    if (gp.dpad & GP_DPAD_LEFT) turn -= 1;
    if (throttle != 0) turn *= 0.5f;  // Diagonals make a gentle curve
  }

  float scale = SPEED_LEVELS[speedLevel];
  drive.setTarget(throttle * scale, turn * TURN_GAIN * scale);
}

}  // namespace

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUTTON, INPUT);  // G39 has no internal pull-up (pulled up on the board)
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
  // Yield the CPU to the Bluepad32 task (avoids watchdog resets)
  delay(10);
}
