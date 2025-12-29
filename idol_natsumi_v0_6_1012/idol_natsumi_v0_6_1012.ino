#include <M5Cardputer.h>
#include <SD.h>
#include <algorithm>
#include <cstring>
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
  FLOWERS_MARKET,
  FLOWERS_MARKET2,
  FLOWERS_MARKET3,
  FLOWERS_MARKET4,
  FLOWERS_MARKET5,
  FLOWERS_MARKET6,
  FLOWERS_MARKET7,
  FOOD_MENU,
  FOOD_CONBINI,
  FOOD_CONBINI2,
  FOOD_CONBINI3,
  FOOD_COOK,
  FOOD_COOK2,
  FOOD_REST,
  FOOD_REST2,
  FOOD_REST3,
  FOOD_REST4,
  FOOD_REST5,
  FOOD_REST6,
  FOOD_REST7,
  FOOD_REST8,
  FOOD_ORDER,
  FOOD_ORDER2,
  FOOD_ORDER3,
  FOOD_ORDER4,
  FOOD_ORDER5,
  FOOD_ORDER6,
  FOOD_ORDER7,
  FOOD_ORDER8,
  HEALTH_MENU,
  HEALTH_WASH,
  HEALTH_WASH2,
  HEALTH_WASH3,
  HEALTH_WASH4,
  HEALTH_WASH5,
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
  GARDEN_MENU,
  GARDEN_LOOP,
  GARDEN_PLANT,
  GARDEN_WATER,
  GARDEN_PICK,
  GARDEN_CLEANUP,
  TRAIN_MENU,
  TRAIN_SING,
  TRAIN_SING2,
  TRAIN_SING3,
  TRAIN_DANCE,
  TRAIN_DANCE2,
  TRAIN_DANCE3,
  TRAIN_SWIM,
  TRAIN_SWIM2,
  TRAIN_SWIM3,
  TRAIN_GYM,
  TRAIN_GYM2,
  TRAIN_GYM3,
  TRAIN_RUN,
  TRAIN_RUN2,
  TRAIN_RUN3,
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
  int money;
  int flowers;
  unsigned long lastHungerUpdate = 0;
  unsigned long lastHygieneUpdate = 0;
  unsigned long lastEnergyUpdate = 0;
};

NatsumiStats natsumi;

struct FridgeStock {
  int redApple;
  int greenApple;
  int avocado;
  int bread;
  int banana;
  int broccoli;
  int sweets;
  int carrot;
  int meat;
  int coconut;
  int coconutJuice;
  int coffee;
  int biscuits;
  int corn;
  int croissant;
  int friedEgg;
  int grapes;
  int kiwi;
  int milk;
  int orange;
  int peach;
  int pear;
  int strawberries;
  int maki;
  int sushi;
  int watermelon;
};

FridgeStock fridge;

// === Image preload system ===
struct ImageBuffer {
  uint8_t* data = nullptr;
  size_t length = 0;
};

struct FoodDisplayItem {
  const char* label;
  const char* iconPath;
  int* quantityPtr;
  int quantity;
  ImageBuffer icon;
};

String selectedFood = "None";

std::vector<FoodDisplayItem> foodGridItems;
int foodSelectionIndex = 0;
bool foodGridInitialized = false;

struct ConbimartItem {
  const char* label;
  int price;
  int* stockPtr;
  int quantity;
};

std::vector<ConbimartItem> conbimartItems;
int conbimartSelectionIndex = 0;
bool conbimartInitialized = false;

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

// Training SING mini-game helpers
void resetTrainSingGame();
void manageTrainSingGame();
void drawTrainSingPlayfield(bool showCompletion);
void startTrainSingGame();

// Training SWIM mini-game helpers
void resetTrainSwimGame();
void manageTrainSwimGame();
void drawTrainSwimPlayfield(bool showCompletion, bool showHitEffect);
void startTrainSwimGame();

// Training GYM mini-game helpers
void resetTrainGymGame();
void manageTrainGymGame();
void drawTrainGymPlayfield(bool showCompletion);
void startTrainGymGame();

// Training RUN mini-game helpers
void resetTrainRunGame();
void manageTrainRunGame();
void drawTrainRunPlayfield(bool showCompletion, bool showFailure);
void startTrainRunGame();

unsigned long changeStateCounter = 0;

const unsigned long hungerInterval = 120000;   // 2 minutes
const unsigned long hygieneInterval = 240000;  // 4 minutes
const unsigned long energyInterval = 240000;   // 4 minutes
const unsigned long meditateInterval = 300000;   // 5 minutes
const unsigned long librarySegmentInterval = 5000; // 5 seconds
const unsigned long fiveSecondInterval = 5000;  // 5 seconds
const int STAT_MAX = 4;
const int librarySegmentCount = 30;
unsigned long meditateStart = 0;
unsigned long lastMeditationRedraw = 0;
unsigned long lastFiveSecondTick = 0;
unsigned long libraryStartTime = 0;

String currentMenuType = "main";
const char* mainMenuItems[] = {"0: NEW GAME", "1: CONTINUE", "2: DEV SCREEN"};
const char* homeMenuItems[] = {"0: STATS", "1: FOOD", "2: TRAINING", "3: COMPETITION", "4: HEALTH", "5: REST", "6: GARDEN", "7: DEBUG"};
const char* devMenuItems[] = {"0: CALIB1", "1: CALIB2", "2: CALIB3", "3: EXIT"};
const char* foodMenuItems[] = {"0: FRIDGE", "1: RESTAURANT", "2: ORDER", "3: CONBINI"};
const char* trainingMenuItems[] = {"0: SING", "1: DANCE", "2: SWIM", "3: GYM", "4: RUN", "5: LIBRARY", "6: MARKET"};
const char* competitionMenuItems[] = {"0: LOCAL", "1: DEPARTMENTAL", "2: REGIONAL", "3: NATIONAL"};
const char* healthMenuItems[] = {"0: WASH", "1: DOCTOR", "2: TEMPLE", "3: ONSEN"};
const char* restMenuItems[] = {"0: MEDITATE", "1: SLEEP"};
const char* gardenMenuItems[] = {"0: PLANT", "1: WATER", "2: PICK", "3: CLEANUP"};
const char** currentMenuItems = nullptr;
const int mainMenuItemCount = 3;
const int homeMenuItemCount = 8;
const int devMenuItemCount = 4;
const int foodMenuItemCount = 4;
const int trainingMenuItemCount = 7;
const int competitionMenuItemCount = 4;
const int healthMenuItemCount = 4;
const int restMenuItemCount = 2;
const int gardenMenuItemCount = 4;
int currentMenuItemsCount = 0;
int homeMenuSelection = 0;
int mainMenuSelection = 0;
int devMenuSelection = 0;
int foodMenuSelection = 0;
int trainingMenuSelection = 0;
int competitionMenuSelection = 0;
int healthMenuSelection = 0;
int restMenuSelection = 0;
int gardenMenuSelection = 0;
const int gardenRows = 3;
const int gardenCols = 3;
int gardenTiles[gardenRows][gardenCols] = {};
int gardenCursorRow = 0;
int gardenCursorCol = 0;
int flowersPrice = 0;
int flowersRevenue = 0;

int lastSleepEnergyDisplayed = -1;
int lastMeditationDisplayed = 0;
int restaurantSelection = 0;
int orderibiSelection = 0;

int foodDeliveryCounter = 0;

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
bool waitingForFoodDelivery = false;

bool meditationActive = false;
bool meditationRewardApplied = false;
bool fiveSecondPulse = false;  // Set true by updateFiveSecondPulse() every five seconds
bool libraryInitialized = false;
bool libraryRewardApplied = false;
bool isNatsumiHappy = false;
bool gardenActive = false;
bool flowersSaleInProgress = false;
int librarySegmentsFilled = 0;

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
const int sliderHeight = 6;
const int sliderStep = 6;
const unsigned long sliderUpdateInterval = 20;
const int idealZoneHeight = 20;
const int idealZoneY = thermometerY + thermometerInnerPadding + ((thermometerHeight - thermometerInnerPadding * 2 - idealZoneHeight) / 2);
int sliderYPosition = thermometerY + thermometerHeight - sliderHeight;
int sliderDirection = -1;  // -1 = moving up, 1 = moving down
unsigned long lastSliderUpdate = 0;

// Training SING mini-game state
struct FallingNote {
  int column;
  int y;
  bool active;
};

const int singColumnCount = 5;
const int singTargetNotes = 30;
const int singNoteRadius = 5;
const int singPlayerWidth = 22;
const int singPlayerHeight = 10;
const unsigned long singNoteSpawnInterval = 700;
const int singNoteFallSpeed = 3;
int singColumnWidth = 48;
int singPlayerY = 118;
int singPlayerColumn = singColumnCount / 2;
int singNotesCollected = 0;
int singNotesSpawned = 0;
unsigned long singLastSpawnTime = 0;
unsigned long singCompletionTime = 0;
bool singGameRunning = false;
bool singGameCompleted = false;
std::vector<FallingNote> singNotes;

// Training DANCE mini-game state
const unsigned long danceCueDuration = 1000;      // How long a cue stays on screen (ms)
const unsigned long danceCueGap = 300;           // Gap before next cue appears (ms)
const int danceTargetScore = 30;
int danceScore = 0;
int danceCuesShown = 0;
int danceCurrentDirection = -1;                  // 0=UP,1=DOWN,2=LEFT,3=RIGHT
bool danceCueActive = false;
unsigned long danceCueStart = 0;
unsigned long danceNextCueTime = 0;
unsigned long danceCompletionTime = 0;
bool danceGameRunning = false;
bool danceGameCompleted = false;
bool danceNeedsRedraw = false;

// Training SWIM mini-game state
struct SwimShark {
  float x;
  int lane;
  float speed;
  bool active;
};

const int swimLaneCount = 3;
const int swimLaneHeight = 36;
const int swimPoolTop = 16;
const int swimTargetSharks = 30;
const int swimMaxSharks = 3;
const int swimPlayerWidth = 18;
const int swimPlayerHeight = 12;
const int swimSharkLength = 26;
const int swimSharkHeight = 12;
const int swimHitPenalty = 4;
const unsigned long swimSpawnIntervalMin = 650;
const unsigned long swimSpawnIntervalMax = 1150;
const float swimMinSpeed = 1.4f;
const float swimMaxSpeed = 2.6f;
const unsigned long swimHitFlashDuration = 200;
const unsigned long swimCompletionDelay = 1200;
int swimPlayerLane = 1;
int swimAvoidedSharks = 0;
int swimCollisions = 0;
bool swimGameRunning = false;
bool swimGameCompleted = false;
bool swimNeedsRedraw = false;
bool swimHitFlash = false;
unsigned long swimHitFlashTime = 0;
unsigned long swimCompletionTime = 0;
unsigned long swimLastSpawnTime = 0;
unsigned long swimNextSpawnDelay = swimSpawnIntervalMin;
std::vector<SwimShark> swimSharks;

// Training GYM mini-game state
const int gymTargetStreak = 10;
const int gymBarWidth = 170;
const int gymBarHeight = 18;
const int gymBarX = 54;
const int gymBarY = 62;
const int gymZoneMinWidth = 26;
const int gymZoneMaxWidth = 44;
const float gymCursorBaseSpeed = 0.12f;     // pixels per millisecond
const float gymCursorSpeedStep = 0.018f;    // incremental speed-up per success
const unsigned long gymResultFlashDuration = 800;
const unsigned long gymCompletionDelay = 1200;
float gymCursorPos = static_cast<float>(gymBarX);
float gymCursorSpeed = gymCursorBaseSpeed;
int gymCursorDirection = 1;                 // 1 = right, -1 = left
int gymZoneStart = gymBarX;
int gymZoneWidth = 32;
int gymCurrentStreak = 0;
int gymMisses = 0;
char gymTargetLetter = 'A';
unsigned long gymLastUpdate = 0;
unsigned long gymResultFlashUntil = 0;
String gymResultText = "";
unsigned long gymCompletionTime = 0;
bool gymGameRunning = false;
bool gymGameCompleted = false;
bool gymNeedsRedraw = false;

// Training RUN mini-game state
const int runBarWidth = 120;
const int runBarHeight = 14;
const int runBarY = 62;
const int runGreenWidth = 44;
// const float runCursorSpeed = 0.18f;           // pixels per millisecond
const float runCursorSpeed = 0.08f;
const unsigned long runTargetGreenTime = 10000;
const unsigned long runMaxRedTime = 3000;
const unsigned long runCompletionDelay = 1200;
const unsigned long runStepInterval = 160;
float runCursorPos = 10.0f;
bool runEnterHeld = false;
unsigned long runGreenTime = 0;
unsigned long runRedTime = 0;
unsigned long runLastUpdate = 0;
unsigned long runCompletionTime = 0;
unsigned long runLastStepTime = 0;
int runStepIndex = 0;
bool runGameRunning = false;
bool runGameCompleted = false;
bool runGameFailed = false;
bool runNeedsRedraw = false;

String copyright = "(c) 2025 - Pantzumatic";
String versionNumber = "0.6.1012";

ImageBuffer currentBackground;
ImageBuffer calib1, calib2, calib3;
ImageBuffer currentCharacter;
ImageBuffer currentIcon;
ImageBuffer natsumiSprite;
ImageBuffer enemySprite;

