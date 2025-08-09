#include <M5Cardputer.h>
#include <EEPROM.h>
#include <SD.h>

// EEPROM
const int EEPROM_SIZE = 16;
const int EEPROM_ADDR_TOTAL_TIME = 0;
const unsigned long SAVE_INTERVAL = 10000; // Save every 10 seconds

// Game states
enum GameState {
  MENU,
  PLAYING,
  GAME_OVER,
  SETTINGS
};

// Player character
struct Player {
  int x, y;
  int health;
  int score;
  bool isMoving;
};

// Global game variables
GameState currentState = MENU;
Player natsumi;
unsigned long sessionStartTime;
unsigned long totalGameTime = 0;
unsigned long lastSaveTime = 0;
unsigned long lastUpdate = 0;
const int FRAME_DELAY = 50;

// Menu state
int menuSelection = 0;
const char* menuItems[] = {"Start Game", "Settings", "High Score"};
const int menuItemCount = 3;

// In-game menu state
bool isInGameMenuVisible = false;
const char* inGameMenuItems[] = {
  "0: Stats", "1: Food", "2: Train",
  "3: Compete", "4: Health", "5: Rest"
};
const int inGameMenuItemCount = 6;
int inGameMenuSelection = 0;

void setup() {
  auto cfg = M5.config();

  // Initialize EEPROM and SD
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(EEPROM_ADDR_TOTAL_TIME, totalGameTime);
  if (!SD.begin()) {
    M5Cardputer.Display.println("SD init failed!");
    while (true);
  }

  M5Cardputer.begin(cfg);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setTextColor(WHITE);

  sessionStartTime = millis();
  initPlayer();
  drawMenu();
}

void loop() {
  M5Cardputer.update();

  if (millis() - lastUpdate < FRAME_DELAY) return;
  lastUpdate = millis();

  // Auto-save total game time
  if (millis() - lastSaveTime >= SAVE_INTERVAL) {
    unsigned long currentSessionTime = millis() - sessionStartTime;
    unsigned long currentTotalTime = totalGameTime + currentSessionTime;
    EEPROM.put(EEPROM_ADDR_TOTAL_TIME, currentTotalTime);
    EEPROM.commit();
    lastSaveTime = millis();
  }

  handleInput();

  switch (currentState) {
    case MENU:
      sessionStartTime = millis();
      updateMenu();
      break;
    case PLAYING:
      updateGame();
      break;
    case GAME_OVER:
      updateGameOver();
      break;
    case SETTINGS:
      updateSettings();
      break;
  }
}

void initPlayer() {
  natsumi.x = 120;
  natsumi.y = 100;
  natsumi.health = 100;
  natsumi.score = 0;
  natsumi.isMoving = false;
}

void handleInput() {
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      uint8_t key = M5Cardputer.Keyboard.getKey(keyList[0]);

      if (currentState == PLAYING && isInGameMenuVisible) {
        switch (key) {
          case 181: case 'w': case 'W':
            inGameMenuSelection = (inGameMenuSelection - 1 + inGameMenuItemCount) % inGameMenuItemCount;
            drawInGameMenu();
            return;
          case 182: case 's': case 'S':
            inGameMenuSelection = (inGameMenuSelection + 1) % inGameMenuItemCount;
            drawInGameMenu();
            return;
          case 13: case ' ':
            // Future: handle menu selection
            return;
        }
      }

      switch (currentState) {
        case MENU: handleMenuInput(key); break;
        case PLAYING: handleGameInput(key); break;
        case GAME_OVER: handleGameOverInput(key); break;
        case SETTINGS: handleSettingsInput(key); break;
      }
    }
  }
}

void handleMenuInput(uint8_t key) {
  switch (key) {
    case 181: case 'w': case 'W':
      menuSelection = (menuSelection - 1 + menuItemCount) % menuItemCount;
      drawMenu(); break;
    case 182: case 's': case 'S':
      menuSelection = (menuSelection + 1) % menuItemCount;
      drawMenu(); break;
    case 13: case ' ':
      selectMenuItem(); break;
  }
}

void handleGameInput(uint8_t key) {
  switch (key) {
    case '\t':
      isInGameMenuVisible = !isInGameMenuVisible;
      if (isInGameMenuVisible) drawInGameMenu();
      else drawGameBackground();
      break;
    case 27: case 'q': case 'Q':
      if (isInGameMenuVisible) {
        isInGameMenuVisible = false;
        drawGameBackground();
      } else {
        currentState = MENU;
        drawMenu();
      }
      break;
    case 181: case 'w': case 'W':
      if (natsumi.y > 10) natsumi.y -= 5; natsumi.isMoving = true; break;
    case 182: case 's': case 'S':
      if (natsumi.y < 125) natsumi.y += 5; natsumi.isMoving = true; break;
    case 180: case 'a': case 'A':
      if (natsumi.x > 10) natsumi.x -= 5; natsumi.isMoving = true; break;
    case 183: case 'd': case 'D':
      if (natsumi.x < 230) natsumi.x += 5; natsumi.isMoving = true; break;
  }
}

