#pragma once
#include <Arduino.h>

// ボタンのビット割り当て (Xbox 配置。Bluepad32 は各社のパッドをこの配置に揃える)
//   A = 下, B = 右, X = 左, Y = 上
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

// 十字キー (ビットの組み合わせ)
enum GamepadDpad : uint8_t {
  GP_DPAD_UP    = 1u << 0,
  GP_DPAD_DOWN  = 1u << 1,
  GP_DPAD_RIGHT = 1u << 2,
  GP_DPAD_LEFT  = 1u << 3,
};

struct GamepadState {
  // スティック: -1.0..1.0 (X は右が正, Y は下が正)
  float lx = 0, ly = 0, rx = 0, ry = 0;
  uint8_t dpad = 0;
  uint32_t buttons = 0;
};

// Bluepad32 でゲームパッドに接続する (操縦に使うのは1台のみ)
namespace Pad {
void begin();
void update();  // loop() から定期的に呼ぶ
bool isConnected();
GamepadState state();
void forgetPairings();  // 保存済みのペアリング情報を消去
void rumble(uint16_t durationMs);
}  // namespace Pad
