#include <M5Cardputer.h>
#include <SD.h>
#include <vector>

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
  HEALTH_WASH2,
  HEALTH_DOCTOR,
  HEALTH_DOCTOR2,
  HEALTH_DOCTOR3,
  HEALTH_DOCTOR4,
  HEALTH_DOCTOR5,
  HEALTH_DOCTOR6,
  HEALTH_TEMPLE,
  HEALTH_TEMPLE2,
  HEALTH_TEMPLE3,
  HEALTH_TEMPLE4,
  HEALTH_TEMPLE5,
  HEALTH_TEMPLE6,
  HEALTH_ONSEN,
  REST_MENU,
  REST_MEDITATE,
  REST_SLEEP,
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
GameState doctorState = HOME_LOOP;
GameState priestState = HOME_LOOP;

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
int spiritScore = 0;
const unsigned long microWait = 60;
const unsigned long shortWait = 200;
const unsigned long mediumWait = 3200;
const unsigned long longWait = 6400;

// Onsen mini-game helpers
void resetBathGame();
void manageBathGame();
void drawBathStaticLayout();
void clearBathSlider(int y);
void drawBathSlider(int y);
void finalizeBathOutcome(String outcomeText);
void startBathGame();

unsigned long changeStateCounter = 0;

const unsigned long hungerInterval = 120000;   // 2 minutes
const unsigned long hygieneInterval = 240000;  // 4 minutes
const unsigned long energyInterval = 240000;   // 4 minutes
const unsigned long meditateInterval = 300000;   // 5 minutes
const unsigned long fiveSecondInterval = 5000;  // 5 seconds
const int STAT_MAX = 4;
unsigned long meditateStart = 0;
unsigned long lastMeditationRedraw = 0;
unsigned long lastFiveSecondTick = 0;

String currentMenuType = "main";
const char* mainMenuItems[] = {"0: NEW GAME", "1: CONTINUE", "2: DEV SCREEN"};
const char* homeMenuItems[] = {"0: STATS", "1: FOOD", "2: TRAINING", "3: COMPETITION", "4: HEALTH", "5: REST", "6: GARDEN", "7: DEBUG"};
const char* devMenuItems[] = {"0: CALIB1", "1: CALIB2", "2: CALIB3", "3: EXIT"};
const char* foodMenuItems[] = {"0: FRIDGE", "1: RESTAURANT", "2: ORDER"};
const char* trainingMenuItems[] = {"0: SING", "1: DANCE", "2: SWIM", "3: GYM", "4: WALK", "5: LIBRARY"};
const char* competitionMenuItems[] = {"0: LOCAL", "1: DEPARTMENTAL", "2: REGIONAL", "3: NATIONAL"};
const char* healthMenuItems[] = {"0: WASH", "1: DOCTOR", "2: TEMPLE", "3: ONSEN"};
const char* restMenuItems[] = {"0: MEDITATE", "1: SLEEP"};
const char** currentMenuItems = nullptr;
const int mainMenuItemCount = 3;
const int homeMenuItemCount = 8;
const int devMenuItemCount = 4;
const int foodMenuItemCount = 3;
const int trainingMenuItemCount = 6;
const int competitionMenuItemCount = 4;
const int healthMenuItemCount = 4;
const int restMenuItemCount = 2;
int currentMenuItemsCount = 0;
int homeMenuSelection = 0;
int mainMenuSelection = 0;
int devMenuSelection = 0;
int foodMenuSelection = 0;
int trainingMenuSelection = 0;
int competitionMenuSelection = 0;
int healthMenuSelection = 0;
int restMenuSelection = 0;
int lastSleepEnergyDisplayed = -1;
int lastMeditationDisplayed = 0;

bool l0NeedsRedraw = false; // Background
bool l1NeedsRedraw = false; // Character
bool l2NeedsRedraw = false; // Debug
bool l3NeedsRedraw = false; // Toast
bool l4NeedsRedraw = false; // Menu
bool l5NeedsRedraw = false; // Overlay (user-defined)

bool backgroundEnabled = false;
bool characterEnabled = false;
bool debugEnabled = false;
bool toastEnabled = false;
bool menuEnabled = false;
bool overlayEnabled = false;
bool helperEnabled = false;

bool menuOpened = false;
bool debugActive = false;
bool toastActive = false;
bool overlayActive = false;

bool meditationActive = false;
bool meditationRewardApplied = false;
bool fiveSecondPulse = false;  // Set true by updateFiveSecondPulse() every five seconds

// Onsen state
unsigned long onsenTicks = 0;  // Number of 5-second pulses spent in the onsen
int onsenStartEnergy = 0;
int onsenStartSpirit = 0;
int lastOnsenEnergyDisplayed = -1;
int lastOnsenSpiritDisplayed = -1;

unsigned long lastUpdate = 0;
const int FRAME_DELAY = 50;
unsigned long lastKeyTime = 0;
const unsigned long keyCooldown = 200;  // milliseconds between accepted presses

// Onsen mini-game state
bool bathGameRunning = false;
bool bathBackgroundDrawn = false;
bool bathResultShown = false;
unsigned long bathGameStart = 0;
unsigned long bathOutcomeTime = 0;
const unsigned long bathGameDuration = 8000;  // milliseconds
const unsigned long bathExitDelay = 3000;      // milliseconds to display the result
const int thermometerX = 200;
const int thermometerY = 18;
const int thermometerWidth = 20;
const int thermometerHeight = 100;
const int thermometerInnerPadding = 2;
/*
const int sliderHeight = 10;
const int sliderStep = 2;
const unsigned long sliderUpdateInterval = 35;
*/
const int sliderHeight = 6;
const int sliderStep = 6;
const unsigned long sliderUpdateInterval = 20;
// const int idealZoneHeight = 26;
const int idealZoneHeight = 20;
const int idealZoneY = thermometerY + thermometerInnerPadding + ((thermometerHeight - thermometerInnerPadding * 2 - idealZoneHeight) / 2);
int sliderYPosition = thermometerY + thermometerHeight - sliderHeight;
int sliderDirection = -1;  // -1 = moving up, 1 = moving down
unsigned long lastSliderUpdate = 0;

// === Image preload system ===
struct ImageBuffer {
  uint8_t* data = nullptr;
  size_t length = 0;
};

String copyright = "(c) 2025 - Pantzumatic";
String versionNumber = "0.6.1012";

ImageBuffer currentBackground;
ImageBuffer calib1, calib2, calib3;
ImageBuffer currentCharacter;

