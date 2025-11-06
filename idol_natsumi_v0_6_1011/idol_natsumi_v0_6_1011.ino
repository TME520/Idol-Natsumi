#include <M5Cardputer.h>
#include <SD.h>

// === Game state definitions ===
enum GameState {
  VERSION_SCREEN,
  M5_SCREEN,
  TITLE_SCREEN,
  CALIBRATION_1,
  CALIBRATION_2,
  CALIBRATION_3,
  NEW_GAME,
  CONTINUE_GAME,
  DEV_SCREEN,
  HOME_LOOP,
  FOOD_MENU,
  FOOD_EAT,
  FOOD_COOK,
  FOOD_REST,
  FOOD_ORDER,
  HEALTH_MENU,
  HEALTH_WASH,
  HEALTH_DOCTOR,
  HEALTH_TEMPLE,
  HEALTH_ONSEN,
  REST_MENU,
  REST_MEDITATE,
  REST_NAP,
  REST_BEDTIME,
  STATS_SCREEN,
  GARDEN_LOOP,
  TRAIN_MENU,
  TRAIN_SING,
  TRAIN_DANCE,
  TRAIN_SWIM,
  TRAIN_GYM,
  TRAIN_WALK,
  TRAIN_LIBRARY,
  COMP_MENU,
  COMP_LOCAL,
  COMP_DEPT,
  COMP_REG,
  COMP_NAT
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
  int spirit;
  int popularity;
  int performance;
  int fitness;
  int culture;
  int charm;
  unsigned long lastHungerUpdate = 0;
  unsigned long lastHygieneUpdate = 0;
  unsigned long lastEnergyUpdate = 0;
};

NatsumiStats natsumi;

// === Game Time Tracking ===
// 60000 milliseconds in a minute
// 86,400,000 milliseconds in a day
unsigned long agingInterval = 60000;  // 1 minute for testing
unsigned long sessionStart = 0;           // millis() when NEW_GAME starts
unsigned long playtimeTotalMs = 0;        // total playtime in ms (could persist later)
int lastAgeTick = 0;
const unsigned long shortWait = 1000;
const unsigned long mediumWait = 3000;
const unsigned long longWait = 6000;

unsigned long generalCounter = 0;

const unsigned long hungerInterval = 120000;   // 2 minutes
const unsigned long hygieneInterval = 240000;  // 4 minutes
const unsigned long energyInterval = 240000;   // 4 minutes

String currentMenuType = "main";
const char* mainMenuItems[] = {"0: NEW GAME", "1: CONTINUE", "2: DEV SCREEN"};
const char* homeMenuItems[] = {"0: STATS", "1: FOOD", "2: TRAINING", "3: COMPETITION", "4: HEALTH", "5: REST", "6: GARDEN", "7: DEBUG"};
const char* devMenuItems[] = {"0: CALIB1", "1: CALIB2", "2: CALIB3", "3: EXIT"};
const char* foodMenuItems[] = {"0: COOK", "1: RESTAURANT", "2: ORDER"};
const char* trainingMenuItems[] = {"0: SING", "1: DANCE", "2: SWIM", "3: GYM", "4: WALK", "5: LIBRARY"};
const char* competitionMenuItems[] = {"0: LOCAL", "1: DEPARTMENTAL", "2: REGIONAL", "3: NATIONAL"};
const char* healthMenuItems[] = {"0: WASH", "1: DOCTOR", "2: TEMPLE", "3: ONSEN"};
const char* restMenuItems[] = {"0: MEDITATE", "1: NAP", "2: BEDTIME"};
const char** currentMenuItems = nullptr;
const int mainMenuItemCount = 3;
const int homeMenuItemCount = 8;
const int devMenuItemCount = 4;
const int foodMenuItemCount = 3;
const int trainingMenuItemCount = 6;
const int competitionMenuItemCount = 4;
const int healthMenuItemCount = 4;
const int restMenuItemCount = 3;
int currentMenuItemsCount = 0;
int homeMenuSelection = 0;
int mainMenuSelection = 0;
int devMenuSelection = 0;
int foodMenuSelection = 0;
int trainingMenuSelection = 0;
int competitionMenuSelection = 0;
int healthMenuSelection = 0;
int restMenuSelection = 0;

bool l0NeedsRedraw = false; // Background
bool l1NeedsRedraw = false; // Character
bool l2NeedsRedraw = false; // Debug
bool l3NeedsRedraw = false; // Toast
bool l4NeedsRedraw = false; // Menu
bool statsNeedsRedraw = false;

bool debugEnabled = false;
bool menuOpened = false;
bool toastActive = false;

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
String versionNumber = "0.6.1011";

ImageBuffer currentBackground;
ImageBuffer calib1, calib2, calib3;
ImageBuffer currentCharacter;

