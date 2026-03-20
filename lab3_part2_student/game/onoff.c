#include <stdbool.h>
#include "object.h"
#include "match.h"
#include "reach.h"
#include "toggle.h"
#include "game_io.h"

bool executeTurnOn(void)
{
   OBJECT *obj = reachableObject("what you want to turn on", params[0]);
   if (obj != NULL)
   {
      if (obj == lampOff)
      {
         toggleLamp();
      }
      else
      {
         GameIO_PutString(obj == lampOn ? "The lamp is already on.\n"
                                        : "You cannot turn that on.\n");
      }
   }
   return true;
}

bool executeTurnOff(void)
{
   OBJECT *obj = reachableObject("what you want to turn off", params[0]);
   if (obj != NULL)
   {
      if (obj == lampOn)
      {
         toggleLamp();
      }
      else
      {
         GameIO_PutString(obj == lampOff ? "The lamp is already off.\n"
                                         : "You cannot turn that off.\n");
      }
   }
   return true;
}