// Toast messages
String toastMsg = "";
unsigned long toastUntil = 0;  // timestamp when toast should disappear
void showToast(const String& msg, unsigned long ms = longWait) {
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

const char* onsenBackgroundForAge(int age) {
  if (age <= 12) {
    return "/idolnat/screens/onsen_11yo.png";
  } else if (age <= 14) {
    return "/idolnat/screens/onsen_13yo.png";
  } else if (age <= 17) {
    return "/idolnat/screens/onsen_15yo.png";
  } else if (age <= 20) {
    return "/idolnat/screens/onsen_18yo.png";
  }
  return "/idolnat/screens/onsen_21yo.png";
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
      preloadImage("/idolnat/screens/fridge_open.png", currentBackground);
      break;
    case FOOD_REST:
      preloadImage("/idolnat/screens/restaurant_bg.png", currentBackground);
      break;
    case FOOD_ORDER:
      preloadImage("/idolnat/screens/phone_app_food_order.png", currentBackground);
      break;
    case HEALTH_WASH: case HEALTH_WASH2:
      preloadImage("/idolnat/screens/bathroom.png", currentBackground);
      break;
    case HEALTH_DOCTOR: case HEALTH_DOCTOR2: case HEALTH_DOCTOR6:
      preloadImage("/idolnat/screens/doctors_office_bg.png", currentBackground);
      break;
    case HEALTH_DOCTOR3:
      preloadImage("/idolnat/screens/doctor_step1.png", currentBackground);
      break;
    case HEALTH_DOCTOR4:
      preloadImage("/idolnat/screens/doctor_step2.png", currentBackground);
      break;
    case HEALTH_DOCTOR5:
      preloadImage("/idolnat/screens/doctor_step3.png", currentBackground);
      break;
    case HEALTH_TEMPLE: case HEALTH_TEMPLE2: case HEALTH_TEMPLE6:
      preloadImage("/idolnat/screens/temple_bg.png", currentBackground);
      break;
    case HEALTH_TEMPLE3:
      preloadImage("/idolnat/screens/priest_step1.png", currentBackground);
      break;
    case HEALTH_TEMPLE4:
      preloadImage("/idolnat/screens/priest_step2.png", currentBackground);
      break;
    case HEALTH_TEMPLE5:
      preloadImage("/idolnat/screens/priest_step3.png", currentBackground);
      break;
    case REST_MEDITATE:
      preloadImage("/idolnat/screens/bedroom.png", currentBackground);
      break;
    case REST_SLEEP:
      preloadImage("/idolnat/screens/bedroom_dark.png", currentBackground);
      break;
    case GARDEN_LOOP:
      preloadImage("/idolnat/screens/garden_bg.png", currentBackground);
      break;
    case STATS_SCREEN:
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
    case HEALTH_ONSEN:
      preloadImage(onsenBackgroundForAge(natsumi.age), currentBackground);
      break;
    case REST_MENU:
      preloadImage("/idolnat/screens/bedroom.png", currentBackground);
      break;
  }
  // Load portraits
  switch(natsumi.age) {
    case 11: case 12:
      switch(currentState) {
        case REST_MEDITATE:
          preloadImage("/idolnat/sprites/natsumi_11yo_meditate-90x135.png", currentCharacter);
          break;
        case HEALTH_WASH: case HEALTH_WASH2:
          preloadImage("/idolnat/sprites/natsumi_11yo_washing-90x135.png", currentCharacter);
          break;
        case HEALTH_DOCTOR: case HEALTH_DOCTOR6:
          preloadImage("/idolnat/sprites/doctor01-90x135.png", currentCharacter);
          break;
        case HEALTH_TEMPLE: case HEALTH_TEMPLE6:
          preloadImage("/idolnat/sprites/priest01-90x135.png", currentCharacter);
          break;
        case REST_SLEEP:
          preloadImage("/idolnat/sprites/natsumi_11yo_asleep-90x135.png", currentCharacter);
          break; 
        default:
          preloadImage("/idolnat/sprites/natsumi_11yo-90x135.png", currentCharacter);
          break;
      }
      break;
    case 13: case 14:
      switch(currentState) {
        case REST_MEDITATE:
          preloadImage("/idolnat/sprites/natsumi_13yo_meditate-90x135.png", currentCharacter);
          break;
        case HEALTH_WASH: case HEALTH_WASH2:
          preloadImage("/idolnat/sprites/natsumi_13yo_washing-90x135.png", currentCharacter);
          break;
        case HEALTH_DOCTOR: case HEALTH_DOCTOR6:
          preloadImage("/idolnat/sprites/doctor01-90x135.png", currentCharacter);
          break;
        case HEALTH_TEMPLE: case HEALTH_TEMPLE6:
          preloadImage("/idolnat/sprites/priest02-90x135.png", currentCharacter);
          break;
        case REST_SLEEP:
          preloadImage("/idolnat/sprites/natsumi_13yo_asleep-90x135.png", currentCharacter);
          break; 
        default:
          preloadImage("/idolnat/sprites/natsumi_13yo-90x135.png", currentCharacter);
          break;
      }
      break;
    case 15: case 16: case 17:
      switch(currentState) {
        case REST_MEDITATE:
          preloadImage("/idolnat/sprites/natsumi_15yo_meditate-90x135.png", currentCharacter);
          break;
        case HEALTH_WASH: case HEALTH_WASH2:
          preloadImage("/idolnat/sprites/natsumi_15yo_washing-90x135.png", currentCharacter);
          break;
        case HEALTH_DOCTOR: case HEALTH_DOCTOR6:
          preloadImage("/idolnat/sprites/doctor02-90x135.png", currentCharacter);
          break;
        case HEALTH_TEMPLE: case HEALTH_TEMPLE6:
          preloadImage("/idolnat/sprites/priest01-90x135.png", currentCharacter);
          break;
        case REST_SLEEP:
          preloadImage("/idolnat/sprites/natsumi_15yo_asleep-90x135.png", currentCharacter);
          break; 
        default:
          preloadImage("/idolnat/sprites/natsumi_15yo-90x135.png", currentCharacter);
          break;
      }
      break;
    case 18: case 19: case 20:
      switch(currentState) {
        case REST_MEDITATE:
          preloadImage("/idolnat/sprites/natsumi_18yo_meditate-90x135.png", currentCharacter);
          break;
        case HEALTH_WASH: case HEALTH_WASH2:
          preloadImage("/idolnat/sprites/natsumi_18yo_washing-90x135.png", currentCharacter);
          break;
        case HEALTH_DOCTOR: case HEALTH_DOCTOR6:
          preloadImage("/idolnat/sprites/doctor02-90x135.png", currentCharacter);
          break;
        case HEALTH_TEMPLE: case HEALTH_TEMPLE6:
          preloadImage("/idolnat/sprites/priest01-90x135.png", currentCharacter);
          break;
        case REST_SLEEP:
          preloadImage("/idolnat/sprites/natsumi_18yo_asleep-90x135.png", currentCharacter);
          break; 
        default:
          preloadImage("/idolnat/sprites/natsumi_18yo-90x135.png", currentCharacter);
          break;
      }
      break;
    case 21: case 22:
      switch(currentState) {
        case REST_MEDITATE:
          preloadImage("/idolnat/sprites/natsumi_21yo_meditate-90x135.png", currentCharacter);
          break;
        case HEALTH_WASH: case HEALTH_WASH2:
          preloadImage("/idolnat/sprites/natsumi_21yo_washing-90x135.png", currentCharacter);
          break;
        case HEALTH_DOCTOR: case HEALTH_DOCTOR6:
          preloadImage("/idolnat/sprites/doctor02-90x135.png", currentCharacter);
          break;
        case HEALTH_TEMPLE: case HEALTH_TEMPLE6:
          preloadImage("/idolnat/sprites/priest02-90x135.png", currentCharacter);
          break;
        case REST_SLEEP:
          preloadImage("/idolnat/sprites/natsumi_21yo_asleep-90x135.png", currentCharacter);
          break; 
        default:
          preloadImage("/idolnat/sprites/natsumi_21yo-90x135.png", currentCharacter);
          break;
      }
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
  updateFiveSecondPulse();
/*
  Serial.println("l0NeedsRedraw: " + String(l0NeedsRedraw) + " - l1NeedsRedraw: " + String(l1NeedsRedraw) + " - l2NeedsRedraw: " + String(l2NeedsRedraw) + " - l3NeedsRedraw: " + String(l3NeedsRedraw));
  Serial.println("l4NeedsRedraw: " + String(l4NeedsRedraw) + " - l5NeedsRedraw: " + String(l5NeedsRedraw));
  Serial.println("debugEnabled: " + String(debugEnabled) + " - menuOpened: " + String(menuOpened) + " - toastActive: " + String(toastActive));
  Serial.println("changeStateCounter: " + String(changeStateCounter) + " - l5NeedsRedraw: " + String(l5NeedsRedraw));
*/
  switch (screenConfig) {
    case CARD:
      manageCard();
      break;
    case DIALOG:
      manageDialog();
      break;
    case GAME:
      manageGame();
      break;
    case IDLE:
      manageIdle();
      break;
    case ROOM:
      manageRoom();
      break;
    case TEXT:
      manageText();
      break;
    default:
      break;
  }
}

// === Menu and state logic ===
void changeState(int baseLayer, GameState targetState, int delay) {
  // Manage state transitions
  Serial.println("> Entering changeState() with baseLayer set to " + String(baseLayer) + " and targetState set to " + String(targetState) + " with delay set to " + String(delay));
  if (changeStateCounter == delay) {
    Serial.println("Proceed with transition");
    changeStateCounter = 0;
    currentState = targetState;
    preloadImages();
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
      case 5:
        l5NeedsRedraw = true;
        break;
      default:
        l0NeedsRedraw = true;
        break;
    }
    switch (targetState) {
      case M5_SCREEN:
        screenConfig = CARD;
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
      case NEW_GAME:
        screenConfig = CARD;
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
        break;
      case DEV_SCREEN:
        screenConfig = CARD;
        currentMenuType = "dev";
        currentMenuItems = devMenuItems;
        currentMenuItemsCount = devMenuItemCount;
        break;
      case CALIBRATION_1: case CALIBRATION_2: case CALIBRATION_3:
        break;
      case HOME_LOOP:
        screenConfig = ROOM;
        currentMenuType = "home";
        currentMenuItems = homeMenuItems;
        currentMenuItemsCount = homeMenuItemCount;
        overlayActive = false;
        break;
      case FOOD_EAT:
        screenConfig = ROOM;
        currentMenuType = "food";
        currentMenuItems = foodMenuItems;
        currentMenuItemsCount = foodMenuItemCount;
        break;
      case STATS_SCREEN:
        screenConfig = GAME;
        overlayActive = true;
        l5NeedsRedraw = true;
        toastEnabled = false;
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
      case HEALTH_WASH:
        screenConfig = GAME;
        overlayActive = false;
        menuOpened = false;
        resetBathGame();
        break;
      case HEALTH_WASH2:
        screenConfig = ROOM;
        break;
      case HEALTH_DOCTOR: case HEALTH_DOCTOR6:
        screenConfig = DIALOG;
        overlayActive = true;
        l5NeedsRedraw = true;
        break;
      case HEALTH_DOCTOR2: case HEALTH_DOCTOR3: case HEALTH_DOCTOR4: case HEALTH_DOCTOR5:
        screenConfig = CARD;
        characterEnabled = false;
        break;
      case HEALTH_TEMPLE: case HEALTH_TEMPLE6:
        screenConfig = DIALOG;
        overlayActive = true;
        l5NeedsRedraw = true;
        break;
      case HEALTH_TEMPLE2: case HEALTH_TEMPLE3: case HEALTH_TEMPLE4: case HEALTH_TEMPLE5:
        screenConfig = CARD;
        characterEnabled = false;
        break;
      case HEALTH_ONSEN:
        screenConfig = CARD;
        characterEnabled = false;
        natsumi.hygiene = 4;
        /*
        onsenActive = true;
        onsenTicks = 0;
        onsenStartEnergy = natsumi.energy;
        onsenStartSpirit = natsumi.spirit;
        lastOnsenEnergyDisplayed = -1;
        lastOnsenSpiritDisplayed = -1;
        */
        break;
      case REST_MENU:
        screenConfig = ROOM;
        currentMenuType = "rest";
        currentMenuItems = restMenuItems;
        currentMenuItemsCount = restMenuItemCount;
        break;
      case REST_MEDITATE:
        screenConfig = IDLE;
        meditateStart = millis();
        lastMeditationRedraw = 0;
        meditationActive = true;
        meditationRewardApplied = false;
        overlayActive = true;
        l5NeedsRedraw = true;
        lastMeditationDisplayed = 0;
        toastEnabled = false;
        break;
      case REST_SLEEP:
        screenConfig = IDLE;
        lastSleepEnergyDisplayed = -1;
        overlayActive = true;
        l5NeedsRedraw = true;
        toastEnabled = false;
        break;
      case GARDEN_LOOP:
        screenConfig = ROOM;
        break;
      default:
        break;
    }
  } else {
    Serial.println("Delay transition");
    changeStateCounter += 1;
  }
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
    updateSpirit();
    showToast(String("Natsumi turned ") + natsumi.age + " years old!");
    l5NeedsRedraw=true;
  }
}

