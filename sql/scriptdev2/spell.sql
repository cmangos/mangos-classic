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
(19832,'spell_possess_razorgore'),
(19872,'spell_calm_dragonkin'),
(19873,'spell_destroy_egg'),
(20038,'spell_explosion_razorgore'),
(25684,'spell_summon_mana_fiends_moam'),
(25937,'spell_viscidus_freeze'),
(26084,'spell_aq_whirlwind'),
(26180,'spell_huhuran_wyvern_string'),
(26584,'spell_summon_toxic_slime'),
(26591,'spell_teleport_image'),
(26619,'spell_periodic_scarab_trigger'),
(26686,'spell_aq_whirlwind');

-- TBC

-- Wotlk


-- Priest


-- Warlock
INSERT INTO spell_scripts(Id, ScriptName) VALUES
(980,'spell_curse_of_agony'),
(1014,'spell_curse_of_agony'),
(6217,'spell_curse_of_agony'),
(11711,'spell_curse_of_agony'),
(11712,'spell_curse_of_agony'),
(11713,'spell_curse_of_agony'),
(27218,'spell_curse_of_agony');

-- Alterac Valley
INSERT INTO spell_scripts(Id, ScriptName) VALUES
(24677,'spell_gy_mid_trigger');

