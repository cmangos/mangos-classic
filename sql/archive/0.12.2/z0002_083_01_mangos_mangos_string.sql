ALTER TABLE db_version CHANGE COLUMN required_z0001_xxx_02_mangos_db_version required_z0002_083_01_mangos_mangos_string bit;

DELETE FROM mangos_string WHERE entry in (1015);

INSERT INTO mangos_string VALUES
 (1015,'Used not fully typed quit command, need type it fully (quit), or command used not in RA command line.',NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
