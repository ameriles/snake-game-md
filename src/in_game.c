#include "in_game.h"
#include "types.h"
#include "resources.h"

#define SNAKE_TILE 1
#define SNAKE_MAX_LENGTH 20
#define SCREEN_TILE_WIDTH 40
#define SCREEN_TILE_HEIGHT 28
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 224
#define SEGMENT_WIDTH 8

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
} SnakeSegment;

typedef struct {
  SnakeSegment segments[SNAKE_MAX_LENGTH];
  u8 length;
  u8 speed;
} Snake;

// --- private state shape
typedef struct {
  bool gameOver;
  Snake snake;
  u16 timerTicks;
  enum SnakeDirection reqDirection;
} InGameState;

// --- private state
static InGameState _state;

static void loadPalettes() {
  PAL_setPalette(PAL1, snakeSegment.palette->data, CPU);
}

static void advanceSnake() {
  for (int i = 0; i < _state.snake.length; i++) {
    enum SnakeDirection direction = _state.snake.segments[i].direction;
    if (direction == UP) {
      _state.snake.segments[i].y = _state.snake.segments[i].y == 0 ? SCREEN_HEIGHT - 1 : _state.snake.segments[i].y - 1;
    } else if (direction == RIGHT) {
      _state.snake.segments[i].x = _state.snake.segments[i].x == SCREEN_WIDTH - 1 ? 0 : _state.snake.segments[i].x + 1;
    } else if (direction == DOWN) {
      _state.snake.segments[i].y = _state.snake.segments[i].y == SCREEN_HEIGHT - 1 ? 0 : _state.snake.segments[i].y + 1;
    } else if (direction == LEFT) {
      _state.snake.segments[i].x = _state.snake.segments[i].x == 0 ? SCREEN_WIDTH - 1 : _state.snake.segments[i].x - 1;
    } 
  }
}

static void updateDirections() {
  for (int i = _state.snake.length - 1; i >= 0; i--) {
    SnakeSegment *segment = &_state.snake.segments[i];
    if (segment->x % SEGMENT_WIDTH != 0 || segment->y % SEGMENT_WIDTH != 0) {
      continue;
    }

    if (i == 0) { // the head
      segment->direction = _state.reqDirection;
    } else {
      SnakeSegment *prevSegment = &_state.snake.segments[i - 1];
      segment->direction = prevSegment->direction;
    }
  }
}

static void updateSnake() {
  // if (_state.timerTicks % 15 != 0) {
  //  return;
  // }

  updateDirections();

  advanceSnake();
}

static void renderSnake() {
  for (int i = 0; i < _state.snake.length; i++) {
    SnakeSegment *segment = &_state.snake.segments[i];
    SPR_setPosition(segment->sprite, segment->x, segment->y);
  }

  SPR_update();
}

static void growSnake() {
  if (_state.snake.length + 1 > SNAKE_MAX_LENGTH) {
    return;
  }

  u16 xOffset = 0;
  u16 yOffset = 0;
  int i = _state.snake.length;  // the new segment
  enum SnakeDirection direction = _state.snake.segments[i - 1].direction;
  if (direction == UP) {
    yOffset = SEGMENT_WIDTH;
  } else if (direction == RIGHT) {
    xOffset = -SEGMENT_WIDTH;
  } else if (direction == DOWN) {
    yOffset = -SEGMENT_WIDTH;
  } else if (direction == LEFT) {
    xOffset = SEGMENT_WIDTH;
  }

  _state.snake.segments[i].x = _state.snake.segments[i - 1].x + xOffset;
  _state.snake.segments[i].y = _state.snake.segments[i - 1].y + yOffset;
  _state.snake.segments[i].direction = _state.snake.segments[i - 1].direction;

  _state.snake.segments[i].sprite = SPR_addSprite(&snakeSegment, _state.snake.segments[i].x, _state.snake.segments[i].y, TILE_ATTR(PAL1, 0, FALSE, FALSE));
  _state.snake.length++;

  advanceSnake();
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
  }
}

static void displayInfo() {
  char buffer[8];
  u32 fps = SYS_getFPS();
  sprintf(buffer, "FPS: %ld", fps);
  VDP_drawText(buffer, SCREEN_TILE_WIDTH - 8, 0);

  sprintf(buffer, "LEN: %d", _state.snake.length);
  VDP_drawText(buffer, SCREEN_TILE_WIDTH - 8, 1);
}

// --- public functions
void inGame_start() {
  // Initialize variables
  _state.gameOver = FALSE;
  _state.timerTicks = 0;

  _state.snake.length = 1;
  _state.snake.speed = 1;
  _state.reqDirection = RIGHT;

  SnakeSegment* segment = &_state.snake.segments[0];
  segment->x = SCREEN_WIDTH / 2;   // half of the screen width
  segment->y = SCREEN_HEIGHT / 2;  // half of the screen height
  segment->direction = RIGHT;
  segment->sprite = SPR_addSprite(&snakeSegment, segment->x, segment->y, TILE_ATTR(PAL1, 0, FALSE, FALSE));

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
  VDP_drawText("HERE IT'S THE GAME", 10, 0);
  VDP_drawText("Press C to exit", 10, 1);

  updateSnake();
  renderSnake();

  if (_state.gameOver) {
    inGame_cleanUp();
    return MAIN_MENU;
  }

  return IN_GAME;
}
