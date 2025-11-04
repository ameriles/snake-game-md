#include "snake.h"
#include "types.h"
#include "resources.h"

// --- public functions

void snake_init(Snake* snake) {
  snake->length = 1;
  snake->speed = 1;

  SnakeSegment* segment = &snake->segments[0];
  segment->x = SCREEN_WIDTH / 2;   // half of the screen width
  segment->y = SCREEN_HEIGHT / 2;  // half of the screen height
  segment->direction = RIGHT;
  segment->sprite = SPR_addSprite(&sprSnakeSegment, segment->x, segment->y, TILE_ATTR(PAL1, 0, FALSE, FALSE));
}

void snake_updateSpeed(Snake* snake, u8 reqSpeed) {
  // Ensure all segments are in the same direction before changing speed
  for (int i = 1; i < snake->length; i++) {
    SnakeSegment *segment = &snake->segments[i];
    if (segment->direction != snake->segments[0].direction) {
      return; // cannot increase speed if segments are going in different directions
    }
  }

  if (snake->speed != reqSpeed) {
    snake->speed = reqSpeed;
  }
}

void snake_updateDirection(Snake* snake, enum SnakeDirection reqDirection) {
  for (int i = snake->length - 1; i >= 0; i--) {
    SnakeSegment *segment = &snake->segments[i];

    if (i == 0 && segment->direction != reqDirection) { // the head
      if ((segment->x % SEGMENT_SIZE) != 0 || (segment->y % SEGMENT_SIZE) != 0) {
        // If the head is not aligned with the grid, we don't change its direction
        continue;
      }
      // Update the head's direction based on the requested direction
      segment->direction = reqDirection;
      segment->prevX = segment->x;
      segment->prevY = segment->y;
    } else {
      // Update the segment's direction to follow the previous segment, only when it is aligned
      SnakeSegment *prevSegment = &snake->segments[i - 1];
      if (segment->x == prevSegment->prevX && segment->y == prevSegment->prevY) {
        segment->direction = prevSegment->direction;
        segment->prevX = segment->x;
        segment->prevY = segment->y;
      }
    }
  }
}

void snake_move(Snake* snake) {
  u8 speed = snake->speed;

  for (int i = 0; i < snake->length; i++) {
    SnakeSegment *segment = &snake->segments[i];
    switch (segment->direction) {
      case UP:
        segment->y = (segment->y - speed < 0) ? SCREEN_HEIGHT - SEGMENT_SIZE : segment->y - speed;
        break;
      case RIGHT:
        segment->x = (segment->x + speed > SCREEN_WIDTH - SEGMENT_SIZE) ? 0 : segment->x + speed;
        break;
      case DOWN:
        segment->y = (segment->y + speed > SCREEN_HEIGHT - SEGMENT_SIZE) ? 0 : segment->y + speed;
        break;
      case LEFT:
        segment->x = (segment->x - speed < 0) ? SCREEN_WIDTH - SEGMENT_SIZE : segment->x - speed;
        break;
    }
  }
}

void snake_grow(Snake* snake) {
  if (snake->length == SNAKE_MAX_LENGTH) {
    return;
  }

  u16 xOffset = 0;
  u16 yOffset = 0;
  
  SnakeSegment *newSegment = &snake->segments[snake->length];
  SnakeSegment *lastSegment = &snake->segments[snake->length - 1];
  
  if (lastSegment->direction == UP) {
    yOffset = SEGMENT_SIZE;
  } else if (lastSegment->direction == RIGHT) {
    xOffset = -SEGMENT_SIZE;
  } else if (lastSegment->direction == DOWN) {
    yOffset = -SEGMENT_SIZE;
  } else if (lastSegment->direction == LEFT) {
    xOffset = SEGMENT_SIZE;
  }

  newSegment->x = lastSegment->x + xOffset;
  newSegment->y = lastSegment->y + yOffset;
  newSegment->direction = lastSegment->direction;

  newSegment->sprite = SPR_addSprite(&sprSnakeSegment, newSegment->x, newSegment->y, TILE_ATTR(PAL1, 0, FALSE, FALSE));
  snake->length++;
}

void snake_render(Snake* snake) {
  for (int i = 0; i < snake->length; i++) {
    SnakeSegment *segment = &snake->segments[i];
    SPR_setPosition(segment->sprite, segment->x, segment->y);
  }
}

void snake_cleanUp(Snake* snake) {
  for (int i = 0; i < snake->length; i++) {
    if (snake->segments[i].sprite) {
      SPR_releaseSprite(snake->segments[i].sprite);
      snake->segments[i].sprite = NULL;
    }
  }
}
