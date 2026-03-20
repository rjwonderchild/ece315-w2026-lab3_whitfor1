BEGIN {
   count = 0;
   obj = "";

   if (pass == "c2") {
      print "";
      print "static bool alwaysTrue(void) { return true; }";
      print "";
      print "OBJECT objs[] = {";
   }
}

/^- / {
   outputRecord(",");

   obj = $2;

   prop["condition"]   = "";
   prop["description"] = "NULL";
   prop["tags"]        = "";
   prop["location"]    = "NULL";
   prop["destination"] = "NULL";
   prop["prospect"]    = "";
   prop["details"]     = "\"You see nothing special.\"";
   prop["contents"]    = "\"You see\"";
   prop["textGo"]      = "\"You can't get much closer than this.\"";
   prop["gossip"]      = "\"I know nothing about that.\"";
   prop["weight"]      = "99";
   prop["capacity"]    = "0";
   prop["health"]      = "0";
   prop["light"]       = "0";
   prop["open"]        = "cannotBeOpened";
   prop["close"]       = "cannotBeClosed";
   prop["lock"]        = "cannotBeLocked";
   prop["unlock"]      = "cannotBeUnlocked";
}

obj && /^[ \t]+[a-z]/ {
   name = $1;
   $1 = "";
   sub(/^[ \t]+/, "", $0);

   if (name in prop) {
      prop[name] = $0;
   }
   else if (pass == "c2") {
      print "#error \"" FILENAME " line " NR ": unknown attribute '" name "'\"";
   }
}

!obj && pass == (/^#include/ ? "c1" : "h") {
   print;
}

END {
   outputRecord("\n};");

   if (pass == "h") {
      print "";
      print "#define endOfObjs\t(objs + " count ")";
   }
}

function outputRecord(separator, conditionName, prospectValue)
{
   if (!obj) {
      return;
   }

   if (pass == "h") {
      print "#define " obj "\t(objs + " count ")";
   }
   else if (pass == "c1") {
      if (prop["condition"] != "") {
         print "static bool condition" count "(void) { " prop["condition"] " }";
      }
      print "static const char *tags" count "[] = { " prop["tags"] ", NULL};";
   }
   else if (pass == "c2") {
      conditionName = (prop["condition"] != "") ? "condition" count : "alwaysTrue";
      prospectValue = (prop["prospect"] != "") ? prop["prospect"] : prop["destination"];

      if (prospectValue == "") {
         prospectValue = "NULL";
      }

      print "\t{\t/* " count " = " obj " */";
      print "\t\t" conditionName ",";
      print "\t\t" prop["description"] ",";
      print "\t\ttags" count ",";
      print "\t\t" prop["location"] ",";
      print "\t\t" prop["destination"] ",";
      print "\t\t" prospectValue ",";
      print "\t\t" prop["details"] ",";
      print "\t\t" prop["contents"] ",";
      print "\t\t" prop["textGo"] ",";
      print "\t\t" prop["gossip"] ",";
      print "\t\t" prop["weight"] ",";
      print "\t\t" prop["capacity"] ",";
      print "\t\t" prop["health"] ",";
      print "\t\t" prop["light"] ",";
      print "\t\t" prop["open"] ",";
      print "\t\t" prop["close"] ",";
      print "\t\t" prop["lock"] ",";
      print "\t\t" prop["unlock"];
      print "\t}" separator;
   }

   delete prop;
   count++;
}
