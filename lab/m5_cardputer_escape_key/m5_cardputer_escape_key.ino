#include "M5Cardputer.h"

void loop() {
  M5.update();

  auto keys = M5Cardputer.Keyboard.keysState();

  for (auto k : keys.hid_keys) {
    if (k == 0x29) {  // Escape key HID code
      M5.Display.println("Escape pressed!");
    }
  }

  delay(100);
}
