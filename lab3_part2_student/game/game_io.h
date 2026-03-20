#ifndef GAME_IO_H
#define GAME_IO_H

#include <stddef.h>
#include <stdbool.h>

#ifdef EMBEDDED_BUILD
#include "PmodOLED.h"
#else
typedef struct PmodOLED PmodOLED;
#endif

void GameIO_Init(PmodOLED *oled);
void GameIO_Clear(void);
void GameIO_Update(void);
void GameIO_PutString(const char *text);
void GameIO_Printf(const char *fmt, ...);
void GameIO_Prompt(void);
bool GameIO_GetLine(char *buffer, size_t bufferSize);
void GameIO_SetStatus(const char *text);

#endif
