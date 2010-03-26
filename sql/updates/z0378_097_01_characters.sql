ALTER TABLE character_db_version CHANGE COLUMN required_z0212_xxx_01_characters_kill required_z0378_097_01_characters bit;

ALTER TABLE `group_member`            ADD INDEX `Idx_memberGuid`(`memberGuid`);
ALTER TABLE `guild_eventlog`          ADD INDEX `Idx_PlayerGuid1`(`PlayerGuid1`);
ALTER TABLE `guild_eventlog`          ADD INDEX `Idx_PlayerGuid2`(`PlayerGuid2`);
ALTER TABLE `petition_sign`           ADD INDEX `Idx_playerguid`(`playerguid`);
ALTER TABLE `petition_sign`           ADD INDEX `Idx_ownerguid`(`ownerguid`);
ALTER TABLE `guild_eventlog`          ADD INDEX `Idx_LogGuid`(`LogGuid`);
ALTER TABLE `corpse`                  ADD INDEX `Idx_player`(`player`);
ALTER TABLE `corpse`                  ADD INDEX `Idx_time`(`time`);
ALTER TABLE `guild_rank`              ADD INDEX `Idx_rid`(`rid`);
