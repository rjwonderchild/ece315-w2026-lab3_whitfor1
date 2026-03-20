#include <stdbool.h>
#include "object.h"
#include "misc.h"
#include "match.h"
#include "reach.h"
#include "talk.h"
#include "game_io.h"

bool executeTalk(void)
{
   if (*noun == '\0')
   {
      GameIO_PutString("Talk about what?\n");
      return true;
   }

   return executeTalkTo();
}

bool executeTalkTo(void)
{
   OBJECT *obj = NULL;

   if (*noun == '\0')
   {
      GameIO_PutString("Talk about what?\n");
      return true;
   }

   if (*secondnoun == '\0')
   {
      obj = actorHere();
      if (obj == NULL)
      {
         GameIO_PutString("Talk with whom?\n");
         return true;
      }
   }
   else
   {
      obj = reachableObject("talk to", secondnoun);
      if (obj == NULL)
      {
         return true;
      }
   }

   if (obj->talk == NULL)
   {
      GameIO_Printf("You can't talk to %s.\n", obj->description);
      return true;
   }

   return obj->talk();
}
