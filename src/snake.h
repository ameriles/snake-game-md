#ifndef SNAKE_H
#define SNAKE_H

#include <genesis.h>

#define SNAKE_TILE 1
#define SNAKE_MAX_LENGTH 20
#define SEGMENT_SIZE 8

enum SnakeDirection {
  UP,
  RIGHT,
  DOWN,
  LEFT
};

typedef struct {
  u16 x;
  u16 y;
  Sprite *sprite;
  enum SnakeDirection direction;
  u16 prevX;
  u16 prevY;
} SnakeSegment;

typedef struct {
  SnakeSegment segments[SNAKE_MAX_LENGTH];
  u8 length;
  u8 speed;
} Snake;

void snake_init(Snake* snake);
void snake_updateSpeed(Snake* snake, u8 reqSpeed);
void snake_updateDirection(Snake* snake, enum SnakeDirection reqDirection);
void snake_move(Snake* snake);
void snake_grow(Snake* snake);
void snake_render(Snake* snake);
void snake_cleanUp(Snake* snake);

#endif