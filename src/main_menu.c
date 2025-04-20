#include "main_menu.h"
#include "types.h"

// --- private state shape
typedef struct {
    u8 selectedOption;
    enum Screen nextScreen;
} MainMenuState;

// --- private state
static MainMenuState _state;

// --- private functions
static void mainMenuJoyEvent(u16 joy, u16 changed, u16 state) {
  
  if (state & BUTTON_UP) {
    _state.selectedOption = _state.selectedOption == 0 ? 1 : 0;
  } else if (state & BUTTON_DOWN) {
    _state.selectedOption = _state.selectedOption == 1 ? 0 : 1;
  } else if (state & BUTTON_C) {
    _state.nextScreen = _state.selectedOption == 0 ? IN_GAME : PAL_TEST;
  }
}

// --- public functions
void mainMenu_start() {
  // Initialize variables
  _state.nextScreen = MAIN_MENU;
  _state.selectedOption = 0;

  // Setup a callback when a button is pressed, we could call it a "pseudo parallel" joypad handler
  JOY_setEventHandler(mainMenuJoyEvent);
}

enum Screen mainMenu_update() {
  VDP_drawText(_state.selectedOption == 0 ? ">START GAME" : " START GAME", 5, 5);
  VDP_drawText(_state.selectedOption == 1 ? ">PALETTE TEST" : " PALETTE TEST", 5, 6);

  return _state.nextScreen;
}
