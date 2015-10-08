[GUIDE] Introduction to Database content for SD2
================================================

This guide is intended to help people

* to understand which information of the database is used with SD2
* who want to contribute their patches as complete as possible

All sql-related files are located in the ScriptDev2/sql and subsequent directories.

SQL-Files
---------

Files that contain full SD2-Database content
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For a script we usually have to take care of these files:

* mangos_scriptname_full.sql
+
This file is applied to the world database (default: mangos), and contains the ScriptNames
+
* scriptdev2_script_full.sql
+
This file is applied to the sd2 database (default: scriptdev2), and contains texts, gossip-items and waypoints

Patchfiles for incremental Updates
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Patches for the databases are stored in the files:

* Updates/rXXXX_mangos.sql
+
This file contains the changes that should be done with the patch to the world-databse
+
* Updates/rXXXX_scriptdev2.sql
+
This file contains the changes that should be done with the patch to the scriptdev2-database

World-Database
--------------

ScriptNames of NPCs:
~~~~~~~~~~~~~~~~~~~~

If we need to assign a ScriptName to a NPC (GameObject-Scripts are similar) the statement is:

-----------
UPDATE creature_template SET ScriptName='npc_and_his_name' WHERE entry=XYZ;
-----------
or
-----------
UPDATE creature_template SET ScriptName='npc_something_identifying' WHERE entry IN (XYZ, ZYX);
-----------

'Remark:' For creatures with many difficulty entries, only the one for normal difficulty needs the ScriptName.

ScriptNames for scripted_areatrigger:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For Areatriggers (or scripted_event_id) we usally cannot use UPDATE, hence we need to DELETE possible old entries first:

-----------
DELETE FROM scripted_areatrigger WHERE entry=XYZ;
INSERT INTO scripted_areatrigger VALUES (XYZ, at_some_place);
-----------

ScriptDev2-Database
-------------------

entry-Format for texts and for gossip-texts:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The to be used entry is a combination of the number depending on the map and a counter.

* This is for texts: -1<MapId><three-digit-counter>
* For gossip-texts: -3<MapId><three-digit-counter>
+
where <MapId> is the ID of the map for instances, or 000 for all other maps.

Example: Text on WorldMap
^^^^^^^^^^^^^^^^^^^^^^^^^

Let's say we want to add a new text to a NPC on Kalimdor (no instance),
then we need to look which is the last text entry of the format -1000XYZ
(this one can be found in scriptdev2_script_full.sql).

On the moment where I write this guide this is:

----------
(-1000589,'Kroshius live? Kroshius crush!',0,1,0,0,'SAY_KROSHIUS_REVIVE');
----------

so our first text entry will be -1000590.

Example: Gossip-Item in Instance
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Let's say we want to add a new gossip item to a NPC in Culling of Stratholme, this map has the ID 595.
At this moment there is already some gossip_text, and the last one is

------------
(-3595005,'So how does the Infinite Dragonflight plan to interfere?','chromie GOSSIP_ITEM_INN_3');
------------

so our first gossip-text entry will be -3595006.

Format for texts
~~~~~~~~~~~~~~~~

The format is `(entry,content_default,sound,type,language,emote,comment)` with these meanings:

entry:: should now be clear ;)
content_default:: is the text (in english) enclosed with '. +
  There are a few placeholders that can be used:
  +
[horizontal]
%s;;  self, is the name of the Unit saying the text +
The $-placeholders work only if you use DoScriptText with a 'target'.
$N, $n;;  the [N, n]ame of the target
$C, $c;;  the [C, c]lass of the target
$R, $r;;  the [R, r]ace of the target
$GA:B; ;;  if the target is male then A else B is displayed, Example:
+
--------------------------------
'Time to teach you a lesson in manners, little $Gboy:girl;!'
--------------------------------
+
Remember to escape [red]#\'# with [red]#\'#, Example:
+
--------------------------------
'That \'s my favourite chocolate bar'.
--------------------------------
sound:: is the sound ID that shall be played on saying, they are stored in SoundEntries.dbc
+
[quote, Ntsc]
_____________________________
Sound Ids are stored within the SoundEntries.dbc file. Within that dbc file you will find a reference to the actual file that is played. We cannot help you with reading these files so please do not ask how.
_____________________________
+
type:: is the type of the text, there are these possibilities:
+
-------------
0 CHAT_TYPE_SAY - 'white' text
1 CHAT_TYPE_YELL - 'red' text
2 CHAT_TYPE_TEXT_EMOTE - 'yellow' emote-text (no <Name>... )
3 CHAT_TYPE_BOSS_EMOTE - 'big yellow' emote-text displayed in the center of the screen
4 CHAT_TYPE_WHISPER - whisper, needs a target
5 CHAT_TYPE_BOSS_WHISPER - whipser, needs a target
6 CHAT_TYPE_ZONE_YELL - 'red' text, displayed to everyone in the zone
--------------
+
language:: is the language of the text (like LANG_GNOMISH), see +enum Language+ in `game/SharedDefines.h` -- usually zero (LANG_UNIVERSAL)
emote:: is the emote the npc shall perform on saying the text, can be found in +enum Emote+ in `game/SharedDefines.h`
comment:: is a comment to this text, usually the used enum of the script, like SAY_KROSHIUS_REVIVE, if this enum is not identifying the npc, then the name of the npc is put before.

