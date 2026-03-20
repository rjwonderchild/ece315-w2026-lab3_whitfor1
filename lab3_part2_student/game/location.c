#include <stdbool.h>
#include "object.h"
#include "misc.h"
#include "match.h"
#include "noun.h"
#include "game_io.h"

bool executeLookAround(void)
{
   if (isLit(player->location))
   {
      GameIO_Printf("You are in %s.\n", player->location->description);
   }
   else
   {
      GameIO_PutString("It is very dark in here.\n");
   }

   listObjectsAtLocation(player->location);
   return true;
}

bool executeLook(void)
{
   OBJECT *obj = getVisible("what you want to look at", params[0]);

   switch (getDistance(player, obj))
   {
   case distHereContained:
      GameIO_PutString("Hard to see, try to get it first.\n");
      break;

   case distOverthere:
      GameIO_PutString("Too far away, move closer please.\n");
      break;

   case distNotHere:
      GameIO_Printf("You don't see any %s here.\n", params[0]);
      break;

   case distUnknownObject:
      /* already handled by getVisible */
      break;

   default:
      GameIO_Printf("%s\n", obj->details);
      listObjectsAtLocation(obj);
   }

   return true;
}

static void movePlayer(OBJECT *passage)
{
   GameIO_Printf("%s\n", passage->textGo);

   if (passage->destination != NULL)
   {
      player->location = passage->destination;
      GameIO_PutString("\n");
      executeLookAround();
   }
}

bool executeGo(void)
{
   OBJECT *obj = getVisible("where you want to go", params[0]);

   switch (getDistance(player, obj))
   {
   case distOverthere:
      movePlayer(getPassage(player->location, obj));
      break;

   case distNotHere:
      GameIO_Printf("You don't see any %s here.\n", params[0]);
      break;

   case distUnknownObject:
      /* already handled by getVisible */
      break;

   default:
      movePlayer(obj);
   }

   return true;
}
