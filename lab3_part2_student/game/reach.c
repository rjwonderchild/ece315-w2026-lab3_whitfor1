#include <stdbool.h>
#include "object.h"
#include "misc.h"
#include "noun.h"
#include "game_io.h"

OBJECT *reachableObject(const char *intention, const char *noun)
{
   OBJECT *obj = getVisible(intention, noun);

   switch (getDistance(player, obj))
   {
   case distSelf:
      GameIO_PutString("You should not be doing that to yourself.\n");
      break;

   case distHeldContained:
   case distHereContained:
      GameIO_Printf("You would have to get it from %s first.\n",
                    obj->location->description);
      break;

   case distOverthere:
      GameIO_PutString("Too far away, move closer please.\n");
      break;

   case distNotHere:
   case distUnknownObject:
      /* already handled by getVisible */
      break;

   default:
      return obj;
   }

   return NULL;
}
