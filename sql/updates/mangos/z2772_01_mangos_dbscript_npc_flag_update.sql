ALTER TABLE db_version CHANGE COLUMN required_z2771_01_mangos_creature_spawn_data required_z2772_01_mangos_dbscript_npc_flag_update bit;

 -- Temp toggle flag -> 4
UPDATE dbscripts_on_creature_movement SET datalong2=4 WHERE datalong2=0 AND command=29;
UPDATE dbscripts_on_creature_death    SET datalong2=4 WHERE datalong2=0 AND command=29;
UPDATE dbscripts_on_event             SET datalong2=4 WHERE datalong2=0 AND command=29;
UPDATE dbscripts_on_go_use            SET datalong2=4 WHERE datalong2=0 AND command=29;
UPDATE dbscripts_on_go_template_use   SET datalong2=4 WHERE datalong2=0 AND command=29;
UPDATE dbscripts_on_gossip            SET datalong2=4 WHERE datalong2=0 AND command=29;
UPDATE dbscripts_on_quest_end         SET datalong2=4 WHERE datalong2=0 AND command=29;
UPDATE dbscripts_on_quest_start       SET datalong2=4 WHERE datalong2=0 AND command=29;
UPDATE dbscripts_on_relay             SET datalong2=4 WHERE datalong2=0 AND command=29;
UPDATE dbscripts_on_spell             SET datalong2=4 WHERE datalong2=0 AND command=29;

 -- Remove flag -> 0
UPDATE dbscripts_on_creature_movement SET datalong2=0 WHERE datalong2=2 AND command=29;
UPDATE dbscripts_on_creature_death    SET datalong2=0 WHERE datalong2=2 AND command=29;
UPDATE dbscripts_on_event             SET datalong2=0 WHERE datalong2=2 AND command=29;
UPDATE dbscripts_on_go_use            SET datalong2=0 WHERE datalong2=2 AND command=29;
UPDATE dbscripts_on_go_template_use   SET datalong2=0 WHERE datalong2=2 AND command=29;
UPDATE dbscripts_on_gossip            SET datalong2=0 WHERE datalong2=2 AND command=29;
UPDATE dbscripts_on_quest_end         SET datalong2=0 WHERE datalong2=2 AND command=29;
UPDATE dbscripts_on_quest_start       SET datalong2=0 WHERE datalong2=2 AND command=29;
UPDATE dbscripts_on_relay             SET datalong2=0 WHERE datalong2=2 AND command=29;
UPDATE dbscripts_on_spell             SET datalong2=0 WHERE datalong2=2 AND command=29;

-- Toggle flag -> 2
UPDATE dbscripts_on_creature_movement SET datalong2=2 WHERE datalong2=4 AND command=29;
UPDATE dbscripts_on_creature_death    SET datalong2=2 WHERE datalong2=4 AND command=29;
UPDATE dbscripts_on_event             SET datalong2=2 WHERE datalong2=4 AND command=29;
UPDATE dbscripts_on_go_use            SET datalong2=2 WHERE datalong2=4 AND command=29;
UPDATE dbscripts_on_go_template_use   SET datalong2=2 WHERE datalong2=4 AND command=29;
UPDATE dbscripts_on_gossip            SET datalong2=2 WHERE datalong2=4 AND command=29;
UPDATE dbscripts_on_quest_end         SET datalong2=2 WHERE datalong2=4 AND command=29;
UPDATE dbscripts_on_quest_start       SET datalong2=2 WHERE datalong2=4 AND command=29;
UPDATE dbscripts_on_relay             SET datalong2=2 WHERE datalong2=4 AND command=29;
UPDATE dbscripts_on_spell             SET datalong2=2 WHERE datalong2=4 AND command=29;
