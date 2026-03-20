#include "story.h"
#include "effects.h"
#include "FreeRTOS.h"
#include "task.h"

// -------------------------
// Title Screen
// -------------------------
void showTitleScreen(PmodOLED *oled)
{
    OLED_ClearBuffer(oled);
    OLED_SetCursor(oled, 0, 1);
    OLED_PutString(oled, "The Lord of the Rings");
    OLED_Update(oled);

    vTaskDelay(1500);

    dissolveScreenFast(oled);
}

// -------------------------
// Story Sequence
// -------------------------
void showStoryCards(PmodOLED *oled)
{
    OLED_ClearBuffer(oled);
    OLED_SetCursor(oled, 0, 1);
    OLED_PutString(oled, "Where am I?...");
    OLED_Update(oled);

    vTaskDelay(1500);
    dissolveScreenFast(oled);

    OLED_ClearBuffer(oled);
    OLED_SetCursor(oled, 0, 1);
    OLED_PutString(oled, "Who am I?");
    OLED_Update(oled);

    vTaskDelay(1500);
    dissolveScreenFast(oled);

    OLED_ClearBuffer(oled);
    OLED_SetCursor(oled, 0, 1);
    OLED_PutString(oled, "Ah... I'm Bilbo");
    OLED_Update(oled);

    vTaskDelay(1500);
    dissolveScreenFast(oled);

    OLED_ClearBuffer(oled);
    OLED_SetCursor(oled, 0, 1);
    OLED_PutString(oled, "Its dark in here");
    OLED_Update(oled);

    vTaskDelay(1500);
    dissolveScreenFast(oled);

    OLED_ClearBuffer(oled);
    OLED_SetCursor(oled, 0, 1);
    OLED_PutString(oled, "There must be a way out of here");
    OLED_Update(oled);

    vTaskDelay(2500);
    dissolveScreenFast(oled);

    OLED_ClearBuffer(oled);
    OLED_SetCursor(oled, 0, 1);
    OLED_PutString(oled, "Is there something I can use to see?");
    OLED_Update(oled);

    vTaskDelay(2500);
    dissolveScreenFast();

    OLED_ClearBuffer(oled);
    OLED_SetCursor(oled, 0, 1);
    OLED_PutString(oled, "Press a button to equip...");
    OLED_Update(oled);
}

