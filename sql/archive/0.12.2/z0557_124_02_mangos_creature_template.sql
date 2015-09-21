ALTER TABLE db_version CHANGE COLUMN required_z0557_124_01_mangos_creature_template required_z0557_124_02_mangos_creature_template bit;

UPDATE creature_template
  SET mindmg = ROUND(mindmg + attackpower), maxdmg=ROUND(maxdmg+attackpower);