void handleGameOverInput(uint8_t key) {
  if (key == 13 || key == ' ') {
    currentState = MENU;
    drawMenu();
  }
}

void handleSettingsInput(uint8_t key) {
  if (key == 27 || key == 'q' || key == 'Q') {
    currentState = MENU;
    drawMenu();
  }
}

void updateMenu() {}

void updateGame() {
  natsumi.isMoving = false;
  natsumi.score += 1;
  drawGame();
}

unsigned long getTotalElapsedSeconds() {
  return (totalGameTime + millis() - sessionStartTime) / 1000;
}

String formatTime(unsigned long totalSeconds) {
  char buffer[9];
  sprintf(buffer, "%02lu:%02lu:%02lu",
          totalSeconds / 3600,
          (totalSeconds % 3600) / 60,
          totalSeconds % 60);
  return String(buffer);
}

void updateGameOver() {}
void updateSettings() {}

void drawMenu() {
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setCursor(50, 10);
  M5Cardputer.Display.setTextColor(PINK);
  M5Cardputer.Display.println("IDOL NATSUMI");

  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setCursor(10, 35);
  M5Cardputer.Display.setTextColor(CYAN);
  M5Cardputer.Display.print("Elapsed: ");
  M5Cardputer.Display.println(formatTime(getTotalElapsedSeconds()));

  for (int i = 0; i < menuItemCount; i++) {
    M5Cardputer.Display.setCursor(60, 55 + i * 15);
    M5Cardputer.Display.setTextColor(i == menuSelection ? YELLOW : WHITE);
    M5Cardputer.Display.print(i == menuSelection ? "> " : "  ");
    M5Cardputer.Display.println(menuItems[i]);
  }

  M5Cardputer.Display.setTextColor(0x7BEF);
  M5Cardputer.Display.setCursor(10, 115);
  M5Cardputer.Display.println("W/S: Navigate, Enter: Select");
}

void drawGame() {
  drawGameBackground();
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setTextColor(CYAN);
  M5Cardputer.Display.setCursor(5, 5);
  M5Cardputer.Display.print("Time: ");
  M5Cardputer.Display.println(formatTime(getTotalElapsedSeconds()));

  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.setCursor(5, 15);
  M5Cardputer.Display.printf("Score: %d", natsumi.score);
  M5Cardputer.Display.setCursor(5, 25);
  M5Cardputer.Display.printf("Health: %d", natsumi.health);

  M5Cardputer.Display.setTextColor(PINK);
  M5Cardputer.Display.setCursor(natsumi.x, natsumi.y);
  M5Cardputer.Display.print("♪");

  M5Cardputer.Display.drawRect(5, 35, 230, 95, WHITE);
}

void drawGameBackground() {
  M5Cardputer.Display.drawPngFile(SD, "/sprites/natsumi_11yo.png", 0, 0);
}

void drawInGameMenu() {
  M5Cardputer.Display.fillRect(20, 20, 200, 90, BLACK);
  M5Cardputer.Display.drawRect(20, 20, 200, 90, WHITE);

  for (int i = 0; i < inGameMenuItemCount; i++) {
    M5Cardputer.Display.setCursor(30, 30 + i * 12);
    M5Cardputer.Display.setTextColor(i == inGameMenuSelection ? YELLOW : WHITE);
    M5Cardputer.Display.print(i == inGameMenuSelection ? "> " : "  ");
    M5Cardputer.Display.println(inGameMenuItems[i]);
  }
}

void selectMenuItem() {
  switch (menuSelection) {
    case 0:
      initPlayer();
      drawGameBackground();
      currentState = PLAYING;
      break;
    case 1:
      currentState = SETTINGS;
      drawSettings();
      break;
    case 2:
      drawHighScore();
      break;
  }
}

void drawSettings() {
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setCursor(50, 40);
  M5Cardputer.Display.setTextColor(CYAN);
  M5Cardputer.Display.println("SETTINGS");

  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setCursor(30, 80);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.println("Settings coming soon!");

  M5Cardputer.Display.setTextColor(0x7BEF);
  M5Cardputer.Display.setCursor(50, 120);
  M5Cardputer.Display.println("Press Q to return");
}

void drawHighScore() {
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setCursor(30, 40);
  M5Cardputer.Display.setTextColor(GOLD);
  M5Cardputer.Display.println("HIGH SCORE");

  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setCursor(80, 80);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.println("1000 pts");

  delay(2000);
  drawMenu();
}
