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
(2400,'spell_anubisath_share_powers'),
(3730,'spell_initialize_image'),
(7054,'spell_forsaken_skill'),
(12639,'spell_summon_hakkar'),
(12948,'spell_hakkar_summoned'),
(13278,'spell_gdr_channel'),
(13493,'spell_gdr_periodic'),
(16380,'spell_greater_invisibility_mob'),
(17016,'spell_placing_beacon_torch'),
(17244,'spell_anastari_possess'),
(19832,'spell_possess_razorgore'),
(19872,'spell_calm_dragonkin'),
(19873,'spell_destroy_egg'),
(20038,'spell_explosion_razorgore'),
(21651,'spell_opening_capping'),
(23226,'spell_ritual_candle_aura'),
(24693,'spell_hakkar_power_down'),
(25684,'spell_summon_mana_fiends_moam'),
(25813,'spell_conjure_dream_rift'),
(25885,'spell_viscidus_summon_globs'),
(25926,'spell_viscidus_frost_weakness'),
(25937,'spell_viscidus_freeze'),
(26003,'spell_viscidus_suicide'),
(26009,'spell_cthun_periodic_rotate'),
(26084,'spell_aq_whirlwind'),
(26136,'spell_cthun_periodic_rotate'),
(26137,'spell_cthun_rotate_trigger'),
(26152,'spell_cthun_periodic_eye_trigger'),
(26180,'spell_huhuran_wyvern_string'),
(26205,'spell_hook_tentacle_trigger'),
(26217,'spell_cthun_tentacles_summon'),
(26230,'spell_cthun_exit_stomach'),
(26237,'spell_cthun_tentacles_summon'),
(26256,'spell_cthun_check_reset'),
(26275,'spell_wondervolt_trap'),
(26332,'spell_cthun_mouth_tentacle'),
(26398,'spell_cthun_tentacles_summon'),
(26476,'spell_cthun_digestive_acid_periodic'),
(26584,'spell_summon_toxic_slime'),
(26585,'spell_viscidus_despawn_adds'),
(26591,'spell_teleport_image'),
(26608,'spell_viscidus_despawn_adds'),
(26619,'spell_periodic_scarab_trigger'),
(26686,'spell_aq_whirlwind'),
(26767,'spell_cthun_tentacles_summon'),
(26769,'spell_cthun_periodic_eye_trigger'),
(28282,'spell_ashbringer_item'),
(27808,'spell_kel_thuzad_frost_blast'),
(28307,'spell_patchwerk_hatefulstrike'),
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
(28441,'spell_ab_effect_000'),
(28732,'spell_faerlina_widow_embrace'),
(28749,'spell_check_gothik_side'),
(28781,'spell_gothik_side_assault'),
(29201,'spell_loatheb_corrupted_mind'),
(29330,'spell_sapphiron_despawn_buffet'),
(29874,'spell_gothik_side_assault'),
(29875,'spell_check_gothik_side'),
(29897,'spell_icecrown_guardian_periodic'),
(30132,'spell_sapphiron_iceblock');

-- TBC

-- Wotlk


-- Priest
INSERT INTO spell_scripts(Id, ScriptName) VALUES
(10060,'spell_power_infusion'),
(15268,'spell_blackout'),
(15323,'spell_blackout'),
(15324,'spell_blackout'),
(15325,'spell_blackout'),
(15326,'spell_blackout'),
(27827,'spell_spirit_of_redemption_heal');

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

-- Druid
INSERT INTO spell_scripts(Id, ScriptName) VALUES
(5215,'spell_stealth'),
(6783,'spell_stealth'),
(9913,'spell_stealth'),
(8936,'spell_regrowth'),
(8938,'spell_regrowth'),
(8939,'spell_regrowth'),
(8940,'spell_regrowth'),
(8941,'spell_regrowth'),
(9750,'spell_regrowth'),
(9856,'spell_regrowth'),
(9857,'spell_regrowth'),
(9858,'spell_regrowth');

-- Rogue
INSERT INTO spell_scripts(Id, ScriptName) VALUES
(1784,'spell_stealth'),
(1785,'spell_stealth'),
(1786,'spell_stealth'),
(1787,'spell_stealth'),
(1856,'spell_vanish'),
(1857,'spell_vanish'),
(14093,'spell_improved_sap'),
(14185,'spell_preparation');

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
