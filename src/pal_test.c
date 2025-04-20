#include "pal_test.h"
#include "types.h"

// --- private state shape
typedef struct {
    int pal;
    int color;
    u16 startColor;
    enum Screen nextScreen;
    char buffer[16];
    char bufferColor[11];
    char bufferOffsetColor[17];
    char bufferTextColor[16];
    u16 palettesData[4][16];
    u16 defaultPalettesData[4][16];
} PalTestState;

// --- private state
static PalTestState _state;

// --- private functions
static void initPalettes(u16 startColor) {
  u16 color = startColor;

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 16; j++) {
      _state.palettesData[i][j] = color++;
    }
  }

  _state.palettesData[0][0] = 0x000;
}

static void saveDefaultPalettes() {
  for (int i = 0; i < 4; i++) {
    PAL_getPalette(i, _state.defaultPalettesData[i]);
  }
}

static void renderPalettes() {
  // Load palettes
  for (int i = 0; i < 4; i++) {
    PAL_setPalette(i, _state.palettesData[i], CPU);

    for (int j = 0; j < 16; j++) {
      VDP_setTileMapXY(VDP_BG_B, TILE_ATTR_FULL(i, 1, 0, 0, j), j * 2, (i * 2) + 6);
    }
  }
}

static void restorePalettes() {
  for (int i = 0; i < 4; i++) {
    PAL_setPalette(i, _state.defaultPalettesData[i], CPU);
  }
}

static void inGameJoyEvent(u16 joy, u16 changed, u16 state) {
  if (state & BUTTON_UP) {
    _state.pal = _state.pal == 3 ? 0 : _state.pal + 1;
  } else if (state & BUTTON_DOWN) {
    _state.pal = _state.pal == 0 ? 3 : _state.pal - 1;
  } else if (state & BUTTON_RIGHT) {
    _state.color = _state.color == 15 ? 0 : _state.color + 1;
  } else if (state & BUTTON_LEFT) {
    _state.color = _state.color == 0 ? 15 : _state.color - 1;
  } else if (state & BUTTON_A) {
    _state.startColor = _state.startColor == 4095 ? 0 : _state.startColor + 1;
    initPalettes(_state.startColor);
    renderPalettes();
  } else if (state & BUTTON_B) {
    _state.startColor = _state.startColor == 0 ? 4095 : _state.startColor - 1;
    initPalettes(_state.startColor);
    renderPalettes();
  } else if (state & BUTTON_C) {
    // restore palettes
    restorePalettes();
    _state.nextScreen = MAIN_MENU;
    return;
  }

  palTest_update();
}

static void displayFps() {
  char buffer[8];
  u32 fps = SYS_getFPS();
  sprintf(buffer, "FPS: %ld", fps);
  VDP_drawText(buffer, SCREEN_TILE_WIDTH - 8, 0);
}

static void loadTiles() {
  u32 tileData[16][8];
  for (u32 i = 0; i < 16; i++) {
    u32 rowValue = 0x11111111 * i;
    for (int j = 0; j < 8; j++) {
      tileData[i][j] = rowValue;
    }
  }

  // load 16 tiles and assign indexes starting from 0
  VDP_loadTileData((const u32 *)tileData, 0, 16, CPU);
}

// --- public functions

void palTest_start() {
  _state.pal = 0;
  _state.color = 0;
  _state.startColor = 0x000;
  _state.nextScreen = PAL_TEST;

  saveDefaultPalettes();

  // Setup a callback when a button is pressed, we could call it a "pseudo parallel" joypad handler
	JOY_setEventHandler(inGameJoyEvent);
  
  loadTiles();
  initPalettes(_state.startColor);
  renderPalettes();
}

enum Screen palTest_update() {
  displayFps();

  sprintf(_state.buffer, "PAL: %02d COL: %02d", _state.pal, _state.color);
  sprintf(_state.bufferColor, "COLOR: %03x", _state.palettesData[_state.pal][_state.color]);
  sprintf(_state.bufferOffsetColor, "START COLOR: %03x", _state.startColor);
  sprintf(_state.bufferTextColor, "TEXT COLOR: %03x", _state.palettesData[0][0xf]);
  VDP_drawText(_state.buffer, SCREEN_TILE_WIDTH - 16, 1);
  VDP_drawText(_state.bufferColor, SCREEN_TILE_WIDTH - 11, 2);
  VDP_drawText(_state.bufferOffsetColor, SCREEN_TILE_WIDTH - 17, 3);
  VDP_drawText(_state.bufferTextColor, SCREEN_TILE_WIDTH - 16, 4);

  return _state.nextScreen;
}
