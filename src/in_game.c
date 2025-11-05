#include "in_game.h"
#include "types.h"
#include "snake.h"
#include "food.h"
#include "resources.h"

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
  PAL_setPalette(PAL1, sprSnakeSegment.palette->data, CPU);
}

static void increaseGameSpeed() {
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

static void checkCollisions() {
  SnakeSegment *head = &_state.snake.segments[0];

  // Check if the snake head collides with the food
  if (head->x >= _state.food.x && head->x < _state.food.x + SEGMENT_SIZE &&
      head->y >= _state.food.y && head->y < _state.food.y + SEGMENT_SIZE) {
    snake_grow(&_state.snake);

    _state.foodLeft--;
    
    // Move food to a new random position
    _state.food.x = (random() % (SCREEN_TILE_WIDTH - 1)) * SEGMENT_SIZE;
    _state.food.y = (random() % (SCREEN_TILE_HEIGHT - 1)) * SEGMENT_SIZE;

    if (_state.foodLeft == 0) {
      _state.gameOver = TRUE;
      return;
    }

    if (_state.foodLeft % 4 == 0) {
      increaseGameSpeed(); // increase speed every 4 food eaten
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
    snake_grow(&_state.snake);
  } else if (state & BUTTON_B) {
    increaseGameSpeed();
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

  sprintf(buffer, "SC: %03d,%03d", _state.snake.segments[0].x, _state.snake.segments[0].y);
  VDP_drawText(buffer, SCREEN_TILE_WIDTH - 11, 3);

  sprintf(buffer, "FC: %03d,%03d", _state.food.x, _state.food.y);
  VDP_drawText(buffer, SCREEN_TILE_WIDTH - 11, 4);
  
  sprintf(buffer, "FOOD: %02d", _state.foodLeft);
  VDP_drawText(buffer, SCREEN_TILE_WIDTH - 8, 5);
}

// --- public functions
void inGame_start() {
  _state.gameOver = FALSE;
  _state.timerTicks = 0;

  _state.reqDirection = RIGHT;
  _state.reqSpeed = 1;
  _state.foodLeft = 16;

  snake_init(&_state.snake);
  food_init(&_state.food);

  loadPalettes();

  // Setup a callback when a button is pressed, we could call it a "pseudo parallel" joypad handler
	JOY_setEventHandler(inGameJoyEvent);
}

void inGame_cleanUp() {
  snake_cleanUp(&_state.snake);
  food_cleanUp(&_state.food);

  SPR_reset();
  PAL_setPalette(PAL1, NULL, CPU);
  JOY_setEventHandler(NULL);
}

enum Screen inGame_update() {
  _state.timerTicks++;

  displayInfo();

  checkCollisions();

  if (_state.foodLeft > 0) {
    food_render(&_state.food);
  }
  
  snake_updateSpeed(&_state.snake, _state.reqSpeed);
  snake_updateDirection(&_state.snake, _state.reqDirection);
  snake_move(&_state.snake);
  snake_render(&_state.snake);

  SPR_update();

  if (_state.gameOver) {
    inGame_cleanUp();
    return MAIN_MENU;
  }

  return IN_GAME;
}
