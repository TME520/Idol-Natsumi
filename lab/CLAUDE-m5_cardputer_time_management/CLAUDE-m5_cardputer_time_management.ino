#include <M5Cardputer.h>
#include <EEPROM.h>

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

// Player character struct
struct Player {
  int x, y;
  int health;
  int score;
  bool isMoving;
};

// Time tracking variables
unsigned long sessionStartTime;
unsigned long totalGameTime = 0; // Total time across all sessions (in milliseconds)
unsigned long lastSaveTime = 0;

// Game variables
GameState currentState = MENU;
Player natsumi;
unsigned long lastUpdate = 0;
const int FRAME_DELAY = 50; // ~20 FPS

// Menu variables
int menuSelection = 0;
const char* menuItems[] = {"Start Game", "Settings", "High Score"};
const int menuItemCount = 3;

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);
  
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setTextColor(WHITE);
  
  // Initialize time tracking
  sessionStartTime = millis();
  
  // TODO: Load totalGameTime from EEPROM in future
  // loadGameState();
  
  // Initialize player
  initPlayer();
  
  // Show initial menu
  drawMenu();
}

void loop() {
  M5Cardputer.update();
  
  // Frame rate control
  if (millis() - lastUpdate < FRAME_DELAY) {
    return;
  }
  lastUpdate = millis();
  
  // Handle input based on current state
  handleInput();
  
  // Update game logic based on state
  switch (currentState) {
    case MENU:
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
  natsumi.x = 120; // Center of screen (240px wide)
  natsumi.y = 100;
  natsumi.health = 100;
  natsumi.score = 0;
  natsumi.isMoving = false;
}

void handleInput() {
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keyList = M5Cardputer.Keyboard.keyList();
    if (keyList.size() > 0) {
      uint8_t keyValue = M5Cardputer.Keyboard.getKey(keyList[0]);
      
      switch (currentState) {
        case MENU:
          handleMenuInput(keyValue);
          break;
        case PLAYING:
          handleGameInput(keyValue);
          break;
        case GAME_OVER:
          handleGameOverInput(keyValue);
          break;
        case SETTINGS:
          handleSettingsInput(keyValue);
          break;
      }
    }
  }
}

void handleMenuInput(uint8_t key) {
  switch (key) {
    case 181: // Up arrow or 'w'
    case 'w':
    case 'W':
      menuSelection = (menuSelection - 1 + menuItemCount) % menuItemCount;
      drawMenu();
      break;
    case 182: // Down arrow or 's'
    case 's':
    case 'S':
      menuSelection = (menuSelection + 1) % menuItemCount;
      drawMenu();
      break;
    case 13: // Enter
    case ' ': // Space
      selectMenuItem();
      break;
  }
}

void handleGameInput(uint8_t key) {
  switch (key) {
    case 181: // Up
    case 'w':
    case 'W':
      if (natsumi.y > 10) natsumi.y -= 5;
      natsumi.isMoving = true;
      break;
    case 182: // Down
    case 's':
    case 'S':
      if (natsumi.y < 125) natsumi.y += 5;
      natsumi.isMoving = true;
      break;
    case 180: // Left
    case 'a':
    case 'A':
      if (natsumi.x > 10) natsumi.x -= 5;
      natsumi.isMoving = true;
      break;
    case 183: // Right
    case 'd':
    case 'D':
      if (natsumi.x < 230) natsumi.x += 5;
      natsumi.isMoving = true;
      break;
    case 27: // Escape
    case 'q':
    case 'Q':
      currentState = MENU;
      drawMenu();
      break;
  }
}

void handleGameOverInput(uint8_t key) {
  if (key == 13 || key == ' ') { // Enter or Space
    currentState = MENU;
    drawMenu();
  }
}

void handleSettingsInput(uint8_t key) {
  if (key == 27 || key == 'q' || key == 'Q') { // Escape
    currentState = MENU;
    drawMenu();
  }
}

void updateMenu() {
  // Menu doesn't need constant updates, only when input changes
}

