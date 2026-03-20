#include "story.h"
#include "effects.h"
#include "game_io.h"
#include "FreeRTOS.h"
#include "task.h"

#define STORY_SHORT_DELAY_MS   1500
#define STORY_LONG_DELAY_MS    2500

static void showCard(PmodOLED *oled, const char *text, TickType_t delayMs)
{
    if (oled == NULL || text == NULL)
    {
        return;
    }

    GameIO_Clear();
    GameIO_SetStatus(text);
    vTaskDelay(pdMS_TO_TICKS(delayMs));
    dissolveScreenFast(oled);
}

void showTitleScreen(PmodOLED *oled)
{
    GameIO_Init(oled);
    showCard(oled, "The Lord of the\nRings", STORY_SHORT_DELAY_MS);
}

void showStoryCards(PmodOLED *oled)
{
    GameIO_Init(oled);

    showCard(oled, "Where am I?...", STORY_SHORT_DELAY_MS);
    showCard(oled, "Who am I?", STORY_SHORT_DELAY_MS);
    showCard(oled, "Ah... I'm Bilbo", STORY_SHORT_DELAY_MS);
    showCard(oled, "Its dark in here", STORY_SHORT_DELAY_MS);
    showCard(oled, "There must be\na way out\nof here", STORY_LONG_DELAY_MS);
    showCard(oled, "Is there\nsomething I\ncan use to\nsee?", STORY_LONG_DELAY_MS);

    GameIO_Clear();
    GameIO_SetStatus("Press a button\nto equip...");
}
