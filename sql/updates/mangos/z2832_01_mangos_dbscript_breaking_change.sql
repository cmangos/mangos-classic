ALTER TABLE db_version CHANGE COLUMN required_z2832_01_mangos_spell_threat_mask required_z2832_01_mangos_dbscript_breaking_change bit;

-- move datalong2 to joint speed field
UPDATE dbscripts_on_event SET speed=datalong2/100, datalong2=0 WHERE datalong2>0 AND command=3;
UPDATE `dbscripts_on_creature_death` SET speed=datalong2/100, datalong2=0 WHERE datalong2>0 AND command=3;
UPDATE `dbscripts_on_creature_movement` SET speed=datalong2/100, datalong2=0 WHERE datalong2>0 AND command=3;
UPDATE `dbscripts_on_go_template_use` SET speed=datalong2/100, datalong2=0 WHERE datalong2>0 AND command=3;
UPDATE `dbscripts_on_go_use` SET speed=datalong2/100, datalong2=0 WHERE datalong2>0 AND command=3;
UPDATE `dbscripts_on_gossip` SET speed=datalong2/100, datalong2=0 WHERE datalong2>0 AND command=3;
UPDATE `dbscripts_on_quest_end` SET speed=datalong2/100, datalong2=0 WHERE datalong2>0 AND command=3;
UPDATE `dbscripts_on_quest_start` SET speed=datalong2/100, datalong2=0 WHERE datalong2>0 AND command=3;
UPDATE `dbscripts_on_relay` SET speed=datalong2/100, datalong2=0 WHERE datalong2>0 AND command=3;
UPDATE `dbscripts_on_spell` SET speed=datalong2/100, datalong2=0 WHERE datalong2>0 AND command=3;

