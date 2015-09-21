ALTER TABLE db_version CHANGE COLUMN required_z1671_s1201_01_mangos_spell_bonus_data required_z1683_s1231_01_mangos_creature_template bit;

UPDATE creature_template SET flags_extra=flags_extra|0x00000400 WHERE npcflag=npcflag|0x00008000;
UPDATE creature_template SET npcflag=npcflag & ~0x00008000;
