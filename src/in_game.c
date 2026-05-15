#include "in_game.h"
#include "types.h"
#include "snake.h"
#include "food.h"
#include "level.h"
#include "resources.h"

#define INACTIVE_TIME 150

enum InGameStatus {
  LEVEL_START,
  PLAYING,
  LEVEL_COMPLETE,
  GAME_OVER
};

// --- private state shape
typedef struct {
  bool gameOver;
  Snake snake;
  Food food;
  u8 foodLeft;
  u16 timerTicks;
  enum SnakeDirection reqDirection;
  u8 reqSpeed;
  Level level;
  enum InGameStatus status;
  u16 score;
} InGameState;

// --- private state
static InGameState _state;

static void loadPalettes() {
  PAL_setPalette(PAL1, sprSnakeSegment.palette->data, CPU);
  food_updatePalette(&_state.food);
}

static void increaseGameSpeed() {
  switch (_state.snake.speed) {
    case 1:
      _state.reqSpeed = 2;
      break;
    case 2:
      _state.reqSpeed = 4;
      break;
    default:
      _state.reqSpeed = 4;  // cap the speed at 4
      break;
  }
}

static void nextLevel(u8 levelNumber) {
  snake_cleanUp(&_state.snake);
  food_cleanUp(&_state.food);

  level_init(&_state.level, levelNumber);
  
  _state.reqSpeed = _state.level.initialSpeed;
  _state.reqDirection = RIGHT;
  _state.foodLeft = _state.level.foodCount;
  _state.timerTicks = 0;
  _state.status = LEVEL_START;
  
  snake_init(&_state.snake, _state.level.initialSegments, _state.level.initialSpeed);
  food_init(&_state.food);
}

