#include "food.h"
#include "types.h"
#include "resources.h"

// --- public functions

void food_init(Food* food) {
  food->x = (random() % (SCREEN_TILE_WIDTH - 1)) * FOOD_SIZE;
  food->y = (random() % (SCREEN_TILE_HEIGHT - 1)) * FOOD_SIZE;
  food->sprite = SPR_addSprite(&sprSnakeSegment, food->x, food->y, TILE_ATTR(PAL1, 0, FALSE, FALSE));
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