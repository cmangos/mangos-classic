UPDATE `mangos_string` SET `content_default` = 'Spell %u %s = %f (*1.88 = %f) DB = %f' WHERE `entry` = 1202;

ALTER TABLE `spell_bonus_data` DROP COLUMN `ap_bonus`, DROP COLUMN `ap_dot_bonus`; 