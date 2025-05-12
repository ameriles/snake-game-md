#include <genesis.h>
#include "resources.h"
#include "types.h"
#include "pal_test.h"
#include "main_menu.h"
#include "in_game.h"

void changeScreen(enum Screen newScreen) {
	// Clear screen
	VDP_clearPlane(BG_A, TRUE);
	VDP_clearPlane(BG_B, TRUE);
	VDP_clearPlane(WINDOW, TRUE);

	switch (newScreen) {
	case MAIN_MENU:
		mainMenu_start();
		break;
	case IN_GAME:
		inGame_start();
		break;
	case PAL_TEST:
		palTest_start();
		break;
	}
}

int main(bool resetType) {

	enum Screen screen = MAIN_MENU;
	bool running = TRUE;

	// Soft resets don't clear RAM, this can bring some bugs so we hard reset every time we detect a soft reset
	if (!resetType)
		SYS_hardReset();

  // Initialize joypad and sprite engine in order to use them
	JOY_init();

	// Initialize SPR engine
	SPR_init();

	enum Screen nextScreen = screen;
	mainMenu_start();

	while (running) {
		if (nextScreen != screen) {
			changeScreen(nextScreen);
			screen = nextScreen;
		}

		switch (screen) {
		case MAIN_MENU:
			nextScreen = mainMenu_update();
			break;
		case IN_GAME:
			nextScreen = inGame_update();
			break;
		case PAL_TEST:
			nextScreen = palTest_update();
			break;
		}

		SYS_doVBlankProcess();
	}

	return 0;
}

