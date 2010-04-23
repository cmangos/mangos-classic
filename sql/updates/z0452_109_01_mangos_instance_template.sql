ALTER TABLE db_version CHANGE COLUMN required_z0451_108_01_mangos_gameobject_template required_z0452_109_01_mangos_instance_template bit;

ALTER TABLE instance_template
  DROP COLUMN startLocX,
  DROP COLUMN startLocY,
  DROP COLUMN startLocZ,
  DROP COLUMN startLocO;

ALTER TABLE instance_template
  ADD COLUMN ghostEntranceMap smallint(5) NOT NULL default '-1' AFTER reset_delay,
  ADD COLUMN ghostEntranceX float NOT NULL default '0' AFTER ghostEntranceMap,
  ADD COLUMN ghostEntranceY float NOT NULL default '0' AFTER ghostEntranceX;

UPDATE instance_template SET ghostEntranceMap = 0, ghostEntranceX = -230.989,  ghostEntranceY =  1571.57  WHERE map = 33;
UPDATE instance_template SET ghostEntranceMap = 0, ghostEntranceX = -8762.379, ghostEntranceY =  848.01   WHERE map = 34;
UPDATE instance_template SET ghostEntranceMap = 0, ghostEntranceX = -11207.799,ghostEntranceY =  1681.151 WHERE map = 36;
UPDATE instance_template SET ghostEntranceMap = 1, ghostEntranceX = -751.131,  ghostEntranceY = -2209.24  WHERE map = 43;
UPDATE instance_template SET ghostEntranceMap = 1, ghostEntranceX = -4459.449, ghostEntranceY = -1660.212 WHERE map = 47;
UPDATE instance_template SET ghostEntranceMap = 1, ghostEntranceX =  4249.121, ghostEntranceY =  748.387  WHERE map = 48;
UPDATE instance_template SET ghostEntranceMap = 0, ghostEntranceX = -6060.182, ghostEntranceY = -2954.997 WHERE map = 70;
UPDATE instance_template SET ghostEntranceMap = 0, ghostEntranceX = -5162.662, ghostEntranceY =  931.599  WHERE map = 90;
UPDATE instance_template SET ghostEntranceMap = 0, ghostEntranceX = -10170.104,ghostEntranceY = -3995.968 WHERE map = 109;
UPDATE instance_template SET ghostEntranceMap = 1, ghostEntranceX = -4662.883, ghostEntranceY = -2535.872 WHERE map = 129;
UPDATE instance_template SET ghostEntranceMap = 0, ghostEntranceX =  2892.235, ghostEntranceY = -811.264  WHERE map = 189;
UPDATE instance_template SET ghostEntranceMap = 1, ghostEntranceX = -6790.577, ghostEntranceY = -2891.278 WHERE map = 209;
UPDATE instance_template SET ghostEntranceMap = 0, ghostEntranceX = -7522.527, ghostEntranceY = -1233.042 WHERE map = 229;
UPDATE instance_template SET ghostEntranceMap = 0, ghostEntranceX = -7178.095, ghostEntranceY = -928.639  WHERE map = 230;
UPDATE instance_template SET ghostEntranceMap = 1, ghostEntranceX = -4753.313, ghostEntranceY = -3752.421 WHERE map = 249;
UPDATE instance_template SET ghostEntranceMap = 0, ghostEntranceX =  1274.781, ghostEntranceY = -2552.564 WHERE map = 289;
UPDATE instance_template SET ghostEntranceMap = 0, ghostEntranceX = -11916.06, ghostEntranceY = -1224.577 WHERE map = 309;
UPDATE instance_template SET ghostEntranceMap = 0, ghostEntranceX =  3392.317, ghostEntranceY = -3378.483 WHERE map = 329;
UPDATE instance_template SET ghostEntranceMap = 1, ghostEntranceX = -1432.697, ghostEntranceY =  2924.978 WHERE map = 349;
UPDATE instance_template SET ghostEntranceMap = 1, ghostEntranceX =  1816.756, ghostEntranceY = -4423.372 WHERE map = 389;
UPDATE instance_template SET ghostEntranceMap = 0, ghostEntranceX = -7510.565, ghostEntranceY = -1036.698 WHERE map = 409;
UPDATE instance_template SET ghostEntranceMap = 1, ghostEntranceX = -3908.032, ghostEntranceY =  1130.004 WHERE map = 429;
UPDATE instance_template SET ghostEntranceMap = 0, ghostEntranceX = -7663.413, ghostEntranceY = -1218.667 WHERE map = 469;
UPDATE instance_template SET ghostEntranceMap = 1, ghostEntranceX = -8114.464, ghostEntranceY =  1526.366 WHERE map = 509;
UPDATE instance_template SET ghostEntranceMap = 1, ghostEntranceX = -8111.724, ghostEntranceY =  1526.786 WHERE map = 531;
