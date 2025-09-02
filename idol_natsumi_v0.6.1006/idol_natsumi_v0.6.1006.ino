#include <M5Cardputer.h>
#include <SD.h>

// === Game state definitions ===
enum GameState {
  VERSION_SCREEN,
  TITLE_SCREEN,
  CALIBRATION_1,
  CALIBRATION_2,
  CALIBRATION_3,
  NEW_GAME,
  CONTINUE_GAME,
  DEV_SCREEN,
  HOME_LOOP,
  ACTION_MENU,
  ACTION_EAT,
  ACTION_WASH,
  ACTION_REST
};

GameState currentState = VERSION_SCREEN;

// === Screen configs definitions ===
enum ScreenState {
  CARD,
  DIALOG,
  GAME,
  IDLE,
  ROOM,
  TEXT
};

ScreenState screenConfig = TEXT;

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
const unsigned long shortWait = 1000;
const unsigned long mediumWait = 3000;
const unsigned long longWait = 6000;

const unsigned long hungerInterval = 120000;   // 2 minutes
const unsigned long hygieneInterval = 240000;  // 4 minutes
const unsigned long energyInterval = 240000;   // 4 minutes

String currentMenuType = "main";
const char* mainMenuItems[] = {"0: NEW GAME", "1: CONTINUE", "2: DEV SCREEN"};
const char* actionMenuItems[] = {"0: EAT", "1: WASH", "2: REST", "3: DEBUG"};
const char* devMenuItems[] = {"0: CALIB1", "1: CALIB2", "2: CALIB3", "3: EXIT"};
const char** currentMenuItems = nullptr;
const int mainMenuItemCount = 3;
const int actionMenuItemCount = 4;
const int devMenuItemCount = 4;
int currentMenuItemsCount = 0;
int actionMenuSelection = 0;
int mainMenuSelection = 0;
int devMenuSelection = 0;
int currentMenuSelection = 0;

bool l0NeedsRedraw = false; // Background
bool l1NeedsRedraw = false; // Character
bool l2NeedsRedraw = false; // Debug
bool l3NeedsRedraw = false; // Toast
bool l4NeedsRedraw = false; // Menu

bool debugEnabled = false;
bool menuOpened = false;

unsigned long lastUpdate = 0;
const int FRAME_DELAY = 50;
unsigned long lastKeyTime = 0;
const unsigned long keyCooldown = 200;  // milliseconds between accepted presses

// === Image preload system ===
struct ImageBuffer {
  uint8_t* data = nullptr;
  size_t length = 0;
};

String copyright = "(c) 2025 - Pantzumatic";
String versionNumber = "0.6.1006";

ImageBuffer currentBackground;
ImageBuffer calib1, calib2, calib3;
ImageBuffer currentCharacter;

// Toast messages
String toastMsg = "";
unsigned long toastUntil = 0;  // timestamp when toast should disappear
void showToast(const String& msg, unsigned long ms = mediumWait) {
  toastMsg = msg;
  toastUntil = millis() + ms;
  l3NeedsRedraw = true;
}

void manageToast() {
  // Serial.println("> Entering manageToast()");
  if (l3NeedsRedraw==true) {
    if (millis() < toastUntil) {
      drawToast();
    } else {
      l3NeedsRedraw = false;
    }
  }
}

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

bool getKeyOnce(uint8_t &key) {
  if (millis() - lastKeyTime < keyCooldown) {
    return false;
  }
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      key = M5Cardputer.Keyboard.getKey(keyList[0]);
      lastKeyTime = millis();
      return true;
    }
  }
  return false;
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
    drawText("SD load image failed!", 120, 131, true, RED, 1); // centered
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

void unloadImage(ImageBuffer &imgBuf) {
  Serial.println("> Entering unloadImage()");
  if (imgBuf.data) {
    free(imgBuf.data);
    imgBuf.data = nullptr;
    imgBuf.length = 0;

    // Debug info
    Serial.print("Image unloaded, free heap: ");
    Serial.println(ESP.getFreeHeap());
  }
}

