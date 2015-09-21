ALTER TABLE db_version CHANGE COLUMN required_z2176_s1809_12258_01_mangos_mangos_string required_z2186_s1819_12268_01_mangos_dbscripts bit;

DROP TABLE IF EXISTS dbscripts_on_creature_movement;
RENAME TABLE creature_movement_scripts TO dbscripts_on_creature_movement;
DROP TABLE IF EXISTS dbscripts_on_event;
RENAME TABLE event_scripts TO dbscripts_on_event;
DROP TABLE IF EXISTS dbscripts_on_go_use;
RENAME TABLE gameobject_scripts TO dbscripts_on_go_use;
DROP TABLE IF EXISTS dbscripts_on_go_template_use;
RENAME TABLE gameobject_template_scripts TO dbscripts_on_go_template_use;
DROP TABLE IF EXISTS dbscripts_on_gossip;
RENAME TABLE gossip_scripts TO dbscripts_on_gossip;
DROP TABLE IF EXISTS dbscripts_on_quest_end;
RENAME TABLE quest_end_scripts TO dbscripts_on_quest_end;
DROP TABLE IF EXISTS dbscripts_on_quest_start;
RENAME TABLE quest_start_scripts TO dbscripts_on_quest_start;
DROP TABLE IF EXISTS dbscripts_on_spell;
RENAME TABLE spell_scripts TO dbscripts_on_spell;

DELETE FROM command WHERE name LIKE 'reload all_scripts';
INSERT INTO command VALUES
('reload all_scripts',3,'Syntax: .reload all_scripts\r\n\r\nReload `dbscripts_on_*` tables.');
