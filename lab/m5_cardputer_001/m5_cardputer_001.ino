#include "M5Cardputer.h"

// Enum to manage game states
enum GameState {
    HOME,
    FEEDING_LOOP,
    CLEAN_LOOP,
    EDUCATION_LOOP,
    RELAXATION_LOOP,
    MENU_NAVIGATION
};

GameState currentState = HOME;
int menuSelection = 0;
const int maxMenuItems = 9;

void setup() {
    M5.begin();
    M5Cardputer.Keyboard.begin();
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("Idol Natsumi - HOME");
}

void loop() {
    M5.update();
    auto keyState = M5Cardputer.Keyboard.getKeys();

    if (keyState.wasPressed()) {
        handleKeyPress(keyState.key);
    }
}

void handleKeyPress(char key) {
    switch (key) {
        case 'f':
        case 'F':
            currentState = FEEDING_LOOP;
            M5.Lcd.println("Entering Feeding Loop...");
            break;

        case 'c':
        case 'C':
            currentState = CLEAN_LOOP;
            M5.Lcd.println("Entering Clean Loop...");
            break;

        case 'e':
        case 'E':
            currentState = EDUCATION_LOOP;
            M5.Lcd.println("Entering Education Loop...");
            break;

        case 'r':
        case 'R':
            currentState = RELAXATION_LOOP;
            M5.Lcd.println("Entering Relaxation Loop...");
            break;

        case 0x80:  // UP Arrow
            if (currentState == MENU_NAVIGATION) {
                menuSelection = (menuSelection - 1 + maxMenuItems) % maxMenuItems;
                M5.Lcd.printf("Menu Selection: %d\n", menuSelection + 1);
            }
            break;

        case 0x81:  // DOWN Arrow
            if (currentState == MENU_NAVIGATION) {
                menuSelection = (menuSelection + 1) % maxMenuItems;
                M5.Lcd.printf("Menu Selection: %d\n", menuSelection + 1);
            }
            break;

        case '1' ... '9':  // Quick select menu entry
            if (currentState == MENU_NAVIGATION) {
                int selected = key - '0';
                if (selected <= maxMenuItems) {
                    menuSelection = selected - 1;
                    M5.Lcd.printf("Quick Selected: %d\n", selected);
                }
            }
            break;

        case '\n':  // ENTER
            if (currentState == MENU_NAVIGATION) {
                M5.Lcd.printf("Validated: Entry %d\n", menuSelection + 1);
                // Trigger action based on selection
            }
            break;

        case 0x1B:  // ESCAPE
            currentState = HOME;
            M5.Lcd.println("Back to HOME screen.");
            break;

        default:
            // Ignore other keys or display debug
            break;
    }
}
