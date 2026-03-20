#include <stdbool.h>
#include "object.h"
#include "match.h"
#include "reach.h"
#include "toggle.h"
#include "game_io.h"
#include "game_state.h"

bool executeTurnOn(void)
{
   OBJECT *obj = reachableObject("what you want to turn on", params[0]);
   if (obj != NULL)
   {
      if (obj == lampOff)
      {
         toggleLamp();
         gStingDrawn = true;
         GameState_UpdateRgbMode();
      }
      else
      {
         GameIO_PutString(obj == lampOn ? "Sting is already drawn.\n"
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
         gStingDrawn = false;
         GameState_UpdateRgbMode();
      }
      else
      {
         GameIO_PutString(obj == lampOff ? "Sting is already sheathed.\n"
                                         : "You cannot turn that off.\n");
      }
   }
   return true;
}
