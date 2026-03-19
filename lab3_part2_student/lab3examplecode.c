#include "FreeRTOS.h"
#include "task.h"

#include "xparameters.h"
#include "xil_printf.h"

#include "PmodOLED.h"
#include <stdint.h>

#define CROSSHAIR_FRAME_DELAY_MS 40U
#define NOISE_SEED 0xA341316Cu

static PmodOLED oledDevice;
static uint32_t noiseState = NOISE_SEED;

static u8 nextNoiseBit(void);
static void setPixel(u8 xco, u8 yco, u8 color);
static void drawNoiseBackground(void);
static void redrawCrossHairNoise(u8 xco, u8 yco);
static void oledTask(void *pvParameters);

int main(void)
{
    BaseType_t taskStatus;

    OLED_Begin(&oledDevice,
               XPAR_GPIO_OLED_BASEADDR,
               XPAR_SPI_OLED_BASEADDR,
               1,
               0);

    xil_printf("OLED noise screen saver started.\r\n");

    taskStatus = xTaskCreate(oledTask,
                             "screen saver",
                             configMINIMAL_STACK_SIZE,
                             NULL,
                             tskIDLE_PRIORITY + 1,
                             NULL);

    if (taskStatus != pdPASS) {
        xil_printf("Failed to create OLED task.\r\n");
        while (1) {
        }
    }

    vTaskStartScheduler();

    while (1) {
    }

    return 0;
}

static u8 nextNoiseBit(void)
{
    noiseState ^= noiseState << 13;
    noiseState ^= noiseState >> 17;
    noiseState ^= noiseState << 5;
    return (u8)(noiseState & 0x01u);
}

static void setPixel(u8 xco, u8 yco, u8 color)
{
    OLED_SetDrawColor(&oledDevice, color);
    OLED_MoveTo(&oledDevice, xco, yco);
    OLED_DrawPixel(&oledDevice);
}

static void drawNoiseBackground(void)
{
    int x;
    int y;

    for (y = 0; y < OledRowMax; ++y) {
        for (x = 0; x < OledColMax; ++x) {
            setPixel((u8)x, (u8)y, nextNoiseBit());
        }
    }
}

static void redrawCrossHairNoise(u8 xco, u8 yco)
{
    int x;
    int y;

    for (y = 0; y < OledRowMax; ++y) {
        setPixel(xco, (u8)y, nextNoiseBit());
    }

    for (x = 0; x < OledColMax; ++x) {
        setPixel((u8)x, yco, nextNoiseBit());
    }
}

static void oledTask(void *pvParameters)
{
    int x = OledColMax / 2;
    int y = OledRowMax / 2;
    int lastX = -1;
    int lastY = -1;
    int dx = 1;
    int dy = 1;
    const TickType_t frameDelay = pdMS_TO_TICKS(CROSSHAIR_FRAME_DELAY_MS);

    (void)pvParameters;

    OLED_SetDrawMode(&oledDevice, OledModeSet);
    OLED_SetCharUpdate(&oledDevice, 0);
    drawNoiseBackground();
    OLED_Update(&oledDevice);

    while (1) {
        if ((x != lastX) || (y != lastY)) {
            redrawCrossHairNoise((u8)x, (u8)y);
            OLED_Update(&oledDevice);
            lastX = x;
            lastY = y;
        }

        x += dx;
        y += dy;

        if (x <= 0) {
            x = 0;
            dx = 1;
        } else if (x >= (OledColMax - 1)) {
            x = OledColMax - 1;
            dx = -1;
        }

        if (y <= 0) {
            y = 0;
            dy = 1;
        } else if (y >= (OledRowMax - 1)) {
            y = OledRowMax - 1;
            dy = -1;
        }

        vTaskDelay(frameDelay);
    }
}
