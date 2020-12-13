DROP TABLE IF EXISTS spell_scripts;
CREATE TABLE spell_scripts(
Id INT NOT NULL COMMENT 'Spell ID',
ScriptName CHAR(64) NOT NULL COMMENT 'Core Spell Script Name',
PRIMARY KEY(Id)
);

-- Classic
INSERT INTO spell_scripts(Id, ScriptName) VALUES
(720,'spell_entangle_fankriss'),
(731,'spell_entangle_fankriss'),
(785,'spell_true_fulfillment'),
(794,'spell_initialize_images'),
(802,'spell_mutate_bug'),
(1121,'spell_entangle_fankriss'),
(3730,'spell_initialize_image'),
(7054,'spell_forsaken_skill'),
(13278,'spell_gdr_channel'),
(13493,'spell_gdr_periodic'),
(16380,'spell_greater_invisibility_mob'),
(17244,'spell_anastari_possess'),
(19832,'spell_possess_razorgore'),
(19872,'spell_calm_dragonkin'),
(19873,'spell_destroy_egg'),
(20038,'spell_explosion_razorgore'),
(21651,'spell_opening_capping'),
(23226,'spell_ritual_candle_aura'),
(25684,'spell_summon_mana_fiends_moam'),
(25937,'spell_viscidus_freeze'),
(26009,'spell_cthun_periodic_rotate'),
(26084,'spell_aq_whirlwind'),
(26136,'spell_cthun_periodic_rotate'),
(26137,'spell_cthun_rotate_trigger'),
(26152,'spell_cthun_periodic_eye_trigger'),
(26180,'spell_huhuran_wyvern_string'),
(26398,'spell_cthun_hook_tentacle'),
(26584,'spell_summon_toxic_slime'),
(26591,'spell_teleport_image'),
(26619,'spell_periodic_scarab_trigger'),
(26686,'spell_aq_whirlwind'),
(28282,'spell_ashbringer_item'),
(27808,'spell_kel_thuzad_frost_blast'),
(28522,'spell_sapphiron_icebolt_aura'),
(28526,'spell_sapphiron_icebolt'),
(28560,'spell_sapphiron_blizzard'),
(28007,'spell_summon_unrelenting'),
(28009,'spell_summon_unrelenting'),
(28011,'spell_summon_unrelenting'),
(28408,'spell_chains_kel_thuzad'),
(28415,'spell_trigger_KT_add'),
(28416,'spell_trigger_KT_add'),
(28417,'spell_trigger_KT_add'),
(28455,'spell_trigger_KT_add'),
(28749,'spell_check_gothik_side'),
(28781,'spell_gothik_side_assault'),
(29330,'spell_sapphiron_despawn_buffet'),
(29874,'spell_gothik_side_assault'),
(29875,'spell_check_gothik_side'),
(29897,'spell_icecrown_guardian_periodic'),
(30132,'spell_sapphiron_iceblock');

-- TBC

-- Wotlk


-- Priest
INSERT INTO spell_scripts(Id, ScriptName) VALUES
(10060,'spell_stacking_rules_override');

-- Warlock
INSERT INTO spell_scripts(Id, ScriptName) VALUES
(980,'spell_curse_of_agony'),
(1014,'spell_curse_of_agony'),
(6217,'spell_curse_of_agony'),
(11711,'spell_curse_of_agony'),
(11712,'spell_curse_of_agony'),
(11713,'spell_curse_of_agony'),
(1454,'spell_life_tap'),
(1455,'spell_life_tap'),
(1456,'spell_life_tap'),
(11687,'spell_life_tap'),
(11688,'spell_life_tap'),
(11689,'spell_life_tap');

-- Warrior
INSERT INTO spell_scripts(Id, ScriptName) VALUES
(5308,'spell_warrior_execute'),
(20658,'spell_warrior_execute'),
(20660,'spell_warrior_execute'),
(20661,'spell_warrior_execute'),
(20662,'spell_warrior_execute'),
(20647,'spell_warrior_execute_damage');

-- Mage
INSERT INTO spell_scripts(Id, ScriptName) VALUES
(11213,'spell_arcane_concentration'),
(12574,'spell_arcane_concentration'),
(12575,'spell_arcane_concentration'),
(12576,'spell_arcane_concentration'),
(12577,'spell_arcane_concentration');

-- Battleground and Outdoor PvP
-- Alterac Valley
INSERT INTO spell_scripts(Id, ScriptName) VALUES
-- Arathi basin
(23936,'spell_battleground_banner_trigger'),
(23932,'spell_battleground_banner_trigger'),
(23938,'spell_battleground_banner_trigger'),
(23935,'spell_battleground_banner_trigger'),
(23937,'spell_battleground_banner_trigger'),
-- Alterac Valley
(24677,'spell_battleground_banner_trigger');
