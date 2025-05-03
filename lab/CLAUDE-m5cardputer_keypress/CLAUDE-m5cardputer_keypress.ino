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
    if (M5Cardputer.Keyboard.isPressed()) {
      M5Cardputer.Display.fillScreen(BLACK);
      M5Cardputer.Display.setCursor(10, 10);
      
      // Obtenir la liste des coordonnées de touches pressées
      auto keyList = M5Cardputer.Keyboard.keyList();
      if (keyList.size() > 0) {
        // Prendre la première touche de la liste
        Point2D_t keyCoor = keyList[0];
        
        // Obtenir la valeur de la touche à partir de ses coordonnées
        uint8_t keyValue = M5Cardputer.Keyboard.getKey(keyCoor);
        
        M5Cardputer.Display.println("Touche appuyee:");
        
        // Afficher des informations sur la touche
        if (keyValue >= 32 && keyValue <= 126) {
          // Caractère imprimable
          M5Cardputer.Display.printf("%c (ASCII: %d)", keyValue, keyValue);
        } else {
          // Touche spéciale
          M5Cardputer.Display.printf("Code: %d", keyValue);
        }
        
        // Afficher les coordonnées de la touche
        M5Cardputer.Display.printf("\nCoord: (%d,%d)", keyCoor.x, keyCoor.y);
      } else {
        M5Cardputer.Display.println("Aucune touche detectee");
      }
    } else {
      M5Cardputer.Display.fillScreen(BLACK);
      M5Cardputer.Display.setCursor(10, 10);
      M5Cardputer.Display.println("Touches relachees");
    }
  }
  
  delay(10);
}