const char* gameStateToString(GameState state) {
  switch (state) {
    case VERSION_SCREEN:   return "VERSION_SCREEN";
    case M5_SCREEN:        return "M5_SCREEN";
    case TITLE_SCREEN:     return "TITLE_SCREEN";
    case CALIBRATION_1:    return "CALIBRATION_1";
    case CALIBRATION_2:    return "CALIBRATION_2";
    case CALIBRATION_3:    return "CALIBRATION_3";
    case NEW_GAME:         return "NEW_GAME";
    case CONTINUE_GAME:    return "CONTINUE_GAME";
    case DEV_SCREEN:       return "DEV_SCREEN";
    case HOME_LOOP:        return "HOME_LOOP";
    case FLOWERS_MARKET:   return "FLOWERS_MARKET";
    case FLOWERS_MARKET2:   return "FLOWERS_MARKET2";
    case FLOWERS_MARKET3:   return "FLOWERS_MARKET3";
    case FLOWERS_MARKET4:   return "FLOWERS_MARKET4";
    case FLOWERS_MARKET5:   return "FLOWERS_MARKET5";
    case FLOWERS_MARKET6:  return "FLOWERS_MARKET6";
    case FLOWERS_MARKET7:  return "FLOWERS_MARKET7";
    case FOOD_MENU:        return "FOOD_MENU";
    case FOOD_CONBINI:     return "FOOD_CONBINI";
    case FOOD_CONBINI2:    return "FOOD_CONBINI2";
    case FOOD_CONBINI3:    return "FOOD_CONBINI3";
    case FOOD_COOK:        return "FOOD_COOK";
    case FOOD_COOK2:       return "FOOD_COOK2";
    case FOOD_REST:        return "FOOD_REST";
    case FOOD_REST2:       return "FOOD_REST2";
    case FOOD_REST3:       return "FOOD_REST3";
    case FOOD_REST4:       return "FOOD_REST4";
    case FOOD_REST5:       return "FOOD_REST5";
    case FOOD_REST6:       return "FOOD_REST6";
    case FOOD_REST7:       return "FOOD_REST7";
    case FOOD_REST8:       return "FOOD_REST8";
    case FOOD_ORDER:       return "FOOD_ORDER";
    case FOOD_ORDER2:      return "FOOD_ORDER2";
    case FOOD_ORDER3:      return "FOOD_ORDER3";
    case FOOD_ORDER4:      return "FOOD_ORDER4";
    case FOOD_ORDER5:      return "FOOD_ORDER5";
    case FOOD_ORDER6:      return "FOOD_ORDER6";
    case FOOD_ORDER7:      return "FOOD_ORDER7";
    case FOOD_ORDER8:      return "FOOD_ORDER8";
    case HEALTH_MENU:      return "HEALTH_MENU";
    case HEALTH_WASH:      return "HEALTH_WASH";
    case HEALTH_WASH2:     return "HEALTH_WASH2";
    case HEALTH_WASH3:     return "HEALTH_WASH3";
    case HEALTH_WASH4:     return "HEALTH_WASH4";
    case HEALTH_WASH5:     return "HEALTH_WASH5";
    case HEALTH_DOCTOR:    return "HEALTH_DOCTOR";
    case HEALTH_DOCTOR2:   return "HEALTH_DOCTOR2";
    case HEALTH_DOCTOR3:   return "HEALTH_DOCTOR3";
    case HEALTH_DOCTOR4:   return "HEALTH_DOCTOR4";
    case HEALTH_DOCTOR5:   return "HEALTH_DOCTOR5";
    case HEALTH_DOCTOR6:   return "HEALTH_DOCTOR6";
    case HEALTH_TEMPLE:    return "HEALTH_TEMPLE";
    case HEALTH_TEMPLE2:   return "HEALTH_TEMPLE2";
    case HEALTH_TEMPLE3:   return "HEALTH_TEMPLE3";
    case HEALTH_TEMPLE4:   return "HEALTH_TEMPLE4";
    case HEALTH_TEMPLE5:   return "HEALTH_TEMPLE5";
    case HEALTH_TEMPLE6:   return "HEALTH_TEMPLE6";
    case HEALTH_ONSEN:     return "HEALTH_ONSEN";
    case REST_MENU:        return "REST_MENU";
    case REST_MEDITATE:    return "REST_MEDITATE";
    case REST_SLEEP:       return "REST_SLEEP";
    case STATS_SCREEN:     return "STATS_SCREEN";
    case GARDEN_MENU:      return "GARDEN_MENU";
    case GARDEN_LOOP:      return "GARDEN_LOOP";
    case GARDEN_PLANT:     return "GARDEN_PLANT";
    case GARDEN_WATER:     return "GARDEN_WATER";
    case GARDEN_PICK:      return "GARDEN_PICK";
    case GARDEN_CLEANUP:   return "GARDEN_CLEANUP";
    case TRAIN_MENU:       return "TRAIN_MENU";
    case TRAIN_SING:       return "TRAIN_SING";
    case TRAIN_SING2:      return "TRAIN_SING2";
    case TRAIN_SING3:      return "TRAIN_SING3";
    case TRAIN_DANCE:      return "TRAIN_DANCE";
    case TRAIN_DANCE2:     return "TRAIN_DANCE2";
    case TRAIN_DANCE3:     return "TRAIN_DANCE3";
    case TRAIN_SWIM:       return "TRAIN_SWIM";
    case TRAIN_SWIM2:      return "TRAIN_SWIM2";
    case TRAIN_SWIM3:      return "TRAIN_SWIM3";
    case TRAIN_GYM:        return "TRAIN_GYM";
    case TRAIN_GYM2:       return "TRAIN_GYM2";
    case TRAIN_GYM3:       return "TRAIN_GYM3";
    case TRAIN_RUN:        return "TRAIN_RUN";
    case TRAIN_RUN2:       return "TRAIN_RUN2";
    case TRAIN_RUN3:       return "TRAIN_RUN3";
    case TRAIN_LIBRARY:    return "TRAIN_LIBRARY";
    case COMP_MENU:        return "COMP_MENU";
    case COMP_LOCAL:       return "COMP_LOCAL";
    case COMP_DEPT:        return "COMP_DEPT";
    case COMP_REG:         return "COMP_REG";
    case COMP_NAT:         return "COMP_NAT";
    default:               return "UNKNOWN";
  }
}

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
  unloadImage(natsumiSprite);
  unloadImage(enemySprite);
  clearFoodGrid();

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
  // Serial.println("> Entering preloadImages() with currentState set to " + String(currentState));
  unloadAllImages();
  // Load backgrounds
  switch (currentState) {
    case M5_SCREEN:
      preloadImage("/idolnat/screens/m5_logo.png", currentBackground);
      break;
    case TITLE_SCREEN:
      preloadImage("/idolnat/screens/title02.png", currentBackground);
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
    case FLOWERS_MARKET: case FLOWERS_MARKET7:
      preloadImage("/idolnat/screens/flower_market_bg.png", currentBackground);
      break;
    case FLOWERS_MARKET2:
      preloadImage("/idolnat/screens/flower_market_step1.png", currentBackground);
      break;
    case FLOWERS_MARKET3:
      preloadImage("/idolnat/screens/flower_market_step2.png", currentBackground);
      break;
    case FLOWERS_MARKET4:
      preloadImage("/idolnat/screens/flower_market_step3.png", currentBackground);
      break;
    case FLOWERS_MARKET5:
      preloadImage("/idolnat/screens/flower_market_bg2.png", currentBackground);
      break;
    case FLOWERS_MARKET6:
      preloadImage("/idolnat/screens/flower_market_bg3.png", currentBackground);
      preloadImage("/idolnat/sprites/flower_stage_08-10x16.png", natsumiSprite);
      break;
    case FOOD_MENU:
      preloadImage("/idolnat/screens/kitchen.png", currentBackground);
      break;
    case FOOD_CONBINI:
      preloadImage("/idolnat/screens/conbimart_front.png", currentBackground);
      break;
    case FOOD_CONBINI2: case FOOD_CONBINI3:
      preloadImage("/idolnat/screens/conbimart_inside.png", currentBackground);
      break;
    case FOOD_COOK:
      preloadImage("/idolnat/screens/fridge_open.png", currentBackground);
      break;
    case FOOD_COOK2:
      preloadImage("/idolnat/screens/food_selected_bg.png", currentBackground);
      if (selectedFood == "Red apple") {
        preloadImage("/idolnat/sprites/food_001.png" ,currentIcon);
      } else if (selectedFood == "Green apple") {
          preloadImage("/idolnat/sprites/food_002.png" ,currentIcon);
      } else if (selectedFood == "Avocado") {
          preloadImage("/idolnat/sprites/food_003.png" ,currentIcon);
      } else if (selectedFood == "Bread") {
          preloadImage("/idolnat/sprites/food_005.png" ,currentIcon);
      } else if (selectedFood == "Banana") {
          preloadImage("/idolnat/sprites/food_008.png" ,currentIcon);
      } else if (selectedFood == "Broccoli") {
          preloadImage("/idolnat/sprites/food_015.png" ,currentIcon);
      } else if (selectedFood == "Sweets") {
          preloadImage("/idolnat/sprites/food_021.png" ,currentIcon);
      } else if (selectedFood == "Carrot") {
          preloadImage("/idolnat/sprites/food_028.png" ,currentIcon);
      } else if (selectedFood == "Meat") {
          preloadImage("/idolnat/sprites/food_033.png" ,currentIcon);
      } else if (selectedFood == "Coconut") {
          preloadImage("/idolnat/sprites/food_038.png" ,currentIcon);
      } else if (selectedFood == "Coconut juice") {
          preloadImage("/idolnat/sprites/food_039.png" ,currentIcon);
      } else if (selectedFood == "Coffee") {
          preloadImage("/idolnat/sprites/food_041.png" ,currentIcon);
      } else if (selectedFood == "Biscuit") {
          preloadImage("/idolnat/sprites/food_044.png" ,currentIcon);
      } else if (selectedFood == "Corn") {
          preloadImage("/idolnat/sprites/food_045.png" ,currentIcon);
      } else if (selectedFood == "Croissant") {
          preloadImage("/idolnat/sprites/food_046.png" ,currentIcon);
      } else if (selectedFood == "Fried egg") {
          preloadImage("/idolnat/sprites/food_053.png" ,currentIcon);
      } else if (selectedFood == "Grape") {
          preloadImage("/idolnat/sprites/food_061.png" ,currentIcon);
      } else if (selectedFood == "Kiwi") {
          preloadImage("/idolnat/sprites/food_081.png" ,currentIcon);
      } else if (selectedFood == "Milk") {
          preloadImage("/idolnat/sprites/food_092.png" ,currentIcon);
      } else if (selectedFood == "Orange") {
          preloadImage("/idolnat/sprites/food_109.png" ,currentIcon);
      } else if (selectedFood == "Peach") {
          preloadImage("/idolnat/sprites/food_111.png" ,currentIcon);
      } else if (selectedFood == "Pear") {
          preloadImage("/idolnat/sprites/food_113.png" ,currentIcon);
      } else if (selectedFood == "Strawberries") {
          preloadImage("/idolnat/sprites/food_149.png" ,currentIcon);
      } else if (selectedFood == "Maki") {
          preloadImage("/idolnat/sprites/food_150.png" ,currentIcon);
      } else if (selectedFood == "Sushi") {
          preloadImage("/idolnat/sprites/food_154.png" ,currentIcon);
      } else if (selectedFood == "Watermelon") {
          preloadImage("/idolnat/sprites/food_168.png" ,currentIcon);
      }
      break;
    case FOOD_REST: case FOOD_REST5:
      preloadImage("/idolnat/screens/restaurant_bg.png", currentBackground);
      break;
    case FOOD_REST2:
      preloadImage("/idolnat/screens/tofu_vegetable_teishoku.png", currentBackground);
      break;
    case FOOD_REST3:
      preloadImage("/idolnat/screens/grilled_salmon_teishoku.png", currentBackground);
      break;
    case FOOD_REST4:
      preloadImage("/idolnat/screens/fried_chicken_teishoku.png", currentBackground);
      break;
    case FOOD_REST6:
      preloadImage("/idolnat/screens/food_rest_step1.png", currentBackground);
      break;
    case FOOD_REST7:
      preloadImage("/idolnat/screens/food_rest_step2.png", currentBackground);
      break;
    case FOOD_REST8:
      preloadImage("/idolnat/screens/food_rest_step3.png", currentBackground);
      break;
    case FOOD_ORDER:
      preloadImage("/idolnat/screens/orderibi_boot_screen.png", currentBackground);
      break;
    case FOOD_ORDER2:
      preloadImage("/idolnat/screens/orderibi_cheap_bento.png", currentBackground);
      break;
    case FOOD_ORDER3:
      preloadImage("/idolnat/screens/orderibi_healthy_salad.png", currentBackground);
      break;
    case FOOD_ORDER4:
      preloadImage("/idolnat/screens/orderibi_deluxe_teishoku.png", currentBackground);
      break;
    case FOOD_ORDER5:
      preloadImage("/idolnat/screens/orderibi_order_confirmed.png", currentBackground);
      break;
    case FOOD_ORDER7: case FOOD_ORDER8:
      preloadImage("/idolnat/screens/orderibi_food_delivered.png", currentBackground);
      break;
    case HEALTH_WASH: case HEALTH_WASH5:
      preloadImage("/idolnat/screens/bathroom.png", currentBackground);
      break;
    case HEALTH_WASH2:
      preloadImage("/idolnat/screens/bathroom_step1.png", currentBackground);
      break;
    case HEALTH_WASH3:
      preloadImage("/idolnat/screens/bathroom_step2.png", currentBackground);
      break;
    case HEALTH_WASH4:
      preloadImage("/idolnat/screens/bathroom_step3.png", currentBackground);
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
    case GARDEN_LOOP: case GARDEN_PLANT: case GARDEN_WATER: case GARDEN_PICK: case GARDEN_CLEANUP: case GARDEN_MENU:
      preloadImage("/idolnat/screens/garden_bg.png", currentBackground);
      break;
    case STATS_SCREEN:
      break;
    case TRAIN_MENU:
      preloadImage("/idolnat/screens/map_training.png", currentBackground);
      break;
    case TRAIN_SING:
      preloadImage("/idolnat/screens/singing_school_bg_BW.png", currentBackground);
      break;
    case TRAIN_SING2:
      preloadImage("/idolnat/sprites/natsumi_head_sprite-22x20.png", natsumiSprite);
      break;
    case TRAIN_SING3:
      preloadImage("/idolnat/screens/singing_school_bg.png", currentBackground);
      break;
    case TRAIN_DANCE:
      preloadImage("/idolnat/screens/ballet_school_bg_BW.png", currentBackground);
      break;
    case TRAIN_DANCE3:
      preloadImage("/idolnat/screens/ballet_school_bg.png", currentBackground);
      break;
    case TRAIN_SWIM:
      preloadImage("/idolnat/screens/swimming_pool_bg_BW.png", currentBackground);
      break;
    case TRAIN_SWIM2:
      preloadImage("/idolnat/sprites/natsumi_head_sprite-22x20.png", natsumiSprite);
      preloadImage("/idolnat/sprites/shark_sprite-26x14.png", enemySprite);
      break;
    case TRAIN_SWIM3:
      preloadImage("/idolnat/screens/swimming_pool_bg.png", currentBackground);
      break;
    case TRAIN_GYM:
      preloadImage("/idolnat/screens/gym_bg_BW.png", currentBackground);
      break;
    case TRAIN_GYM3:
      preloadImage("/idolnat/screens/gym_bg.png", currentBackground);
      break;
    case TRAIN_RUN:
      preloadImage("/idolnat/screens/forest_bg_BW.png", currentBackground);
      break;
    case TRAIN_RUN2:
      preloadImage("/idolnat/sprites/natsumi_running_1-90x90.png", natsumiSprite);
      preloadImage("/idolnat/sprites/natsumi_running_2-90x90.png", enemySprite);
      break;
    case TRAIN_RUN3:
      preloadImage("/idolnat/screens/forest_bg.png", currentBackground);
      break;
    case TRAIN_LIBRARY:
      switch(natsumi.age) {
        case 11: case 12:
          preloadImage("/idolnat/screens/natsumi_11_library.png", currentBackground);
          break;
        case 13: case 14:
          preloadImage("/idolnat/screens/natsumi_13_library.png", currentBackground);
          break;
        case 15: case 16: case 17:
          preloadImage("/idolnat/screens/natsumi_15_library.png", currentBackground);
          break;
        case 18: case 19: case 20:
          preloadImage("/idolnat/screens/natsumi_18_library.png", currentBackground);
          break;
        case 21: case 22:
          preloadImage("/idolnat/screens/natsumi_21_library.png", currentBackground);
          break;
        default:
          preloadImage("/idolnat/screens/natsumi_21_library.png", currentBackground);
          break;
      }
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
        case FOOD_CONBINI3:
          preloadImage("/idolnat/sprites/cashier01-90x135.png", currentCharacter);
          break;
        case FOOD_REST: case FOOD_REST5:
          preloadImage("/idolnat/sprites/waitress01-90x135.png", currentCharacter);
          break;
        case FOOD_ORDER8:
          preloadImage("/idolnat/sprites/delivery_girl-90x135.png", currentCharacter);
          break;
        case HEALTH_WASH: case HEALTH_WASH5:
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
        case TRAIN_DANCE3:
          preloadImage("/idolnat/sprites/dance_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_SING3:
          preloadImage("/idolnat/sprites/music_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_SWIM3:
          preloadImage("/idolnat/sprites/swim_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_GYM3:
          preloadImage("/idolnat/sprites/gym_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_RUN3:
          preloadImage("/idolnat/sprites/gym_teacher-90x135.png", currentCharacter);
          break;
        default:
          if (isNatsumiHappy) {
            preloadImage("/idolnat/sprites/natsumi_11yo_happy-90x135.png", currentCharacter);
          } else if (natsumi.hunger < 1) {
            preloadImage("/idolnat/sprites/natsumi_11yo_hungry-90x135.png", currentCharacter);
          } else if (natsumi.hygiene < 1) {
            preloadImage("/idolnat/sprites/natsumi_11yo_dirty-90x135.png", currentCharacter);
          } else if (natsumi.energy < 1) {
            preloadImage("/idolnat/sprites/natsumi_11yo_tired-90x135.png", currentCharacter);
          } else if (natsumi.spirit < 1) {
            preloadImage("/idolnat/sprites/natsumi_11yo_sad-90x135.png", currentCharacter);
          } else {
            preloadImage("/idolnat/sprites/natsumi_11yo-90x135.png", currentCharacter);
          }
          break;
      }
      break;
    case 13: case 14:
      switch(currentState) {
        case REST_MEDITATE:
          preloadImage("/idolnat/sprites/natsumi_13yo_meditate-90x135.png", currentCharacter);
          break;
        case FOOD_CONBINI3:
          preloadImage("/idolnat/sprites/cashier01-90x135.png", currentCharacter);
          break;
        case FOOD_REST: case FOOD_REST5:
          preloadImage("/idolnat/sprites/waitress01-90x135.png", currentCharacter);
          break;
        case FOOD_ORDER8:
          preloadImage("/idolnat/sprites/delivery_girl-90x135.png", currentCharacter);
          break;
        case HEALTH_WASH: case HEALTH_WASH5:
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
        case TRAIN_DANCE3:
          preloadImage("/idolnat/sprites/dance_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_SING3:
          preloadImage("/idolnat/sprites/music_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_SWIM3:
          preloadImage("/idolnat/sprites/swim_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_GYM3:
          preloadImage("/idolnat/sprites/gym_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_RUN3:
          preloadImage("/idolnat/sprites/gym_teacher-90x135.png", currentCharacter);
          break;
        default:
          if (isNatsumiHappy) {
            preloadImage("/idolnat/sprites/natsumi_13yo_happy-90x135.png", currentCharacter);
          } else if (natsumi.hunger < 1) {
            preloadImage("/idolnat/sprites/natsumi_13yo_hungry-90x135.png", currentCharacter);
          } else if (natsumi.hygiene < 1) {
            preloadImage("/idolnat/sprites/natsumi_13yo_dirty-90x135.png", currentCharacter);
          } else if (natsumi.energy < 1) {
            preloadImage("/idolnat/sprites/natsumi_13yo_tired-90x135.png", currentCharacter);
          } else if (natsumi.spirit < 1) {
            preloadImage("/idolnat/sprites/natsumi_13yo_sad-90x135.png", currentCharacter);
          } else {
            preloadImage("/idolnat/sprites/natsumi_13yo-90x135.png", currentCharacter);
          }
          break;
      }
      break;
    case 15: case 16: case 17:
      switch(currentState) {
        case REST_MEDITATE:
          preloadImage("/idolnat/sprites/natsumi_15yo_meditate-90x135.png", currentCharacter);
          break;
        case FOOD_CONBINI3:
          preloadImage("/idolnat/sprites/cashier01-90x135.png", currentCharacter);
          break;
        case FOOD_REST: case FOOD_REST5:
          preloadImage("/idolnat/sprites/waitress01-90x135.png", currentCharacter);
          break;
        case FOOD_ORDER8:
          preloadImage("/idolnat/sprites/delivery_girl-90x135.png", currentCharacter);
          break;
        case HEALTH_WASH: case HEALTH_WASH5:
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
        case TRAIN_DANCE3:
          preloadImage("/idolnat/sprites/dance_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_SING3:
          preloadImage("/idolnat/sprites/music_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_SWIM3:
          preloadImage("/idolnat/sprites/swim_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_GYM3:
          preloadImage("/idolnat/sprites/gym_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_RUN3:
          preloadImage("/idolnat/sprites/gym_teacher-90x135.png", currentCharacter);
          break;
        default:
          if (isNatsumiHappy) {
            preloadImage("/idolnat/sprites/natsumi_15yo_happy-90x135.png", currentCharacter);
          } else if (natsumi.hunger < 1) {
            preloadImage("/idolnat/sprites/natsumi_15yo_hungry-90x135.png", currentCharacter);
          } else if (natsumi.hygiene < 1) {
            preloadImage("/idolnat/sprites/natsumi_15yo_dirty-90x135.png", currentCharacter);
          } else if (natsumi.energy < 1) {
            preloadImage("/idolnat/sprites/natsumi_15yo_tired-90x135.png", currentCharacter);
          } else if (natsumi.spirit < 1) {
            preloadImage("/idolnat/sprites/natsumi_15yo_sad-90x135.png", currentCharacter);
          } else {
            preloadImage("/idolnat/sprites/natsumi_15yo-90x135.png", currentCharacter);
          }
          break;
      }
      break;
    case 18: case 19: case 20:
      switch(currentState) {
        case REST_MEDITATE:
          preloadImage("/idolnat/sprites/natsumi_18yo_meditate-90x135.png", currentCharacter);
          break;
        case FOOD_CONBINI3:
          preloadImage("/idolnat/sprites/cashier01-90x135.png", currentCharacter);
          break;
        case FOOD_REST: case FOOD_REST5:
          preloadImage("/idolnat/sprites/waitress01-90x135.png", currentCharacter);
          break;
        case FOOD_ORDER8:
          preloadImage("/idolnat/sprites/delivery_girl-90x135.png", currentCharacter);
          break;
        case HEALTH_WASH: case HEALTH_WASH5:
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
        case TRAIN_DANCE3:
          preloadImage("/idolnat/sprites/dance_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_SING3:
          preloadImage("/idolnat/sprites/music_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_SWIM3:
          preloadImage("/idolnat/sprites/swim_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_GYM3:
          preloadImage("/idolnat/sprites/gym_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_RUN3:
          preloadImage("/idolnat/sprites/gym_teacher-90x135.png", currentCharacter);
          break;
        default:
          if (isNatsumiHappy) {
            preloadImage("/idolnat/sprites/natsumi_18yo_happy-90x135.png", currentCharacter);
          } else if (natsumi.hunger < 1) {
            preloadImage("/idolnat/sprites/natsumi_18yo_hungry-90x135.png", currentCharacter);
          } else if (natsumi.hygiene < 1) {
            preloadImage("/idolnat/sprites/natsumi_18yo_dirty-90x135.png", currentCharacter);
          } else if (natsumi.energy < 1) {
            preloadImage("/idolnat/sprites/natsumi_18yo_tired-90x135.png", currentCharacter);
          } else if (natsumi.spirit < 1) {
            preloadImage("/idolnat/sprites/natsumi_18yo_sad-90x135.png", currentCharacter);
          } else {
            preloadImage("/idolnat/sprites/natsumi_18yo-90x135.png", currentCharacter);
          }
          break;
      }
      break;
    case 21: case 22:
      switch(currentState) {
        case REST_MEDITATE:
          preloadImage("/idolnat/sprites/natsumi_21yo_meditate-90x135.png", currentCharacter);
          break;
        case FOOD_CONBINI3:
          preloadImage("/idolnat/sprites/cashier01-90x135.png", currentCharacter);
          break;
        case FOOD_REST: case FOOD_REST5:
          preloadImage("/idolnat/sprites/waitress01-90x135.png", currentCharacter);
          break;
        case FOOD_ORDER8:
          preloadImage("/idolnat/sprites/delivery_girl-90x135.png", currentCharacter);
          break;
        case HEALTH_WASH: case HEALTH_WASH5:
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
        case TRAIN_DANCE3:
          preloadImage("/idolnat/sprites/dance_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_SING3:
          preloadImage("/idolnat/sprites/music_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_SWIM3:
          preloadImage("/idolnat/sprites/swim_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_GYM3:
          preloadImage("/idolnat/sprites/gym_teacher-90x135.png", currentCharacter);
          break;
        case TRAIN_RUN3:
          preloadImage("/idolnat/sprites/gym_teacher-90x135.png", currentCharacter);
          break;
        default:
          if (isNatsumiHappy) {
            preloadImage("/idolnat/sprites/natsumi_21yo_happy-90x135.png", currentCharacter);
          } else if (natsumi.hunger < 1) {
            preloadImage("/idolnat/sprites/natsumi_21yo_hungry-90x135.png", currentCharacter);
          } else if (natsumi.hygiene < 1) {
            preloadImage("/idolnat/sprites/natsumi_21yo_dirty-90x135.png", currentCharacter);
          } else if (natsumi.energy < 1) {
            preloadImage("/idolnat/sprites/natsumi_21yo_tired-90x135.png", currentCharacter);
          } else if (natsumi.spirit < 1) {
            preloadImage("/idolnat/sprites/natsumi_21yo_sad-90x135.png", currentCharacter);
          } else {
            preloadImage("/idolnat/sprites/natsumi_21yo-90x135.png", currentCharacter);
          }
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
  // Serial.println("> currentState = " + String(gameStateToString(currentState)));
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
  // Serial.println("> Entering changeState() with baseLayer set to " + String(baseLayer) + " and targetState set to " + String(targetState) + " with delay set to " + String(delay));
  if (changeStateCounter == delay) {
    Serial.println("Proceed with transition");
    Serial.println("> currentState = " + String(gameStateToString(currentState)));
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
        natsumi.money = 1800;
        natsumi.flowers = 0;
        natsumi.lastHungerUpdate = 0;
        natsumi.lastHygieneUpdate = 0;
        natsumi.lastEnergyUpdate = 0;
        fridge.redApple = 3;
        fridge.greenApple = 0;
        fridge.avocado = 0;
        fridge.bread = 10;
        fridge.banana = 6;
        fridge.broccoli = 0;
        fridge.sweets = 1;
        fridge.carrot = 0;
        fridge.meat = 1;
        fridge.coconut = 1;
        fridge.coconutJuice = 0;
        fridge.coffee = 0;
        fridge.biscuits = 0;
        fridge.corn = 0;
        fridge.croissant = 0;
        fridge.friedEgg = 0;
        fridge.grapes = 0;
        fridge.kiwi = 0;
        fridge.milk = 0;
        fridge.orange = 2;
        fridge.peach = 0;
        fridge.pear = 0;
        fridge.strawberries = 0;
        fridge.maki = 0;
        fridge.sushi = 0;
        fridge.watermelon = 0;
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
        natsumi.money = 1800;
        natsumi.flowers = 0;
        natsumi.lastHungerUpdate = 0;
        natsumi.lastHygieneUpdate = 0;
        natsumi.lastEnergyUpdate = 0;
        fridge.redApple = 3;
        fridge.greenApple = 0;
        fridge.avocado = 0;
        fridge.bread = 10;
        fridge.banana = 6;
        fridge.broccoli = 0;
        fridge.sweets = 1;
        fridge.carrot = 0;
        fridge.meat = 1;
        fridge.coconut = 1;
        fridge.coconutJuice = 0;
        fridge.coffee = 0;
        fridge.biscuits = 0;
        fridge.corn = 0;
        fridge.croissant = 0;
        fridge.friedEgg = 0;
        fridge.grapes = 0;
        fridge.kiwi = 0;
        fridge.milk = 0;
        fridge.orange = 2;
        fridge.peach = 0;
        fridge.pear = 0;
        fridge.strawberries = 0;
        fridge.maki = 0;
        fridge.sushi = 0;
        fridge.watermelon = 0;
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
      case STATS_SCREEN:
        screenConfig = GAME;
        overlayActive = true;
        l5NeedsRedraw = true;
        toastEnabled = false;
        break;
      case FLOWERS_MARKET:
        screenConfig = IDLE;
        characterEnabled = false;
        break;
      case FLOWERS_MARKET2: case FLOWERS_MARKET3: case FLOWERS_MARKET4:
        screenConfig = CARD;
        characterEnabled = false;
        break;
      case FLOWERS_MARKET5: case FLOWERS_MARKET6:
        screenConfig = GAME;
        overlayActive = true;
        l5NeedsRedraw = true;
        toastEnabled = false;
        break;
      case FLOWERS_MARKET7:
        screenConfig = DIALOG;
        overlayActive = true;
        l5NeedsRedraw = true;
        break;
      case FOOD_MENU:
        screenConfig = ROOM;
        currentMenuType = "food";
        currentMenuItems = foodMenuItems;
        currentMenuItemsCount = foodMenuItemCount;
        break;
      case FOOD_CONBINI:
        screenConfig = IDLE;
        characterEnabled = false;
        break;
      case FOOD_CONBINI2:
        screenConfig = ROOM;
        characterEnabled = false;
        break;
      case FOOD_CONBINI3:
        screenConfig = DIALOG;
        overlayActive = true;
        l5NeedsRedraw = true;
        break;
      case FOOD_COOK:
        screenConfig = ROOM;
        break;
      case FOOD_COOK2:
        overlayActive = true;
        l5NeedsRedraw = true;
        break;
      case FOOD_REST: case FOOD_REST5:
        screenConfig = DIALOG;
        overlayActive = true;
        l5NeedsRedraw = true;
        break;
      case FOOD_REST2: case FOOD_REST3: case FOOD_REST4:
        screenConfig = IDLE;
        characterEnabled = false;
        break;
      case FOOD_REST6: case FOOD_REST7: case FOOD_REST8:
        screenConfig = CARD;
        characterEnabled = false;
        break;
      case FOOD_ORDER: case FOOD_ORDER2: case FOOD_ORDER3: case FOOD_ORDER4:
        screenConfig = IDLE;
        overlayActive = true;
        l5NeedsRedraw = true;
        characterEnabled = false;
        break;
      case FOOD_ORDER5:
        screenConfig = IDLE;
        overlayActive = false;
        l5NeedsRedraw = false;
        characterEnabled = true;
        break;
      case FOOD_ORDER6: case FOOD_ORDER7:
        screenConfig = ROOM;
        break;
      case FOOD_ORDER8:
        screenConfig = DIALOG;
        overlayActive = true;
        l5NeedsRedraw = true;
        break;
      case TRAIN_MENU:
        screenConfig = ROOM;
        currentMenuType = "training";
        currentMenuItems = trainingMenuItems;
        currentMenuItemsCount = trainingMenuItemCount;
        break;
      case TRAIN_DANCE:
        screenConfig = ROOM;
        overlayActive = true;
        l5NeedsRedraw = true;
        break;
      case TRAIN_DANCE2:
        screenConfig = GAME;
        break;
      case TRAIN_DANCE3:
        screenConfig = DIALOG;
        overlayActive = true;
        l5NeedsRedraw = true;
        break;
      case TRAIN_SING:
        screenConfig = ROOM;
        overlayActive = true;
        l5NeedsRedraw = true;
        break;
      case TRAIN_SING2:
        screenConfig = GAME;
        break;
      case TRAIN_SING3:
        screenConfig = DIALOG;
        overlayActive = true;
        l5NeedsRedraw = true;
        break;
      case TRAIN_SWIM:
        screenConfig = ROOM;
        overlayActive = true;
        l5NeedsRedraw = true;
        break;
      case TRAIN_SWIM2:
        screenConfig = GAME;
        break;
      case TRAIN_SWIM3:
        screenConfig = DIALOG;
        overlayActive = true;
        l5NeedsRedraw = true;
        break;
      case TRAIN_GYM:
        screenConfig = ROOM;
        overlayActive = true;
        l5NeedsRedraw = true;
        break;
      case TRAIN_GYM2:
        screenConfig = GAME;
        break;
      case TRAIN_GYM3:
        screenConfig = DIALOG;
        overlayActive = true;
        l5NeedsRedraw = true;
        break;
      case TRAIN_RUN:
        screenConfig = ROOM;
        overlayActive = true;
        l5NeedsRedraw = true;
        break;
      case TRAIN_RUN2:
        screenConfig = GAME;
        break;
      case TRAIN_RUN3:
        screenConfig = DIALOG;
        overlayActive = true;
        l5NeedsRedraw = true;
        break;
      case TRAIN_LIBRARY:
        screenConfig = ROOM;
        libraryInitialized = false;
        libraryRewardApplied = false;
        librarySegmentsFilled = 0;
        libraryStartTime = 0;
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
      case HEALTH_WASH2: case HEALTH_WASH3: case HEALTH_WASH4:
        screenConfig = CARD;
        break;
      case HEALTH_WASH5:
        screenConfig = ROOM;
        menuEnabled = false;
        l4NeedsRedraw = false;
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
        menuOpened = false;
        break;
      case GARDEN_MENU:
        screenConfig = ROOM;
        currentMenuType = "garden";
        currentMenuItems = gardenMenuItems;
        currentMenuItemsCount = gardenMenuItemCount;
        menuOpened = true;
        break;
      case GARDEN_PLANT: case GARDEN_WATER: case GARDEN_PICK: case GARDEN_CLEANUP:
        screenConfig = ROOM;
        menuOpened = false;
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
    natsumi.money += 70000;
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
  // Serial.println("> Entering updateSpirit()");
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
    if (waitingForFoodDelivery) {
      Serial.println(">>> Pending food delivery (" + String(foodDeliveryCounter) + ")");
      foodDeliveryCounter += 1;
      if (foodDeliveryCounter == 10) {
        Serial.println(">>> Food delivered");
        foodDeliveryCounter = 0;
        waitingForFoodDelivery = false;
        Serial.println(">>> Switching to FOOD_ORDER7");
        changeState(0, FOOD_ORDER7, 0);
      }
    }
    if (gardenActive) {
      // Grow the seeds
      for (int row = 0; row < gardenRows; row++) {
        for (int col = 0; col < gardenCols; col++) {
          int tileValue = gardenTiles[row][col];
          if (tileValue > 1) {
            gardenTiles[row][col] += 5;
            Serial.println(">>> Gardening -> Current tile value is " + String(tileValue));  
          }
        }
      }
    }
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
    case FLOWERS_MARKET2:
      changeState(0, FLOWERS_MARKET3, 10);
      break;
    case FLOWERS_MARKET3:
      changeState(0, FLOWERS_MARKET4, 10);
      break;
    case FLOWERS_MARKET4:
      changeState(0, FLOWERS_MARKET5, 10);
      break;
    case FOOD_REST6:
      changeState(0, FOOD_REST7, 20);
      break;
    case FOOD_REST7:
      changeState(0, FOOD_REST8, 20);
      break;
    case FOOD_REST8:
      changeState(0, HOME_LOOP, 20);
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
    case HEALTH_WASH2:
      changeState(0, HEALTH_WASH3, 20);
      break;
    case HEALTH_WASH3:
      changeState(0, HEALTH_WASH4, 20);
      break;
    case HEALTH_WASH4:
      changeState(0, HEALTH_WASH5, 20);
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
    case FLOWERS_MARKET7:
      miniGameDebrief();
      break;
    case FOOD_CONBINI3:
      cashier();
      break;
    case FOOD_ORDER8:
      foodDelivery();
      break;
    case FOOD_REST: case FOOD_REST5:
      gotoRestaurant();
      break;
    case HEALTH_DOCTOR: case HEALTH_DOCTOR6:
      doctor();
      break;
    case HEALTH_TEMPLE: case HEALTH_TEMPLE6:
      priest();
      break;
    case TRAIN_DANCE3:
      miniGameDebrief();
      break;
    case TRAIN_SING3:
      miniGameDebrief();
      break;
    case TRAIN_SWIM3:
      miniGameDebrief();
      break;
    case TRAIN_GYM3:
      miniGameDebrief();
      break;
    case TRAIN_RUN3:
      miniGameDebrief();
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
    case TRAIN_DANCE2:
      manageTrainDanceGame();
      break;
    case TRAIN_SING2:
      manageTrainSingGame();
      break;
    case TRAIN_SWIM2:
      manageTrainSwimGame();
      break;
    case TRAIN_GYM2:
      manageTrainGymGame();
      break;
    case TRAIN_RUN2:
      manageTrainRunGame();
      break;
    case FLOWERS_MARKET5:
      manageFlowersMarket();
      break;
    case FLOWERS_MARKET6:
      manageFlowersSale();
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
    case FLOWERS_MARKET:
      if (natsumi.flowers > 0) {
        characterEnabled = false;
        changeState(0, FLOWERS_MARKET2, microWait);
      } else {
        changeState(0, HOME_LOOP, 0);
        showToast("No flowers to sell");
      }
      break;
    case FOOD_CONBINI:
      characterEnabled = false;
      changeState(0, FOOD_CONBINI2, microWait);
      break;
    case FOOD_ORDER:
      changeState(0, FOOD_ORDER2, microWait);
      break;
    case FOOD_ORDER2: case FOOD_ORDER3: case FOOD_ORDER4:
      orderibiFoodSelection();
      break;
    case FOOD_ORDER5:
      changeState(0, FOOD_ORDER6, microWait);
      break;
    case FOOD_REST2: case FOOD_REST3: case FOOD_REST4:
      restaurantFoodSelection();
      break;
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
    case HEALTH_WASH5:
      wash();
      break;
    case GARDEN_MENU:
      overlayEnabled = false;
      break;
    case GARDEN_LOOP: case GARDEN_PLANT: case GARDEN_WATER: case GARDEN_PICK: case GARDEN_CLEANUP:
      manageGarden();
      break;
    case FOOD_MENU:
      menuOpened = true;
      break;
    case FOOD_CONBINI2:
      gotoConbimart();
      break;
    case FOOD_COOK:
      cookFood();
      break;
    case FOOD_COOK2:
      showFood();
      break;
    case FOOD_ORDER6:
      waitingForFoodDelivery = true;
      changeState(0, HOME_LOOP, 0);
      break;
    case FOOD_ORDER7:
      Serial.println(">>> In FOOD_ORDER7 waiting loop");
      changeState(0, FOOD_ORDER8, microWait);
      break;
    case TRAIN_DANCE:
      characterEnabled = false;
      manageMiniGameCountdown();
      break;
    case TRAIN_LIBRARY:
      characterEnabled = false;
      manageLibrary();
      break;
    case TRAIN_SING:
      characterEnabled = false;
      manageMiniGameCountdown();
      break;
    case TRAIN_SWIM:
      characterEnabled = false;
      manageMiniGameCountdown();
      break;
    case TRAIN_GYM:
      characterEnabled = false;
      manageMiniGameCountdown();
      break;
    case TRAIN_RUN:
      characterEnabled = false;
      manageMiniGameCountdown();
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
  } else if (currentMenuType == "garden") {
    selectionPtr = &gardenMenuSelection;
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

void drawGardenTile(int topX, int topY, int tileW, int tileH, uint16_t fillColor, uint16_t borderColor) {
  int halfW = tileW / 2;
  int halfH = tileH / 2;
  int leftX = topX - halfW;
  int rightX = topX + halfW;
  int midY = topY + halfH;
  int bottomY = topY + tileH;

  M5Cardputer.Display.fillTriangle(topX, topY, leftX, midY, rightX, midY, fillColor);
  M5Cardputer.Display.fillTriangle(topX, bottomY, leftX, midY, rightX, midY, fillColor);

  M5Cardputer.Display.drawLine(topX, topY, leftX, midY, borderColor);
  M5Cardputer.Display.drawLine(leftX, midY, topX, bottomY, borderColor);
  M5Cardputer.Display.drawLine(topX, bottomY, rightX, midY, borderColor);
  M5Cardputer.Display.drawLine(rightX, midY, topX, topY, borderColor);
}

void drawGardenPlanter() {
  const int tileW = 48;
  const int tileH = 36;
  const int originX = 147;
  const int originY = 7;
  const uint16_t soilColor = M5Cardputer.Display.color565(120, 86, 48);
  const uint16_t soilWetColor = M5Cardputer.Display.color565(70, 110, 150);
  const uint16_t soilBorder = M5Cardputer.Display.color565(170, 120, 70);
  const uint16_t activeBorder = YELLOW;
  const uint16_t sproutColor = M5Cardputer.Display.color565(80, 200, 90);
  const uint16_t waterColor = M5Cardputer.Display.color565(90, 180, 255);

  gardenActive = false;
  for (int row = 0; row < gardenRows; row++) {
    for (int col = 0; col < gardenCols; col++) {
      int topX = originX + (col - row) * (tileW / 2);
      int topY = originY + (col + row) * (tileH / 2);
      int tileValue = gardenTiles[row][col];
      uint16_t fillColor = (tileValue == 2) ? soilWetColor : soilColor;
      uint16_t borderColor = (row == gardenCursorRow && col == gardenCursorCol) ? activeBorder : soilBorder;

      drawGardenTile(topX, topY, tileW, tileH, fillColor, borderColor);
      int centerX = topX;
      int centerY = topY + (tileH / 2);

      if (tileValue > 1) {
        gardenActive = true;
      }

      if (tileValue == 1) {
        M5Cardputer.Display.fillCircle(centerX, centerY + 2, 3, sproutColor);
        M5Cardputer.Display.drawFastVLine(centerX, centerY - 2, 4, sproutColor);
      } else if (tileValue == 2) {
        M5Cardputer.Display.fillCircle(centerX + 6, centerY + 4, 2, waterColor);
      } else if (tileValue > 2 && tileValue < 30) {
        preloadImage("/idolnat/sprites/flower_stage_01-22x16.png", natsumiSprite);
        M5Cardputer.Display.drawPng(natsumiSprite.data, natsumiSprite.length, centerX - 6, centerY - 6);
        unloadImage(natsumiSprite);
      } else if (tileValue > 30 && tileValue <= 60) {
        preloadImage("/idolnat/sprites/flower_stage_02-20x16.png", natsumiSprite);
        M5Cardputer.Display.drawPng(natsumiSprite.data, natsumiSprite.length, centerX - 6, centerY - 6);
        unloadImage(natsumiSprite);
      } else if (tileValue > 60 && tileValue <= 90) {
        preloadImage("/idolnat/sprites/flower_stage_03-14x16.png", natsumiSprite);
        M5Cardputer.Display.drawPng(natsumiSprite.data, natsumiSprite.length, centerX - 6, centerY - 6);
        unloadImage(natsumiSprite);
      } else if (tileValue > 90 && tileValue <= 120) {
        preloadImage("/idolnat/sprites/flower_stage_04-11x16.png", natsumiSprite);
        M5Cardputer.Display.drawPng(natsumiSprite.data, natsumiSprite.length, centerX - 6, centerY - 6);
        unloadImage(natsumiSprite);
      } else if (tileValue > 120 && tileValue <= 150) {
        preloadImage("/idolnat/sprites/flower_stage_05-10x16.png", natsumiSprite);
        M5Cardputer.Display.drawPng(natsumiSprite.data, natsumiSprite.length, centerX - 6, centerY - 6);
        unloadImage(natsumiSprite);
      } else if (tileValue > 150 && tileValue <= 180) {
        preloadImage("/idolnat/sprites/flower_stage_06-12x16.png", natsumiSprite);
        M5Cardputer.Display.drawPng(natsumiSprite.data, natsumiSprite.length, centerX - 6, centerY - 6);
        unloadImage(natsumiSprite);
      } else if (tileValue > 180 && tileValue <= 210) {
        preloadImage("/idolnat/sprites/flower_stage_07-10x16.png", natsumiSprite);
        M5Cardputer.Display.drawPng(natsumiSprite.data, natsumiSprite.length, centerX - 6, centerY - 6);
        unloadImage(natsumiSprite);
      } else if (tileValue > 210) {
        preloadImage("/idolnat/sprites/flower_stage_08-10x16.png", natsumiSprite);
        M5Cardputer.Display.drawPng(natsumiSprite.data, natsumiSprite.length, centerX - 6, centerY - 6);
        unloadImage(natsumiSprite);
      }
    }
  }

  if (!menuOpened) {
    M5Cardputer.Display.fillRect(0, 125, 240, 10, BLACK);
    drawText("ARROWS: Move  ENTER: Menu  ESC: Home", 120, 131, true, WHITE, 1);
  }
}

void manageGarden() {
  // Serial.println("> Entering manageGarden()");
  // Serial.println(">> currentState set to " + String(currentState));
  overlayActive = true;
  updateAging();
  updateStats();

  int &tile = gardenTiles[gardenCursorRow][gardenCursorCol];
  Serial.println(">> tile: " + String(tile));
  switch (currentState) {
    case GARDEN_PLANT:
      Serial.println(">> GARDEN_PLANT");
      if (tile == 0) {
        tile = 1;
        showToast("Seed planted");
      } else {
        showToast("Tile already planted");
      }
      changeState(0, GARDEN_LOOP, 0);
      break;
    case GARDEN_WATER:
      Serial.println(">> GARDEN_WATER");
      if (tile == 0) {
        showToast("Nothing to water");
      } else if (tile == 1) {
        tile = 2;
        showToast("Watered");
      } else {
        showToast("No need to water");
      }
      changeState(0, GARDEN_LOOP, 0);
      break;
    case GARDEN_PICK:
      Serial.println(">> GARDEN_PICK");
      if (tile == 0) {
        showToast("Nothing to pick");
      } else if (tile > 209) {
        if (natsumi.flowers < 24) {
          tile = 0;
          natsumi.flowers += 1;
          showToast("Natsumi now has " + String(natsumi.flowers) + " flowers");
        } else {
          showToast("FLowers storage full. Sell some");
        }
      } else {
        showToast("Not ready yet");
      }
      changeState(0, GARDEN_LOOP, 0);
      break;
    case GARDEN_CLEANUP:
      Serial.println(">> GARDEN_CLEANUP");
      tile = 0;
      showToast("Tile cleaned");
      changeState(0, GARDEN_LOOP, 0);
      break;
    case GARDEN_LOOP: {
      Serial.println(">> GARDEN_LOOP");
      uint8_t key = 0;
      if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
        auto keyList = M5Cardputer.Keyboard.keyList();
        if (keyList.size() > 0) {
          key = M5Cardputer.Keyboard.getKey(keyList[0]);
          bool moved = false;
          switch (key) {
            // UP
            case 181: case 59: case 'w': case 'W':
              Serial.println(">>> UP");
              if (gardenCursorRow > 0) {
                gardenCursorRow--;
                moved = true;
              }
              break;
            // DOWN
            case 182: case 46: case 's': case 'S':
              Serial.println(">>> DOWN");
              if (gardenCursorRow < gardenRows - 1) {
                gardenCursorRow++;
                moved = true;
              }
              break;
            // LEFT
            case 180: case 44: case 'a': case 'A':
              Serial.println(">>> LEFT");
              if (gardenCursorCol > 0) {
                gardenCursorCol--;
                moved = true;
              }
              break;
            // RIGHT
            case 183: case 47: case 'd': case 'D':
              Serial.println(">>> RIGHT");
              if (gardenCursorCol < gardenCols - 1) {
                gardenCursorCol++;
                moved = true;
              }
              break;
            // ENTER
            case 13: case 40: case ' ':
              Serial.println(">>> ENTER");
              menuOpened = true;
              changeState(0, GARDEN_MENU, 0);
              break;
            // ESC
            case 96:
              Serial.println(">>> ESC");
              menuOpened = false;
              changeState(0, HOME_LOOP, 0);
              return;
          }
          if (moved) {
            l5NeedsRedraw = true;
          }
        }
      }
      break;
    }
    case GARDEN_MENU:
      // Serial.println(">> GARDEN_MENU");
      break;
    default:
      showToast("Meh!");
      break;
  }
  return;
}

bool miniGameCountdownActive = false;
unsigned long miniGameCountdownStart = 0;
int miniGameCountdownValue = 3;

void resetMiniGameCountdown() {
  miniGameCountdownActive = false;
  miniGameCountdownStart = 0;
  miniGameCountdownValue = 3;
}

void drawMiniGameCountdown() {
  drawText(String(miniGameCountdownValue), 120, 67, true, RED, 7, BLACK);
}

void manageMiniGameCountdown() {
  if (!miniGameCountdownActive) {
    miniGameCountdownActive = true;
    miniGameCountdownStart = millis();
    miniGameCountdownValue = 3;
    switch(currentState) {
      case TRAIN_DANCE:
        resetTrainDanceGame();
        break;
      case TRAIN_SING:
        resetTrainSingGame();
        break;
      case TRAIN_GYM:
        resetTrainGymGame();
        break;
      case TRAIN_RUN:
        resetTrainRunGame();
        break;
      case TRAIN_SWIM:
        resetTrainSwimGame();
        break;
      default:
        break;
    }
    l5NeedsRedraw = true;
  }

  unsigned long now = millis();
  if (now - miniGameCountdownStart >= 1000) {
    miniGameCountdownValue--;
    miniGameCountdownStart = now;
    l5NeedsRedraw = true;
    if (miniGameCountdownValue == 0) {
      resetMiniGameCountdown();
      switch(currentState) {
        case TRAIN_DANCE:
          changeState(0, TRAIN_DANCE2, 0);
          break;
        case TRAIN_SING:
          changeState(0, TRAIN_SING2, 0);
          break;
      case TRAIN_SWIM:
        changeState(0, TRAIN_SWIM2, 0);
        break;
      case TRAIN_GYM:
        changeState(0, TRAIN_GYM2, 0);
        break;
      case TRAIN_RUN:
        changeState(0, TRAIN_RUN2, 0);
        break;
      default:
        break;
      }
      return;
    }
  }
}

// === TRAIN_DANCE2 Mini-game ===
void resetTrainDanceGame() {
  danceScore = 0;
  danceCuesShown = 0;
  danceCurrentDirection = -1;
  danceCueActive = false;
  danceCueStart = 0;
  danceNextCueTime = 0;
  danceCompletionTime = 0;
  danceGameRunning = false;
  danceGameCompleted = false;
  danceNeedsRedraw = true;
}

void spawnDanceCue() {
  danceCurrentDirection = static_cast<int>(random(0, 4));
  danceCueStart = millis();
  danceCueActive = true;
  danceCuesShown++;
  danceNeedsRedraw = true;
}

void startTrainDanceGame() {
  resetTrainDanceGame();
  overlayActive = false;
  danceGameRunning = true;
  M5Cardputer.Display.fillScreen(BLACK);
  spawnDanceCue();
}

void drawDanceArrow(int direction) {
  const int cx = M5Cardputer.Display.width() / 2;
  const int cy = M5Cardputer.Display.height() / 2;
  const int size = 32;
  const uint16_t arrowColor = M5Cardputer.Display.color565(255, 120, 180);

  switch (direction) {
    case 0:  // UP
      M5Cardputer.Display.fillTriangle(cx, cy - size, cx - size, cy + size, cx + size, cy + size, arrowColor);
      break;
    case 1:  // DOWN
      M5Cardputer.Display.fillTriangle(cx - size, cy - size, cx + size, cy - size, cx, cy + size, arrowColor);
      break;
    case 2:  // LEFT
      M5Cardputer.Display.fillTriangle(cx + size, cy - size, cx + size, cy + size, cx - size, cy, arrowColor);
      break;
    case 3:  // RIGHT
      M5Cardputer.Display.fillTriangle(cx - size, cy - size, cx - size, cy + size, cx + size, cy, arrowColor);
      break;
    default:
      break;
  }
}

void drawTrainDancePlayfield() {
  const int screenWidth = M5Cardputer.Display.width();
  const int screenHeight = M5Cardputer.Display.height();

  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setTextColor(WHITE, BLACK);
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setTextDatum(top_left);
  M5Cardputer.Display.drawString(String("Score: ") + danceScore + String("/") + danceTargetScore, 6, 6);

  if (danceGameCompleted) {
    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.drawString("Training complete!", screenWidth / 2, screenHeight / 2);
    return;
  }

  if (danceCueActive && danceCurrentDirection >= 0) {
    drawDanceArrow(danceCurrentDirection);
    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.drawString("Hit the matching arrow!", screenWidth / 2, screenHeight - 12);
  } else {
    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.drawString("Get ready...", screenWidth / 2, screenHeight / 2);
  }
}

// === TRAIN_SING2 Mini-game ===
void resetTrainSingGame() {
  singNotes.clear();
  singNotesCollected = 0;
  singNotesSpawned = 0;
  singPlayerColumn = singColumnCount / 2;
  singLastSpawnTime = 0;
  singCompletionTime = 0;
  singGameRunning = false;
  singGameCompleted = false;
}

void startTrainSingGame() {
  resetTrainSingGame();
  singColumnWidth = M5Cardputer.Display.width() / singColumnCount;
  singPlayerY = M5Cardputer.Display.height() - 12;
  overlayActive = false;
  singGameRunning = true;
  M5Cardputer.Display.fillScreen(BLACK);
}

void drawTrainSingPlayfield(bool showCompletion) {
  const int screenWidth = M5Cardputer.Display.width();
  const int screenHeight = M5Cardputer.Display.height();
  const uint16_t laneColor = M5Cardputer.Display.color565(40, 40, 60);
  const uint16_t noteColor = M5Cardputer.Display.color565(255, 215, 0);
  const uint16_t playerColor = M5Cardputer.Display.color565(120, 200, 255);

  M5Cardputer.Display.fillScreen(BLACK);

  for (int i = 1; i < singColumnCount; i++) {
    int x = i * singColumnWidth;
    M5Cardputer.Display.drawFastVLine(x, 0, screenHeight, laneColor);
  }

  int groundY = singPlayerY + (singPlayerHeight / 2);
  M5Cardputer.Display.drawFastHLine(0, groundY, screenWidth, laneColor);

  for (const auto &note : singNotes) {
    if (!note.active) continue;
    int x = note.column * singColumnWidth + (singColumnWidth / 2);
    M5Cardputer.Display.fillCircle(x, note.y, singNoteRadius, noteColor);
  }

  int playerCenterX = singPlayerColumn * singColumnWidth + (singColumnWidth / 2);
  M5Cardputer.Display.drawPng(natsumiSprite.data, natsumiSprite.length, playerCenterX - (singPlayerWidth / 2), groundY - singPlayerHeight);

  M5Cardputer.Display.setTextDatum(top_left);
  M5Cardputer.Display.setTextColor(WHITE, BLACK);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.drawString(String("Notes: ") + singNotesCollected + String("/") + singTargetNotes, 6, 4);

  if (showCompletion) {
    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.drawString("Training complete!", screenWidth / 2, screenHeight / 2);
  }
}

void manageTrainSingGame() {
  if (!singGameRunning && !singGameCompleted) {
    startTrainSingGame();
  }

  unsigned long now = millis();

  if (singGameCompleted) {
    drawTrainSingPlayfield(true);
    if (now - singCompletionTime >= 1200) {
      changeState(0, TRAIN_SING3, 0);
    }
    return;
  }

  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (!keyList.empty()) {
      uint8_t key = M5Cardputer.Keyboard.getKey(keyList[0]);
      switch (key) {
        case 44: case 'a': case 'A':  // LEFT
          if (singPlayerColumn > 0) {
            singPlayerColumn--;
          } else {
            singPlayerColumn = singColumnCount - 1;
          }
          break;
        case 47: case 'd': case 'D':  // RIGHT
          if (singPlayerColumn < singColumnCount - 1) {
            singPlayerColumn++;
          } else {
            singPlayerColumn = 0;
          }
          break;
        default:
          break;
      }
    }
  }

  if (now - singLastSpawnTime >= singNoteSpawnInterval) {
    if (singNotes.size() < 12) {
      FallingNote newNote = {static_cast<int>(random(0, singColumnCount)), 0, true};
      singNotes.push_back(newNote);
      singNotesSpawned++;
    }
    singLastSpawnTime = now;
  }

  for (auto &note : singNotes) {
    if (!note.active) continue;
    note.y += singNoteFallSpeed;
    if (note.y >= singPlayerY - singNoteRadius) {
      if (note.column == singPlayerColumn) {
        singNotesCollected++;
        note.active = false;
      } else if (note.y > M5Cardputer.Display.height()) {
        note.active = false;
      }
    }
  }

  singNotes.erase(std::remove_if(singNotes.begin(), singNotes.end(), [](const FallingNote &note) {
    return !note.active || note.y > M5Cardputer.Display.height();
  }), singNotes.end());

  if (singNotesCollected >= singTargetNotes) {
    singGameCompleted = true;
    singCompletionTime = now;
    if (natsumi.performance < 4) {
      natsumi.performance += 1;
    }
    drawTrainSingPlayfield(true);
    return;
  }

  drawTrainSingPlayfield(false);
}

void manageTrainDanceGame() {
  if (!danceGameRunning && !danceGameCompleted) {
    startTrainDanceGame();
  }

  unsigned long now = millis();

  if (danceGameCompleted) {
    if (danceNeedsRedraw) {
      drawTrainDancePlayfield();
      danceNeedsRedraw = false;
    }
    if (now - danceCompletionTime >= 1200) {
      changeState(0, TRAIN_DANCE3, 0);
    }
    return;
  }

  if (danceCueActive && now - danceCueStart >= danceCueDuration) {
    danceCueActive = false;
    danceNextCueTime = now + danceCueGap;
    danceNeedsRedraw = true;
  }

  if (!danceCueActive && now >= danceNextCueTime && danceGameRunning) {
    spawnDanceCue();
  }

  if (danceCueActive && M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (!keyList.empty()) {
      uint8_t key = M5Cardputer.Keyboard.getKey(keyList[0]);
      int inputDirection = -1;
      switch (key) {
        case 59: case 'w': case 'W':  // UP
          inputDirection = 0;
          break;
        case 46: case 's': case 'S':  // DOWN
          inputDirection = 1;
          break;
        case 44: case 'a': case 'A':   // LEFT
          inputDirection = 2;
          break;
        case 47: case 'd': case 'D':   // RIGHT
          inputDirection = 3;
          break;
        default:
          break;
      }
      if (inputDirection == danceCurrentDirection) {
        danceScore++;
        danceCueActive = false;
        danceNextCueTime = now + danceCueGap;
        danceNeedsRedraw = true;
      }
    }
  }

  if (danceScore >= danceTargetScore && !danceGameCompleted) {
    danceGameCompleted = true;
    danceCompletionTime = now;
    if (natsumi.performance < 4) {
      natsumi.performance += 1;
    }
    danceNeedsRedraw = true;
  }

  if (danceNeedsRedraw) {
    drawTrainDancePlayfield();
    danceNeedsRedraw = false;
  }
}

int getSwimLaneCenter(int lane) {
  return swimPoolTop + (lane * swimLaneHeight) + (swimLaneHeight / 2);
}

float getRandomSwimSpeed() {
  return random(static_cast<long>(swimMinSpeed * 100), static_cast<long>(swimMaxSpeed * 100)) / 100.0f;
}

void resetTrainSwimGame() {
  swimSharks.clear();
  swimPlayerLane = 1;
  swimAvoidedSharks = 0;
  swimCollisions = 0;
  swimGameRunning = false;
  swimGameCompleted = false;
  swimNeedsRedraw = true;
  swimHitFlash = false;
  swimHitFlashTime = 0;
  swimCompletionTime = 0;
  swimLastSpawnTime = 0;
  swimNextSpawnDelay = swimSpawnIntervalMin;
}

void spawnSwimShark() {
  SwimShark shark;
  shark.lane = static_cast<int>(random(0, swimLaneCount));
  shark.x = -swimSharkLength;
  shark.speed = 10;
  shark.active = true;
  swimSharks.push_back(shark);
}

void startTrainSwimGame() {
  resetTrainSwimGame();
  overlayActive = false;
  swimGameRunning = true;
  swimNeedsRedraw = true;
  M5Cardputer.Display.fillScreen(M5Cardputer.Display.color565(150, 220, 255));
}

void drawTrainSwimPlayfield(bool showCompletion, bool showHitEffect) {
  const int screenWidth = M5Cardputer.Display.width();
  const int screenHeight = M5Cardputer.Display.height();
  const uint16_t poolColor = M5Cardputer.Display.color565(150, 220, 255);
  const uint16_t laneColor = M5Cardputer.Display.color565(180, 235, 255);
  const uint16_t laneDividerColor = WHITE;
  const uint16_t sharkColor = M5Cardputer.Display.color565(255, 110, 110);
  const uint16_t sharkBelly = M5Cardputer.Display.color565(255, 210, 210);
  const uint16_t playerColor = M5Cardputer.Display.color565(70, 140, 255);
  const uint16_t textColor = BLACK;

  for (const auto &shark : swimSharks) {
    if (!shark.active) continue;
    int sharkX = static_cast<int>(shark.x);
    int sharkY = getSwimLaneCenter(shark.lane);
    // M5Cardputer.Display.fillRect((sharkX - shark.speed), sharkY, (sharkX + swimSharkLength), (sharkY + swimSharkHeight), poolColor);
    M5Cardputer.Display.fillRect((sharkX - shark.speed), sharkY, (sharkX + swimSharkLength), (sharkY + swimSharkHeight), playerColor);
    M5Cardputer.Display.drawPng(enemySprite.data, enemySprite.length, sharkX, sharkY);
  }

  int playerX = screenWidth - 32;
  int playerY = getSwimLaneCenter(swimPlayerLane);

  // M5Cardputer.Display.fillRect(playerX, 20, playerX + 22, 130, poolColor);
  M5Cardputer.Display.fillRect(playerX, 20, playerX + 22, 130, playerColor);
  M5Cardputer.Display.drawPng(natsumiSprite.data, natsumiSprite.length, playerX, playerY);

  M5Cardputer.Display.setTextDatum(top_left);
  M5Cardputer.Display.setTextColor(textColor, poolColor);
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.drawString(String("Dodged: ") + swimAvoidedSharks + String("/") + swimTargetSharks + " ", 6, 4);

  if (showCompletion) {
    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.drawString("Training complete!", screenWidth / 2, screenHeight / 2);
  }
}

void handleSwimCollision() {
  swimCollisions++;
  swimAvoidedSharks = max(0, swimAvoidedSharks - swimHitPenalty);
}

void manageTrainSwimGame() {
  if (!swimGameRunning && !swimGameCompleted) {
    startTrainSwimGame();
  }

  unsigned long now = millis();

  if (swimGameCompleted) {
    if (swimNeedsRedraw) {
      drawTrainSwimPlayfield(true, false);
      swimNeedsRedraw = false;
    }
    if (now - swimCompletionTime >= swimCompletionDelay) {
      if (natsumi.fitness < 4) {
        natsumi.fitness += 1;
      }
      changeState(0, TRAIN_SWIM3, 0);
    }
    return;
  }

  bool showHitEffect = swimHitFlash && (now - swimHitFlashTime < swimHitFlashDuration);
  if (swimHitFlash && !showHitEffect) {
    swimHitFlash = false;
    swimNeedsRedraw = true;
  }

  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      uint8_t key = M5Cardputer.Keyboard.getKey(keyList[0]);
      if ((key == 59 || key == 'w' || key == 'W') && swimPlayerLane > 0) { // UP
        swimPlayerLane--;
        swimNeedsRedraw = true;
      } else if ((key == 46 || key == 's' || key == 'S') && swimPlayerLane < swimLaneCount - 1) { // DOWN
        swimPlayerLane++;
        swimNeedsRedraw = true;
      }
    }
  }

  if (now - swimLastSpawnTime >= swimNextSpawnDelay && swimSharks.size() < static_cast<size_t>(swimMaxSharks)) {
    spawnSwimShark();
    swimLastSpawnTime = now;
    swimNextSpawnDelay = random(swimSpawnIntervalMin, swimSpawnIntervalMax);
    swimNeedsRedraw = true;
  }

  bool collision = false;
  int playerX = M5Cardputer.Display.width() - 32;
  int playerLeft = playerX - (swimPlayerWidth / 2);
  int playerRight = playerX + (swimPlayerWidth / 2);

  for (auto &shark : swimSharks) {
    if (!shark.active) continue;
    shark.x += shark.speed;
    // shark.x += (shark.speed + swimSharkLength);
    int sharkLeft = static_cast<int>(shark.x);
    int sharkRight = sharkLeft + swimSharkLength;

    if (shark.lane == swimPlayerLane && sharkRight > playerLeft && sharkLeft < playerRight) {
      collision = true;
      shark.active = false;
    } else if (shark.x > M5Cardputer.Display.width() + swimSharkLength) {
      shark.active = false;
      swimAvoidedSharks++;
      swimNeedsRedraw = true;
    }
  }

  if (collision) {
    handleSwimCollision();
  }

  swimSharks.erase(std::remove_if(swimSharks.begin(), swimSharks.end(), [](const SwimShark &s) {
    return !s.active;
  }), swimSharks.end());

  if (swimAvoidedSharks >= swimTargetSharks && !swimGameCompleted) {
    swimGameCompleted = true;
    swimCompletionTime = now;
    swimSharks.clear();
    swimGameRunning = false;
    if (natsumi.fitness < 4) {
      natsumi.fitness += 1;
    }
    swimNeedsRedraw = true;
  }

  if (swimGameRunning) {
    swimNeedsRedraw = true;
  }

  if (swimNeedsRedraw) {
    drawTrainSwimPlayfield(false, showHitEffect);
    swimNeedsRedraw = false;
  }
}

char getRandomGymLetter() {
  return static_cast<char>('A' + random(0, 26));
}

void refreshGymChallenge() {
  gymZoneWidth = random(gymZoneMinWidth, gymZoneMaxWidth + 1);
  int maxOffset = gymBarWidth - gymZoneWidth;
  gymZoneStart = gymBarX + random(0, maxOffset + 1);
  gymTargetLetter = getRandomGymLetter();
}

void resetTrainGymGame() {
  gymCurrentStreak = 0;
  gymCursorPos = gymBarX;
  gymCursorDirection = 1;
  gymCursorSpeed = gymCursorBaseSpeed;
  gymGameRunning = false;
  gymGameCompleted = false;
  gymLastUpdate = 0;
  gymResultFlashUntil = 0;
  gymResultText = "";
  gymCompletionTime = 0;
  refreshGymChallenge();
  gymNeedsRedraw = true;
  gymMisses = 0;
}

void startTrainGymGame() {
  resetTrainGymGame();
  overlayActive = false;
  gymGameRunning = true;
  gymLastUpdate = millis();
  M5Cardputer.Display.fillScreen(BLACK);
}

void drawTrainGymPlayfield(bool showCompletion) {
  const int screenWidth = M5Cardputer.Display.width();
  const int screenHeight = M5Cardputer.Display.height();
  const uint16_t barColor = M5Cardputer.Display.color565(40, 60, 90);
  const uint16_t zoneColor = M5Cardputer.Display.color565(120, 210, 130);
  const uint16_t cursorColor = M5Cardputer.Display.color565(255, 190, 90);
  const uint16_t frameColor = WHITE;

  // M5Cardputer.Display.fillScreen(BLACK);

  M5Cardputer.Display.setTextDatum(middle_left);
  M5Cardputer.Display.setTextColor(WHITE, BLACK);
  M5Cardputer.Display.setTextSize(3);
  M5Cardputer.Display.drawString(String(gymTargetLetter), 14, gymBarY + (gymBarHeight / 2));

  M5Cardputer.Display.fillRect(gymBarX, gymBarY, gymBarWidth, gymBarHeight, barColor);
  M5Cardputer.Display.drawRect(gymBarX - 1, gymBarY - 1, gymBarWidth + 2, gymBarHeight + 2, frameColor);
  M5Cardputer.Display.fillRect(gymZoneStart, gymBarY, gymZoneWidth, gymBarHeight, zoneColor);

  int cursorX = static_cast<int>(gymCursorPos);
  M5Cardputer.Display.fillRect(cursorX - 3, gymBarY - 6, 6, gymBarHeight + 12, cursorColor);

  M5Cardputer.Display.setTextDatum(top_left);
  M5Cardputer.Display.setTextColor(WHITE, BLACK);
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.drawString(String("Streak: ") + gymCurrentStreak + String("/") + gymTargetStreak, 6, 4);

  unsigned long now = millis();
  if (!showCompletion && gymResultFlashUntil > now && gymResultText.length() > 0) {
    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.drawString(gymResultText, screenWidth / 2, gymBarY - 14);
  }

  if (showCompletion) {
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.drawString("Training complete!", screenWidth / 2, screenHeight / 2);
  }
}

void manageTrainGymGame() {
  if (!gymGameRunning && !gymGameCompleted) {
    startTrainGymGame();
  }

  unsigned long now = millis();

  if (gymGameCompleted) {
    if (gymNeedsRedraw) {
      drawTrainGymPlayfield(true);
      gymNeedsRedraw = false;
    }
    if (now - gymCompletionTime >= gymCompletionDelay) {
      if (natsumi.fitness < 4) {
        natsumi.fitness += 1;
      }
      changeState(0, TRAIN_GYM3, 0);
    }
    return;
  }

  if (gymGameRunning) {
    unsigned long delta = (gymLastUpdate == 0) ? 0 : (now - gymLastUpdate);
    if (delta > 0) {
      gymCursorPos += gymCursorSpeed * static_cast<float>(delta) * gymCursorDirection;
      int minPos = gymBarX;
      int maxPos = gymBarX + gymBarWidth;
      if (gymCursorPos <= minPos) {
        gymCursorPos = minPos;
        gymCursorDirection = 1;
      } else if (gymCursorPos >= maxPos) {
        gymCursorPos = maxPos;
        gymCursorDirection = -1;
      }
      gymNeedsRedraw = true;
    }
    gymLastUpdate = now;
  }

  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (!keyList.empty()) {
      uint8_t key = M5Cardputer.Keyboard.getKey(keyList[0]);
      char pressed = static_cast<char>(key);
      bool isLetter = (pressed >= 'a' && pressed <= 'z') || (pressed >= 'A' && pressed <= 'Z');
      if (isLetter) {
        char normalized = (pressed >= 'a' && pressed <= 'z') ? (pressed - 32) : pressed;
        bool correctLetter = normalized == gymTargetLetter;
        bool inZone = gymCursorPos >= gymZoneStart && gymCursorPos <= gymZoneStart + gymZoneWidth;
        if (correctLetter && inZone) {
          gymCurrentStreak++;
          gymCursorSpeed += gymCursorSpeedStep;
          gymResultText = " Great timing! ";
          gymResultFlashUntil = now + gymResultFlashDuration;
          if (gymCurrentStreak >= gymTargetStreak) {
            gymGameCompleted = true;
            gymGameRunning = false;
            gymCompletionTime = now;
            gymNeedsRedraw = true;
            return;
          }
          refreshGymChallenge();
          gymCursorDirection = -gymCursorDirection;
          gymLastUpdate = now;
          gymNeedsRedraw = true;
        } else {
          gymCurrentStreak = 0;
          gymCursorSpeed = gymCursorBaseSpeed;
          gymResultText = " Miss! Try again ";
          gymResultFlashUntil = now + gymResultFlashDuration;
          refreshGymChallenge();
          gymCursorDirection = 1;
          gymCursorPos = gymBarX;
          gymLastUpdate = now;
          gymNeedsRedraw = true;
          gymMisses += 1;
        }
      }
    }
  }

  if (gymNeedsRedraw) {
    drawTrainGymPlayfield(false);
    gymNeedsRedraw = false;
  }
}

int getRunBarX() {
  return (M5Cardputer.Display.width() - runBarWidth) / 2;
}

void resetTrainRunGame() {
  runCursorPos = 0.0f;
  runEnterHeld = false;
  runGreenTime = 0;
  runRedTime = 0;
  runLastUpdate = 0;
  runCompletionTime = 0;
  runLastStepTime = 0;
  runStepIndex = 1;
  runGameRunning = false;
  runGameCompleted = false;
  runGameFailed = false;
  runNeedsRedraw = true;
}

void startTrainRunGame() {
  resetTrainRunGame();
  overlayActive = false;
  runGameRunning = true;
  int barX = getRunBarX();
  runCursorPos = barX + (runBarWidth / 2);
  runLastUpdate = millis();
  runLastStepTime = runLastUpdate;
  M5Cardputer.Display.fillScreen(BLACK);
}

void drawTrainRunPlayfield(bool showCompletion, bool showFailure) {
  const int screenWidth = M5Cardputer.Display.width();
  const int screenHeight = M5Cardputer.Display.height();
  const uint16_t barOutline = WHITE;
  const uint16_t redZoneColor = M5Cardputer.Display.color565(220, 80, 80);
  const uint16_t greenZoneColor = M5Cardputer.Display.color565(80, 200, 120);
  const uint16_t cursorColor = M5Cardputer.Display.color565(255, 210, 110);

  int barX = getRunBarX();
  int barY = runBarY;
  int greenStart = barX + (runBarWidth - runGreenWidth) / 2;
  int greenEnd = greenStart + runGreenWidth;

  int natsumiX = 6;
  int natsumiY = max(0, (screenHeight - 90) / 2);
  if (runStepIndex % 2 == 1) {
    M5Cardputer.Display.drawPng(natsumiSprite.data, natsumiSprite.length, natsumiX, natsumiY);
  } else {
    M5Cardputer.Display.drawPng(enemySprite.data, enemySprite.length, natsumiX, natsumiY);
  }

  M5Cardputer.Display.fillRect(barX, barY, runBarWidth, runBarHeight, redZoneColor);
  M5Cardputer.Display.fillRect(greenStart, barY, runGreenWidth, runBarHeight, greenZoneColor);
  M5Cardputer.Display.drawRect(barX - 1, barY - 1, runBarWidth + 2, runBarHeight + 2, barOutline);
  int cursorX = static_cast<int>(runCursorPos);
  M5Cardputer.Display.fillRect(cursorX - 2, barY - 4, 4, runBarHeight + 8, cursorColor);

  M5Cardputer.Display.setTextDatum(top_left);
  M5Cardputer.Display.setTextColor(WHITE, BLACK);
  M5Cardputer.Display.setTextSize(2);
  int secondsLeft = max(0, static_cast<int>((runTargetGreenTime - runGreenTime) / 1000));
  M5Cardputer.Display.drawString(String("Time left: ") + String(secondsLeft) + String("s "), 6, 4);

  if (showCompletion || showFailure) {
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextSize(2);
    if (showCompletion) {
      M5Cardputer.Display.drawString("Training complete!", screenWidth / 2, screenHeight / 2);
    } else {
      M5Cardputer.Display.drawString("Training failed!", screenWidth / 2, screenHeight / 2);
    }
  }
}

void manageTrainRunGame() {
  if (!runGameRunning && !runGameCompleted && !runGameFailed) {
    startTrainRunGame();
  }

  unsigned long now = millis();

  if (runGameCompleted || runGameFailed) {
    if (runNeedsRedraw) {
      drawTrainRunPlayfield(runGameCompleted, runGameFailed);
      runNeedsRedraw = false;
    }
    if (now - runCompletionTime >= runCompletionDelay) {
      if (natsumi.fitness < 4) {
        natsumi.fitness += 1;
      }
      changeState(0, TRAIN_RUN3, 0);
    }
    return;
  }

  bool enterHeld = false;
  if (M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    for (size_t i = 0; i < keyList.size(); i++) {
      uint8_t key = M5Cardputer.Keyboard.getKey(keyList[i]);
      if (key == 13 || key == 40) {
        enterHeld = true;
        break;
      }
    }
  }
  runEnterHeld = enterHeld;

  unsigned long delta = (runLastUpdate == 0) ? 0 : (now - runLastUpdate);
  if (delta > 0 && runGameRunning) {
    float direction = runEnterHeld ? 1.0f : -1.0f;
    runCursorPos += runCursorSpeed * static_cast<float>(delta) * direction;
    int barX = getRunBarX();
    int minPos = barX;
    int maxPos = barX + runBarWidth;
    if (runCursorPos < minPos) {
      runCursorPos = minPos;
    } else if (runCursorPos > maxPos) {
      runCursorPos = maxPos;
    }

    int greenStart = barX + (runBarWidth - runGreenWidth) / 2;
    int greenEnd = greenStart + runGreenWidth;
    bool inGreen = runCursorPos >= greenStart && runCursorPos <= greenEnd;
    if (inGreen) {
      runGreenTime += delta;
      runRedTime = 0;
    } else {
      runGreenTime = 0;
      runRedTime += delta;
    }

    if (runGreenTime >= runTargetGreenTime) {
      runGameCompleted = true;
      runGameRunning = false;
      runCompletionTime = now;
      runNeedsRedraw = true;
      if (natsumi.fitness < 4) {
        natsumi.fitness += 1;
      }
      return;
    }

    if (runRedTime >= runMaxRedTime) {
      runGameFailed = true;
      runGameRunning = false;
      runCompletionTime = now;
      runNeedsRedraw = true;
      return;
    }
    runNeedsRedraw = true;
  }

  if (now - runLastStepTime >= runStepInterval) {
    runStepIndex++;
    runLastStepTime = now;
    runNeedsRedraw = true;
  }

  runLastUpdate = now;

  if (runNeedsRedraw) {
    drawTrainRunPlayfield(false, false);
    runNeedsRedraw = false;
  }
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
  drawText("Shower temperature is " + outcomeText, 120, 131, true, WHITE, 1);
  showToast("Shower is " + outcomeText);

  if (outcomeText == "perfect!") {
    if (natsumi.hygiene < 4) {
      natsumi.hygiene = 4;
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
      finalizeBathOutcome("too hot!");
    } else if (sliderCenter > zoneBottom) {
      finalizeBathOutcome("too cold!");
    } else {
      finalizeBathOutcome("perfect!");
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
      finalizeBathOutcome("too hot!");
    } else if (sliderCenter > zoneBottom) {
      finalizeBathOutcome("too cold!");
    } else {
      finalizeBathOutcome("perfect!");
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

void drawLibraryProgressBar() {
  const int barY = 121;
  const int barH = 10;
  const int barX = 6;
  const int barW = 228;
  const int gap = 1;
  const int segmentCount = librarySegmentCount;
  const uint16_t fillColor = M5Cardputer.Display.color565(120, 170, 255);
  const uint16_t emptyColor = M5Cardputer.Display.color565(28, 36, 48);

  int availableWidth = barW - (segmentCount - 1) * gap;
  int segmentWidth = availableWidth / segmentCount;
  int extraPixels = availableWidth % segmentCount;

  M5Cardputer.Display.fillRect(0, barY - 2, 240, barH + 4, BLACK);
  int currentX = barX;
  for (int i = 0; i < segmentCount; ++i) {
    int width = segmentWidth + (i < extraPixels ? 1 : 0);
    uint16_t color = (i < librarySegmentsFilled) ? fillColor : emptyColor;
    M5Cardputer.Display.fillRect(currentX, barY, width, barH, color);
    currentX += width + gap;
  }
}

void manageLibrary() {
  overlayActive = true;

  if (!libraryInitialized) {
    libraryInitialized = true;
    libraryRewardApplied = false;
    libraryStartTime = millis();
    librarySegmentsFilled = 0;
    l5NeedsRedraw = true;
  }

  unsigned long now = millis();
  unsigned long elapsed = (now >= libraryStartTime) ? (now - libraryStartTime) : 0;
  int segments = elapsed / librarySegmentInterval;
  if (segments > librarySegmentCount) {
    segments = librarySegmentCount;
  }

  if (segments != librarySegmentsFilled) {
    librarySegmentsFilled = segments;
    l5NeedsRedraw = true;
  }

  if (librarySegmentsFilled >= librarySegmentCount) {
    if (!libraryRewardApplied) {
      if (natsumi.culture < STAT_MAX) {
        natsumi.culture += 1;
      }
      libraryRewardApplied = true;
      overlayActive = false;
      changeState(0, HOME_LOOP, 0);
    }
  }
  return;
}

void manageFlowersMarket() {
  static bool flowerMarketInitialized = false;
  static int flowerMarketSelection = 1;
  static bool flowerMarketNeedsRedraw = true;

  overlayActive = false;

  if (!flowerMarketInitialized) {
    flowerMarketSelection = 1;
    flowerMarketNeedsRedraw = true;
    flowerMarketInitialized = true;
  }

  int handicapScore = natsumi.spirit + natsumi.charm + natsumi.culture;
  int prices[3];
  if (handicapScore <= 3) {
    prices[0] = 200;
    prices[1] = 250;
    prices[2] = 300;
  } else if (handicapScore <= 8) {
    prices[0] = 350;
    prices[1] = 400;
    prices[2] = 450;
  } else if (handicapScore <= 11) {
    prices[0] = 500;
    prices[1] = 550;
    prices[2] = 600;
  } else {
    prices[0] = 650;
    prices[1] = 700;
    prices[2] = 800;
  }

  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (!keyList.empty()) {
      uint8_t key = M5Cardputer.Keyboard.getKey(keyList[0]);
      switch (key) {
        // LEFT
        case 180: case 44: case 'a': case 'A':
          if (flowerMarketSelection > 0) {
            flowerMarketSelection -= 1;
            flowerMarketNeedsRedraw = true;
          }
          break;
        // RIGHT
        case 183: case 47: case 'd': case 'D':
          if (flowerMarketSelection < 2) {
            flowerMarketSelection += 1;
            flowerMarketNeedsRedraw = true;
          }
          break;
        // ESC
        case 96: case 43:
          flowerMarketInitialized = false;
          overlayActive = false;
          changeState(0, HOME_LOOP, 0);
          return;
        // ENTER
        case 13: case 40: case ' ':
          flowersPrice = prices[flowerMarketSelection];
          flowerMarketInitialized = false;
          overlayActive = false;
          flowersSaleInProgress = true;
          changeState(0, FLOWERS_MARKET6, 0);
          return;
      }
    }
  }

  if (flowerMarketNeedsRedraw) {
    const int screenWidth = M5Cardputer.Display.width();
    const int screenHeight = M5Cardputer.Display.height();
    const int buttonCount = 3;
    const int buttonW = 60;
    const int buttonH = 26;
    const int buttonGap = 12;
    const int totalW = (buttonW * buttonCount) + (buttonGap * (buttonCount - 1));
    const int startX = (screenWidth - totalW) / 2;
    const int startY = screenHeight - buttonH - 24;

    drawImage(currentBackground);

    const uint16_t buttonFill = M5Cardputer.Display.color565(18, 26, 48);
    const uint16_t buttonBorder = M5Cardputer.Display.color565(120, 170, 255);
    const uint16_t buttonActive = M5Cardputer.Display.color565(255, 200, 40);
    const uint16_t buttonActiveFill = M5Cardputer.Display.color565(60, 48, 12);

    for (int i = 0; i < buttonCount; ++i) {
      int x = startX + i * (buttonW + buttonGap);
      uint16_t fillColor = (i == flowerMarketSelection) ? buttonActiveFill : buttonFill;
      uint16_t borderColor = (i == flowerMarketSelection) ? buttonActive : buttonBorder;
      uint16_t textColor = (i == flowerMarketSelection) ? buttonActive : WHITE;

      M5Cardputer.Display.fillRoundRect(x, startY, buttonW, buttonH, 6, fillColor);
      M5Cardputer.Display.drawRoundRect(x, startY, buttonW, buttonH, 6, borderColor);
      drawText("$" + String(prices[i]), x + (buttonW / 2), startY + 9, true, textColor, 1);
    }

    M5Cardputer.Display.fillRect(0, 125, 240, 10, BLACK);
    drawText("LEFT/RIGHT: Price  ENTER: Sell  ESC: Home", 120, 131, true, WHITE, 1);
    flowerMarketNeedsRedraw = false;
  }
}

void manageFlowersSale() {
  static bool flowersSaleInitialized = false;
  static std::vector<int> flowerSlots;
  static std::vector<std::pair<int, int>> flowerPositions;
  static bool flowerSaleNeedsRedraw = true;

  const int spriteScale = 2;
  const int spriteW = 10 * spriteScale;
  const int spriteH = 16 * spriteScale;
  const int padding = 4;
  const int initialX = 20;
  const int initialY = 10;
  const int screenWidth = M5Cardputer.Display.width();
  const int screenHeight = M5Cardputer.Display.height();

  if (!flowersSaleInitialized) {
    flowersSaleInitialized = true;
    flowerSaleNeedsRedraw = true;
    flowerSlots.clear();
    flowerPositions.clear();

    // int columns = (screenWidth - padding) / (spriteW + padding);
    int columns = 8;
    if (columns < 1) {
      columns = 1;
    }

    for (int i = 0; i < natsumi.flowers; ++i) {
      int col = i % columns;
      int row = i / columns;
      int x = initialX + padding + col * (spriteW + padding);
      int y = initialY + padding + row * (spriteH + padding);
      // int x = padding + col * (spriteW + padding);
      // int y = padding + row * (spriteH + padding);
      flowerPositions.push_back({x, y});
      flowerSlots.push_back(i);
    }
  }

  if (fiveSecondPulse && !flowerSlots.empty()) {
    bool sold = (random(0, 2) == 0);
    if (sold) {
      int soldIndex = random(0, flowerSlots.size());
      flowerSlots.erase(flowerSlots.begin() + soldIndex);
      if (natsumi.flowers > 0) {
        natsumi.flowers -= 1;
      }
      flowersRevenue += flowersPrice;
      flowerSaleNeedsRedraw = true;
    }
  }

  if (natsumi.flowers <= 0 || flowerSlots.empty()) {
    flowersSaleInProgress = false;
    flowersSaleInitialized = false;
    changeState(0, FLOWERS_MARKET7, 0);
    return;
  }

  if (flowerSaleNeedsRedraw) {
    drawImage(currentBackground);
    for (int index : flowerSlots) {
      if (index < static_cast<int>(flowerPositions.size())) {
        auto position = flowerPositions[index];
        M5Cardputer.Display.drawPng(
          natsumiSprite.data,
          natsumiSprite.length,
          position.first,
          position.second,
          0,
          0,
          0,
          0,
          spriteScale
        );
      }
    }
    flowerSaleNeedsRedraw = false;
  }
}

void wash() {
  if (changeStateCounter==0) {
    if (natsumi.hygiene < 4) {
      showToast("Washed (+1 Hygiene)");
    } else {
      showToast("Natsumi is clean");
    }
  }
  changeState(0, HOME_LOOP, 20);
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
    // Serial.println(">> drawMeditationOverlay: End of meditation session");
    meditationActive = false;
    if (!meditationRewardApplied) {
      if (natsumi.spirit < 4 ) {
        natsumi.spirit += 1;
        // Serial.println(">> drawMeditationOverlay: natsumi.spirit=" + String(natsumi.spirit));
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
  // Serial.println("> Entering drawBackground() L0 with backgroundEnabled set to " + String(backgroundEnabled));
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
  // Serial.println("> Entering drawCharacter() L1 with characterEnabled set to " + String(characterEnabled));
  if (l1NeedsRedraw) {
    if (characterEnabled) {
      drawImage(currentCharacter);
      if (!menuOpened && !overlayActive && menuEnabled) {
        // Helper text at the bottom
        // Serial.println("[DEBUG] manageHomeScreen() -> l5NeedsRedraw TRUE");
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
  // Serial.println("> Entering drawDebug() L2 with debugEnabled set to " + String(debugEnabled));
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
  // Serial.println("> Entering drawToast() L3 with toastEnabled set to " + String(toastEnabled));
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
  // Serial.println("> Entering drawMenu() L4 with menuEnabled set to " + String(menuEnabled));
  // Serial.println(">> menuType set to " + String(menuType));
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
        case 51:
          // 3: CONBINI
          menuOpened = false;
          changeState(0, FOOD_CONBINI, 0);
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
            changeState(0, FOOD_CONBINI, 0);
          } else if (selection == 4) {
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
          // 4: RUN
          menuOpened = false;
          changeState(0, TRAIN_RUN, 0);
          break;
        case 53:
          // 5: LIBRARY
          menuOpened = false;
          changeState(0, TRAIN_LIBRARY, 0);
          break;
        case 54:
          // 6: MARKET
          menuOpened = false;
          changeState(0, FLOWERS_MARKET, 0);
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
            changeState(0, TRAIN_RUN, 0);
          } else if (selection == 5) {
            changeState(0, TRAIN_LIBRARY, 0);
          } else if (selection == 6) {
            changeState(0, FLOWERS_MARKET, 0);
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
    } else if (menuType == "garden") {
      Serial.println(">> drawMenu - garden menuType");
      switch (key) {
        case 48:
          // 0: PLANT
          Serial.println(">>> drawMenu - 0: PLANT");
          menuOpened = false;
          changeState(0, GARDEN_PLANT, 0);
          break;
        case 49:
          // 1: WATER
          Serial.println(">>> drawMenu - 1: WATER");
          menuOpened = false;
          changeState(0, GARDEN_WATER, 0);
          break;
        case 50:
          // 2: PICK
          Serial.println(">>> drawMenu - 2: PICK");
          menuOpened = false;
          changeState(0, GARDEN_PICK, 0);
          break;
        case 51:
          // 3: CLEANUP
          Serial.println(">>> drawMenu - 3: CLEANUP");
          menuOpened = false;
          changeState(0, GARDEN_CLEANUP, 0);
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
            changeState(0, GARDEN_LOOP, 0);
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
          changeState(0, GARDEN_LOOP, 0);
          break;
        case 181: case 'w': case 'W': case 59:
          // UP
          Serial.println(">>> drawMenu - UP");
          selection = (selection - 1 + gardenMenuItemCount) % gardenMenuItemCount;
          l4NeedsRedraw = true;
          break;
        case 182: case 's': case 'S': case 46:
          // DOWN
          Serial.println(">>> drawMenu - DOWN");
          selection = (selection + 1) % gardenMenuItemCount;
          l4NeedsRedraw = true;
          break;
        case 13: case 40: case ' ':
          // VALIDATE
          if (selection == 0) {
            changeState(0, GARDEN_PLANT, 0);
          } else if (selection == 1) {
            changeState(0, GARDEN_WATER, 0);
          } else if (selection == 2) {
            changeState(0, GARDEN_PICK, 0);
          } else if (selection == 3) {
            changeState(0, GARDEN_CLEANUP, 0);
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

bool preloadFoodIcon(FoodDisplayItem &item) {
  if (item.icon.data) {
    unloadImage(item.icon);
  }

  if (preloadImage(item.iconPath, item.icon)) {
    return true;
  }

  String altPath = String("/idolnat/sprites/food/") + String(strrchr(item.iconPath, '/') ? strrchr(item.iconPath, '/') + 1 : item.iconPath);
  return preloadImage(altPath.c_str(), item.icon);
}

void clearFoodGrid() {
  for (auto &item : foodGridItems) {
    unloadImage(item.icon);
  }
  foodGridItems.clear();
  foodGridInitialized = false;
}

void prepareFoodGrid() {
  clearFoodGrid();

  std::vector<FoodDisplayItem> options = {
    {"Red apple", "/idolnat/sprites/food_001.png", &fridge.redApple},
    {"Green apple", "/idolnat/sprites/food_002.png", &fridge.greenApple},
    {"Avocado", "/idolnat/sprites/food_003.png", &fridge.avocado},
    {"Bread", "/idolnat/sprites/food_005.png", &fridge.bread},
    {"Banana", "/idolnat/sprites/food_008.png", &fridge.banana},
    {"Broccoli", "/idolnat/sprites/food_015.png", &fridge.broccoli},
    {"Sweets", "/idolnat/sprites/food_021.png", &fridge.sweets},
    {"Carrot", "/idolnat/sprites/food_028.png", &fridge.carrot},
    {"Meat", "/idolnat/sprites/food_033.png", &fridge.meat},
    {"Coconut", "/idolnat/sprites/food_038.png", &fridge.coconut},
    {"Coconut juice", "/idolnat/sprites/food_039.png", &fridge.coconutJuice},
    {"Coffee", "/idolnat/sprites/food_041.png", &fridge.coffee},
    {"Biscuit", "/idolnat/sprites/food_044.png", &fridge.biscuits},
    {"Corn", "/idolnat/sprites/food_045.png", &fridge.corn},
    {"Croissant", "/idolnat/sprites/food_046.png", &fridge.croissant},
    {"Fried egg", "/idolnat/sprites/food_053.png", &fridge.friedEgg},
    {"Grape", "/idolnat/sprites/food_061.png", &fridge.grapes},
    {"Kiwi", "/idolnat/sprites/food_081.png", &fridge.kiwi},
    {"Milk", "/idolnat/sprites/food_092.png", &fridge.milk},
    {"Orange", "/idolnat/sprites/food_109.png", &fridge.orange},
    {"Peach", "/idolnat/sprites/food_111.png", &fridge.peach},
    {"Pear", "/idolnat/sprites/food_113.png", &fridge.pear},
    {"Strawberries", "/idolnat/sprites/food_149.png", &fridge.strawberries},
    {"Maki", "/idolnat/sprites/food_150.png", &fridge.maki},
    {"Sushi", "/idolnat/sprites/food_154.png", &fridge.sushi},
    {"Watermelon", "/idolnat/sprites/food_168.png", &fridge.watermelon}
  };

  for (auto &option : options) {
    option.quantity = *(option.quantityPtr);
  }

  std::sort(options.begin(), options.end(), [](const FoodDisplayItem &a, const FoodDisplayItem &b) {
    return a.quantity > b.quantity;
  });

  size_t limit = std::min<size_t>(8, options.size());
  for (size_t i = 0; i < limit; i++) {
    foodGridItems.push_back(options[i]);
    preloadFoodIcon(foodGridItems.back());
  }

  foodSelectionIndex = 0;
  overlayActive = true;
  l5NeedsRedraw = true;
  foodGridInitialized = true;
}

void drawFoodGrid(const std::vector<FoodDisplayItem> &items, int selectedIndex) {
  const int panelX = 6;
  const int panelY = 6;
  const int panelW = 228;
  const int panelH = 123;
  const int headerHeight = 18;
  const uint16_t shadowColor = M5Cardputer.Display.color565(10, 14, 32);
  const uint16_t panelColor = M5Cardputer.Display.color565(16, 24, 44);
  const uint16_t accentColor = M5Cardputer.Display.color565(120, 200, 255);
  const uint16_t cellColor = M5Cardputer.Display.color565(28, 40, 64);
  const uint16_t highlightColor = M5Cardputer.Display.color565(60, 90, 140);

  M5Cardputer.Display.fillRoundRect(panelX + 2, panelY + 2, panelW, panelH, 10, shadowColor);
  M5Cardputer.Display.fillRoundRect(panelX, panelY, panelW, panelH, 10, panelColor);
  M5Cardputer.Display.drawRoundRect(panelX, panelY, panelW, panelH, 10, accentColor);

  M5Cardputer.Display.setTextDatum(middle_center);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setTextColor(WHITE, panelColor);
  M5Cardputer.Display.drawString("What\'s in the fridge?", panelX + panelW / 2, panelY + headerHeight / 2 + 1);

  const int cols = 4;
  const int rows = 2;
  const int padding = 8;
  const int cellW = (panelW - padding * (cols + 1)) / cols;
  const int cellH = (panelH - headerHeight - padding * (rows + 1)) / rows;

  for (size_t i = 0; i < items.size(); i++) {
    int col = i % cols;
    int row = i / cols;
    int cellX = panelX + padding + col * (cellW + padding);
    int cellY = panelY + headerHeight + padding + row * (cellH + padding);
    bool selected = (static_cast<int>(i) == selectedIndex);
    uint16_t fill = selected ? highlightColor : cellColor;

    M5Cardputer.Display.fillRoundRect(cellX, cellY, cellW, cellH, 6, fill);
    M5Cardputer.Display.drawRoundRect(cellX, cellY, cellW, cellH, 6, accentColor);

    int iconOffset = (cellW - 32) / 2;
    if (items[i].icon.data && items[i].icon.length > 0) {
      M5Cardputer.Display.drawPng(items[i].icon.data, items[i].icon.length, cellX + iconOffset, cellY + 4);
    } else {
      M5Cardputer.Display.fillCircle(cellX + cellW / 2, cellY + 14, 10, accentColor);
    }

    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(WHITE, fill);
    M5Cardputer.Display.drawString(items[i].label, cellX + cellW / 2, cellY + cellH - 16);
    M5Cardputer.Display.drawString(String("x") + String(items[i].quantity), cellX + cellW / 2, cellY + cellH - 4);
  }

  M5Cardputer.Display.fillRect(0, 125, 240, 10, BLACK);
  drawText("Arrows: Move  ENTER: Eat  ESC: Close", 120, 131, true, WHITE, 1);
}

int getConbimartTotal() {
  int total = 0;
  for (const auto &item : conbimartItems) {
    total += item.price * item.quantity;
  }
  return total;
}

void drawConbimartOverlay() {
  const int panelX = 6;
  const int panelY = 6;
  const int panelW = 228;
  const int panelH = 123;
  const uint16_t shadowColor = M5Cardputer.Display.color565(10, 14, 32);
  const uint16_t panelColor = M5Cardputer.Display.color565(16, 24, 44);
  const uint16_t accentColor = M5Cardputer.Display.color565(120, 200, 255);
  const uint16_t highlightColor = M5Cardputer.Display.color565(60, 90, 140);

  M5Cardputer.Display.fillRoundRect(panelX + 2, panelY + 2, panelW, panelH, 10, shadowColor);
  M5Cardputer.Display.fillRoundRect(panelX, panelY, panelW, panelH, 10, panelColor);
  M5Cardputer.Display.drawRoundRect(panelX, panelY, panelW, panelH, 10, accentColor);

  M5Cardputer.Display.setTextDatum(middle_center);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setTextColor(WHITE, panelColor);
  M5Cardputer.Display.drawString("-= ConbiMart Specials =-", panelX + panelW / 2, panelY + 8);

  const int startY = panelY + 20;
  const int lineHeight = 12;

  M5Cardputer.Display.setTextDatum(top_left);
  for (size_t i = 0; i < conbimartItems.size(); i++) {
    const auto &item = conbimartItems[i];
    int rowY = startY + (i * lineHeight);
    bool selected = (static_cast<int>(i) == conbimartSelectionIndex);
    uint16_t rowBg = selected ? highlightColor : panelColor;

    M5Cardputer.Display.fillRect(panelX + 4, rowY - 1, panelW - 8, lineHeight, rowBg);
    M5Cardputer.Display.setTextColor(selected ? YELLOW : WHITE, rowBg);
    M5Cardputer.Display.setCursor(panelX + 8, rowY);
    M5Cardputer.Display.print(item.label);

    M5Cardputer.Display.setTextColor(M5Cardputer.Display.color565(180, 220, 255), rowBg);
    M5Cardputer.Display.setCursor(panelX + 136, rowY);
    // M5Cardputer.Display.print("\xC2\xA5");
    M5Cardputer.Display.print(item.price);

    M5Cardputer.Display.setTextColor(WHITE, rowBg);
    M5Cardputer.Display.setCursor(panelX + 190, rowY);
    M5Cardputer.Display.print("x");
    M5Cardputer.Display.print(item.quantity);
  }

  int total = getConbimartTotal();
  M5Cardputer.Display.fillRect(panelX + 6, panelY + panelH - 30, panelW - 12, 20, panelColor);
  M5Cardputer.Display.drawRoundRect(panelX + 6, panelY + panelH - 30, panelW - 12, 20, 4, accentColor);
  M5Cardputer.Display.setTextColor(WHITE, panelColor);
  M5Cardputer.Display.setCursor(panelX + 12, panelY + panelH - 26);
  M5Cardputer.Display.print("Cart: " + String(total));
  // M5Cardputer.Display.print(total);
  M5Cardputer.Display.setCursor(panelX + 120, panelY + panelH - 26);
  M5Cardputer.Display.print("Cash: " + String(natsumi.money));
  // M5Cardputer.Display.print();

  M5Cardputer.Display.fillRect(0, 125, 240, 10, BLACK);
  drawText("+/-: Qty  ENTER: Buy  ESC: Cancel", 120, 131, true, WHITE, 1);
  Serial.println(">> drawConbimartOverlay -  Cart: " + String(total));
  Serial.println(">> drawConbimartOverlay -  Cash: " + String(natsumi.money));
}

void prepareConbimartItems() {
  struct ConbimartCatalogEntry {
    const char* label;
    int price;
    int* stockPtr;
  };

  std::vector<ConbimartCatalogEntry> savoury = {
    {"Avocado", 220, &fridge.avocado},
    {"Bread", 180, &fridge.bread},
    {"Broccoli", 160, &fridge.broccoli},
    {"Carrot", 140, &fridge.carrot},
    {"Meat", 420, &fridge.meat},
    {"Corn", 150, &fridge.corn},
    {"Fried egg", 200, &fridge.friedEgg},
    {"Maki", 380, &fridge.maki},
    {"Sushi", 520, &fridge.sushi}
  };

  std::vector<ConbimartCatalogEntry> sugary = {
    {"Green apple", 160, &fridge.greenApple},
    {"Banana", 150, &fridge.banana},
    {"Sweets", 200, &fridge.sweets},
    {"Coconut", 260, &fridge.coconut},
    {"Biscuit", 180, &fridge.biscuits},
    {"Croissant", 240, &fridge.croissant},
    {"Grape", 210, &fridge.grapes},
    {"Kiwi", 190, &fridge.kiwi},
    {"Orange", 170, &fridge.orange},
    {"Peach", 200, &fridge.peach},
    {"Pear", 200, &fridge.pear},
    {"Strawberries", 260, &fridge.strawberries},
    {"Watermelon", 320, &fridge.watermelon}
  };

  std::vector<ConbimartCatalogEntry> drinks = {
    {"Coconut juice", 230, &fridge.coconutJuice},
    {"Coffee", 180, &fridge.coffee},
    {"Milk", 160, &fridge.milk}
  };

  auto addRandomItems = [&](std::vector<ConbimartCatalogEntry> &source, int count) {
    for (int i = 0; i < count && !source.empty(); i++) {
      int index = random(0, source.size());
      auto entry = source[index];
      conbimartItems.push_back({entry.label, entry.price, entry.stockPtr, 0});
      source.erase(source.begin() + index);
    }
  };

  conbimartItems.clear();
  addRandomItems(savoury, 3);
  addRandomItems(sugary, 2);
  addRandomItems(drinks, 1);

  conbimartSelectionIndex = 0;
  conbimartInitialized = true;
  overlayActive = true;
  l5NeedsRedraw = true;
}

void drawOverlay() {
  // Draw the overlay (L5)
  // Serial.println("> Entering drawOverlay() L5 with l5NeedsRedraw set to " + String(l5NeedsRedraw) + " and overlayActive set to " + String(overlayActive));
  if (overlayActive && overlayEnabled) {
    // Serial.println(">> drawOverlay: l5NeedsRedraw is TRUE");
    uint8_t key = 0;
    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
      auto keyList = M5Cardputer.Keyboard.keyList();
      Serial.println(">>> drawOverlay: Testing for key pressed");
      if (keyList.size() > 0) {
        key = M5Cardputer.Keyboard.getKey(keyList[0]);
        if (currentState != FOOD_COOK && currentState != FOOD_CONBINI3 &&
            currentState != GARDEN_LOOP && currentState != GARDEN_PLANT &&
            currentState != GARDEN_WATER && currentState != GARDEN_PICK &&
            currentState != GARDEN_CLEANUP && currentState != GARDEN_MENU) {
          overlayActive = false;
          changeState(0, HOME_LOOP, 0);
          return;
        }
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
      case TRAIN_DANCE: case TRAIN_SING: case TRAIN_SWIM: case TRAIN_GYM: case TRAIN_RUN:
        drawMiniGameCountdown();
        break;
      case TRAIN_LIBRARY:
        drawLibraryProgressBar();
        break;
      case TRAIN_DANCE3: {
        int missedDanceCues = danceCuesShown - danceScore;
        String danceTeacherFeedback = "";
        switch(missedDanceCues) {
          case 0:
            danceTeacherFeedback = "excellent!!";
            break;
          case 1: case 2: case 3:
            danceTeacherFeedback = "very good!!";
            break;
          case 4: case 5:
            danceTeacherFeedback = "good enough.";
            break;
          case 6: case 7: case 8:
            danceTeacherFeedback = "quite poor...";
            break;
          default:
            danceTeacherFeedback = "very bad...";
            break;
        }
        drawDialogBubble("You matched " + String(danceScore) + " / " + String(danceCuesShown) + " dance cues (missed " + String(missedDanceCues) + "). Your performance was " + danceTeacherFeedback);
        break;
      }
      case TRAIN_SING3: {
        int missedMusicCoins = singNotesSpawned - singNotesCollected;
        String musicTeacherFeedback = "";
        switch(missedMusicCoins) {
          case 0:
            musicTeacherFeedback = "excellent!!";
            break;
          case 1: case 2: case 3:
            musicTeacherFeedback = "very good!!";
            break;
          case 4: case 5:
            musicTeacherFeedback = "good enough.";
            break;
          case 6: case 7: case 8:
            musicTeacherFeedback = "quite poor...";
            break;
          default:
            musicTeacherFeedback = "very bad...";
            break;
        }
        drawDialogBubble("You collected " + String(singNotesCollected) + " / " + String(singNotesSpawned) +" music coins (missed " + String(missedMusicCoins) +"). Your performance was " + musicTeacherFeedback);
        break;
      }
      case TRAIN_SWIM3: {
        String swimFeedback = "";
        if (swimCollisions == 0 && swimAvoidedSharks >= swimTargetSharks) {
          swimFeedback = "You were unstoppable!!";
        } else if (swimCollisions <= 2) {
          swimFeedback = "Great reflexes!";
        } else if (swimCollisions <= 5) {
          swimFeedback = "Good effort, keep focusing.";
        } else {
          swimFeedback = "Careful of those fins...";
        }
        drawDialogBubble("You dodged " + String(swimAvoidedSharks) + " sharks and were bumped " + String(swimCollisions) + " time(s). " + swimFeedback);
        break;
      }
      case TRAIN_GYM3: {
        String gymFeedback = "";
        if (gymMisses < 1) {
          gymFeedback = "You are excellent!!";
        } else if (gymMisses > 0 && gymMisses < 3) {
          gymFeedback = "Great reflexes!";
        } else if (gymMisses > 2 && gymMisses < 6) {
          gymFeedback = "Good effort, keep training.";
        } else {
          gymFeedback = "Careful not to hurt yourself...";
        }
        drawDialogBubble("You missed " + String(gymMisses) + " moves. " + gymFeedback);
        break;
      }
      case TRAIN_RUN3:
        drawDialogBubble("Running is very good for your health, see you again very soon!!");
        break;
      case FOOD_CONBINI2:
        drawConbimartOverlay();
        break;
      case FOOD_CONBINI3:
        drawDialogBubble("Thanks for shopping with us, come again!!");
        break;
      case FOOD_ORDER2:
        M5Cardputer.Display.fillRect(0, 0, 72, 10, BLACK);
        if (natsumi.money >= 600) {
          drawText("Price: $600", 5, 2, false, GREEN, 1);
        } else {
          drawText("Price: $600", 5, 2, false, RED, 1);
        }
        break;
      case FOOD_ORDER3:
        M5Cardputer.Display.fillRect(0, 0, 72, 10, BLACK);
        if (natsumi.money >= 750) {
          drawText("Price: $750", 5, 2, false, GREEN, 1);
        } else {
          drawText("Price: $750", 5, 2, false, RED, 1);
        }
        break;
      case FOOD_ORDER4:
        M5Cardputer.Display.fillRect(0, 0, 72, 10, BLACK);
        if (natsumi.money >= 1200) {
          drawText("Price: $1200", 5, 2, false, GREEN, 1);
        } else {
          drawText("Price: $1200", 5, 2, false, RED, 1);
        }
        break;
      case FOOD_ORDER8:
        natsumi.hunger = 4;
        if (natsumi.charm < 4) {
          natsumi.charm += 1;
        }
        drawDialogBubble("Hello, here is the food you ordered.");
        break;
      case FOOD_REST:
        drawDialogBubble("Irasshaimase! Please come in and enjoy your meal!");
        break;
      case FOOD_REST2:
        M5Cardputer.Display.fillRect(0, 0, 72, 10, BLACK);
        if (natsumi.money >= 700) {
          drawText("Price: $700", 5, 2, false, GREEN, 1);
        } else {
          drawText("Price: $700", 5, 2, false, RED, 1);
        }
        break;
      case FOOD_REST3:
        M5Cardputer.Display.fillRect(0, 0, 72, 10, BLACK);
        if (natsumi.money >= 800) {
          drawText("Price: $800", 5, 2, false, GREEN, 1);
        } else {
          drawText("Price: $800", 5, 2, false, RED, 1);
        }
        break;
      case FOOD_REST4:
        M5Cardputer.Display.fillRect(0, 0, 72, 10, BLACK);
        if (natsumi.money >= 900) {
          drawText("Price: $900", 5, 2, false, GREEN, 1);
        } else {
          drawText("Price: $900", 5, 2, false, RED, 1);
        }
        break;
      case FOOD_REST5:
        drawDialogBubble("Your food is on the way!! Call me if you need anything else.");
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
          drawDialogBubble("Treat yourself to some nice food, it\'s good for the soul.");
          priestState = FOOD_MENU;
        } else {
          drawDialogBubble("Congratulations!! You have a strong mind!");
          priestState = HOME_LOOP;
        }
        break;
      case FOOD_COOK:
        drawFoodGrid(foodGridItems, foodSelectionIndex);
        break;
      case FOOD_COOK2:
        {
          const float zoom = 2.0f;
          const int iconWidth = 18;
          const int iconHeight = 18;
          const int centerX = M5Cardputer.Display.width() / 2;
          const int centerY = M5Cardputer.Display.height() / 2;
          const int drawX = centerX - static_cast<int>((iconWidth * zoom) / 2);
          const int drawY = centerY - static_cast<int>((iconHeight * zoom) / 2);

          M5Cardputer.Display.drawPng(
            currentIcon.data,
            currentIcon.length,
            drawX,
            drawY,
            M5Cardputer.Display.width(),
            M5Cardputer.Display.height(),
            0,
            0,
            zoom,
            zoom
          );
        }
        break;
      case GARDEN_LOOP: case GARDEN_PLANT: case GARDEN_WATER: case GARDEN_PICK: case GARDEN_CLEANUP:
        drawGardenPlanter();
        break;
      case FLOWERS_MARKET7:
        drawDialogBubble("I sold all my flowers and made " + String(flowersRevenue) + "$");
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
  // Serial.println("> Entering drawStats()");
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
  // Serial.println("> Exiting drawStats()");
}

void drawStatBar(const String &label, int value, int maxValue, int x, int y, int width, int barHeight, uint16_t barColor, uint16_t bgColor, uint16_t frameColor) {
  // Serial.println("> Entering drawStatBar()");
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
  // Serial.println("> Entering manageStats()");
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
  if (natsumi.hunger < 4) {
    if (!foodGridInitialized) {
      prepareFoodGrid();
    }

    uint8_t key = 0;
    bool selectionChanged = false;

    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
      auto keyList = M5Cardputer.Keyboard.keyList();
      if (keyList.size() > 0) {
        key = M5Cardputer.Keyboard.getKey(keyList[0]);
        Serial.println(">> [KEY] = " + String(key));
        int currentCol = foodSelectionIndex % 4;
        int currentRow = foodSelectionIndex / 4;

        switch (key) {
          // UP
          case 59: case 'w': case 'W':
            if (currentRow > 0) {
              foodSelectionIndex -= 4;
              selectionChanged = true;
            }
            break;
          // DOWN
          case 46: case 's': case 'S':
            if (currentRow < 1 && foodSelectionIndex + 4 < static_cast<int>(foodGridItems.size())) {
              foodSelectionIndex += 4;
              selectionChanged = true;
            }
            break;
          // LEFT
          case 44: case 'a': case 'A':
            if (currentCol > 0) {
              foodSelectionIndex -= 1;
              selectionChanged = true;
            }
            break;
          // RIGHT
          case 47: case 'd': case 'D':
            if (currentCol < 3 && foodSelectionIndex + 1 < static_cast<int>(foodGridItems.size())) {
              foodSelectionIndex += 1;
              selectionChanged = true;
            }
            break;
          // ESC
          case 96: case 43:
            clearFoodGrid();
            overlayActive = false;
            changeState(0, HOME_LOOP, 0);
            return;
            break;
          // ENTER
          case 13: case 40: case ' ':
            if (!foodGridItems.empty()) {
              FoodDisplayItem &choice = foodGridItems[foodSelectionIndex];
              if (*(choice.quantityPtr) > 0) {
                *(choice.quantityPtr) -= 1;
                choice.quantity = *(choice.quantityPtr);
                if (natsumi.hunger < 4) {
                  natsumi.hunger += 1;
                }
                if (natsumi.charm < 4) {
                  natsumi.charm += 1;
                }
                showToast("Eating " + String(choice.label));
                clearFoodGrid();
                overlayActive = false;
                selectedFood = String(choice.label);
                changeState(0, FOOD_COOK2, 0);
              }
            }
            return;
            break;
        }
      }
    }

    if (selectionChanged) {
      l5NeedsRedraw = true;
    }
  } else {
    showToast("Natsumi is not hungry");
    overlayActive = false;
    changeState(0, HOME_LOOP, 0);
  }
}

void gotoRestaurant() {
  // Eat at the restaurant
  Serial.println("> Entering gotoRestaurant()");
  uint8_t key = 0;
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      key = M5Cardputer.Keyboard.getKey(keyList[0]);
      switch (currentState) {
        case FOOD_REST:
          changeState(0, FOOD_REST2, 0);
          break;
        case FOOD_REST5:
          changeState(0, FOOD_REST6, 0);
          break;
      }
    }
  }
  return;
}

void doctor() {
  // Serial.println("> Entering doctor()");
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
  // Serial.println("> Entering priest()");
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

void cashier() {
  // Serial.println("> Entering cashier()");
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

void foodDelivery() {
  // Serial.println("> Entering foodDelivery()");
  uint8_t key = 0;
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      key = M5Cardputer.Keyboard.getKey(keyList[0]);
      switch (currentState) {
        case FOOD_ORDER8:
          changeState(0, HOME_LOOP, 0);
          break;
      }
      return;
    }
  }
}

void miniGameDebrief() {
  // Serial.println("> Entering miniGameDebrief()");
  uint8_t key = 0;
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      key = M5Cardputer.Keyboard.getKey(keyList[0]);
      switch (currentState) {
        case FLOWERS_MARKET7:
          changeState(0, HOME_LOOP, 0);
          break;
        case TRAIN_SING3:
          changeState(0, HOME_LOOP, 0);
          break;
        case TRAIN_DANCE3:
          changeState(0, HOME_LOOP, 0);
          break;
        case TRAIN_SWIM3:
          changeState(0, HOME_LOOP, 0);
          break;
        case TRAIN_GYM3:
          changeState(0, HOME_LOOP, 0);
          break;
        case TRAIN_RUN3:
          changeState(0, HOME_LOOP, 0);
          break;
      }
      return;
    }
  }
}

void gotoConbimart() {
  Serial.println("> Entering gotoConbimart()");
  if (!conbimartInitialized) {
    prepareConbimartItems();
    return;
  }

  uint8_t key = 0;
  bool needsRedraw = false;
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      key = M5Cardputer.Keyboard.getKey(keyList[0]);
      int itemCount = static_cast<int>(conbimartItems.size());
      Serial.println(">> gotoConbimart - Key: " + String(key));
      switch (key) {
        // UP
        case 59: case 'w': case 'W':
          if (itemCount > 0) {
            conbimartSelectionIndex = (conbimartSelectionIndex - 1 + itemCount) % itemCount;
            needsRedraw = true;
          }
          break;
        // DOWN
        case 46: case 's': case 'S':
          if (itemCount > 0) {
            conbimartSelectionIndex = (conbimartSelectionIndex + 1) % itemCount;
            needsRedraw = true;
          }
          break;
        // ADD ONE ITEM
        case 47: case 61:
          if (itemCount > 0) {
            conbimartItems[conbimartSelectionIndex].quantity += 1;
            needsRedraw = true;
          }
          break;
        // REMOVE ONE ITEM
        case 44: case 45:
          if (itemCount > 0 && conbimartItems[conbimartSelectionIndex].quantity > 0) {
            conbimartItems[conbimartSelectionIndex].quantity -= 1;
            needsRedraw = true;
          }
          break;
        // CONFIRM PURCHASE
        case 13: case 40: case ' ':
          {
            int total = getConbimartTotal();
            if (total == 0) {
              showToast("Your basket is empty.");
              break;
            }
            if (natsumi.money >= total) {
              natsumi.money -= total;
              for (auto &item : conbimartItems) {
                *(item.stockPtr) += item.quantity;
              }
              // showToast("Thanks for shopping with us!");
              conbimartItems.clear();
              conbimartInitialized = false;
              overlayActive = false;
              menuEnabled = true;
              changeState(0, FOOD_CONBINI3, 0);
              return;
            } else {
              showToast("Not enough money :(");
            }
          }
          break;
        // CANCEL
        case 96:
          conbimartItems.clear();
          conbimartInitialized = false;
          overlayActive = false;
          changeState(0, HOME_LOOP, 0);
          return;
      }
    }
  }

  if (needsRedraw) {
    l5NeedsRedraw = true;
  }
}

void manageOnsen() {
  // Serial.println("> Entering manageOnsen()");
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

  if (fiveSecondPulse || lastOnsenEnergyDisplayed != natsumi.energy || lastOnsenSpiritDisplayed != natsumi.spirit) {
    drawOnsenOverlay();
  }
  /*
  if (natsumi.energy == 4 && natsumi.hygiene == 4) {
    changeState(0, HOME_LOOP, 0);
  }
  */
  return;
}

void showFood() {
  //
  changeState(0, HOME_LOOP, microWait);
  return;
}

void restaurantFoodSelection() {
  Serial.println("> Entering restaurantFoodSelection()");
  uint8_t key = 0;
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      Serial.println(">> restaurantFoodSelection() Key pressed");
      key = M5Cardputer.Keyboard.getKey(keyList[0]);
      switch (currentState) {
        case FOOD_REST2:
          switch (key) {
            // LEFT
            case 44: case 'a': case 'A':
              changeState(0, FOOD_REST4, 0);
              break;
            // RIGHT
            case 47: case 'd': case 'D':
              changeState(0, FOOD_REST3, 0);
              break;
            // ENTER
            case 13: case 40: case ' ':
              restaurantSelection = 0;
              if (natsumi.money >= 700) {
                natsumi.money -= 700;
                natsumi.hunger = 4;
                if (natsumi.charm < 4) {
                  natsumi.charm += 1;
                }
              } else {
                showToast("Not enough money :(");
              }
              changeState(0, FOOD_REST5, 0);
              break;
          }
          break;
        case FOOD_REST3:
          switch (key) {
            // LEFT
            case 44: case 'a': case 'A':
              changeState(0, FOOD_REST2, 0);
              break;
            // RIGHT
            case 47: case 'd': case 'D':
              changeState(0, FOOD_REST4, 0);
              break;
            // ENTER
            case 13: case 40: case ' ':
              restaurantSelection = 1;
              if (natsumi.money >= 800) {
                natsumi.money -= 800;
                natsumi.hunger = 4;
                if (natsumi.charm < 4) {
                  natsumi.charm += 1;
                }
              } else {
                showToast("Not enough money :(");
              }
              changeState(0, FOOD_REST5, 0);
              break;
          }
          break;
        case FOOD_REST4:
          switch (key) {
            // LEFT
            case 44: case 'a': case 'A':
              changeState(0, FOOD_REST3, 0);
              break;
            // RIGHT
            case 47: case 'd': case 'D':
              changeState(0, FOOD_REST2, 0);
              break;
            // ENTER
            case 13: case 40: case ' ':
              restaurantSelection = 2;
              if (natsumi.money >= 900) {
                natsumi.money -= 900;
                natsumi.hunger = 4;
                if (natsumi.charm < 4) {
                  natsumi.charm += 1;
                }
              } else {
                showToast("Not enough money :(");
              }
              changeState(0, FOOD_REST5, 0);
              break;
          }
          break;
        default:
          changeState(0, HOME_LOOP, 0);
          break;
      }
    }
  }

  /*
  // Stats management
  updateAging();
  updateStats();
  */
  return;
}

void orderibiFoodSelection() {
  // Serial.println("> Entering orderibiFoodSelection()");
  uint8_t key = 0;
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      key = M5Cardputer.Keyboard.getKey(keyList[0]);
      switch (currentState) {
        case FOOD_ORDER2:
          switch (key) {
            // LEFT
            case 44: case 'a': case 'A':
              changeState(0, FOOD_ORDER4, 0);
              break;
            // RIGHT
            case 47: case 'd': case 'D':
              changeState(0, FOOD_ORDER3, 0);
              break;
            // ENTER
            case 13: case 40: case ' ':
              orderibiSelection = 0;
              if (natsumi.money >= 600) {
                natsumi.money -= 600;
              } else {
                showToast("Not enough money :(");
              }
              changeState(0, FOOD_ORDER5, 0);
              break;
          }
          break;
        case FOOD_ORDER3:
          switch (key) {
            // LEFT
            case 44: case 'a': case 'A':
              changeState(0, FOOD_ORDER2, 0);
              break;
            // RIGHT
            case 47: case 'd': case 'D':
              changeState(0, FOOD_ORDER4, 0);
              break;
            // ENTER
            case 13: case 40: case ' ':
              orderibiSelection = 1;
              if (natsumi.money >= 750) {
                natsumi.money -= 750;
              } else {
                showToast("Not enough money :(");
              }
              changeState(0, FOOD_ORDER5, 0);
              break;
          }
          break;
        case FOOD_ORDER4:
          switch (key) {
            // LEFT
            case 44: case 'a': case 'A':
              changeState(0, FOOD_ORDER3, 0);
              break;
            // RIGHT
            case 47: case 'd': case 'D':
              changeState(0, FOOD_ORDER2, 0);
              break;
            // ENTER
            case 13: case 40: case ' ':
              orderibiSelection = 2;
              if (natsumi.money >= 1200) {
                natsumi.money -= 1200;
              } else {
                showToast("Not enough money :(");
              }
              changeState(0, FOOD_ORDER5, 0);
              break;
          }
          break;
        default:
          changeState(0, HOME_LOOP, 0);
          break;
      }
    }
  }
  return;
}