static void checkCollisions() {
  SnakeSegment *head = &_state.snake.segments[0];

  // Check if the snake head collides with the food
  if (head->x >= _state.food.x && head->x < _state.food.x + SEGMENT_SIZE &&
      head->y >= _state.food.y && head->y < _state.food.y + SEGMENT_SIZE) {
    snake_grow(&_state.snake);

    _state.score += 10 * _state.snake.speed; // increase score based on the current speed
    _state.foodLeft--;
    
    // Move food to a new random position
    _state.food.x = (random() % (SCREEN_TILE_WIDTH - 1)) * SEGMENT_SIZE;
    _state.food.y = (random() % (SCREEN_TILE_HEIGHT - 1)) * SEGMENT_SIZE;

    if ((_state.level.foodCount - _state.foodLeft) % _state.level.foodStep == 0) {
      increaseGameSpeed(); // increase speed every N food eaten
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

static void clearCenteredMessage() {
  VDP_clearTextArea((SCREEN_TILE_WIDTH / 2) - 10, (SCREEN_TILE_HEIGHT / 2) - 1, 20, 2);
}

static void inGameJoyEvent(u16 joy, u16 changed, u16 state) {
  if (state & BUTTON_C) {
    clearCenteredMessage();
    _state.status = GAME_OVER;
  } else if (state & BUTTON_UP) {
    _state.reqDirection = UP;
  } else if (state & BUTTON_RIGHT) {
    _state.reqDirection = RIGHT;
  } else if (state & BUTTON_DOWN) {
    _state.reqDirection = DOWN;
  } else if (state & BUTTON_LEFT) {
    _state.reqDirection = LEFT;
  }
}

static void displayInfo() {
  char buffer[8];
  u32 fps = SYS_getFPS();
  sprintf(buffer, "FPS %ld", fps);
  VDP_drawText(buffer, SCREEN_TILE_WIDTH - 6, 0);

  sprintf(buffer, "SCORE %04d", _state.score);
  VDP_drawText(buffer, (SCREEN_TILE_WIDTH / 2) - 5, 0);

  // sprintf(buffer, "SC: %03d,%03d", _state.snake.segments[0].x, _state.snake.segments[0].y);
  // sprintf(buffer, "TT: %05d", _state.timerTicks);
  // VDP_drawText(buffer, SCREEN_TILE_WIDTH - 9, 3);

  sprintf(buffer, "LEVEL %02d", _state.level.number);
  VDP_drawText(buffer, 0, 0);

  sprintf(buffer, "BONUS %04d", _state.level.bonus);
  VDP_drawText(buffer, 0, 1);
  
  sprintf(buffer, "FOOD  %02d", _state.foodLeft);
  VDP_drawText(buffer, 0, 2);

  // Display the level number in the center of the screen at the start of each level
  if (_state.status == LEVEL_START) {
    sprintf(buffer, "LEVEL %02d", _state.level.number);
    VDP_drawText(buffer, (SCREEN_TILE_WIDTH / 2) - 4, (SCREEN_TILE_HEIGHT / 2) - 1);
  } if (_state.status == LEVEL_COMPLETE) {
    sprintf(buffer, "LEVEL %02d COMPLETED!", _state.level.number);
    VDP_drawText(buffer, (SCREEN_TILE_WIDTH / 2) - 10, (SCREEN_TILE_HEIGHT / 2) - 1);

    sprintf(buffer, "BONUS %04d", _state.level.bonus);
    VDP_drawText(buffer, (SCREEN_TILE_WIDTH / 2) - 5, (SCREEN_TILE_HEIGHT / 2));
  } else if (_state.status == GAME_OVER) {
    sprintf(buffer, "GAME OVER!");
    VDP_drawText(buffer, (SCREEN_TILE_WIDTH / 2) - 5, (SCREEN_TILE_HEIGHT / 2) - 1);
  }
}

void updateStatus() {
  if (_state.status == LEVEL_START && _state.timerTicks > INACTIVE_TIME) {
    clearCenteredMessage();
    _state.status = PLAYING;
  } else if (_state.status == PLAYING && _state.foodLeft == 0) {
    clearCenteredMessage();
    _state.status = LEVEL_COMPLETE;
    _state.timerTicks = 0; // reset timer for the level complete message
    _state.score += _state.level.bonus; // add level bonus to score
  } else if (_state.status == LEVEL_COMPLETE && _state.timerTicks > INACTIVE_TIME) {
    clearCenteredMessage();
    if (_state.level.number == MAX_LEVELS) {
      // TODO: maybe show a "you win" screen instead of going back to the main menu?
      _state.status = GAME_OVER;
      _state.timerTicks = 0; // reset timer for the game over message
      return;
    }

    nextLevel(_state.level.number + 1); // Start next level
  } else if (_state.status == GAME_OVER && _state.timerTicks > INACTIVE_TIME) {
    _state.gameOver = TRUE;
  }
}

// --- public functions
void inGame_start() {
  _state.gameOver = FALSE;
  _state.score = 0;

  nextLevel(1);

  loadPalettes();

  // Setup a callback when a button is pressed, we could call it a "pseudo parallel" joypad handler
	JOY_setEventHandler(inGameJoyEvent);
}

void inGame_cleanUp() {
  snake_cleanUp(&_state.snake);
  food_cleanUp(&_state.food);

  SPR_reset();
  PAL_setPalette(PAL1, NULL, CPU);
  PAL_setPalette(PAL2, NULL, CPU);
  JOY_setEventHandler(NULL);
}

enum Screen inGame_update() {
  _state.timerTicks++;

  displayInfo();

  checkCollisions();

  updateStatus();

  if (_state.foodLeft > 0) {
    food_render(&_state.food);
  }
  
  if (_state.status == PLAYING) {
    if (_state.timerTicks % (60 * 5) == 0) { // decrease bonus every 5 seconds
      level_decreaseBonus(&_state.level, _state.snake.speed); // decrease bonus based on the current speed
    }

    if (_state.timerTicks % (60 / 2) == 0) { // swap pallete every half second
      food_updatePalette(&_state.food);
    }

    snake_updateSpeed(&_state.snake, _state.reqSpeed);
    snake_updateDirection(&_state.snake, _state.reqDirection);
    snake_move(&_state.snake);
  }
  
  snake_render(&_state.snake);
  
  SPR_update();

  if (_state.gameOver) {
    inGame_cleanUp();
    return MAIN_MENU;
  }

  return IN_GAME;
}
