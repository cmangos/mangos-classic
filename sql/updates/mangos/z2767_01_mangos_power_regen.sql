ALTER TABLE db_version CHANGE COLUMN required_z2765_01_mangos_command_ahbot required_z2767_01_mangos_power_regen bit;

UPDATE creature_template SET RegenerateStats=RegenerateStats*2 WHERE RegenerateStats>0;
-- not adding 0x001 because its unused and mobs generally do not regen hp in combat anyway
UPDATE creature_template SET RegenerateStats=RegenerateStats|0x008 WHERE RegenerateStats & 4;