Format for gossip-texts
~~~~~~~~~~~~~~~~~~~~~~~

The format for gossip texts is `(entry,content_default,comment)` +
The fields have the same meaning as for script-texts.

Format for waypoints
~~~~~~~~~~~~~~~~~~~~

The format for waypoints is `(entry,pointid,location_x,location_y,location_z,waittime,point_comment)` with these meanings:

entry:: is the entry of the scripted NPC
pointid:: is the ID of the point, usally starting with 01, and increasing
location_*:: describes the position of the waypoint
waittime:: is the time, the mob will wait after reaching _this_ waypoint, before he continues to go to the next
point_comment:: is used to note if something special is happening at this point, like quest credit


Creating the Patch
------------------

There are different ways to get to a patch, I prefer this workflow:

For the scriptdev2 database (patch files):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Open scriptdev2_script_full.txt +
scroll to the right place for the needed SQL-statements, to note the entry. +
(for texts depending on mapId, and to the last counter, for waypoints behind the last inserted waypoint)

Example for normal world text:
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Assume the last entry in your map (here world-map) was:

--------------
(-1000589,'Kroshius live? Kroshius crush!',0,1,0,0,'SAY_KROSHIUS_REVIVE');
--------------

Now create a new file: Updates/r0000_scriptdev2.sql
Add there:

-----------
DELETE FROM script_texts WHERE entry=-1000590;
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1000590,'My fancy aggro-text',0,1,0,0,'boss_hogger SAY_AGGRO');
-----------
or
-----------
DELETE FROM script_texts WHERE entry BETWEEN -1000592 AND -1000590;
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1000590,'My fancy aggro-text1',0,1,0,0,'boss_hogger SAY_AGGRO1'),
(-1000591,'My fancy aggro-text2',0,1,0,0,'boss_hogger SAY_AGGRO2'),
(-1000592,'My fancy aggro-text3',0,1,0,0,'boss_hogger SAY_AGGRO3');
-----------

Hint: the INSERT statements can also be copied from the scriptdev2_script_full.sql

Example for waypoints:
^^^^^^^^^^^^^^^^^^^^^^

The required SQL code to add a waypoint is:

----------
DELETE FROM script_waypoint WHERE entry=<MyNpcEntry>;
INSERT INTO script_waypoint VALUES
(<MyNpcEntry>, 1, 4013.51,6390.33, 29.970, 0, '<MyNPCName> - start escort'),
(<MyNpcEntry>, 2, 4060.51,6400.33, 20.970, 0, '<MyNPCName> - finish escort');
----------

When the Update file is done, append an additional empty line +
And test these lines for correctness!

For the scriptdev2 database (full files):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If everything works alright, and you finally intend to prepare the full-patch, copy the SQL-Code that is needed to the proper place in scriptdev2_script_full.sql,
(for a new npc add an empty line), and change the semicolon to a comma:

Example for world text:
^^^^^^^^^^^^^^^^^^^^^^

-----------
(-1000589,'Kroshius live? Kroshius crush!',0,1,0,0,'SAY_KROSHIUS_REVIVE'),

(-1000590,'My fancy aggro-text',0,1,0,0,'boss_hogger SAY_AGGRO');
-----------

The waypoints are added behind the last waypoint, after an empty line.

For the world database:
~~~~~~~~~~~~~~~~~~~~~~~

Create a new file:  Updates/r0000_mangos.sql +
In this file put the needed statements for your ScriptNames, append an empty line, convert lineendings to Unix, and then test it for correctness.

If everything is alright, open mangos_scriptname_full.sql and go to the right place (this is usally sorted alphabetically by zone). +
Insert the needed statement where it fits (usally again ordered alphabetically)

After this is done, Create a patch including the (untracked) files in Update/ +
then you have all information in the created patch, and anyone who wants to test your patch just needs to apply the created files from Updates/
