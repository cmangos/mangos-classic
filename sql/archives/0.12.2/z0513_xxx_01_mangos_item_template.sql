ALTER TABLE db_version CHANGE COLUMN required_z0488_xxx_01_mangos_spell_proc_event required_z0513_xxx_01_mangos_item_template bit;

ALTER TABLE item_template
  DROP COLUMN RequiredDisenchantSkill,
  DROP COLUMN ArmorDamageModifier;
