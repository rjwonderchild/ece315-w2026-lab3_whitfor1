#include <stdbool.h>
#include "object.h"
#include "misc.h"
#include "location.h"
#include "game_io.h"

static void swapLocations(const char *verb1, OBJECT *obj1,
                          const char *verb2, OBJECT *obj2)
{
   OBJECT *tmp = obj1->location;
   OBJECT *obj = tmp != NULL ? obj1 : obj2;
   const char *verb = tmp != NULL ? verb1 : verb2;

   obj1->location = obj2->location;
   obj2->location = tmp;

   if (verb != NULL)
   {
      GameIO_Printf("You %s %s.\n", verb, obj->description);
   }
}

void cannotBeOpened(void)    { GameIO_PutString("That cannot be opened.\n");    }
void cannotBeClosed(void)    { GameIO_PutString("That cannot be closed.\n");    }
void cannotBeLocked(void)    { GameIO_PutString("That cannot be locked.\n");    }
void cannotBeUnlocked(void)  { GameIO_PutString("That cannot be unlocked.\n");  }

void isAlreadyOpen(void)     { GameIO_PutString("That is already open.\n");     }
void isAlreadyClosed(void)   { GameIO_PutString("That is already closed.\n");   }
void isAlreadyLocked(void)   { GameIO_PutString("That is already locked.\n");   }
void isAlreadyUnlocked(void) { GameIO_PutString("That is already unlocked.\n"); }

void isStillOpen(void)       { GameIO_PutString("That is still open.\n");       }
void isStillLocked(void)     { GameIO_PutString("That is locked.\n");           }

void toggleDoorToBackroom(void)
{
   swapLocations(NULL, openDoorToCave, NULL, closedDoorToCave);
   swapLocations("close", openDoorToBackroom, "open", closedDoorToBackroom);
}

void toggleDoorToCave(void)
{
   swapLocations(NULL, openDoorToBackroom, NULL, closedDoorToBackroom);
   swapLocations("close", openDoorToCave, "open", closedDoorToCave);
}

void toggleBox(void)
{
   swapLocations("close", openBox, "open", closedBox);
}

void toggleBoxLock(void)
{
   if (keyForBox->location == player)
   {
      swapLocations("lock", closedBox, "unlock", lockedBox);
   }
   else
   {
      GameIO_PutString("You don't have a key.\n");
   }
}

void toggleLamp(void)
{
   bool oldLit = isLit(player->location);

   swapLocations("turn off", lampOn, "turn on", lampOff);

   if (oldLit != isLit(player->location))
   {
      GameIO_PutString("\n");
      executeLookAround();
   }
}
