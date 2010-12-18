ALTER TABLE db_version CHANGE COLUMN required_z1108_s0481_01_mangos_spell_proc_item_enchant required_z1108_s0481_02_mangos_spell_chain bit;

DELETE FROM `spell_chain` WHERE `spell_id` IN
(8034, 8037, 10458, 16352, 16353, 8680, 8685, 8689, 11335, 11336, 11337, 13218, 13222, 13223, 13224);

INSERT INTO `spell_chain` (`spell_id`, `prev_spell`, `first_spell`, `rank`, `req_spell`) VALUES
(8034,0,8034,1,0),     -- Frostbrand Attack
(8037,8034,8034,2,0),
(10458,8037,8034,3,0),
(16352,10458,8034,4,0),
(16353,16352,8034,5,0),
(8680,0,8680,1,0),     -- Instant Poison
(8685,8680,8680,2,0),
(8689,8685,8680,3,0),
(11335,8689,8680,4,0),
(11336,11335,8680,5,0),
(11337,11336,8680,6,0),
(13218,0,13218,1,0),   -- Wound Poison
(13222,13218,13218,2,0),
(13223,13222,13218,3,0),
(13224,13223,13218,4,0);
