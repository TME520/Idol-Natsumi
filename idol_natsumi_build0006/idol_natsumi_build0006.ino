#include <M5Cardputer.h>
#include <SD.h>


// === Game state definitions ===
enum GameState {
  VERSION_SCREEN,
  TITLE_SCREEN,
  CALIBRATION_1,
  CALIBRATION_2,
  CALIBRATION_3,
  MAIN_MENU,
  NEW_GAME,
  CONTINUE_GAME,
  DEBUG_MODE,
  HOME_LOOP,
  ACTION_MENU
};

GameState currentState = VERSION_SCREEN;

// === Player data ===
struct NatsumiStats {
  int age;
  unsigned long ageMilliseconds;
  int hunger;
  int hygiene;
  int energy;
  int skill;
  int mood;
  int popularity;
  unsigned long lastHungerUpdate = 0;
  unsigned long lastHygieneUpdate = 0;
  unsigned long lastEnergyUpdate = 0;
};

NatsumiStats natsumi;

// === Game Time Tracking ===
// 60000 milliseconds in a minute
// 86,400,000 milliseconds in a day
unsigned long agingIntervalMs = 60000;  // 1 minute for testing
unsigned long sessionStart = 0;           // millis() when NEW_GAME starts
unsigned long playtimeTotalMs = 0;        // total playtime in ms (could persist later)
int lastAgeTick = 0;
int shortWait = 1000;
int mediumWait = 3000;
int longWait = 6000;

const unsigned long hungerInterval = 120000;   // 2 minutes
const unsigned long hygieneInterval = 240000;  // 4 minutes
const unsigned long energyInterval = 240000;   // 4 minutes

const char* mainMenuItems[] = {"0: NEW GAME", "1: CONTINUE", "2: DEBUG"};
const char* actionMenuItems[] = {"0: EAT", "1: WASH", "2: REST", "3: BACK"};
const int mainMenuItemCount = 3;
int actionMenuSelection = 0;
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

String copyright = "(c) 2025 - Pantsumatic";
String versionNumber = "0.6.1002";

ImageBuffer titleImage;
ImageBuffer calib1, calib2, calib3;
ImageBuffer natsumi11, natsumi13, natsumi15, natsumi18, natsumi21;
ImageBuffer natsumi11age, natsumi13age, natsumi15age, natsumi18age, natsumi21age;

// === UI Helper Functions ===
void drawText(String text, int x, int y, bool centerAlign, uint16_t color = WHITE, int textSize = 2, uint16_t bgColor = BLACK) {
    M5Cardputer.Display.setTextSize(textSize);

    // Set alignment for size calculation
    if (centerAlign) {
        M5Cardputer.Display.setTextDatum(middle_center);
    } else {
        M5Cardputer.Display.setTextDatum(top_left);
    }

    // Measure text size
    int16_t textWidth = M5Cardputer.Display.textWidth(text);
    int16_t textHeight = M5Cardputer.Display.fontHeight();

    // Compute clear rectangle position based on alignment
    int clearX = x;
    int clearY = y;

    if (centerAlign) {
        clearX = x - textWidth / 2;
        clearY = y - textHeight / 2;
    }

    // Clear previous text area
    M5Cardputer.Display.fillRect(clearX, clearY, textWidth, textHeight, bgColor);

    // Draw new text
    M5Cardputer.Display.setTextColor(color, bgColor); // Text with background
    M5Cardputer.Display.drawString(text, x, y);
}

bool preloadImage(const char* path, ImageBuffer &imgBuf) {
  File f = SD.open(path, FILE_READ);
  if (!f) return false;
  Serial.print("SD load image: ");
  Serial.println(path);

  imgBuf.length = f.size();
  imgBuf.data = (uint8_t*)malloc(imgBuf.length);
  if (!imgBuf.data) {
    f.close();
    drawText("SD load image failed!", 120, 110, true, RED, 1); // centered
    Serial.println("SD load image failed!");
    return false;
  }

  f.read(imgBuf.data, imgBuf.length);
  f.close();

  // Show how much memory is left
  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());
  return true;
}

void preloadImages() {
  // screens
  preloadImage("/idolnat/screens/title01.png", titleImage);
  preloadImage("/idolnat/screens/setup_3tiers_busybar.png", calib1);
  preloadImage("/idolnat/screens/setup_menubox.png", calib2);
  preloadImage("/idolnat/screens/setup_dialog.png", calib3);
  // sprites
  /*
  preloadImage("/idolnat/sprites/natsumi_11yo-90x135.png", natsumi11);
  preloadImage("/idolnat/sprites/natsumi_13yo-90x135.png", natsumi13);
  preloadImage("/idolnat/sprites/natsumi_15yo-90x135.png", natsumi15);
  preloadImage("/idolnat/sprites/natsumi_18yo-90x135.png", natsumi18);
  preloadImage("/idolnat/sprites/natsumi_21yo-90x135.png", natsumi21);
  */
  preloadImage("/idolnat/sprites/natsumi_11yo-240x135.png", natsumi11age);
  preloadImage("/idolnat/sprites/natsumi_13yo-240x135.png", natsumi13age);
  preloadImage("/idolnat/sprites/natsumi_15yo-240x135.png", natsumi15age);
  preloadImage("/idolnat/sprites/natsumi_18yo-240x135.png", natsumi18age);
  preloadImage("/idolnat/sprites/natsumi_21yo-240x135.png", natsumi21age);
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
  delay(1000);
  Serial.begin(115200);
  delay(1000);
  Serial.println("");
  Serial.println("*** BEGIN ***");

  if (!SD.begin()) {
    drawText("SD init failed!", 120, 110, true, RED, 1); // centered
    Serial.println("SD init failed!");
    while (true);
  }

  preloadImages();

  currentState = VERSION_SCREEN;
  bgNeedsRedraw = true;
  fgNeedsRedraw = true;
}

