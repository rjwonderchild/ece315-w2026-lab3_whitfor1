#include <stdbool.h>
#include <stdio.h>
#include "object.h"
#include "misc.h"
#include "match.h"
#include "noun.h"
#include "reach.h"
#include "game_state.h"

static void completeRiddleGame(void)
{
   if (!gRiddleCompleted)
   {
      gRiddleCompleted = true;
      riddleSolvedToken->location = player;
      printf("Bilbo answers the riddle at last.\n");
      printf("Gollum hisses and shrinks back from the stone.\n");
      printf("The way to the escape tunnel is now open.\n");
   }
   else
   {
      printf("The riddle game is already over.\n");
   }
}

static void talk(const char *about, OBJECT *to)
{
   OBJECT *topic = getTopic(about);

   if (topic == NULL)
   {
      printf("I don't understand what you want to talk about.\n");
      return;
   }

   if (to == gollum && topic == riddlerock)
   {
      completeRiddleGame();
      return;
   }

   printf("You hear %s say: '%s'\n",
          to->description,
          topic == to ? "I don't want to talk about myself."
                      : topic->gossip);
}

bool executeTalk(void)
{
   OBJECT *to = actorHere();
   if (to != NULL)
   {
      talk(params[0], to);
   }
   else
   {
      printf("There is nobody here to talk to.\n");
   }
   return true;
}

bool executeTalkTo(void)
{
   OBJECT *to = reachableObject("who to talk to", params[1]);
   if (to != NULL)
   {
      if (to->health > 0)
      {
         talk(params[0], to);
      }
      else
      {
         printf("There is no response from %s.\n", to->description);
      }
   }
   return true;
}
