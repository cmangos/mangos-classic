ALTER TABLE db_version CHANGE COLUMN required_078_8749_01_mangos_mail_loot_template required_062______01_mangos_spell_learn_spell bit;

RENAME TABLE mail_loot_template TO quest_mail_loot_template;

UPDATE quest_mail_loot_template, quest_template
  SET mail_loot_template.entry = quest_template.entry WHERE mail_loot_template.entry = quest_template.RewMailTemplateId;
