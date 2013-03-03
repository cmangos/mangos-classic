= MaNGOS -- README =

This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information

See the COPYING file for copying conditions.

NOTE: This is file special addition for mangoszero fork (from mangos 0.12)
      It describe work with backported sql updates

== Database Updates ==

This folder contains SQL files which will apply required updates to your MySQL
database, whenever the MaNGOS database structure has been changed or extended.

To see if you need an update, the file names have been given a fixed structure
that should enable you to see if you need an update or not.

=== File Name Description ===

File names are divided into two parts.

First part is the order_number, revision and counter that shows the commit revision that
will be compatible with the database after apply that particular update.
Counter sets the order to apply sql updates for the same revision. Order_number set
global order in directory for backported sql updates (its backported in random order
and need special mangoszero side order for apply).

The second part of the name is the database and the table that needs an update or has been added.

See an example below:

         z0001_xxx_01_characters_character_db_version.sql
         |     |   |  |          |
         |     |   |  |          |
         |     |   |  |          The table `character_db_version`
         |     |   |  |          will need an update.
         |     |   |  |
         |     |   |  Name of affected DB (default recommended name)
         |     |   |  Can be: characters, mangos, realmd
         |     |   |
         |     |   Counter show number of sql update in 0.12 backported or own (non bakported, xxx) updates list for provided master revision
         |     |   and set proper order for sql updates for same revision
         |     |
         |     MaNGOS 0.12 commit revision related to sql update (if this backported sql update.
         |     It can be absent (and replaced by xxx) if sql update not really backported but only mangoszero specific.
         |
         Order_number set special order for apply sql updates from different backported revisions in mangoszero


After applying an update the DB is compatible with the mangos revision of this sql update.
SQL updates include special protection against multiple and wrong order of update application.

Attempts to apply sql updates to an older DB without previous SQL updates in list for the database
or to DB with already applied this or later SQL update will generate an error and not be applied.

=== For Commiters ====

IMPORTANT: Need use git_id tool from MaNGOSZero sources.

===== For backporting sql updates =====

1. Rename sql update from mangos-0.12 form (s0070_7932_01_characters_character_pet.sql) to mangoszero form (z9999_s0070_01_characters_character_pet.sql)
   using any free order_number in sql/updates files list (it will replaced by proper at git_id -s use).

   For realmd sql update you need use master revision instead mangos-0.12 (realmd revisons master based):
   (s0070_10008_01_realmd.sql) to mangoszero form (z9999_10008_01_realmd.sql)

2. You can ignore sql order guards in sql update file. It will fixed at git_id -s use.
   You can ignore content and changes/conflicts in sql/update/Makefile.am. It will regenerated at git_id -s use.
   For make possible test code part build you can resolve revision_sql.h conflicts in any way.
   Work test better do after apply git_id -s to comit when revision_sql.h will regenerated.
   You also need resolve conflicts in DB sql file before git_id tool use (used revison in DB version not important and will replaced by git_id -s)

3. Now use git_id -s

===== For new sql updates =====

1. Name sql update in mangoszero form (z9999_xxxxxx_01_characters_character_pet.sql)
   using any free order_number in sql/updates files list (it will replaced by proper at git_id -s use).

2. Add sql update to commit and manually include sql update changes in to related DB sql file.

3. Now use git_id -s (sql update revison will updates in name, sql guards will added to sql update,
   required revision data will update in related DB sql file and in revision_sql.h)
