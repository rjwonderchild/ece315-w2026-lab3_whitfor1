#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include "PmodOLED.h"

void gameInit(void);
void gameShowIntro(PmodOLED *oled);
bool gameProcessCommand(const char *command);
bool gamePollAndProcess(void);

#endif
