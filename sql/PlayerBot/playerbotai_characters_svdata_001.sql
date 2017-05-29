/*
Navicat MySQL Data Transfer

Source Server : Moo
Source Server Version : 50145
Source Host : localhost:****
Source Database : characters

Target Server Type : MYSQL
Target Server Version : 50145
File Encoding : yep

Date: 2012-03-05 20:25:14
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `playerbot_saved_data`
-- ----------------------------
DROP TABLE IF EXISTS `playerbot_saved_data`;
CREATE TABLE `playerbot_saved_data` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `bot_primary_order` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `bot_secondary_order` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `primary_target` int(11) unsigned NOT NULL DEFAULT '0',
  `secondary_target` int(11) unsigned NOT NULL DEFAULT '0',
  `pname` varchar(12) NOT NULL DEFAULT '',
  `sname` varchar(12) NOT NULL DEFAULT '',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of playerbot_saved_data
-- ----------------------------
