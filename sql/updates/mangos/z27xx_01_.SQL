
ALTER TABLE account ADD COLUMN MaxLevel tinyint(3) unsigned NOT NULL DEFAULT '255' AFTER MinLevel;
