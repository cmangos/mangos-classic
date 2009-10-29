ALTER TABLE db_version CHANGE COLUMN required_062______01_mangos_spell_learn_spell required_078_8749_01_mangos_mail_loot_template bit;

RENAME TABLE quest_mail_loot_template TO mail_loot_template;

UPDATE mail_loot_template, quest_template
  SET mail_loot_template.entry = quest_template.RewMailTemplateId WHERE mail_loot_template.entry = quest_template.entry;
