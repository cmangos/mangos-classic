UPDATE creature_template SET RegenerateStats=RegenerateStats*2 WHERE RegenerateStats>0;
-- not adding 0x001 because its unused and mobs generally do not regen hp in combat anyway
UPDATE creature_template SET RegenerateStats=RegenerateStats|0x008 WHERE RegenerateStats & 4;

