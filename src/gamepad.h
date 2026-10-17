#pragma once
#include <Arduino.h>

// Button bit assignments (Xbox layout; Bluepad32 maps every vendor's gamepad to this layout)
//   A = bottom, B = right, X = left, Y = top
enum GamepadButton : uint32_t {
  GP_BTN_A      = 1u << 0,
  GP_BTN_B      = 1u << 1,
  GP_BTN_X      = 1u << 2,
  GP_BTN_Y      = 1u << 3,
  GP_BTN_L1     = 1u << 4,
  GP_BTN_R1     = 1u << 5,
  GP_BTN_L2     = 1u << 6,
  GP_BTN_R2     = 1u << 7,
  GP_BTN_SELECT = 1u << 8,
  GP_BTN_START  = 1u << 9,
  GP_BTN_HOME   = 1u << 10,
};

// D-pad (combination of bits)
enum GamepadDpad : uint8_t {
  GP_DPAD_UP    = 1u << 0,
  GP_DPAD_DOWN  = 1u << 1,
  GP_DPAD_RIGHT = 1u << 2,
  GP_DPAD_LEFT  = 1u << 3,
};

struct GamepadState {
  // Sticks: -1.0..1.0 (X positive to the right, Y positive downward)
  float lx = 0, ly = 0, rx = 0, ry = 0;
  uint8_t dpad = 0;
  uint32_t buttons = 0;
};

// Connects to a gamepad via Bluepad32 (only one is used for driving)
namespace Pad {
void begin();
void update();  // Call periodically from loop()
bool isConnected();
GamepadState state();
void forgetPairings();  // Clear stored pairing information
void rumble(uint16_t durationMs);
}  // namespace Pad
