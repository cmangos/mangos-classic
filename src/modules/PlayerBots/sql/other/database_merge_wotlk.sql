DROP TABLE IF EXISTS wotlkcharacters.ahbot_history;
CREATE TABLE wotlkcharacters.ahbot_history LIKE wotlkplayerbots.ahbot_history;
INSERT INTO wotlkcharacters.ahbot_history SELECT * FROM wotlkplayerbots.ahbot_history;

DROP TABLE IF EXISTS wotlkcharacters.ahbot_category;
CREATE TABLE wotlkcharacters.ahbot_category LIKE wotlkplayerbots.ahbot_category;
INSERT INTO wotlkcharacters.ahbot_category SELECT * FROM wotlkplayerbots.ahbot_category;

DROP TABLE IF EXISTS wotlkcharacters.ahbot_price;
CREATE TABLE wotlkcharacters.ahbot_price LIKE wotlkplayerbots.ahbot_price;
INSERT INTO wotlkcharacters.ahbot_price SELECT * FROM wotlkplayerbots.ahbot_price;

DROP TABLE IF EXISTS wotlkcharacters.ai_playerbot_custom_strategy;
CREATE TABLE wotlkcharacters.ai_playerbot_custom_strategy LIKE wotlkplayerbots.ai_playerbot_custom_strategy;
INSERT INTO wotlkcharacters.ai_playerbot_custom_strategy SELECT * FROM wotlkplayerbots.ai_playerbot_custom_strategy;

DROP TABLE IF EXISTS wotlkcharacters.ai_playerbot_db_store;
CREATE TABLE wotlkcharacters.ai_playerbot_db_store LIKE wotlkplayerbots.ai_playerbot_db_store;
INSERT INTO wotlkcharacters.ai_playerbot_db_store SELECT * FROM wotlkplayerbots.ai_playerbot_db_store;

DROP TABLE IF EXISTS wotlkmangos.ai_playerbot_enchants;
CREATE TABLE wotlkmangos.ai_playerbot_enchants LIKE wotlkplayerbots.ai_playerbot_enchants;
INSERT INTO wotlkmangos.ai_playerbot_enchants SELECT * FROM wotlkplayerbots.ai_playerbot_enchants;

DROP TABLE IF EXISTS wotlkmangos.ai_playerbot_enchants;
CREATE TABLE wotlkmangos.ai_playerbot_enchants LIKE wotlkplayerbots.ai_playerbot_enchants;
INSERT INTO wotlkmangos.ai_playerbot_enchants SELECT * FROM wotlkplayerbots.ai_playerbot_enchants;

DROP TABLE IF EXISTS wotlkcharacters.ai_playerbot_equip_cache;
CREATE TABLE wotlkcharacters.ai_playerbot_equip_cache LIKE wotlkplayerbots.ai_playerbot_equip_cache;
INSERT INTO wotlkcharacters.ai_playerbot_equip_cache SELECT * FROM wotlkplayerbots.ai_playerbot_equip_cache;

DROP TABLE IF EXISTS wotlkmangos.ai_playerbot_help_texts;
CREATE TABLE wotlkmangos.ai_playerbot_help_texts LIKE wotlkplayerbots.ai_playerbot_help_texts;
INSERT INTO wotlkmangos.ai_playerbot_help_texts SELECT * FROM wotlkplayerbots.ai_playerbot_help_texts;

DROP TABLE IF EXISTS wotlkcharacters.ai_playerbot_item_info_cache;
CREATE TABLE wotlkcharacters.ai_playerbot_item_info_cache LIKE wotlkplayerbots.ai_playerbot_item_info_cache;
INSERT INTO wotlkcharacters.ai_playerbot_item_info_cache SELECT * FROM wotlkplayerbots.ai_playerbot_item_info_cache;

DROP TABLE IF EXISTS wotlkcharacters.ai_playerbot_random_bots;
CREATE TABLE wotlkcharacters.ai_playerbot_random_bots LIKE wotlkplayerbots.ai_playerbot_random_bots;
INSERT INTO wotlkcharacters.ai_playerbot_random_bots SELECT * FROM wotlkplayerbots.ai_playerbot_random_bots;

DROP TABLE IF EXISTS wotlkcharacters.ai_playerbot_rarity_cache;
CREATE TABLE wotlkcharacters.ai_playerbot_rarity_cache LIKE wotlkplayerbots.ai_playerbot_rarity_cache;
INSERT INTO wotlkcharacters.ai_playerbot_rarity_cache SELECT * FROM wotlkplayerbots.ai_playerbot_rarity_cache;

