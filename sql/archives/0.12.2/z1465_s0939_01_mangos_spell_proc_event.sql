ALTER TABLE db_version CHANGE COLUMN required_z1455_s0955_01_mangos_creature_equip_template required_z1465_s0939_01_mangos_spell_proc_event bit;

delete from `spell_proc_event` where entry = 16164;
insert into `spell_proc_event` values
(16164, 0x1C,  0, 0x0000000000000000, 0x00000000, 0x00000000, 0.000000, 0.000000,  0);
