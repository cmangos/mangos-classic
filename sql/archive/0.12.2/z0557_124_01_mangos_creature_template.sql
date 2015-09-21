ALTER TABLE db_version CHANGE COLUMN required_z0555_120_01_mangos_spell_proc_event required_z0557_124_01_mangos_creature_template bit;

ALTER TABLE creature_template
  ADD COLUMN dmg_multiplier float NOT NULL default '1' AFTER attackpower;

UPDATE creature_template SET mindmg      = round(mindmg + attackpower / 14);
UPDATE creature_template SET maxdmg      = round(maxdmg + attackpower / 14);
UPDATE creature_template SET attackpower = round((maxdmg + mindmg) * 0.3);
UPDATE creature_template SET mindmg      = round(mindmg * 0.7), maxdmg = round(maxdmg * 0.7);
