ALTER TABLE db_version CHANGE COLUMN required_z2792_01_mangos_worldstate_name required_z2793_01_mangos_dbscript_extension bit;

ALTER TABLE dbscripts_on_creature_death ADD COLUMN `speed` float NOT NULL DEFAULT '0' AFTER `o`;
ALTER TABLE dbscripts_on_creature_death ADD COLUMN `datafloat` float NOT NULL DEFAULT '0' AFTER `dataint4`;
ALTER TABLE dbscripts_on_creature_movement ADD COLUMN `speed` float NOT NULL DEFAULT '0' AFTER `o`;
ALTER TABLE dbscripts_on_creature_movement ADD COLUMN `datafloat` float NOT NULL DEFAULT '0' AFTER `dataint4`;
ALTER TABLE dbscripts_on_event ADD COLUMN `speed` float NOT NULL DEFAULT '0' AFTER `o`;
ALTER TABLE dbscripts_on_event ADD COLUMN `datafloat` float NOT NULL DEFAULT '0' AFTER `dataint4`;
ALTER TABLE dbscripts_on_go_template_use ADD COLUMN `speed` float NOT NULL DEFAULT '0' AFTER `o`;
ALTER TABLE dbscripts_on_go_template_use ADD COLUMN `datafloat` float NOT NULL DEFAULT '0' AFTER `dataint4`;
ALTER TABLE dbscripts_on_go_use ADD COLUMN `speed` float NOT NULL DEFAULT '0' AFTER `o`;
ALTER TABLE dbscripts_on_go_use ADD COLUMN `datafloat` float NOT NULL DEFAULT '0' AFTER `dataint4`;
ALTER TABLE dbscripts_on_gossip ADD COLUMN `speed` float NOT NULL DEFAULT '0' AFTER `o`;
ALTER TABLE dbscripts_on_gossip ADD COLUMN `datafloat` float NOT NULL DEFAULT '0' AFTER `dataint4`;
ALTER TABLE dbscripts_on_quest_end ADD COLUMN `speed` float NOT NULL DEFAULT '0' AFTER `o`;
ALTER TABLE dbscripts_on_quest_end ADD COLUMN `datafloat` float NOT NULL DEFAULT '0' AFTER `dataint4`;
ALTER TABLE dbscripts_on_quest_start ADD COLUMN `speed` float NOT NULL DEFAULT '0' AFTER `o`;
ALTER TABLE dbscripts_on_quest_start ADD COLUMN `datafloat` float NOT NULL DEFAULT '0' AFTER `dataint4`;
ALTER TABLE dbscripts_on_relay ADD COLUMN `speed` float NOT NULL DEFAULT '0' AFTER `o`;
ALTER TABLE dbscripts_on_relay ADD COLUMN `datafloat` float NOT NULL DEFAULT '0' AFTER `dataint4`;
ALTER TABLE dbscripts_on_spell ADD COLUMN `speed` float NOT NULL DEFAULT '0' AFTER `o`;
ALTER TABLE dbscripts_on_spell ADD COLUMN `datafloat` float NOT NULL DEFAULT '0' AFTER `dataint4`;


