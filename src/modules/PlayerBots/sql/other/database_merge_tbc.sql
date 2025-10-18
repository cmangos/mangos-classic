DROP TABLE IF EXISTS tbccharacters.ahbot_history;
CREATE TABLE tbccharacters.ahbot_history LIKE tbcplayerbots.ahbot_history;
INSERT INTO tbccharacters.ahbot_history SELECT * FROM tbcplayerbots.ahbot_history;

DROP TABLE IF EXISTS tbccharacters.ahbot_category;
CREATE TABLE tbccharacters.ahbot_category LIKE tbcplayerbots.ahbot_category;
INSERT INTO tbccharacters.ahbot_category SELECT * FROM tbcplayerbots.ahbot_category;

DROP TABLE IF EXISTS tbccharacters.ahbot_price;
CREATE TABLE tbccharacters.ahbot_price LIKE tbcplayerbots.ahbot_price;
INSERT INTO tbccharacters.ahbot_price SELECT * FROM tbcplayerbots.ahbot_price;

DROP TABLE IF EXISTS tbccharacters.ai_playerbot_custom_strategy;
CREATE TABLE tbccharacters.ai_playerbot_custom_strategy LIKE tbcplayerbots.ai_playerbot_custom_strategy;
INSERT INTO tbccharacters.ai_playerbot_custom_strategy SELECT * FROM tbcplayerbots.ai_playerbot_custom_strategy;

DROP TABLE IF EXISTS tbccharacters.ai_playerbot_db_store;
CREATE TABLE tbccharacters.ai_playerbot_db_store LIKE tbcplayerbots.ai_playerbot_db_store;
INSERT INTO tbccharacters.ai_playerbot_db_store SELECT * FROM tbcplayerbots.ai_playerbot_db_store;

DROP TABLE IF EXISTS tbcmangos.ai_playerbot_enchants;
CREATE TABLE tbcmangos.ai_playerbot_enchants LIKE tbcplayerbots.ai_playerbot_enchants;
INSERT INTO tbcmangos.ai_playerbot_enchants SELECT * FROM tbcplayerbots.ai_playerbot_enchants;

DROP TABLE IF EXISTS tbcmangos.ai_playerbot_enchants;
CREATE TABLE tbcmangos.ai_playerbot_enchants LIKE tbcplayerbots.ai_playerbot_enchants;
INSERT INTO tbcmangos.ai_playerbot_enchants SELECT * FROM tbcplayerbots.ai_playerbot_enchants;

DROP TABLE IF EXISTS tbccharacters.ai_playerbot_equip_cache;
CREATE TABLE tbccharacters.ai_playerbot_equip_cache LIKE tbcplayerbots.ai_playerbot_equip_cache;
INSERT INTO tbccharacters.ai_playerbot_equip_cache SELECT * FROM tbcplayerbots.ai_playerbot_equip_cache;

DROP TABLE IF EXISTS tbcmangos.ai_playerbot_help_texts;
CREATE TABLE tbcmangos.ai_playerbot_help_texts LIKE tbcplayerbots.ai_playerbot_help_texts;
INSERT INTO tbcmangos.ai_playerbot_help_texts SELECT * FROM tbcplayerbots.ai_playerbot_help_texts;

DROP TABLE IF EXISTS tbccharacters.ai_playerbot_item_info_cache;
CREATE TABLE tbccharacters.ai_playerbot_item_info_cache LIKE tbcplayerbots.ai_playerbot_item_info_cache;
INSERT INTO tbccharacters.ai_playerbot_item_info_cache SELECT * FROM tbcplayerbots.ai_playerbot_item_info_cache;

DROP TABLE IF EXISTS tbccharacters.ai_playerbot_random_bots;
CREATE TABLE tbccharacters.ai_playerbot_random_bots LIKE tbcplayerbots.ai_playerbot_random_bots;
INSERT INTO tbccharacters.ai_playerbot_random_bots SELECT * FROM tbcplayerbots.ai_playerbot_random_bots;

DROP TABLE IF EXISTS tbccharacters.ai_playerbot_rarity_cache;
CREATE TABLE tbccharacters.ai_playerbot_rarity_cache LIKE tbcplayerbots.ai_playerbot_rarity_cache;
INSERT INTO tbccharacters.ai_playerbot_rarity_cache SELECT * FROM tbcplayerbots.ai_playerbot_rarity_cache;