void updateStats() {
  // Serial.println("> Entering updateStats()");
  unsigned long currentMillis = millis();
  int previousEnergy = natsumi.energy;

  // Hunger decreases every 2 minutes
  if (currentMillis - natsumi.lastHungerUpdate >= hungerInterval) {
    if (natsumi.hunger > 0) natsumi.hunger--;
    natsumi.lastHungerUpdate = currentMillis;
    Serial.print("Hunger decreased: ");
    Serial.println(natsumi.hunger);
    l5NeedsRedraw=true;
  }

  // Hygiene decreases every 4 minutes
  if (currentMillis - natsumi.lastHygieneUpdate >= hygieneInterval) {
    if (natsumi.hygiene > 0) natsumi.hygiene--;
    natsumi.lastHygieneUpdate = currentMillis;
    Serial.print("Hygiene decreased: ");
    Serial.println(natsumi.hygiene);
    l5NeedsRedraw=true;
  }

  // Energy decreases every 4 minutes
  if (currentMillis - natsumi.lastEnergyUpdate >= energyInterval) {
    switch (currentState) {
      case REST_SLEEP: case HEALTH_ONSEN:
        if (natsumi.energy < 4) natsumi.energy++;
        break;
      default:
        if (natsumi.energy > 0) natsumi.energy--;
        break;
    }
    natsumi.lastEnergyUpdate = currentMillis;
    Serial.print("Energy decreased: ");
    Serial.println(natsumi.energy);
    l5NeedsRedraw=true;
  }
}

void updateSpirit() {
  Serial.println("> Entering updateSpirit()");
  if (!meditationActive) {
    spiritScore = natsumi.hygiene + natsumi.energy + natsumi.hunger + natsumi.performance + natsumi.popularity;
    if ( spiritScore >= 0 && spiritScore < 5 ) {
      natsumi.spirit = 0;
    } else if ( spiritScore >= 5 && spiritScore < 10 ) {
      natsumi.spirit = 1;
    } else if ( spiritScore >= 10 && spiritScore < 15 ) {
      natsumi.spirit = 2;
    } else if ( spiritScore >= 15 && spiritScore < 20 ) {
      natsumi.spirit = 3;
    } else if ( spiritScore == 20 ) {
      natsumi.spirit = 4;
    }
    Serial.println(">> updateSpirit: spiritScore=" + String(spiritScore));
    Serial.println(">> updateSpirit: natsumi.spirit=" + String(natsumi.spirit));
  } else {
    Serial.println(">> updateSpirit: meditationActive is TRUE, no refresh of Spirit");
  }
  return;
}

