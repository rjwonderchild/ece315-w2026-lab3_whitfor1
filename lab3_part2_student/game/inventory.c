#include <stdbool.h>
#include "object.h"
#include "misc.h"
#include "inventory.h"
#include "game_io.h"

bool executeInventory(void)
{
   int count = listObjectsAtLocation(player);

   if (count == 0)
   {
      GameIO_PutString("You are empty-handed.\n");
   }

   return true;
}
