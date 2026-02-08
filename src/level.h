#ifndef LEVEL_H
#define LEVEL_H

#include <genesis.h>
#include "block.h"

#define MAX_LEVELS 10

typedef struct {
  u8 number;
  u8 initialSpeed;
  u8 initialSegments;
  u8 foodCount;
  u8 foodStep;
  u16 bonus;
//  Block blocks[];
} Level;

void level_init(Level* level, u8 number);
void level_decreaseBonus(Level* level, u8 amount);
// void level_render(Level* level);
// void level_cleanUp(Level* level);

#endif