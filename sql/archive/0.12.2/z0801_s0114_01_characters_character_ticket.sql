ALTER TABLE character_db_version CHANGE COLUMN required_z0788_s0099_02_characters_pet_aura required_z0801_s0114_01_characters_character_ticket bit;

alter table `character_ticket`
    add column `response_text` text CHARSET utf8 COLLATE utf8_general_ci NULL after `ticket_text`;
