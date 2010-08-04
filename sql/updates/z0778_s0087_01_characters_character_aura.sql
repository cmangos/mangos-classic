ALTER TABLE character_db_version CHANGE COLUMN required_z0738_s0041_01_characters_auctionhouse required_z0778_s0087_01_characters_character_aura bit;

ALTER TABLE `character_aura` DROP PRIMARY KEY;
ALTER TABLE `character_aura` ADD PRIMARY KEY  (`guid`,`caster_guid`,`spell`,`effect_index`);
