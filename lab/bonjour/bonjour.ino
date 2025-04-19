#include "M5Cardputer.h"

void setup() {
  // Initialisation de M5
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);

  // Efface l'écran et initialise le texte
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setCursor(10, 10);
  M5Cardputer.Display.println("Appuyez sur B pour 'Bonjour'");
  M5Cardputer.Display.println("Appuyez sur Escape pour quitter");
}

void loop() {
  M5Cardputer.update();  // Met à jour les entrées (clavier, etc.)

  if (M5Cardputer.Keyboard.isKeyPressed('b')) {
    M5Cardputer.Display.clear();
    M5Cardputer.Display.setCursor(10, 10);
    M5Cardputer.Display.println("Bonjour");
  }

  if (M5Cardputer.Keyboard.isKeyPressed('q')) {
    M5Cardputer.Display.clear();
    M5Cardputer.Display.setCursor(10, 10);
    M5Cardputer.Display.println("Au revoir!");
    delay(2000);  // Affiche le message pendant 2 secondes
    esp_deep_sleep_start();  // Met l'appareil en sommeil profond (équivaut à quitter)
  }

  delay(100);  // Petite pause pour éviter trop de rafraîchissements
}
