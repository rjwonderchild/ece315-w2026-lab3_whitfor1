#ifndef SRC_EFFECTS_H_
#define SRC_EFFECTS_H_

#include "PmodOLED.h"

void dissolveScreenFast(PmodOLED *oled);
void showTextWithDissolve(PmodOLED *oled, char *text, int delayMs);
void setOLEDInvert(PmodOLED *oled, u8 *invert);

#endif /* SRC_EFFECTS_H_ */