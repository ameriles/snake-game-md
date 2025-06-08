#include "in_game.h"
#include "types.h"
#include "resources.h"

#define SNAKE_TILE 1
#define SNAKE_MAX_LENGTH 20
#define SCREEN_TILE_WIDTH 40
#define SCREEN_TILE_HEIGHT 28
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 224
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

typedef struct {
  u16 x;
  u16 y;
  Sprite *sprite;
} Food;

// --- private state shape
typedef struct {
  bool gameOver;
  Snake snake;
  Food food;
  u8 foodLeft;
  u16 timerTicks;
  enum SnakeDirection reqDirection;
  u8 reqSpeed;
} InGameState;

// --- private state
static InGameState _state;

static void loadPalettes() {
  PAL_setPalette(PAL1, snakeSegment.palette->data, CPU);
}

static void advanceSnake() {
  u8 speed = _state.snake.speed;

  for (int i = 0; i < _state.snake.length; i++) {
    SnakeSegment *segment = &_state.snake.segments[i];
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

static void updateDirections() {
  for (int i = _state.snake.length - 1; i >= 0; i--) {
    SnakeSegment *segment = &_state.snake.segments[i];

    if (i == 0 && segment->direction != _state.reqDirection) { // the head
      if ((segment->x % SEGMENT_SIZE) != 0 || (segment->y % SEGMENT_SIZE) != 0) {
        // If the head is not aligned with the grid, we don't change its direction
        continue;
      }
      // Update the head's direction based on the requested direction
      segment->direction = _state.reqDirection;
      segment->prevX = segment->x;
      segment->prevY = segment->y;
    } else {
      // Update the segment's direction to follow the previous segment, only when it is aligned
      SnakeSegment *prevSegment = &_state.snake.segments[i - 1];
      if (segment->x == prevSegment->prevX && segment->y == prevSegment->prevY) {
        segment->direction = prevSegment->direction;
        segment->prevX = segment->x;
        segment->prevY = segment->y;
      }
    }
  }
}

static void updateSpeed() {
  // Ensure all segments are in the same direction before changing speed
  for (int i = 1; i < _state.snake.length; i++) {
    SnakeSegment *segment = &_state.snake.segments[i];
    if (segment->direction != _state.snake.segments[0].direction) {
      return; // cannot increase speed if segments are going in different directions
    }
  }

  if (_state.snake.speed != _state.reqSpeed) {
    _state.snake.speed = _state.reqSpeed;
  }
}

static void updateSnake() {
  // if (_state.timerTicks % 25 != 0) {
  //  return;
  // }

  updateSpeed();

  updateDirections();

  advanceSnake();
}

static void renderSnake() {
  for (int i = 0; i < _state.snake.length; i++) {
    SnakeSegment *segment = &_state.snake.segments[i];
    SPR_setPosition(segment->sprite, segment->x, segment->y);
  }
}

static void growSnake() {
  if (_state.snake.length == SNAKE_MAX_LENGTH) {
    return;
  }

  u16 xOffset = 0;
  u16 yOffset = 0;
  
  SnakeSegment *newSegment = &_state.snake.segments[_state.snake.length];
  SnakeSegment *lastSegment = &_state.snake.segments[_state.snake.length - 1];
  
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

  newSegment->sprite = SPR_addSprite(&snakeSegment, newSegment->x, newSegment->y, TILE_ATTR(PAL1, 0, FALSE, FALSE));
  _state.snake.length++;
}

static void increaseSpeed() {
  switch (_state.snake.speed) {
    case 1:
      _state.reqSpeed = 2;
      break;
    case 2:
      _state.reqSpeed = 4;
      break;
    case 4:
      _state.reqSpeed = 8;
      break;
    default:
      _state.reqSpeed = 8;  // cap the speed at 8
      break;
  }
}

static void renderFood() {
  if (_state.foodLeft == 0) {
    return; // no food to render
  }

  SPR_setPosition(_state.food.sprite, _state.food.x, _state.food.y);
}

static void checkCollisions() {
  SnakeSegment *head = &_state.snake.segments[0];

  // Check if the snake head collides with the food
  if (head->x >= _state.food.x && head->x < _state.food.x + SEGMENT_SIZE &&
      head->y >= _state.food.y && head->y < _state.food.y + SEGMENT_SIZE) {
    growSnake();

    _state.foodLeft--;
    
    // Move food to a new random position
    _state.food.x = (random() % (SCREEN_TILE_WIDTH - 1)) * SEGMENT_SIZE;
    _state.food.y = (random() % (SCREEN_TILE_HEIGHT - 1)) * SEGMENT_SIZE;

    if (_state.foodLeft == 0) {
      _state.gameOver = TRUE;
      return;
    }

    if (_state.foodLeft % 4 == 0) {
      increaseSpeed(); // increase speed every 4 food eaten
    }
  }

  // // Check if the snake collides with itself
  // for (int i = 1; i < _state.snake.length; i++) {
  //   SnakeSegment *segment = &_state.snake.segments[i];
  //   if (head->x >= segment->x && head->x < segment->x + SEGMENT_SIZE &&
  //       head->y >= segment->y && head->y < segment->y + SEGMENT_SIZE) {
  //     _state.gameOver = TRUE;
  //     break;
  //   }
  // }
}

static void inGameJoyEvent(u16 joy, u16 changed, u16 state) {
  if (state & BUTTON_C) {
    _state.gameOver = TRUE;
  } else if (state & BUTTON_UP) {
    _state.reqDirection = UP;
  } else if (state & BUTTON_RIGHT) {
    _state.reqDirection = RIGHT;
  } else if (state & BUTTON_DOWN) {
    _state.reqDirection = DOWN;
  } else if (state & BUTTON_LEFT) {
    _state.reqDirection = LEFT;
  } else if (state & BUTTON_A) {
    growSnake();
  } else if (state & BUTTON_B) {
    increaseSpeed();
  }
}

static void displayInfo() {
  char buffer[8];
  u32 fps = SYS_getFPS();
  sprintf(buffer, "FPS: %ld", fps);
  VDP_drawText(buffer, SCREEN_TILE_WIDTH - 8, 0);

  sprintf(buffer, "LEN: %d", _state.snake.length);
  VDP_drawText(buffer, SCREEN_TILE_WIDTH - 8, 1);

  sprintf(buffer, "SPD: %d", _state.snake.speed);
  VDP_drawText(buffer, SCREEN_TILE_WIDTH - 8, 2);

  sprintf(buffer, "S: %03d,%03d", _state.snake.segments[0].x, _state.snake.segments[0].y);
  VDP_drawText(buffer, SCREEN_TILE_WIDTH - 10, 3);

  sprintf(buffer, "F: %03d,%03d", _state.food.x, _state.food.y);
  VDP_drawText(buffer, SCREEN_TILE_WIDTH - 10, 4);
  
  sprintf(buffer, "FOOD: %d", _state.foodLeft);
  VDP_drawText(buffer, SCREEN_TILE_WIDTH - 8, 5);
}

// --- public functions
void inGame_start() {
  // Initialize variables
  _state.gameOver = FALSE;
  _state.timerTicks = 0;

  _state.snake.length = 1;
  _state.snake.speed = 1;
  _state.reqDirection = RIGHT;
  _state.reqSpeed = 1;
  _state.foodLeft = 16;

  SnakeSegment* segment = &_state.snake.segments[0];
  segment->x = SCREEN_WIDTH / 2;   // half of the screen width
  segment->y = SCREEN_HEIGHT / 2;  // half of the screen height
  segment->bx = MAX_U16;
  segment->by = MAX_U16;
  segment->direction = RIGHT;
  segment->sprite = SPR_addSprite(&snakeSegment, segment->x, segment->y, TILE_ATTR(PAL1, 0, FALSE, FALSE));

  // TODO: for now using snakeSegment as food sprite, but we should use a different sprite for food
  Food* food = &_state.food;
  food->x = (random() % (SCREEN_TILE_WIDTH - 1)) * SEGMENT_SIZE;
  food->y = (random() % (SCREEN_TILE_HEIGHT - 1)) * SEGMENT_SIZE;
  food->sprite = SPR_addSprite(&snakeSegment, food->x, food->y, TILE_ATTR(PAL1, 0, FALSE, FALSE));
  
  loadPalettes();

  // Setup a callback when a button is pressed, we could call it a "pseudo parallel" joypad handler
	JOY_setEventHandler(inGameJoyEvent);
}

void inGame_cleanUp() {
  for (int i = 0; i < _state.snake.length; i++) {
    if (_state.snake.segments[i].sprite) {
      SPR_releaseSprite(_state.snake.segments[i].sprite);
      _state.snake.segments[i].sprite = NULL;
    }
  }

  SPR_reset();
  PAL_setPalette(PAL1, NULL, CPU);
  JOY_setEventHandler(NULL);
}

enum Screen inGame_update() {
  _state.timerTicks++;

  displayInfo();
  // VDP_drawText("HERE IT'S THE GAME", 10, 0);
  // VDP_drawText("Press C to exit", 10, 1);

  checkCollisions();

  renderFood();
  updateSnake();
  renderSnake();

  SPR_update();

  if (_state.gameOver) {
    inGame_cleanUp();
    return MAIN_MENU;
  }

  return IN_GAME;
}
