#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdbool.h>

typedef enum
{
    RGB_MODE_OFF = 0,
    RGB_MODE_CYAN,
    RGB_MODE_YELLOW
} RGBMode;

extern bool gHasRing;
extern bool gHasSting;

extern bool gRingDrawn;
extern bool gStingDrawn;

extern bool gRiddleCompleted;
extern RGBMode gRgbMode;

void GameState_UpdateRgbMode(void);

#endif
