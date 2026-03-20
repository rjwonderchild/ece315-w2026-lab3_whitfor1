#include "effects.h"
#include "FreeRTOS.h"
#include "task.h"

// -------------------------
// Fast Random Generator
// -------------------------
static u32 lfsrFast = 0xA341316C;

static u8 nextNoiseBitFast(void) {
    lfsrFast ^= lfsrFast << 13;
    lfsrFast ^= lfsrFast >> 17;
    lfsrFast ^= lfsrFast << 5;
    return (u8)(lfsrFast & 0xFF);
}

// -------------------------
// Dissolve Effect
// -------------------------
void dissolveScreenFast(PmodOLED *oled)
{
    int i, j;
    int x, y;

    for (i = 0; i < 200; i++) {
        for (j = 0; j < 110; j++) {
            x = nextNoiseBitFast() % OledColMax;
            y = nextNoiseBitFast() % OledRowMax;

            OLED_SetDrawColor(oled, 0);
            OLED_MoveTo(oled, x, y);
            OLED_DrawPixel(oled);
        }

        OLED_Update(oled);
        vTaskDelay(1);
    }

    OLED_Update(oled);
}