#include "in_game.h"
#include "types.h"

#define SNAKE_TILE 1
#define SNAKE_MAX_LENGTH 100
#define SCREEN_TILE_WIDTH 40
#define SCREEN_TILE_HEIGHT 28

enum SnakeDirection {
  UP,
  RIGHT,
  DOWN,
  LEFT
};

typedef struct {
  u8 x;
  u8 y;
} SnakeSegment;

typedef struct {
  SnakeSegment segments[SNAKE_MAX_LENGTH];
  u8 length;
  u8 speed;
  enum SnakeDirection direction;
} Snake;

// --- private state shape
typedef struct {
  bool gameOver;
  Snake snake;
  u16 timerTicks;
} InGameState;

// --- private state
static InGameState _state;

// --- private functions
static void loadTiles() {
  // A 8x8 circle tile using 0 for transparency and 1 and 2 for the color gradients
  // The pattern is expressed in 32 bits, 4 bits per pixel (hexadecimal)
  u32 tileData[8] = {
    0x00111100,
    0x01222210,
    0x12233221,
    0x12344321,
    0x12344321,
    0x12233221,
    0x01222210,
    0x00111100
  };

  VDP_loadTileData((const u32 *)tileData, SNAKE_TILE, 1, CPU);
}

static void loadPalettes() {
  u16 palette[16] = {
    0x000, 0x080, 0x0a0, 0x0c0,
    0x0f0, 0xfff, 0xfff, 0xfff,
    0xfff, 0xfff, 0xfff, 0xfff,
    0xfff, 0xfff, 0xfff, 0xfff,
  };

  PAL_setPalette(PAL0, palette, CPU);
}

static void advanceSnake() {
  // Clear the tilemap for the snake segments and copy the segments into the next segment
  for (int i = 0; i < _state.snake.length; i++) {
    SnakeSegment segment = _state.snake.segments[i];
    VDP_clearTileMapRect(VDP_BG_A, segment.x, segment.y, 1, 1);
  }  

  for (int i = _state.snake.length - 1; i > 0; i--) {
    _state.snake.segments[i] = _state.snake.segments[i - 1];
  }

  enum SnakeDirection direction = _state.snake.direction;
  SnakeSegment segment = _state.snake.segments[0];
  if (direction == UP) {
    segment.y = segment.y == 0 ? SCREEN_TILE_HEIGHT - 1 : segment.y - 1;
  } else if (direction == RIGHT) {
    segment.x = segment.x == SCREEN_TILE_WIDTH - 1 ? 0 : segment.x + 1;
  } else if (direction == DOWN) {
    segment.y = segment.y == SCREEN_TILE_HEIGHT - 1 ? 0 : segment.y + 1;
  } else if (direction == LEFT) {
    segment.x = segment.x == 0 ? SCREEN_TILE_WIDTH - 1 : segment.x - 1;
  }

  _state.snake.segments[0] = segment;
}

static void updateSnake() {
  if (_state.timerTicks % 15 != 0) {
    return;
  }

  advanceSnake();
}

static void renderSnake() {
  // TODO: optimize this to use a single call
  for (int i = 0; i < _state.snake.length; i++) {
    SnakeSegment segment = _state.snake.segments[i];
    VDP_setTileMapXY(VDP_BG_A, TILE_ATTR_FULL(PAL0, 0, 0, 0, SNAKE_TILE), segment.x, segment.y);  
  }
}

static void growSnake() {
  if (_state.snake.length + 1 > SNAKE_MAX_LENGTH) {
    return;
  }

  _state.snake.segments[_state.snake.length] = _state.snake.segments[_state.snake.length - 1];
  _state.snake.length++;

  advanceSnake();
}

static void inGameJoyEvent(u16 joy, u16 changed, u16 state) {
  if (state & BUTTON_C) {
    _state.gameOver = TRUE;
  } else if (state & BUTTON_UP) {
    _state.snake.direction = UP;
  } else if (state & BUTTON_RIGHT) {
    _state.snake.direction = RIGHT;
  } else if (state & BUTTON_DOWN) {
    _state.snake.direction = DOWN;
  } else if (state & BUTTON_LEFT) {
    _state.snake.direction = LEFT;
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
  _state.snake.segments[0].x = SCREEN_TILE_WIDTH / 2; // half of the screen width
  _state.snake.segments[0].y = SCREEN_TILE_HEIGHT / 2; // half of the screen height
  _state.snake.speed = 1;
  _state.snake.direction = RIGHT;

  loadTiles();
  loadPalettes();

  // Setup a callback when a button is pressed, we could call it a "pseudo parallel" joypad handler
	JOY_setEventHandler(inGameJoyEvent);
}

enum Screen inGame_update() {
  _state.timerTicks++;

  displayInfo();
  VDP_drawText("HERE IT'S THE GAME", 10, 0);
  VDP_drawText("Press C to exit", 10, 1);

  updateSnake();
  renderSnake();

  return _state.gameOver ? MAIN_MENU : IN_GAME;
}