DROP TABLE IF EXISTS wotlkcharacters.ai_playerbot_rnditem_cache;
CREATE TABLE wotlkcharacters.ai_playerbot_rnditem_cache LIKE wotlkplayerbots.ai_playerbot_rnditem_cache;
INSERT INTO wotlkcharacters.ai_playerbot_rnditem_cache SELECT * FROM wotlkplayerbots.ai_playerbot_rnditem_cache;

DROP TABLE IF EXISTS wotlkcharacters.ai_playerbot_tele_cache;
CREATE TABLE wotlkcharacters.ai_playerbot_tele_cache LIKE wotlkplayerbots.ai_playerbot_tele_cache;
INSERT INTO wotlkcharacters.ai_playerbot_tele_cache SELECT * FROM wotlkplayerbots.ai_playerbot_tele_cache;

DROP TABLE IF EXISTS wotlkcharacters.ai_playerbot_tele_cache;
CREATE TABLE wotlkcharacters.ai_playerbot_tele_cache LIKE wotlkplayerbots.ai_playerbot_tele_cache;
INSERT INTO wotlkcharacters.ai_playerbot_tele_cache SELECT * FROM wotlkplayerbots.ai_playerbot_tele_cache;

DROP TABLE IF EXISTS wotlkmangos.ai_playerbot_texts;
CREATE TABLE wotlkmangos.ai_playerbot_texts LIKE wotlkplayerbots.ai_playerbot_texts;
INSERT INTO wotlkmangos.ai_playerbot_texts SELECT * FROM wotlkplayerbots.ai_playerbot_texts;

DROP TABLE IF EXISTS wotlkmangos.ai_playerbot_texts_chance;
CREATE TABLE wotlkmangos.ai_playerbot_texts_chance LIKE wotlkplayerbots.ai_playerbot_texts_chance;
INSERT INTO wotlkmangos.ai_playerbot_texts_chance SELECT * FROM wotlkplayerbots.ai_playerbot_texts_chance;

DROP TABLE IF EXISTS wotlkmangos.ai_playerbot_travelnode;
CREATE TABLE wotlkmangos.ai_playerbot_travelnode LIKE wotlkplayerbots.ai_playerbot_travelnode;
INSERT INTO wotlkmangos.ai_playerbot_travelnode SELECT * FROM wotlkplayerbots.ai_playerbot_travelnode;

DROP TABLE IF EXISTS wotlkmangos.ai_playerbot_travelnode_link;
CREATE TABLE wotlkmangos.ai_playerbot_travelnode_link LIKE wotlkplayerbots.ai_playerbot_travelnode_link;
INSERT INTO wotlkmangos.ai_playerbot_travelnode_link SELECT * FROM wotlkplayerbots.ai_playerbot_travelnode_link;

DROP TABLE IF EXISTS wotlkmangos.ai_playerbot_travelnode_path;
CREATE TABLE wotlkmangos.ai_playerbot_travelnode_path LIKE wotlkplayerbots.ai_playerbot_travelnode_path;
INSERT INTO wotlkmangos.ai_playerbot_travelnode_path SELECT * FROM wotlkplayerbots.ai_playerbot_travelnode_path;

DROP TABLE IF EXISTS wotlkmangos.ai_playerbot_weightscales;
CREATE TABLE wotlkmangos.ai_playerbot_weightscales LIKE wotlkplayerbots.ai_playerbot_weightscales;
INSERT INTO wotlkmangos.ai_playerbot_weightscales SELECT * FROM wotlkplayerbots.ai_playerbot_weightscales;

DROP TABLE IF EXISTS wotlkmangos.ai_playerbot_weightscale_data;
CREATE TABLE wotlkmangos.ai_playerbot_weightscale_data LIKE wotlkplayerbots.ai_playerbot_weightscale_data;
INSERT INTO wotlkmangos.ai_playerbot_weightscale_data SELECT * FROM wotlkplayerbots.ai_playerbot_weightscale_data;

DROP TABLE IF EXISTS wotlkmangos.ai_playerbot_zone_level;
CREATE TABLE wotlkmangos.ai_playerbot_zone_level LIKE wotlkplayerbots.ai_playerbot_zone_level;
INSERT INTO wotlkmangos.ai_playerbot_zone_level SELECT * FROM wotlkplayerbots.ai_playerbot_zone_level;

DROP DATABASE IF EXISTS wotlkplayerbots;