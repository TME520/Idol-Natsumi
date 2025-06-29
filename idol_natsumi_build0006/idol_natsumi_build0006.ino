
#include <M5Cardputer.h>
#include <SD.h>

// === Game Time Tracking ===
// 60000 milliseconds in a minute
// 86,400,000 milliseconds in a day
unsigned long sessionStart = 0;           // millis() when NEW_GAME starts
unsigned long playtimeTotalMs = 0;        // total playtime in ms (could persist later)
int lastAgeTick = 0;

// === Game state definitions ===
enum GameState {
  TITLE_SCREEN,
  CALIBRATION_1,
  CALIBRATION_2,
  CALIBRATION_3,
  MAIN_MENU,
  NEW_GAME,
  CONTINUE_GAME,
  DEBUG_MODE
};

GameState currentState = TITLE_SCREEN;

// === Player data ===
struct NatsumiStats {
  int age;
  int hunger;
};

NatsumiStats natsumi;

const char* mainMenuItems[] = {"0: NEW GAME", "1: CONTINUE", "2: DEBUG"};
const int mainMenuItemCount = 3;
int mainMenuSelection = 0;
bool bgNeedsRedraw = true;
bool fgNeedsRedraw = true;

unsigned long lastUpdate = 0;
const int FRAME_DELAY = 50;

// === Image preload system ===
struct ImageBuffer {
  uint8_t* data = nullptr;
  size_t length = 0;
};

ImageBuffer titleImage;
ImageBuffer calib1, calib2, calib3;

bool preloadImage(const char* path, ImageBuffer &imgBuf) {
  File f = SD.open(path, FILE_READ);
  if (!f) return false;

  imgBuf.length = f.size();
  imgBuf.data = (uint8_t*)malloc(imgBuf.length);
  if (!imgBuf.data) {
    f.close();
    return false;
  }

  f.read(imgBuf.data, imgBuf.length);
  f.close();
  return true;
}

void preloadImages() {
  preloadImage("/idolnat/screens/title01.png", titleImage);
  preloadImage("/idolnat/screens/setup_3tiers_busybar.png", calib1);
  preloadImage("/idolnat/screens/setup_menubox.png", calib2);
  preloadImage("/idolnat/screens/setup_dialog.png", calib3);
}

void drawImage(const ImageBuffer& img) {
  if (img.data && img.length > 0) {
    M5Cardputer.Display.drawPng(img.data, img.length, 0, 0);
  }
}

// === Setup and loop ===

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);

  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setTextColor(RED);

  if (!SD.begin()) {
    M5Cardputer.Display.println("SD init failed!");
    while (true);
  }

  preloadImages();

  currentState = TITLE_SCREEN;
  bgNeedsRedraw = true;
  fgNeedsRedraw = true;
}

void loop() {
  M5Cardputer.update();

  if (millis() - lastUpdate < FRAME_DELAY) return;
  lastUpdate = millis();

  switch (currentState) {
    case TITLE_SCREEN:
      manageTitleScreen();
      break;
    case NEW_GAME:
      manageHomeScreen();
      break;
    case DEBUG_MODE: case CALIBRATION_1: case CALIBRATION_2: case CALIBRATION_3:
      manageDebugMode();
      break;
  }
}

// === Menu and state logic ===

void updateAging() {
  unsigned long currentPlaytime = millis() - sessionStart;
  unsigned long totalMs = playtimeTotalMs + currentPlaytime;
  int ageOffset = totalMs / 86400000UL; // 1 day in milliseconds

  /*
  if (ageOffset > lastAgeTick) {
    int delta = ageOffset - lastAgeTick;
    natsumi.age += delta;
    lastAgeTick = ageOffset;
    fgNeedsRedraw = true;
  }
  */ 
}

