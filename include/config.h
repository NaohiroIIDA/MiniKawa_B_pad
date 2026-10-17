#pragma once
// ============================================================
//  Robot settings — edit these to match your hardware and preferences
// ============================================================
#include <Arduino.h>

// ---------- Pin assignments (M5Atom Lite) ----------
// Connect the servo signal wires to G21 (left wheel) / G25 (right wheel) on the side header
constexpr uint8_t PIN_SERVO_LEFT  = 21;
constexpr uint8_t PIN_SERVO_RIGHT = 25;
constexpr uint8_t PIN_RGB_LED     = 27;  // On-board SK6812 RGB LED
constexpr uint8_t PIN_BUTTON      = 39;  // On-board front button (LOW when pressed)

// LEDC channels (a separate channel per servo)
constexpr uint8_t LEDC_CH_LEFT  = 0;
constexpr uint8_t LEDC_CH_RIGHT = 1;

// RGB LED brightness (0..255). Maximum is very bright.
constexpr uint8_t LED_BRIGHTNESS = 40;

// ---------- Continuous-rotation servos ----------
// Typical continuous-rotation servos (FS90R, SM-S4303R, etc.):
//   1500us = stop, 1000us = full speed one way, 2000us = full speed the other way
constexpr uint16_t SERVO_STOP_US  = 1500;
constexpr uint16_t SERVO_RANGE_US = 500;   // Maximum deviation from the stop value

// Trim (us) for servos that creep at the stop pulse / left-right speed mismatch
constexpr int16_t SERVO_TRIM_LEFT_US  = 0;
constexpr int16_t SERVO_TRIM_RIGHT_US = 0;

// The servos are mounted as mirror images, so one side is usually inverted.
constexpr bool SERVO_INVERT_LEFT  = true;
constexpr bool SERVO_INVERT_RIGHT = false;

// Stop PWM output while stopped (no pulses -> most servos stop fully without creeping)
constexpr bool SERVO_RELEASE_ON_STOP = true;

// ---------- Driving ----------
constexpr float STICK_DEADZONE = 0.10f;          // Stick dead zone (0..1)
// Speed levels cycled with the A button (fraction of full output)
constexpr float SPEED_LEVELS[] = {0.35f, 0.65f, 1.0f};
constexpr uint8_t SPEED_LEVEL_DEFAULT = 1;
constexpr float TURN_GAIN = 0.7f;                // Turning strength (0..1)
// Acceleration limit: output change per second (0..1 scale). Higher = more responsive.
constexpr float ACCEL_PER_SEC = 4.0f;

// ---------- Gamepads allowed to connect ----------
// Gamepads whose VID/PID do not match are disconnected right after connecting (0 = no restriction)
// Use the VID / PID / addr values printed to serial when a gamepad connects
constexpr uint16_t GAMEPAD_ALLOWED_VID = 0x2DC8;  // 8BitDo
constexpr uint16_t GAMEPAD_ALLOWED_PID = 0x9020;  // 8BitDo Micro (D-input mode)
// To lock to a single gamepad, set its Bluetooth address (empty string = no restriction)
// Example: "E4:17:D8:12:34:56"
#define GAMEPAD_ALLOWED_ADDR "E4:17:D8:2A:5D:A2"

// Holding the on-board button this long clears the pairing information
constexpr uint32_t FORGET_KEYS_HOLD_MS = 3000;

// Print gamepad input to serial (for testing)
#define DEBUG_PRINT_GAMEPAD 0