void updateFiveSecondPulse() {
  unsigned long now = millis();
  if (now < lastFiveSecondTick) {
    lastFiveSecondTick = now;
    Serial.println(">> 5 sec tick");
  }
  if (now - lastFiveSecondTick >= fiveSecondInterval) {
    lastFiveSecondTick = now;
    Serial.println(">>> 5 sec tick");
    fiveSecondPulse = true;
  } else {
    fiveSecondPulse = false;
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
  backgroundEnabled = true;
  characterEnabled = false;
  debugEnabled = true;
  toastEnabled = false;
  menuEnabled = true;
  overlayEnabled = false;
  helperEnabled = false;
  switch (currentState) {
    case M5_SCREEN:
      changeState(0, TITLE_SCREEN, microWait);
      break;
    case TITLE_SCREEN:
      break;
    case NEW_GAME:
      changeState(0, HOME_LOOP, 0);
      break;
    case CONTINUE_GAME:
      changeState(0, HOME_LOOP, 0);
      break;
    case HEALTH_DOCTOR2:
      changeState(0, HEALTH_DOCTOR3, 20);
      break;
    case HEALTH_DOCTOR3:
      changeState(0, HEALTH_DOCTOR4, 20);
      break;
    case HEALTH_DOCTOR4:
      changeState(0, HEALTH_DOCTOR5, 20);
      break;
    case HEALTH_DOCTOR5:
      changeState(0, HEALTH_DOCTOR6, 20);
      break;
    case HEALTH_TEMPLE2:
      changeState(0, HEALTH_TEMPLE3, 20);
      break;
    case HEALTH_TEMPLE3:
      changeState(0, HEALTH_TEMPLE4, 20);
      break;
    case HEALTH_TEMPLE4:
      changeState(0, HEALTH_TEMPLE5, 20);
      break;
    case HEALTH_TEMPLE5:
      changeState(0, HEALTH_TEMPLE6, 20);
      break;
    case HEALTH_ONSEN:
      manageOnsen();
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
  return;
}

void manageDialog() {
  // Manage DIALOG screens
  /*
      Dialog between Natsumi and NPC
      Background: 1 x bitmap
      Character: Natsumi + NPC
      Debug: Available
      Toast: None
      Menu: None
      Interactive (timer + keypress + escape)
  */
  backgroundEnabled = true;
  characterEnabled = true;
  debugEnabled = true;
  toastEnabled = false;
  menuEnabled = false;
  overlayEnabled = true;
  helperEnabled = false;
  switch (currentState) {
    case HEALTH_DOCTOR: case HEALTH_DOCTOR6:
      doctor();
      break;
    case HEALTH_TEMPLE: case HEALTH_TEMPLE6:
      priest();
      break;
    default:
      break;
  }

  // Stats management
  updateAging();
  updateStats();
  
  // Draw required layers for DIALOG screens
  drawBackground(currentBackground);
  drawCharacter();
  drawDebug();
  drawOverlay();
  return;
}

void manageGame() {
  // Manage GAME screens
  /*
      Mini-games
      Background: None
      Character: None
      Debug: Available
      Toast: None
      Menu: None
      Interactive (timer + keypress + escape)
  */
  backgroundEnabled = false;
  characterEnabled = false;
  debugEnabled = true;
  toastEnabled = false;
  menuEnabled = false;
  overlayEnabled = true;
  helperEnabled = false;
  switch (currentState) {
    case HEALTH_WASH:
      manageBathGame();
      break;
    case STATS_SCREEN:
      manageStats();
      break;
    default:
      playGame();
      break;
  }

  // Stats management
  updateAging();
  updateStats();

  // Draw required layers for GAME screens
  drawDebug();
  drawOverlay();
  return;
}

void manageIdle() {
  // Manage IDLE screens
  /*
      Idle mode, minimal screen activity
      Background: Yes, Always black
      Character: Natsumi
      Debug: Available
      Toast: Yes
      Menu: None
      Interactive (escape)
  */
  backgroundEnabled = true;
  characterEnabled = true;
  debugEnabled = true;
  toastEnabled = true;
  menuEnabled = false;
  overlayEnabled = true;
  helperEnabled = false;
  switch (currentState) {
    case REST_MEDITATE:
      meditate();
      break;
    case REST_SLEEP:
      sleep();
      break;
    default:
      break;
  }

  // Stats management
  updateAging();
  updateStats();

  // Draw required layers for IDLE screens
  drawBackground(currentBackground); // Always set to black.png
  drawCharacter();
  drawDebug();
  drawToast();
  drawOverlay();
  return;
}

void manageRoom() {
  // Manage ROOM screens
  /*
  Background: Yes
  Character: Yes
  Debug: Available
  Toast: Yes
  Menu: Yes
  Interactive (timer + keypress + escape)
  */
  backgroundEnabled = true;
  characterEnabled = true;
  debugEnabled = true;
  toastEnabled = true;
  menuEnabled = true;
  overlayEnabled = true;
  helperEnabled = false;
  switch (currentState) {
    case HOME_LOOP:
      manageHomeScreen();
      break;
    case FOOD_EAT:
      eat();
      break;
    case HEALTH_WASH2:
      wash();
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

  // Draw required layers for ROOM screens
  drawBackground(currentBackground);
  drawCharacter();
  drawDebug();
  drawToast();
  drawOverlay();

  int *selectionPtr;
  if (currentMenuType == "home") {
    selectionPtr = &homeMenuSelection;
  } else if (currentMenuType == "dev") {
    selectionPtr = &devMenuSelection;
  } else {
    selectionPtr = &mainMenuSelection;
  }
  drawMenu(currentMenuType, currentMenuItems, currentMenuItemsCount, *selectionPtr);
  return;
}

void manageText() {
  // Manage TEXT screens
  /*
  Transition text (version)
  Background: None
  Character: None
  Debug: None
  Toast: None
  Menu: None
  Non-interactive (timer)
  */
  backgroundEnabled = false;
  characterEnabled = false;
  debugEnabled = false;
  toastEnabled = false;
  menuEnabled = false;
  overlayEnabled = false;
  helperEnabled = false;
  switch (currentState) {
    case VERSION_SCREEN:
      displayVersionScreen();
      break;
    default:
      break;
  }

  // Draw required layers for TEXT screens
  // Meh
  return;
}

void displayBlackScreen() {
  M5Cardputer.Display.fillScreen(BLACK);
  return;
}

void displayVersionScreen() {
  // Serial.println("> Entering displayVersionScreen()");
  if (changeStateCounter==0) {
    displayBlackScreen();
    drawText("IDOL NATSUMI", 120, 30, true, RED, 3); // centered
    drawText("for M5 Cardputer", 120, 50, true, BLUE, 2); // centered
    drawText(copyright, 120, 100, true, WHITE, 1); // centered
    drawText(versionNumber, 120, 110, true, WHITE, 1); // centered
  }
  changeState(0, M5_SCREEN, microWait);
  return;
}

void manageHomeScreen() {
  // Serial.println("> Entering manageHomeScreen()");
  // Serial.print("natsumi.age: ");
  // Serial.println(natsumi.age);
  // Serial.print("currentAge: ");
  // Serial.println(currentAge);
  updateAging();
  updateStats();
  return;
}

void manageGarden() {
  // Serial.println("> Entering manageGarden()");
  updateAging();
  updateStats();
  return;
}

void resetBathGame() {
  bathGameRunning = false;
  bathBackgroundDrawn = false;
  bathResultShown = false;
  sliderYPosition = thermometerY + thermometerHeight - sliderHeight;
  sliderDirection = -1;
  lastSliderUpdate = 0;
  bathGameStart = 0;
  bathOutcomeTime = 0;
  return;
}

void drawBathStaticLayout() {
  drawImage(currentBackground);
  drawImage(currentCharacter);
  const uint16_t frameColor = WHITE;
  const uint16_t fillColor = M5Cardputer.Display.color565(22, 32, 48);
  const uint16_t idealColor = M5Cardputer.Display.color565(64, 200, 120);
  const uint16_t idealOutline = M5Cardputer.Display.color565(140, 235, 200);
  const int innerX = thermometerX + thermometerInnerPadding;
  const int innerWidth = thermometerWidth - thermometerInnerPadding * 2;

  M5Cardputer.Display.fillRect(0, 125, 240, 10, BLACK);
  drawText("Press ENTER at right temperature", 120, 131, true, WHITE, 1);

  M5Cardputer.Display.drawRect(thermometerX, thermometerY, thermometerWidth, thermometerHeight, frameColor);
  M5Cardputer.Display.fillRect(innerX, thermometerY + thermometerInnerPadding, innerWidth, thermometerHeight - thermometerInnerPadding * 2, fillColor);
  M5Cardputer.Display.fillRect(innerX, idealZoneY, innerWidth, idealZoneHeight, idealColor);
  M5Cardputer.Display.drawRect(innerX - 1, idealZoneY - 1, innerWidth + 2, idealZoneHeight + 2, idealOutline);
  bathBackgroundDrawn = true;
  return;
}

void clearBathSlider(int y) {
  const uint16_t fillColor = M5Cardputer.Display.color565(22, 32, 48);
  const uint16_t idealColor = M5Cardputer.Display.color565(64, 200, 120);
  const int innerX = thermometerX + thermometerInnerPadding;
  const int innerWidth = thermometerWidth - thermometerInnerPadding * 2;
  const int sliderBottom = y + sliderHeight;
  const int idealBottom = idealZoneY + idealZoneHeight;

  M5Cardputer.Display.fillRect(innerX, y, innerWidth, sliderHeight, fillColor);

  int overlapTop = max(y, idealZoneY);
  int overlapBottom = min(sliderBottom, idealBottom);
  if (overlapBottom > overlapTop) {
    M5Cardputer.Display.fillRect(innerX, overlapTop, innerWidth, overlapBottom - overlapTop, idealColor);
  }
  return;
}

void drawBathSlider(int y) {
  const uint16_t sliderColor = M5Cardputer.Display.color565(240, 170, 60);
  const int innerX = thermometerX + thermometerInnerPadding;
  const int innerWidth = thermometerWidth - thermometerInnerPadding * 2;
  M5Cardputer.Display.fillRect(innerX, y, innerWidth, sliderHeight, sliderColor);
  return;
}

void finalizeBathOutcome(String outcomeText) {
  bathOutcomeTime = millis();
  bathGameRunning = false;
  M5Cardputer.Display.fillRect(0, 125, 240, 10, BLACK);
  drawText("Bath is " + outcomeText, 120, 131, true, WHITE, 1);
  showToast("Bath is " + outcomeText);

  if (outcomeText == "Perfect!") {
    if (natsumi.hygiene < 4) {
      natsumi.hygiene += 1;
    }
    changeState(0, HEALTH_WASH2, 0);
  }
  return;
}

void startBathGame() {
  resetBathGame();
  bathGameStart = millis();
  bathGameRunning = true;
  drawBathStaticLayout();
  drawBathSlider(sliderYPosition);
  return;
}

void manageBathGame() {
  if (!bathGameRunning && bathOutcomeTime == 0) {
    startBathGame();
    return;
  }

  unsigned long now = millis();

  if (bathOutcomeTime > 0) {
    if (!bathResultShown) {
      bathResultShown = true;
    }
    if (now - bathOutcomeTime >= bathExitDelay) {
      changeState(0, HOME_LOOP, 0);
    }
    return;
  }

  if (now - bathGameStart >= bathGameDuration) {
    int zoneTop = idealZoneY;
    int zoneBottom = idealZoneY + idealZoneHeight;
    int sliderCenter = sliderYPosition + (sliderHeight / 2);
    if (sliderCenter < zoneTop) {
      finalizeBathOutcome("Too hot!");
    } else if (sliderCenter > zoneBottom) {
      finalizeBathOutcome("Too cold!");
    } else {
      finalizeBathOutcome("Perfect!");
    }
    return;
  }

  bool buttonPressed = false;
  if (M5Cardputer.BtnA.wasPressed()) {
    buttonPressed = true;
  }
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      uint8_t key = M5Cardputer.Keyboard.getKey(keyList[0]);
      if (key == 13 || key == 40 || key == ' ') {
        buttonPressed = true;
      }
    }
  }

  if (buttonPressed) {
    int zoneTop = idealZoneY;
    int zoneBottom = idealZoneY + idealZoneHeight;
    int sliderCenter = sliderYPosition + (sliderHeight / 2);
    if (sliderCenter < zoneTop) {
      finalizeBathOutcome("Too hot!");
    } else if (sliderCenter > zoneBottom) {
      finalizeBathOutcome("Too cold!");
    } else {
      finalizeBathOutcome("Perfect!");
    }
    return;
  }

  if (!bathBackgroundDrawn) {
    drawBathStaticLayout();
  }

  if (now - lastSliderUpdate >= sliderUpdateInterval) {
    clearBathSlider(sliderYPosition);
    sliderYPosition += sliderStep * sliderDirection;
    if (sliderYPosition <= thermometerY + thermometerInnerPadding) {
      sliderYPosition = thermometerY + thermometerInnerPadding;
      sliderDirection = 1;
    }
    if (sliderYPosition + sliderHeight >= thermometerY + thermometerHeight - thermometerInnerPadding) {
      sliderYPosition = thermometerY + thermometerHeight - thermometerInnerPadding - sliderHeight;
      sliderDirection = -1;
    }
    drawBathSlider(sliderYPosition);
    lastSliderUpdate = now;
  }
}

void drawDialogBubble(const String& dialogText) {
  // Draw a dialog bubble on the right, matching the menu style
  const int x = 80;              // leave space for the character on the left
  const int w = 150;             // width of the bubble
  const int padding = 8;         // inner padding
  const int lineSpacing = 10;    // spacing between lines (matches drawMenu)
  const int screenHeight = 135;  // display height
  const int topMargin = 8;
  const int bottomMargin = 6;
  const int textInset = padding + 5;  // align text similarly to drawMenu

  M5Cardputer.Display.setTextSize(1);
  const int maxTextWidth = w - (textInset * 2);

  // Split text into wrapped lines
  std::vector<String> lines;
  auto addWrappedLine = [&](const String& line) {
    String currentLine = "";
    int start = 0;
    while (start < line.length()) {
      int spaceIndex = line.indexOf(' ', start);
      String word;
      if (spaceIndex == -1) {
        word = line.substring(start);
        start = line.length();
      } else {
        word = line.substring(start, spaceIndex);
        start = spaceIndex + 1;
      }

      String candidate = currentLine.length() ? currentLine + " " + word : word;
      if (M5Cardputer.Display.textWidth(candidate) <= maxTextWidth) {
        currentLine = candidate;
      } else {
        if (currentLine.length()) {
          lines.push_back(currentLine);
        }

        if (M5Cardputer.Display.textWidth(word) <= maxTextWidth) {
          currentLine = word;
        } else {
          String remainingWord = word;
          while (remainingWord.length()) {
            int cut = remainingWord.length();
            while (cut > 0 && M5Cardputer.Display.textWidth(remainingWord.substring(0, cut)) > maxTextWidth) {
              cut--;
            }
            if (cut == 0) break;
            lines.push_back(remainingWord.substring(0, cut));
            remainingWord = remainingWord.substring(cut);
          }
          currentLine = "";
        }
      }
    }
    if (currentLine.length()) {
      lines.push_back(currentLine);
    }
  };

  int lastPos = 0;
  int newlineIndex = dialogText.indexOf('\n', lastPos);
  while (newlineIndex != -1) {
    addWrappedLine(dialogText.substring(lastPos, newlineIndex));
    lastPos = newlineIndex + 1;
    newlineIndex = dialogText.indexOf('\n', lastPos);
  }
  addWrappedLine(dialogText.substring(lastPos));

  if (lines.empty()) {
    lines.push_back("");
  }

  int h = padding * 2 + ((lines.size() - 1) * lineSpacing);
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

  // Draw the dialog bubble with the same style as drawMenu
  M5Cardputer.Display.fillRect(x, y, w, h, TFT_NAVY);
  M5Cardputer.Display.drawRect(x, y, w, h, WHITE);

  for (size_t i = 0; i < lines.size(); i++) {
    M5Cardputer.Display.setCursor(x + textInset, y + padding + (i * lineSpacing));
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.println(lines[i]);
  }

  // Helper text at the bottom
  M5Cardputer.Display.fillRect(0, 125, 240, 10, BLACK);
  drawText("Press any key to continue", 120, 131, true, WHITE, 1);
}

void eat() {
  // Serial.println("> Entering eat()");
  if (changeStateCounter==0) {
    if (natsumi.hunger < 4) {
      natsumi.hunger += 1;
      showToast("Ate (+1 Hunger)");
    } else {
      showToast("Natsumi is not hungry");
    }
  }
  changeState(0, HOME_LOOP, shortWait);
}

void wash() {
  if (changeStateCounter==0) {
    if (natsumi.hygiene < 4) {
      showToast("Washed (+1 Hygiene)");
    } else {
      showToast("Natsumi is clean");
    }
  }
  changeState(0, HOME_LOOP, shortWait);
}

void drawSleepEnergyOverlay() {
  const int panelX = 20;
  const int panelY = 30;
  const int panelW = 200;
  const int panelH = 75;
  const uint16_t panelColor = M5Cardputer.Display.color565(12, 20, 32);
  const uint16_t borderColor = M5Cardputer.Display.color565(0, 180, 200);
  const uint16_t accentColor = M5Cardputer.Display.color565(255, 200, 40);
  const uint16_t emptyColor = M5Cardputer.Display.color565(28, 36, 48);
  const uint16_t fillHighlight = M5Cardputer.Display.color565(120, 255, 200);

  M5Cardputer.Display.fillRoundRect(panelX - 4, panelY - 4, panelW + 8, panelH + 8, 12, BLACK);
  M5Cardputer.Display.fillRoundRect(panelX, panelY, panelW, panelH, 10, panelColor);
  M5Cardputer.Display.drawRoundRect(panelX, panelY, panelW, panelH, 10, borderColor);
  M5Cardputer.Display.drawRoundRect(panelX + 2, panelY + 2, panelW - 4, panelH - 4, 8, borderColor);

  drawText("REST MODE", panelX + panelW / 2, panelY + 12, true, borderColor, 1, panelColor);
  drawText("Energy", panelX + panelW / 2, panelY + 28, true, WHITE, 2, panelColor);

  const int segmentCount = 4;
  const int barY = panelY + 42;
  const int barHeight = 20;
  const int barWidth = panelW - 40;
  const int segmentSpacing = 6;
  const int totalSegmentsWidth = barWidth - segmentSpacing * (segmentCount - 1);
  const int segmentWidth = totalSegmentsWidth / segmentCount;
  int startX = panelX + (panelW - (segmentWidth * segmentCount + segmentSpacing * (segmentCount - 1))) / 2;

  for (int i = 0; i < segmentCount; ++i) {
    bool filled = i < natsumi.energy;
    uint16_t segmentColor = filled ? accentColor : emptyColor;
    M5Cardputer.Display.fillRoundRect(startX, barY, segmentWidth, barHeight, 4, segmentColor);
    M5Cardputer.Display.drawRoundRect(startX, barY, segmentWidth, barHeight, 4, borderColor);
    if (filled) {
      M5Cardputer.Display.fillRoundRect(startX + 2, barY + 2, segmentWidth - 4, barHeight / 2, 4, fillHighlight);
    }
    startX += segmentWidth + segmentSpacing;
  }
}

void drawMeditationOverlay() {
  const int panelX = 18;
  const int panelY = 20;
  const int panelW = 204;
  const int panelH = 96;

  const uint16_t shadowColor = M5Cardputer.Display.color565(6, 10, 24);
  const uint16_t panelColor = M5Cardputer.Display.color565(16, 24, 48);
  const uint16_t borderColor = M5Cardputer.Display.color565(80, 160, 200);
  const uint16_t accentColor = M5Cardputer.Display.color565(180, 255, 210);
  const uint16_t accentMuted = M5Cardputer.Display.color565(40, 70, 110);
  const uint16_t textColor = WHITE;

  unsigned long now = millis();
  unsigned long elapsed = (now >= meditateStart) ? (now - meditateStart) : 0;
  if (elapsed > meditateInterval) {
    elapsed = meditateInterval;
  }

  unsigned long remaining = (elapsed >= meditateInterval) ? 0 : (meditateInterval - elapsed);
  float progress = meditateInterval == 0 ? 1.0f : (float)elapsed / (float)meditateInterval;
  if (progress > 1.0f) progress = 1.0f;
  
  if ( lastMeditationDisplayed == 0 || lastMeditationDisplayed == 100 ) {
    lastMeditationDisplayed = 0;
    // Panel frame
    M5Cardputer.Display.fillRoundRect(panelX + 3, panelY + 4, panelW, panelH, 14, shadowColor);
    M5Cardputer.Display.fillRoundRect(panelX, panelY, panelW, panelH, 12, panelColor);
    M5Cardputer.Display.drawRoundRect(panelX, panelY, panelW, panelH, 12, borderColor);
    M5Cardputer.Display.drawRoundRect(panelX + 2, panelY + 2, panelW - 4, panelH - 4, 10, accentMuted);
  
    // Title
    drawText("INNER CALM", panelX + panelW / 2, panelY + 14, true, borderColor, 1, panelColor);
    drawText("Meditation", panelX + panelW / 2, panelY + 30, true, textColor, 2, panelColor);
  
    // Progress bar made of soft segments
    const int barX = panelX + 24;
    const int barY = panelY + panelH - 30;
    const int barW = panelW - 48;
    const int barH = 14;
    const int segmentCount = 24;
    const int gap = 2;
    int availableWidth = barW - (segmentCount - 1) * gap;
    int segmentWidth = availableWidth / segmentCount;
    int extraPixels = availableWidth % segmentCount;
  
    M5Cardputer.Display.fillRoundRect(barX - 2, barY - 2, barW + 4, barH + 4, 8, shadowColor);
    int filledSegments = (int)(progress * segmentCount + 0.5f);
    int currentX = barX;
    for (int i = 0; i < segmentCount; ++i) {
      int width = segmentWidth + (i < extraPixels ? 1 : 0);
      uint16_t color = (i < filledSegments) ? accentColor : accentMuted;
      M5Cardputer.Display.fillRoundRect(currentX, barY, width, barH, 6, color);
      currentX += width + gap;
    }
    M5Cardputer.Display.drawRoundRect(barX - 1, barY - 1, barW + 2, barH + 2, 7, borderColor);
  }
  if (remaining == 0) {
    Serial.println(">> drawMeditationOverlay: End of meditation session");
    meditationActive = false;
    if (!meditationRewardApplied) {
      if (natsumi.spirit < 4 ) {
        natsumi.spirit += 1;
        Serial.println(">> drawMeditationOverlay: natsumi.spirit=" + String(natsumi.spirit));
      }
      meditationRewardApplied = true;
    }
  }
  lastMeditationDisplayed++;
}

void sleep() {
  uint8_t key = 0;
  if ((l5NeedsRedraw || lastSleepEnergyDisplayed != natsumi.energy) && natsumi.energy < 4) {
    drawSleepEnergyOverlay();
    lastSleepEnergyDisplayed = natsumi.energy;
    l5NeedsRedraw = false;
  }
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      key = M5Cardputer.Keyboard.getKey(keyList[0]);
      overlayActive = false;
      changeState(0, HOME_LOOP, 0);
      return;
    }
  }
  if (natsumi.energy >= 4) {
    showToast("Natsumi is well rested");
    changeState(0, HOME_LOOP, 0);
    return;
  }
}

