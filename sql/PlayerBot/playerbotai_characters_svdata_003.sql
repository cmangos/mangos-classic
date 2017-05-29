ALTER TABLE `playerbot_saved_data` DROP COLUMN `bot_secondary_order`;
ALTER TABLE playerbot_saved_data CHANGE COLUMN `bot_primary_order` `combat_order` int(11);