void loop() {
  M5Cardputer.update();

  if (millis() - lastUpdate < FRAME_DELAY) return;
  lastUpdate = millis();

  switch (currentState) {
    case VERSION_SCREEN:
      manageVersionScreen();
      break;
    case TITLE_SCREEN:
      // Serial.println("> Title screen");
      manageTitleScreen();
      break;
    case NEW_GAME:
      natsumi.age = 0;
      natsumi.ageMilliseconds = 0;
      natsumi.hunger = 4;
      natsumi.hygiene = 4;
      natsumi.energy = 4;
      natsumi.skill = 0;
      natsumi.mood = 0;
      natsumi.popularity = 0;
      natsumi.lastHungerUpdate = 0;
      natsumi.lastHygieneUpdate = 0;
      natsumi.lastEnergyUpdate = 0;
      playtimeTotalMs = 0;
      sessionStart = millis();
      lastAgeTick = 0;
      bgNeedsRedraw = false;
      fgNeedsRedraw = true;
      currentState = HOME_LOOP;
      break;
    case CONTINUE_GAME:
      natsumi.age = 0;
      natsumi.ageMilliseconds = 0;
      natsumi.hunger = 4;
      natsumi.hygiene = 4;
      natsumi.energy = 4;
      natsumi.skill = 0;
      natsumi.mood = 0;
      natsumi.popularity = 0;
      natsumi.lastHungerUpdate = 0;
      natsumi.lastHygieneUpdate = 0;
      natsumi.lastEnergyUpdate = 0;
      playtimeTotalMs = 0;
      sessionStart = millis();
      lastAgeTick = 0;
      bgNeedsRedraw = false;
      fgNeedsRedraw = true;
      currentState = HOME_LOOP;
      break;
    case DEBUG_MODE: case CALIBRATION_1: case CALIBRATION_2: case CALIBRATION_3:
      manageDebugMode();
      break;
    case HOME_LOOP:
      manageHomeScreen();
      break;
  }
}

// === Menu and state logic ===

void updateAging() {
  Serial.println("> Entering updateAging()");
  unsigned long currentMilli = millis();
  unsigned long currentPlaytime = currentMilli - sessionStart;
  unsigned long totalMs = playtimeTotalMs + currentPlaytime;
 
  natsumi.ageMilliseconds = currentPlaytime;
  Serial.print("natsumi.age: ");
  Serial.println(natsumi.age);
  if (natsumi.ageMilliseconds < agingIntervalMs) {
    // 11yo
    natsumi.age = 11;
  } else if ((natsumi.ageMilliseconds >= agingIntervalMs) && (natsumi.ageMilliseconds < (agingIntervalMs * 2))) {
    // 12yo
    natsumi.age = 12;
  } else if ((natsumi.ageMilliseconds >= (agingIntervalMs * 2)) && (natsumi.ageMilliseconds < (agingIntervalMs * 3))) {
    // 13yo
    natsumi.age = 13;
  } else if ((natsumi.ageMilliseconds >= (agingIntervalMs * 3)) && (natsumi.ageMilliseconds < (agingIntervalMs * 4))) {
    // 14yo
    natsumi.age = 14;
  } else if ((natsumi.ageMilliseconds >= (agingIntervalMs * 4)) && (natsumi.ageMilliseconds < (agingIntervalMs * 5))) {
    // 15yo
    natsumi.age = 15;
  } else if ((natsumi.ageMilliseconds >= (agingIntervalMs * 5)) && (natsumi.ageMilliseconds < (agingIntervalMs * 6))) {
    // 16yo
    natsumi.age = 16;
  } else if ((natsumi.ageMilliseconds >= (agingIntervalMs * 6)) && (natsumi.ageMilliseconds < (agingIntervalMs * 7))) {
    // 17yo
    natsumi.age = 17;
  } else if ((natsumi.ageMilliseconds >= (agingIntervalMs * 7)) && (natsumi.ageMilliseconds < (agingIntervalMs * 8))) {
    // 18yo
    natsumi.age = 18;
  } else if ((natsumi.ageMilliseconds >= (agingIntervalMs * 8)) && (natsumi.ageMilliseconds < (agingIntervalMs * 9))) {
    // 19yo
    natsumi.age = 19;
  } else if ((natsumi.ageMilliseconds >= (agingIntervalMs * 9)) && (natsumi.ageMilliseconds < (agingIntervalMs * 10))) {
    // 20yo
    natsumi.age = 20;
  } else if ((natsumi.ageMilliseconds >= (agingIntervalMs * 10)) && (natsumi.ageMilliseconds < (agingIntervalMs * 11))) {
    // 21 yo
    natsumi.age = 21;
  } else if ((natsumi.ageMilliseconds >= (agingIntervalMs * 11)) && (natsumi.ageMilliseconds < (agingIntervalMs * 12))) {
    // 22yo - Game ends
    natsumi.age = 22;
  }
  Serial.print("natsumi.age: ");
  Serial.println(natsumi.age);
}

