ALTER TABLE character_db_version CHANGE COLUMN required_z2698_01_characters_playerbot_saved_data required_z2707_01_characters_spell_cooldown bit;

ALTER TABLE character_spell_cooldown CHANGE COLUMN guid LowGuid int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Global Unique Identifier, Low part';
ALTER TABLE character_spell_cooldown CHANGE COLUMN spell SpellId int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Spell Identifier';
ALTER TABLE character_spell_cooldown CHANGE COLUMN time SpellExpireTime bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Spell cooldown expire time';
ALTER TABLE character_spell_cooldown CHANGE COLUMN item ItemId int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Item Identifier' AFTER SpellExpireTime;
ALTER TABLE character_spell_cooldown ADD Category int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Spell category' AFTER SpellExpireTime;
ALTER TABLE character_spell_cooldown ADD CategoryExpireTime bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Spell category cooldown expire time' AFTER Category;