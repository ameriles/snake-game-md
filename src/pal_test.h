#ifndef PAL_TEST_H
#define PAL_TEST_H

#include <genesis.h>

#define SCREEN_TILE_WIDTH 40 // 320px / 8px per tile
#define SCREEN_TILE_HEIGHT 28 // 224px / 8px per tile

void palTest_start();
enum Screen palTest_update();

#endif