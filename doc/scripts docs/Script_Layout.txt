--- Script Layout ---
A quick explanation of the layout I hope everyone will follow for scriptdev2.

--- Sub Folders ---

battlegrounds - Contains scripts for npcs in Battlegrounds

custom - Intentionally empty folder from SVN. If you make a custom script please put it here.

examples - Contains a couple of example scripts showing the most commonly used functions within SD2

eastern_kingdoms, kalimdor, northrend, outland - Contain scripts for anything related to npcs or instances on the corresponding continent.
Instances are grouped to subfolders, the normal continent-maps are divided into zones

world - Contains scripts for anything that is not related to a specified zone.
This includes item_scripts, areatrigger_scripts, some npcs that can be found at many places, go_scripts and spell_scripts

--- Naming Conventions ---

Please keep file names to "type_objectname.cpp" where type is replaced by the type of object and objectname is replaced by the name of the object, creature, item, or area that this script will be used by.

AddSC functions should follow "void AddSC_filename(void);" format. Do not append AI or anything else.
