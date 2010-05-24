ALTER TABLE db_version CHANGE COLUMN required_z0537_118_02_mangos_command required_z0540_119_01_mangos_quest_template bit;

ALTER TABLE quest_template
  DROP COLUMN ReqSourceRef1,
  DROP COLUMN ReqSourceRef2,
  DROP COLUMN ReqSourceRef3,
  DROP COLUMN ReqSourceRef4;