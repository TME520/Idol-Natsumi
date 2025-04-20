#include <M5Unified.h>

String keysStateToString(const Keyboard_Class::KeysState& keys) {
  String result;

  // Modifier keys
  result += "Modifiers: ";
  if (keys.ctrl) result += "[Ctrl] ";
  if (keys.shift) result += "[Shift] ";
  if (keys.alt) result += "[Alt] ";
  if (keys.fn) result += "[Fn] ";
  if (keys.opt) result += "[Opt] ";
  if (keys.tab) result += "[Tab] ";
  result += "\n";

  // Special keys
  if (keys.enter) result += "Enter pressed\n";
  if (keys.space) result += "Space pressed\n";
  if (keys.del) result += "Delete pressed\n";

  // Word input
  result += "Word: ";
  for (char c : keys.word) result += c;
  result += "\n";

  // HID key codes
  result += "HID keys: ";
  for (auto hid : keys.hid_keys) {
    result += String(hid, HEX);
    result += " ";
  }
  result += "\n";

  return result;
}

void loop() {
  M5.update();
  auto keys = M5Cardputer.Keyboard.keysState();
  
  String debugInfo = keysStateToString(keys);

  M5.Display.setCursor(0, 0);
  M5.Display.clear();
  M5.Display.println(debugInfo);

  delay(200);
}
