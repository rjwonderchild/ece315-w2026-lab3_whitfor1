typedef struct object {
   bool         (*condition)(void);
   const char    *description;
   const char   **tags;
   struct object *location;
   struct object *destination;
   struct object *prospect;
   const char    *details;
   const char    *contents;
   const char    *textGo;
   const char    *gossip;
   int            weight;
   int            capacity;
   int            health;
   int            light;
   void         (*open)(void);
   void         (*close)(void);
   void         (*lock)(void);
   void         (*unlock)(void);
} OBJECT;

extern OBJECT objs[];
#define gossipEWNS	(objs + 0)
#define tunnel1	(objs + 1)
#define tunnel2	(objs + 2)
#define tunnel3	(objs + 3)
#define laketunnel	(objs + 4)
#define undergroundLake	(objs + 5)
#define gollumsisland	(objs + 6)
#define riddlerock	(objs + 7)
#define escapetunnel	(objs + 8)
#define riddleSolvedToken	(objs + 9)
#define thering	(objs + 10)
#define gollum	(objs + 11)
#define player	(objs + 12)
#define lampOff	(objs + 13)
#define lampOn	(objs + 14)
#define northFromTunnel1	(objs + 15)
#define southFromTunnel2	(objs + 16)
#define northFromTunnel2	(objs + 17)
#define northFromTunnel2Blocked	(objs + 18)
#define southFromTunnel3	(objs + 19)
#define northFromTunnel3	(objs + 20)
#define northFromTunnel3Blocked	(objs + 21)
#define southFromLakeTunnel	(objs + 22)
#define northFromLakeTunnel	(objs + 23)
#define northFromLakeTunnelBlocked	(objs + 24)
#define southFromLake	(objs + 25)
#define eastFromLake	(objs + 26)
#define westFromIsland	(objs + 27)
#define eastFromIsland	(objs + 28)
#define westFromRock	(objs + 29)
#define westFromRockBlocked	(objs + 30)
#define eastToEscape	(objs + 31)
#define eastToEscapeBlocked	(objs + 32)
#define openDoorToBackroom	(objs + 33)
#define closedDoorToBackroom	(objs + 34)
#define openDoorToCave	(objs + 35)
#define closedDoorToCave	(objs + 36)
#define openBox	(objs + 37)
#define closedBox	(objs + 38)
#define lockedBox	(objs + 39)
#define keyForBox	(objs + 40)

#define endOfObjs	(objs + 41)
#define validObject(obj) ((obj) != NULL && (obj) >= objs && (obj) < endOfObjs)
