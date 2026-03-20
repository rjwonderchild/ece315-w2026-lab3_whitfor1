#include "story.h"
#include "effects.h"
#include "FreeRTOS.h"
#include "task.h"

#define STORY_SHORT_DELAY_MS   1500
#define STORY_LONG_DELAY_MS    2500

static void showCard(PmodOLED *oled, const char *line1, const char *line2, TickType_t delayMs)
{
    if (oled == NULL)
    {
        return;
    }

    OLED_ClearBuffer(oled);
    OLED_SetCursor(oled, 0, 1);
    OLED_PutString(oled, line1);

    if (line2 != NULL)
    {
        OLED_SetCursor(oled, 0, 2);
        OLED_PutString(oled, line2);
    }

    OLED_Update(oled);
    vTaskDelay(pdMS_TO_TICKS(delayMs));
    dissolveScreenFast(oled);
}

void showTitleScreen(PmodOLED *oled)
{
    showCard(oled, "The Lord of the Rings", NULL, STORY_SHORT_DELAY_MS);
}

void showStoryCards(PmodOLED *oled)
{
    showCard(oled, "Where am I?...", NULL, STORY_SHORT_DELAY_MS);
    showCard(oled, "Who am I?", NULL, STORY_SHORT_DELAY_MS);
    showCard(oled, "Ah... I'm Bilbo", NULL, STORY_SHORT_DELAY_MS);
    showCard(oled, "Its dark in here", NULL, STORY_SHORT_DELAY_MS);
    showCard(oled, "There must be a way", "out of here", STORY_LONG_DELAY_MS);
    showCard(oled, "Is there something", "I can use to see?", STORY_LONG_DELAY_MS);

    if (oled != NULL)
    {
        OLED_ClearBuffer(oled);
        OLED_SetCursor(oled, 0, 1);
        OLED_PutString(oled, "Press a button");
        OLED_SetCursor(oled, 0, 2);
        OLED_PutString(oled, "to equip...");
        OLED_Update(oled);
    }
}