// Frees all preloaded image buffers to reclaim memory
void unloadAllImages() {
  size_t heapBefore = ESP.getFreeHeap();

  unloadImage(currentBackground);
  unloadImage(currentCharacter);

  // currentBackground points to one of the room images, so just reset it
  currentBackground.data = nullptr;
  currentBackground.length = 0;

  size_t heapAfter = ESP.getFreeHeap();
  Serial.print("Unloaded all images. Freed bytes: ");
  Serial.print(heapAfter - heapBefore);
  Serial.print(", free heap now: ");
  Serial.println(heapAfter);
}

void preloadImages() {
  // unloadAllImages();
  // Load backgrounds
  switch (currentState) {
    case TITLE_SCREEN:
      preloadImage("/idolnat/screens/title01.png", currentBackground);
      break;
    case CALIBRATION_1:
      preloadImage("/idolnat/screens/setup_3tiers_busybar.png", currentBackground);
      break;
    case CALIBRATION_2:
      preloadImage("/idolnat/screens/setup_menubox.png", currentBackground);
      break;
    case CALIBRATION_3:
      preloadImage("/idolnat/screens/setup_dialog.png", currentBackground);
      break;
    case HOME_LOOP:
      preloadImage("/idolnat/screens/lounge.png", currentBackground);
      break;
    case ACTION_EAT:
      preloadImage("/idolnat/screens/kitchen.png", currentBackground);
      break;
    case ACTION_WASH:
      preloadImage("/idolnat/screens/bathroom.png", currentBackground);
      break;
    case ACTION_REST:
      preloadImage("/idolnat/screens/bedroom.png", currentBackground);
      break;
  }
  // Load portraits
  switch(natsumi.age) {
    case 11: case 12:
      preloadImage("/idolnat/sprites/natsumi_11yo-90x135.png", currentCharacter);
      break;
    case 13: case 14:
      preloadImage("/idolnat/sprites/natsumi_13yo-90x135.png", currentCharacter);
      break;
    case 15: case 16: case 17:
      preloadImage("/idolnat/sprites/natsumi_15yo-90x135.png", currentCharacter);
      break;
    case 18: case 19: case 20:
      preloadImage("/idolnat/sprites/natsumi_18yo-90x135.png", currentCharacter);
      break;
    case 21: case 22:
      preloadImage("/idolnat/sprites/natsumi_21yo-90x135.png", currentCharacter);
      break;
    default:
      preloadImage("/idolnat/sprites/natsumi_21yo-90x135.png", currentCharacter);
      break;
  }
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
    drawText("SD init failed!", 120, 131, true, RED, 1); // centered
    Serial.println("SD init failed!");
    while (true);
  }
}

void loop() {
  M5Cardputer.update();

  if (millis() - lastUpdate < FRAME_DELAY) return;
  lastUpdate = millis();

  switch (screenConfig) {
    case CARD:
      manageCard();
      break;
    case DIALOG:
      /*
      Dialog between Natsumi and NPC
      Background: 1 x bitmap
      Character: Natsumi + NPC
      Debug: Available
      Toast: None
      Menu: None
      Interactive (timer + keypress + escape)
      */
      manageDialog();
      /*
      drawBackground(currentBackground);
      drawCharacter();
      drawDebug();
      */
      break;
    case GAME:
      /*
      Mini-games
      Background: None
      Character: None
      Debug: Available
      Toast: None
      Menu: None
      Interactive (timer + keypress + escape)
      */
      manageGame();
      break;
    case IDLE:
      /*
      Idle mode, minimal screen activity
      Background: None
      Character: Natsumi
      Debug: Available
      Toast: Yes
      Menu: None
      Interactive (escape)
      */
      manageIdle();
      /*
      drawCharacter();
      drawDebug();
      drawToast();
      */
      break;
    case ROOM:
      /*
      Location: Bathroom, Bedroom, Kitchen, Lounge
      Background: None
      Character: None
      Debug: Available
      Toast: Yes
      Menu: Yes
      Interactive (timer + keypress + escape)
      */
      manageRoom();
      /*
      drawBackground(currentBackground);
      drawCharacter();
      drawDebug();
      drawToast();
      drawMenu();
      */
      break;
    case TEXT:
      /*
      Transition text (version)
      Background: None
      Character: None
      Debug: None
      Toast: None
      Menu: None
      Non-interactive (timer)
      */
      manageText();
      break;
    default:
      break;
  }
}

