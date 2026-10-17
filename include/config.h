#pragma once
// ============================================================
//  ロボット設定 — ハードウェアや好みに合わせてここを編集
// ============================================================
#include <Arduino.h>

// ---------- ピン割り当て (M5Atom Lite) ----------
// 側面ヘッダの G21 (左車輪) / G25 (右車輪) にサーボ信号線を接続
constexpr uint8_t PIN_SERVO_LEFT  = 21;
constexpr uint8_t PIN_SERVO_RIGHT = 25;
constexpr uint8_t PIN_RGB_LED     = 27;  // 本体の SK6812 フルカラーLED
constexpr uint8_t PIN_BUTTON      = 39;  // 本体の画面ボタン (押すとLOW)

// LEDC チャンネル (サーボごとに別チャンネル)
constexpr uint8_t LEDC_CH_LEFT  = 0;
constexpr uint8_t LEDC_CH_RIGHT = 1;

// フルカラーLEDの明るさ (0..255)。最大だとかなり眩しい。
constexpr uint8_t LED_BRIGHTNESS = 40;

// ---------- 連続回転サーボ ----------
// 一般的な連続回転サーボ (FS90R, SM-S4303R など):
//   1500us = 停止, 1000us = 一方向最大, 2000us = 逆方向最大
constexpr uint16_t SERVO_STOP_US  = 1500;
constexpr uint16_t SERVO_RANGE_US = 500;   // 停止値からの最大振れ幅

// 停止パルスでも微妙に回ってしまう / 左右の速度が違う場合の補正 (us)
constexpr int16_t SERVO_TRIM_LEFT_US  = 0;
constexpr int16_t SERVO_TRIM_RIGHT_US = 0;

// 左右のサーボは鏡像に取り付けられるため、通常どちらかを反転する。
constexpr bool SERVO_INVERT_LEFT  = true;
constexpr bool SERVO_INVERT_RIGHT = false;

// 停止中はPWM出力を止める (パルス無し → 多くのサーボは完全停止しクリープしない)
constexpr bool SERVO_RELEASE_ON_STOP = true;

// ---------- 操縦 ----------
constexpr float STICK_DEADZONE = 0.10f;          // スティックの不感帯 (0..1)
// A ボタンで切り替える速度レベル (最大出力に対する割合)
constexpr float SPEED_LEVELS[] = {0.35f, 0.65f, 1.0f};
constexpr uint8_t SPEED_LEVEL_DEFAULT = 1;
constexpr float TURN_GAIN = 0.7f;                // 旋回の効き (0..1)
// 加減速の制限: 1秒あたりの出力変化量 (0..1 スケール)。大きいほど機敏。
constexpr float ACCEL_PER_SEC = 4.0f;

// ---------- 接続を許可するゲームパッド ----------
// VID/PID が一致しないパッドは接続後すぐに切断する (0 = その項目は制限しない)
// 接続時にシリアルへ表示される VID / PID / addr の値を書き込む
constexpr uint16_t GAMEPAD_ALLOWED_VID = 0x2DC8;  // 8BitDo
constexpr uint16_t GAMEPAD_ALLOWED_PID = 0x9020;  // 8BitDo Micro (D-input モード)
// 手持ちの1台だけに固定する場合は Bluetooth アドレスを書く (空文字 = 制限しない)
// 例: "E4:17:D8:12:34:56"
#define GAMEPAD_ALLOWED_ADDR "E4:17:D8:2A:5D:A2"

// 本体ボタンをこの時間長押しするとペアリング情報を消去する
constexpr uint32_t FORGET_KEYS_HOLD_MS = 3000;

// ゲームパッドの入力内容をシリアルに表示する (動作確認用)
#define DEBUG_PRINT_GAMEPAD 0