void meditate() {
  unsigned long now = millis();

  if (meditationActive) {
    l5NeedsRedraw = true;
  }

  bool meditationFinished = (!meditationActive && meditationRewardApplied);

  if (meditationFinished) {
    showToast("Natsumi feels relaxed");
    changeState(0, HOME_LOOP, 0);
    return;
  }
}

// === Draw functions ===
void drawBackground(const ImageBuffer& bg) {
  // Draw the background of the screen (layer 0)
  Serial.println("> Entering drawBackground() L0 with backgroundEnabled set to " + String(backgroundEnabled));
  if (l0NeedsRedraw) {
    if (backgroundEnabled) {
      drawImage(bg);
    }
    l0NeedsRedraw = false;
    l1NeedsRedraw = true;
    l2NeedsRedraw = true;
    l3NeedsRedraw = true;
    l4NeedsRedraw = true;
    l5NeedsRedraw = true;
  }
}

void drawCharacter() {
  // Draw the character(s) on the screen (layer 1)
  Serial.println("> Entering drawCharacter() L1 with characterEnabled set to " + String(characterEnabled));
  if (l1NeedsRedraw) {
    if (characterEnabled) {
      drawImage(currentCharacter);
      if (!menuOpened && !overlayActive) {
        // Helper text at the bottom
        Serial.println("[DEBUG] manageHomeScreen() -> l5NeedsRedraw TRUE");
        M5Cardputer.Display.fillRect(0, 125, 240, 10, BLACK);
        drawText("TAB: Open menu", 120, 131, true, WHITE, 1);
      }
    }
    l1NeedsRedraw = false;
    l2NeedsRedraw = true;
    l3NeedsRedraw = true;
    l4NeedsRedraw = true;
    l5NeedsRedraw = true;
    lastMeditationDisplayed = 0;
  }
}

