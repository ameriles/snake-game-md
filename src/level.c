#include "level.h"

// --- public functions
void level_init(Level* level, u8 number) {

  // generate the levels procedurally based on the level number
  level->number = number;
  level->bonus = 100 * number;

  if (number < 3) {
    level->initialSpeed = 1;
    level->foodStep = 4;
    level->initialSegments = 3;
    level->foodCount = 3;
  } else if (number < 8) {
    level->initialSpeed = 2;
    level->foodStep = 8;
    level->initialSegments = 4;
    level->foodCount = 6;
  } else {
    level->initialSpeed = 4;
    level->foodStep = 10;
    level->initialSegments = 5;
    level->foodCount = 10;
  }
}

void level_decreaseBonus(Level* level, u8 amount) {
  if (level->bonus > 0) {
    level->bonus = (level->bonus > amount) ? level->bonus - amount : 0;
  }
}
