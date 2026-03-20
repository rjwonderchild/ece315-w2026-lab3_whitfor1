#include <stdbool.h>
#include <string.h>
#include "expand.h"
#include "parsexec.h"
#include "game_io.h"
#include "story.h"

static char input[100] = "look around";
static bool gameRunning = true;

static void normalizeInput(char *text)
{
   size_t len;

   if (text == NULL)
   {
      return;
   }

   len = strcspn(text, "\r\n");
   text[len] = '\0';
}

void gameInit(void)
{
   gameRunning = true;
   strncpy(input, "look around", sizeof(input) - 1);
   input[sizeof(input) - 1] = '\0';

   GameIO_PutString("Welcome to Little Cave Adventure.\n");
}

void gameShowIntro(PmodOLED *oled)
{
   if (oled != NULL)
   {
      showTitleScreen(oled);
      showStoryCards(oled);
   }
}

bool gameProcessCommand(const char *command)
{
   if (!gameRunning)
   {
      return false;
   }

   if (command == NULL)
   {
      return true;
   }

   strncpy(input, command, sizeof(input) - 1);
   input[sizeof(input) - 1] = '\0';
   normalizeInput(input);

   if (input[0] == '\0')
   {
      return true;
   }

   gameRunning = parseAndExecute(expand(input, sizeof(input)));

   if (!gameRunning)
   {
      GameIO_PutString("\nBye!\n");
   }

   return gameRunning;
}

bool gamePollAndProcess(void)
{
   if (!gameRunning)
   {
      return false;
   }

   GameIO_Prompt();

   if (GameIO_GetLine(input, sizeof(input)))
   {
      normalizeInput(input);

      if (input[0] != '\0')
      {
         gameRunning = parseAndExecute(expand(input, sizeof(input)));

         if (!gameRunning)
         {
            GameIO_PutString("\nBye!\n");
         }
      }
   }

   return gameRunning;
}

#ifdef DESKTOP_BUILD
int main(void)
{
   gameInit();

   while (gamePollAndProcess())
   {
      /* desktop polling loop */
   }

   return 0;
}
#endif