void drawDebug() {
  // Draw debug information (layer 2)
  Serial.println("> Entering drawDebug() L2 with debugEnabled set to " + String(debugEnabled));
  if (l2NeedsRedraw && debugActive) {
    if (debugEnabled) {
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
    }
  } else {
    l2NeedsRedraw = false;
  }
}

void drawToast() {
  // Draw toast messages (layer 3)
  Serial.println("> Entering drawToast() L3 with toastEnabled set to " + String(toastEnabled));
  if (toastActive) {
    if (millis() > toastUntil) {
      // Toast expired
      toastActive = false;
      l0NeedsRedraw = true;
      l3NeedsRedraw = false;
      lastMeditationDisplayed = 0;
    }
  }
  if (l3NeedsRedraw && toastActive) {
    if (toastEnabled) {
      const int tx = 120;  // center X (screen is 240 wide in landscape)
      const int ty = 117;  // near bottom for 135px height
      M5Cardputer.Display.fillRect(0, ty - 8, 240, 18, BLACK); // clear strip
      M5Cardputer.Display.setTextDatum(middle_center);
      M5Cardputer.Display.setTextSize(1);
      M5Cardputer.Display.setTextColor(YELLOW, BLACK);
      M5Cardputer.Display.drawString(toastMsg, tx, ty);
      l3NeedsRedraw = false;
      l4NeedsRedraw = true;
      l5NeedsRedraw = true;
    }
  } else {
    l3NeedsRedraw = false;
  }
}

