#ifndef GAME_IO_H
#define GAME_IO_H

#include <stddef.h>
#include <stdbool.h>
#include "PmodOLED.h"

/*
 * Central I/O abstraction for the text adventure.
 * All game modules should use these functions instead of printf(),
 * fgets(), or direct OLED access.
 */

/* Initialize the game I/O layer with the OLED instance used by the system. */
void GameIO_Init(PmodOLED *oled);

/* Clear the current output area / screen. */
void GameIO_Clear(void);

/* Flush any buffered output to the display. */
void GameIO_Update(void);

/* Print one string exactly as given. */
void GameIO_PutString(const char *text);

/* Print formatted text like printf. */
void GameIO_Printf(const char *fmt, ...);

/* Print a prompt, such as "> ". */
void GameIO_Prompt(void);

/*
 * Poll for one complete input line from the platform.
 * Returns true when a line is ready and copied into buffer.
 * Returns false when no complete line is available yet.
 */
bool GameIO_GetLine(char *buffer, size_t bufferSize);

/*
 * Optional helper to show status / short messages in a reserved area.
 * You can leave this stubbed at first if you want.
 */
void GameIO_SetStatus(const char *text);

#endif /* GAME_IO_H */
