#include "gamepad.h"

#include <Bluepad32.h>

#include "config.h"

namespace Pad {
namespace {

ControllerPtr active = nullptr;
GamepadState current;

constexpr float AXIS_MAX = 512.0f;  // Bluepad32 stick values range from -512..511

float axis(int32_t v) { return constrain(v / AXIS_MAX, -1.0f, 1.0f); }

// Check against GAMEPAD_ALLOWED_* in config.h
bool isAllowed(const ControllerProperties& p) {
  if (GAMEPAD_ALLOWED_VID != 0 && p.vendor_id != GAMEPAD_ALLOWED_VID) return false;
  if (GAMEPAD_ALLOWED_PID != 0 && p.product_id != GAMEPAD_ALLOWED_PID) return false;

  const char* allowedAddr = GAMEPAD_ALLOWED_ADDR;
  if (allowedAddr[0] != '\0') {
    uint8_t a[6];
    if (sscanf(allowedAddr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &a[0], &a[1], &a[2], &a[3], &a[4],
               &a[5]) != 6) {
      Serial.println("[PAD] GAMEPAD_ALLOWED_ADDR has an invalid format");
      return false;
    }
    if (memcmp(a, p.btaddr, 6) != 0) return false;
  }
  return true;
}

void onConnected(ControllerPtr ctl) {
  ControllerProperties p = ctl->getProperties();
  Serial.printf("[PAD] connected: %s VID=0x%04X PID=0x%04X addr=%02X:%02X:%02X:%02X:%02X:%02X\n",
                ctl->getModelName().c_str(), p.vendor_id, p.product_id, p.btaddr[0],
                p.btaddr[1], p.btaddr[2], p.btaddr[3], p.btaddr[4], p.btaddr[5]);

  if (!isAllowed(p)) {
    Serial.println("[PAD] not allowed by config.h -> disconnect");
    ctl->disconnect();
    return;
  }
  if (active) {
    // Additional controllers are not used for driving
    Serial.println("[PAD] extra controller rejected");
    ctl->disconnect();
    return;
  }
  active = ctl;
  current = GamepadState();
  // Prevent other devices from connecting while one is connected
  BP32.enableNewBluetoothConnections(false);
  ctl->setPlayerLEDs(0x01);
  ctl->setColorLED(0, 0, 255);
  ctl->playDualRumble(0, 150, 0x80, 0x40);
}

void onDisconnected(ControllerPtr ctl) {
  if (ctl != active) return;
  Serial.println("[PAD] disconnected");
  active = nullptr;
  current = GamepadState();
  BP32.enableNewBluetoothConnections(true);
}

void readGamepad(ControllerPtr ctl) {
  GamepadState s;
  s.lx = axis(ctl->axisX());
  s.ly = axis(ctl->axisY());
  s.rx = axis(ctl->axisRX());
  s.ry = axis(ctl->axisRY());

  uint8_t d = ctl->dpad();
  if (d & DPAD_UP) s.dpad |= GP_DPAD_UP;
  if (d & DPAD_DOWN) s.dpad |= GP_DPAD_DOWN;
  if (d & DPAD_RIGHT) s.dpad |= GP_DPAD_RIGHT;
  if (d & DPAD_LEFT) s.dpad |= GP_DPAD_LEFT;

  uint32_t b = 0;
  if (ctl->a()) b |= GP_BTN_A;
  if (ctl->b()) b |= GP_BTN_B;
  if (ctl->x()) b |= GP_BTN_X;
  if (ctl->y()) b |= GP_BTN_Y;
  if (ctl->l1()) b |= GP_BTN_L1;
  if (ctl->r1()) b |= GP_BTN_R1;
  if (ctl->l2()) b |= GP_BTN_L2;
  if (ctl->r2()) b |= GP_BTN_R2;
  if (ctl->miscSelect()) b |= GP_BTN_SELECT;
  if (ctl->miscStart()) b |= GP_BTN_START;
  if (ctl->miscSystem()) b |= GP_BTN_HOME;
  s.buttons = b;
  current = s;
}

// For gamepads in keyboard mode (e.g. 8BitDo K mode):
// arrow keys / WASD act as the D-pad, Space/Enter as A, Esc as B
void readKeyboard(ControllerPtr ctl) {
  GamepadState s;
  if (ctl->isKeyPressed(Keyboard_UpArrow) || ctl->isKeyPressed(Keyboard_W)) s.dpad |= GP_DPAD_UP;
  if (ctl->isKeyPressed(Keyboard_DownArrow) || ctl->isKeyPressed(Keyboard_S)) s.dpad |= GP_DPAD_DOWN;
  if (ctl->isKeyPressed(Keyboard_RightArrow) || ctl->isKeyPressed(Keyboard_D)) s.dpad |= GP_DPAD_RIGHT;
  if (ctl->isKeyPressed(Keyboard_LeftArrow) || ctl->isKeyPressed(Keyboard_A)) s.dpad |= GP_DPAD_LEFT;
  if (ctl->isKeyPressed(Keyboard_Spacebar) || ctl->isKeyPressed(Keyboard_Enter)) s.buttons |= GP_BTN_A;
  if (ctl->isKeyPressed(Keyboard_Escape)) s.buttons |= GP_BTN_B;
  current = s;
}

void debugPrint() {
#if DEBUG_PRINT_GAMEPAD
  Serial.printf("[PAD] L(%5.2f,%5.2f) R(%5.2f,%5.2f) dpad=0x%x btn=0x%04lx\n", current.lx,
                current.ly, current.rx, current.ry, current.dpad, (unsigned long)current.buttons);
#endif
}

}  // namespace

void begin() {
  Serial.printf("[PAD] Bluepad32 %s\n", BP32.firmwareVersion());
  const uint8_t* addr = BP32.localBdAddress();
  Serial.printf("[PAD] BD addr %02X:%02X:%02X:%02X:%02X:%02X\n", addr[0], addr[1], addr[2],
                addr[3], addr[4], addr[5]);
  BP32.setup(&onConnected, &onDisconnected);
  BP32.enableVirtualDevice(false);
  BP32.enableNewBluetoothConnections(true);
}

void update() {
  if (!BP32.update()) return;
  if (!active || !active->isConnected() || !active->hasData()) return;
  if (active->isGamepad()) {
    readGamepad(active);
  } else if (active->isKeyboard()) {
    readKeyboard(active);
  } else {
    return;
  }
  debugPrint();
}

bool isConnected() { return active && active->isConnected(); }

GamepadState state() { return current; }

void forgetPairings() {
  Serial.println("[PAD] forgetting Bluetooth pairings");
  BP32.forgetBluetoothKeys();
}

void rumble(uint16_t durationMs) {
  if (isConnected()) active->playDualRumble(0, durationMs, 0x60, 0x30);
}

}  // namespace Pad