// === Menu and state logic ===
void changeState(int baseLayer, GameState targetState) {
  // Manage state transitions
  switch (baseLayer) {
    case 0:
      l0NeedsRedraw = true;
      break;
    case 1:
      l1NeedsRedraw = true;
      break;
    case 2:
      l2NeedsRedraw = true;
      break;
    case 3:
      l3NeedsRedraw = true;
      break;
    case 4:
      l4NeedsRedraw = true;
      break;
    default:
      l0NeedsRedraw = true;
      break;
  }
  switch (targetState) {
    case VERSION_SCREEN:
      screenConfig = TEXT;
      break;
    case TITLE_SCREEN: case NEW_GAME: case CONTINUE_GAME: case DEV_SCREEN: case CALIBRATION_1: case CALIBRATION_2: case CALIBRATION_3:
      screenConfig = CARD;
      break;
    case HOME_LOOP: case ACTION_EAT: case ACTION_WASH: case ACTION_REST:
      screenConfig = ROOM;
      break;
    default:
      break;
  }
  currentState = targetState;
  preloadImages();
}

void updateAging() {
  // Serial.println("> Entering updateAging()");
  unsigned long currentMilli = millis();
  unsigned long currentPlaytime = currentMilli - sessionStart;
  unsigned long totalMs = playtimeTotalMs + currentPlaytime;
 
  natsumi.ageMilliseconds = currentPlaytime;
  // Serial.print("natsumi.age: ");
  // Serial.println(natsumi.age);
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
  // Serial.print("natsumi.age: ");
  // Serial.println(natsumi.age);
}

