#ifndef FOOD_H
#define FOOD_H

#include <genesis.h>

#define FOOD_SIZE 8

typedef struct {
  u16 x;
  u16 y;
  Sprite *sprite;
} Food;

void food_init(Food* food);
void food_render(Food* food);
void food_cleanUp(Food* food);

#endif