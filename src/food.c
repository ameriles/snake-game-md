#include "food.h"
#include "types.h"
#include "resources.h"

static const u16 FOOD_FRAME_PALETTE_1[16] = {
  0x0000, 0x0005, 0x000A, 0x000F, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

static const u16 FOOD_FRAME_PALETTE_2[16] = {
  0x0000, 0x0004, 0x0009, 0x000E, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

static const u16 FOOD_FRAME_PALETTE_3[16] = {
  0x0000, 0x0003, 0x0008, 0x000D, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

// --- public functions

void food_init(Food* food) {
  food->x = (random() % (SCREEN_TILE_WIDTH - 1)) * FOOD_SIZE;
  food->y = (random() % (SCREEN_TILE_HEIGHT - 1)) * FOOD_SIZE;
  food->sprite = SPR_addSprite(&sprSnakeSegment, food->x, food->y, TILE_ATTR(PAL2, 0, FALSE, FALSE));
}

void food_render(Food* food) {
  SPR_setPosition(food->sprite, food->x, food->y);
}

void food_cleanUp(Food* food) {
  if (food->sprite != NULL) {
    SPR_releaseSprite(food->sprite);
    food->sprite = NULL;
  }
}

void food_updatePalette(Food* food) {
  u8 frame = food->frame;
  if (frame == 0) {
    PAL_setPalette(PAL2, FOOD_FRAME_PALETTE_1, CPU);
    food->frame = 1;
  } else if (frame == 1) {
    PAL_setPalette(PAL2, FOOD_FRAME_PALETTE_2, CPU);
    food->frame = 2;
  } else if (frame == 2) {
    PAL_setPalette(PAL2, FOOD_FRAME_PALETTE_3, CPU);
    food->frame = 3;
  } else {
    PAL_setPalette(PAL2, FOOD_FRAME_PALETTE_2, CPU);
    food->frame = 0;
  }
}