DROP TABLE IF EXISTS tbccharacters.ai_playerbot_rnditem_cache;
CREATE TABLE tbccharacters.ai_playerbot_rnditem_cache LIKE tbcplayerbots.ai_playerbot_rnditem_cache;
INSERT INTO tbccharacters.ai_playerbot_rnditem_cache SELECT * FROM tbcplayerbots.ai_playerbot_rnditem_cache;

DROP TABLE IF EXISTS tbccharacters.ai_playerbot_tele_cache;
CREATE TABLE tbccharacters.ai_playerbot_tele_cache LIKE tbcplayerbots.ai_playerbot_tele_cache;
INSERT INTO tbccharacters.ai_playerbot_tele_cache SELECT * FROM tbcplayerbots.ai_playerbot_tele_cache;

DROP TABLE IF EXISTS tbccharacters.ai_playerbot_tele_cache;
CREATE TABLE tbccharacters.ai_playerbot_tele_cache LIKE tbcplayerbots.ai_playerbot_tele_cache;
INSERT INTO tbccharacters.ai_playerbot_tele_cache SELECT * FROM tbcplayerbots.ai_playerbot_tele_cache;

DROP TABLE IF EXISTS tbcmangos.ai_playerbot_texts;
CREATE TABLE tbcmangos.ai_playerbot_texts LIKE tbcplayerbots.ai_playerbot_texts;
INSERT INTO tbcmangos.ai_playerbot_texts SELECT * FROM tbcplayerbots.ai_playerbot_texts;

DROP TABLE IF EXISTS tbcmangos.ai_playerbot_texts_chance;
CREATE TABLE tbcmangos.ai_playerbot_texts_chance LIKE tbcplayerbots.ai_playerbot_texts_chance;
INSERT INTO tbcmangos.ai_playerbot_texts_chance SELECT * FROM tbcplayerbots.ai_playerbot_texts_chance;

DROP TABLE IF EXISTS tbcmangos.ai_playerbot_travelnode;
CREATE TABLE tbcmangos.ai_playerbot_travelnode LIKE tbcplayerbots.ai_playerbot_travelnode;
INSERT INTO tbcmangos.ai_playerbot_travelnode SELECT * FROM tbcplayerbots.ai_playerbot_travelnode;

DROP TABLE IF EXISTS tbcmangos.ai_playerbot_travelnode_link;
CREATE TABLE tbcmangos.ai_playerbot_travelnode_link LIKE tbcplayerbots.ai_playerbot_travelnode_link;
INSERT INTO tbcmangos.ai_playerbot_travelnode_link SELECT * FROM tbcplayerbots.ai_playerbot_travelnode_link;

DROP TABLE IF EXISTS tbcmangos.ai_playerbot_travelnode_path;
CREATE TABLE tbcmangos.ai_playerbot_travelnode_path LIKE tbcplayerbots.ai_playerbot_travelnode_path;
INSERT INTO tbcmangos.ai_playerbot_travelnode_path SELECT * FROM tbcplayerbots.ai_playerbot_travelnode_path;

DROP TABLE IF EXISTS tbcmangos.ai_playerbot_weightscales;
CREATE TABLE tbcmangos.ai_playerbot_weightscales LIKE tbcplayerbots.ai_playerbot_weightscales;
INSERT INTO tbcmangos.ai_playerbot_weightscales SELECT * FROM tbcplayerbots.ai_playerbot_weightscales;

DROP TABLE IF EXISTS tbcmangos.ai_playerbot_weightscale_data;
CREATE TABLE tbcmangos.ai_playerbot_weightscale_data LIKE tbcplayerbots.ai_playerbot_weightscale_data;
INSERT INTO tbcmangos.ai_playerbot_weightscale_data SELECT * FROM tbcplayerbots.ai_playerbot_weightscale_data;

DROP TABLE IF EXISTS tbcmangos.ai_playerbot_zone_level;
CREATE TABLE tbcmangos.ai_playerbot_zone_level LIKE tbcplayerbots.ai_playerbot_zone_level;
INSERT INTO tbcmangos.ai_playerbot_zone_level SELECT * FROM tbcplayerbots.ai_playerbot_zone_level;

DROP DATABASE IF EXISTS tbcplayerbots;