void updateStats() {
  // Serial.println("> Entering updateStats()");
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

void manageCard() {
  // Manage CARD screens
  /*
  Transition bitmap (loading screen, narration, debug...)
  Background: 1 x bitmap
  Character: None
  Debug: Available
  Toast: None
  Menu: None
  Interactive (timer + keypress)
  */
  switch (currentState) {
    case TITLE_SCREEN:
      currentMenuType = "main";
      currentMenuItems = mainMenuItems;
      currentMenuItemsCount = mainMenuItemCount;
      currentMenuSelection = mainMenuSelection;
      break;
    case NEW_GAME:
      natsumi.age = 11;
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
      changeState(0, HOME_LOOP);
      break;
    case CONTINUE_GAME:
      natsumi.age = 11;
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
      changeState(0, HOME_LOOP);
      break;
    case DEV_SCREEN:
      currentMenuType = "dev";
      currentMenuItems = devMenuItems;
      currentMenuItemsCount = devMenuItemCount;
      currentMenuSelection = devMenuSelection;
      break;
    default:
      break;
  }
  drawBackground(currentBackground);
  drawDebug();
  drawMenu(currentMenuType, currentMenuItems, currentMenuItemsCount, currentMenuSelection);
}

void manageDialog() {
  // Manage DIALOG screens
  switch (currentState) {
    default:
      break;
  }
}

void manageGame() {
  // Manage GAME screens
  switch (currentState) {
    default:
      playGame();
      break;
  }
}

void manageIdle() {
  // Manage IDLE screens
  switch (currentState) {
    default:
      break;
  }
}

void manageRoom() {
  // Manage ROOM screens
  switch (currentState) {
    case HOME_LOOP:
      manageHomeScreen();
      break;
    case ACTION_EAT:
      eat();
      break;
    case ACTION_WASH:
      wash();
      break;
    case ACTION_REST:
      rest();
      break;
    default:
      break;
  }
}

void manageText() {
  // Manage TEXT screens
  switch (currentState) {
    case VERSION_SCREEN:
      displayVersionScreen();
      changeState(0, TITLE_SCREEN);
      break;
    default:
      break;
  }
}

void displayBlackScreen() {
  M5Cardputer.Display.fillScreen(BLACK);
}

void displayVersionScreen() {
  // Serial.println("> Entering displayVersionScreen()");
  displayBlackScreen();
  drawText("IDOL NATSUMI", 120, 30, true, RED, 3); // centered
  drawText("for M5 Cardputer", 120, 50, true, BLUE, 2); // centered
  drawText(copyright, 120, 100, true, WHITE, 1); // centered
  drawText(versionNumber, 120, 110, true, WHITE, 1); // centered
  delay(mediumWait);
}

void manageHomeScreen() {
  // Serial.println("> Entering manageHomeScreen()");
  int currentAge = natsumi.age;
  // Serial.print("natsumi.age: ");
  // Serial.println(natsumi.age);
  // Serial.print("currentAge: ");
  // Serial.println(currentAge);
  updateAging();
  updateStats();
  if (natsumi.age > currentAge) {
    // Load updated portrait
    preloadImages();
    showToast(String("Natsumi turned ") + natsumi.age + " years old!");
  }
}

void manageDevScreen() {
  uint8_t key;
  if (getKeyOnce(key)) {
    drawText("Key " + String(key) + " pressed...", 120, 131, true, BLUE, 1);
    switch (currentState) {
      case DEV_SCREEN:
        if (key == 43) {
          changeState(0, CALIBRATION_1);
        }
        break;
      case CALIBRATION_1:
        if (key == 43) {
          changeState(0, CALIBRATION_2);
        }
        break;
      case CALIBRATION_2:
        if (key == 43) {
          changeState(0, CALIBRATION_3);
        }
        break;
      case CALIBRATION_3:
        if (key == 43) {
          changeState(0, TITLE_SCREEN);
        }
        break;
      default:
        break;
    }
  }
}

// === Draw functions ===
void drawDevSCreen() {
  switch (currentState) {
    case DEV_SCREEN:
      displayBlackScreen();
      drawText("DEV_SCREEN", 30, 30, false, ORANGE, 3);
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

void eat() {
  if (natsumi.hunger < 4) natsumi.hunger += 1;
  showToast("Ate (+1 Hunger)");
  changeState(0, HOME_LOOP);
}

void wash() {
  if (natsumi.hygiene < 4) natsumi.hygiene += 1;
  showToast("Washed (+1 Hygiene)");
  changeState(0, HOME_LOOP);
}

void rest() {
  if (natsumi.energy < 4) natsumi.energy += 1;
  showToast("Rested (+1 Energy)");
  changeState(0, HOME_LOOP);
}

void drawBackground(const ImageBuffer& bg) {
  // Draw the background of the screen (layer 0)
  if (l0NeedsRedraw) {
    drawImage(bg);
    l0NeedsRedraw = false;
    l1NeedsRedraw = true;
    l2NeedsRedraw = true;
    l3NeedsRedraw = true;
    l4NeedsRedraw = true;
  }
}

void drawCharacter() {
  // Draw the character(s) on the screen (layer 1)
  if (l1NeedsRedraw) {
    drawImage(currentCharacter);
    l1NeedsRedraw = false;
    l2NeedsRedraw = true;
    l3NeedsRedraw = true;
    l4NeedsRedraw = true;
  }
}

void drawDebug() {
  // Draw debug information (layer 2)
  if (l2NeedsRedraw && debugEnabled) {
    drawText(String("Memory: ") + ESP.getFreeHeap(), 80, 40, false, WHITE, 1);
    drawText(String("Time: ") + natsumi.ageMilliseconds, 80, 50, false, WHITE, 1);
    drawText(String("Age: ") + natsumi.age + " y.o.", 80, 60, false, WHITE, 1);
    drawText(String("Hunger: ") + natsumi.hunger, 80, 70, false, WHITE, 1);
    drawText(String("Hygiene: ") + natsumi.hygiene, 80, 80, false, WHITE, 1);
    drawText(String("Energy: ") + natsumi.energy, 80, 90, false, WHITE, 1);
    drawText(String("Skill: ") + natsumi.skill, 80, 100, false, WHITE, 1);
    drawText(String("Mood: ") + natsumi.mood, 80, 110, false, WHITE, 1);
    drawText(String("Popularity: ") + natsumi.popularity, 80, 120, false, WHITE, 1);
    l2NeedsRedraw = false;
    l3NeedsRedraw = true;
    l4NeedsRedraw = true;
  }
}

void drawToast() {
  // Draw toast messages (layer 3)
  if (l3NeedsRedraw) {
    const int tx = 120;  // center X (screen is 240 wide in landscape)
    const int ty = 100;  // near bottom for 135px height
    M5Cardputer.Display.fillRect(0, ty - 8, 240, 18, BLACK); // clear strip
    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.drawString(toastMsg, tx, ty);
    l3NeedsRedraw = false;
    l4NeedsRedraw = true;
  }
}

void drawMenu(String menuType, const char* items[], int itemCount, int selection) {
  // Draw menus on the screen (layer 4)
  if (l4NeedsRedraw || (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed())) {
    uint16_t overlayColor = M5Cardputer.Display.color888(30, 30, 30);
    int x = 60, y = 35, w = 120, h = 65;

    M5Cardputer.Display.fillRect(x, y, w, h, overlayColor);
    M5Cardputer.Display.drawRect(x, y, w, h, WHITE);

    M5Cardputer.Display.setTextSize(1);
    for (int i = 0; i < itemCount; i++) {
      M5Cardputer.Display.setCursor(65, 45 + i * 15);
      if (i == selection) {
        M5Cardputer.Display.setTextColor(YELLOW);
        M5Cardputer.Display.print("> ");
      } else {
        M5Cardputer.Display.setTextColor(WHITE);
        M5Cardputer.Display.print("  ");
      }
      M5Cardputer.Display.println(items[i]);
    }

    // Helper text at the bottom
    M5Cardputer.Display.fillRect(0, 125, 240, 10, BLACK);
    drawText("UP/DOWN: Navigate, ENTER: Validate", 120, 131, true, WHITE, 1);
    l4NeedsRedraw = false;
  }

  // Keyboard management
  if (menuType == "action") {
    uint8_t key;
    if (getKeyOnce(key)) {
      switch (key) {
        case 48:
          // 0: EAT
          changeState(0, ACTION_EAT);
          menuOpened = false;
          break;
        case 49:
          // 1: WASH
          changeState(0, ACTION_WASH);
          menuOpened = false;
          break;
        case 50:
          // 2: REST
          changeState(0, ACTION_REST);
          menuOpened = false;
          break;
        case 51:
          // 3: DEBUG
          if (debugEnabled) {
            debugEnabled = false;
          } else {
            debugEnabled = true;
          }
          menuOpened = false;
          break;
        case 43:
          // TAB
          if (menuOpened) {
            menuOpened = false;
            l0NeedsRedraw = true;
          } else {
            menuOpened = true;
            l4NeedsRedraw = true;
          }
          break;
        case 96:
          // ESC
          if (menuOpened) {
            menuOpened = false;
            l0NeedsRedraw = true;
          }
          break;
        case 181: case 'w': case 'W': case 59:
          // UP
          actionMenuSelection = (actionMenuSelection - 1 + actionMenuItemCount) % actionMenuItemCount;
          l4NeedsRedraw = true;
          break;
        case 182: case 's': case 'S': case 46:
          // DOWN
          actionMenuSelection = (actionMenuSelection + 1) % actionMenuItemCount;
          l4NeedsRedraw = true;
          break;
        case 13: case 40: case ' ':
          // VALIDATE
          if (actionMenuSelection == 0) {
            changeState(0, ACTION_EAT);
          } else if (actionMenuSelection == 1) {
            changeState(0, ACTION_WASH);
          } else if (actionMenuSelection == 2) {
            changeState(0, ACTION_REST);
          }
          menuOpened = false;
          break;
      }
    }
  } else if (menuType == "dev") {
    uint8_t key;
    if (getKeyOnce(key)) {
      switch (key) {
        case 48:
          // 0: CALIB1
          changeState(0, CALIBRATION_1);
          menuOpened = false;
          break;
        case 49:
          // 1: CALIB2
          changeState(0, CALIBRATION_2);
          menuOpened = false;
          break;
        case 50:
          // 2: CALIB3
          changeState(0, CALIBRATION_3);
          menuOpened = false;
          break;
        case 51:
          // 3: EXIT
          changeState(0, TITLE_SCREEN);
          menuOpened = true;
          break;
        case 43:
          // TAB
          if (menuOpened) {
            menuOpened = false;
            l0NeedsRedraw = true;
          } else {
            menuOpened = true;
            l4NeedsRedraw = true;
          }
          break;
        case 96:
          // ESC
          if (menuOpened) {
            changeState(0, TITLE_SCREEN);
            menuOpened = true;
          }
          break;
        case 181: case 'w': case 'W': case 59:
          // UP
          devMenuSelection = (devMenuSelection - 1 + devMenuItemCount) % devMenuItemCount;
          l4NeedsRedraw = true;
          break;
        case 182: case 's': case 'S': case 46:
          // DOWN
          devMenuSelection = (devMenuSelection + 1) % devMenuItemCount;
          l4NeedsRedraw = true;
          break;
        case 13: case 40: case ' ':
          // VALIDATE
          if (devMenuSelection == 0) {
            changeState(0, CALIBRATION_1);
            menuOpened = false;
          } else if (devMenuSelection == 1) {
            changeState(0, CALIBRATION_2);
            menuOpened = false;
          } else if (devMenuSelection == 2) {
            changeState(4, CALIBRATION_3);
            menuOpened = false;
          } else if (devMenuSelection == 3) {
            changeState(0, TITLE_SCREEN);
            menuOpened = true;
          }
          break;
      }
    }
  } else if (menuType == "main") {
    uint8_t key;
    if (getKeyOnce(key)) {
      switch (key) {
        case 48:
          // 0: NEW GAME
          changeState(0, NEW_GAME);
          menuOpened = false;
          break;
        case 49:
          // 1: CONTINUE
          changeState(0, CONTINUE_GAME);
          menuOpened = false;
          break;
        case 50:
          // 2: DEV SCREEN
          changeState(4, DEV_SCREEN);
          menuOpened = true;
          break;
        case 181: case 'w': case 'W': case 59:
          // UP
          mainMenuSelection = (mainMenuSelection - 1 + mainMenuItemCount) % mainMenuItemCount;
          l4NeedsRedraw = true;
          break;
        case 182: case 's': case 'S': case 46:
          // DOWN
          mainMenuSelection = (mainMenuSelection + 1) % mainMenuItemCount;
          l4NeedsRedraw = true;
          break;
        case 13: case 40: case ' ':
          // VALIDATE
          if (mainMenuSelection == 0) {
            changeState(0, NEW_GAME);
            menuOpened = false;
          } else if (mainMenuSelection == 1) {
            changeState(0, CONTINUE_GAME);
            menuOpened = false;
          } else {
            changeState(4, DEV_SCREEN);
            menuOpened = true;
          }
          break;
      }
    }
  }
}

void playGame() {
  // Play one of the mini-games
  M5Cardputer.Display.fillRect(0, 125, 240, 10, BLACK);
  drawText("Mini-game", 120, 131, true, WHITE, 1);
}