void drawMenu(String menuType, const char* items[], int itemCount, int &selection) {
  // Draw menus on the screen (layer 4)
  Serial.println("> Entering drawMenu() L4 with menuEnabled set to " + String(menuEnabled));
  if (menuEnabled) {
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
          menuOpened = false;
          changeState(0, STATS_SCREEN, 0);
          break;
        case 49:
          // 1: FOOD
          menuOpened = true;
          changeState(0, FOOD_MENU, 0);
          break;
        case 50:
          // 2: TRAINING
          menuOpened = true;
          changeState(0, TRAIN_MENU, 0);
          break;
        case 51:
          // 3: COMPETITION
          menuOpened = true;
          changeState(0, COMP_MENU, 0);
          break;
        case 52:
          // 4: HEALTH
          menuOpened = true;
          changeState(0, HEALTH_MENU, 0);
          break;
        case 53:
          // 5: REST
          menuOpened = true;
          changeState(0, REST_MENU, 0);
          break;
        case 54:
          // 6: GARDEN
          menuOpened = false;
          changeState(0, GARDEN_LOOP, 0);
          break;
        case 55:
          // 7: DEBUG
          if (debugActive) {
            debugActive = false;
            l0NeedsRedraw = true;
            l2NeedsRedraw = false;
            l5NeedsRedraw = true;
          } else {
            debugActive = true;
            l2NeedsRedraw = true;
          }
          break;
        case 43:
          // TAB
          if (menuOpened) {
            menuOpened = false;
            l0NeedsRedraw = true;
            changeState(0, HOME_LOOP, 0);
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
            changeState(0, HOME_LOOP, 0);
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
            changeState(0, STATS_SCREEN, 0);
          } else if (selection == 1) {
            changeState(0, FOOD_MENU, 0);
          } else if (selection == 2) {
            changeState(0, TRAIN_MENU, 0);
          } else if (selection == 3) {
            changeState(0, COMP_MENU, 0);
          } else if (selection == 4) {
            changeState(0, HEALTH_MENU, 0);
          } else if (selection == 5) {
            changeState(0, REST_MENU, 0);
          } else if (selection == 6) {
            changeState(0, GARDEN_LOOP, 0);
          } else if (selection == 7) {
            if (debugActive) {
              debugActive = false;
              l0NeedsRedraw = true;
              l2NeedsRedraw = false;
            } else {
              debugActive = true;
              l2NeedsRedraw = true;
            }
          }
          l5NeedsRedraw = true;
          menuOpened = false;
          break;
      }
    } else if (menuType == "food") {
      switch (key) {
        case 48:
          // 0: FRIDGE
          menuOpened = false;
          changeState(0, FOOD_COOK, 0);
          break;
        case 49:
          // 1: RESTAURANT
          menuOpened = false;
          changeState(0, FOOD_REST, 0);
          break;
        case 50:
          // 2: ORDER
          menuOpened = false;
          changeState(0, FOOD_ORDER, 0);
          break;
        case 43:
          // TAB
          if (menuOpened) {
            menuOpened = false;
            l0NeedsRedraw = true;
            changeState(0, HOME_LOOP, 0);
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
          changeState(0, HOME_LOOP, 0);
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
            changeState(0, FOOD_COOK, 0);
          } else if (selection == 1) {
            changeState(0, FOOD_REST, 0);
          } else if (selection == 2) {
            changeState(0, FOOD_ORDER, 0);
          } else if (selection == 3) {
            if (debugActive) {
              debugActive = false;
              l0NeedsRedraw = true;
              l2NeedsRedraw = false;
            } else {
              debugActive = true;
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
          menuOpened = false;
          changeState(0, TRAIN_SING, 0);
          break;
        case 49:
          // 1: DANCE
          menuOpened = false;
          changeState(0, TRAIN_DANCE, 0);
          break;
        case 50:
          // 2: SWIM
          menuOpened = false;
          changeState(0, TRAIN_SWIM, 0);
          break;
        case 51:
          // 3: GYM
          menuOpened = false;
          changeState(0, TRAIN_GYM, 0);
          break;
        case 52:
          // 4: WALK
          menuOpened = false;
          changeState(0, TRAIN_WALK, 0);
          break;
        case 53:
          // 5: LIBRARY
          menuOpened = false;
          changeState(0, TRAIN_LIBRARY, 0);
          break;
        case 55:
          // 7: DEBUG
          if (debugActive) {
            debugActive = false;
            l0NeedsRedraw = true;
            l2NeedsRedraw = false;
          } else {
            debugActive = true;
            l2NeedsRedraw = true;
          }
          break;
        case 43:
          // TAB
          if (menuOpened) {
            menuOpened = false;
            l0NeedsRedraw = true;
            changeState(0, HOME_LOOP, 0);
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
          changeState(0, HOME_LOOP, 0);
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
            changeState(0, TRAIN_SING, 0);
          } else if (selection == 1) {
            changeState(0, TRAIN_DANCE, 0);
          } else if (selection == 2) {
            changeState(0, TRAIN_SWIM, 0);
          } else if (selection == 3) {
            changeState(0, TRAIN_GYM, 0);
          } else if (selection == 4) {
            changeState(0, TRAIN_WALK, 0);
          } else if (selection == 5) {
            changeState(0, TRAIN_LIBRARY, 0);
          } else if (selection == 7) {
            if (debugActive) {
              debugActive = false;
              l0NeedsRedraw = true;
              l2NeedsRedraw = false;
            } else {
              debugActive = true;
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
          menuOpened = false;
          changeState(0, COMP_LOCAL, 0);
          break;
        case 49:
          // 1: DEPARTMENTAL
          menuOpened = false;
          changeState(0, COMP_DEPT, 0);
          break;
        case 50:
          // 2: REGIONAL
          menuOpened = false;
          changeState(0, COMP_REG, 0);
          break;
        case 51:
          // 3: NATIONAL
          menuOpened = false;
          changeState(0, COMP_NAT, 0);
          break;
        case 55:
          // 7: DEBUG
          if (debugActive) {
            debugActive = false;
            l0NeedsRedraw = true;
            l2NeedsRedraw = false;
          } else {
            debugActive = true;
            l2NeedsRedraw = true;
          }
          break;
        case 43:
          // TAB
          if (menuOpened) {
            menuOpened = false;
            l0NeedsRedraw = true;
            changeState(0, HOME_LOOP, 0);
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
          changeState(0, HOME_LOOP, 0);
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
            changeState(0, COMP_LOCAL, 0);
          } else if (selection == 1) {
            changeState(0, COMP_DEPT, 0);
          } else if (selection == 2) {
            changeState(0, COMP_REG, 0);
          } else if (selection == 3) {
            changeState(0, COMP_NAT, 0);
          } else if (selection == 7) {
            if (debugActive) {
              debugActive = false;
              l0NeedsRedraw = true;
              l2NeedsRedraw = false;
            } else {
              debugActive = true;
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
          menuOpened = false;
          changeState(0, HEALTH_WASH, 0);
          break;
        case 49:
          // 1: DOCTOR
          menuOpened = false;
          changeState(0, HEALTH_DOCTOR, 0);
          break;
        case 50:
          // 2: TEMPLE
          menuOpened = false;
          changeState(0, HEALTH_TEMPLE, 0);
          break;
        case 51:
          // 3: ONSEN
          menuOpened = false;
          changeState(0, HEALTH_ONSEN, 0);
          break;
        case 55:
          // 7: DEBUG
          if (debugActive) {
            debugActive = false;
            l0NeedsRedraw = true;
            l2NeedsRedraw = false;
          } else {
            debugActive = true;
            l2NeedsRedraw = true;
          }
          break;
        case 43:
          // TAB
          if (menuOpened) {
            menuOpened = false;
            l0NeedsRedraw = true;
            changeState(0, HOME_LOOP, 0);
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
          changeState(0, HOME_LOOP, 0);
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
            changeState(0, HEALTH_WASH, 0);
          } else if (selection == 1) {
            changeState(0, HEALTH_DOCTOR, 0);
          } else if (selection == 2) {
            changeState(0, HEALTH_TEMPLE, 0);
          } else if (selection == 3) {
            changeState(0, HEALTH_ONSEN, 0);
          } else if (selection == 7) {
            if (debugActive) {
              debugActive = false;
              l0NeedsRedraw = true;
              l2NeedsRedraw = false;
            } else {
              debugActive = true;
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
          menuOpened = false;
          changeState(0, REST_MEDITATE, 0);
          break;
        case 49:
          // 1: SLEEP
          menuOpened = false;
          changeState(0, REST_SLEEP, 0);
          break;
        case 55:
          // 7: DEBUG
          if (debugActive) {
            debugActive = false;
            l0NeedsRedraw = true;
            l2NeedsRedraw = false;
          } else {
            debugActive = true;
            l2NeedsRedraw = true;
          }
          break;
        case 43:
          // TAB
          if (menuOpened) {
            menuOpened = false;
            l0NeedsRedraw = true;
            changeState(0, HOME_LOOP, 0);
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
          changeState(0, HOME_LOOP, 0);
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
            changeState(0, REST_MEDITATE, 0);
          } else if (selection == 1) {
            changeState(0, REST_SLEEP, 0);
          } else if (selection == 7) {
            if (debugActive) {
              debugActive = false;
              l0NeedsRedraw = true;
              l2NeedsRedraw = false;
            } else {
              debugActive = true;
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
          menuOpened = false;
          changeState(0, CALIBRATION_1, 0);
          break;
        case 49:
          // 1: CALIB2
          menuOpened = false;
          changeState(0, CALIBRATION_2, 0);
          break;
        case 50:
          // 2: CALIB3
          menuOpened = false;
          changeState(0, CALIBRATION_3, 0);
          break;
        case 51:
          // 3: EXIT
          menuOpened = true;
          changeState(0, TITLE_SCREEN, 0);
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
            menuOpened = true;
          }
          changeState(0, TITLE_SCREEN, 0);
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
            menuOpened = false;
            changeState(0, CALIBRATION_1, 0);
          } else if (selection == 1) {
            menuOpened = false;
            changeState(0, CALIBRATION_2, 0);
          } else if (selection == 2) {
            menuOpened = false;
            changeState(4, CALIBRATION_3, 0);
          } else if (selection == 3) {
            menuOpened = true;
            changeState(0, TITLE_SCREEN, 0);
          }
          break;
      }
    } else if (menuType == "main") {
      switch (key) {
        case 48:
          // 0: NEW GAME
          menuOpened = false;
          changeState(0, NEW_GAME, 0);
          break;
        case 49:
          // 1: CONTINUE
          menuOpened = false;
          changeState(0, CONTINUE_GAME, 0);
          break;
        case 50:
          // 2: DEV SCREEN
          menuOpened = true;
          changeState(4, DEV_SCREEN, 0);
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
            menuOpened = false;
            changeState(0, NEW_GAME, 0);
          } else if (selection == 1) {
            menuOpened = false;
            changeState(0, CONTINUE_GAME, 0);
          } else {
            menuOpened = true;
            changeState(4, DEV_SCREEN, 0);
            return;
          }
  
      }
    }
    if (!menuOpened) {
      l4NeedsRedraw = false;
      return;
    } else if (l4NeedsRedraw && menuOpened) {
      const int x = 60;
      const int w = 120;
      const int padding = 8;
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
  } else {
    l4NeedsRedraw = false;
  }
}

void drawOverlay() {
  // Draw the overlay (L5)
  Serial.println("> Entering drawOverlay() L5 with l5NeedsRedraw set to " + String(l5NeedsRedraw) + " and overlayActive set to " + String(overlayActive));
  if (overlayActive && overlayEnabled) {
    Serial.println(">> drawOverlay: l5NeedsRedraw is TRUE");
    uint8_t key = 0;
    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
      auto keyList = M5Cardputer.Keyboard.keyList();
      Serial.println(">>> drawOverlay: Testing for key pressed");
      if (keyList.size() > 0) {
        key = M5Cardputer.Keyboard.getKey(keyList[0]);
        overlayActive = false;
        changeState(0, HOME_LOOP, 0);
        return;
      }
    }
  }
  if (l5NeedsRedraw && overlayActive && overlayEnabled) {
    switch (currentState) {
      case HOME_LOOP:
        Serial.println(">>> drawOverlay: HOME_LOOP");
        break;
      case STATS_SCREEN:
        Serial.println(">>> drawOverlay: STATS_SCREEN");
        drawStats();
        break;
      case REST_SLEEP:
        Serial.println(">>> drawOverlay: REST_SLEEP");
        if (natsumi.energy < 4) {
          drawSleepEnergyOverlay();
        }
        break;
      case REST_MEDITATE:
        Serial.println(">>> drawOverlay: REST_MEDITATE");
        Serial.println(">>> drawOverlay: lastMeditationDisplayed=" + String(lastMeditationDisplayed));
        drawMeditationOverlay();
        break;
      case HEALTH_DOCTOR:
        drawDialogBubble("Hello Miss Hasegawa. Thanks for coming to our medical center. I will check your health and see if everything is OK.");
        break;
      case HEALTH_DOCTOR6:
        if (natsumi.hunger < 2) {
          drawDialogBubble("You need to eat more...");
          doctorState = FOOD_MENU;
        } else if (natsumi.hygiene < 2) {
          drawDialogBubble("You need better hygiene...");
          doctorState = HEALTH_MENU;
        } else if (natsumi.fitness < 2) {
          drawDialogBubble("You need to exercise more...");
          doctorState = TRAIN_MENU;
        } else if (natsumi.energy < 2) {
          drawDialogBubble("You need to sleep more...");
          doctorState = REST_MENU;
        } else {
          drawDialogBubble("Congratulations!! You are in shape!");
          doctorState = HOME_LOOP;
        }
        break;
      case HEALTH_TEMPLE:
        drawDialogBubble("Hello Miss. Thanks for visiting our temple. I will pray for you and see if everything is OK.");
        break;
      case HEALTH_TEMPLE6:
        if (natsumi.spirit < 2) {
          if (natsumi.hunger < 2) {
            drawDialogBubble("You need to eat more...");
            priestState = FOOD_MENU;
          } else if (natsumi.hygiene < 2) {
            drawDialogBubble("You need better hygiene...");
            priestState = HEALTH_MENU;
          } else if (natsumi.energy < 2) {
            drawDialogBubble("You need to exercise more...");
            priestState = HEALTH_MENU;
          } else if (natsumi.performance < 2) {
            drawDialogBubble("You need to train more...");
            priestState = TRAIN_MENU;
          } else if (natsumi.popularity < 2) {
            drawDialogBubble("You need to compete more...");
            priestState = COMP_MENU;
          }
        } else if (natsumi.charm < 2) {
          drawDialogBubble("Treat yourself to some nice food, it is good for the soul.");
          priestState = FOOD_MENU;
        } else {
          drawDialogBubble("Congratulations!! You have a strong mind!");
          priestState = HOME_LOOP;
        }
        break;
      default:
        break;
    }
  }
  l5NeedsRedraw = false;
}

void playGame() {
  // Play one of the mini-games
  M5Cardputer.Display.fillRect(0, 125, 240, 10, BLACK);
  drawText("Mini-game", 120, 131, true, WHITE, 1);
}

void drawStats() {
  // Draw the Status Board / Statistics screen
  Serial.println("> Entering drawStats()");
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
    {"Age", natsumi.age, 22, M5Cardputer.Display.color565(255, 149, 64)}
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
  Serial.println("> Exiting drawStats()");
}

void drawStatBar(const String &label, int value, int maxValue, int x, int y, int width, int barHeight, uint16_t barColor, uint16_t bgColor, uint16_t frameColor) {
  Serial.println("> Entering drawStatBar()");
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

void drawOnsenOverlay() {
  const int panelX = 4;
  const int panelY = 4;
  // const int panelWidth = 118;
  const int panelWidth = 148;
  const int panelHeight = 38;
  const int barWidth = 54;
  const int barHeight = 7;
  const int rowHeight = 13;

  uint16_t panelBg = M5Cardputer.Display.color565(12, 16, 24);
  uint16_t panelFrame = M5Cardputer.Display.color565(90, 140, 210);
  uint16_t shadow = M5Cardputer.Display.color565(5, 7, 12);

  M5Cardputer.Display.fillRoundRect(panelX + 1, panelY + 1, panelWidth, panelHeight, 5, shadow);
  M5Cardputer.Display.fillRoundRect(panelX, panelY, panelWidth, panelHeight, 5, panelBg);
  M5Cardputer.Display.drawRoundRect(panelX, panelY, panelWidth, panelHeight, 5, panelFrame);

  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setTextColor(panelFrame, panelBg);
  M5Cardputer.Display.setCursor(panelX + 6, panelY + 2);
  M5Cardputer.Display.print("Onsen Status");

  auto drawRow = [&](const char* label, int value, int rowIndex, uint16_t mainColor, uint16_t accentColor) {
    int clamped = value;
    if (clamped < 0) clamped = 0;
    if (clamped > STAT_MAX) clamped = STAT_MAX;

    int rowY = panelY + 10 + rowHeight * rowIndex;
    int iconX = panelX + 8;
    // int barX = panelX + 28;
    int barX = panelX + 58;
    int barY = rowY + 4;

    // M5Cardputer.Display.fillCircle(iconX, rowY + 3, 4, mainColor);
    // M5Cardputer.Display.fillCircle(iconX, rowY + 3, 2, accentColor);

    M5Cardputer.Display.setTextColor(WHITE, panelBg);
    // M5Cardputer.Display.setCursor(iconX + 7, rowY - 2);
    M5Cardputer.Display.setCursor(iconX + 4, rowY + 7);
    M5Cardputer.Display.print(label);

    String valueText = String(clamped) + "/" + String(STAT_MAX);
    M5Cardputer.Display.setTextColor(accentColor, panelBg);
    // M5Cardputer.Display.setCursor(panelX + panelWidth - 24, rowY - 2);
    M5Cardputer.Display.setCursor(panelX + panelWidth - 24, rowY + 7);
    M5Cardputer.Display.print(valueText);

    uint16_t barBg = M5Cardputer.Display.color565(24, 32, 46);
    M5Cardputer.Display.fillRoundRect(barX, barY, barWidth, barHeight, 3, barBg);
    M5Cardputer.Display.drawRoundRect(barX, barY, barWidth, barHeight, 3, accentColor);

    int filled = (barWidth - 2) * clamped / STAT_MAX;
    int fillWidth = filled;
    if (fillWidth < 0) {
      fillWidth = 0;
    }
    if (fillWidth > barWidth - 2) {
      fillWidth = barWidth - 2;
    }

    if (fillWidth > 0) {
      M5Cardputer.Display.fillRoundRect(barX + 1, barY + 1, fillWidth, barHeight - 2, 2, mainColor);
      uint16_t highlight = M5Cardputer.Display.color565(230, 230, 255);
      int highlightWidth = fillWidth - 1;
      if (highlightWidth > 0) {
        M5Cardputer.Display.drawFastHLine(barX + 2, barY + 1, highlightWidth, highlight);
      }
    }
  };

  drawRow("Energy", natsumi.energy, 0, M5Cardputer.Display.color565(255, 214, 102), M5Cardputer.Display.color565(255, 240, 180));
  drawRow("Spirit", natsumi.spirit, 1, M5Cardputer.Display.color565(180, 140, 255), M5Cardputer.Display.color565(215, 195, 255));

  lastOnsenEnergyDisplayed = natsumi.energy;
  lastOnsenSpiritDisplayed = natsumi.spirit;
}

void manageStats() {
  Serial.println("> Entering manageStats()");
  uint8_t key = 0;
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      key = M5Cardputer.Keyboard.getKey(keyList[0]);
      overlayActive = false;
      changeState(0, HOME_LOOP, 0);
      return;
    }
  }
}

void cookFood() {
  // Cook food from the fridge
  if (changeStateCounter==0) {
    // Select food
  }
  changeState(0, HOME_LOOP, shortWait);
}

void gotoRestaurant() {
  // Eat at the restaurant
  if (changeStateCounter==0) {
    // Select food
  }
  changeState(0, HOME_LOOP, shortWait);
}

void orderFood() {
  // Order takeaway food
  if (changeStateCounter==0) {
    // Select food
  }
  changeState(0, HOME_LOOP, shortWait);
}

void doctor() {
  Serial.println("> Entering doctor()");
  uint8_t key = 0;
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      key = M5Cardputer.Keyboard.getKey(keyList[0]);
      switch (currentState) {
        case HEALTH_DOCTOR:
          changeState(0, HEALTH_DOCTOR2, 0);
          break;
        case HEALTH_DOCTOR6:
          overlayActive = false;
          changeState(0, doctorState, 0);
          break;
      }
      return;
    }
  }
}

void priest() {
  Serial.println("> Entering priest()");
  uint8_t key = 0;
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      key = M5Cardputer.Keyboard.getKey(keyList[0]);
      switch (currentState) {
        case HEALTH_TEMPLE:
          changeState(0, HEALTH_TEMPLE2, 0);
          break;
        case HEALTH_TEMPLE6:
          overlayActive = false;
          changeState(0, priestState, 0);
          break;
      }
      return;
    }
  }
}

void manageOnsen() {
  Serial.println("> Entering manageOnsen()");
  uint8_t key = 0;
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      key = M5Cardputer.Keyboard.getKey(keyList[0]);
      overlayActive = false;
      changeState(0, HOME_LOOP, 0);
      return;
    }
  }
  // Stats management
  updateAging();
  updateStats();
  if (changeStateCounter==0) {
    // meh
  }
  if (fiveSecondPulse) {
    drawOnsenOverlay();
  }
  if (lastOnsenEnergyDisplayed != natsumi.energy || lastOnsenSpiritDisplayed != natsumi.spirit) {
    drawOnsenOverlay();
  }
  /*
  if (natsumi.energy == 4 && natsumi.hygiene == 4) {
    changeState(0, HOME_LOOP, 0);
  }
  */
  return;
}
;
