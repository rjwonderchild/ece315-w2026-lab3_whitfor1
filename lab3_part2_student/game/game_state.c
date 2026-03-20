#include "game_state.h"

bool gHasRing = false;
bool gHasSting = false;

bool gRingDrawn = false;
bool gStingDrawn = false;

bool gRiddleCompleted = false;
RGBMode gRgbMode = RGB_MODE_OFF;

void GameState_UpdateRgbMode(void)
{
    if (gRingDrawn)
    {
        gRgbMode = RGB_MODE_YELLOW;
    }
    else if (gStingDrawn)
    {
        gRgbMode = RGB_MODE_CYAN;
    }
    else
    {
        gRgbMode = RGB_MODE_OFF;
    }
}
