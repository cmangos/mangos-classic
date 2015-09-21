ALTER TABLE db_version CHANGE COLUMN required_z1614_s1120_02_mangos_mangos_string required_z1661_s1188_01_mangos_item_template bit;

UPDATE item_template
  SET ScriptName = '' WHERE ScriptName = 'internalItemHandler';