void manageTitleScreen() {
  if (bgNeedsRedraw) {
    drawTitleScreen();
    bgNeedsRedraw = false;
  }
  if (fgNeedsRedraw) {
    drawMainMenu();
    fgNeedsRedraw = false;
  }
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      uint8_t key = M5Cardputer.Keyboard.getKey(keyList[0]);
      switch (key) {
        case 181: case 'w': case 'W':
          mainMenuSelection = (mainMenuSelection - 1 + mainMenuItemCount) % mainMenuItemCount;
          bgNeedsRedraw = false;
          fgNeedsRedraw = true;
          break;
        case 182: case 's': case 'S':
          mainMenuSelection = (mainMenuSelection + 1) % mainMenuItemCount;
          bgNeedsRedraw = false;
          fgNeedsRedraw = true;
          break;
        case 13: case 40: case ' ':
          if (mainMenuSelection == 0) {
            currentState = NEW_GAME;
            natsumi.age = 11;
            natsumi.hunger = 0;
            playtimeTotalMs = 0;
            sessionStart = millis();
            lastAgeTick = 0;
            bgNeedsRedraw = false;
            fgNeedsRedraw = true;
            drawHomeScreen();
          } else if (mainMenuSelection == 1) {
            currentState = CONTINUE_GAME;
            drawContinueScreen();
          } else {
            currentState = DEBUG_MODE;
            bgNeedsRedraw = true;
            fgNeedsRedraw = true;
            manageDebugMode();
          }
          break;
      }
    }
  }
}

void manageHomeScreen() {
  if (bgNeedsRedraw) {
    drawHomeScreen();
    bgNeedsRedraw = false;
  }
  if (fgNeedsRedraw) {
    drawHomeScreen();
    fgNeedsRedraw = false;
  }
  updateAging();
  switch(natsumi.age) {
    case 11: case 12:
      fgNeedsRedraw = true;
      break;
    case 13: case 14: case 15:
      break;
    case 16: case 17: case 18:
      break;
    case 19: case 20: case 21:
      break;
  }
}

void manageDebugMode() {
  if (bgNeedsRedraw) {
    drawDebugScreen();
    bgNeedsRedraw = false;
  }
  if (fgNeedsRedraw) {
    drawDebugScreen();
    fgNeedsRedraw = false;
  }
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      uint8_t key = M5Cardputer.Keyboard.getKey(keyList[0]);
      M5Cardputer.Display.println("Key " + String(key) + " pressed...");
      switch (currentState) {
        case DEBUG_MODE:
          if (key == 43) {
            currentState = CALIBRATION_1;
            bgNeedsRedraw = true;
            fgNeedsRedraw = false;
          }
          break;
        case CALIBRATION_1:
          if (key == 43) {
            currentState = CALIBRATION_2;
            bgNeedsRedraw = true;
            fgNeedsRedraw = false;
          }
          break;
        case CALIBRATION_2:
          if (key == 43) {
            currentState = CALIBRATION_3;
            bgNeedsRedraw = true;
            fgNeedsRedraw = false;
          }
          break;
        case CALIBRATION_3:
          if (key == 43) {
            currentState = TITLE_SCREEN;
            bgNeedsRedraw = true;
            fgNeedsRedraw = true;
          }
          break;
        default:
          break;
      }
    }
  }
}

// === Draw functions ===
void drawTitleScreen() {
  M5Cardputer.Display.fillScreen(BLACK);
  drawImage(titleImage);
}

void drawMainMenu() {
  uint16_t overlayColor = M5Cardputer.Display.color888(30, 30, 30);
  int x = 60, y = 35, w = 120, h = 65;

  M5Cardputer.Display.fillRect(x, y, w, h, overlayColor);
  M5Cardputer.Display.drawRect(x, y, w, h, WHITE);

  M5Cardputer.Display.setTextSize(1);
  for (int i = 0; i < mainMenuItemCount; i++) {
    M5Cardputer.Display.setCursor(55, 40 + i * 15);
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
  M5Cardputer.Display.println("W/S: Navigate, ENTER: Validate");
}

void drawDebugScreen() {
  switch (currentState) {
    case DEBUG_MODE:
      M5Cardputer.Display.fillScreen(BLACK);
      M5Cardputer.Display.setCursor(10, 10);
      M5Cardputer.Display.println("DEBUG_MODE");
      break;
    case CALIBRATION_1:
      drawImage(calib1);
      break;
    case CALIBRATION_2:
      drawImage(calib2);
      break;
    case CALIBRATION_3:
      drawImage(calib3);
      break;
  }
}

void drawHomeScreen() {
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setCursor(20, 20);
  M5Cardputer.Display.setTextColor(CYAN);
  M5Cardputer.Display.println("Natsumi Hasegawa");
  M5Cardputer.Display.println("-----------------");

  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.print("Age: ");
  M5Cardputer.Display.println(natsumi.age);
  M5Cardputer.Display.println(millis());

  M5Cardputer.Display.print("Hunger: ");
  M5Cardputer.Display.println(natsumi.hunger);
}

void drawContinueScreen() {
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setCursor(20, 60);
  M5Cardputer.Display.setTextColor(CYAN);
  M5Cardputer.Display.println("Continuing your game...");
}