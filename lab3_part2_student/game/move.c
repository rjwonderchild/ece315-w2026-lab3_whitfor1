#include <stdbool.h>
#include "object.h"
#include "misc.h"
#include "game_io.h"

static int weightOfContents(OBJECT *container)
{
   int sum = 0;
   OBJECT *obj;
   for (obj = objs; obj < endOfObjs; obj++)
   {
      if (isHolding(container, obj)) sum += obj->weight;
   }
   return sum;
}

static void describeMove(OBJECT *obj, OBJECT *to)
{
   if (to == player->location)
   {
      GameIO_Printf("You drop %s.\n", obj->description);
   }
   else if (to != player)
   {
      GameIO_Printf(to->health > 0 ? "You give %s to %s.\n" : "You put %s in %s.\n",
                    obj->description, to->description);
   }
   else if (obj->location == player->location)
   {
      GameIO_Printf("You pick up %s.\n", obj->description);
   }
   else
   {
      GameIO_Printf("You get %s from %s.\n",
                    obj->description, obj->location->description);
   }
}

void moveObject(OBJECT *obj, OBJECT *to)
{
   if (obj == NULL)
   {
      /* already handled by getVisible or getPossession */
   }
   else if (to == NULL)
   {
      GameIO_PutString("There is nobody here to give that to.\n");
   }
   else if (to->capacity == 0)
   {
      GameIO_PutString(obj == keyForBox && (to == closedBox || to == lockedBox) ?
                       "The key seems to fit the lock.\n" :
                       "It doesn't seem to fit in.\n");
   }
   else if (obj->weight > to->capacity)
   {
      GameIO_PutString("That is way too heavy.\n");
   }
   else if (obj->weight + weightOfContents(to) > to->capacity)
   {
      GameIO_PutString("That would become too heavy.\n");
   }
   else
   {
      describeMove(obj, to);
      obj->location = to;
   }
}
