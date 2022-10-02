ALTER TABLE db_version CHANGE COLUMN required_z2797_01_mangos_eai_dbguid_support required_z2798_01_mangos_battleground_reflooot bit;

ALTER TABLE battleground_template ADD COLUMN PlayerSkinReflootId MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'reference_loot_template entry';


