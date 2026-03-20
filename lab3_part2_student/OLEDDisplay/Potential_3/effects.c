#include "effects.h"
#include "PmodOLED.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdlib.h>

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

void showTextWithDissolve(PmodOLED *oled, char *text, int delayMs)
{
    OLED_ClearBuffer(oled);
    OLED_SetCursor(oled, 0, 1);
    OLED_PutString(oled, text);
    OLED_Update(oled);

    vTaskDelay(delayMs);

    dissolveScreenFast(oled);
}

// Dynamically set OLED invert mode
void setOLEDInvert(PmodOLED *oled, u8 *invert)
{
    // Reinitialize OLED with updated invert
    OLED_Begin(oled, XPAR_GPIO_OLED_BASEADDR, XPAR_SPI_OLED_BASEADDR, 0x1, *invert);
    OLED_ClearBuffer(oled);
    OLED_Update(oled);
}