void updateStats() {
  Serial.println("> Entering updateStats()");
  unsigned long currentMillis = millis();

  // Hunger decreases every 2 minutes
  if (currentMillis - natsumi.lastHungerUpdate >= hungerInterval) {
    if (natsumi.hunger > 0) natsumi.hunger--;
    natsumi.lastHungerUpdate = currentMillis;
    Serial.print("Hunger decreased: ");
    Serial.println(natsumi.hunger);
  }

  // Hygiene decreases every 4 minutes
  if (currentMillis - natsumi.lastHygieneUpdate >= hygieneInterval) {
    if (natsumi.hygiene > 0) natsumi.hygiene--;
    natsumi.lastHygieneUpdate = currentMillis;
    Serial.print("Hygiene decreased: ");
    Serial.println(natsumi.hygiene);
  }

  // Energy decreases every 4 minutes
  if (currentMillis - natsumi.lastEnergyUpdate >= energyInterval) {
    if (natsumi.energy > 0) natsumi.energy--;
    natsumi.lastEnergyUpdate = currentMillis;
    Serial.print("Energy decreased: ");
    Serial.println(natsumi.energy);
  }
}

void manageVersionScreen() {
  Serial.println("> Entering manageVersionScreen()");
  M5Cardputer.Display.fillScreen(BLACK);
  drawText("IDOL NATSUMI", 120, 30, true, RED, 3); // centered
  drawText("for M5 Cardputer", 120, 50, true, BLUE, 2); // centered
  drawText(copyright, 120, 100, true, WHITE, 1); // centered
  drawText(versionNumber, 120, 110, true, WHITE, 1); // centered
  delay(mediumWait);
  currentState = TITLE_SCREEN;
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
          } else if (mainMenuSelection == 1) {
            currentState = CONTINUE_GAME;
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
  Serial.println("> Entering manageHomeScreen()");
  int currentAge = natsumi.age;
  Serial.print("natsumi.age: ");
  Serial.println(natsumi.age);
  Serial.print("currentAge: ");
  Serial.println(currentAge);
  updateAging();
  updateStats();
  Serial.print("natsumi.age: ");
  Serial.println(natsumi.age);
  Serial.print("currentAge: ");
  Serial.println(currentAge);
  if (natsumi.age > currentAge) {
    bgNeedsRedraw = true;
  }
  // bgNeedsRedraw = true;
  fgNeedsRedraw = true;
  if (bgNeedsRedraw) {
    drawHomeScreen();
    bgNeedsRedraw = false;
  }
  if (fgNeedsRedraw) {
    drawHomeStats();
    fgNeedsRedraw = false;
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
      drawText("Key " + String(key) + " pressed...", 120, 110, true, WHITE, 1); // centered
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
    M5Cardputer.Display.setCursor(65, 45 + i * 15);
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
      drawText("DEBUG_MODE", 30, 30, false, ORANGE, 3);
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
  switch(natsumi.age) {
    case 11: case 12:
      drawImage(natsumi11age);
      break;
    case 13: case 14:
      drawImage(natsumi13age);
      break;
    case 15: case 16: case 17:
      drawImage(natsumi15age);
      break;
    case 18: case 19: case 20:
      drawImage(natsumi18age);
      break;
    case 21: case 22:
      drawImage(natsumi21age);
      break;
    default:
      break;
  }
  drawText("Natsumi Hasegawa", 20, 20, false, CYAN, 2);
}

void drawHomeStats() {
  drawText(String("Time: ") + natsumi.ageMilliseconds, 80, 50, false, WHITE, 1);
  drawText(String("Age: ") + natsumi.age + " y.o.", 80, 60, false, WHITE, 1);
  drawText(String("Hunger: ") + natsumi.hunger, 80, 70, false, WHITE, 1);
  drawText(String("Hygiene: ") + natsumi.hygiene, 80, 80, false, WHITE, 1);
  drawText(String("Energy: ") + natsumi.energy, 80, 90, false, WHITE, 1);
  drawText(String("Skill: ") + natsumi.skill, 80, 100, false, WHITE, 1);
  drawText(String("Mood: ") + natsumi.mood, 80, 110, false, WHITE, 1);
  drawText(String("Popularity: ") + natsumi.popularity, 80, 120, false, WHITE, 1);
}

void eat() { if (natsumi.hunger < 4) natsumi.hunger += 1; }
void wash() { if (natsumi.hygiene < 4) natsumi.hygiene += 1; }
void rest() { if (natsumi.energy < 4) natsumi.energy += 1; }
