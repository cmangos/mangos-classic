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

ALTER TABLE `playerbot_saved_data`
    ADD COLUMN `combat_delay` INT(11) unsigned NOT NULL DEFAULT '0';
