#include "game_io.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "xil_printf.h"

/*
 * This queue already exists in OLEDDisplay_pot2.c and is filled by keyboardTask.
 * We reuse it here so the game engine can read complete lines without knowing
 * anything about UART details.
 */
extern QueueHandle_t xKeyboardQueue;

/* OLED owned by the platform layer, not by the game engine directly. */
static PmodOLED *gOled = NULL;

/* Simple text input buffer for one command line. */
static char gLineBuffer[64];
static size_t gLineIndex = 0;

/* -------------------------------------------------------------------------- */
/* Internal helpers                                                            */
/* -------------------------------------------------------------------------- */

static void GameIO_ClearIfReady(void)
{
    if (gOled == NULL) {
        return;
    }

    OLED_ClearBuffer(gOled);
    OLED_SetCursor(gOled, 0, 0);
}

static void GameIO_WriteWrapped(const char *text)
{
    /*
     * Minimal first-pass implementation:
     * - clears the OLED
     * - writes the supplied text starting at row 0, col 0
     * - updates the screen
     *
     * This is intentionally simple so we can get the game talking to the OLED
     * first, then improve scrolling/paging in a later pass if needed.
     */
    if ((gOled == NULL) || (text == NULL)) {
        return;
    }

    OLED_ClearBuffer(gOled);
    OLED_SetCursor(gOled, 0, 0);
    OLED_PutString(gOled, text);
    OLED_Update(gOled);
}

/* -------------------------------------------------------------------------- */
/* Public API                                                                  */
/* -------------------------------------------------------------------------- */

void GameIO_Init(PmodOLED *oled)
{
    gOled = oled;
    gLineIndex = 0;
    gLineBuffer[0] = '\0';

    if (gOled != NULL) {
        OLED_SetDrawMode(gOled, 0);
        OLED_SetCharUpdate(gOled, 0);
        OLED_ClearBuffer(gOled);
        OLED_SetCursor(gOled, 0, 0);
        OLED_Update(gOled);
    }
}

void GameIO_Clear(void)
{
    GameIO_ClearIfReady();

    if (gOled != NULL) {
        OLED_Update(gOled);
    }
}

void GameIO_Update(void)
{
    if (gOled != NULL) {
        OLED_Update(gOled);
    }
}

void GameIO_PutString(const char *text)
{
    if (text == NULL) {
        return;
    }

    GameIO_WriteWrapped(text);
    xil_printf("%s", text);
}

void GameIO_Printf(const char *fmt, ...)
{
    char buffer[256];
    va_list args;

    if (fmt == NULL) {
        return;
    }

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    GameIO_WriteWrapped(buffer);
    xil_printf("%s", buffer);
}

void GameIO_Prompt(void)
{
    /*
     * For now, prompt is shown only over UART.
     * We can later reserve a bottom OLED line for prompts if you want.
     */
    xil_printf("> ");
}

bool GameIO_GetLine(char *buffer, size_t bufferSize)
{
    char ch;

    if ((buffer == NULL) || (bufferSize == 0)) {
        return false;
    }

    if (xKeyboardQueue == NULL) {
        return false;
    }

    /*
     * Non-blocking poll. The FreeRTOS game task can call this repeatedly.
     */
    while (xQueueReceive(xKeyboardQueue, &ch, 0) == pdTRUE) {
        /* Normalize newline handling from UART */
        if ((ch == '\r') || (ch == '\n')) {
            gLineBuffer[gLineIndex] = '\0';

            /* Ignore empty lines */
            if (gLineIndex == 0) {
                continue;
            }

            strncpy(buffer, gLineBuffer, bufferSize - 1);
            buffer[bufferSize - 1] = '\0';

            gLineIndex = 0;
            gLineBuffer[0] = '\0';
            return true;
        }

        /* Handle backspace */
        if ((ch == '\b') || (ch == 127)) {
            if (gLineIndex > 0) {
                gLineIndex--;
                gLineBuffer[gLineIndex] = '\0';
            }
            continue;
        }

        /* Store normal printable characters */
        if (gLineIndex < (sizeof(gLineBuffer) - 1)) {
            gLineBuffer[gLineIndex++] = ch;
            gLineBuffer[gLineIndex] = '\0';
        }
    }

    return false;
}

void GameIO_SetStatus(const char *text)
{
    /*
     * First pass: send status over UART only.
     * Later we can split the OLED into story area + status area if desired.
     */
    if (text != NULL) {
        xil_printf("[STATUS] %s\n", text);
    }
}
