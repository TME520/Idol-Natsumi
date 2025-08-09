#include <M5Cardputer.h>
#include <SD.h>

// Game states
enum GameState {
  TITLE_SCREEN,
  MAIN_MENU,
  NEW_GAME,
  CONTINUE_GAME
};

GameState currentState = TITLE_SCREEN;

// Menu items
const char* mainMenuItems[] = {"0: NEW GAME", "1: CONTINUE"};
const int mainMenuItemCount = 2;
int mainMenuSelection = 0;

// Frame rate control
unsigned long lastUpdate = 0;
const int FRAME_DELAY = 50;

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);

  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setTextColor(WHITE);

  if (!SD.begin()) {
    M5Cardputer.Display.println("SD init failed!");
    while (true);
  }

  drawTitleScreen();
}

void loop() {
  M5Cardputer.update();

  if (millis() - lastUpdate < FRAME_DELAY) return;
  lastUpdate = millis();

  handleInput();
}

void handleInput() {
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      uint8_t key = M5Cardputer.Keyboard.getKey(keyList[0]);

      switch (currentState) {
        case TITLE_SCREEN:
          currentState = MAIN_MENU;
          drawMainMenu();
          break;
        case MAIN_MENU:
          handleMainMenuInput(key);
          break;
        default:
          break;
      }
    }
  }
}

void handleMainMenuInput(uint8_t key) {
  switch (key) {
    case 181: case 'w': case 'W':
      mainMenuSelection = (mainMenuSelection - 1 + mainMenuItemCount) % mainMenuItemCount;
      drawMainMenu();
      break;
    case 182: case 's': case 'S':
      mainMenuSelection = (mainMenuSelection + 1) % mainMenuItemCount;
      drawMainMenu();
      break;
    case 13: case ' ':
      if (mainMenuSelection == 0) {
        currentState = NEW_GAME;
        drawNewGameScreen();
      } else {
        currentState = CONTINUE_GAME;
        drawContinueScreen();
      }
      break;
  }
}

void drawTitleScreen() {
  M5Cardputer.Display.fillScreen(BLACK);

  File f = SD.open("/idolnat/screens/title01.png", FILE_READ);
  if (!f) {
    M5Cardputer.Display.setCursor(10, 60);
    M5Cardputer.Display.setTextColor(RED);
    M5Cardputer.Display.println("Missing /idolnat/screens/title01.png");
    return;
  }

  size_t len = f.size();
  uint8_t* buf = (uint8_t*)malloc(len);
  if (!buf) {
    f.close();
    M5Cardputer.Display.setCursor(10, 60);
    M5Cardputer.Display.setTextColor(RED);
    M5Cardputer.Display.println("Out of memory");
    return;
  }

  f.read(buf, len);
  f.close();

  M5Cardputer.Display.drawPng(buf, len, 0, 0);
  free(buf);
}

void drawMainMenu() {
  M5Cardputer.Display.fillScreen(BLACK);

  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setTextColor(PINK);
  M5Cardputer.Display.setCursor(30, 10);
  M5Cardputer.Display.println("MAIN MENU");

  M5Cardputer.Display.setTextSize(1);
  for (int i = 0; i < mainMenuItemCount; i++) {
    M5Cardputer.Display.setCursor(50, 40 + i * 15);
    if (i == mainMenuSelection) {
      M5Cardputer.Display.setTextColor(YELLOW);
      M5Cardputer.Display.print("> ");
    } else {
      M5Cardputer.Display.setTextColor(WHITE);
      M5Cardputer.Display.print("  ");
    }
    M5Cardputer.Display.println(mainMenuItems[i]);
  }

  M5Cardputer.Display.setTextColor(0x7BEF);
  M5Cardputer.Display.setCursor(10, 115);
  M5Cardputer.Display.println("W/S: Navigate, Enter: Select");
}

void drawNewGameScreen() {
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setCursor(20, 60);
  M5Cardputer.Display.setTextColor(CYAN);
  M5Cardputer.Display.println("Starting a new game...");
}

void drawContinueScreen() {
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setCursor(20, 60);
  M5Cardputer.Display.setTextColor(CYAN);
  M5Cardputer.Display.println("Continuing your game...");
}
