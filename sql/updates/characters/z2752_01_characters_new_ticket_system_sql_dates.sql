ALTER TABLE character_db_version CHANGE COLUMN required_z2751_01_characters_new_ticket_system required_z2752_01_characters_new_ticket_system_sql_dates bit;

--
-- Modify timestamp column to be compatible with strict MySQL dates for table `gm_tickets`
--

ALTER TABLE `gm_tickets` MODIFY COLUMN `created` bigint(40) unsigned NOT NULL;
ALTER TABLE `gm_tickets` MODIFY COLUMN `updated` bigint(40) unsigned NOT NULL DEFAULT 0;
ALTER TABLE `gm_tickets` MODIFY COLUMN `seen` bigint(40) unsigned NOT NULL DEFAULT 0;
ALTER TABLE `gm_tickets` MODIFY COLUMN `answered` bigint(40) unsigned NOT NULL DEFAULT 0;
ALTER TABLE `gm_tickets` MODIFY COLUMN `closed` bigint(40) unsigned NOT NULL DEFAULT 0;

