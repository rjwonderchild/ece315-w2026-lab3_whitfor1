#include <stdbool.h>
#include <string.h>
#include "object.h"
#include "misc.h"
#include "game_io.h"

static bool objectHasTag(OBJECT *obj, const char *noun)
{
   if (noun != NULL && *noun != '\0')
   {
      const char **tag;
      for (tag = obj->tags; *tag != NULL; tag++)
      {
         if (strcmp(*tag, noun) == 0) return true;
      }
   }
   return false;
}

static OBJECT ambiguousNoun;

static OBJECT *getObject(const char *noun, OBJECT *from, DISTANCE maxDistance)
{
   OBJECT *obj, *res = NULL;
   for (obj = objs; obj < endOfObjs; obj++)
   {
      if (objectHasTag(obj, noun) && getDistance(from, obj) <= maxDistance)
      {
         res = res == NULL ? obj : &ambiguousNoun;
      }
   }
   return res;
}

OBJECT *getVisible(const char *intention, const char *noun)
{
   OBJECT *obj = getObject(noun, player, distOverthere);
   if (obj == NULL)
   {
      if (getObject(noun, player, distNotHere) == NULL)
      {
         GameIO_Printf("I don't understand %s.\n", intention);
      }
      else if (isLit(player->location))
      {
         GameIO_Printf("You don't see any %s here.\n", noun);
      }
      else
      {
         GameIO_PutString("It's too dark.\n");
      }
   }
   else if (obj == &ambiguousNoun)
   {
      GameIO_Printf("Please be specific about which %s you mean.\n", noun);
      obj = NULL;
   }
   return obj;
}

OBJECT *getPossession(OBJECT *from, const char *verb, const char *noun)
{
   OBJECT *obj = NULL;
   if (from == NULL)
   {
      GameIO_Printf("I don't understand who you want to %s.\n", verb);
   }
   else if ((obj = getObject(noun, from, distHeldContained)) == NULL)
   {
      if (getObject(noun, player, distNotHere) == NULL)
      {
         GameIO_Printf("I don't understand what you want to %s.\n", verb);
      }
      else if (from == player)
      {
         GameIO_Printf("You are not holding any %s.\n", noun);
      }
      else
      {
         GameIO_Printf("There appears to be no %s you can get from %s.\n",
                       noun, from->description);
      }
   }
   else if (obj == &ambiguousNoun)
   {
      GameIO_Printf("Please be specific about which %s you want to %s.\n",
                    noun, verb);
      obj = NULL;
   }
   else if (obj == from)
   {
      GameIO_Printf("You should not be doing that to %s.\n", obj->description);
      obj = NULL;
   }
   return obj;
}

OBJECT *getTopic(const char *noun)
{
   OBJECT *obj;
   for (obj = objs; obj < endOfObjs; obj++)
   {
      if (objectHasTag(obj, noun)) return obj;
   }
   return NULL;
}
