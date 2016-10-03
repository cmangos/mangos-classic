ALTER TABLE db_version CHANGE COLUMN required_z2685_01_mangos_event_linkedto required_z2687_01_mangos_spell_chain BIT(1) NULL DEFAULT NULL;

DELETE FROM `spell_chain` WHERE `spell_id` IN (759,3552,10053,10054);
INSERT INTO `spell_chain` VALUES
(759,0,759,1,0),
(3552,759,759,2,0),
(10053,3552,759,3,0),
(10054,10053,759,4,0);