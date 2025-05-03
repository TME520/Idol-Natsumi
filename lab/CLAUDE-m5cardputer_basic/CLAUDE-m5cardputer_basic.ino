#include <M5Cardputer.h>

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);
  
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setTextColor(WHITE);
  
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setCursor(10, 10);
  M5Cardputer.Display.println("M5 Cardputer");
  M5Cardputer.Display.println("Appuyez sur une touche");
}

void loop() {
  M5Cardputer.update();
  
  if (M5Cardputer.Keyboard.isChange()) {
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setCursor(10, 10);
    
    if (M5Cardputer.Keyboard.isPressed()) {
      M5Cardputer.Display.println("Touche appuyee!");
    } else {
      M5Cardputer.Display.println("Touches relachees");
    }
  }
  
  delay(10);
}