// Toast messages
String toastMsg = "";
unsigned long toastUntil = 0;  // timestamp when toast should disappear
void showToast(const String& msg, unsigned long ms = mediumWait) {
  toastActive = true;
  toastMsg = msg;
  toastUntil = millis() + ms;
  l3NeedsRedraw = true;
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
  Serial.println("> Entering preloadImages() with currentState set to " + String(currentState));
  unloadAllImages();
  // Load backgrounds
  switch (currentState) {
    case M5_SCREEN:
      preloadImage("/idolnat/screens/m5_logo.png", currentBackground);
      break;
    case TITLE_SCREEN:
      preloadImage("/idolnat/screens/title01.png", currentBackground);
      break;
    case DEV_SCREEN:
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
    case FOOD_MENU:
      preloadImage("/idolnat/screens/kitchen.png", currentBackground);
      break;
    case FOOD_EAT:
      preloadImage("/idolnat/screens/kitchen.png", currentBackground);
      break;
    case FOOD_COOK:
      preloadImage("/idolnat/screens/kitchen.png", currentBackground);
      break;
    case FOOD_REST:
      preloadImage("/idolnat/screens/restaurant_bg.png", currentBackground);
      break;
    case FOOD_ORDER:
      preloadImage("/idolnat/screens/phone_app_food_order.png", currentBackground);
      break;
    case HEALTH_WASH:
      preloadImage("/idolnat/screens/bathroom.png", currentBackground);
      break;
    case REST_NAP:
      preloadImage("/idolnat/screens/bedroom.png", currentBackground);
      break;
    case GARDEN_LOOP:
      // preloadImage("/idolnat/screens/garden_bg.png", currentBackground);
      break;
    case STATS_SCREEN:
      preloadImage("/idolnat/screens/stats_bg.png", currentBackground);
      break;
    case TRAIN_MENU:
      preloadImage("/idolnat/screens/map_training.png", currentBackground);
      break;
    case TRAIN_SING:
      preloadImage("/idolnat/screens/singing_school_bg.png", currentBackground);
      break;
    case TRAIN_DANCE:
      preloadImage("/idolnat/screens/ballet_school_bg.png", currentBackground);
      break;
    case TRAIN_SWIM:
      preloadImage("/idolnat/screens/swimming_pool_bg.png", currentBackground);
      break;
    case TRAIN_GYM:
      preloadImage("/idolnat/screens/gym_bg.png", currentBackground);
      break;
    case TRAIN_WALK:
      preloadImage("/idolnat/screens/forest_bg.png", currentBackground);
      break;
    case TRAIN_LIBRARY:
      preloadImage("/idolnat/screens/library_bg.png", currentBackground);
      break;
    case COMP_MENU:
      preloadImage("/idolnat/screens/competition.png", currentBackground);
      break;
    case HEALTH_MENU:
      preloadImage("/idolnat/screens/bathroom.png", currentBackground);
      break;
    case REST_MENU:
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

  Serial.println("l0NeedsRedraw: " + String(l0NeedsRedraw) + " - l1NeedsRedraw: " + String(l1NeedsRedraw) + " - l2NeedsRedraw: " + String(l2NeedsRedraw) + " - l3NeedsRedraw: " + String(l3NeedsRedraw) + " - l4NeedsRedraw: " + String(l4NeedsRedraw));
  Serial.println("debugEnabled: " + String(debugEnabled) + " - menuOpened: " + String(menuOpened) + " - toastActive: " + String(toastActive));

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
  Serial.println("> Entering changeState() with baseLayer set to " + String(baseLayer) + " and targetState set to " + String(targetState));
  generalCounter = 0;
  // Manage state transitions
  switch (baseLayer) {
    case 0:
      l0NeedsRedraw = true;
      statsNeedsRedraw = true;
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
    case M5_SCREEN:
      screenConfig = IDLE;
      break;
    case VERSION_SCREEN:
      screenConfig = TEXT;
      break;
    case TITLE_SCREEN:
      screenConfig = CARD;
      currentMenuType = "main";
      currentMenuItems = mainMenuItems;
      currentMenuItemsCount = mainMenuItemCount;
      menuOpened = true;
      break;
    case NEW_GAME: case CONTINUE_GAME: case CALIBRATION_1: case CALIBRATION_2: case CALIBRATION_3:
      screenConfig = CARD;
      break;
    case DEV_SCREEN:
      screenConfig = CARD;
      currentMenuType = "dev";
      currentMenuItems = devMenuItems;
      currentMenuItemsCount = devMenuItemCount;
      break;
    case HOME_LOOP:
      screenConfig = ROOM;
      currentMenuType = "home";
      currentMenuItems = homeMenuItems;
      currentMenuItemsCount = homeMenuItemCount;
      break;
    case FOOD_EAT: case HEALTH_WASH: case REST_NAP:
      screenConfig = ROOM;
      currentMenuType = "food";
      currentMenuItems = foodMenuItems;
      currentMenuItemsCount = foodMenuItemCount;
      break;
    case STATS_SCREEN:
      screenConfig = GAME;
      break;
    case FOOD_MENU:
      screenConfig = ROOM;
      currentMenuType = "food";
      currentMenuItems = foodMenuItems;
      currentMenuItemsCount = foodMenuItemCount;
      break;
    case FOOD_COOK:
      screenConfig = ROOM;
      break;
    case FOOD_REST:
      screenConfig = ROOM;
      break;
    case FOOD_ORDER:
      screenConfig = ROOM;
      break;
    case TRAIN_MENU:
      screenConfig = ROOM;
      currentMenuType = "training";
      currentMenuItems = trainingMenuItems;
      currentMenuItemsCount = trainingMenuItemCount;
      break;
    case TRAIN_SING:
      screenConfig = ROOM;
      break;
    case TRAIN_DANCE:
      screenConfig = ROOM;
      break;
    case TRAIN_SWIM:
      screenConfig = ROOM;
      break;
    case TRAIN_GYM:
      screenConfig = ROOM;
      break;
    case TRAIN_WALK:
      screenConfig = ROOM;
      break;
    case TRAIN_LIBRARY:
      screenConfig = ROOM;
      break;
    case COMP_MENU:
      screenConfig = ROOM;
      currentMenuType = "competition";
      currentMenuItems = competitionMenuItems;
      currentMenuItemsCount = competitionMenuItemCount;
      break;
    case HEALTH_MENU:
      screenConfig = ROOM;
      currentMenuType = "health";
      currentMenuItems = healthMenuItems;
      currentMenuItemsCount = healthMenuItemCount;
      break;
    case REST_MENU:
      screenConfig = ROOM;
      currentMenuType = "rest";
      currentMenuItems = restMenuItems;
      currentMenuItemsCount = restMenuItemCount;
      break;
    case GARDEN_LOOP:
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
  int currentAge = natsumi.age;
 
  natsumi.ageMilliseconds = currentPlaytime;
  // Serial.print("natsumi.age: ");
  // Serial.println(natsumi.age);
  if (natsumi.ageMilliseconds < agingInterval) {
    // 11yo
    natsumi.age = 11;
  } else if ((natsumi.ageMilliseconds >= agingInterval) && (natsumi.ageMilliseconds < (agingInterval * 2))) {
    // 12yo
    natsumi.age = 12;
  } else if ((natsumi.ageMilliseconds >= (agingInterval * 2)) && (natsumi.ageMilliseconds < (agingInterval * 3))) {
    // 13yo
    natsumi.age = 13;
  } else if ((natsumi.ageMilliseconds >= (agingInterval * 3)) && (natsumi.ageMilliseconds < (agingInterval * 4))) {
    // 14yo
    natsumi.age = 14;
  } else if ((natsumi.ageMilliseconds >= (agingInterval * 4)) && (natsumi.ageMilliseconds < (agingInterval * 5))) {
    // 15yo
    natsumi.age = 15;
  } else if ((natsumi.ageMilliseconds >= (agingInterval * 5)) && (natsumi.ageMilliseconds < (agingInterval * 6))) {
    // 16yo
    natsumi.age = 16;
  } else if ((natsumi.ageMilliseconds >= (agingInterval * 6)) && (natsumi.ageMilliseconds < (agingInterval * 7))) {
    // 17yo
    natsumi.age = 17;
  } else if ((natsumi.ageMilliseconds >= (agingInterval * 7)) && (natsumi.ageMilliseconds < (agingInterval * 8))) {
    // 18yo
    natsumi.age = 18;
  } else if ((natsumi.ageMilliseconds >= (agingInterval * 8)) && (natsumi.ageMilliseconds < (agingInterval * 9))) {
    // 19yo
    natsumi.age = 19;
  } else if ((natsumi.ageMilliseconds >= (agingInterval * 9)) && (natsumi.ageMilliseconds < (agingInterval * 10))) {
    // 20yo
    natsumi.age = 20;
  } else if ((natsumi.ageMilliseconds >= (agingInterval * 10)) && (natsumi.ageMilliseconds < (agingInterval * 11))) {
    // 21 yo
    natsumi.age = 21;
  } else if ((natsumi.ageMilliseconds >= (agingInterval * 11)) && (natsumi.ageMilliseconds < (agingInterval * 12))) {
    // 22yo - Game ends
    natsumi.age = 22;
  }
  // Serial.print("natsumi.age: ");
  // Serial.println(natsumi.age);
  if (natsumi.age > currentAge) {
    // Load updated portrait
    preloadImages();
    showToast(String("Natsumi turned ") + natsumi.age + " years old!");
    statsNeedsRedraw=true;
  }
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
    statsNeedsRedraw=true;
  }

  // Hygiene decreases every 4 minutes
  if (currentMillis - natsumi.lastHygieneUpdate >= hygieneInterval) {
    if (natsumi.hygiene > 0) natsumi.hygiene--;
    natsumi.lastHygieneUpdate = currentMillis;
    Serial.print("Hygiene decreased: ");
    Serial.println(natsumi.hygiene);
    statsNeedsRedraw=true;
  }

  // Energy decreases every 4 minutes
  if (currentMillis - natsumi.lastEnergyUpdate >= energyInterval) {
    if (natsumi.energy > 0) natsumi.energy--;
    natsumi.lastEnergyUpdate = currentMillis;
    Serial.print("Energy decreased: ");
    Serial.println(natsumi.energy);
    statsNeedsRedraw=true;
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
  l1NeedsRedraw = false;
  l3NeedsRedraw = false;
  switch (currentState) {
    case TITLE_SCREEN:
      break;
    case NEW_GAME:
      natsumi.age = 11;
      natsumi.ageMilliseconds = 0;
      natsumi.hunger = 4;
      natsumi.hygiene = 4;
      natsumi.energy = 4;
      natsumi.spirit = 4;
      natsumi.popularity = 0;
      natsumi.performance = 0;
      natsumi.fitness = 0;
      natsumi.culture = 0;
      natsumi.charm = 0;
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
      natsumi.spirit = 4;
      natsumi.popularity = 0;
      natsumi.performance = 0;
      natsumi.fitness = 0;
      natsumi.culture = 0;
      natsumi.charm = 0;
      natsumi.lastHungerUpdate = 0;
      natsumi.lastHygieneUpdate = 0;
      natsumi.lastEnergyUpdate = 0;
      playtimeTotalMs = 0;
      sessionStart = millis();
      lastAgeTick = 0;
      changeState(0, HOME_LOOP);
      break;
    case DEV_SCREEN:
      break;
    default:
      break;
  }
  drawBackground(currentBackground);
  drawDebug();
  int *selectionPtr;
  if (currentMenuType == "home") {
    selectionPtr = &homeMenuSelection;
  } else if (currentMenuType == "dev") {
    selectionPtr = &devMenuSelection;
  } else {
    selectionPtr = &mainMenuSelection;
  }
  drawMenu(currentMenuType, currentMenuItems, currentMenuItemsCount, *selectionPtr);
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
    case STATS_SCREEN:
      manageStats();
      break;
    default:
      playGame();
      break;
  }
  updateAging();
  updateStats();
}

void manageIdle() {
  // Manage IDLE screens
  switch (currentState) {
    case M5_SCREEN:
      displayM5Logo();
      changeState(0, TITLE_SCREEN);
      break;
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
    case FOOD_EAT:
      eat();
      break;
    case HEALTH_WASH:
      wash();
      break;
    case REST_NAP:
      rest();
      break;
    case GARDEN_LOOP:
      manageGarden();
      break;
    case FOOD_MENU:
      menuOpened = true;
      break;
    case FOOD_COOK:
      cookFood();
      break;
    case FOOD_REST:
      gotoRestaurant();
      break;
    case FOOD_ORDER:
      orderFood();
      break;
    case TRAIN_MENU:
      menuOpened = true;
      break;
    case COMP_MENU:
      menuOpened = true;
      break;
    case HEALTH_MENU:
      menuOpened = true;
      break;
    case REST_MENU:
      menuOpened = true;
      break;
    default:
      break;
  }

  // After state-specific updates, draw all layers for room screens
  drawBackground(currentBackground);
  drawCharacter();
  drawDebug();
  drawToast();

  int *selectionPtr;
  if (currentMenuType == "home") {
    selectionPtr = &homeMenuSelection;
  } else if (currentMenuType == "dev") {
    selectionPtr = &devMenuSelection;
  } else {
    selectionPtr = &mainMenuSelection;
  }
  drawMenu(currentMenuType, currentMenuItems, currentMenuItemsCount, *selectionPtr);
}

void manageText() {
  // Manage TEXT screens
  switch (currentState) {
    case VERSION_SCREEN:
      displayVersionScreen();
      changeState(0, M5_SCREEN);
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

void displayM5Logo() {
  drawBackground(currentBackground);
  delay(mediumWait);
}

void manageHomeScreen() {
  // Serial.println("> Entering manageHomeScreen()");
  // Serial.print("natsumi.age: ");
  // Serial.println(natsumi.age);
  // Serial.print("currentAge: ");
  // Serial.println(currentAge);
  updateAging();
  updateStats();
}

void manageGarden() {
  // Serial.println("> Entering manageGarden()");
  updateAging();
  updateStats();
}

void eat() {
  // Serial.println("> Entering eat()");
  if (generalCounter==0) {
    if (natsumi.hunger < 4) {
      natsumi.hunger += 1;
      showToast("Ate (+1 Hunger)");
    } else {
      showToast("Natsumi is not hungry");
    }
  }
  generalCounter += 1;
  if (generalCounter >= 100) {
    changeState(0, HOME_LOOP);
  }
}

void wash() {
  if (generalCounter==0) {
    if (natsumi.hygiene < 4) {
      natsumi.hygiene += 1;
      showToast("Washed (+1 Hygiene)");
    } else {
      showToast("Natsumi is not dirty");
    }
  }
  generalCounter += 1;
  if (generalCounter >= 100) {
    changeState(0, HOME_LOOP);
  }
}

void rest() {
  if (generalCounter==0) {
    if (natsumi.energy < 4) {
      natsumi.energy += 1;
      showToast("Rested (+1 Energy)");
    } else {
      showToast("Natsumi is not tired");
    }
  }
  generalCounter += 1;
  if (generalCounter >= 100) {
    changeState(0, HOME_LOOP);
  }
}

// === Draw functions ===
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
    drawText(String("Memory: ") + ESP.getFreeHeap(), 80, 10, false, WHITE, 1);
    drawText(String("Time: ") + natsumi.ageMilliseconds, 80, 20, false, WHITE, 1);
    drawText(String("Age: ") + natsumi.age + " y.o.", 80, 30, false, WHITE, 1);
    drawText(String("Hunger: ") + natsumi.hunger, 80, 40, false, WHITE, 1);
    drawText(String("Hygiene: ") + natsumi.hygiene, 80, 50, false, WHITE, 1);
    drawText(String("Energy: ") + natsumi.energy, 80, 60, false, WHITE, 1);
    drawText(String("Spirit: ") + natsumi.spirit, 80, 70, false, WHITE, 1);
    drawText(String("Popularity: ") + natsumi.popularity, 80, 80, false, WHITE, 1);
    drawText(String("Performance: ") + natsumi.performance, 80, 90, false, WHITE, 1);
    drawText(String("Fitness: ") + natsumi.fitness, 80, 100, false, WHITE, 1);
    drawText(String("Culture: ") + natsumi.culture, 80, 110, false, WHITE, 1);
    drawText(String("Charm: ") + natsumi.charm, 80, 120, false, WHITE, 1);
    l2NeedsRedraw = true;
    l3NeedsRedraw = true;
    l4NeedsRedraw = true;
  } else {
    l2NeedsRedraw = false;
  }
}

void drawToast() {
  // Draw toast messages (layer 3)
  if (toastActive) {
    if (millis() > toastUntil) {
      // Toast expired
      toastActive = false;
      l0NeedsRedraw = true;
      l3NeedsRedraw = false;
    }
  }
  if (l3NeedsRedraw && toastActive) {
    const int tx = 120;  // center X (screen is 240 wide in landscape)
    const int ty = 100;  // near bottom for 135px height
    M5Cardputer.Display.fillRect(0, ty - 8, 240, 18, BLACK); // clear strip
    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.drawString(toastMsg, tx, ty);
    l3NeedsRedraw = false;
    l4NeedsRedraw = true;
  } else {
    l3NeedsRedraw = false;
  }
}

void drawMenu(String menuType, const char* items[], int itemCount, int &selection) {
  // Draw menus on the screen (layer 4)
  uint8_t key = 0;
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      key = M5Cardputer.Keyboard.getKey(keyList[0]);
    }
  }

  if (menuType == "home") {
    switch (key) {
      case 48:
        // 0: STATS
        changeState(0, STATS_SCREEN);
        menuOpened = false;
        break;
      case 49:
        // 1: FOOD
        changeState(0, FOOD_MENU);
        menuOpened = true;
        break;
      case 50:
        // 2: TRAINING
        changeState(0, TRAIN_MENU);
        menuOpened = true;
        break;
      case 51:
        // 3: COMPETITION
        changeState(0, COMP_MENU);
        menuOpened = true;
        break;
      case 52:
        // 4: HEALTH
        changeState(0, HEALTH_MENU);
        menuOpened = true;
        break;
      case 53:
        // 5: REST
        changeState(0, REST_MENU);
        menuOpened = true;
        break;
      case 54:
        // 6: GARDEN
        changeState(0, GARDEN_LOOP);
        menuOpened = false;
        break;
      case 55:
        // 7: DEBUG
        if (debugEnabled) {
          debugEnabled = false;
          l0NeedsRedraw = true;
          l2NeedsRedraw = false;
        } else {
          debugEnabled = true;
          l2NeedsRedraw = true;
        }
        break;
      case 43:
        // TAB
        if (menuOpened) {
          changeState(0, HOME_LOOP);
          menuOpened = false;
          l0NeedsRedraw = true;
        } else {
          menuOpened = true;
          l4NeedsRedraw = true;
        }
        break;
      case 96:
        // ESC
        changeState(0, HOME_LOOP);
        if (menuOpened) {
          menuOpened = false;
          l0NeedsRedraw = true;
        }
        break;
      case 181: case 'w': case 'W': case 59:
        // UP
        selection = (selection - 1 + homeMenuItemCount) % homeMenuItemCount;
        l4NeedsRedraw = true;
        break;
      case 182: case 's': case 'S': case 46:
        // DOWN
        selection = (selection + 1) % homeMenuItemCount;
        l4NeedsRedraw = true;
        break;
      case 13: case 40: case ' ':
        // VALIDATE
        if (selection == 0) {
          changeState(0, STATS_SCREEN);
        } else if (selection == 1) {
          changeState(0, FOOD_MENU);
        } else if (selection == 2) {
          changeState(0, TRAIN_MENU);
        } else if (selection == 3) {
          changeState(0, COMP_MENU);
        } else if (selection == 4) {
          changeState(0, HEALTH_MENU);
        } else if (selection == 5) {
          changeState(0, REST_MENU);
        } else if (selection == 6) {
          changeState(0, GARDEN_LOOP);
        } else if (selection == 7) {
          if (debugEnabled) {
            debugEnabled = false;
            l0NeedsRedraw = true;
            l2NeedsRedraw = false;
          } else {
            debugEnabled = true;
            l2NeedsRedraw = true;
          }
        }
        menuOpened = false;
        break;
    }
  } else if (menuType == "food") {
    switch (key) {
      case 48:
        // 0: COOK
        changeState(0, FOOD_COOK);
        menuOpened = false;
        break;
      case 49:
        // 1: RESTAURANT
        changeState(0, FOOD_REST);
        menuOpened = false;
        break;
      case 50:
        // 2: ORDER
        changeState(0, FOOD_ORDER);
        menuOpened = false;
        break;
      case 43:
        // TAB
        if (menuOpened) {
          changeState(0, HOME_LOOP);
          menuOpened = false;
          l0NeedsRedraw = true;
        } else {
          menuOpened = true;
          l4NeedsRedraw = true;
        }
        break;
      case 96:
        // ESC
        changeState(0, HOME_LOOP);
        if (menuOpened) {
          menuOpened = false;
          l0NeedsRedraw = true;
        }
        break;
      case 181: case 'w': case 'W': case 59:
        // UP
        selection = (selection - 1 + foodMenuItemCount) % foodMenuItemCount;
        l4NeedsRedraw = true;
        break;
      case 182: case 's': case 'S': case 46:
        // DOWN
        selection = (selection + 1) % foodMenuItemCount;
        l4NeedsRedraw = true;
        break;
      case 13: case 40: case ' ':
        // VALIDATE
        if (selection == 0) {
          changeState(0, FOOD_COOK);
        } else if (selection == 1) {
          changeState(0, FOOD_REST);
        } else if (selection == 2) {
          changeState(0, FOOD_ORDER);
        } else if (selection == 3) {
          if (debugEnabled) {
            debugEnabled = false;
            l0NeedsRedraw = true;
            l2NeedsRedraw = false;
          } else {
            debugEnabled = true;
            l2NeedsRedraw = true;
          }
        }
        menuOpened = false;
        break;
    }
  } else if (menuType == "training") {
    switch (key) {
      case 48:
        // 0: SING
        changeState(0, TRAIN_SING);
        menuOpened = false;
        break;
      case 49:
        // 1: DANCE
        changeState(0, TRAIN_DANCE);
        menuOpened = false;
        break;
      case 50:
        // 2: SWIM
        changeState(0, TRAIN_SWIM);
        menuOpened = false;
        break;
      case 51:
        // 3: GYM
        changeState(0, TRAIN_GYM);
        menuOpened = false;
        break;
      case 52:
        // 4: WALK
        changeState(0, TRAIN_WALK);
        menuOpened = false;
        break;
      case 53:
        // 5: LIBRARY
        changeState(0, TRAIN_LIBRARY);
        menuOpened = false;
        break;
      case 55:
        // 7: DEBUG
        if (debugEnabled) {
          debugEnabled = false;
          l0NeedsRedraw = true;
          l2NeedsRedraw = false;
        } else {
          debugEnabled = true;
          l2NeedsRedraw = true;
        }
        break;
      case 43:
        // TAB
        if (menuOpened) {
          changeState(0, HOME_LOOP);
          menuOpened = false;
          l0NeedsRedraw = true;
        } else {
          menuOpened = true;
          l4NeedsRedraw = true;
        }
        break;
      case 96:
        // ESC
        changeState(0, HOME_LOOP);
        if (menuOpened) {
          menuOpened = false;
          l0NeedsRedraw = true;
        }
        break;
      case 181: case 'w': case 'W': case 59:
        // UP
        selection = (selection - 1 + trainingMenuItemCount) % trainingMenuItemCount;
        l4NeedsRedraw = true;
        break;
      case 182: case 's': case 'S': case 46:
        // DOWN
        selection = (selection + 1) % trainingMenuItemCount;
        l4NeedsRedraw = true;
        break;
      case 13: case 40: case ' ':
        // VALIDATE
        if (selection == 0) {
          changeState(0, TRAIN_SING);
        } else if (selection == 1) {
          changeState(0, TRAIN_DANCE);
        } else if (selection == 2) {
          changeState(0, TRAIN_SWIM);
        } else if (selection == 3) {
          changeState(0, TRAIN_GYM);
        } else if (selection == 4) {
          changeState(0, TRAIN_WALK);
        } else if (selection == 5) {
          changeState(0, TRAIN_LIBRARY);
        } else if (selection == 7) {
          if (debugEnabled) {
            debugEnabled = false;
            l0NeedsRedraw = true;
            l2NeedsRedraw = false;
          } else {
            debugEnabled = true;
            l2NeedsRedraw = true;
          }
        }
        menuOpened = false;
        break;
    }
  } else if (menuType == "competition") {
    switch (key) {
      case 48:
        // 0: LOCAL
        changeState(0, COMP_LOCAL);
        menuOpened = false;
        break;
      case 49:
        // 1: DEPARTMENTAL
        changeState(0, COMP_DEPT);
        menuOpened = false;
        break;
      case 50:
        // 2: REGIONAL
        changeState(0, COMP_REG);
        menuOpened = false;
        break;
      case 51:
        // 3: NATIONAL
        changeState(0, COMP_NAT);
        menuOpened = false;
        break;
      case 55:
        // 7: DEBUG
        if (debugEnabled) {
          debugEnabled = false;
          l0NeedsRedraw = true;
          l2NeedsRedraw = false;
        } else {
          debugEnabled = true;
          l2NeedsRedraw = true;
        }
        break;
      case 43:
        // TAB
        if (menuOpened) {
          changeState(0, HOME_LOOP);
          menuOpened = false;
          l0NeedsRedraw = true;
        } else {
          menuOpened = true;
          l4NeedsRedraw = true;
        }
        break;
      case 96:
        // ESC
        changeState(0, HOME_LOOP);
        if (menuOpened) {
          menuOpened = false;
          l0NeedsRedraw = true;
        }
        break;
      case 181: case 'w': case 'W': case 59:
        // UP
        selection = (selection - 1 + competitionMenuItemCount) % competitionMenuItemCount;
        l4NeedsRedraw = true;
        break;
      case 182: case 's': case 'S': case 46:
        // DOWN
        selection = (selection + 1) % competitionMenuItemCount;
        l4NeedsRedraw = true;
        break;
      case 13: case 40: case ' ':
        // VALIDATE
        if (selection == 0) {
          changeState(0, COMP_LOCAL);
        } else if (selection == 1) {
          changeState(0, COMP_DEPT);
        } else if (selection == 2) {
          changeState(0, COMP_REG);
        } else if (selection == 3) {
          changeState(0, COMP_NAT);
        } else if (selection == 7) {
          if (debugEnabled) {
            debugEnabled = false;
            l0NeedsRedraw = true;
            l2NeedsRedraw = false;
          } else {
            debugEnabled = true;
            l2NeedsRedraw = true;
          }
        }
        menuOpened = false;
        break;
    }
  } else if (menuType == "health") {
    switch (key) {
      case 48:
        // 0: WASH
        changeState(0, HEALTH_WASH);
        menuOpened = false;
        break;
      case 49:
        // 1: DOCTOR
        changeState(0, HEALTH_DOCTOR);
        menuOpened = false;
        break;
      case 50:
        // 2: TEMPLE
        changeState(0, HEALTH_TEMPLE);
        menuOpened = false;
        break;
      case 51:
        // 3: ONSEN
        changeState(0, HEALTH_ONSEN);
        menuOpened = false;
        break;
      case 55:
        // 7: DEBUG
        if (debugEnabled) {
          debugEnabled = false;
          l0NeedsRedraw = true;
          l2NeedsRedraw = false;
        } else {
          debugEnabled = true;
          l2NeedsRedraw = true;
        }
        break;
      case 43:
        // TAB
        if (menuOpened) {
          changeState(0, HOME_LOOP);
          menuOpened = false;
          l0NeedsRedraw = true;
        } else {
          menuOpened = true;
          l4NeedsRedraw = true;
        }
        break;
      case 96:
        // ESC
        changeState(0, HOME_LOOP);
        if (menuOpened) {
          menuOpened = false;
          l0NeedsRedraw = true;
        }
        break;
      case 181: case 'w': case 'W': case 59:
        // UP
        selection = (selection - 1 + healthMenuItemCount) % healthMenuItemCount;
        l4NeedsRedraw = true;
        break;
      case 182: case 's': case 'S': case 46:
        // DOWN
        selection = (selection + 1) % healthMenuItemCount;
        l4NeedsRedraw = true;
        break;
      case 13: case 40: case ' ':
        // VALIDATE
        if (selection == 0) {
          changeState(0, HEALTH_WASH);
        } else if (selection == 1) {
          changeState(0, HEALTH_DOCTOR);
        } else if (selection == 2) {
          changeState(0, HEALTH_TEMPLE);
        } else if (selection == 3) {
          changeState(0, HEALTH_ONSEN);
        } else if (selection == 7) {
          if (debugEnabled) {
            debugEnabled = false;
            l0NeedsRedraw = true;
            l2NeedsRedraw = false;
          } else {
            debugEnabled = true;
            l2NeedsRedraw = true;
          }
        }
        menuOpened = false;
        break;
    }
  } else if (menuType == "rest") {
    switch (key) {
      case 48:
        // 0: MEDITATE
        changeState(0, REST_MEDITATE);
        menuOpened = false;
        break;
      case 49:
        // 1: NAP
        changeState(0, REST_NAP);
        menuOpened = false;
        break;
      case 50:
        // 2: BEDTIME
        changeState(0, REST_BEDTIME);
        menuOpened = false;
        break;
      case 55:
        // 7: DEBUG
        if (debugEnabled) {
          debugEnabled = false;
          l0NeedsRedraw = true;
          l2NeedsRedraw = false;
        } else {
          debugEnabled = true;
          l2NeedsRedraw = true;
        }
        break;
      case 43:
        // TAB
        if (menuOpened) {
          changeState(0, HOME_LOOP);
          menuOpened = false;
          l0NeedsRedraw = true;
        } else {
          menuOpened = true;
          l4NeedsRedraw = true;
        }
        break;
      case 96:
        // ESC
        changeState(0, HOME_LOOP);
        if (menuOpened) {
          menuOpened = false;
          l0NeedsRedraw = true;
        }
        break;
      case 181: case 'w': case 'W': case 59:
        // UP
        selection = (selection - 1 + restMenuItemCount) % restMenuItemCount;
        l4NeedsRedraw = true;
        break;
      case 182: case 's': case 'S': case 46:
        // DOWN
        selection = (selection + 1) % restMenuItemCount;
        l4NeedsRedraw = true;
        break;
      case 13: case 40: case ' ':
        // VALIDATE
        if (selection == 0) {
          changeState(0, REST_MEDITATE);
        } else if (selection == 1) {
          changeState(0, REST_NAP);
        } else if (selection == 2) {
          changeState(0, REST_BEDTIME);
        } else if (selection == 7) {
          if (debugEnabled) {
            debugEnabled = false;
            l0NeedsRedraw = true;
            l2NeedsRedraw = false;
          } else {
            debugEnabled = true;
            l2NeedsRedraw = true;
          }
        }
        menuOpened = false;
        break;
    }
  } else if (menuType == "dev") {
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
        changeState(0, TITLE_SCREEN);
        if (menuOpened) {
          menuOpened = true;
        }
        break;
      case 181: case 'w': case 'W': case 59:
        // UP
        selection = (selection - 1 + devMenuItemCount) % devMenuItemCount;
        l4NeedsRedraw = true;
        break;
      case 182: case 's': case 'S': case 46:
        // DOWN
        selection = (selection + 1) % devMenuItemCount;
        l4NeedsRedraw = true;
        break;
      case 13: case 40: case ' ':
        // VALIDATE
        if (selection == 0) {
          changeState(0, CALIBRATION_1);
          menuOpened = false;
        } else if (selection == 1) {
          changeState(0, CALIBRATION_2);
          menuOpened = false;
        } else if (selection == 2) {
          changeState(4, CALIBRATION_3);
          menuOpened = false;
        } else if (selection == 3) {
          changeState(0, TITLE_SCREEN);
          menuOpened = true;
        }
        break;
    }
  } else if (menuType == "main") {
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
        return;
      case 181: case 'w': case 'W': case 59:
        // UP
        selection = (selection - 1 + mainMenuItemCount) % mainMenuItemCount;
        l4NeedsRedraw = true;
        break;
      case 182: case 's': case 'S': case 46:
        // DOWN
        selection = (selection + 1) % mainMenuItemCount;
        l4NeedsRedraw = true;
        break;
      case 13: case 40: case ' ':
        // VALIDATE
        if (selection == 0) {
          changeState(0, NEW_GAME);
          menuOpened = false;
        } else if (selection == 1) {
          changeState(0, CONTINUE_GAME);
          menuOpened = false;
        } else {
          changeState(4, DEV_SCREEN);
          menuOpened = true;
          return;
        }

    }
  }
  if (!menuOpened) {
    l4NeedsRedraw = false;
    return;
  }

  if (l4NeedsRedraw) {
    const int x = 60;
    const int w = 120;
    const int padding = 8;
    // const int lineSpacing = 14;
    const int lineSpacing = 10;
    const int h = padding * 2 + ((itemCount - 1) * lineSpacing);
    const int screenHeight = 135;
    const int topMargin = 8;
    const int bottomMargin = 6;
    int y = (screenHeight - h) / 2;

    if (y < topMargin) {
      y = topMargin;
    }

    int maxY = screenHeight - bottomMargin - h;
    if (y > maxY) {
      y = maxY;
    }

    if (y < 0) {
      y = 0;
    }

    M5Cardputer.Display.fillRect(x, y, w, h, TFT_NAVY);
    M5Cardputer.Display.drawRect(x, y, w, h, WHITE);

    M5Cardputer.Display.setTextSize(1);
    for (int i = 0; i < itemCount; i++) {
      M5Cardputer.Display.setCursor(x + padding + 5, y + padding + (i * lineSpacing));
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
}

void playGame() {
  // Play one of the mini-games
  M5Cardputer.Display.fillRect(0, 125, 240, 10, BLACK);
  drawText("Mini-game", 120, 131, true, WHITE, 1);
}

void drawStats() {
  // Draw the Status Board / Statistics screen
  static unsigned long lastDraw = 0;
  unsigned long now = millis();
    if (now - lastDraw < 120 && (l0NeedsRedraw || l1NeedsRedraw || l3NeedsRedraw)) {
    return;
  }
  lastDraw = now;

  const int cardX = 8;
  const int cardY = 8;
  const int cardW = 224;
  const int cardH = 119;

  uint16_t shadowColor = M5Cardputer.Display.color565(10, 14, 32);
  uint16_t panelColor = M5Cardputer.Display.color565(20, 28, 64);
  uint16_t accentColor = M5Cardputer.Display.color565(120, 170, 255);

  M5Cardputer.Display.fillRoundRect(cardX + 3, cardY + 4, cardW, cardH, 12, shadowColor);
  M5Cardputer.Display.fillRoundRect(cardX, cardY, cardW, cardH, 12, panelColor);
  M5Cardputer.Display.drawRoundRect(cardX, cardY, cardW, cardH, 12, accentColor);

  M5Cardputer.Display.setTextColor(TFT_WHITE, panelColor);
  M5Cardputer.Display.setTextDatum(middle_center);
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.drawString("Status Board", cardX + cardW / 2, cardY + 16);

  /* M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setTextColor(accentColor, panelColor);
  M5Cardputer.Display.drawString(String("Age: ") + natsumi.age + " yrs", cardX + cardW / 2, cardY + 32);
  M5Cardputer.Display.drawFastHLine(cardX + 16, cardY + 35, cardW - 32, accentColor); */

  struct StatEntry {
    const char* name;
    int value;
    int maxValue;
    uint16_t color;
  };

  StatEntry stats[] = {
    {"Hunger", natsumi.hunger, 4, M5Cardputer.Display.color565(255, 149, 64)},
    {"Hygiene", natsumi.hygiene, 4, M5Cardputer.Display.color565(64, 224, 208)},
    {"Energy", natsumi.energy, 4, M5Cardputer.Display.color565(102, 255, 128)},
    {"Spirit", natsumi.spirit, 4, M5Cardputer.Display.color565(255, 99, 255)},
    {"Popularity", natsumi.popularity, 4, M5Cardputer.Display.color565(255, 234, 102)},
    {"Performance", natsumi.performance, 4, M5Cardputer.Display.color565(102, 163, 255)},
    {"Fitness", natsumi.fitness, 4, M5Cardputer.Display.color565(140, 255, 182)},
    {"Culture", natsumi.culture, 4, M5Cardputer.Display.color565(178, 130, 255)},
    {"Charm", natsumi.charm, 4, M5Cardputer.Display.color565(255, 163, 210)},
    {"Age", natsumi.age, 120, M5Cardputer.Display.color565(255, 149, 64)}
  };

  const int statsCount = sizeof(stats) / sizeof(stats[0]);
  const int barHeight = 8;
  const int spacing = 17;
  const int columnWidth = (cardW - 36) / 2;
  const int columnGap = columnWidth + 12;
  const int firstColumnCount = (statsCount + 1) / 2;
  // int startY = cardY + 38;
  int startY = cardY + 28;

  for (int i = 0; i < statsCount; ++i) {
    int column = (i < firstColumnCount) ? 0 : 1;
    int rowIndex = (column == 0) ? i : (i - firstColumnCount);
    int x = cardX + 12 + column * columnGap;
    int rowY = startY + rowIndex * spacing;
    drawStatBar(stats[i].name, stats[i].value, stats[i].maxValue, x, rowY, columnWidth, barHeight, stats[i].color, panelColor, accentColor);
  }

  M5Cardputer.Display.setTextDatum(top_left);
  M5Cardputer.Display.setTextSize(1);
}

void drawStatBar(const String &label, int value, int maxValue, int x, int y, int width, int barHeight, uint16_t barColor, uint16_t bgColor, uint16_t frameColor) {
  if (maxValue <= 0) {
    maxValue = 1;
  }

  int clampedValue = value;
  if (clampedValue < 0) {
    clampedValue = 0;
  }
  if (clampedValue > maxValue) {
    clampedValue = maxValue;
  }

  const int labelHeight = 8;
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setTextDatum(top_left);
  M5Cardputer.Display.setTextColor(TFT_WHITE, bgColor);
  M5Cardputer.Display.drawString(label, x, y);

  String valueStr = String(clampedValue) + "/" + String(maxValue);
  M5Cardputer.Display.setTextDatum(top_right);
  M5Cardputer.Display.setTextColor(frameColor, bgColor);
  M5Cardputer.Display.drawString(valueStr, x + width, y);

  const int barY = y + labelHeight;
  M5Cardputer.Display.fillRoundRect(x, barY, width, barHeight, 3, M5Cardputer.Display.color565(30, 30, 40));
  M5Cardputer.Display.drawRoundRect(x, barY, width, barHeight, 3, frameColor);

  int innerWidth = width - 4;
  int filled = (innerWidth * clampedValue) / maxValue;
  if (filled < 0) {
    filled = 0;
  }
  if (filled > innerWidth) {
    filled = innerWidth;
  }

  if (filled > 0) {
    M5Cardputer.Display.fillRoundRect(x + 2, barY + 2, filled, barHeight - 4, 2, barColor);
    // glossy highlight line
    uint16_t highlight = M5Cardputer.Display.color565(220, 220, 255);
    int highlightWidth = filled;
    if (highlightWidth > 0) {
      M5Cardputer.Display.drawFastHLine(x + 2, barY + 2, highlightWidth, highlight);
    }
  }
}

void manageStats() {
  uint8_t key = 0;
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      key = M5Cardputer.Keyboard.getKey(keyList[0]);
      changeState(0, HOME_LOOP);
    }
  }
  if (statsNeedsRedraw) {
    drawStats();
    statsNeedsRedraw=false;
  }
}

void cookFood() {
  // FOOD_COOK
}

void gotoRestaurant() {
  // FOOD_REST
}

void orderFood() {
  // FOOD_ORDER
}
