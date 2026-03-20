#include "game_io.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "xil_printf.h"

/*
 * This queue is filled elsewhere by your keyboard/UART task.
 * We reuse it here so the game engine can read one complete line at a time.
 */
extern QueueHandle_t xKeyboardQueue;

/* 128x32 OLED with the usual 8x8 font -> 16 chars x 4 rows */
#define OLED_COLS            16
#define OLED_ROWS             4
#define INPUT_BUFFER_SIZE    64
#define PRINT_BUFFER_SIZE   256

static PmodOLED *gOled = NULL;

/* Rolling 4-line display buffer */
static char gDisplay[OLED_ROWS][OLED_COLS + 1];

/* Current cursor position within the rolling text console */
static int gRow = 0;
static int gCol = 0;

/* Simple command-line input buffer */
static char gLineBuffer[INPUT_BUFFER_SIZE];
static size_t gLineIndex = 0;

/* -------------------------------------------------------------------------- */
/* Internal helpers                                                            */
/* -------------------------------------------------------------------------- */

static void clearDisplayBuffer(void)
{
    int r;

    for (r = 0; r < OLED_ROWS; r++)
    {
        memset(gDisplay[r], ' ', OLED_COLS);
        gDisplay[r][OLED_COLS] = '\0';
    }

    gRow = 0;
    gCol = 0;
}

static void scrollDisplayUp(void)
{
    int r;

    for (r = 0; r < OLED_ROWS - 1; r++)
    {
        memcpy(gDisplay[r], gDisplay[r + 1], OLED_COLS + 1);
    }

    memset(gDisplay[OLED_ROWS - 1], ' ', OLED_COLS);
    gDisplay[OLED_ROWS - 1][OLED_COLS] = '\0';

    gRow = OLED_ROWS - 1;
    gCol = 0;
}

static void advanceLine(void)
{
    gRow++;
    gCol = 0;

    if (gRow >= OLED_ROWS)
    {
        scrollDisplayUp();
    }
}

static void putCharToDisplay(char ch)
{
    if (ch == '\r')
    {
        return;
    }

    if (ch == '\n')
    {
        advanceLine();
        return;
    }

    if (ch == '\t')
    {
        int spaces = 4 - (gCol % 4);
        while (spaces-- > 0)
        {
            putCharToDisplay(' ');
        }
        return;
    }

    if (gCol >= OLED_COLS)
    {
        advanceLine();
    }

    gDisplay[gRow][gCol++] = ch;
}

static void putWrappedText(const char *text)
{
    const char *p = text;

    while (p != NULL && *p != '\0')
    {
        /* Preserve explicit newlines */
        if (*p == '\n')
        {
            putCharToDisplay('\n');
            p++;
            continue;
        }

        /* Collapse carriage returns */
        if (*p == '\r')
        {
            p++;
            continue;
        }

        /* Preserve spaces as typed */
        if (isspace((unsigned char)*p))
        {
            putCharToDisplay(*p);
            p++;
            continue;
        }

        /* Measure next word length */
        {
            const char *wordStart = p;
            int wordLen = 0;

            while (p[wordLen] != '\0' &&
                   p[wordLen] != '\n' &&
                   p[wordLen] != '\r' &&
                   !isspace((unsigned char)p[wordLen]))
            {
                wordLen++;
            }

            /*
             * If the word does not fit on this line and it would fit on a fresh
             * line, wrap before printing it.
             */
            if (gCol > 0 && wordLen <= OLED_COLS && (gCol + wordLen) > OLED_COLS)
            {
                advanceLine();
            }

            /*
             * Print the word. If the word itself is longer than one line, it
             * will naturally continue across lines character by character.
             */
            while (wordLen-- > 0)
            {
                putCharToDisplay(*wordStart++);
            }

            p = wordStart;
        }
    }
}

static void renderDisplay(void)
{
    int r;

    if (gOled == NULL)
    {
        return;
    }

    OLED_ClearBuffer(gOled);

    for (r = 0; r < OLED_ROWS; r++)
    {
        OLED_SetCursor(gOled, 0, r);
        OLED_PutString(gOled, gDisplay[r]);
    }

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

    clearDisplayBuffer();

    if (gOled != NULL)
    {
        OLED_SetDrawMode(gOled, 0);
        OLED_SetCharUpdate(gOled, 0);
        renderDisplay();
    }
}

void GameIO_Clear(void)
{
    clearDisplayBuffer();
    renderDisplay();
}

void GameIO_Update(void)
{
    renderDisplay();
}

void GameIO_PutString(const char *text)
{
    if (text == NULL)
    {
        return;
    }

    putWrappedText(text);
    renderDisplay();
    xil_printf("%s", text);
}

void GameIO_Printf(const char *fmt, ...)
{
    char buffer[PRINT_BUFFER_SIZE];
    va_list args;

    if (fmt == NULL)
    {
        return;
    }

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    putWrappedText(buffer);
    renderDisplay();
    xil_printf("%s", buffer);
}

void GameIO_Prompt(void)
{
    /*
     * Keep the visible prompt on UART only for now.
     * The OLED is too small to constantly reserve a full row for prompts.
     */
    xil_printf("> ");
}

bool GameIO_GetLine(char *buffer, size_t bufferSize)
{
    char ch;

    if (buffer == NULL || bufferSize == 0)
    {
        return false;
    }

    if (xKeyboardQueue == NULL)
    {
        return false;
    }

    while (xQueueReceive(xKeyboardQueue, &ch, 0) == pdTRUE)
    {
        if (ch == '\r' || ch == '\n')
        {
            gLineBuffer[gLineIndex] = '\0';

            if (gLineIndex == 0)
            {
                continue;
            }

            strncpy(buffer, gLineBuffer, bufferSize - 1);
            buffer[bufferSize - 1] = '\0';

            gLineIndex = 0;
            gLineBuffer[0] = '\0';
            return true;
        }

        if (ch == '\b' || ch == 127)
        {
            if (gLineIndex > 0)
            {
                gLineIndex--;
                gLineBuffer[gLineIndex] = '\0';
            }
            continue;
        }

        if (isprint((unsigned char)ch) && gLineIndex < (sizeof(gLineBuffer) - 1))
        {
            gLineBuffer[gLineIndex++] = ch;
            gLineBuffer[gLineIndex] = '\0';
        }
    }

    return false;
}

void GameIO_SetStatus(const char *text)
{
    /*
     * First useful embedded version:
     * clear the screen and show the status text as a short 4-line message.
     * This is good for splash/status screens but should be used sparingly
     * during gameplay because it replaces the rolling text view.
     */
    if (text == NULL)
    {
        return;
    }

    clearDisplayBuffer();
    putWrappedText(text);
    renderDisplay();
    xil_printf("[STATUS] %s\n", text);
}