void updateGame() {
  // Game logic updates
  natsumi.isMoving = false; // Reset movement flag
  
  // Example: increase score over time
  natsumi.score += 1;
  
  // Draw game screen
  drawGame();
}

// Function to get current total elapsed time in seconds
unsigned long getTotalElapsedSeconds() {
  unsigned long currentSessionTime = millis() - sessionStartTime;
  unsigned long currentTotalTime = totalGameTime + currentSessionTime;
  return currentTotalTime / 1000; // Convert to seconds
}

// Function to format time as HH:MM:SS
String formatTime(unsigned long totalSeconds) {
  unsigned long hours = totalSeconds / 3600;
  unsigned long minutes = (totalSeconds % 3600) / 60;
  unsigned long seconds = totalSeconds % 60;
  
  String timeStr = "";
  if (hours < 10) timeStr += "0";
  timeStr += String(hours) + ":";
  if (minutes < 10) timeStr += "0";
  timeStr += String(minutes) + ":";
  if (seconds < 10) timeStr += "0";
  timeStr += String(seconds);
  
  return timeStr;
}

void updateGameOver() {
  // Game over logic
}

void updateSettings() {
  // Settings logic
}

void drawMenu() {
  M5Cardputer.Display.fillScreen(BLACK);
  
  // Title
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setCursor(50, 10);
  M5Cardputer.Display.setTextColor(PINK);
  M5Cardputer.Display.println("IDOL NATSUMI");
  
  // Display elapsed time
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setCursor(10, 35);
  M5Cardputer.Display.setTextColor(CYAN);
  M5Cardputer.Display.print("Elapsed: ");
  M5Cardputer.Display.println(formatTime(getTotalElapsedSeconds()));
  
  // Menu items
  M5Cardputer.Display.setTextSize(1);
  for (int i = 0; i < menuItemCount; i++) {
    M5Cardputer.Display.setCursor(60, 55 + i * 15);
    
    if (i == menuSelection) {
      M5Cardputer.Display.setTextColor(YELLOW);
      M5Cardputer.Display.print("> ");
    } else {
      M5Cardputer.Display.setTextColor(WHITE);
      M5Cardputer.Display.print("  ");
    }
    
    M5Cardputer.Display.println(menuItems[i]);
  }
  
  // Instructions
  M5Cardputer.Display.setTextColor(0x7BEF); // Light gray color
  M5Cardputer.Display.setCursor(10, 115);
  M5Cardputer.Display.println("W/S: Navigate, Enter: Select");
}

void drawGame() {
  M5Cardputer.Display.fillScreen(BLACK);
  
  // Draw elapsed time at top
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setTextColor(CYAN);
  M5Cardputer.Display.setCursor(5, 5);
  M5Cardputer.Display.print("Time: ");
  M5Cardputer.Display.println(formatTime(getTotalElapsedSeconds()));
  
  // Draw UI
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.setCursor(5, 15);
  M5Cardputer.Display.printf("Score: %d", natsumi.score);
  
  M5Cardputer.Display.setCursor(5, 25);
  M5Cardputer.Display.printf("Health: %d", natsumi.health);
  
  // Draw Natsumi (simple representation)
  M5Cardputer.Display.setTextColor(PINK);
  M5Cardputer.Display.setCursor(natsumi.x, natsumi.y);
  M5Cardputer.Display.print("♪"); // Musical note character
  
  // Draw game area border
  M5Cardputer.Display.drawRect(5, 35, 230, 95, WHITE);
  
  // Instructions
  M5Cardputer.Display.setTextColor(0x7BEF); // Light gray color
  M5Cardputer.Display.setCursor(150, 15);
  M5Cardputer.Display.println("WASD: Move, Q: Menu");
}

void selectMenuItem() {
  switch (menuSelection) {
    case 0: // Start Game
      initPlayer();
      currentState = PLAYING;
      break;
    case 1: // Settings
      currentState = SETTINGS;
      drawSettings();
      break;
    case 2: // High Score
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
  
  M5Cardputer.Display.setTextColor(0x7BEF); // Light gray color
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
  
  delay(2000); // Show for 2 seconds
  drawMenu();
}