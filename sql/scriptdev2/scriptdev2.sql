/* INITIAL CLEANUP */
-- Clear all ScriptNames
-- This will clear all ScriptNames from any table in the World-Database

TRUNCATE scripted_areatrigger;
TRUNCATE scripted_event_id;
UPDATE creature_template SET ScriptName='';
UPDATE gameobject_template SET ScriptName='';
UPDATE item_template SET ScriptName='';
UPDATE instance_template SET ScriptName='';
UPDATE world_template SET ScriptName='';
/*  */

/* AREATRIGGER */
INSERT INTO scripted_areatrigger VALUES
(171, 'at_huldar_miran'),
(1726,'at_scent_larkorwi'),
(1727,'at_scent_larkorwi'),
(1728,'at_scent_larkorwi'),
(1729,'at_scent_larkorwi'),
(1730,'at_scent_larkorwi'),
(1731,'at_scent_larkorwi'),
(1732,'at_scent_larkorwi'),
(1733,'at_scent_larkorwi'),
(1734,'at_scent_larkorwi'),
(1735,'at_scent_larkorwi'),
(1736,'at_scent_larkorwi'),
(1737,'at_scent_larkorwi'),
(1738,'at_scent_larkorwi'),
(1739,'at_scent_larkorwi'),
(1740,'at_scent_larkorwi'),
(1966,'at_murkdeep'),
(2026,'at_blackrock_spire'),
(2046,'at_blackrock_spire'),
(2066,'at_blackrock_spire'),
(2067,'at_blackrock_spire'),
(3066,'at_ravenholdt'),
(3146,'at_hive_tower'),
(3546,'at_childrens_week_spot'), -- Darnassian bank
(3547,'at_childrens_week_spot'), -- Undercity - thone room
(3548,'at_childrens_week_spot'), -- Stonewrought Dam
(3549,'at_childrens_week_spot'), -- The Mor'shan Rampart
(3550,'at_childrens_week_spot'), -- Ratchet Docks
(3552,'at_childrens_week_spot'), -- Westfall Lighthouse
(3587,'at_ancient_leaf'),
(3626,'at_vaelastrasz'),
(3958,'at_zulgurub'),
(3960,'at_zulgurub'),
(4017,'at_twilight_grove'),
(4047,'at_temple_ahnqiraj'),
(4052,'at_temple_ahnqiraj'),
(4112,'at_naxxramas'),
(4113,'at_naxxramas'),
(4156,'at_naxxramas');

/* BATTLEGROUNDS */
UPDATE creature_template SET ScriptName='npc_spirit_guide' WHERE entry IN (13116, 13117);

/* WORLD BOSS */
UPDATE creature_template SET ScriptName='boss_ysondre' WHERE entry=14887;
UPDATE creature_template SET ScriptName='boss_emeriss' WHERE entry=14889;
UPDATE creature_template SET ScriptName='boss_taerar' WHERE entry=14890;
UPDATE creature_template SET ScriptName='boss_azuregos' WHERE entry=6109;
UPDATE creature_template SET ScriptName='boss_lethon' WHERE entry=14888;
UPDATE creature_template SET ScriptName='npc_spirit_shade' WHERE entry=15261;

/* GO */
UPDATE gameobject_template SET ScriptName='go_andorhal_tower' WHERE entry IN (176094,176095,176096,176097);
UPDATE gameobject_template SET ScriptName='go_veil_skith_cage' WHERE entry IN (185202,185203,185204,185205);
UPDATE gameobject_template SET ScriptName='go_bells' WHERE entry IN(175885,176573);
UPDATE gameobject_template SET ScriptName='go_darkmoon_faire_music' WHERE entry=180335;
UPDATE gameobject_template SET ScriptName='go_elemental_rift' WHERE entry IN (179664, 179665, 179666, 179667);
UPDATE gameobject_template SET ScriptName='go_dragon_head' WHERE entry IN(179556,179558,179881,179882);
UPDATE gameobject_template SET ScriptName='go_unadorned_spike' WHERE entry IN(175787);

/* GUARD */
UPDATE creature_template SET ScriptName='guard_orgrimmar' WHERE entry=3296;
UPDATE creature_template SET ScriptName='guard_stormwind' WHERE entry IN (68,1976);
UPDATE creature_template SET ScriptName='guard_contested' WHERE entry IN (9460,4624,3502,11190,15184);
UPDATE creature_template SET ScriptName='guard_elwynnforest' WHERE entry=1423;
UPDATE creature_template SET ScriptName='guard_darnassus' WHERE entry=4262;
UPDATE creature_template SET ScriptName='guard_teldrassil' WHERE entry=3571;
UPDATE creature_template SET ScriptName='guard_ironforge' WHERE entry=5595;
UPDATE creature_template SET ScriptName='guard_dunmorogh' WHERE entry IN (727,13076);
UPDATE creature_template SET ScriptName='guard_undercity' WHERE entry=5624;
UPDATE creature_template SET ScriptName='guard_bluffwatcher' WHERE entry=3084;
UPDATE creature_template SET ScriptName='guard_durotar' WHERE entry=5953;
UPDATE creature_template SET ScriptName='guard_mulgore' WHERE entry IN (3212,3215,3217,3218,3219,3220,3221,3222,3223,3224);
UPDATE creature_template SET ScriptName='guard_dunmorogh' WHERE entry IN (727,13076);
UPDATE creature_template SET ScriptName='guard_tirisfal' WHERE entry IN (1735,1738,2210,1744,1745,5725,1743,2209,1746,1742);

/* ITEM */
UPDATE item_template SET ScriptName='item_orb_of_draconic_energy' WHERE entry=12300;

/* NPC (usually creatures to be found in more than one specific zone) */
UPDATE creature_template SET ScriptName='npc_chicken_cluck' WHERE entry=620;
UPDATE creature_template SET ScriptName='npc_dancing_flames' WHERE entry=25305;
UPDATE creature_template SET ScriptName='npc_garments_of_quests' WHERE entry IN (12429,12423,12427,12430,12428);
UPDATE creature_template SET ScriptName='npc_guardian' WHERE entry=5764;
UPDATE creature_template SET ScriptName='npc_doctor' WHERE entry IN (12939,12920);
UPDATE creature_template SET ScriptName='npc_injured_patient' WHERE entry IN (12936,12937,12938,12923,12924,12925);
UPDATE creature_template SET ScriptName='npc_prof_blacksmith' WHERE entry IN (5164,11145,11146,11176,11177,11178,11191,11192,11193);
UPDATE creature_template SET ScriptName='npc_prof_leather' WHERE entry IN (7866,7867,7868,7869,7870,7871);
-- disabled, but can be used for custom
-- UPDATE creature_template SET ScriptName='' WHERE npcflag!=npcflag|65536 AND ScriptName='npc_innkeeper';
-- UPDATE creature_template SET ScriptName='npc_innkeeper' WHERE npcflag=npcflag|65536;
UPDATE creature_template SET ScriptName='npc_redemption_target' WHERE entry IN (6172,6177,17542,17768);
UPDATE creature_template SET ScriptName='npc_the_cleaner' WHERE entry=14503;

/* SPELL */
UPDATE creature_template SET ScriptName='spell_dummy_npc' WHERE entry IN (
-- eastern kingdoms
1200,8888,13016,
-- kalimdor
9299,12296,12298);

/* WORLD MAP SCRIPTS */
DELETE FROM world_template WHERE map IN (0, 1, 530);
INSERT INTO world_template VALUES
(0, 'world_map_eastern_kingdoms'),
(1, 'world_map_kalimdor');

/*  */
/* ZONE */
/* */

/* ALTERAC MOUNTAINS */

/* ALTERAC VALLEY */

/* ARATHI HIGHLANDS */
UPDATE creature_template SET ScriptName='npc_professor_phizzlethorpe' WHERE entry=2768;
UPDATE creature_template SET ScriptName='npc_kinelory' WHERE entry=2713;

/* ASHENVALE */
UPDATE creature_template SET ScriptName='npc_muglash' WHERE entry=12717;
UPDATE gameobject_template SET ScriptName='go_naga_brazier' WHERE entry=178247;
UPDATE creature_template SET ScriptName='npc_ruul_snowhoof' WHERE entry=12818;
UPDATE creature_template SET ScriptName='npc_torek' WHERE entry=12858;
UPDATE creature_template SET ScriptName='npc_feero_ironhand' WHERE entry=4484;

/* AZSHARA */
UPDATE creature_template SET ScriptName='mobs_spitelashes' WHERE entry IN (6190,6193,6194,6195,6196);
UPDATE creature_template SET ScriptName='npc_loramus_thalipedes' WHERE entry=7783;
UPDATE creature_template SET ScriptName='npc_felhound_tracker' WHERE entry=8668;
UPDATE creature_template SET ScriptName='boss_maws' WHERE entry=15571;
UPDATE gameobject_template SET ScriptName='go_lightning' WHERE entry=183356;
INSERT INTO scripted_event_id VALUES
(9542,'event_arcanite_buoy');

/* BADLANDS */

/* BARRENS */
UPDATE creature_template SET ScriptName='npc_gilthares' WHERE entry=3465;
UPDATE creature_template SET ScriptName='npc_taskmaster_fizzule' WHERE entry=7233;
UPDATE creature_template SET ScriptName='npc_twiggy_flathead' WHERE entry=6248;
INSERT INTO scripted_areatrigger VALUES
(522,'at_twiggy_flathead');
UPDATE creature_template SET ScriptName='npc_wizzlecranks_shredder' WHERE entry=3439;
UPDATE creature_template SET ScriptName='npc_gallywix' WHERE entry=7288;
UPDATE creature_template SET ScriptName = 'npc_regthar_deathgate' WHERE entry = 3389;

/* BLACKFATHOM DEPTHS */
UPDATE instance_template SET ScriptName='instance_blackfathom_deeps' WHERE map=48;
UPDATE gameobject_template SET ScriptName='go_fire_of_akumai' WHERE entry IN (21118,21119,21120,21121);
UPDATE gameobject_template SET ScriptName='go_fathom_stone' WHERE entry=177964;

/* BLACKROCK DEPTHS */

INSERT INTO scripted_areatrigger VALUES
(1526,'at_ring_of_law'),
(1786,'at_shadowforge_bridge');
UPDATE instance_template SET ScriptName='instance_blackrock_depths' WHERE map =230;
UPDATE creature_template SET ScriptName='boss_emperor_dagran_thaurissan' WHERE entry=9019;
UPDATE creature_template SET ScriptName='boss_moira_bronzebeard' WHERE entry=8929;
UPDATE creature_template SET ScriptName='boss_ambassador_flamelash' WHERE entry=9156;
UPDATE creature_template SET ScriptName='boss_doomrel' WHERE entry=9039;
UPDATE creature_template SET ScriptName='boss_general_angerforge' WHERE entry=9033;
UPDATE creature_template SET ScriptName='boss_high_interrogator_gerstahn' WHERE entry=9018;
UPDATE creature_template SET ScriptName='npc_grimstone' WHERE entry=10096;
UPDATE creature_template SET ScriptName='npc_theldren_trigger' WHERE entry=16079;
UPDATE creature_template SET ScriptName='npc_phalanx' WHERE entry=9502;
UPDATE creature_template SET ScriptName='npc_mistress_nagmara' WHERE entry=9500;
UPDATE creature_template SET ScriptName='npc_rocknot' WHERE entry=9503;
UPDATE creature_template SET ScriptName='npc_marshal_windsor' WHERE entry=9023;
UPDATE creature_template SET ScriptName='npc_dughal_stormwing' WHERE entry=9022;
UPDATE creature_template SET ScriptName='npc_tobias_seecher' WHERE entry=9679;
UPDATE creature_template SET ScriptName='npc_hurley_blackbreath' WHERE entry=9537;
UPDATE creature_template SET ScriptName='boss_plugger_spazzring' WHERE entry=9499;
UPDATE creature_template SET ScriptName='npc_ironhand_guardian' WHERE entry=8982;
UPDATE gameobject_template SET ScriptName='go_shadowforge_brazier' WHERE entry IN (174744, 174745);
UPDATE gameobject_template SET ScriptName='go_relic_coffer_door' WHERE entry IN (174554, 174555, 174556, 174557, 174558, 174559, 174560, 174561, 174562, 174563, 174564, 174566);
UPDATE gameobject_template SET ScriptName='go_bar_beer_keg' WHERE entry=164911;
UPDATE gameobject_template SET ScriptName='go_bar_ale_mug' WHERE entry IN (165738,165739);

/* BLACKROCK SPIRE */
UPDATE instance_template SET ScriptName='instance_blackrock_spire' WHERE map=229;
UPDATE creature_template SET ScriptName='boss_overlord_wyrmthalak' WHERE entry=9568;
UPDATE creature_template SET ScriptName='boss_gyth' WHERE entry=10339;
UPDATE creature_template SET ScriptName='boss_pyroguard_emberseer' WHERE entry=9816;
UPDATE creature_template SET ScriptName='npc_rookery_hatcher' WHERE entry=10683;
INSERT INTO scripted_event_id VALUES
(4884,'event_spell_altar_emberseer');
UPDATE gameobject_template SET ScriptName='go_father_flame' WHERE entry=175245;

/* BLACKWING LAIR */
UPDATE instance_template SET ScriptName='instance_blackwing_lair' WHERE map=469;
UPDATE creature_template SET ScriptName='boss_razorgore' WHERE entry=12435;
UPDATE creature_template SET ScriptName='npc_blackwing_orb' WHERE entry=14449;
UPDATE gameobject_template SET ScriptName='go_black_dragon_egg' WHERE entry=177807;
UPDATE creature_template SET ScriptName='boss_vaelastrasz' WHERE entry=13020;
UPDATE gameobject_template SET ScriptName='go_suppression' WHERE entry=179784;
UPDATE creature_template SET ScriptName='boss_broodlord' WHERE entry=12017;
UPDATE creature_template SET ScriptName='boss_firemaw' WHERE entry=11983;
UPDATE creature_template SET ScriptName='boss_ebonroc' WHERE entry=14601;
UPDATE creature_template SET ScriptName='boss_flamegor' WHERE entry=11981;
UPDATE creature_template SET ScriptName='boss_chromaggus' WHERE entry=14020;
UPDATE creature_template SET ScriptName='boss_victor_nefarius' WHERE entry=10162;
UPDATE creature_template SET ScriptName='boss_nefarian' WHERE entry=11583;
INSERT INTO scripted_event_id VALUES
(8446, 'event_weekly_chromatic_selection'),
(8447, 'event_weekly_chromatic_selection'),
(8448, 'event_weekly_chromatic_selection'),
(8449, 'event_weekly_chromatic_selection'),
(8450, 'event_weekly_chromatic_selection'),
(8451, 'event_weekly_chromatic_selection'),
(8452, 'event_weekly_chromatic_selection'),
(8453, 'event_weekly_chromatic_selection'),
(8454, 'event_weekly_chromatic_selection'),
(8455, 'event_weekly_chromatic_selection'),
(8520, 'event_weekly_chromatic_selection'),
(8521, 'event_weekly_chromatic_selection'),
(8522, 'event_weekly_chromatic_selection'),
(8523, 'event_weekly_chromatic_selection'),
(8524, 'event_weekly_chromatic_selection'),
(8525, 'event_weekly_chromatic_selection'),
(8526, 'event_weekly_chromatic_selection'),
(8527, 'event_weekly_chromatic_selection'),
(8528, 'event_weekly_chromatic_selection'),
(8529, 'event_weekly_chromatic_selection');


/* BLASTED LANDS */
UPDATE creature_template SET ScriptName='boss_kazzak' WHERE entry=12397;

/* BURNING STEPPES */
UPDATE creature_template SET ScriptName='npc_grark_lorkrub' WHERE entry=9520;
UPDATE creature_template SET ScriptName='npc_klinfran' WHERE entry IN (14529,14534);

/* DARKSHORE */
UPDATE creature_template SET ScriptName='npc_kerlonian' WHERE entry=11218;
UPDATE creature_template SET ScriptName='npc_prospector_remtravel' WHERE entry=2917;
UPDATE creature_template SET ScriptName='npc_threshwackonator' WHERE entry=6669;
UPDATE creature_template SET ScriptName='npc_volcor' WHERE entry=3692;
UPDATE creature_template SET ScriptName='npc_therylune' WHERE entry=3584;
UPDATE creature_template SET ScriptName='npc_rabid_bear' WHERE entry=2164;

/* DARNASSUS */

/* DEADMINES */
UPDATE creature_template SET ScriptName='boss_mr_smite' WHERE entry=646;
UPDATE instance_template SET ScriptName='instance_deadmines' WHERE map=36;
UPDATE gameobject_template SET ScriptName='go_defias_cannon' WHERE entry=16398;

/* DEADWIND PASS */

/* DESOLACE */
UPDATE creature_template SET ScriptName='npc_aged_dying_ancient_kodo' WHERE entry IN (4700, 4701, 4702, 11627);
UPDATE creature_template SET ScriptName='npc_dalinda_malem' WHERE entry=5644;
UPDATE creature_template SET ScriptName='npc_melizza_brimbuzzle' WHERE entry=12277;
UPDATE creature_template SET ScriptName='npc_cork_gizelton' WHERE entry=11625;
UPDATE creature_template SET ScriptName='npc_rigger_gizelton' WHERE entry=11626;
UPDATE creature_template SET ScriptName='npc_magrami_spectre' WHERE entry=11560;

/* DIRE MAUL */
UPDATE instance_template SET ScriptName='instance_dire_maul' WHERE map=429;
UPDATE gameobject_template SET ScriptName='go_fixed_trap' WHERE entry=179512;
UPDATE creature_template SET ScriptName='npc_mizzle_crafty' WHERE entry=14353;
INSERT INTO scripted_event_id VALUES
(8420,'event_spells_warlock_dreadsteed'),
(8428,'event_spells_warlock_dreadsteed');

/* DUN MOROGH */

/* DUROTAR */
UPDATE creature_template SET ScriptName='npc_lazy_peon' WHERE entry=10556;

/* DUSKWOOD */

/* DUSTWALLOW MARSH */
UPDATE creature_template SET ScriptName='npc_ogron' WHERE entry=4983;
UPDATE creature_template SET ScriptName='npc_morokk' WHERE entry=4500;
UPDATE creature_template SET ScriptName='npc_private_hendel' WHERE entry=4966;
UPDATE creature_template SET ScriptName='npc_stinky_ignatz' WHERE entry=4880;
INSERT INTO scripted_areatrigger VALUES
(302,'at_sentry_point');

/* EASTERN PLAGUELANDS */
UPDATE creature_template SET ScriptName='npc_eris_havenfire' WHERE entry=14494;

/* ELWYNN FOREST */

/* FELWOOD */
INSERT INTO scripted_event_id VALUES
(8328, 'npc_kroshius');
UPDATE creature_template SET ScriptName='npc_kitten' WHERE entry=9937;
UPDATE creature_template SET ScriptName='npc_corrupt_saber' WHERE entry=10042;
UPDATE creature_template SET ScriptName='npc_niby_the_almighty' WHERE entry=14469;
UPDATE creature_template SET ScriptName='npc_kroshius' WHERE entry=14467;
UPDATE creature_template SET ScriptName='npc_captured_arkonarin' WHERE entry=11016;
UPDATE creature_template SET ScriptName='npc_arei' WHERE entry=9598;

/* FERALAS */
UPDATE creature_template SET ScriptName='npc_oox22fe' WHERE entry=7807;
UPDATE creature_template SET ScriptName='npc_shay_leafrunner' WHERE entry=7774;

/* GNOMEREGAN */
UPDATE creature_template SET ScriptName='boss_thermaplugg' WHERE entry=7800;
UPDATE gameobject_template SET ScriptName='go_gnomeface_button' WHERE entry BETWEEN 142214 AND 142219;
UPDATE creature_template SET ScriptName='npc_blastmaster_emi_shortfuse' WHERE entry=7998;
UPDATE creature_template SET ScriptName='npc_kernobee' WHERE entry=7850;
UPDATE instance_template SET ScriptName='instance_gnomeregan' WHERE map=90;

/* HILLSBRAD FOOTHILLS */

/* HINTERLANDS */
UPDATE creature_template SET ScriptName='npc_00x09hl' WHERE entry=7806;
UPDATE creature_template SET ScriptName='npc_rinji' WHERE entry=7780;

/* IRONFORGE */

/* LOCH MODAN */
UPDATE creature_template SET ScriptName='npc_mountaineer_pebblebitty' WHERE entry=3836;
UPDATE creature_template SET ScriptName='npc_miran' WHERE entry=1379;

/* MARAUDON */
UPDATE instance_template SET ScriptName="instance_maraudon" WHERE map=349;
UPDATE creature_template SET ScriptName='boss_noxxion' WHERE entry=13282;
UPDATE gameobject_template SET ScriptName="go_larva_spewer" WHERE entry=178559;

/* MOLTEN CORE */
UPDATE instance_template SET ScriptName='instance_molten_core' WHERE map=409;
UPDATE creature_template SET ScriptName='boss_lucifron' WHERE entry=12118;
UPDATE creature_template SET ScriptName='boss_magmadar' WHERE entry=11982;
UPDATE creature_template SET ScriptName='boss_gehennas' WHERE entry=12259;
UPDATE creature_template SET ScriptName='boss_garr' WHERE entry=12057;
UPDATE creature_template SET ScriptName='boss_baron_geddon' WHERE entry=12056;
UPDATE creature_template SET ScriptName='boss_shazzrah' WHERE entry=12264;
UPDATE creature_template SET ScriptName='boss_golemagg' WHERE entry=11988;
UPDATE creature_template SET ScriptName='boss_sulfuron' WHERE entry=12098;
UPDATE creature_template SET ScriptName='boss_majordomo' WHERE entry=12018;
UPDATE creature_template SET ScriptName='boss_ragnaros' WHERE entry=11502;
UPDATE creature_template SET ScriptName='mob_firesworn' WHERE entry=12099;
UPDATE creature_template SET ScriptName='mob_core_rager' WHERE entry=11672;
UPDATE creature_template SET ScriptName='mob_flamewaker_priest' WHERE entry=11662;
UPDATE gameobject_template SET ScriptName='go_molten_core_rune' WHERE entry IN (176951, 176952, 176953, 176954, 176955, 176956, 176957);

/* MOONGLADE */
UPDATE creature_template SET ScriptName='npc_keeper_remulos' WHERE entry=11832;
UPDATE creature_template SET ScriptName='boss_eranikus' WHERE entry=15491;
UPDATE gameobject_template SET ScriptName='go_omen_cluster' WHERE entry=180859;

/* MULGORE */

/* NAXXRAMAS */
UPDATE instance_template SET ScriptName='instance_naxxramas' WHERE map=533;
UPDATE creature_template SET ScriptName='boss_anubrekhan' WHERE entry=15956;
UPDATE creature_template SET ScriptName='boss_faerlina' WHERE entry=15953;
UPDATE creature_template SET ScriptName='boss_maexxna' WHERE entry=15952;
UPDATE creature_template SET ScriptName='npc_web_wrap' WHERE entry=16486;
UPDATE creature_template SET ScriptName='boss_noth' WHERE entry=15954;
UPDATE creature_template SET ScriptName='boss_heigan' WHERE entry=15936;
UPDATE creature_template SET ScriptName='boss_loatheb' WHERE entry=16011;
UPDATE creature_template SET ScriptName='boss_razuvious' WHERE entry=16061;
UPDATE creature_template SET ScriptName='boss_gothik' WHERE entry=16060;
UPDATE creature_template SET ScriptName='spell_anchor' WHERE entry=16137;
UPDATE creature_template SET ScriptName='boss_thane_korthazz' WHERE entry=16064;
UPDATE creature_template SET ScriptName='boss_sir_zeliek' WHERE entry=16063;
UPDATE creature_template SET ScriptName='boss_lady_blaumeux' WHERE entry=16065;
UPDATE creature_template SET ScriptName='boss_alexandros_mograine' WHERE entry=16062;
UPDATE creature_template SET ScriptName='boss_patchwerk' WHERE entry=16028;
UPDATE creature_template SET ScriptName='boss_grobbulus' WHERE entry=15931;
UPDATE creature_template SET ScriptName='boss_gluth' WHERE entry=15932;
UPDATE creature_template SET ScriptName='boss_thaddius' WHERE entry=15928;
UPDATE creature_template SET ScriptName='boss_stalagg' WHERE entry=15929;
UPDATE creature_template SET ScriptName='boss_feugen' WHERE entry=15930;
UPDATE creature_template SET ScriptName='npc_tesla_coil' WHERE entry=16218;
UPDATE creature_template SET ScriptName='boss_sapphiron' WHERE entry=15989;
UPDATE gameobject_template SET ScriptName='go_sapphiron_birth' WHERE entry=181356;
UPDATE gameobject_template SET ScriptName='go_anub_door' WHERE entry=181126;
UPDATE creature_template SET ScriptName='boss_kelthuzad' WHERE entry=15990;

/* ONYXIA'S LAIR */
UPDATE instance_template SET ScriptName='instance_onyxias_lair' WHERE map=249;
UPDATE creature_template SET ScriptName='boss_onyxia' WHERE entry=10184;

/* ORGRIMMAR */
UPDATE creature_template SET ScriptName='npc_shenthul' WHERE entry=3401;

/* RAGEFIRE CHASM */

/* RAZORFEN DOWNS */
UPDATE instance_template SET ScriptName='instance_razorfen_downs' WHERE map=129;
UPDATE creature_template SET ScriptName='npc_belnistrasz' WHERE entry=8516;
INSERT INTO scripted_event_id VALUES (3130, 'event_go_tutenkash_gong');

/* RAZORFEN KRAUL */
UPDATE instance_template SET ScriptName='instance_razorfen_kraul' WHERE map=47;
UPDATE creature_template SET ScriptName='npc_willix_the_importer' WHERE entry=4508;
UPDATE creature_template SET ScriptName='npc_snufflenose_gopher' WHERE entry=4781;

/* REDRIDGE MOUNTAINS */
UPDATE creature_template SET ScriptName='npc_corporal_keeshan' WHERE entry=349;

/* RUINS OF AHN'QIRAJ */
UPDATE instance_template SET ScriptName='instance_ruins_of_ahnqiraj' WHERE map=509;
UPDATE creature_template SET ScriptName='mob_anubisath_guardian' WHERE entry=15355;
UPDATE creature_template SET ScriptName='boss_kurinnaxx' WHERE entry=15348;
UPDATE creature_template SET ScriptName='boss_ayamiss' WHERE entry=15369;
UPDATE creature_template SET ScriptName='boss_moam' WHERE entry=15340;
UPDATE creature_template SET ScriptName='boss_ossirian' WHERE entry=15339;
UPDATE gameobject_template SET ScriptName='go_ossirian_crystal' WHERE entry=180619;
UPDATE creature_template SET ScriptName='npc_hive_zara_larva' WHERE entry=15555;
UPDATE creature_template SET ScriptName='boss_buru' WHERE entry=15370;
UPDATE creature_template SET ScriptName='npc_buru_egg' WHERE entry=15514;
UPDATE creature_template SET ScriptName='npc_general_andorov' WHERE entry=15471;
UPDATE creature_template SET ScriptName='npc_kaldorei_elite' WHERE entry=15473;

/* SCARLET MONASTERY */
UPDATE instance_template SET ScriptName='instance_scarlet_monastery' WHERE map=189;
UPDATE creature_template SET ScriptName='boss_arcanist_doan' WHERE entry=6487;
UPDATE creature_template SET ScriptName='boss_herod' WHERE entry=3975;
UPDATE creature_template SET ScriptName='boss_high_inquisitor_whitemane' WHERE entry=3977;
UPDATE creature_template SET ScriptName='boss_scarlet_commander_mograine' WHERE entry=3976;
UPDATE creature_template SET ScriptName='mob_scarlet_trainee' WHERE entry=6575;

/* SCHOLOMANCE */
UPDATE instance_template SET ScriptName='instance_scholomance' WHERE map=289;
UPDATE creature_template SET ScriptName='boss_darkmaster_gandling' WHERE entry=1853;
UPDATE creature_template SET ScriptName='boss_jandice_barov' WHERE entry=10503;
UPDATE creature_template SET ScriptName='npc_spectral_tutor' WHERE entry=10498;
INSERT INTO scripted_event_id VALUES
(5140,'event_dawn_gambit'),
(5618,'event_spell_gandling_shadow_portal'),
(5619,'event_spell_gandling_shadow_portal'),
(5620,'event_spell_gandling_shadow_portal'),
(5621,'event_spell_gandling_shadow_portal'),
(5622,'event_spell_gandling_shadow_portal'),
(5623,'event_spell_gandling_shadow_portal');

/* SEARING GORGE */
UPDATE creature_template SET ScriptName='npc_dorius_stonetender' WHERE entry=8284;

/* SHADOWFANG KEEP */
UPDATE instance_template SET ScriptName='instance_shadowfang_keep' WHERE map=33;
UPDATE creature_template SET ScriptName='npc_shadowfang_prisoner' WHERE entry IN (3849,3850);
UPDATE creature_template SET ScriptName='npc_arugal' WHERE entry=10000;
UPDATE creature_template SET ScriptName='npc_deathstalker_vincent' WHERE entry=4444;
UPDATE creature_template SET ScriptName='mob_arugal_voidwalker' WHERE entry=4627;
UPDATE creature_template SET ScriptName='boss_arugal' WHERE entry=4275;

/* SILITHUS */
UPDATE creature_template SET ScriptName='npc_anachronos_the_ancient' WHERE entry=15381;
UPDATE gameobject_template SET ScriptName='go_crystalline_tear' WHERE entry=180633;
UPDATE creature_template SET ScriptName='npc_solenor' WHERE entry IN (14530,14536);

/* SILVERPINE FOREST */
UPDATE creature_template SET ScriptName='npc_deathstalker_erland' WHERE entry=1978;
UPDATE creature_template SET ScriptName='npc_deathstalker_faerleia' WHERE entry=2058;

/* STOCKADES */

/* STONETALON MOUNTAINS */
UPDATE creature_template SET ScriptName='npc_kaya' WHERE entry=11856;

/* STORMWIND CITY */
UPDATE creature_template SET ScriptName='npc_bartleby' WHERE entry=6090;
UPDATE creature_template SET ScriptName='npc_dashel_stonefist' WHERE entry=4961;
UPDATE creature_template SET ScriptName='npc_lady_katrana_prestor' WHERE entry=1749;
UPDATE creature_template SET ScriptName='npc_squire_rowe' WHERE entry=17804;
UPDATE creature_template SET ScriptName='npc_reginald_windsor' WHERE entry =12580;

/* STRANGLETHORN VALE */
UPDATE creature_template SET ScriptName='mob_yenniku' WHERE entry=2530;
UPDATE gameobject_template SET ScriptName='go_transpolyporter_bb' WHERE entry IN(142172);

/* STRATHOLME */
UPDATE instance_template SET ScriptName='instance_stratholme' WHERE map=329;
UPDATE creature_template SET ScriptName='boss_dathrohan_balnazzar' WHERE entry=10812;
UPDATE creature_template SET ScriptName='boss_maleki_the_pallid' WHERE entry=10438;
UPDATE creature_template SET ScriptName='boss_cannon_master_willey' WHERE entry=10997;
UPDATE creature_template SET ScriptName='boss_baroness_anastari' WHERE entry=10436;
UPDATE creature_template SET ScriptName='npc_aurius' WHERE entry=10917;
UPDATE creature_template SET ScriptName='mobs_spectral_ghostly_citizen' WHERE entry IN (10384,10385);
UPDATE creature_template SET ScriptName='mob_restless_soul' WHERE entry=11122;
UPDATE gameobject_template SET ScriptName='go_gauntlet_gate' WHERE entry=175357;
UPDATE gameobject_template SET ScriptName='go_service_gate' WHERE entry=175368;
UPDATE gameobject_template SET ScriptName='go_stratholme_postbox' WHERE entry IN (176346,176349,176350,176351,176352,176353);

/* SUNKEN TEMPLE */
UPDATE instance_template SET ScriptName='instance_sunken_temple' WHERE map=109;
INSERT INTO scripted_areatrigger VALUES
(4016,'at_shade_of_eranikus');
UPDATE creature_template SET ScriptName='npc_malfurion_stormrage' WHERE entry=15362;
INSERT INTO scripted_event_id VALUES
(3094,'event_antalarion_statue_activation'),
(3095,'event_antalarion_statue_activation'),
(3097,'event_antalarion_statue_activation'),
(3098,'event_antalarion_statue_activation'),
(3099,'event_antalarion_statue_activation'),
(3100,'event_antalarion_statue_activation');
UPDATE creature_template SET ScriptName='npc_shade_of_hakkar' WHERE entry=8440;
UPDATE gameobject_template SET ScriptName='go_eternal_flame' WHERE entry IN (148418,148419,148420,148421);
INSERT INTO scripted_event_id VALUES
(8502,'event_avatar_of_hakkar');

/* SWAMP OF SORROWS */
UPDATE creature_template SET ScriptName='npc_galen_goodward' WHERE entry=5391;

/* TANARIS */
UPDATE creature_template SET ScriptName='mob_aquementas' WHERE entry=9453;
UPDATE creature_template SET ScriptName='npc_oox17tn' WHERE entry=7784;
UPDATE creature_template SET ScriptName='npc_stone_watcher_of_norgannon' WHERE entry=7918;
UPDATE creature_template SET ScriptName='npc_tooga' WHERE entry=5955;

/* TELDRASSIL */
UPDATE creature_template SET ScriptName='npc_mist' WHERE entry=3568;

/* TEMPLE OF AHN'QIRAJ */
UPDATE instance_template SET ScriptName='instance_temple_of_ahnqiraj' WHERE map=531;
UPDATE creature_template SET ScriptName='boss_cthun' WHERE entry=15727;
UPDATE creature_template SET ScriptName='boss_skeram' WHERE entry=15263;
UPDATE creature_template SET ScriptName='boss_vem' WHERE entry=15544;
UPDATE creature_template SET ScriptName='boss_yauj' WHERE entry=15543;
UPDATE creature_template SET ScriptName='boss_kri' WHERE entry=15511;
UPDATE creature_template SET ScriptName='boss_sartura' WHERE entry=15516;
UPDATE creature_template SET ScriptName='boss_fankriss' WHERE entry=15510;
UPDATE creature_template SET ScriptName='boss_viscidus' WHERE entry=15299;
UPDATE creature_template SET ScriptName='npc_glob_of_viscidus' WHERE entry=15667;
UPDATE creature_template SET ScriptName='boss_huhuran' WHERE entry=15509;
UPDATE creature_template SET ScriptName='boss_veklor' WHERE entry=15276;
UPDATE creature_template SET ScriptName='boss_veknilash' WHERE entry=15275;
UPDATE creature_template SET ScriptName='boss_ouro' WHERE entry=15517;
UPDATE creature_template SET ScriptName='npc_ouro_spawner' WHERE entry=15957;
UPDATE creature_template SET ScriptName='boss_eye_of_cthun' WHERE entry=15589;
UPDATE creature_template SET ScriptName='mob_sartura_royal_guard' WHERE entry=15984;
UPDATE creature_template SET ScriptName='mob_giant_claw_tentacle' WHERE entry=15728;
UPDATE creature_template SET ScriptName='mob_anubisath_sentinel' WHERE entry=15264;
INSERT INTO scripted_areatrigger VALUES
(4033,'at_stomach_cthun'),
(4034,'at_stomach_cthun');

/* THOUSAND NEEDLES */
UPDATE creature_template SET ScriptName='npc_kanati' WHERE entry=10638;
UPDATE creature_template SET ScriptName='npc_plucky_johnson' WHERE entry=6626;
UPDATE creature_template SET ScriptName='npc_paoka_swiftmountain' WHERE entry=10427;
UPDATE creature_template SET ScriptName='npc_lakota_windsong' WHERE entry=10646;

/* THUNDER BLUFF */

/* TIRISFAL GLADES */
UPDATE gameobject_template SET ScriptName='go_mausoleum_trigger' WHERE entry=104593;
UPDATE gameobject_template SET ScriptName='go_mausoleum_door' WHERE entry=176594;
UPDATE creature_template SET ScriptName='npc_calvin_montague' WHERE entry=6784;

/* ULDAMAN */
INSERT INTO scripted_event_id VALUES
(2228,'event_spell_altar_boss_aggro'),
(2268,'event_spell_altar_boss_aggro');
UPDATE creature_template SET ScriptName='boss_archaedas' WHERE entry=2748;
UPDATE creature_template SET ScriptName='mob_archaeras_add' WHERE entry IN (7309,7076,7077,10120);
UPDATE instance_template SET ScriptName='instance_uldaman' WHERE map=70;

/* UN'GORO CRATER */
UPDATE creature_template SET ScriptName='npc_ame01' WHERE entry=9623;
UPDATE creature_template SET ScriptName='npc_ringo' WHERE entry=9999;
UPDATE creature_template SET ScriptName='npc_simone_seductress' WHERE entry=14533;
UPDATE creature_template SET ScriptName='npc_simone_the_inconspicuous' WHERE entry=14527;
UPDATE creature_template SET ScriptName='npc_precious_the_devourer' WHERE entry=14538;

/* UNDERCITY */

/* WAILING CAVERNS */
UPDATE instance_template SET ScriptName='instance_wailing_caverns' WHERE map=43;
UPDATE creature_template SET ScriptName='npc_disciple_of_naralex' WHERE entry=3678;

/* WESTERN PLAGUELANDS */
UPDATE creature_template SET ScriptName='npc_the_scourge_cauldron' WHERE entry=11152;
UPDATE creature_template SET ScriptName='npc_taelan_fordring' WHERE entry=1842;
UPDATE creature_template SET ScriptName='npc_isillien' WHERE entry=1840;
UPDATE creature_template SET ScriptName='npc_tirion_fordring' WHERE entry=12126;

/* WESTFALL */
UPDATE creature_template SET ScriptName='npc_daphne_stilwell' WHERE entry=6182;
UPDATE creature_template SET ScriptName='npc_defias_traitor' WHERE entry=467;

/* WETLANDS */
UPDATE creature_template SET ScriptName='npc_tapoke_slim_jahn' WHERE entry=4962;
UPDATE creature_template SET ScriptName='npc_mikhail' WHERE entry=4963;

/* WINTERSPRING */
UPDATE creature_template SET ScriptName='npc_ranshalla' WHERE entry=10300;
UPDATE gameobject_template SET ScriptName='go_elune_fire' WHERE entry IN (177417, 177404);
UPDATE creature_template SET ScriptName='npc_artorius' WHERE entry IN (14531,14535);

/* ZUL'FARRAK */
UPDATE instance_template SET ScriptName='instance_zulfarrak' WHERE map=209;
INSERT INTO scripted_event_id VALUES
(2488,'event_go_zulfarrak_gong'),
(2609,'event_spell_unlocking');
INSERT INTO scripted_areatrigger VALUES
(1447,'at_zulfarrak');
UPDATE creature_template SET ScriptName='boss_zumrah' WHERE entry=7271;

/* ZUL'GURUB */
UPDATE instance_template SET ScriptName='instance_zulgurub' WHERE map=309;
UPDATE creature_template SET ScriptName='boss_jeklik' WHERE entry=14517;
UPDATE creature_template SET ScriptName='boss_venoxis' WHERE entry=14507;
UPDATE creature_template SET ScriptName='boss_marli' WHERE entry=14510;
UPDATE creature_template SET ScriptName='boss_mandokir' WHERE entry=11382;
UPDATE creature_template SET ScriptName='mob_ohgan' WHERE entry=14988;
UPDATE creature_template SET ScriptName='boss_jindo' WHERE entry=11380;
UPDATE creature_template SET ScriptName='boss_hakkar' WHERE entry=14834;
UPDATE creature_template SET ScriptName='boss_thekal' WHERE entry=14509;
UPDATE creature_template SET ScriptName='boss_arlokk' WHERE entry=14515;
UPDATE gameobject_template SET ScriptName='go_gong_of_bethekk' WHERE entry=180526;
UPDATE creature_template SET ScriptName='boss_hazzarah' WHERE entry=15083;
UPDATE creature_template SET ScriptName='boss_renataki' WHERE entry=15084;
UPDATE creature_template SET ScriptName='mob_zealot_lorkhan' WHERE entry=11347;
UPDATE creature_template SET ScriptName='mob_zealot_zath' WHERE entry=11348;
UPDATE creature_template SET ScriptName='mob_healing_ward' WHERE entry=14987;
UPDATE creature_template SET ScriptName='npc_gurubashi_bat_rider' WHERE entry=14750;
UPDATE creature_template SET ScriptName='npc_zulian_prowler' WHERE entry=15101;
UPDATE creature_template SET ScriptName='npc_soulflayer' WHERE entry=11359;

--
-- Below contains data for table `script_texts` mainly used in C++ parts.
-- valid entries for table are between -1000000 and -1999999
--

TRUNCATE script_texts;

--
-- -1 000 000 First 100 entries are reserved for special use, do not add regular text here.
--

INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1000000,'<ScriptDev2 Text Entry Missing!>',0,0,0,0,'DEFAULT_TEXT'),
(-1000001,'%s goes into a killing frenzy!',0,2,0,0,'EMOTE_GENERIC_FRENZY_KILL'),
(-1000002,'%s goes into a frenzy!',0,2,0,0,'EMOTE_GENERIC_FRENZY'),
(-1000003,'%s becomes enraged!',0,2,0,0,'EMOTE_GENERIC_ENRAGED'),
(-1000004,'%s goes into a berserker rage!',0,2,0,0,'EMOTE_GENERIC_BERSERK'),
(-1000005,'%s goes into a frenzy!',0,3,0,0,'EMOTE_BOSS_GENERIC_FRENZY'),
(-1000006,'%s becomes enraged!',0,3,0,0,'EMOTE_BOSS_GENERIC_ENRAGED');

--
-- Normal text entries below. Say/Yell/Whisper/Emote for any regular world object.
-- Text entries for text used by creatures in instances are using map id as part of entry.
-- Example: for map 430, the database text entry for all creatures normally on this map start with -1430
-- Values decrement as they are made.
-- For creatures outside instance, use -1 000 100 and below. Decrement value as they are made.
--
-- Comment should contain a unique name that can be easily identified later by using sql queries like for example
-- SELECT * FROM script_texts WHERE comment LIKE '%defias%';
-- Place the define used in script itself at the end of the comment.
--
-- Do not change entry without a very good reason. Localization projects depends on using entries that does not change!
-- Example: UPDATE script_texts SET content_loc1 = 'the translated text' WHERE entry = -1000100;
--

-- -1 000 100 GENERAL MAPS (not instance maps)
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1000100,'Come, little ones. Face me!',0,1,0,0,'azuregos SAY_TELEPORT'),

(-1000101,'Follow me, $N. I\'ll take you to the Defias hideout. But you better protect me or I am as good as dead',0,0,7,0,'defias traitor SAY_START'),
(-1000102,'The entrance is hidden here in Moonbrook. Keep your eyes peeled for thieves. They want me dead.',0,0,7,0,'defias traitor SAY_PROGRESS'),
(-1000103,'You can go tell Stoutmantle this is where the Defias Gang is holed up, $N.',0,0,7,0,'defias traitor SAY_END'),
(-1000104,'$N coming in fast! Prepare to fight!',0,0,7,0,'defias traitor SAY_AGGRO_1'),
(-1000105,'Help!',0,0,7,0,'defias traitor SAY_AGGRO_2'),

(-1000106,'Everyone ready?',0,0,1,0,'torek SAY_READY'),
(-1000107,'Ok, Lets move out!',0,0,1,0,'torek SAY_MOVE'),
(-1000108,'Prepare yourselves. Silverwing is just around the bend.',0,0,1,0,'torek SAY_PREPARE'),
(-1000109,'Silverwing is ours!',0,0,1,0,'torek SAY_WIN'),
(-1000110,'Go report that the outpost is taken. We will remain here.',0,0,1,0,'torek SAY_END'),

(-1000111,'Our house is this way, through the thicket.',0,0,7,0,'magwin SAY_START'),
(-1000112,'Help me!',0,0,7,0,'magwin SAY_AGGRO'),
(-1000113,'My poor family. Everything has been destroyed.',0,0,7,0,'magwin SAY_PROGRESS'),
(-1000114,'Father! Father! You\'re alive!',0,0,7,0,'magwin SAY_END1'),
(-1000115,'You can thank $N for getting me back here safely, father.',0,0,7,0,'magwin SAY_END2'),
(-1000116,'%s hugs her father.',0,2,7,0,'magwin EMOTE_HUG'),

(-1000117,'Thank you for agreeing to help. Now, let\'s get out of here $N.',0,0,1,0,'wounded elf SAY_ELF_START'),
(-1000118,'Over there! They\'re following us!',0,0,1,0,'wounded elf SAY_ELF_SUMMON1'),
(-1000119,'Allow me a moment to rest. The journey taxes what little strength I have.',0,0,1,16,'wounded elf SAY_ELF_RESTING'),
(-1000120,'Did you hear something?',0,0,1,0,'wounded elf SAY_ELF_SUMMON2'),
(-1000121,'Falcon Watch, at last! Now, where\'s my... Oh no! My pack, it\'s missing! Where has -',0,0,1,0,'wounded elf SAY_ELF_COMPLETE'),
(-1000122,'You won\'t keep me from getting to Falcon Watch!',0,0,1,0,'wounded elf SAY_ELF_AGGRO'),

(-1000123,'Ready when you are, $c.',0,0,0,15,'big will SAY_BIG_WILL_READY'),
(-1000124,'The Affray has begun. $n, get ready to fight!',0,0,0,0,'twiggy SAY_TWIGGY_BEGIN'),
(-1000125,'You! Enter the fray!',0,0,0,0,'twiggy SAY_TWIGGY_FRAY'),
(-1000126,'Challenger is down!',0,0,0,0,'twiggy SAY_TWIGGY_DOWN'),
(-1000127,'The Affray is over.',0,0,0,0,'twiggy SAY_TWIGGY_OVER'),

(-1000128,'We need you to send reinforcements to Manaforge Duro, Ardonis. This is not a request, it\'s an order.',0,0,0,0,'dawnforge SAY_COMMANDER_DAWNFORGE_1'),
(-1000129,'You cannot be serious! We are severely understaffed and can barely keep this manaforge functional!',0,0,0,0,'dawnforge SAY_ARCANIST_ARDONIS_1'),
(-1000130,'You will do as ordered. Manaforge Duro has come under heavy attack by mana creatures and the situation is out of control. Failure to comply will not be tolerated!',0,0,0,0,'dawnforge SAY_COMMANDER_DAWNFORGE_2'),
(-1000131,'Indeed, it is not a request.',0,0,0,0,'dawnforge SAY_PATHALEON_CULATOR_IMAGE_1'),
(-1000132,'My lord!',0,0,0,0,'dawnforge SAY_COMMANDER_DAWNFORGE_3'),
(-1000133,'Duro will be reinforced! Ultris was a complete disaster. I will NOT have that mistake repeated!',0,0,0,0,'dawnforge PATHALEON_CULATOR_IMAGE_2'),
(-1000134,'We\'ve had too many setbacks along the way: Hellfire Citadel, Fallen Sky Ridge, Firewing Point... Prince Kael\'thas will tolerate no further delays. I will tolerate nothing other than complete success!',0,0,0,0,'dawnforge PATHALEON_CULATOR_IMAGE_2_1'),
(-1000135,'I am returning to Tempest Keep. See to it that I do not have reason to return!',0,0,0,0,'dawnforge PATHALEON_CULATOR_IMAGE_2_2' ),
(-1000136,'Yes, my lord.',0,0,0,0,'dawnforge COMMANDER_DAWNFORGE_4 SAY_ARCANIST_ARDONIS_2'),
(-1000137,'See to it, Ardonis!',0,0,0,0,'dawnforge COMMANDER_DAWNFORGE_5'),

(-1000138,'Avruu\'s magic... it still controls me. You must fight me, mortal. It\'s the only way to break the spell!',0,0,0,0,'aeranas SAY_SUMMON'),
(-1000139,'Avruu\'s magic is broken! I\'m free once again!',0,0,0,0,'aeranas SAY_FREE'),

(-1000140,'Let\'s go.',0,0,1,0,'lilatha SAY_START'),
(-1000141,'$N, let\'s use the antechamber to the right.',0,0,1,0,'lilatha SAY_PROGRESS1'),
(-1000142,'I can see the light at the end of the tunnel!',0,0,1,0,'lilatha SAY_PROGRESS2'),
(-1000143,'There\'s Farstrider Enclave now, $C. Not far to go... Look out! Troll ambush!!',0,0,1,0,'lilatha SAY_PROGRESS3'),
(-1000144,'Thank you for saving my life and bringing me back to safety, $N',0,0,1,0,'lilatha SAY_END1'),
(-1000145,'Captain Helios, I\'ve been rescued from the Amani Catacombs. Reporting for duty, sir!',0,0,1,0,'lilatha SAY_END2'),
(-1000146,'Liatha, get someone to look at those injuries. Thank you for bringing her back safely.',0,0,1,0,'lilatha CAPTAIN_ANSWER'),

-- Worldboss: Doomloard Kazzak
(-1000147,'I remember well the sting of defeat at the conclusion of the Third War. I have waited far too long for my revenge. Now the shadow of the Legion falls over this world. It is only a matter of time until all of your failed creation... is undone.',0,1,0,0,'kazzak SAY_INTRO'),
(-1000148,'The Legion will conquer all!',0,1,0,0,'kazzak SAY_AGGRO1'),
(-1000149,'All mortals will perish!',0,1,0,0,'kazzak SAY_AGGRO2'),
(-1000150,'All life must be eradicated!',0,1,0,0,'kazzak SAY_SURPREME1'),
(-1000151,'I\'ll rip the flesh from your bones!',0,1,0,0,'kazzak SAY_SURPREME2'),
(-1000152,'Kirel Narak!',0,1,0,0,'kazzak SAY_KILL1'),
(-1000153,'Contemptible wretch!',0,1,0,0,'kazzak SAY_KILL2'),
(-1000154,'The universe will be remade.',0,1,0,0,'kazzak SAY_KILL3'),
(-1000155,'The Legion... will never... fall.',0,1,0,0,'kazzak SAY_DEATH'),
(-1000156,'REUSE ME',0,0,0,0,'REUSE ME'),
(-1000157,'Invaders, you dangle upon the precipice of oblivion! The Burning Legion comes and with it comes your end.',0,1,0,0,'kazzak SAY_RAND1'),
(-1000158,'Impudent whelps, you only delay the inevitable. Where one has fallen, ten shall rise. Such is the will of Kazzak...',0,1,0,0,'kazzak SAY_RAND2'),

-- Worldboss: Doomwalker
-- (-1000159,'Do not proceed. You will be eliminated!',11344,1,0,0,'doomwalker SAY_AGGRO'),
-- (-1000160,'Tectonic disruption commencing.',11345,1,0,0,'doomwalker SAY_EARTHQUAKE_1'),
-- (-1000161,'Magnitude set. Release.',11346,1,0,0,'doomwalker SAY_EARTHQUAKE_2'),
-- (-1000162,'Trajectory locked.',11347,1,0,0,'doomwalker SAY_OVERRUN_1'),
-- (-1000163,'Engage maximum speed.',11348,1,0,0,'doomwalker SAY_OVERRUN_2'),
-- (-1000164,'Threat level zero.',11349,1,0,0,'doomwalker SAY_SLAY_1'),
-- (-1000165,'Directive accomplished.',11350,1,0,0,'doomwalker SAY_SLAY_2'),
-- (-1000166,'Target exterminated.',11351,1,0,0,'doomwalker SAY_SLAY_3'),
-- (-1000167,'System failure in five, f-o-u-r...',11352,1,0,0,'doomwalker SAY_DEATH'),

(-1000168,'Who dares awaken Aquementas?',0,1,0,0,'aquementas AGGRO_YELL_AQUE'),

(-1000169,'Muahahahaha! You fool! You\'ve released me from my banishment in the interstices between space and time!',0,1,0,0,'nether_drake SAY_NIHIL_1'),
(-1000170,'All of Draenor shall quick beneath my feet! I will destroy this world and reshape it in my image!',0,1,0,0,'nether_drake SAY_NIHIL_2'),
(-1000171,'Where shall I begin? I cannot bother myself with a worm such as yourself. There is a world to be conquered!',0,1,0,0,'nether_drake SAY_NIHIL_3'),
(-1000172,'No doubt the fools that banished me are long dead. I shall take wing survey my demesne. Pray to whatever gods you hold dear that we do not meet again.',0,1,0,0,'nether_drake SAY_NIHIL_4'),
(-1000173,'NOOOOooooooo!',0,1,0,0,'nether_drake SAY_NIHIL_INTERRUPT'),

(-1000174,'Good $N, you are under the spell\'s influence. I must analyze it quickly, then we can talk.',0,0,7,0,'daranelle SAY_SPELL_INFLUENCE'),

(-1000175,'Thank you, mortal.',0,0,11,0,' SAY_JUST_EATEN'),

(-1000176,'The last thing I remember is the ship falling and us getting into the pods. I\'ll go see how I can help. Thank you!',0,0,7,0,'draenei_survivor SAY_HEAL1'),
(-1000177,'$C, Where am I? Who are you? Oh no! What happened to the ship?',0,0,7,0,'draenei_survivor SAY_HEAL2'),
(-1000178,'$C You saved me! I owe you a debt that I can never repay. I\'ll go see if I can help the others.',0,0,7,0,'draenei_survivor SAY_HEAL3'),
(-1000179,'Ugh... what is this place? Is that all that\'s left of the ship over there?',0,0,7,0,'draenei_survivor SAY_HEAL4'),
(-1000180,'Oh, the pain...',0,0,7,0,'draenei_survivor SAY_HELP1'),
(-1000181,'Everything hurts, Please make it stop...',0,0,7,0,'draenei_survivor SAY_HELP2'),
(-1000182,'Ughhh... I hurt. Can you help me?',0,0,7,0,'draenei_survivor SAY_HELP3'),
(-1000183,'I don\'t know if I can make it, please help me...',0,0,7,0,'draenei_survivor SAY_HELP4'),

(-1000184,'Yes Master, all goes along as planned.',0,0,7,0,'engineer_spark SAY_TEXT'),
(-1000185,'%s puts the shell to his ear.',0,2,7,0,'engineer_spark EMOTE_SHELL'),
(-1000186,'Now I cut you!',0,1,7,0,'engineer_spark SAY_ATTACK'),

(-1000187,'Thank you, dear $C, you just saved my life.',0,0,0,0,'npc_redemption_target SAY_HEAL'),

(-1000188,'Deployment sucessful. Trespassers will be neutralized.',0,0,0,0,'converted_sentry SAY_CONVERTED_1'),
(-1000189,'Objective acquired. Initiating security routines.',0,0,0,0,'converted_sentry SAY_CONVERTED_2'),

(-1000190,'In Nagrand, food hunt ogre!',0,0,0,0,' SAY_LUMP_0'),
(-1000191,'You taste good with maybe a little salt and pepper.',0,0,0,0,' SAY_LUMP_1'),
(-1000192,'OK, OK! Lump give up!',0,0,0,0,' SAY_LUMP_DEFEAT'),

-- Lady Silvanas Deadrunner (Silvermoon?)
-- (-1000193,'%s looks down at the discarded necklace. In her sadness, the lady incants a glamour, which beckons forth Highborne spirits. The chamber resonates with their ancient song about the Sin\'dorei...',10896,2,1,0,'lady_sylvanas EMOTE_LAMENT_START'),

(-1000194,'I give up! Please don\'t kill me!',0,0,0,0,'unkor SAY_SUBMIT'),

(-1000195,'Thank you again, $N. I\'ll make my way to the road now. When you can, find Terenthis and let him know we escaped.',0,0,0,1,'volcor SAY_ESCAPE'),

(-1000196,'Belore...',0,0,1,0,'lady_sylvanas SAY_LAMENT_END'),
(-1000197,'%s kneels down and pick up the amulet.',0,2,1,16,'lady_sylvanas EMOTE_LAMENT_END'),

(-1000198,'Taste blade, mongrel!',0,0,0,0,'SAY_GUARD_SIL_AGGRO1'),
(-1000199,'Please tell me that you didn\'t just do what I think you just did. Please tell me that I\'m not going to have to hurt you...',0,0,0,0,'SAY_GUARD_SIL_AGGRO2'),
(-1000200,'As if we don\'t have enough problems, you go and create more!',0,0,0,0,'SAY_GUARD_SIL_AGGRO3'),

(-1000201,'I\'m saved! Thank you, doctor!',0,0,0,0,'injured_patient SAY_DOC1'),
(-1000202,'HOORAY! I AM SAVED!',0,0,0,0,'injured_patient SAY_DOC2'),
(-1000203,'Sweet, sweet embrace... take me...',0,0,0,0,'injured_patient SAY_DOC3'),

(-1000204,'%s looks up at you quizzically. Maybe you should inspect it?',0,2,0,0,'cluck EMOTE_CLUCK_TEXT1'),
(-1000205,'%s looks at you unexpectedly.',0,2,0,0,'cluck EMOTE_CLUCK_TEXT2'),

(-1000206,'REUSE ME',0,0,0,0,'REUSE ME'),
(-1000207,'REUSE ME',0,0,0,0,'REUSE ME'),

(-1000208,'Frenzyheart kill you if you come back. You no welcome here no more!',0,0,0,0,'vekjik SAY_TEXTID_VEKJIK1'),

(-1000209,'Very well. Let\'s see what you have to show me, $N.',0,0,1,0,'anvilward SAY_ANVIL1'),
(-1000210,'What manner of trick is this, $R? If you seek to ambush me, I warn you I will not go down quietly!',0,0,1,0,'anvilward SAY_ANVIL2'),

(-1000211,'Warning! %s emergency shutdown process initiated by $N. Shutdown will complete in two minutes.',0,2,0,0,'manaforge_control EMOTE_START'),
(-1000212,'Emergency shutdown will complete in one minute.',0,2,0,0,'manaforge_control EMOTE_60'),
(-1000213,'Emergency shutdown will complete in thirty seconds.',0,2,0,0,'manaforge_control EMOTE_30'),
(-1000214,'Emergency shutdown will complete in ten seconds.',0,2,0,0,'manaforge_control EMOTE_10'),
(-1000215,'Emergency shutdown complete.',0,2,0,0,'manaforge_control EMOTE_COMPLETE'),
(-1000216,'Emergency shutdown aborted.',0,2,0,0,'manaforge_control EMOTE_ABORT'),

(-1000217,'Greetings, $N. I will guide you through the cavern. Please try and keep up.',0,4,0,0,'WHISPER_CUSTODIAN_1'),
(-1000218,'We do not know if the Caverns of Time have always been accessible to mortals. Truly, it is impossible to tell as the Timeless One is in perpetual motion, changing our timeways as he sees fit. What you see now may very well not exist tomorrow. You may wake up and have no memory of this place.',0,4,0,0,'WHISPER_CUSTODIAN_2'),
(-1000219,'It is strange, I know... Most mortals cannot actually comprehend what they see here, as often, what they see is not anchored within their own perception of reality.',0,4,0,0,'WHISPER_CUSTODIAN_3'),
(-1000220,'Follow me, please.',0,4,0,0,'WHISPER_CUSTODIAN_4'),
(-1000221,'There are only two truths to be found here: First, that time is chaotic, always in flux, and completely malleable and second, perception does not dictate reality.',0,4,0,0,'WHISPER_CUSTODIAN_5'),
(-1000222,'As custodians of time, we watch over and care for Nozdormu\'s realm. The master is away at the moment, which means that attempts are being made to dramatically alter time. The master never meddles in the affairs of mortals but instead corrects the alterations made to time by others. He is reactionary in this regard.',0,4,0,0,'WHISPER_CUSTODIAN_6'),
(-1000223,'For normal maintenance of time, the Keepers of Time are sufficient caretakers. We are able to deal with most ordinary disturbances. I speak of little things, such as rogue mages changing something in the past to elevate their status or wealth in the present.',0,4,0,0,'WHISPER_CUSTODIAN_7'),
(-1000224,'These tunnels that you see are called timeways. They are infinite in number. The ones that currently exist in your reality are what the master has deemed as \'trouble spots.\' These trouble spots may differ completely in theme but they always share a cause. That is, their existence is a result of the same temporal disturbance. Remember that should you venture inside one...',0,4,0,0,'WHISPER_CUSTODIAN_8'),
(-1000225,'This timeway is in great disarray! We have agents inside right now attempting to restore order. What information I have indicates that Thrall\'s freedom is in jeopardy. A malevolent organization known as the Infinite Dragonflight is trying to prevent his escape. I fear without outside assistance, all will be lost.',0,4,0,0,'WHISPER_CUSTODIAN_9'),
(-1000226,'We have very little information on this timeway. Sa\'at has been dispatched and is currently inside. The data we have gathered from his correspondence is that the Infinite Dragonflight are once again attempting to alter time. Could it be that the opening of the Dark Portal is being targeted for sabotage? Let us hope not...',0,4,0,0,'WHISPER_CUSTODIAN_10'),
(-1000227,'This timeway is currently collapsing. What that may hold for the past, present and future is currently unknown...',0,4,0,0,'WHISPER_CUSTODIAN_11'),
(-1000228,'The timeways are currently ranked in order from least catastrophic to most catastrophic. Note that they are all classified as catastrophic, meaning that any single one of these timeways collapsing would mean that your world would end. We only classify them in such a way so that the heroes and adventurers that are sent here know which timeway best suits their abilities.',0,4,0,0,'WHISPER_CUSTODIAN_12'),
(-1000229,'All we know of this timeway is that it leads to Mount Hyjal. The Infinite Dragonflight have gone to great lengths to prevent our involvement. We know next to nothing, mortal. Soridormi is currently attempting to break through the timeway\'s defenses but has thus far been unsuccessful. You might be our only hope of breaking through and resolving the conflict.',0,4,0,0,'WHISPER_CUSTODIAN_13'),
(-1000230,'Our time is at an end $N. I would wish you luck, if such a thing existed.',0,4,0,0,'WHISPER_CUSTODIAN_14'),

(-1000231,'Ah, $GPriest:Priestess; you came along just in time. I appreciate it.',0,0,0,20,'garments SAY_COMMON_HEALED'),
(-1000232,'Thank you! Thank you, $GPriest:Priestess;. Now I can take on those gnolls with your power to back me!',0,0,1,4,'garments SAY_DG_KEL_THANKS'),
(-1000233,'Farewell to you, and may shadow always protect you!',0,0,1,3,'garments SAY_DG_KEL_GOODBYE'),

(-1000234,'Follow me, stranger. This won\'t take long.',0,0,0,0,'SAY_KHAD_SERV_0'),
(-1000235,'Shattrath was once the draenei capital of this world. Its name means "dwelling of light."',0,4,0,0,'SAY_KHAD_SERV_1'),
(-1000236,'When the Burning Legion turned the orcs against the draenei, the fiercest battle was fought here. The draenei fought tooth and nail, but in the end the city fell.',0,4,0,0,'SAY_KHAD_SERV_2'),
(-1000237,'The city was left in ruins and darkness... until the Sha\'tar arrived.',0,4,0,0,'SAY_KHAD_SERV_3'),
(-1000238,'Let us go into the Lower City. I will warn you that as one of the only safe havens in Outland, Shattrath has attracted droves of refugees from all wars, current and past.',0,4,0,0,'SAY_KHAD_SERV_4'),
(-1000239,'The Sha\'tar, or "born from light" are the naaru that came to Outland to fight the demons of the Burning Legion.',0,4,0,0,'SAY_KHAD_SERV_5'),
(-1000240,'They were drawn to the ruins of Shattrath City where a small remnant of the draenei priesthood conducted its rites inside a ruined temple on this very spot.',0,4,0,0,'SAY_KHAD_SERV_6'),
(-1000241,'The priesthood, known as the Aldor, quickly regained its strength as word spread that the naaru had returned and reconstruction soon began. The ruined temple is now used as an infirmary for injured refugees.',0,4,0,0,'SAY_KHAD_SERV_7'),
(-1000242,'It wouldn\'t be long, however, before the city came under attack once again. This time, the attack came from Illidan\'s armies. A large regiment of blood elves had been sent by Illidan\'s ally, Kael\'thas Sunstrider, to lay waste to the city.',0,4,0,0,'SAY_KHAD_SERV_8'),
(-1000243,'As the regiment of blood elves crossed this very bridge, the Aldor\'s exarchs and vindicators lined up to defend the Terrace of Light. But then the unexpected happened.',0,4,0,0,'SAY_KHAD_SERV_9'),
(-1000244,'The blood elves laid down their weapons in front of the city\'s defenders; their leader, a blood elf elder known as Voren\'thal, stormed into the Terrace of Light and demanded to speak to A\'dal.',0,4,0,0,'SAY_KHAD_SERV_10'),
(-1000245,'As the naaru approached him, Voren\'thal kneeled before him and uttered the following words: "I\'ve seen you in a vision, naaru. My race\'s only hope for survival lies with you. My followers and I are here to serve you."',0,4,0,0,'SAY_KHAD_SERV_11'),
(-1000246,'The defection of Voren\'thal and his followers was the largest loss ever incurred by Kael\'s forces. And these weren\'t just any blood elves. Many of the best and brightest amongst Kael\'s scholars and magisters had been swayed by Voren\'thal\'s influence.',0,4,0,0,'SAY_KHAD_SERV_12'),
(-1000247,'The naaru accepted the defectors, who would become known as the Scryers; their dwelling lies in the platform above. Only those initiated with the Scryers are allowed there.',0,4,0,0,'SAY_KHAD_SERV_13'),
(-1000248,'The Aldor are followers of the Light and forgiveness and redemption are values they understand. However, they found hard to forget the deeds of the blood elves while under Kael\'s command.',0,4,0,0,'SAY_KHAD_SERV_14'),
(-1000249,'Many of the priesthood had been slain by the same magisters who now vowed to serve the naaru. They were not happy to share the city with their former enemies.',0,4,0,0,'SAY_KHAD_SERV_15'),
(-1000250,'The Aldor\'s most holy temple and its surrounding dwellings lie on the terrace above. As a holy site, only the initiated are welcome inside.',0,4,0,0,'SAY_KHAD_SERV_16'),
(-1000251,'The attacks against Shattrath continued, but the city did not fall\, as you can see. On the contrary, the naaru known as Xi\'ri led a successful incursion into Shadowmoon Valley - Illidan\'s doorstep.',0,4,0,0,'SAY_KHAD_SERV_17'),
(-1000252,'There he continues to wage war on Illidan with the assistance of the Aldor and the Scryers. The two factions have not given up on their old feuds, though.',0,4,0,0,'SAY_KHAD_SERV_18'),
(-1000253,'Such is their animosity that they vie for the honor of being sent to assist the naaru there. Each day, that decision is made here by A\'dal. The armies gather here to receive A\'dal\'s blessing before heading to Shadowmoon.',0,4,0,0,'SAY_KHAD_SERV_19'),
(-1000254,'Khadgar should be ready to see you again. Just remember that to serve the Sha\'tar you will most likely have to ally with the Aldor or the Scryers. And seeking the favor of one group will cause the others\' dislike.',0,4,0,0,'SAY_KHAD_SERV_20'),
(-1000255,'Good luck stranger, and welcome to Shattrath City.',0,4,0,0,'SAY_KHAD_SERV_21'),

(-1000256,'Thank you! Thank you, $GPriest:Priestess;. Now I can take on those murlocs with the Light on my side!',0,0,7,4,'garments SAY_ROBERTS_THANKS'),
(-1000257,'Farewell to you, and may the Light be with you always.',0,0,7,3,'garments SAY_ROBERTS_GOODBYE'),
(-1000258,'Thank you! Thank you, $GPriest:Priestess;. Now I can take on those humans with your power to back me!',0,0,1,4,'garments SAY_KORJA_THANKS'),
(-1000259,'Farewell to you, and may our ancestors be with you always!',0,0,1,3,'garments SAY_KORJA_GOODBYE'),
(-1000260,'Thank you! Thank you, $GPriest:Priestess;. Now I can take on those wendigo with the Light on my side!',0,0,7,4,'garments SAY_DOLF_THANKS'),
(-1000261,'Farewell to you, and may the Light be with you always.',0,0,7,3,'garments SAY_DOLF_GOODBYE'),
(-1000262,'Thank you! Thank you, $GPriest:Priestess;. Now I can take on those corrupt timberlings with Elune\'s power behind me!',0,0,2,4,'garments SAY_SHAYA_THANKS'),
(-1000263,'Farewell to you, and may Elune be with you always.',0,0,2,3,'garments SAY_SHAYA_GOODBYE'),

(-1000264,'Ok, $N. Follow me to the cave where I\'ll attempt to harness the power of the rune stone into these goggles.',0,0,0,1,'phizzlethorpe SAY_PROGRESS_1'),
(-1000265,'I discovered this cave on our first day here. I believe the energy in the stone can be used to our advantage.',0,0,0,1,'phizzlethorpe SAY_PROGRESS_2'),
(-1000266,'I\'ll begin drawing energy from the stone. Your job, $N, is to defend me. This place is cursed... trust me.',0,0,0,1,'phizzlethorpe SAY_PROGRESS_3'),
(-1000267,'%s begins tinkering with the goggles before the stone.',0,2,0,0,'phizzlethorpe EMOTE_PROGRESS_4'),
(-1000268,'Help!!! Get these things off me so I can get my work done!',0,0,0,0,'phizzlethorpe SAY_AGGRO'),
(-1000269,'Almost done! Just a little longer!',0,0,0,1,'phizzlethorpe SAY_PROGRESS_5'),
(-1000270,'I\'ve done it! I have harnessed the power of the stone into the goggles! Let\'s get out of here!',0,0,0,1,'phizzlethorpe SAY_PROGRESS_6'),
(-1000271,'Phew! Glad to be back from that creepy cave.',0,0,0,1,'phizzlethorpe SAY_PROGRESS_7'),
(-1000272,'%s hands one glowing goggles over to Doctor Draxlegauge.',0,2,0,0,'phizzlethorpe EMOTE_PROGRESS_8'),
(-1000273,'Doctor Draxlegauge will give you further instructions, $N. Many thanks for your help!',0,0,0,1,'phizzlethorpe SAY_PROGRESS_9'),

(-1000274,'Time to teach you a lesson in manners, little $Gboy:girl;!',0,0,0,0,'larry SAY_START'),
(-1000275,'Now I\'m gonna give you to the count of \'3\' to get out of here before I sick the dogs on you.',0,0,0,0,'larry SAY_COUNT'),
(-1000276,'1...',0,0,0,0,'larry SAY_COUNT_1'),
(-1000277,'2...',0,0,0,0,'larry SAY_COUNT_2'),
(-1000278,'Time to meet your maker!',0,0,0,0,'larry SAY_ATTACK_5'),
(-1000279,'Alright, we give up! Don\'t hurt us!',0,0,0,0,'larry SAY_GIVEUP'),

(-1000280,'A shadowy, sinister presence has invaded the Emerald Dream. Its power is poised to spill over into our world, $N. We must oppose it! That\'s why I cannot accompany you in person.',0,0,0,1,'clintar SAY_START'),
(-1000281,'The Emerald Dream will never be yours!',0,0,0,0,'clintar SAY_AGGRO_1'),
(-1000282,'Begone from this place!',0,0,0,0,'clintar SAY_AGGRO_2'),
(-1000283,'That\'s the first relic, but there are still two more. Follow me, $N.',0,0,0,0,'clintar SAY_RELIC1'),
(-1000284,'I\'ve recovered the second relic. Take a moment to rest, and then we\'ll continue to the last reliquary.',0,0,0,0,'clintar SAY_RELIC2'),
(-1000285,'We have all three of the relics, but my energy is rapidly fading. We must make our way back to Dreamwarden Lurosa! He will let you know what to do next.',0,0,0,0,'clintar SAY_RELIC3'),
(-1000286,'Lurosa, I am entrusting the Relics of Aviana to $N, who will take them to Morthis Whisperwing. I must return completely to the Emerald Dream now. Do not let $N fail!',0,0,0,1,'clintar SAY_END'),

(-1000287,'Emergency power activated! Initializing ambulatory motor! CLUCK!',0,0,0,0,'oox SAY_OOX_START'),
(-1000288,'Physical threat detected! Evasive action! CLUCK!',0,0,0,0,'oox SAY_OOX_AGGRO1'),
(-1000289,'Threat analyzed! Activating combat plan beta! CLUCK!',0,0,0,0,'oox SAY_OOX_AGGRO2'),
(-1000290,'CLUCK! Sensors detect spatial anomaly - danger imminent! CLUCK!',0,0,0,0,'oox SAY_OOX_AMBUSH'),
(-1000291,'No one challenges the Wastewander nomads - not even robotic chickens! ATTACK!',0,0,0,0,'oox SAY_OOX17_AMBUSH_REPLY'),
(-1000292,'Cloaking systems online! CLUCK! Engaging cloak for transport to Booty Bay!',0,0,0,0,'oox SAY_OOX_END'),

(-1000293,'To the house! Stay close to me, no matter what! I have my gun and ammo there!',0,0,7,0,'stilwell SAY_DS_START'),
(-1000294,'We showed that one!',0,0,7,0,'stilwell SAY_DS_DOWN_1'),
(-1000295,'One more down!',0,0,7,0,'stilwell SAY_DS_DOWN_2'),
(-1000296,'We\'ve done it! We won!',0,0,7,0,'stilwell SAY_DS_DOWN_3'),
(-1000297,'Meet me down by the orchard--I just need to put my gun away.',0,0,7,0,'stilwell SAY_DS_PROLOGUE'),

(-1000298,'Alright, alright I think I can figure out how to operate this thing...',0,0,0,0,'wizzlecrank SAY_START'),
(-1000299,'Arrrgh! This isn\'t right!',0,0,0,0,'wizzlecrank SAY_STARTUP1'),
(-1000300,'Okay, I think I\'ve got it, now. Follow me, $n!',0,0,0,1,'wizzlecrank SAY_STARTUP2'),
(-1000301,'There\'s the stolen shredder! Stop it or Lugwizzle will have our hides!',0,1,0,0,'wizzlecrank SAY_MERCENARY'),
(-1000302,'Looks like we\'re out of woods, eh? Wonder what this does...',0,0,0,0,'wizzlecrank SAY_PROGRESS_1'),
(-1000303,'Come on, don\'t break down on me now!',0,0,0,0,'wizzlecrank SAY_PROGRESS_2'),
(-1000304,'That was a close one! Well, let\'s get going, it\'s still a ways to Ratchet!',0,0,0,0,'wizzlecrank SAY_PROGRESS_3'),
(-1000305,'Hmm... I don\'t think this blinking red light is a good thing...',0,0,0,0,'wizzlecrank SAY_END'),

(-1000306,'Let\'s get to the others, and keep an eye open for those wolves cutside...',0,0,1,0,'erland SAY_START_1'),
(-1000307,'Be careful, $N. Those wolves like to hide among the trees.',0,0,1,0,'erland SAY_START_2'),
(-1000308,'A $C attacks!',0,0,1,0,'erland SAY_AGGRO_1'),
(-1000309,'Beware! I am under attack!',0,0,1,0,'erland SAY_AGGRO_2'),
(-1000310,'Oh no! A $C is upon us!',0,0,1,0,'erland SAY_AGGRO_3'),
(-1000311,'We\'re almost there!',0,0,1,0,'erland SAY_PROGRESS'),
(-1000312,'We made it! Thanks, $N. I couldn\'t have gotten without you.',0,0,1,0,'erland SAY_END'),
(-1000313,'It\'s good to see you again, Erland. What is your report?',0,0,33,1,'erland SAY_RANE'),
(-1000314,'Masses of wolves are to the east, and whoever lived at Malden\'s Orchard is gone.',0,0,1,1,'erland SAY_RANE_REPLY'),
(-1000315,'If I am excused, then I\'d like to check on Quinn...',0,0,1,1,'erland SAY_CHECK_NEXT'),
(-1000316,'Hello, Quinn. How are you faring?',0,0,1,1,'erland SAY_QUINN'),
(-1000317,'I\'ve been better. Ivar the Foul got the better of me...',0,0,33,1,'erland SAY_QUINN_REPLY'),
(-1000318,'Try to take better care of yourself, Quinn. You were lucky this time.',0,0,1,1,'erland SAY_BYE'),

(-1000319,'Let the trial begin, Bloodwrath, attack!',0,1,1,0,'kelerun SayId1'),
(-1000320,'Champion Lightrend, make me proud!',0,1,1,0,'kelerun SayId2'),
(-1000321,'Show this upstart how a real Blood Knight fights, Swiftblade!',0,1,1,0,'kelerun SayId3'),
(-1000322,'Show $n the meaning of pain, Sunstriker!',0,1,1,0,'kelerun SayId4'),

(-1000323,'Mist! I feared I would never see you again! Yes, I am well, do not worry for me. You must rest and recover your health.',0,0,7,0,'mist SAY_AT_HOME'),
(-1000324,'%s growls in acknowledgement before straightening and making her way off into the forest.',0,2,0,0,'mist EMOTE_AT_HOME'),

(-1000325,'"Threshwackonator First Mate unit prepared to follow"',0,2,0,0,'threshwackonator EMOTE_START'),
(-1000326,'YARRR! Swabie, what have ye done?! He\'s gone mad! Baton him down the hatches! Hoist the mast! ARRRR! Every man for hi\'self!',0,0,7,0,'threshwackonator SAY_AT_CLOSE'),

(-1000327,'Ok, $n, let\'s go find where I left that mysterious fossil. Follow me!',0,0,7,0,'remtravel SAY_REM_START'),
(-1000328,'Now where did I put that mysterious fossil? Ah, maybe up there...',0,0,7,0,'remtravel SAY_REM_RAMP1_1'),
(-1000329,'Hrm, nothing up here.',0,0,7,0,'remtravel SAY_REM_RAMP1_2'),
(-1000330,'No mysterious fossil here... Ah, but my copy of Green Hills of Stranglethorn. What a good book!',0,0,7,0,'remtravel SAY_REM_BOOK'),
(-1000331,'I bet you I left it in the tent!',0,0,7,0,'remtravel SAY_REM_TENT1_1'),
(-1000332,'Oh wait, that\'s Hollee\'s tent... and it\'s empty.',0,0,7,0,'remtravel SAY_REM_TENT1_2'),
(-1000333,'Interesting... I hadn\'t noticed this earlier...',0,0,7,0,'remtravel SAY_REM_MOSS'),
(-1000334,'%s inspects the ancient, mossy stone.',0,2,7,0,'remtravel EMOTE_REM_MOSS'),
(-1000335,'Oh wait! I\'m supposed to be looking for that mysterious fossil!',0,0,7,0,'remtravel SAY_REM_MOSS_PROGRESS'),
(-1000336,'Nope. didn\'t leave the fossil back here!',0,0,7,0,'remtravel SAY_REM_PROGRESS'),
(-1000337,'Ah. I remember now! I gave the mysterious fossil to Hollee! Check with her.',0,0,7,0,'remtravel SAY_REM_REMEMBER'),
(-1000338,'%s goes back to work, oblivious to everything around him.',0,2,7,0,'remtravel EMOTE_REM_END'),
(-1000339,'Something tells me this $r wants the mysterious fossil too. Help!',0,0,7,0,'remtravel SAY_REM_AGGRO'),

(-1000340,'%s howls in delight at the sight of his lunch!',0,2,0,0,'kyle EMOTE_SEE_LUNCH'),
(-1000341,'%s eats his lunch.',0,2,0,0,'kyle EMOTE_EAT_LUNCH'),
(-1000342,'%s thanks you with a special dance.',0,2,0,0,'kyle EMOTE_DANCE'),

(-1000343,'Is the way clear? Let\'s get out while we can, $N.',0,0,0,0,'kayra SAY_START'),
(-1000344,'Looks like we won\'t get away so easy. Get ready!',0,0,0,0,'kayra SAY_AMBUSH1'),
(-1000345,'Let\'s keep moving. We\'re not safe here!',0,0,0,0,'kayra SAY_PROGRESS'),
(-1000346,'Look out, $N! Enemies ahead!',0,0,0,0,'kayra SAY_AMBUSH2'),
(-1000347,'We\'re almost to the refuge! Let\'s go.',0,0,0,0,'kayra SAY_END'),

(-1000348,'Ah...the wondrous sound of kodos. I love the way they make the ground shake... inspect the beast for me.',0,0,0,0,'kodo round SAY_SMEED_HOME_1'),
(-1000349,'Hey, look out with that kodo! You had better inspect that beast before i give you credit!',0,0,0,0,'kodo round SAY_SMEED_HOME_2'),
(-1000350,'That kodo sure is a beauty. Wait a minute, where are my bifocals? Perhaps you should inspect the beast for me.',0,0,0,0,'kodo round SAY_SMEED_HOME_3'),

(-1000351,'You, there! Hand over that moonstone and nobody gets hurt!',0,1,0,0,'sprysprocket SAY_START'),
(-1000352,'%s takes the Southfury moonstone and escapes into the river. Follow her!',0,3,0,0,'sprysprocket EMOTE_START'),
(-1000353,'Just chill!',0,4,0,0,'sprysprocket SAY_WHISPER_CHILL'),
(-1000354,'Stupid grenade picked a fine time to backfire! So much for high quality goblin engineering!',0,1,0,0,'sprysprocket SAY_GRENADE_FAIL'),
(-1000355,'All right, you win! I surrender! Just don\'t hurt me!',0,1,0,0,'sprysprocket SAY_END'),

(-1000356,'Okay, okay... gimme a minute to rest now. You gone and beat me up good.',0,0,1,14,'calvin SAY_COMPLETE'),

(-1000357,'Let\'s go before they find out I\'m free!',0,0,0,1,'KAYA_SAY_START'),
(-1000358,'Look out! We\'re under attack!',0,0,0,0,'KAYA_AMBUSH'),
(-1000359,'Thank you for helping me. I know my way back from here.',0,0,0,0,'KAYA_END'),

(-1000360,'The strands of LIFE have been severed! The Dreamers must be avenged!',0,1,0,0,' ysondre SAY_AGGRO'),
(-1000361,'Come forth, ye Dreamers - and claim your vengeance!',0,1,0,0,' ysondre SAY_SUMMONDRUIDS'),

(-1000362,'Let\'s go $N. I am ready to reach Whitereach Post.',0,0,1,0,'paoka SAY_START'),
(-1000363,'Now this looks familiar. If we keep heading east, I think we can... Ahh, Wyvern on the attack!',0,0,1,0,'paoka SAY_WYVERN'),
(-1000364,'Thanks a bunch... I can find my way back to Whitereach Post from here. Be sure to talk with Motega Firemane; perhaps you can keep him from sending me home.',0,0,1,0,'paoka SAY_COMPLETE'),

(-1000365,'Be on guard... Arnak has some strange power over the Grimtotem... they will not be happy to see me escape.',0,0,1,0,'lakota SAY_LAKO_START'),
(-1000366,'Look out, the Grimtotem are upon us!',0,0,1,0,'lakota SAY_LAKO_LOOK_OUT'),
(-1000367,'Here they come.',0,0,1,0,'lakota SAY_LAKO_HERE_COME'),
(-1000368,'More Grimtotems are coming this way!',0,0,1,0,'lakota SAY_LAKO_MORE'),
(-1000369,'Finally, free at last... I must be going now, thanks for helping me escape. I can get back to Freewind Post by myself.',0,0,1,0,'lakota SAY_LAKO_END'),

(-1000370,'Stay close, $n. I\'ll need all the help I can get to break out of here. Let\'s go!',0,0,1,1,'gilthares SAY_GIL_START'),
(-1000371,'At last! Free from Northwatch Hold! I need a moment to catch my breath...',0,0,1,5,'gilthares SAY_GIL_AT_LAST'),
(-1000372,'Now I feel better. Let\'s get back to Ratchet. Come on, $n.',0,0,1,23,'gilthares SAY_GIL_PROCEED'),
(-1000373,'Looks like the Southsea Freebooters are heavily entrenched on the coast. This could get rough.',0,0,1,25,'gilthares SAY_GIL_FREEBOOTERS'),
(-1000374,'Help! $C attacking!',0,0,1,0,'gilthares SAY_GIL_AGGRO_1'),
(-1000375,'$C heading this way fast! Time for revenge!',0,0,1,0,'gilthares SAY_GIL_AGGRO_2'),
(-1000376,'$C coming right at us!',0,0,1,0,'gilthares SAY_GIL_AGGRO_3'),
(-1000377,'Get this $C off of me!',0,0,1,0,'gilthares SAY_GIL_AGGRO_4'),
(-1000378,'Almost back to Ratchet! Let\'s keep up the pace...',0,0,1,0,'gilthares SAY_GIL_ALMOST'),
(-1000379,'Ah, the sweet salt air of Ratchet.',0,0,1,0,'gilthares SAY_GIL_SWEET'),
(-1000380,'Captain Brightsun, $N here has freed me! $N, I am certain the Captain will reward your bravery.',0,0,1,66,'gilthares SAY_GIL_FREED'),

(-1000381,'I sense the tortured spirits, $n. They are this way, come quickly!',0,0,0,1,'wilda SAY_WIL_START'),
(-1000382,'Watch out!',0,0,0,0,'wilda SAY_WIL_AGGRO1'),
(-1000383,'Naga attackers! Defend yourself!',0,0,0,0,'wilda SAY_WIL_AGGRO2'),
(-1000384,'Grant me protection $n, i must break trough their foul magic!',0,0,0,0,'wilda SAY_WIL_PROGRESS1'),
(-1000385,'The naga of Coilskar are exceptionally cruel to their prisoners. It is a miracle that I survived inside that watery prison for as long as I did. Earthmother be praised.',0,0,0,0,'wilda SAY_WIL_PROGRESS2'),
(-1000386,'Now we must find the exit.',0,0,0,0,'wilda SAY_WIL_FIND_EXIT'),
(-1000387,'Lady Vashj must answer for these atrocities. She must be brought to justice!',0,0,0,0,'wilda SAY_WIL_PROGRESS4'),
(-1000388,'The tumultuous nature of the great waterways of Azeroth and Draenor are a direct result of tormented water spirits.',0,0,0,0,'wilda SAY_WIL_PROGRESS5'),
(-1000389,'It shouldn\'t be much further, $n. The exit is just up ahead.',0,0,0,0,'wilda SAY_WIL_JUST_AHEAD'),
(-1000390,'Thank you, $n. Please return to my brethren at the Altar of Damnation, near the Hand of Gul\'dan, and tell them that Wilda is safe. May the Earthmother watch over you...',0,0,0,0,'wilda SAY_WIL_END'),

(-1000391,'I\'m Thirsty.',0,0,0,0,'tooga SAY_TOOG_THIRST'),
(-1000392,'Torta must be so worried.',0,0,0,0,'tooga SAY_TOOG_WORRIED'),
(-1000393,'Torta, my love! I have returned at long last.',0,0,0,0,'tooga SAY_TOOG_POST_1'),
(-1000394,'You have any idea how long I\'ve been waiting here? And where\'s dinner? All that time gone and nothing to show for it?',0,0,0,0,'tooga SAY_TORT_POST_2'),
(-1000395,'My dearest Torta. I have been gone for so long. Finally we are reunited. At long last our love can blossom again.',0,0,0,0,'tooga SAY_TOOG_POST_3'),
(-1000396,'Enough with the rambling. I am starving! Now, get your dusty shell into that ocean and bring momma some grub.',0,0,0,0,'tooga SAY_TORT_POST_4'),
(-1000397,'Yes Torta. Whatever your heart desires...',0,0,0,0,'tooga SAY_TOOG_POST_5'),
(-1000398,'And try not to get lost this time...',0,0,0,0,'tooga SAY_TORT_POST_6'),

(-1000399,'Peace is but a fleeting dream! Let the NIGHTMARE reign!',0,1,0,0,'taerar SAY_AGGRO'),
(-1000400,'Children of Madness - I release you upon this world!',0,1,0,0,'taerar SAY_SUMMONSHADE'),

(-1000401,'Hope is a DISEASE of the soul! This land shall wither and die!',0,1,0,0,'emeriss SAY_AGGRO'),
(-1000402,'Taste your world\'s corruption!',0,1,0,0,'emeriss SAY_CASTCORRUPTION'),

(-1000403,'Rin\'ji is free!',0,0,0,0,'SAY_RIN_FREE'),
(-1000404,'Attack my sisters! The troll must not escape!',0,0,0,0,'SAY_RIN_BY_OUTRUNNER'),
(-1000405,'Rin\'ji needs help!',0,0,1,0,'SAY_RIN_HELP_1'),
(-1000406,'Rin\'ji is being attacked!',0,0,1,0,'SAY_RIN_HELP_2'),
(-1000407,'Rin\'ji can see road now, $n. Rin\'ji knows the way home.',0,0,1,0,'SAY_RIN_COMPLETE'),
(-1000408,'Rin\'ji will tell you secret now... $n, should go to the Overlook Cliffs. Rin\'ji hid something on island  there',0,0,1,0,'SAY_RIN_PROGRESS_1'),
(-1000409,'You find it, you keep it! Don\'t tell no one that Rin\'ji talked to you!',0,0,1,0,'SAY_RIN_PROGRESS_2'),

(-1000410,'Here they come! Defend yourself!',0,0,1,5,'kanati SAY_KAN_START'),

(-1000411,'Come, $N. See what the Nightmare brings...',0,4,0,0,'Twilight Corrupter SAY_TWILIGHT_CORRUPTER_SPAWN'),

-- REUSE -1000412 TO -1000414
(-1000415,'%s, too injured, gives up the chase.',0,2,0,0,'hendel EMOTE_SURRENDER'),

(-1000416,'Well, I\'m not sure how far I\'ll make it in this state... I\'m feeling kind of faint...',0,0,0,0,'ringo SAY_RIN_START_1'),
(-1000417,'Remember, if I faint again, the water that Spraggle gave you will revive me.',0,0,0,0,'ringo SAY_RIN_START_2'),
(-1000418,'The heat... I can\'t take it...',0,0,0,0,'ringo SAY_FAINT_1'),
(-1000419,'Maybe... you could carry me?',0,0,0,0,'ringo SAY_FAINT_2'),
(-1000420,'Uuuuuuggggghhhhh....',0,0,0,0,'ringo SAY_FAINT_3'),
(-1000421,'I\'m not feeling so well...',0,0,0,0,'ringo SAY_FAINT_4'),
(-1000422,'Where... Where am I?',0,0,0,0,'ringo SAY_WAKE_1'),
(-1000423,'I am feeling a little better now, thank you.',0,0,0,0,'ringo SAY_WAKE_2'),
(-1000424,'Yes, I must go on.',0,0,0,0,'ringo SAY_WAKE_3'),
(-1000425,'How am I feeling? Quite soaked, thank you.',0,0,0,0,'ringo SAY_WAKE_4'),
(-1000426,'Spraggle! I didn\'t think I\'d make it back!',0,0,0,0,'ringo SAY_RIN_END_1'),
(-1000427,'Ringo! You\'re okay!',0,0,0,0,'ringo SAY_SPR_END_2'),
(-1000428,'Oh... I\'m feeling faint...',0,0,0,0,'ringo SAY_RIN_END_3'),
(-1000429,'%s collapses onto the ground.',0,2,0,0,'ringo EMOTE_RIN_END_4'),
(-1000430,'%s stands up after a short pause.',0,2,0,0,'ringo EMOTE_RIN_END_5'),
(-1000431,'Ugh.',0,0,0,0,'ringo SAY_RIN_END_6'),
(-1000432,'Ringo? Wake up! Don\'t worry, I\'ll take care of you.',0,0,0,0,'ringo SAY_SPR_END_7'),
(-1000433,'%s fades away after a long pause.',0,2,0,0,'ringo EMOTE_RIN_END_8'),

(-1000434,'Liladris has been waiting for me at Maestra\'s Post, so we should make haste, $N.',0,0,0,0,'kerlonian SAY_KER_START'),
(-1000435,'%s looks very sleepy...',0,2,0,0,'kerlonian EMOTE_KER_SLEEP_1'),
(-1000436,'%s suddenly falls asleep',0,2,0,0,'kerlonian EMOTE_KER_SLEEP_2'),
(-1000437,'%s begins to drift off...',0,2,0,0,'kerlonian EMOTE_KER_SLEEP_3'),
(-1000438,'This looks like the perfect place for a nap...',0,0,0,0,'kerlonian SAY_KER_SLEEP_1'),
(-1000439,'Yaaaaawwwwwnnnn...',0,0,0,0,'kerlonian SAY_KER_SLEEP_2'),
(-1000440,'Oh, I am so tired...',0,0,0,0,'kerlonian SAY_KER_SLEEP_3'),
(-1000441,'You don\'t mind if I stop here for a moment, do you?',0,0,0,0,'kerlonian SAY_KER_SLEEP_4'),
(-1000442,'Be on the alert! The Blackwood furbolgs are numerous in the area...',0,0,0,0,'kerlonian SAY_KER_ALERT_1'),
(-1000443,'It\'s quiet... Too quiet...',0,0,0,0,'kerlonian SAY_KER_ALERT_2'),
(-1000444,'Oh, I can see Liladris from here... Tell her I\'m here, won\'t you?',0,0,0,0,'kerlonian SAY_KER_END'),
(-1000445,'%s wakes up!',0,2,0,0,'kerlonian EMOTE_KER_AWAKEN'),

(-1000446,'A-Me good. Good, A-Me. Follow... follow A-Me. Home. A-Me go home.',0,0,0,0,'ame01 SAY_AME_START'),
(-1000447,'Good... good, A-Me. A-Me good. Home. Find home.',0,0,0,0,'ame01 SAY_AME_PROGRESS'),
(-1000448,'A-Me home! A-Me good! Good A-Me. Home. Home. Home.',0,0,0,0,'ame01 SAY_AME_END'),
(-1000449,'$c, no hurt A-Me. A-Me good.',0,0,0,0,'ame01 SAY_AME_AGGRO1'),
(-1000450,'Danger. Danger! $c try hurt A-Me.',0,0,0,0,'ame01 SAY_AME_AGGRO2'),
(-1000451,'Bad, $c. $c, bad!',0,0,0,0,'ame01 SAY_AME_AGGRO3'),

(-1000452,'I noticed some fire on that island over there. A human, too. Let\'s go check it out, $n.',0,0,1,0,'ogron SAY_OGR_START'),
(-1000453,'That\'s Reethe alright. Let\'s go see what he has to say, yeah?',0,0,1,1,'ogron SAY_OGR_SPOT'),
(-1000454,'W-what do you want? Just leave me alone...',0,0,0,6,'ogron SAY_OGR_RET_WHAT'),
(-1000455,'I swear. I didn\'t steal anything from you! Here, take some of my supplies, just go away!',0,0,0,27,'ogron SAY_OGR_RET_SWEAR'),
(-1000456,'Just tell us what you know about the Shady Rest Inn, and I won\'t bash your skull in.',0,0,1,0,'ogron SAY_OGR_REPLY_RET'),
(-1000457,'I... Well, I may of taken a little thing or two from the inn... but what would an ogre care about that?',0,0,0,6,'ogron SAY_OGR_RET_TAKEN'),
(-1000458,'Look here, if you don\'t tell me about the fire--',0,0,1,0,'ogron SAY_OGR_TELL_FIRE'),
(-1000459,'Not one step closer, ogre!',0,0,0,27,'ogron SAY_OGR_RET_NOCLOSER'),
(-1000460,'And I don\'t know anything about this fire of yours...',0,0,0,0,'ogron SAY_OGR_RET_NOFIRE'),
(-1000461,'What was that? Did you hear something?',0,0,0,0,'ogron SAY_OGR_RET_HEAR'),
(-1000462,'Paval Reethe! Found you at last. And consorting with ogres now? No fear, even deserters and traitors are afforded some mercy.',0,0,0,0,'ogron SAY_OGR_CAL_FOUND'),
(-1000463,'Private, show Lieutenant Reethe some mercy.',0,0,0,29,'ogron SAY_OGR_CAL_MERCY'),
(-1000464,'Gladly, sir.',0,0,0,0,'ogron SAY_OGR_HALL_GLAD'),
(-1000465,'%s staggers backwards as the arrow lodges itself deeply in his chest.',0,2,0,0,'ogron EMOTE_OGR_RET_ARROW'),
(-1000466,'Ugh... Hallan, didn\'t think you had it in you...',0,0,0,34,'ogron SAY_OGR_RET_ARROW'),
(-1000467,'Now, let\'s clean up the rest of the trash, men!',0,0,0,0,'ogron SAY_OGR_CAL_CLEANUP'),
(-1000468,'Damn it! You\'d better not die on me, human!',0,0,1,0,'ogron SAY_OGR_NODIE'),
(-1000469,'Still with us, Reethe?',0,0,1,0,'ogron SAY_OGR_SURVIVE'),
(-1000470,'Must be your lucky day. Alright, I\'ll talk. Just leave me alone. Look, you\'re not going to believe me, but it wa... oh, Light, looks like the girl could shoot...',0,0,0,0,'ogron SAY_OGR_RET_LUCKY'),
(-1000471,'By the way, thanks for watching my back.',0,0,1,0,'ogron SAY_OGR_THANKS'),

(-1000472,'1...',0,3,0,0,'mana bomb SAY_COUNT_1'),
(-1000473,'2...',0,3,0,0,'mana bomb SAY_COUNT_2'),
(-1000474,'3...',0,3,0,0,'mana bomb SAY_COUNT_3'),
(-1000475,'4...',0,3,0,0,'mana bomb SAY_COUNT_4'),
(-1000476,'5...',0,3,0,0,'mana bomb SAY_COUNT_5'),

(-1000477,'Let us leave this place. I\'ve had enough of these madmen!',0,0,0,0,'akuno SAY_AKU_START'),
(-1000478,'You\'ll go nowhere, fool!',0,0,0,0,'akuno SAY_AKU_AMBUSH_A'),
(-1000479,'Beware! More cultists come!',0,0,0,0,'akuno SAY_AKU_AMBUSH_B'),
(-1000480,'You will not escape us so easily!',0,0,0,0,'akuno SAY_AKU_AMBUSH_B_REPLY'),
(-1000481,'I can find my way from here. My friend Mekeda will reward you for your kind actions.',0,0,0,0,'akuno SAY_AKU_COMPLETE'),

(-1000482,'Look out!',0,0,0,0,'maghar captive SAY_MAG_START'),
(-1000483,'Don\'t let them escape! Kill the strong one first!',0,0,0,0,'maghar captive SAY_MAG_NO_ESCAPE'),
(-1000484,'More of them coming! Watch out!',0,0,0,0,'maghar captive SAY_MAG_MORE'),
(-1000485,'Where do you think you\'re going? Kill them all!',0,0,0,0,'maghar captive SAY_MAG_MORE_REPLY'),
(-1000486,'Ride the lightning, filth!',0,0,0,0,'maghar captive SAY_MAG_LIGHTNING'),
(-1000487,'FROST SHOCK!!!',0,0,0,0,'maghar captive SAY_MAG_SHOCK'),
(-1000488,'It is best that we split up now, in case they send more after us. Hopefully one of us will make it back to Garrosh. Farewell stranger.',0,0,0,0,'maghar captive SAY_MAG_COMPLETE'),

(-1000489,'Show our guest around Shattrath, will you? Keep an eye out for pickpockets in the lower city.',0,0,0,0,'SAY_KHAD_START'),
(-1000490,'A newcomer to Shattrath! Make sure to drop by later. We can always use a hand with the injured.',0,0,0,0,'SAY_KHAD_INJURED'),
(-1000491,'Be mindful of what you say, this one\'s being shown around by Khadgar\'s pet.',0,0,0,0,'SAY_KHAD_MIND_YOU'),
(-1000492,'Are you joking? I\'m a Scryer, I always watch what i say. More enemies than allies in this city, it seems.',0,0,0,0,'SAY_KHAD_MIND_ALWAYS'),
(-1000493,'Light be with you, $n. And welcome to Shattrath.',0,0,0,0,'SAY_KHAD_ALDOR_GREET'),
(-1000494,'We\'re rather selective of who we befriend, $n. You think you have what it takes?',0,0,0,0,'SAY_KHAD_SCRYER_GREET'),
(-1000495,'Khadgar himself is showing you around? You must have made a good impression, $n.',0,0,0,0,'SAY_KHAD_HAGGARD'),

(-1000496,'%s lifts its head into the air, as if listening for something.',0,2,0,0,'ancestral wolf EMOTE_WOLF_LIFT_HEAD'),
(-1000497,'%s lets out a howl that rings across the mountains to the north and motions for you to follow.',0,2,0,0,'ancestral wolf EMOTE_WOLF_HOWL'),
(-1000498,'Welcome, kind spirit. What has brought you to us?',0,0,0,0,'ancestral wolf SAY_WOLF_WELCOME'),

(-1000499,'Puny $r wanna fight %s? Me beat you! Me boss here!',0,0,1,0,'morokk SAY_MOR_CHALLENGE'),
(-1000500,'Me scared! Me run now!',0,1,0,0,'morokk SAY_MOR_SCARED'),

(-1000501,'Are you sure that you are ready? If we do not have a group of your allies to aid us, we will surely fail.',0,0,1,0,'muglash SAY_MUG_START1'),
(-1000502,'This will be a tough fight, $n. Follow me closely.',0,0,1,0,'muglash SAY_MUG_START2'),
(-1000503,'This is the brazier, $n. Put it out. Vorsha is a beast, worthy of praise from no one!',0,0,1,0,'muglash SAY_MUG_BRAZIER'),
(-1000504,'Now we must wait. It won\'t be long before the naga realize what we have done.',0,0,1,0,'muglash SAY_MUG_BRAZIER_WAIT'),
(-1000505,'Be on your guard, $n!',0,0,1,0,'muglash SAY_MUG_ON_GUARD'),
(-1000506,'Perhaps we will get a moment to rest. But i will not give up until we have faced off against Vorsha!',0,0,1,0,'muglash SAY_MUG_REST'),
(-1000507,'We have done it!',0,0,1,0,'muglash SAY_MUG_DONE'),
(-1000508,'You have my deepest gratitude. I thank you.',0,0,1,0,'muglash SAY_MUG_GRATITUDE'),
(-1000509,'I am going to patrol the area for a while longer and ensure that things are truly safe.',0,0,1,0,'muglash SAY_MUG_PATROL'),
(-1000510,'Please return to Zoram\'gar and report our success to the Warsong runner.',0,0,1,0,'muglash SAY_MUG_RETURN'),

(-1000511,'Aright, listen up! Form a circle around me and move out!',0,0,0,0,'letoll SAY_LE_START'),
(-1000512,'Aright, $r, just keep us safe from harm while we work. We\'ll pay you when we return.',0,0,0,0,'letoll SAY_LE_KEEP_SAFE'),
(-1000513,'The dig site is just north of here.',0,0,0,0,'letoll SAY_LE_NORTH'),
(-1000514,'We\'re here! Start diggin\'!',0,0,0,0,'letoll SAY_LE_ARRIVE'),
(-1000515,'I think there\'s somethin\' buried here, beneath the sand!',0,0,0,0,'letoll SAY_LE_BURIED'),
(-1000516,'Almost got it!',0,0,0,0,'letoll SAY_LE_ALMOST'),
(-1000517,'By brann\'s brittle bananas! What is it!? It... It looks like a drum.',0,0,0,0,'letoll SAY_LE_DRUM'),
(-1000518,'Wow... a drum.',0,0,0,0,'letoll SAY_LE_DRUM_REPLY'),
(-1000519,'This discovery will surely rock the foundation of modern archaeology.',0,0,0,0,'letoll SAY_LE_DISCOVERY'),
(-1000520,'Yea, great. Can we leave now? This desert is giving me hives.',0,0,0,0,'letoll SAY_LE_DISCOVERY_REPLY'),
(-1000521,'Have ye gone mad? You expect me to leave behind a drum without first beatin\' on it? Not this son of Ironforge! No sir!',0,0,0,0,'letoll SAY_LE_NO_LEAVE'),
(-1000522,'This reminds me of that one time where you made us search Silithus for evidence of sand gnomes.',0,0,0,0,'letoll SAY_LE_NO_LEAVE_REPLY1'),
(-1000523,'Or that time when you told us that you\'d discovered the cure for the plague of the 20th century. What is that even? 20th century?',0,0,0,0,'letoll SAY_LE_NO_LEAVE_REPLY2'),
(-1000524,'I don\'t think it can top the one time where he told us that he\'d heard that Artha\'s "cousin\'s" skeleton was frozen beneath a glacier in Winterspring. I\'ll never forgive you for that one, Letoll. I mean honestly... Artha\'s cousin?',0,0,0,0,'letoll SAY_LE_NO_LEAVE_REPLY3'),
(-1000525,'I dunno. It can\'t possibly beat the time he tried to convince us that we\'re all actually a figment of some being\'s imagination and that they only use us for their own personal amusement. That went over well during dinner with the family.',0,0,0,0,'letoll SAY_LE_NO_LEAVE_REPLY4'),
(-1000526,'Shut yer yaps! I\'m gonna bang on this drum and that\'s that!',0,0,0,0,'letoll SAY_LE_SHUT'),
(-1000527,'Say, do you guys hear that?',0,0,0,0,'letoll SAY_LE_REPLY_HEAR'),
(-1000528,'IN YOUR FACE! I told you there was somethin\' here!',0,0,0,0,'letoll SAY_LE_IN_YOUR_FACE'),
(-1000529,'Don\'t just stand there! Help him out!',0,0,0,0,'letoll SAY_LE_HELP_HIM'),
(-1000530,'%s picks up the drum.',0,2,0,0,'letoll EMOTE_LE_PICK_UP'),
(-1000531,'You\'ve been a tremendous help, $r! Let\'s get out of here before more of those things show up! I\'ll let Dwarfowitz know you did the job asked of ya\' admirably.',0,0,0,0,'letoll SAY_LE_THANKS'),

(-1000532,'At your command, my liege...',0,0,0,0,'torloth TORLOTH_DIALOGUE1'),
(-1000533,'As you desire, Lord Illidan.',0,0,0,0,'torloth TORLOTH_DIALOGUE2'),
(-1000534,'Yes, Lord Illidan, I would sacrifice to you this magnificent physique. On this day you will fall - another victim of Torloth...',0,0,0,0,'torloth TORLOTH_DIALOGUE3'),
(-1000535,'Destroy them, Torloth. Let loose their blood like a river upon this hallowed ground.',0,0,0,0,'lordillidan ILLIDAN_DIALOGUE'),
(-1000536,'What manner of fool dares stand before Illidan Stormrage? Soldiers, destroy these insects!',0,1,0,0,'lordillidan ILLIDAN_SUMMON1'),
(-1000537,'You are no challenge for the Crimson Sigil. Mind breakers, end this nonsense.',0,1,0,0,'lordillidan ILLIDAN_SUMMON2'),
(-1000538,'Run while you still can. The highlords come soon...',0,1,0,0,'lordillidan ILLIDAN_SUMMON3'),
(-1000539,'Torloth your master calls!',0,1,0,0,'lordillidan ILLIDAN_SUMMON4'),
(-1000540,'So you have defeated the Crimson Sigil. You now seek to challenge my rule? Not even Arthas could defeat me, yet you dare to even harbor such thoughts? Then I say to you, come! Come $N! The Black Temple awaits...',0,1,0,0,'lordillidan EVENT_COMPLETED'),

(-1000541,'%s jumps into the moonwell and goes underwater...',0,2,0,0,'kitten EMOTE_SAB_JUMP'),
(-1000542,'%s follows $n obediently.',0,2,0,0,'kitten EMOTE_SAB_FOLLOW'),

(-1000543,'Why have you come here, outsider? You will only find pain! Our fate will be yours...',0,0,0,25,'restless app SAY_RAND_1'),
(-1000544,'It was ... terrible... the demon...',0,0,0,25,'restless app SAY_RAND_2'),
(-1000545,'The darkness... the corruption... they came too quickly for anyone to know...',0,0,0,25,'restless app SAY_RAND_3'),
(-1000546,'The darkness will consume all... all the living...',0,0,0,25,'restless app SAY_RAND_4'),
(-1000547,'It is too late for us, living one. Take yourself and your friend away from here before you both are... claimed...',0,0,0,25,'restless app SAY_RAND_5'),
(-1000548,'It is too late for Jarl... its hold is too strong...',0,0,0,25,'restless app SAY_RAND_6'),
(-1000549,'Go away, whoever you are! Witch Hill is mine... mine!',0,0,0,25,'restless app SAY_RAND_7'),
(-1000550,'The manor... someone else... will soon be consumed...',0,0,0,25,'restless app SAY_RAND_8'),

(-1000553,'Be ready, $N. I hear the council returning. Prepare to ambush!',0,0,0,0,'deathstalker_faerleia SAY_START'),
(-1000554,'Well done. A blow to Arugal no doubt!',0,0,0,0,'deathstalker_faerleia SAY_END'),

(-1000561,'My wounds are grave. Forgive my slow pace but my injuries won\'t allow me to walk any faster.',0,0,0,0,'SAY_CORPORAL_KEESHAN_1'),
(-1000562,'Ah, fresh air, at last! I need a moment to rest.',0,0,0,0,'SAY_CORPORAL_KEESHAN_2'),
(-1000563,'The Blackrock infestation is thick in these parts. I will do my best to keep the pace. Let\'s go!',0,0,0,0,'SAY_CORPORAL_KEESHAN_3'),
(-1000564,'Marshal Marris, sir. Corporal Keeshan of the 12th Sabre Regiment returned from battle and reporting for duty!',0,0,0,0,'SAY_CORPORAL_KEESHAN_4'),
(-1000565,'Brave adventurer, thank you for rescuing me! I am sure Marshal Marris will reward your kind deed.',0,0,0,0,'SAY_CORPORAL_KEESHAN_5'),

(-1000566,'Stand back! Stand clear! The infernal will need to be given a wide berth!',0,0,0,0,'SAY_NIBY_1'),
(-1000567,'BOW DOWN TO THE ALMIGHTY! BOW DOWN BEFORE MY INFERNAL DESTRO... chicken?',0,0,0,0,'SAY_NIBY_2'),
(-1000568,'%s rolls on the floor laughing.',0,2,0,0,'EMOTE_IMPSY_1'),
(-1000569,'Niby, you\' re an idiot.',0,0,0,0,'SAY_IMPSY_1'),
(-1000570,'Silence, servant! Vengeance will be mine! Death to Stormwind! Death by chicken!',0,0,0,0,'SAY_NIBY_3'),

(-1000571,'Help! I\'ve only one hand to defend myself with.',0,0,0,0,'SAY_MIRAN_1'),
(-1000572,'Feel the power of the Dark Iron Dwarves!',0,0,0,0,'SAY_DARK_IRON_DWARF'),
(-1000573,'Send them on! I\'m not afraid of some scrawny beasts!',0,0,0,0,'SAY_MIRAN_2'),
(-1000574,'Ah, here at last! It\'s going to feel so good to get rid of these barrels.',0,0,0,0,'SAY_MIRAN_3'),

(-1000579,'REUSE ME',0,0,0,0,'REUSE ME'),

(-1000582,'Help! Please, You must help me!',0,0,0,0,'Galen - periodic say'),
(-1000583,'Let us leave this place.',0,0,0,0,'Galen - quest accepted'),
(-1000584,'Look out! The $c attacks!',0,0,0,0,'Galen - aggro 1'),
(-1000585,'Help! I\'m under attack!',0,0,0,0,'Galen - aggro 2'),
(-1000586,'Thank you $N. I will remember you always. You can find my strongbox in my camp, north of Stonard.',0,0,0,0,'Galen - quest complete'),
(-1000587,'%s whispers to $N the secret to opening his strongbox.',0,2,0,0,'Galen - emote whisper'),
(-1000588,'%s disappears into the swamp.',0,2,0,0,'Galen - emote disappear'),

(-1000589,'Kroshius live? Kroshius crush!',0,1,0,0,'SAY_KROSHIUS_REVIVE'),

(-1000590,'Woot!',0,0,0,0,'Captive Child SAY_THANKS_1'),
(-1000591,'I think those weird bird guys were going to eat us. Gross!',0,0,0,0,'Captive Child SAY_THANKS_2'),
(-1000592,'Yay! We\'re free!',0,0,0,0,'Captive Child SAY_THANKS_3'),
(-1000593,'Gross!',0,0,0,0,'Captive Child SAY_THANKS_4'),

(-1000603,'Do not test me, scurvy dog! I\'m trained in the way of the Blood Knights!',0,0,0,0,'silvermoon harry SAY_AGGRO'),
(-1000604,'I\'ll pay! I\'ll pay! Eeeek! Please don\'t hurt me!',0,0,0,0,'silvermoon harry SAY_BEATEN'),

(-1000615,'Use my shinies...make weather good again...make undead things go away.',0,0,0,0,'mosswalker victim SAY_DIE_1'),
(-1000616,'We gave shinies to shrine... we not greedy... why this happen?',0,0,0,0,'mosswalker victim SAY_DIE_2'),
(-1000617,'I do something bad? I sorry....',0,0,0,0,'mosswalker victim SAY_DIE_3'),
(-1000618,'We not do anything... to them... I no understand.',0,0,0,0,'mosswalker victim SAY_DIE_4'),
(-1000619,'Thank...you.',0,0,0,0,'mosswalker victim SAY_DIE_5'),
(-1000620,'Please take... my shinies. All done...',0,0,0,0,'mosswalker victim SAY_DIE_6'),

(-1000621,'All systems on-line.  Prepare yourself, we leave shortly.',0,0,0,0,'maxx SAY_START'),
(-1000622,'Be careful in there and come back in one piece!',0,0,0,0,'maxx SAY_ALLEY_FAREWELL'),
(-1000623,'Proceed.',0,0,0,0,'maxx SAY_CONTINUE'),
(-1000624,'You\'re back!  Were you able to get all of the machines?',0,0,0,0,'maxx SAY_ALLEY_FINISH'),

(-1000625,'%s gathers the warp chaser\'s blood.',0,2,0,0,'zeppit EMOTE_GATHER_BLOOD'),

(-1000628,'%s feeds on the freshly-killed warp chaser.',0,2,0,0,'hungry ray EMOTE_FEED'),

(-1000629,'<cough> <cough> Damsel in distress over here!',0,0,0,0,'isla starmane - SAY_ISLA_PERIODIC_1'),
(-1000630,'Hello? Help?',0,0,0,0,'isla starmane - SAY_ISLA_PERIODIC_2'),
(-1000631,'Don\'t leave me in here! Cause if you do I will find you!',0,0,0,0,'isla starmane - SAY_ISLA_PERIODIC_3'),
(-1000632,'Ok, let\'s get out of here!',0,0,0,0,'isla starmane - SAY_ISLA_START'),
(-1000633,'You sure you\'re ready? Take a moment.',0,0,0,0,'isla starmane - SAY_ISLA_WAITING'),
(-1000634,'Alright, let\'s do this!',0,0,0,0,'isla starmane - SAY_ISLA_LEAVE_BUILDING'),

(-1000644,'Ouch! That\'s it, I quit the target business!',0,0,0,0,'SAY_LUCKY_HIT_1'),
(-1000645,'My ear! You grazed my ear!',0,0,0,0,'SAY_LUCKY_HIT_2'),
(-1000646,'Not the \'stache! Now I\'m asymmetrical!',0,0,0,5,'SAY_LUCKY_HIT_3'),
(-1000647,'Good shot!',0,0,0,4,'SAY_LUCKY_HIT_APPLE'),
(-1000648,'Stop whining. You\'ve still got your luck.',0,0,0,0,'SAY_DROSTAN_GOT_LUCKY_1'),
(-1000649,'Bah, it\'s an improvement.',0,0,0,11,'SAY_DROSTAN_GOT_LUCKY_2'),
(-1000650,'Calm down lad, it\'s just a birdshot!',0,0,0,0,'SAY_DROSTAN_HIT_BIRD_1'),
(-1000651,'The only thing hurt is your pride, lad! Buck up!',0,0,0,0,'SAY_DROSTAN_HIT_BIRD_2'),

(-1000652,'Me so hungry! YUM!',0,0,0,71,'dragonmaw peon SAY_PEON_1'),
(-1000653,'Hey... me not feel so good.',0,0,0,0,'dragonmaw peon SAY_PEON_2'),
(-1000654,'You is bad orc... baaad... or... argh!',0,0,0,0,'dragonmaw peon SAY_PEON_3'),
(-1000655,'Time for eating!?',0,0,0,71,'dragonmaw peon SAY_PEON_4'),
(-1000656,'It put the mutton in the stomach!',0,0,0,71,'dragonmaw peon SAY_PEON_5'),

(-1000666,'I can sense the SHADOW on your hearts. There can be no rest for the wicked!',0,1,0,0,'lethon SAY_LETHON_AGGRO'),
(-1000667,'Your wicked souls shall feed my power!',0,1,0,0,'lethon SAY_LETHON_SHADE'),

(-1000668,'%s releases the last of its energies into the nearby runestone, successfully reactivating it.',0,2,0,0,'infused crystal SAY_DEFENSE_FINISH'),

(-1000669,'We will locate the origin of the Nightmare through the fragments you collected, $N. From there, we will pull Eranikus through a rift in the Dream. Steel yourself, $C. We are inviting the embodiment of the Nightmare into our world.',0,0,0,0,'remulos SAY_REMULOS_INTRO_1'),
(-1000670,'To Nighthaven! Keep your army close, champion. ',0,0,0,0,'remulos SAY_REMULOS_INTRO_2'),
(-1000671,'The rift will be opened there, above the Lake Elun\'ara. Prepare yourself, $N. Eranikus\'s entry into our world will be wrought with chaos and strife.',0,0,0,0,'remulos SAY_REMULOS_INTRO_3'),
(-1000672,'He will stop at nothing to get to Malfurion\'s physical manifestation. That must not happen... We must keep the beast occupied long enough for Tyrande to arrive.',0,0,0,0,'remulos SAY_REMULOS_INTRO_4'),
(-1000673,'Defend Nightaven, hero...',0,0,0,0,'remulos SAY_REMULOS_INTRO_5'),
(-1000674,'%s has entered our world',0,3,0,0,'eranikus EMOTE_SUMMON_ERANIKUS'),
(-1000675,'Pitful predictable mortals... You know not what you have done! The master\'s will fulfilled. The Moonglade shall be destroyed and Malfurion along with it!',0,1,0,0,'eranikus SAY_ERANIKUS_SPAWN'),
(-1000676,'Fiend! Face the might of Cenarius!',0,1,0,1,'remulos SAY_REMULOS_TAUNT_1'),
(-1000677,'%s lets loose a sinister laugh.',0,2,0,0,'eranikus EMOTE_ERANIKUS_LAUGH'),
(-1000678,'You are certainly not your father, insect. Should it interest me, I would crush you with but a swipe of my claws. Turn Shan\'do Stormrage over to me and your pitiful life will be spared along with the lives of your people.',0,1,0,0,'eranikus SAY_ERANIKUS_TAUNT_2'),
(-1000679,'Who is the predictable one, beast? Surely you did not think that we would summon you on top of Malfurion? Your redemption comes, Eranikus. You will be cleansed of this madness - this corruption.',0,1,0,1,'remulos SAY_REMULOS_TAUNT_3'),
(-1000680,'My redemption? You are bold, little one. My redemption comes by the will of my god.',0,1,0,0,'eranikus SAY_ERANIKUS_TAUNT_4'),
(-1000681,'%s roars furiously.',0,2,0,0,'eranikus EMOTE_ERANIKUS_ATTACK'),
(-1000682,'Hurry, $N! We must find protective cover!',0,0,0,0,'remulos SAY_REMULOS_DEFEND_1'),
(-1000683,'Please, champion, protect our people.',0,0,0,1,'remulos SAY_REMULOS_DEFEND_2'),
(-1000684,'Rise, servants of the Nightmare! Rise and destroy this world! Let there be no survivors...',0,1,0,0,'eranikus SAY_ERANIKUS_SHADOWS'),
(-1000685,'We will battle these fiends, together! Nighthaven\'s Defenders are also among us. They will fight to the death if asked. Now, quickly, we must drive these aberrations back to the Nightmare. Destroy them all!',0,0,0,1,'remulos SAY_REMULOS_DEFEND_3'),
(-1000686,'Where is your savior? How long can you hold out against my attacks?',0,1,0,0,'eranikus SAY_ERANIKUS_ATTACK_1'),
(-1000687,'Defeated my minions? Then face me, mortals!',0,1,0,0,'eranikus SAY_ERANIKUS_ATTACK_2'),
(-1000688,'Remulos, look how easy they fall before me? You can stop this, fool. Turn the druid over to me and it will all be over...',0,1,0,0,'eranikus SAY_ERANIKUS_ATTACK_3'),
(-1000689,'Elune, hear my prayers. Grant us serenity! Watch over our fallen...',0,1,0,0,'tyrande SAY_TYRANDE_APPEAR'),
(-1000690,'Tend to the injuries of the wounded, sisters!',0,0,0,0,'tyrande SAY_TYRANDE_HEAL'),
(-1000691,'Seek absolution, Eranikus. All will be forgiven...',0,1,0,0,'tyrande SAY_TYRANDE_FORGIVEN_1'),
(-1000692,'You will be forgiven, Eranikus. Elune will always love you. Break free of the bonds that command you!',0,1,0,0,'tyrande SAY_TYRANDE_FORGIVEN_2'),
(-1000693,'The grasp of the Old Gods is unmoving. He is consumed by their dark thoughts... I... I... I cannot... cannot channel much longer... Elune aide me.',0,0,0,0,'tyrande SAY_TYRANDE_FORGIVEN_3'),
(-1000694,'IT BURNS! THE PAIN.. SEARING...',0,1,0,0,'eranikus SAY_ERANIKUS_DEFEAT_1'),
(-1000695,'WHY? Why did this happen to... to me? Where were you Tyrande? Where were you when I fell from the grace of Elune?',0,1,0,0,'eranikus SAY_ERANIKUS_DEFEAT_2'),
(-1000696,'I... I feel... I feel the touch of Elune upon my being once more... She smiles upon me... Yes... I...', 0,1,0,0,'eranikus SAY_ERANIKUS_DEFEAT_3'),
(-1000697,'%s is wholly consumed by the Light of Elune. Tranquility sets in over the Moonglade',0,3,0,0,'eranikus EMOTE_ERANIKUS_REDEEM'),
(-1000698,'%s falls to one knee.',0,2,0,0,'tyrande EMOTE_TYRANDE_KNEEL'),
(-1000699,'Praise be to Elune... Eranikus is redeemed.',0,1,0,0,'tyrande SAY_TYRANDE_REDEEMED'),
(-1000700,'For so long, I was lost... The Nightmare\'s corruption had consumed me... And now, you... all of you... you have saved me. Released me from its grasp.',0,0,0,0,'eranikus SAY_REDEEMED_1'),
(-1000701,'But... Malfurion, Cenarius, Ysera... They still fight. They need me. I must return to the Dream at once.', 0,0,0,0,'eranikus SAY_REDEEMED_2'),
(-1000702,'My lady, I am unworthy of your prayer. Truly, you are an angel of light. Please, assist me in returning to the barrow den so that I may return to the Dream. I, like Malfurion, also have a love awaiting me... I must return to her... to protect her...', 0,0,0,0,'eranikus SAY_REDEEMED_3'),
(-1000703,'And heroes... I hold that which you seek. May it once more see the evil dissolved. Remulos, see to it that our champion receives the shard of the Green Flight.',0,0,0,0,'eranikus SAY_REDEEMED_4'),
(-1000704,'It will be done, Eranikus. Be well, ancient one.',0,0,0,0,'remulos SAY_REMULOS_OUTRO_1'),
(-1000705,'Let us leave Nighthave, hero. Seek me out at the grove.',0,0,0,0,'remulos SAY_REMULOS_OUTRO_2'),
(-1000706,'Your world shall suffer an unmerciful end. The Nightmare comes for you!',0,0,0,0,'eranikus SAY_ERANIKUS_KILL'),

(-1000707,'This blue light... It\'s strange. What do you think it means?',0,0,0,0,'Ranshalla SAY_ENTER_OWL_THICKET'),
(-1000708,'We\'ve found it!',0,0,0,0,'Ranshalla SAY_REACH_TORCH_1'),
(-1000709,'Please, light this while I am channeling',0,0,0,0,'Ranshalla SAY_REACH_TORCH_2'),
(-1000710,'This is the place. Let\'s light it.',0,0,0,0,'Ranshalla SAY_REACH_TORCH_3'),
(-1000711,'Let\'s find the next one.',0,0,0,0,'Ranshalla SAY_AFTER_TORCH_1'),
(-1000712,'We must continue on now.',0,0,0,0,'Ranshalla SAY_AFTER_TORCH_2'),
(-1000713,'It is time for the final step; we must activate the altar.',0,0,0,0,'Ranshalla SAY_REACH_ALTAR_1'),
(-1000714,'I will read the words carved into the stone, and you must find a way to light it.',0,0,0,0,'Ranshalla SAY_REACH_ALTAR_2'),
(-1000715,'The altar is glowing! We have done it!',0,0,0,0,'Ranshalla SAY_RANSHALLA_ALTAR_1'),
(-1000716,'What is happening? Look!',0,0,0,0,'Ranshalla SAY_RANSHALLA_ALTAR_2'),
(-1000717,'It has been many years...',0,0,0,0,'Priestess of Elune SAY_PRIESTESS_ALTAR_3'),
(-1000718,'Who has disturbed the altar of the goddess?',0,0,0,0,'Priestess of Elune SAY_PRIESTESS_ALTAR_4'),
(-1000719,'Please, priestesses, forgive us for our intrusion. We do not wish any harm here.',0,0,0,0,'Ranshalla SAY_RANSHALLA_ALTAR_5'),
(-1000720,'We only wish to know why the wildkin guard this area...',0,0,0,0,'Ranshalla SAY_RANSHALLA_ALTAR_6'),
(-1000721,'Enu thora\'serador. This is a sacred place.',0,0,0,0,'Priestess of Elune SAY_PRIESTESS_ALTAR_7'),
(-1000722,'We will show you...',0,0,0,0,'Priestess of Elune SAY_PRIESTESS_ALTAR_8'),
(-1000723,'Look above you; thara dormil dorah...',0,0,0,0,'Priestess of Elune SAY_PRIESTESS_ALTAR_9'),
(-1000724,'This gem once allowed direct communication with Elune, herself.',0,0,0,0,'Priestess of Elune SAY_PRIESTESS_ALTAR_10'),
(-1000725,'Through the gem, Elune channeled her infinite wisdom...',0,0,0,0,'Priestess of Elune SAY_PRIESTESS_ALTAR_11'),
(-1000726,'Realizing that the gem needed to be protected, we turned to the goddess herself.',0,0,0,0,'Priestess of Elune SAY_PRIESTESS_ALTAR_12'),
(-1000727,'Soon after, we began to have visions of a creature... A creature with the feathers of an owl, but the will and might of a bear...',0,0,0,0,'Priestess of Elune SAY_PRIESTESS_ALTAR_13'),
(-1000728,'It was on that day that the wildkin were given to us. Fierce guardians, the goddess assigned the wildkin to protect all of her sacred places.',0,0,0,0,'Priestess of Elune SAY_PRIESTESS_ALTAR_14'),
(-1000729,'Anu\'dorini Talah, Ru shallora enudoril.',0,0,0,0,'Voice of Elune SAY_VOICE_ALTAR_15'),
(-1000730,'But now, many years later, the wildkin have grown more feral, and without the guidance of the goddess, they are confused...',0,0,0,0,'Priestess of Elune SAY_PRIESTESS_ALTAR_16'),
(-1000731,'Without a purpose, they wander... But many find their way back to the sacred areas that they once were sworn to protect.',0,0,0,0,'Priestess of Elune SAY_PRIESTESS_ALTAR_17'),
(-1000732,'Wildkin are inherently magical; this power was bestowed upon them by the goddess.',0,0,0,0,'Priestess of Elune SAY_PRIESTESS_ALTAR_18'),
(-1000733,'Know that wherever you might find them in the world, they are protecting something of importance, as they were entrusted to do so long ago.',0,0,0,0,'Priestess of Elune SAY_PRIESTESS_ALTAR_19'),
(-1000734,'Please, remember what we have shown you...',0,0,0,0,'Priestess of Elune SAY_PRIESTESS_ALTAR_20'),
(-1000735,'Farewell.',0,0,0,0,'Priestess of Elune SAY_PRIESTESS_ALTAR_21'),
(-1000736,'Thank you for your help, $n. I wish you well in your adventures.',0,0,0,0,'Ranshalla SAY_QUEST_END_1'),
(-1000737,'I want to stay here and reflect on what we have seen. Please see Erelas and tell him what we have learned.',0,0,0,0,'Ranshalla SAY_QUEST_END_2'),
(-1000738,'%s begins chanting a strange spell...',0,2,0,0,'Ranshalla EMOTE_CHANT_SPELL'),
(-1000739,'Remember, I need your help to properly channel. I will ask you to aid me several times in our path, so please be ready.',0,0,0,0,'Ranshalla SAY_QUEST_START'),

(-1000740,'We must act quickly or all shall be lost!',0,0,0,1,'SAY_ANACHRONOS_INTRO_1'),
(-1000741,'My forces cannot overcome the Qiraji defenses. We will not be able to get close enough to place your precious barrier, dragon.',0,0,0,0,'SAY_FANDRAL_INTRO_2'),
(-1000742,'There is a way...',0,0,0,22,'SAY_MERITHRA_INTRO_3'),
(-1000743,'%s nods knowingly.',0,2,0,0,'EMOTE_ARYGOS_NOD'),
(-1000744,'Aye, Fandral, remember these words: Let not your grief guide your faith. These thoughts you hold... dark places you go, night elf. Absolution cannot be had through misguided vengeance.',0,0,0,1,'SAY_CAELESTRASZ_INTRO_4'),
(-1000745,'%s glances at her compatriots.',0,2,0,0,'EMOTE_MERITHRA_GLANCE'),
(-1000746,'We will push him back, Anachronos. This I vow. Uphold your end of this task. Let not your hands falter as you seal our fates behind the barrier.',0,0,0,1,'SAY_MERITHRA_INTRO_5'),
(-1000747,'Succumb to the endless dream, little ones. Let it consume you!',0,1,0,22,'SAY_MERITHRA_ATTACK_1'),
(-1000748,'Anachronos, this diversion will give you and the young druid time enough to seal the gate. Do not falter. Now, let us see how they deal with chaotic magic.',0,0,0,1,'SAY_ARYGOS_ATTACK_2'),
(-1000749,'Let them feel the wrath of the Blue Flight! May Malygos protect me!',0,1,0,22,'SAY_ARYGOS_ATTACK_3'),
(-1000750,'Do not forget the sacrifices made on this day, night elf. We have all suffered immensely at the hands of these beasts.',0,0,0,1,'SAY_CAELESTRASZ_ATTACK_4'),
(-1000751,'Alexstrasza grant me the resolve to drive our enemies back!',0,1,0,22,'SAY_CAELESTRASZ_ATTACK_5'),
(-1000752,'NOW, STAGHELM! WE GO NOW! Prepare your magic!',0,0,0,22,'SAY_ANACHRONOS_SEAL_1'),
(-1000753,'It is done, dragon. Lead the way!',0,0,0,25,'SAY_FANDRAL_SEAL_2'),
(-1000754,'Stay close...',0,0,0,0,'SAY_ANACHRONOS_SEAL_3'),
(-1000755,'The sands of time will halt, but only for a moment! I will now conjure the barrier.',0,0,0,0,'SAY_ANACHRONOS_SEAL_4'),
(-1000756,'FINISH THE SPELL, STAGHELM! I CANNOT HOLD THE GLYPHS OF WARDING IN PLACE MUCH LONGER! CALL FORTH THE ROOTS!', 0,0,0,0,'SAY_ANACHRONOS_SEAL_5'),
(-1000757,'Ancient ones guide my hand... Wake from your slumber! WAKE AND SEAL THIS CURSED PLACE!',0,0,0,0, 'SAY_FANDRAL_SEAL_6'),
(-1000758,'%s falls to one knee - exhausted.',0,2,0,0,'EMOTE_FANDRAL_EXHAUSTED'),
(-1000759,'It... It is over, Lord Staghelm. We are victorious. Albeit the cost for this victory was great.',0,0,0,1,'SAY_ANACHRONOS_EPILOGUE_1'),
(-1000760,'There is but one duty that remains...',0,0,0,1,'SAY_ANACHRONOS_EPILOGUE_2'),
(-1000761,'Before I leave this place, I make one final offering for you, Lord Staghelm. Should a time arise in which you must gain entry to this accursed fortress, use the Scepter of the Shifting Sands on the sacred gong. The magic holding the barrier together will dissipate and the horrors of Ahn\'Qiraj will be unleashed upon the world once more.',0,0,0,1,'SAY_ANACHRONOS_EPILOGUE_3'),
(-1000762,'%s hands the Scepter of the Shifting Sands to $N.',0,2,0,0,'EMOTE_ANACHRONOS_SCEPTER'),
(-1000763,'After the savagery that my people have witnessed and felt, you expect me to accept another burden, dragon? Surely you are mad.',0,0,0,1,'SAY_FANDRAL_EPILOGUE_4'),
(-1000764,'I want nothing to do with Silithus, the Qiraji and least of all, any damned dragons!',0,0,0,1,'SAY_FANDRAL_EPILOGUE_5'),
(-1000765,'%s hurls the Scepter of the Shifting Sands into the barrier, shattering it.',0,2,0,0,'EMOTE_FANDRAL_SHATTER'),
(-1000766,'Lord Staghelm, where are you going? You would shatter our bond for the sake of pride?',0,0,0,1,'SAY_ANACHRONOS_EPILOGUE_6'),
(-1000767,'My son\'s soul will find no comfort in this hollow victory, dragon. I will have him back. Though it takes a millennia, I WILL have my son back!',0,0,0,1,'SAY_FANDRAL_EPILOGUE_7'),
(-1000768,'%s shakes his head in disappointment.',0,2,0,25,'EMOTE_ANACHRONOS_DISPPOINTED'),
(-1000769,'%s kneels down to pickup the fragments of the shattered scepter.',0,2,0,0,'EMOTE_ANACHRONOS_PICKUP'),
(-1000770,'And now you know all that there is to know, mortal...',0,0,0,0,'SAY_ANACHRONOS_EPILOGUE_8'),

(-1000771,'Let\'s go $N!',0,0,0,0,'Feero Ironhand SAY_QUEST_START'),
(-1000772,'It looks like we\'re in trouble. Look lively, here they come!',0,0,0,0,'Feero Ironhand SAY_FIRST_AMBUSH_START'),
(-1000773,'Assassins from that cult you found... Let\'s get moving before someone else finds us out here.',0,0,0,0,'Feero Ironhand SAY_FIRST_AMBUSH_END'),
(-1000774,'Hold! I sense an evil presence... Undead!',0,0,0,0,'Feero Ironhand SAY_SECOND_AMBUSH_START'),
(-1000775,'A $C! Slaying him would please the master. Attack!',0,0,0,0,'Forsaken Scout SAY_SCOUT_SECOND_AMBUSH'),
(-1000776,'They\'re coming out of the woodwork today. Let\'s keep moving or we may find more things that want me dead.',0,0,0,0,'Feero Ironhand SAY_SECOND_AMBUSH_END'),
(-1000777,'These three again?',0,0,0,0,'Feero Ironhand SAY_FINAL_AMBUSH_START'),
(-1000778,'Not quite so sure of yourself without the Purifier, hm?',0,0,0,0,'Balizar the Umbrage SAY_BALIZAR_FINAL_AMBUSH'),
(-1000779,'I\'ll finish you off for good this time!',0,0,0,0,'Feero Ironhand SAY_FINAL_AMBUSH_ATTACK'),
(-1000780,'Well done! I should be fine on my own from here. Remember to talk to Delgren when you return to Maestra\'s Post in Ashenvale.',0,0,0,0,'Feero Ironhand SAY_QUEST_END'),

(-1000784,'Thanks $N. Now let\'s get out of here!',0,0,0,0,'melizza SAY_MELIZZA_START'),
(-1000785,'We made it! Thanks again! I\'m going to run ahead!',0,0,0,0,'melizza SAY_MELIZZA_FINISH'),
(-1000786,'Hey Hornizz! I\'m back! And there are some people behind me who helped me out of a jam.',0,0,0,1,'melizza SAY_MELIZZA_1'),
(-1000787,'We\'re going to have to scratch the Maraudines off our list. Too hard to work with...',0,0,0,1,'melizza SAY_MELIZZA_2'),
(-1000788,'Well, I\'m off to the Gelkis. They\'re not as dumb as the Maraudines, but they\'re more reasonable.',0,0,0,3,'melizza SAY_MELIZZA_3'),

(-1000789,'Well, now or never I suppose. Remember, once we get to the road safety, return to Terenthis to let him know we escaped.',0,0,0,0,'volcor SAY_START'),
(-1000790,'We made it, My friend. Remember to find Terenthis and let him know we\'re safe. Thank you again.',0,0,0,0,'volcor SAY_END'),
(-1000791,'Here they come.',0,0,0,0,'volcor SAY_FIRST_AMBUSH'),
(-1000792,'We can overcome these foul creatures.',0,0,0,0,'volcor SAY_AGGRO_1'),
(-1000793,'We shall earn our deaths at the very least!',0,0,0,0,'volcor SAY_AGGRO_2'),
(-1000794,'Don\'t give up! Fight, to the death!',0,0,0,0,'volcor SAY_AGGRO_3'),

(-1000795,'Ow! Ok, I\'ll get back to work, $N!',0,0,1,0,'Lazy Peon SAY_PEON_AWOKEN'),
(-1000796,'REUSE_ME',0,0,0,0,'REUSE_ME'),

(-1000797,'%s squawks and heads toward Veil Shalas. Hurry and follow!',0,2,0,0,'skywing SAY_SKYWING_START'),
(-1000798,'%s pauses briefly before the tree and then heads inside.',0,2,0,0,'skywing SAY_SKYWING_TREE_DOWN'),
(-1000799,'%s seems to be looking for something. He wants you to follow.',0,2,0,0,'skywing SAY_SKYWING_TREE_UP'),
(-1000800,'%s flies to the platform below! You\'d better jump if you want to keep up. Hurry!',0,2,0,0,'skywing SAY_SKYWING_JUMP'),
(-1000801,'%s bellows a loud squawk!',0,2,0,0,'skywing SAY_SKYWING_SUMMON'),
(-1000802,'Free at last from that horrible curse! Thank you! Please send word to Rilak the Redeemed that I am okay. My mission lies in Skettis. Terokk must be defeated!',0,0,0,0,'skywing SAY_SKYWING_END'),

(-1000803,'You do not fight alone, %n! Together, we will banish this spawn of hellfire!',0,1,0,0,'Oronok SAY_ORONOK_TOGETHER'),
(-1000804,'We will fight when you are ready.',0,0,0,0, 'Oronok SAY_ORONOK_READY'),
(-1000805,'We will set the elements free of your grasp by force!',0,1,0,0,'Oronok SAY_ORONOK_ELEMENTS'),
(-1000806,'What say the elements, Torlok? I only hear silence.',0,0,0,1,'Oronok SAY_ORONOK_EPILOGUE_1'),
(-1000807,'I hear what you hear, brother. Look behind you...',0,0,0,1,'Torlok SAY_TORLOK_EPILOGUE_2'),
(-1000808,'They are redeemed! Then we have won?',0,0,0,1,'Oronok SAY_ORONOK_EPILOGUE_3'),
(-1000809,'It is now as it should be, shaman. You have done well.',0,0,0,0,'Spirit of Earth SAY_EARTH_EPILOGUE_4'),
(-1000810,'Yes... Well enough for the elements that are here, but the cipher is known to another... The spirits of fire are in turmoil... If this force is not stopped, the world where these mortals came from will cease.',0,0,0,0,'Spirit of Fire SAY_FIRE_EPILOGUE_5'),
(-1000811,'Farewell, mortals... The earthmender knows what fire feels...',0,0,0,0, 'Spirit of Earth SAY_EARTH_EPILOGUE_6'),
(-1000812,'We leave, Torlok. I have only one request...',0,0,0,1,'Oronok SAY_ORONOK_EPILOGUE_7'),
(-1000813,'The Torn-heart men give their weapons to Earthmender Torlok.',0,2,0,0,'Torlok EMOTE_GIVE_WEAPONS'),
(-1000814,'Give these to the heroes that made this possible.',0,0,0,1,'Oronok SAY_ORONOK_EPILOGUE_8'),

(-1000815,'Be healed!',0,1,0,0,'Eris Havenfire SAY_PHASE_HEAL'),
(-1000816,'We are saved! The peasants have escaped the Scourge!',0,1,0,0,'Eris Havenfire SAY_EVENT_END'),
(-1000817,'I have failed once more...',0,1,0,0,'Eris Havenfire SAY_EVENT_FAIL_1'),
(-1000818,'I now return to whence I came, only to find myself here once more to relive the same epic tragedy.',0,0,0,0,'Eris Havenfire SAY_EVENT_FAIL_2'),
(-1000819,'The Scourge are upon us! Run! Run for your lives!',0,1,0,0,'Peasant SAY_PEASANT_APPEAR_1'),
(-1000820,'Please help us! The Prince has gone mad!',0,1,0,0,'Peasant SAY_PEASANT_APPEAR_2'),
(-1000821,'Seek sanctuary in Hearthglen! It is our only hope!',0,1,0,0,'Peasant SAY_PEASANT_APPEAR_3'),

(-1000822,'The signal has been sent. He should be arriving shortly.',0,0,0,1,'squire rowe SAY_SIGNAL_SENT'),
(-1000823,'Yawww!',0,0,0,35,'reginald windsor SAY_DISMOUNT'),
(-1000824,'I knew you would come, $N. It is good to see you again, friend.',0,0,0,1,'reginald windsor SAY_WELCOME'),

(-1000825,'On guard, friend. The lady dragon will not give in without a fight.',0,0,0,1,'reginald windsor SAY_QUEST_ACCEPT'),
(-1000826,'As was fated a lifetime ago in Karazhan, monster - I come - and with me I bring justice.',0,6,0,22,'reginald windsor SAY_GET_READY'),
(-1000827,'Seize him! Seize the worthless criminal and his allies!',0,6,0,0,'prestor SAY_GONNA_DIE'),
(-1000828,'Reginald, you know that I cannot let you pass.',0,0,0,1,'jonathan SAY_DIALOG_1'),
(-1000829,'You must do what you think is right, Marcus. We served together under Turalyon. He made us both the men that we are today. Did he err with me? Do you truly believe my intent is to cause harm to our alliance? Would I shame our heroes?',0,0,0,1,'reginald windsor SAY_DIALOG_2'),
(-1000830,'Holding me here is not the right decision, Marcus.',0,0,0,1,'reginald windsor SAY_DIALOG_3'),
(-1000831,'%s appears lost in contemplation.',0,2,0,0,'jonathan EMOTE_CONTEMPLATION'),
(-1000832,'I am ashamed, old friend. I know not what I do anymore. It is not you that would dare bring shame to the heroes of legend - it is I. It is I and the rest of these corrupt politicians. They fill our lives with empty promises, unending lies.',0,0,0,1,'jonathan SAY_DIALOG_4'),
(-1000833,'We shame our ancestors. We shame those lost to us... forgive me, Reginald.',0,0,0,1,'jonathan SAY_DIALOG_5'),
(-1000834,'Dear friend, you honor them with your vigilant watch. You are steadfast in your allegiance. I do not doubt for a moment that you would not give as great a sacrifice for your people as any of the heroes you stand under.',0,0,0,1,'reginald windsor SAY_DIALOG_6'),
(-1000835,'Now, it is time to bring her reign to an end, Marcus. Stand down, friend.',0,0,0,1,'reginald windsor SAY_DIALOG_7'),
(-1000836,'Stand down! Can you not see that heroes walk among us?',0,0,0,5,'jonathan SAY_DIALOG_8'),
(-1000837,'Move aside! Let them pass!',0,0,0,5,'jonathan SAY_DIALOG_9'),
(-1000838,'Reginald Windsor is not to be harmed! He shall pass through untouched!',0,1,0,22,'jonathan SAY_DIALOG_10'),
(-1000839,'Go, Reginald. May the light guide your hand.',0,0,0,1,'jonathan SAY_DIALOG_11'),
(-1000840,'Thank you, old friend. You have done the right thing.',0,0,0,1,'reginald windsor SAY_DIALOG_12'),
(-1000841,'Follow me, friends. To Stormwind Keep!',0,0,0,0,'reginald windsor SAY_DIALOG_13'),
(-1000842,'Light be with you, sir.',0,0,0,66,'guard SAY_1'),
(-1000843,'We are but dirt beneath your feet, sir.',0,0,0,66,'guard SAY_2'),
(-1000844,'...nerves of thorium.',0,0,0,66,'guard SAY_3'),
(-1000845,'Make way!',0,0,0,66,'guard SAY_4'),
(-1000846,'A living legend...',0,0,0,66,'guard SAY_5'),
(-1000847,'A moment I shall remember for always.',0,0,0,66,'guard SAY_6'),
(-1000848,'You are an inspiration to us all, sir.',0,0,0,66,'guard SAY_7'),
(-1000849,'Be brave, friends. The reptile will thrash wildly. It is an act of desperation. When you are ready, give me the word.',0,0,0,25,'reginald windsor SAY_BEFORE_KEEP'),
(-1000850,'Onward!',0,0,0,5,'reginald windsor SAY_GO_TO_KEEP'),
(-1000851,'Majesty, run while you still can. She is not what you think her to be...',0,0,0,1,'reginald windsor SAY_IN_KEEP_1'),
(-1000852,'To the safe hall, your majesty.',0,0,0,1,'bolvar SAY_IN_KEEP_2'),
(-1000853,'The masquerade is over, Lady Prestor. Or should I call you by your true name... Onyxia...',0,0,0,25,'reginald windsor SAY_IN_KEEP_3'),
(-1000854,'%s laughs.',0,2,0,11,'prestor EMOTE_IN_KEEP_LAUGH'),
(-1000855,'You will be incarcerated and tried for treason, Windsor. I shall watch with glee as they hand down a guilty verdict and sentence you to death by hanging...',0,0,0,1,'prestor SAY_IN_KEEP_4'),
(-1000856,'And as your limp body dangles from the rafters, I shall take pleasure in knowing that a mad man has been put to death. After all, what proof do you have? Did you expect to come in here and point your fingers at royalty and leave unscathed?',0,0,0,6,'prestor SAY_IN_KEEP_5'),
(-1000857,'You will not escape your fate, Onyxia. It has been prophesied - a vision resonating from the great halls of Karazhan. It ends now...',0,0,0,1,'reginald windsor SAY_IN_KEEP_6'),
(-1000858,'%s reaches into his pack and pulls out the encoded tablets...',0,2,0,0,'reginald windsor EMOTE_IN_KEEP_REACH'),
(-1000859,'The Dark Irons thought these tablets to be encoded. This is not any form of coding, it is the tongue of ancient dragon.',0,0,0,1,'reginald windsor SAY_IN_KEEP_7'),
(-1000860,'Listen, dragon. Let the truth resonate throughout these halls.',0,0,0,1,'reginald windsor SAY_IN_KEEP_8'),
(-1000861,'%s reads from the tablets. Unknown, unheard sounds flow through your consciousness',0,2,0,0,'reginald windsor EMOTE_IN_KEEP_READ'),
(-1000862,'%s gasps.',0,2,0,0,'bolvar EMOTE_IN_KEEP_GASP'),
(-1000863,'Curious... Windsor, in this vision, did you survive? I only ask because one thing that I can and will assure is your death. Here and now.',0,0,0,1,'onyxia SAY_IN_KEEP_9'),
(-1000864,'Dragon filth! Guards! Guards! Seize this monster!',0,1,0,22,'bolvar SAY_IN_KEEP_1'),
(-1000865,'Yesss... Guards, come to your lord\'s aid!',0,0,0,1,'onyxia SAY_IN_KEEP_10'),
(-1000866,'DO NOT LET HER ESCAPE!',0,0,0,1,'reginald windsor SAY_IN_KEEP_11'),
(-1000867,'Was this fabled, Windsor? If it was death that you came for then the prophecy has been fulfilled. May your consciousness rot in the Twisting Nether. Finish the rest of these meddlesome insects, children. Bolvar, you have been a pleasureable puppet.',0,0,0,0,'onyxia SAY_IN_KEEP_12'),
(-1000868,'You have failed him, mortalsss... Farewell!',0,1,0,0,'onyxia SAY_IN_KEEP_12'),
(-1000869,'Reginald... I... I am sorry.',0,0,0,0,'bolvar SAY_IN_KEEP_13'),
(-1000870,'Bol... Bolvar... the medallion... use...',0,0,0,0,'reginald windsor SAY_IN_KEEP_14'),
(-1000871,'%s dies.',0,2,0,0,'reginald windsor EMOTE_IN_KEEP_DIE'),
(-1000872,'%s hisses',0,2,0,0,'reginald windsor EMOTE_GUARD_TRANSFORM'),

(-1000873,'I know the way, insect. There is no need to prod me as if I were cattle.',0,0,0,1,'grark SAY_START'),
(-1000874,'Surely you do not think that you will get away with this incursion. They will come for me and you shall pay for your insolence.',0,0,0,1,'grark SAY_PAY'),
(-1000875,'RUN THEM THROUGH BROTHERS!',0,0,0,5,'grark SAY_FIRST_AMBUSH_START'),
(-1000876,'I doubt you will be so lucky the next time you encounter my brethren.',0,0,0,1,'grark SAY_FIRST_AMBUSH_END'),
(-1000877,'They come for you, fool!',0,0,0,5,'grark SAY_SEC_AMBUSH_START'),
(-1000878,'What do you think you accomplish from this, fool? Even now, the Blackrock armies make preparations to destroy your world.',0,0,0,1,'grark SAY_SEC_AMBUSH_END'),
(-1000879,'On darkest wing they fly. Prepare to meet your end!',0,0,0,5,'grark SAY_THIRD_AMBUSH_START'),
(-1000880,'The worst is yet to come!',0,0,0,1,'grark SAY_THIRD_AMBUSH_END'),
(-1000881,'%s laughs.',0,2,0,11,'grark EMOTE_LAUGH'),
(-1000882,'Time to make your final stand, Insect.',0,0,0,0,'grark SAY_LAST_STAND'),
(-1000883,'Kneel, Grark',0,0,0,1,'lexlort SAY_LEXLORT_1'),
(-1000884,'Grark Lorkrub, you have been charged and found guilty of treason against Horde. How you plead is unimportant. High Executioner Nuzrak, step forward.',0,0,0,1,'lexlort SAY_LEXLORT_2'),
(-1000885,'%s raises his massive axe over Grark.',0,2,0,27,'nuzark EMOTE_RAISE_AXE'),
(-1000886,'%s raises his hand and then lowers it.',0,2,0,0,'lexlort EMOTE_LOWER_HAND'),
(-1000887,'End him...',0,0,0,0,'lexlort SAY_LEXLORT_3'),
(-1000888,'You, soldier, report back to Kargath at once!',0,0,0,1,'lexlort SAY_LEXLORT_4'),
(-1000889,'%s submits.',0,2,0,0,'grark EMOTE_SUBMIT'),
(-1000890,'You have come to play? Then let us play!',0,0,0,0,'grark SAY_AGGRO'),

(-1000891,'The beast returns from whence it came. The wrath of Neptulon has subsided.',0,3,0,0,'Maws EMOTE_MAWS_KILL'),

(-1000892,'Beware, $N! Look to the west!',0,0,0,0,'regthar SAY_START_REGTHAR'),
(-1000893,'A defender has fallen!',0,0,0,0,'regthar SAY_DEFENDER'),
(-1000894,'The Kolkar invaders are retreating!',0,1,0,0,'regthar YELL_RETREAT'),
(-1000895,'REUSE_ME',0,0,0,0,'REUSE_ME'),
(-1000896,'REUSE_ME',0,0,0,0,'REUSE_ME'),
(-1000897,'REUSE_ME',0,0,0,0,'REUSE_ME'),
(-1000898,'REUSE_ME',0,0,0,0,'REUSE_ME'),
(-1000899,'REUSE_ME',0,0,0,0,'REUSE_ME'),
(-1000900,'REUSE_ME',0,0,0,0,'REUSE_ME'),

(-1000901,'REUSE_ME',0,0,0,0,'REUSE_ME'),
(-1000902,'REUSE_ME',0,0,0,0,'REUSE_ME'),
(-1000903,'REUSE_ME',0,0,0,0,'REUSE_ME'),
(-1000904,'REUSE_ME',0,0,0,0,'REUSE_ME'),

(-1000905,'Ok, let\'s go!!',0,0,0,1,'therylune SAY_THERYLUNE_START'),
(-1000906,'I can make it the rest of the way. $N. THANKS!',0,0,0,1,'therylune SAY_THERYLUNE_START'),

(-1000948,'Well then, let\'s get this started. The longer we\'re here, the more damage the undead could be doing back in Hilsbrad.',0,0,0,0,'kinelory SAY_START'),
(-1000949,'All right, this is where we really have to be on our paws. Be ready!',0,0,0,0,'kinelory SAY_REACH_BOTTOM'),
(-1000950,'Attack me if you will, but you won\'t stop me from getting back to Quae.',0,0,0,0,'kinelory SAY_AGGRO_KINELORY'),
(-1000951,'You have my word that I shall find a use for your body after I\'ve killed you, Kinelory.',0,0,0,0,'jorell SAY_AGGRO_JORELL'),
(-1000952,'Watch my rear! I\'ll see what I can find in all this junk...',0,0,0,0,'kinelory SAY_WATCH_BACK'),
(-1000953,'%s begins rummaging through the apothecary\'s belongings.',0,2,0,0,'kinelory EMOTE_BELONGINGS'),
(-1000954,'I bet Quae\'ll think this is important. She\'s pretty knowledgeable about these things--no expert, but knowledgeable.',0,0,0,0,'kinelory SAY_DATA_FOUND'),
(-1000955,'Okay, let\'s get out of here quick quick! Try and keep up. I\'m going to make a break for it.',0,0,0,0,'kinelory SAY_ESCAPE'),
(-1000956,'We made it! Quae, we made it!',0,0,0,0,'kinelory SAY_FINISH'),
(-1000957,'%s hands her pack to Quae.',0,2,0,0,'kinelory EMOTE_HAND_PACK'),

(-1000958,'Ok, let\'s get started.',0,0,0,0,'stinky ignatz SAY_STINKY_BEGIN'),
(-1000959,'Now let\'s look for the herb.',0,0,0,0,'stinky ignatz SAY_STINKY_FIRST_STOP'),
(-1000960,'Help! The beast is on me!',0,0,0,0,'stinky ignatz SAY_AGGRO_1'),
(-1000961,'Help! I\'m under attack!',0,0,0,0,'stinky ignatz SAY_AGGRO_2'),
(-1000962,'I can make it from here. Thanks, $N! And talk to my employer about a reward!',0,0,0,0,'stinky ignatz SAY_STINKY_END'),

(-1000977,'Oh, it\'s on now! But you thought I\'d be alone too, huh?!',0,0,0,0,'tapoke slim jahn SAY_AGGRO'),
(-1000978,'Okay, okay! No need to get all violent. I\'ll talk. I\'ll talk!',0,0,0,20,'tapoke slim jahn SAY_DEFEAT'),
(-1000979,'Whoa! This is way more than what I bargained for, you\'re on your own, Slim!',0,0,0,0,'slim\'s friend SAY_FRIEND_DEFEAT'),
(-1000980,'I have a few notes from the job back at my place. I\'ll get them and then meet you back in the inn.',0,0,0,1,'tapoke slim jahn SAY_NOTES'),

(-1000993,'It\'s on! $N, meet my fists. Fists, say hello to $N.',0,0,0,0,'dorius stonetender SAY_AGGRO_1'),
(-1000994,'I\'m about to open a can on this $N.',0,0,0,0,'dorius stonetender SAY_AGGRO_2'),

(-1001072,'Something is wrong with the Highlord. Do something!',0,0,0,1,'scarlet cavalier SAY_CAVALIER_WORRY_1'),
(-1001073,'Hey, what is going on over there? Sir, are you alright?',0,0,0,1,'scarlet cavalier SAY_CAVALIER_WORRY_2'),
(-1001074,'What the....',0,0,0,1,'scarlet cavalier SAY_CAVALIER_WORRY_3'),
(-1001075,'Sir?',0,0,0,1,'scarlet cavalier SAY_CAVALIER_WORRY_4'),
(-1001076,'NOOOOOOOOOOOOO!',0,1,0,15,'taelan fordring SAY_SCARLET_COMPLETE_1'),
(-1001077,'I will lead us through Hearthglen to the forest\'s edge. From there, you will take me to my father.',0,0,0,1,'taelan fordring SAY_SCARLET_COMPLETE_2'),
(-1001078,'Remove your disguise, lest you feel the bite of my blade when the fury has taken control.',0,0,0,1,'taelan fordring SAY_ESCORT_START'),
(-1001079,'Halt.',0,0,0,0,'taelan fordring SAY_TAELAN_MOUNT'),
(-1001080,'%s calls for his mount.',0,2,0,22,'taelan fordring EMOTE_TAELAN_MOUNT'),
(-1001081,'It\'s not much further. The main road is just up ahead.',0,0,0,1,'taelan fordring SAY_REACH_TOWER'),
(-1001082,'You will not make it to the forest\'s edge, Fordring.',0,0,0,1,'isillien SAY_ISILLIEN_1'),
(-1001083,'Isillien!',0,1,0,25,'taelan fordring SAY_ISILLIEN_2'),
(-1001084,'This is not your fight, stranger. Protect yourself from the attacks of the Crimson Elite. I shall battle the Grand Inquisitor.',0,0,0,1,'taelan fordring SAY_ISILLIEN_3'),
(-1001085,'You disappoint me, Taelan. I had plans for you... grand plans. Alas, it was only a matter of time before your filthy bloodline would catch up with you.',0,0,0,1,'isillien SAY_ISILLIEN_4'),
(-1001086,'It is as they say: Like father, like son. You are as weak of will as Tirion... perhaps more so. I can only hope my assassins finally succeeded in ending his pitiful life.',0,0,0,1,'isillien SAY_ISILLIEN_5'),
(-1001087,'The Grand Crusader has charged me with destroying you and your newfound friends, Taelan, but know this: I do this for pleasure, not of obligation or duty.',0,0,0,1,'isillien SAY_ISILLIEN_6'),
(-1001088,'%s calls for his guardsman.',0,2,0,22,'isillien EMOTE_ISILLIEN_ATTACK'),
(-1001089,'The end is now, Fordring.',0,0,0,25,'isillien SAY_ISILLIEN_ATTACK'),
(-1001090,'Enough!',0,0,0,0,'isillien SAY_KILL_TAELAN_1'),
(-1001091,'%s laughs.',0,2,0,11,'isillien EMOTE_ISILLIEN_LAUGH'),
(-1001092,'Did you really believe that you could defeat me? Your friends are soon to join you, Taelan.',0,0,0,0,'isillien SAY_KILL_TAELAN_2'),
(-1001093,'%s turns his attention towards you.',0,2,0,0,'isillien EMOTE_ATTACK_PLAYER'),
(-1001094,'What have you done, Isillien? You once fought with honor, for the good of our people... and now... you have murdered my boy...',0,0,0,0,'tirion fordring SAY_TIRION_1'),
(-1001095,'Tragic. The elder Fordring lives on... You are too late, old man. Retreat back to your cave, hermit, unless you wish to join your son in the Twisting Nether.',0,0,0,0,'isillien SAY_TIRION_2'),
(-1001096,'May your soul burn in anguish, Isillien! Light give me strength to battle this fiend.',0,0,0,15,'tirion fordring SAY_TIRION_3'),
(-1001097,'Face me, coward. Face the faith and strength that you once embodied.',0,0,0,25,'tirion fordring SAY_TIRION_4'),
(-1001098,'Then come, hermit!',0,0,0,0,'isillien SAY_TIRION_5'),
(-1001099,'A thousand more like him exist. Ten thousand. Should one fall, another will rise to take the seat of power.',0,0,0,0,'tirion fordring SAY_EPILOG_1'),
(-1001100,'%s falls to one knee.',0,2,0,16,'tirion fordring EMOTE_FALL_KNEE'),
(-1001101,'Look what they did to my boy.',0,0,0,5,'tirion fordring SAY_EPILOG_2'),
(-1001102,'%s holds the limp body of Taelan Fordring and softly sobs.',0,2,0,0,'tirion fordring EMOTE_HOLD_TAELAN'),
(-1001103,'Too long have I sat idle, gripped in this haze... this malaise, lamenting what could have been... what should have been.',0,0,0,0,'tirion fordring SAY_EPILOG_3'),
(-1001104,'Your death will not have been in vain, Taelan. A new Order is born on this day... an Order which will dedicate itself to extinguishing the evil that plagues this world. An evil that cannot hide behind politics and pleasantries.',0,0,0,0,'tirion fordring SAY_EPILOG_4'),
(-1001105,'This I promise... This I vow...',0,0,0,15,'tirion fordring SAY_EPILOG_5'),

(-1001106,'Don\'t forget to get my bell out of the chest here. And remember, if do happen to wander off, just ring it and I\'ll find you again.',0,0,0,1,'shay leafrunner SAY_ESCORT_START'),
(-1001107,'Are we taking the scenic route?',0,0,0,0,'shay leafrunner SAY_WANDER_1'),
(-1001108,'Oh, what a beautiful flower over there...',0,0,0,0,'shay leafrunner SAY_WANDER_2'),
(-1001109,'Are you sure this is the right way? Maybe we should go this way instead...',0,0,0,0,'shay leafrunner SAY_WANDER_3'),
(-1001110,'Hmmm, I wonder what\'s over this way?',0,0,0,0,'shay leafrunner SAY_WANDER_4'),
(-1001111,'This is quite an adventure!',0,0,0,0,'shay leafrunner SAY_WANDER_DONE_1'),
(-1001112,'Oh, I wandered off again. I\'m sorry.',0,0,0,0,'shay leafrunner SAY_WANDER_DONE_2'),
(-1001113,'The bell again, such a sweet sound.',0,0,0,0,'shay leafrunner SAY_WANDER_DONE_3'),
(-1001114,'%s begins to wander off.',0,2,0,0,'shay leafrunner EMOTE_WANDER'),
(-1001115,'Oh, here you are, Rockbiter! I\'m sorry, I know I\'m not supposed to wander off.',0,0,0,1,'shay leafrunner SAY_EVENT_COMPLETE_1'),
(-1001116,'I\'m so glad yer back Shay. Please, don\'t ever run off like that again! What would I tell yer parents if I lost ya?',0,0,0,1,'rockbiter SAY_EVENT_COMPLETE_2'),

(-1001141,'Nope, not here...',0,0,0,0,'stinky ignatz SAY_SECOND_STOP'),
(-1001142,'There must be one around here somewhere...',0,0,0,0,'stinky ignatz SAY_THIRD_STOP_1'),
(-1001143,'Ah, there\'s one!',0,0,0,0,'stinky ignatz SAY_THIRD_STOP_2'),
(-1001144,'Come, $N! Let\'s go over there and collect it!',0,0,0,0,'stinky ignatz SAY_THIRD_STOP_3'),
(-1001145,'Ok, let\'s get out of here!',0,0,0,0,'stinky ignatz SAY_PLANT_GATHERED'),
(-1001146,'I\'m glad you\'re here! Because I need your help!!',0,0,0,0,'stinky ignatz SAY_AGGRO_3'),
(-1001147,'Look out! The $N attacks!',0,0,0,0,'stinky ignatz SAY_AGGRO_4'),

(-1001148,'I am ready, $N. Let\'s find my equipment and get out of here. I think I know where it is.',0,0,0,1,'captured arko\'narin SAY_ESCORT_START'),
(-1001149,'Oh my! Look at this... all these candles. I\'m sure they\'re used for some terrible ritual or dark summoning. We best make haste.',0,0,0,18,'captured arko\'narin SAY_FIRST_STOP'),
(-1001150,'There! Over there!',0,0,0,25,'captured arko\'narin SAY_SECOND_STOP'),
(-1001151,'You will not stop me from escaping here, $N!',0,0,0,0,'captured arko\'narin SAY_AGGRO'),
(-1001152,'All I need now is a golden lasso.',0,0,0,0,'captured arko\'narin SAY_EQUIPMENT'),
(-1001153,'DIE DEMON DOGS!',0,0,0,0,'captured arko\'narin SAY_ESCAPE'),
(-1001154,'Ah! Fresh air at last! I never thought I\'d see the day.',0,0,0,4,'captured arko\'narin SAY_FRESH_AIR'),
(-1001155,'BETRAYER!',0,1,0,0,'spirit of trey lightforge SAY_BETRAYER'),
(-1001156,'What was that?! Trey? TREY?',0,0,0,22,'captured arko\'narin SAY_TREY'),
(-1001157,'You kept me in the cell for too long, monster!',0,0,0,0,'captured arko\'narin SAY_ATTACK_TREY'),
(-1001158,'No! My friend... what\'s happened? This is all my fault...',0,0,0,18,'captured arko\'narin SAY_ESCORT_COMPLETE'),

(-1001159,'Please, help me to get through this cursed forest, $r.',0,0,0,0,'arei SAY_ESCORT_START'),
(-1001160,'This creature suffers from the effect of the fel... We must end its misery.',0,0,0,0,'arei SAY_ATTACK_IRONTREE'),
(-1001161,'The corruption of the fel has not left any of the creatures of Felwood untouched, $N. Please, be on your guard.',0,0,0,0,'arei SAY_ATTACK_TOXIC_HORROR'),
(-1001162,'I sense the taint of corruption upon this $N. Help me detroy it.',0,0,0,0,'arei SAY_EXIT_WOODS'),
(-1001163,'That I must fight against my own kind deeply saddens me.',0,0,0,0,'arei SAY_CLEAR_PATH'),
(-1001164,'I can sense it now, $N. Ashenvale lies down this path.',0,0,0,0,'arei SAY_ASHENVALE'),
(-1001165,'I feel... something strange...',0,0,0,0,'arei SAY_TRANSFORM'),
(-1001166,'$N my form has now changed! The true strength of my spirit is returning to me now... The cursed grasp of the forest is leaving me.',0,0,0,0,'arei SAY_LIFT_CURSE'),
(-1001167,'Thank you, $N. Now my spirit will finally be at peace.',0,0,0,0,'arei SAY_ESCORT_COMPLETE'),

(-1001168,'REUSE ME',0,0,0,0,'REUSE_ME'),
(-1001169,'REUSE ME',0,0,0,0,'REUSE_ME'),

(-1001170,'REUSE ME',0,0,0,0,'REUSE_ME'),
(-1001171,'REUSE ME',0,0,0,0,'REUSE_ME'),

(-1001172,'REUSE ME',0,0,0,0,'REUSE_ME'),

(-1001173,'REUSE ME',0,0,0,0,'REUSE_ME'),
(-1001174,'REUSE ME',0,0,0,0,'REUSE_ME'),
(-1001175,'REUSE ME',0,0,0,0,'REUSE_ME'),
(-1001176,'REUSE ME',0,0,0,0,'REUSE_ME'),
(-1001177,'REUSE ME',0,0,0,0,'REUSE_ME'),
(-1001178,'REUSE ME',0,0,0,0,'REUSE_ME'),
(-1001179,'REUSE ME',0,0,0,0,'REUSE_ME'),
(-1001180,'REUSE ME',0,0,0,0,'REUSE_ME'),
(-1001181,'REUSE ME',0,0,0,0,'REUSE_ME'),
(-1001182,'REUSE ME',0,0,0,0,'REUSE_ME'),
(-1001183,'REUSE ME',0,0,0,0,'REUSE_ME'),

(-1001184,'REUSE ME',0,0,0,0,'REUSE_ME'),
(-1001185,'REUSE ME',0,0,0,0,'REUSE_ME'),
(-1001186,'REUSE ME',0,0,0,0,'REUSE_ME'),
(-1001187,'REUSE ME',0,0,0,0,'REUSE_ME'),
(-1001188,'REUSE ME',0,0,0,0,'REUSE_ME'),
(-1001189,'REUSE ME',0,0,0,0,'REUSE_ME'),
(-1001190,'REUSE ME',0,0,0,0,'REUSE_ME'),

(-1001191,'Hey, you call yourself a body guard? Get to work and protect us...',0,0,0,0,'SAY_CORK_AMBUSH1'),
(-1001192,'Mister body guard, are you going to earn your money or what?',0,0,0,0,'SAY_CORK_AMBUSH2'),
(-1001193,'You\'re fired! <Cough...Cork clears throat.> I mean, help!',0,0,0,0,'SAY_CORK_AMBUSH3'),
(-1001194,'Blast those stupid centaurs! Sigh - well, it seems you kept your bargain. Up the road you shall find Smeed Scrabblescrew, he has your money.',0,0,0,1,'SAY_CORK_END'),
(-1001195,'Eeck! Demons hungry for the kodos!',0,0,0,0,'SAY_RIGGER_AMBUSH1'),
(-1001196,'What am I paying you for? The kodos are nearly dead!',0,0,0,0,'SAY_RIGGER_AMBUSH2'),
(-1001197,'Wow! We did it... not sure why we thought we needed the likes of you. Nevertheless, speak with Smeed Srablescrew; he will give you your earnings!',0,0,0,0,'SAY_RIGGER_END'),
-- REUSE_ME -1001198 to -1001225
(-1001226,'%s is drawn to the ghost magnet...',0,2,0,0,'Magrami Spectre on spawn 1'),
(-1001227,'%s is angered!',0,2,0,0,'Magrami Spectre on spawn 2'),
-- used in TBC
(-1001250,'%s is silenced by the venomous sting.',0,2,0,0,'npc_simone EMOTE_SILENCE'),
(-1001251,'%s is stricken by a virulent poison.',0,2,0,0,'npc_artorius EMOTE_POISON'),
(-1001252,'%s is immobilized.',0,2,0,0,'npc_solenor EMOTE_IMMOBILIZED'),
(-1001253,'You dare interfere with this being''s testing? The battle must be fought alone! You shall all pay for this interference!',0,0,0,0,'the_cleaner SAY_CLEANER_AGGRO'),
-- used in TBC core
(-1001274,'Now you''re gonna get it good, $n!', '0', '0', '0', '0','dashel stonefist SAY_STONEFIST_1'),
(-1001275,'Okay, okay! Enough fighting. No one else needs to get hurt.', '0', '0', '0', '0','dashel stonefist SAY_STONEFIST_2'),
(-1001276,'It''s okay, boys. Back off. You''ve done enough. I''ll meet up with you later.', '0', '0', '0', '0','dashel stonefist SAY_STONEFIST_3');

-- -1 010 000 Classic texts

-- -1 015 000 TBC texts

-- -1 020 000 WOTLK texts

-- -1 033 000 SHADOWFANG KEEP
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1033000,'Follow me and I\'ll open the courtyard door for you.',0,0,7,1,'prisoner ashcrombe SAY_FREE_AS'),
(-1033001,'I have just the spell to get this door open. Too bad the cell doors weren\'t locked so haphazardly.',0,0,7,1,'prisoner ashcrombe SAY_OPEN_DOOR_AS'),
(-1033002,'There it is! Wide open. Good luck to you conquering what lies beyond. I must report back to the Kirin Tor at once!',0,0,7,1,'prisoner ashcrombe SAY_POST_DOOR_AS'),

(-1033003,'Free from this wretched cell at last! Let me show you to the courtyard....',0,0,1,1,'prisoner adamant SAY_FREE_AD'),
(-1033004,'You are indeed courageous for wanting to brave the horrors that lie beyond this door.',0,0,1,1,'prisoner adamant SAY_OPEN_DOOR_AD'),
(-1033005,'There we go!',0,0,1,1,'prisoner adamant SAY_POST1_DOOR_AD'),
(-1033006,'Good luck with Arugal. I must hurry back to Hadrec now.',0,0,1,1,'prisoner adamant SAY_POST2_DOOR_AD'),

(-1033007,'About time someone killed the wretch.',0,0,1,1,'prisoner adamant SAY_BOSS_DIE_AD'),
(-1033008,'For once I agree with you... scum.',0,0,7,1,'prisoner ashcrombe SAY_BOSS_DIE_AS'),

(-1033009,'I have changed my mind loyal servants, you do not need to bring the prisoner all the way to my study, I will deal with him here and now.',0,0,0,1,'arugal SAY_INTRO_1'),
(-1033010,'Vincent!  You and your pathetic ilk will find no more success in routing my sons and I than those beggardly remnants of the Kirin Tor.',0,0,0,0,'arugal SAY_INTRO_2'),
(-1033011,'If you will not serve my Master with your sword and knowledge of his enemies...',0,0,0,1,'arugal SAY_INTRO_3'),
(-1033012,'Your moldering remains will serve ME as a testament to what happens when one is foolish enough to trespass in my domain!\n',0,0,0,0,'arugal SAY_INTRO_4'),

(-1033013,'Who dares interfere with the Sons of Arugal?',5791,1,0,0,'boss_arugal YELL_FENRUS'),
(-1033014,'%s vanishes.',0,2,0,0,'prisoner ashcrombe EMOTE_VANISH_AS'),
(-1033015,'%s fumbles with the rusty lock on the courtyard door.',0,2,0,0,'prisoner adamant EMOTE_UNLOCK_DOOR_AD'),
(-1033016,'Arrrgh!',0,0,0,0,'deathstalker vincent SAY_VINCENT_DIE'),
(-1033017,'You, too, shall serve!',5793,1,0,0,'boss_arugal YELL_AGGRO'),
(-1033018,'Another Falls!',5795,1,0,0,'boss_arugal YELL_KILLED_PLAYER'),
(-1033019,'Release your rage!',5797,1,0,0,'boss_arugal YELL_COMBAT'),
(-1033020,'I can\'t believe it! You\'ve destroyed my pack... Now face my wrath!',0,1,0,0,'wolf master nandos YELL_PACK_DEAD');

-- -1 034 000 STOCKADES

-- -1 036 000 DEADMINES
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1036000,'You there! Check out that noise.',5775,6,7,0,'smite INST_SAY_ALARM1'),
(-1036001,'We\'re under attack!  A vast, ye swabs! Repel the invaders!',5777,6,7,0,'smite INST_SAY_ALARM2'),
(-1036002,'You land lubbers are tougher than I thought! I\'ll have to improvise!',5778,0,0,21,'smite SAY_PHASE_2'),
(-1036003,'D\'ah! Now you\'re making me angry!',5779,0,0,15,'smite SAY_PHASE_3');

-- -1 043 000 WAILING CAVERNS
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1043000,'At last! Naralex can be awakened! Come aid me, brave adventurers!',0,6,0,0,'Disciple of Naralex - SAY_INTRO'),
(-1043001,'I must make the necessary preparations before the awakening ritual can begin. You must protect me!',0,0,0,0,'SAY_PREPARE'),
(-1043002,'These caverns were once a temple of promise for regrowth in the Barrens. Now, they are the halls of nightmares.',0,0,0,0,'Disciple of Naralex - SAY_FIRST_CORNER'),
(-1043003,'Come. We must continue. There is much to be done before we can pull Naralex from his nightmare.',0,0,0,0,'Disciple of Naralex - SAY_CONTINUE'),
(-1043004,'Within this circle of fire I must cast the spell to banish the spirits of the slain Fanglords.',0,0,0,0,'Disciple of Naralex - SAY_CIRCLE_BANISH'),
(-1043005,'The caverns have been purified. To Naralex\'s chamber we go!',0,0,0,0,'Disciple of Naralex - SAY_PURIFIED'),
(-1043006,'Beyond this corridor, Naralex lies in fitful sleep. Let us go awaken him before it is too late.',0,0,0,0,'Disciple of Naralex - SAY_NARALEX_CHAMBER'),
(-1043007,'Protect me brave souls as I delve into the Emerald Dream to rescue Naralex and put an end to this corruption!',0,1,0,0,'Disciple of Naralex - SAY_BEGIN_RITUAL'),
(-1043008,'%s begins to perform the awakening ritual on Naralex.',0,2,0,0,'Disciple of Naralex - EMOTE_RITUAL_BEGIN'),
(-1043009,'%s tosses fitfully in troubled sleep.',0,2,0,0,'Naralex - EMOTE_NARALEX_AWAKE'),
(-1043010,'%s writhes in agony. The Disciple seems to be breaking through.',0,2,0,0,'Naralex - EMOTE_BREAK_THROUGH'),
(-1043011,'%s dreams up a horrendous vision. Something stirs beneath the murky waters.',0,2,0,0,'Naralex - EMOTE_VISION'),
(-1043012,'This $N is a minion from Naralex\'s nightmare no doubt!.',0,0,0,0,'Disciple of Naralex - SAY_MUTANUS'),
(-1043013,'I AM AWAKE, AT LAST!',5789,1,0,0,'Naralex - SAY_NARALEX_AWAKE'),
(-1043014,'At last! Naralex awakes from the nightmare.',0,0,0,0,'Disciple of Naralex - SAY_AWAKE'),
(-1043015,'Ah, to be pulled from the dreaded nightmare! I thank you, my loyal Disciple, along with your brave companions.',0,0,0,0,'Naralex - SAY_NARALEX_THANKYOU'),
(-1043016,'We must go and gather with the other Disciplies. There is much work to be done before I can make another attempt to restore the Barrens. Farewell, brave souls!',0,0,0,0,'Naralex - SAY_FAREWELL'),
(-1043017,'Attacked! Help get this $N off of me!',0,0,0,0,'Disciple of Naralex - SAY_AGGRO_1'),
(-1043018,'Help!',0,0,0,0,'Disciple of Naralex - SAY_AGGRO_2'),
(-1043019,'Deal with this $N! I need to prepare to awake Naralex!',0,0,0,0,'Disciple of Naralex - SAY_AGGRO_3');

-- -1 047 000 RAZORFEN KRAUL
INSERT INTO script_texts (entry,content_default,sound,type,LANGUAGE,emote,comment) VALUES
(-1047000,'Woo hoo! Finally getting out of here. It\'s going to be rough though. Keep your eyes peeled for trouble.',0,0,0,0,'willix SAY_READY'),
(-1047001,'Up there is where Charlga Razorflank resides. Blasted old crone.',0,0,0,25,'willix SAY_1'),
(-1047002,'There\'s blueleaf tuber in this trench! It\'s like gold waiting to be mined I tell you!',0,0,0,0,'willix SAY_2'),
(-1047003,'There could be danger around every corner here.',0,0,0,0,'willix SAY_3'),
(-1047004,'I don\'t see how these foul animals live in this place... sheesh it smells!',0,0,0,0,'willix SAY_4'),
(-1047005,'I think I see a way for us to get out of this big twisted mess of a bramble.',0,0,0,0,'willix SAY_5'),
(-1047006,'Glad to be out of that wretched trench. Not much nicer up here though!',0,0,0,0,'willix SAY_6'),
(-1047007,'Finally! I\'ll be glad to get out of this place.',0,0,0,0,'willix SAY_7'),
(-1047008,'I think I\'ll rest a moment and catch my breath before heading back to Ratchet. Thanks for all the help!',0,0,0,0,'willix SAY_END'),
(-1047009,'$N heading this way fast! To arms!',0,0,0,0,'willix SAY_AGGRO_1'),
(-1047010,'Eek! $N coming right at us!',0,0,0,0,'willix SAY_AGGRO_2'),
(-1047011,'Egads! $N on me!',0,0,0,0,'willix SAY_AGGRO_3'),
(-1047012,'Help! Get this $N off of me!',0,0,0,0,'willix SAY_AGGRO_4');

-- -1 048 000 BLACKFATHOM DEEPS

-- -1 070 000 ULDAMAN
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1070000,'REUSE ME',0,0,0,0,'REUSE ME'),
(-1070001,'Who dares awaken Archaedas? Who dares the wrath of the makers!',5855,1,0,0,'archaedas SAY_AGGRO'),
(-1070002,'Awake ye servants, defend the discs!',5856,1,0,0,'archaedas SAY_AWAKE_GUARDIANS'),
(-1070003,'To my side, brothers. For the makers!',5857,1,0,0,'archaedas SAY_AWAKE_WARDERS'),
(-1070004,'Reckless mortal.',5858,1,0,0,'archaedas SAY_UNIT_SLAIN'),
(-1070005,'%s breaks free from his stone slumber!',0,2,0,0,'archaedas EMOTE_BREAK_FREE');

-- -1 090 000 GNOMEREGAN
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1090000,'With your help, I can evaluate these tunnels.',0,0,0,1,'emi shortfuse SAY_START'),
(-1090001,'Let\'s see if we can find out where these Troggs are coming from.... and put a stop to the invasion!',0,0,0,1,'emi shortfuse SAY_INTRO_1'),
(-1090002,'Such devastation... what a horrible mess...',0,0,0,5,'emi shortfuse SAY_INTRO_2'),
(-1090003,'It\'s quiet here....',0,0,0,1,'emi shortfuse SAY_INTRO_3'),
(-1090004,'...too quiet.',0,0,0,1,'emi shortfuse SAY_INTRO_4'),
(-1090005,'Look! Over there at the tunnel wall!',0,0,0,25,'emi shortfuse SAY_LOOK_1'),
(-1090006,'Trogg incursion! Defend me while I blast the hole closed!',0,0,0,5,'emi shortfuse SAY_HEAR_1'),
(-1090007,'Get this, $n off of me!',0,0,0,0,'emi shortfuse SAY_AGGRO_1'),
(-1090008,'I don\'t think one charge is going to cut it. Keep fending them off!',0,0,0,0,'emi shortfuse SAY_CHARGE_1'),
(-1090009,'The charges are set. Get back before they blow!',0,0,0,5,'emi shortfuse SAY_CHARGE_2'),
(-1090010,'Incoming blast in 10 seconds!',0,1,0,5,'emi shortfuse SAY_BLOW_1_10'),
(-1090011,'Incoming blast in 5 seconds. Clear the tunnel! Stay back!',0,1,0,5,'emi shortfuse SAY_BLOW_1_5'),
(-1090012,'FIRE IN THE HOLE!',0,1,0,25,'emi shortfuse SAY_BLOW_1'),
(-1090013,'Well done! Without your help I would have never been able to thwart that wave of troggs.',0,0,0,4,'emi shortfuse SAY_FINISH_1'),
(-1090014,'Did you hear something?',0,0,0,6,'emi shortfuse SAY_LOOK_2'),
(-1090015,'I heard something over there.',0,0,0,25,'emi shortfuse SAY_HEAR_2'),
(-1090016,'More troggs! Ward them off as I prepare the explosives!',0,0,0,0,'emi shortfuse SAY_CHARGE_3'),
(-1090017,'The final charge is set. Stand back!',0,0,0,1,'emi shortfuse SAY_CHARGE_4'),
(-1090018,'10 seconds to blast! Stand back!!!',0,1,0,5,'emi shortfuse SAY_BLOW_2_10'),
(-1090019,'5 seconds until detonation!!!!!',0,1,0,5,'emi shortfuse SAY_BLOW_2_5'),
(-1090020,'Nice work! I\'ll set off the charges to prevent any more troggs from making it to the surface.',0,0,0,1,'emi shortfuse SAY_BLOW_SOON'),
(-1090021,'FIRE IN THE HOLE!',0,1,0,0,'emi shortfuse SAY_BLOW_2'),
(-1090022,'Superb! Because of your help, my people stand a chance of re-taking our beloved city. Three cheers to you!',0,0,0,0,'emi shortfuse SAY_FINISH_2'),

(-1090023,'We come from below! You can never stop us!',0,1,0,1,'grubbis SAY_GRUBBIS_SPAWN'),

(-1090024,'Usurpers! Gnomeregan is mine!',5807,1,0,0,'thermaplugg SAY_AGGRO'),
(-1090025,'My machines are the future! They\'ll destroy you all!',5808,1,0,0,'thermaplugg  SAY_PHASE'),
(-1090026,'Explosions! MORE explosions! I\'ve got to have more explosions!',5809,1,0,0,'thermaplugg  SAY_BOMB'),
(-1090027,'...and stay dead! He got served',5810,1,0,0,'thermaplugg  SAY_SLAY'),

(-1090028,'$n attacking! Help!',0,0,0,0,'emi shortfuse SAY_AGGRO_2');

-- -1 109 000 SUNKEN TEMPLE
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1109000,'The walls of the chamber tremble. Something is happening...',0,2,0,0,'malfurion stormrage EMOTE_MALFURION'),
(-1109001,'Be steadfast, champion. I know why it is that you are here and I know what it is that you seek. Eranikus will not give up the shard freely. He has been twisted... twisted by the same force that you seek to destroy.',0,0,0,0,'malfurion stormrge SAY_MALFURION1'),
(-1109002,'Are you really surprised? Is it hard to believe that the power of an Old God could reach even inside the Dream? It is true - Eranikus, Tyrant of the Dream, wages a battle against us all. The Nightmare follows in his wake of destruction.',0,0,0,0,'malfurion stormrge SAY_MALFURION2'),
(-1109003,'Understand this, Eranikus wants nothing more than to be brought to Azeroth from the Dream. Once he is out, he will stop at nothing to destroy my physical manifestation. This, however, is the only way in which you could recover the scepter shard.',0,0,0,0,'malfurion stormrge SAY_MAFLURION3'),
(-1109004,'You will bring him back into this world, champion.',0,0,0,0,'malfurion Stormrge SAY_MALFURION4'),

(-1109005,'The shield be down! Rise up Atal\'ai! Rise up!',5861,6,0,0,'jammalan SAY_JAMMALAN_INTRO'),

(-1109006,'HAKKAR LIVES!',5870,1,0,0,'avatar SAY_AVATAR_BRAZIER_1'),
(-1109007,'I TASTE THE BLOOD OF LIFE!',5868,1,0,0,'avatar SAY_AVATAR_BRAZIER_2'),
(-1109008,'I DRAW CLOSER TO YOUR WORLD!',5867,1,0,0,'avatar SAY_AVATAR_BRAZIER_3'),
(-1109009,'I AM NEAR!',5869,1,0,0,'avatar SAY_AVATAR_BRAZIER_4'),
(-1109010,'I AM HERE!',0,1,0,0,'avatar SAY_AVATAR_SPAWN');

-- -1 129 000 RAZORFEN DOWNS
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1129000,'REUSE_ME',0,0,0,0,'REUSE_ME'),
(-1129001,'REUSE_ME',0,0,0,0,'REUSE_ME'),
(-1129002,'REUSE_ME',0,0,0,0,'REUSE_ME'),
(-1129003,'REUSE_ME',0,0,0,0,'REUSE_ME'),
(-1129004,'REUSE_ME',0,0,0,0,'REUSE_ME'),

(-1129005,'All right, stay close. These fiends will jump right out of the shadows at you if you let your guard down.',0,0,0,0,'belnistrasz SAY_READY'),
(-1129006,'Okay, here we go. It\'s going to take about five minutes to shut this thing down through the ritual. Once I start, keep the vermin off of me or it will be the end of us all!',0,0,0,0,'belnistrasz SAY_START_RIT'),
(-1129007,'You\'ll rue the day you crossed me, $N',0,0,0,0,'belnistrasz SAY_AGGRO_1'),
(-1129008,'Incoming $N - look sharp, friends!',0,0,0,0,'belnistrasz SAY_AGGRO_2'),
(-1129009,'Three minutes left -- I can feel the energy starting to build! Keep up the solid defense!',0,1,0,0,'belnistrasz SAY_3_MIN'),
(-1129010,'Just two minutes to go! We\'re half way there, but don\'t let your guard down!',0,1,0,0,'belnistrasz SAY_2_MIN'),
(-1129011,'One more minute! Hold on now, the ritual is about to take hold!',0,1,0,0,'belnistrasz SAY_1_MIN'),
(-1129012,'That\'s it -- we made it! The ritual is set in motion, and idol fires are about to go out for good! You truly are the heroes I thought you would be!',0,1,0,4,'belnistrasz SAY_FINISH');

-- -1 189 000 SCARLET MONASTERY
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1189000,'Ah, I have been waiting for a real challenge!',5830,1,0,0,'herod SAY_AGGRO'),
(-1189001,'Blades of Light!',5832,1,0,0,'herod SAY_WHIRLWIND'),
(-1189002,'Light, give me strength!',5833,1,0,0,'herod SAY_ENRAGE'),
(-1189003,'Hah, is that all?',5831,1,0,0,'herod SAY_KILL'),
(-1189004,'REUSE_ME',0,0,0,0,'REUSE_ME'),

(-1189005,'Infidels! They must be purified!',5835,1,0,0,'mograine SAY_MO_AGGRO'),
(-1189006,'Unworthy!',5836,1,0,0,'mograine SAY_MO_KILL'),
(-1189007,'At your side, milady!',5837,1,0,0,'mograine SAY_MO_RESSURECTED'),

(-1189008,'What, Mograine has fallen? You shall pay for this treachery!',5838,1,0,0,'whitemane SAY_WH_INTRO'),
(-1189009,'The Light has spoken!',5839,1,0,0,'whitemane SAY_WH_KILL'),
(-1189010,'Arise, my champion!',5840,1,0,0,'whitemane SAY_WH_RESSURECT'),

(-1189011,'REUSE_ME',0,0,0,0,'REUSE_ME'),
(-1189012,'REUSE_ME',0,0,0,0,'REUSE_ME'),
(-1189013,'REUSE_ME',0,0,0,0,'REUSE_ME'),
(-1189014,'REUSE_ME',0,0,0,0,'REUSE_ME'),

(-1189015,'The bastard got what he deserved.',0,0,1,0,'vishas SAY_TRIGGER_VORREL'),

(-1189016,'REUSE_ME',0,0,0,0,'REUSE_ME'),
(-1189017,'REUSE_ME',0,0,0,0,'REUSE_ME'),
(-1189018,'REUSE_ME',0,0,0,0,'REUSE_ME'),

(-1189019,'You will not defile these mysteries!',5842,1,0,0,'doan SAY_AGGRO'),
(-1189020,'Burn in righteous fire!',5843,1,0,0,'doan SAY_SPECIALAE'),

(-1189021,'REUSE_ME',0,0,0,0,'REUSE_ME'),

(-1189022,'It is over, your search is done! Let fate choose now, the righteous one.',11961,1,0,0,'horseman SAY_ENTRANCE'),
(-1189023,'Here\'s my body, fit and pure! Now, your blackened souls I\'ll cure!',12567,1,0,0,'horseman SAY_REJOINED'),
(-1189024,'So eager you are for my blood to spill, yet to vanquish me this my head you must kill!',11969,1,0,0,'horseman SAY_BODY_DEFEAT'),
(-1189025,'Over here, you idiot!',12569,1,0,0,'horseman SAY_LOST_HEAD'),
(-1189026,'Harken, cur! Tis you I spurn! Now, $N, feel the burn!',12573,1,0,0,'horseman SAY_CONFLAGRATION'),
(-1189027,'Soldiers arise, stand and fight! Bring victory at last to this fallen knight!',11963,1,0,0,'horseman SAY_SPROUTING_PUMPKINS'),
(-1189028,'Your body lies beaten, battered and broken. Let my curse be your own, fate has spoken.',11962,1,0,0,'horseman SAY_SLAY'),
(-1189029,'This end have I reached before. What new adventure lies in store?',11964,1,0,0,'horseman SAY_DEATH'),
(-1189030,'%s laughs.',0,2,0,0,'horseman EMOTE_LAUGH'),
(-1189031,'Horseman rise...',0,0,0,0,'horseman SAY_PLAYER1'),
(-1189032,'Your time is night...',0,0,0,0,'horseman SAY_PLAYER2'),
(-1189033,'You felt death once...',0,0,0,0,'horseman SAY_PLAYER3'),
(-1189034,'Now, know demise!',0,0,0,0,'horseman SAY_PLAYER4'),

(-1189035,'The master has fallen! Avenge him my brethren!',5834,1,0,0,'trainee SAY_TRAINEE_SPAWN'),

(-1189036,'Bow down! Kneel before the Ashbringer! A new dawn approaches, brother and sisters! Our message will be delivered to the filth of this world through the chosen one!',0,6,0,0,'mograine SAY_ASHBRINGER_ENTRANCE');

-- -1 209 000 ZUL'FARRAK
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1209000,'How dare you enter my sanctum!',0,0,0,0,'zumrah SAY_INTRO'),
(-1209001,'Sands consume you!',5872,1,14,0,'zumrah SAY_AGGRO'),
(-1209002,'Fall!',5873,1,14,0,'zumrah SAY_KILL'),
(-1209003,'Come to me, my children!',0,0,8,0,'zumrah SAY_SUMMON');

-- -1 229 000 BLACKROCK SPIRE
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1229000,'%s begins to regain its strength!',0,2,0,0,'pyroguard EMOTE_BEGIN'),
(-1229001,'%s is nearly at full strength!',0,2,0,0,'pyroguard EMOTE_NEAR'),
(-1229002,'%s regains its power and breaks free of its bonds!',0,2,0,0,'pyroguard EMOTE_FULL'),
(-1229003,'Ha! Ha! Ha! Thank you for freeing me, fools. Now let me repay you by charring the flesh from your bones.',0,1,0,0,'pyroguard SAY_FREE'),

(-1229004,'Excellent... it would appear as if the meddlesome insects have arrived just in time to feed my legion. Welcome, mortals!',0,1,0,1,'nefarius SAY_INTRO_1'),
(-1229005,'Let not even a drop of their blood remain upon the arena floor, my children. Feast on their souls!',0,1,0,1,'nefarius SAY_INTRO_2'),
(-1229006,'Foolsss... Kill the one in the dress!',0,1,0,0,'nefarius SAY_NEFARIUS_TAUNT1'),
(-1229007,'Sire, let me join the fray! I shall tear their spines out with my bare hands!',0,1,0,1,'rend SAY_REND_TAUNT1'),
(-1229008,'Concentrate your attacks upon the healer!',0,1,0,0,'nefarius SAY_NEFARIUS_TAUNT2'),
(-1229009,'Inconceivable!',0,1,0,0,'nefarius SAY_NEFARIUS_TAUNT3'),
(-1229010,'Do not force my hand, children! I shall use your hides to line my boots.',0,1,0,0,'nefarius SAY_NEFARIUS_TAUNT4'),
(-1229011,'Defilers!',0,1,0,0,'rend SAY_REND_TAUNT2'),
(-1229012,'Impossible!',0,1,0,0,'rend SAY_REND_TAUNT3'),
(-1229013,'Your efforts will prove fruitless. None shall stand in our way!',0,1,0,0,'nefarius SAY_NEFARIUS_TAUNT5'),
(-1229014,'THIS CANNOT BE!!! Rend, deal with these insects.',0,1,0,1,'nefarius SAY_NEFARIUS_LOSE1'),
(-1229015,'With pleasure...',0,1,0,0,'rend SAY_REND_ATTACK'),
(-1229016,'The Warchief shall make quick work of you, mortals. Prepare yourselves!',0,1,0,25,'nefarius SAY_WARCHIEF'),
(-1229017,'Taste in my power!',0,1,0,0,'nefarius SAY_BUFF_GYTH'),
(-1229018,'Your victory shall be short lived. The days of both the Alliance and Horde are coming to an end. The next time we meet shall be the last.',0,1,0,1,'nefarius SAY_VICTORY'),

(-1229019,'%s is knocked off his drake!',0,2,0,0,'rend EMOTE_KNOCKED_OFF'),

(-1229020,'Intruders are destroying our eggs! Stop!!',0,1,0,0,'rookery hatcher - SAY_ROOKERY_EVENT_START'),

(-1229021,'I promise you an eternity of dung clean up duty for this failure!',0,1,0,0,'nefarius SAY_NEFARIUS_TAUNT6'),
(-1229022,'Use the freezing breath, imbecile!',0,1,0,0,'nefarius SAY_NEFARIUS_TAUNT7'),
(-1229023,'I will wear your skin as a fashion accessory!',0,1,0,0,'nefarius SAY_NEFARIUS_TAUNT8'),
(-1229024,'Flee while  you still have chance, mortals. You will pray for a swift death should I enter the arena.',0,1,0,0,'rend SAY_REND_TAUNT3');

-- -1 230 000 BLACKROCK DEPTHS
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1230000,'Ah, hits the spot!',0,0,0,5,'rocknot SAY_GOT_BEER'),
(-1230001,'Come to aid the Throne!',0,1,0,0,'dagran SAY_AGGRO'),
(-1230002,'Hail to the king, baby!',0,1,0,0,'dagran SAY_SLAY'),
(-1230003,'You have challenged the Seven, and now you will die!',0,0,0,0,'doomrel SAY_DOOMREL_START_EVENT'),

(-1230004,'The Sons of Thaurissan shall watch you perish in the Ring of the Law!',0,1,0,5,'grimstone SAY_START_1'),
(-1230005,'You have been sentenced to death for crimes against the Dark Iron Nation!',0,1,0,25,'grimstone SAY_START_2'),
(-1230006,'Unleash the fury and let it be done!',0,1,0,15,'grimstone SAY_OPEN_EAST_GATE'),
(-1230007,'But your real punishment lies ahead.',0,1,0,1,'grimstone SAY_SUMMON_BOSS_1'),
(-1230008,'Haha! I bet you thought you were done!',0,1,0,153,'grimstone SAY_SUMMON_BOSS_2'),
(-1230009,'Good Riddance!',0,1,0,5,'grimstone SAY_OPEN_NORTH_GATE'),

(-1230010,'Thank you, $N! I\'m free!!!',0,0,0,0,'dughal SAY_FREE'),
(-1230011,'You locked up the wrong Marshal, $N. Prepare to be destroyed!',0,0,0,0,'windsor SAY_AGGRO_1'),
(-1230012,'I bet you\'re sorry now, aren\'t you?',0,0,0,0,'windsor SAY_AGGRO_2'),
(-1230013,'You better hold me back or $N is going to feel some prison house beatings.',0,0,0,0,'windsor SAY_AGGRO_3'),
(-1230014,'Let\'s get a move on. My gear should be in the storage area up this way...',0,0,0,0,'windsor SAY_START'),
(-1230015,'Check that cell, $N. If someone is alive in there, we need to get them out.',0,0,0,25,'windsor SAY_CELL_DUGHAL_1'),
(-1230016,'Good work! We\'re almost there, $N. This way.',0,0,0,0,'windsor SAY_CELL_DUGHAL_3'),
(-1230017,'This is it, $N. My stuff should be in that room. Cover me, I\'m going in!',0,0,0,0,'windsor SAY_EQUIPMENT_1'),
(-1230018,'Ah, there it is!',0,0,0,0,'windsor SAY_EQUIPMENT_2'),
(-1230019,'Can you feel the power, $N??? It\'s time to ROCK!',0,0,0,0,'reginald_windsor SAY__EQUIPMENT_3'),
(-1230020,'Now we just have to free Tobias and we can get out of here. This way!',0,0,0,0,'reginald_windsor SAY__EQUIPMENT_4'),
(-1230021,'Open it.',0,0,0,25,'reginald_windsor SAY_CELL_JAZ_1'),
(-1230022,'I never did like those two. Let\'s get moving.',0,0,0,0,'reginald_windsor SAY_CELL_JAZ_2'),
(-1230023,'Open it and be careful this time!',0,0,0,25,'reginald_windsor SAY_CELL_SHILL_1'),
(-1230024,'That intolerant dirtbag finally got what was coming to him. Good riddance!',0,0,0,66,'reginald_windsor SAY_CELL_SHILL_2'),
(-1230025,'Alright, let\'s go.',0,0,0,0,'reginald_windsor SAY_CELL_SHILL_3'),
(-1230026,'Open it. We need to hurry up. I can smell those Dark Irons coming a mile away and I can tell you one thing, they\'re COMING!',0,0,0,25,'reginald_windsor SAY_CELL_CREST_1'),
(-1230027,'He has to be in the last cell. Unless... they killed him.',0,0,0,0,'reginald_windsor SAY_CELL_CREST_2'),
(-1230028,'Get him out of there!',0,0,0,25,'reginald_windsor SAY_CELL_TOBIAS_1'),
(-1230029,'Excellent work, $N. Let\'s find the exit. I think I know the way. Follow me!',0,0,0,0,'reginald_windsor SAY_CELL_TOBIAS_2'),
(-1230030,'We made it!',0,0,0,4,'reginald_windsor SAY_FREE_1'),
(-1230031,'Meet me at Maxwell\'s encampment. We\'ll go over the next stages of the plan there and figure out a way to decode my tablets without the decryption ring.',0,0,0,1,'reginald_windsor SAY_FREE_2'),
(-1230032,'Thank you! I will run for safety immediately!',0,0,0,0,'tobias SAY_TOBIAS_FREE_1'),
(-1230033,'Finally!! I can leave this dump.',0,0,0,0,'tobias SAY_TOBIAS_FREE_2'),

(-1230035,'%s cries out an alarm!',0,2,0,0,'general_angerforge EMOTE_ALARM'),

(-1230036,'I want more ale! Give me more ale!',0,0,0,0,'rocknot SAY_MORE_BEER'),
(-1230037,'You\'ll pay for that!',0,0,0,0,'Grim Patron SAY_PISSED_PATRON_1'),
(-1230038,'No!',0,0,0,0,'Grim Patron SAY_PISSED_PATRON_2'),
(-1230039,'Oh! Now I\'m pissed!',0,0,0,0,'Grim Patron SAY_PISSED_PATRON_3'),
(-1230040,'Violence! Property damage! None shall pass!!',0,1,0,0,'Phalanx YELL_PHALANX_AGGRO'),
(-1230041,'Get away from those kegs!',0,1,0,0,'Hurley Blackbreath YELL_HURLEY_SPAWN'),
(-1230042,'You\'ll pay for that!',0,0,0,0,'Hurley Blackbreath SAY_HURLEY_AGGRO'),
(-1230043,'You can\'t hide from us. Prepare to burn!',0,1,0,0,'anvilrage guardsman SAY_GUARD_AGGRO'),
(-1230044,'That one\'s empty!!',0,0,0,60,'rocknot SAY_BARREL_1'),
(-1230045,'Ah, empty again!',0,0,0,35,'rocknot SAY_BARREL_2'),
(-1230046,'ALE!',0,0,0,60,'rocknot SAY_BARREL_3'),
(-1230047,'I\m out of here!',0,0,0,0,'rocknot SAY_ROCKNOT_DESPAWN'),
(-1230048,'HALT! There were...reports...of a...disturbance.',0,1,0,0,'Fireguard Destroyer YELL_PATROL_1'),
(-1230049,'We...are...investigating.',0,1,0,0,'Fireguard Destroyer YELL_PATROL_2'),
(-1230050,'Try the boar! It\'s my new recipe!',0,0,0,0,'Plugger Spazzring SAY_OOC_1'),
(-1230051,'Enjoy! You won\'t find better ale anywhere!',0,0,0,0,'Plugger Spazzring SAY_OOC_2'),
(-1230052,'Drink up! There\'s more where that came from!',0,0,0,0,'Plugger Spazzring SAY_OOC_3'),
(-1230053,'Have you tried the Dark Iron Ale? It\'s the best!',0,0,0,0,'Plugger Spazzring SAY_OOC_4'),
(-1230054,'What are you doing over there?',0,1,0,0,'Plugger Spazzring YELL_STOLEN_1'),
(-1230055,'Hey! Get away from that!',0,1,0,0,'Plugger Spazzring YELL_STOLEN_2'),
(-1230056,'No stealing the goods!',0,1,0,0,'Plugger Spazzring YELL_STOLEN_3'),
(-1230057,'That\'s it! You\'re going down!',0,1,0,0,'Plugger Spazzring YELL_AGRRO_1'),
(-1230058,'That\'s it! No more beer until this mess is sorted out!',0,1,0,0,'Plugger Spazzring YELL_AGRRO_2'),
(-1230059,'Hey, my pockets were picked!',0,1,0,0,'Plugger Spazzring YELL_PICKPOCKETED'),
(-1230060,'Ha! You can\'t even begin to imagine the futility of your efforts.',0,1,0,0,'Emperor Thaurissan YELL_SENATOR_1'),
(-1230061,'Is that the best you can do? Do you really expect that you could defeat someone as awe inspiring as me?',0,1,0,0,'Emperor Thaurissan YELL_SENATOR_2'),
(-1230062,'They were just getting in the way anyways.',0,1,0,0,'Emperor Thaurissan YELL_SENATOR_3'),
(-1230063,'Your efforts are utterly pointless, fools! You will never be able to defeat me!',0,1,0,0,'Emperor Thaurissan YELL_SENATOR_4'),
(-1230064,'I will crush you into little tiny pieces!',0,1,0,0,'Emperor Thaurissan YELL_AGGRO_2'),
(-1230065,'Prepare to meet your doom at the hands of Ragnaros\' most powerful servant!',0,1,0,0,'Emperor Thaurissan YELL_AGGRO_3'),
(-1230066,'Hey, Rocknot!',0,0,0,0,'Nagmara SAY_NAGMARA_1'),
(-1230067,'Let\'s go, honey.',0,0,0,0,'Nagmara SAY_NAGMARA_2'),
(-1230068,'%s kisses her lover.',0,2,0,0,'Nagmara TEXTEMOTE_NAGMARA'),
(-1230069,'%s kisses Mistress Nagmara',0,2,0,17,'Rocknot TEXTEMOTE_ROCKNOT');

-- -1 249 000 ONYXIA'S LAIR
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1249000,'How fortuitous. Usually, I must leave my lair to feed.',0,1,0,0,'onyxia SAY_AGGRO'),
(-1249001,'Learn your place mortal!',0,1,0,0,'onyxia SAY_KILL'),
(-1249002,'This meaningless exertion bores me. I\'ll incinerate you all from above!',0,1,0,254,'onyxia SAY_PHASE_2_TRANS'),
(-1249003,'It seems you\'ll need another lesson, mortals!',0,1,0,293,'onyxia SAY_PHASE_3_TRANS'),
(-1249004,'%s takes in a deep breath...',0,3,0,0,'onyxia EMOTE_BREATH'),
(-1249005,'You seek to lure me from my clutch? You shall pay for your insolence!',0,1,0,0,'onyxia SAY_KITE');

-- -1 289 000 SCHOLOMANCE
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1289000,'School is in session!',0,1,0,0,'gandling SAY_GANDLING_SPAWN'),
(-1289001,'What is this?! How dare you!',0,1,0,0,'vectus YELL_VECTUS_GAMBIT');

-- -1 309 000 ZUL'GURUB
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1309000,'Let the coils of hate unfurl!',8421,1,0,0,'venoxis SAY_TRANSFORM'),
(-1309001,'Ssserenity..at lassst!',0,1,0,0,'venoxis SAY_DEATH'),

(-1309002,'Lord Hir\'eek, grant me wings of vengance!',8417,1,0,0,'jeklik SAY_AGGRO'),
(-1309003,'I command you to rain fire down upon these invaders!',0,1,0,0,'jeklik SAY_RAIN_FIRE'),
(-1309004,'Finally ...death. Curse you Hakkar! Curse you!',8422,1,0,0,'jeklik SAY_DEATH'),

(-1309005,'Draw me to your web mistress Shadra. Unleash your venom!',8418,1,0,0,'marli SAY_AGGRO'),
(-1309006,'Shadra, make of me your avatar!',0,1,0,0,'marli SAY_TRANSFORM'),
(-1309007,'Aid me my brood!',0,1,0,0,'marli SAY_SPIDER_SPAWN'),
(-1309008,'Bless you mortal for this release. Hakkar controls me no longer...',8423,1,0,0,'marli SAY_DEATH'),

(-1309009,'Shirvallah, fill me with your RAGE!',8419,1,0,0,'thekal SAY_AGGRO'),
(-1309010,'Hakkar binds me no more! Peace at last!',8424,1,0,0,'thekal SAY_DEATH'),

(-1309011,'Bethekk, your priestess calls upon your might!',8416,1,0,0,'arlokk SAY_AGGRO'),
(-1309012,'Feast on $n, my pretties!',0,1,0,0,'arlokk SAY_FEAST_PANTHER'),
(-1309013,'At last, I am free of the Soulflayer!',8412,1,0,0,'arlokk SAY_DEATH'),

(-1309014,'Welcome to da great show friends! Step right up to die!',8425,1,0,0,'jindo SAY_AGGRO'),

(-1309015,'I\'ll feed your souls to Hakkar himself!',8413,1,0,0,'mandokir SAY_AGGRO'),
(-1309016,'DING!',0,1,0,0,'mandokir SAY_DING_KILL'),
(-1309017,'GRATS!',0,1,0,0,'mandokir SAY_GRATS_JINDO'),
(-1309018,'$N! I\'m watching you!',0,1,0,0,'mandokir SAY_WATCH'),
(-1309019,'Don\'t make me angry. You won\'t like it when I\'m angry.',0,4,0,0,'mandokir SAY_WATCH_WHISPER'),

(-1309020,'PRIDE HERALDS THE END OF YOUR WORLD. COME, MORTALS! FACE THE WRATH OF THE SOULFLAYER!',8414,1,0,0,'hakkar SAY_AGGRO'),
(-1309021,'Fleeing will do you no good, mortals!',0,1,0,0,'hakkar SAY_FLEEING'),
(-1309022,'You dare set foot upon Hakkari holy ground? Minions of Hakkar, destroy the infidels!',0,6,0,0,'hakkar SAY_MINION_DESTROY'),
(-1309023,'Minions of Hakkar, hear your God. The sanctity of this temple has been compromised. Invaders encroach upon holy ground! The Altar of Blood must be protected. Kill them all!',0,6,0,0,'hakkar SAY_PROTECT_ALTAR'),

(-1309024,'%s goes into a rage after seeing his raptor fall in battle!',0,2,0,0,'mandokir EMOTE_RAGE'),

(-1309025,'The brood shall not fall!',0,1,0,0,'marli SAY_TRANSFORM_BACK'),

(-1309026,'%s emits a deafening shriek!',0,2,0,0,'jeklik SAY_SHRIEK'),
(-1309027,'%s begins to cast a Great Heal!',0,2,0,0,'jeklik SAY_HEAL'),

(-1309028,'%s fully engulfs in flame and a maddened look appears in his eyes!',0,2,0,0,'gurubashi bat rider - SAY_SELF_DETONATE');

-- -1 329 000 STRATHOLME
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1329000,'Thanks to Egan',0,0,0,0,'freed_soul SAY_ZAPPED0'),
(-1329001,'Rivendare must die',0,0,0,0,'freed_soul SAY_ZAPPED1'),
(-1329002,'Who you gonna call?',0,0,0,0,'freed_soul SAY_ZAPPED2'),
(-1329003,'Don\'t cross those beams!',0,0,0,0,'freed_soul SAY_ZAPPED3'),

(-1329004,'An Ash\'ari Crystal has fallen! Stay true to the Lich King, my brethren, and attempt to resummon it.',0,6,0,0,'thuzadin acolyte SAY_ANNOUNCE_ZIGGURAT_1'),
(-1329005,'One of the Ash\'ari Crystals has been destroyed! Slay the intruders!',0,6,0,0,'thuzadin acolyte SAY_ANNOUNCE_ZIGGURAT_2'),
(-1329006,'An Ash\'ari Crystal has been toppled! Restore the ziggurat before the Necropolis is vulnerable!',0,6,0,0,'thuzadin acolyte SAY_ANNOUNCE_ZIGGURAT_3'),
(-1329007,'The Ash\'ari Crystals have been destroyed! The Slaughterhouse is vulnerable!',0,6,0,0,'baron rivendare SAY_ANNOUNCE_RIVENDARE'),

(-1329008,'Intruders at the Service Gate! Lord Rivendare must be warned!',0,6,0,0,'barthilas SAY_WARN_BARON'),
(-1329009,'Intruders! More pawns of the Argent Dawn, no doubt. I already count one of their number among my prisoners. Withdraw from my domain before she is executed!',0,6,0,0,'baron rivendare SAY_ANNOUNCE_RUN_START'),
(-1329010,'You\'re still here? Your foolishness is amusing! The Argent Dawn wench needn\'t suffer in vain. Leave at once and she shall be spared!',0,6,0,0,'baron rivendare SAY_ANNOUNCE_RUN_10_MIN'),
(-1329011,'I shall take great pleasure in taking this poor wretch\'s life! It\'s not too late, she needn\'t suffer in vain. Turn back and her death shall be merciful!',0,6,0,0,'baron rivendare SAY_ANNOUNCE_RUN_5_MIN'),
(-1329012,'May this prisoner\'s death serve as a warning. None shall defy the Scourge and live!',0,6,0,0,'baron rivendare SAY_ANNOUNCE_RUN_FAIL'),
(-1329013,'So you see fit to toy with the Lich King\'s creations? Ramstein, be sure to give the intruders a proper greeting.',0,6,0,0,'baron rivendare SAY_ANNOUNCE_RAMSTEIN'),
(-1329014,'Time to take matters into my own hands. Come. Enter my domain and challenge the might of the Scourge!',0,6,0,0,'baron rivendare SAY_UNDEAD_DEFEAT'),
(-1329015,'You did it... you\'ve slain Baron Rivendare! The Argent Dawn shall hear of your valiant deeds!',0,0,0,0,'ysida SAY_EPILOGUE'),

(-1329016,'Today you have unmade what took me years to create! For this you shall all die by my hand!',0,1,0,0,'dathrohan SAY_AGGRO'),
(-1329017,'You fools think you can defeat me so easily? Face the true might of the Nathrezim!',0,1,0,0,'dathrohan SAY_TRANSFORM'),
(-1329018,'Damn you mortals! All my plans of revenge, all my hate... all burned to ash...',0,0,0,0,'dathrohan SAY_DEATH'),
(-1329019,'Don\'t worry about me!  Slay this dreadful beast and cleanse this world of his foul taint!',0,6,0,0,'ysida YSIDA_SAY_RUN_5_MIN'),
(-1329020,'My death means nothing.... light... will... prevail!',0,6,0,0,'ysida YSIDA_SAY_RUN_FAIL'),

(-1329021,'Rivendare! I come for you!',0,1,0,0,'aurius YELL_AURIUS_AGGRO'),
(-1329022,'Argh!',0,0,0,0,'aurius SAY_AURIUS_DEATH'),

(-1329023,'Move back and hold the line! We cannot fail or all will be lost!',0,1,0,0,'YELL_BASTION_BARRICADE'),
(-1329024,'Move to the stairs and defend!',0,1,0,0,'YELL_BASTION_STAIRS'),
(-1329025,'The Scourge have broken into the Bastion! Redouble your efforts! We must not fail!',0,1,0,0,'YELL_BASTION_ENTRANCE'),
(-1329026,'They have broken into the Hall of Lights! We must stop the intruders!',0,1,0,0,'YELL_BASTION_HALL_LIGHTS'),
(-1329027,'The Scourge have broken through in all wings! May the light defeat these foul creatures! We shall fight to the last!',0,1,0,0,'YELL_BASTION_INNER_1'),
(-1329028,'This will not be the end of the Scarlet Crusade! You will not break our line!',0,1,0,0,'YELL_BASTION_INNER_2');

-- -1 349 000 MARAUDON

-- -1 389 000 RAGEFIRE CHASM

-- -1 409 000 MOLTEN CORE
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1409000,'%s performs one last service for Ragnaros.',0,2,0,0,'geddon EMOTE_SERVICE'),
(-1409001,'REUSE ME',0,0,0,0,'REUSE ME'),
(-1409002,'%s refuses to die while its master is in trouble.',0,2,0,0,'core rager EMOTE_LOWHP'),

(-1409003,'Reckless mortals! None may challenge the Sons of the Living flame!',8035,1,0,0,'majordomo SAY_AGGRO'),
(-1409004,'The runes of warding have been destroyed! Hunt down the infidels, my brethren!',8039,6,0,0,'majordomo SAY_SPAWN'),
(-1409005,'Ashes to Ashes!',8037,1,0,0,'majordomo SAY_SLAY_1'),
(-1409006,'Burn mortals! Burn for this transgression!',8036,1,0,0,'majordomo SAY_SLAY_2'),
(-1409007,'Impossible! Stay your attack, mortals... I submit! I submit!',8038,1,0,0,'majordomo SAY_DEFEAT_1'),

(-1409008,'Behold Ragnaros - the Firelord! He who was ancient when this world was young! Bow before him, mortals! Bow before your ending!',8040,1,0,0,'ragnaros SAY_SUMMON_MAJ'),
(-1409009,'TOO SOON! YOU HAVE AWAKENED ME TOO SOON, EXECUTUS! WHAT IS THE MEANING OF THIS INTRUSION???',8043,1,0,0,'ragnaros SAY_ARRIVAL1_RAG'),
(-1409010,'These mortal infidels, my lord! They have invaded your sanctum and seek to steal your secrets!',8041,1,0,0,'ragnaros SAY_ARRIVAL2_MAJ'),
(-1409011,'FOOL! YOU ALLOWED THESE INSECTS TO RUN RAMPANT THROUGH THE HALLOWED CORE? AND NOW YOU LEAD THEM TO MY VERY LAIR? YOU HAVE FAILED ME, EXECUTUS! JUSTICE SHALL BE MET, INDEED!',8044,1,0,0,'ragnaros SAY_ARRIVAL3_RAG'),
(-1409012,'NOW FOR YOU, INSECTS! BOLDLY, YOU SOUGHT THE POWER OF RAGNAROS. NOW YOU SHALL SEE IT FIRSTHAND!',8045,1,0,0,'ragnaros SAY_ARRIVAL5_RAG'),

(-1409013,'COME FORTH, MY SERVANTS! DEFEND YOUR MASTER!',8049,1,0,0,'ragnaros SAY_REINFORCEMENTS1'),
(-1409014,'YOU CANNOT DEFEAT THE LIVING FLAME! COME YOU MINIONS OF FIRE! COME FORTH YOU CREATURES OF HATE! YOUR MASTER CALLS!',8050,1,0,0,'ragnaros SAY_REINFORCEMENTS2'),
(-1409015,'BY FIRE BE PURGED!',8046,1,0,0,'ragnaros SAY_HAND'),
(-1409016,'TASTE THE FLAMES OF SULFURON!',8047,1,0,0,'ragnaros SAY_WRATH'),
(-1409017,'DIE INSECT!',8051,1,0,0,'ragnaros SAY_KILL'),
(-1409018,'MY PATIENCE IS DWINDLING! COME GNATS: TO YOUR DEATH!',8048,1,0,0,'ragnaros SAY_MAGMABURST'),

(-1409019,'You think you\'ve won already? Perhaps you\'ll need another lesson in pain!',0,1,0,0,'majordomo SAY_LAST_ADD'),
(-1409020,'Brashly, you have come to wrest the secrets of the Living Flame! You will soon regret the recklessness of your quest.',0,1,0,0,'majordomo SAY_DEFEAT_2'),
(-1409021,'I go now to summon the lord whose house this is. Should you seek an audience with him, your paltry lives will surely be forfeit! Nevertheless, seek out his lair, if you dare!',0,1,0,0,'majordomo SAY_DEFEAT_3'),
(-1409022,'My flame! Please don\'t take away my flame... ',8042,1,0,0,'ragnaros SAY_ARRIVAL4_MAJ'),
(-1409023,'Very well, $n.',0,0,0,0,'majordomo SAY_SUMMON_0'),
(-1409024,'Impudent whelps! You\'ve rushed headlong to your own deaths! See now, the master stirs!',0,1,0,0,'majordomo SAY_SUMMON_1'),
(-1409025,'%s forces one of his Firesworn minions to erupt!',0,2,0,0,'Garr EMOTE_MASSIVE_ERUPTION');

-- -1 429 000 DIRE MAUL
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1429000,'The demon is loose! Quickly we must restrain him!',0,6,0,0,'highborne summoner SAY_FREE_IMMOLTHAR'),
(-1429001,'Who dares disrupt the sanctity of Eldre\'Thalas? Face me, cowards!',0,6,0,0,'prince tortheldrin SAY_KILL_IMMOLTHAR'),

(-1429002,'At last... Freed from his cursed grasp!',0,6,0,0,'old ironbark SAY_IRONBARK_REDEEM'),

(-1429003,'The king is dead - OH NOES! Summon Mizzle da Crafty! He knows what to do next!',0,1,0,0,'cho\'rush SAY_KING_DEAD'),

(-1429004,'Doh!',0,0,0,0,'slip\'kik SAY_SLIPKIK_TRAP');

-- -1 469 000 BLACKWING LAIR
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1469000,'None of your kind should be here! You\'ve doomed only yourselves!',8286,1,0,0,'broodlord SAY_AGGRO'),
(-1469001,'Clever Mortals but I am not so easily lured away from my sanctum!',8287,1,0,0,'broodlord SAY_LEASH'),

(-1469002,'Run! They are coming!',0,1,0,0,'vaelastrasz blackwing tech SAY_INTRO_TECH'),
(-1469003,'%s flinches as its skin shimmers.',0,2,0,0,'chromaggus EMOTE_SHIMMER'),

(-1469004,'In this world where time is your enemy, it is my greatest ally. This grand game of life that you think you play in fact plays you. To that I say...',0,1,0,1,'victor_nefarius SAY_GAMESBEGIN_1'),
(-1469005,'Let the games begin!',8280,1,0,22,'victor_nefarius SAY_GAMESBEGIN_2'),
(-1469006,'Ah...the heroes. You are persistent, aren\'t you? Your ally here attempted to match his power against mine - and paid the price. Now he shall serve me...by slaughtering you.',8279,1,0,23,'victor_nefarius SAY_NEFARIUS_CORRUPT'),

(-1469007,'Well done, my minions. The mortals\' courage begins to wane! Now, let\'s see how they contend with the true Lord of Blackrock Spire!',8288,6,0,0,'nefarian SAY_AGGRO'),
(-1469008,'Enough! Now you vermin shall feel the force of my birthright, the fury of the earth itself.',8289,1,0,0,'nefarian SAY_XHEALTH'),
(-1469009,'BURN! You wretches! BURN!',8290,6,0,0,'nefarian SAY_SHADOWFLAME'),
(-1469010,'Impossible! Rise my minions! Serve your master once more!',8291,1,0,0,'nefarian SAY_RAISE_SKELETONS'),
(-1469011,'Worthless $N! Your friends will join you soon enough!',8293,1,0,0,'nefarian SAY_SLAY'),
(-1469012,'This cannot be!  I am the master here!  You mortals are nothing to my kind!  Do you hear me? Nothing!',8292,1,0,0,'nefarian SAY_DEATH'),
(-1469013,'Mages too? You should be more careful when you play with magic...',0,1,0,0,'nefarian SAY_MAGE'),
(-1469014,'Warriors, I know you can hit harder than that! Let\'s see it!',0,1,0,0,'nefarian SAY_WARRIOR'),
(-1469015,'Druids and your silly shapeshifting. Let\'s see it in action!',0,1,0,0,'nefarian SAY_DRUID'),
(-1469016,'Priests! If you\'re going to keep healing like that, we might as well make it a little more interesting!',0,1,0,0,'nefarian SAY_PRIEST'),
(-1469017,'Paladins, I\'ve heard you have many lives. Show me.',0,1,0,0,'nefarian SAY_PALADIN'),
(-1469018,'Shamans, show me what your totems can do!',0,1,0,0,'nefarian SAY_SHAMAN'),
(-1469019,'Warlocks, you shouldn\'t be playing with magic you don\'t understand. See what happens?',0,1,0,0,'nefarian SAY_WARLOCK'),
(-1469020,'Hunters and your annoying pea-shooters!',0,1,0,0,'nefarian SAY_HUNTER'),
(-1469021,'Rogues? Stop hiding and face me!',0,1,0,0,'nefarian SAY_ROGUE'),

(-1469022,'You\'ll pay for forcing me to do this.',8275,1,0,0,'razorgore SAY_EGGS_BROKEN1'),
(-1469023,'Fools! These eggs are more precious than you know!',8276,1,0,0,'razorgore SAY_EGGS_BROKEN2'),
(-1469024,'No - not another one! I\'ll have your heads for this atrocity!',8277,1,0,0,'razorgore SAY_EGGS_BROKEN3'),
(-1469025,'If I fall into the abyss I\'ll take all of you mortals with me...',8278,1,0,0,'razorgore SAY_RAZORGORE_DEATH'),

(-1469026,'Too late, friends! Nefarius\' corruption has taken hold...I cannot...control myself.',8281,1,0,1,'vaelastrasz SAY_LINE1'),
(-1469027,'I beg you, mortals - FLEE! Flee before I lose all sense of control! The black fire rages within my heart! I MUST- release it!',8282,1,0,1,'vaelastrasz SAY_LINE2'),
(-1469028,'FLAME! DEATH! DESTRUCTION! Cower, mortals before the wrath of Lord...NO - I MUST fight this! Alexstrasza help me, I MUST fight it!',8283,1,0,1,'vaelastrasz SAY_LINE3'),
(-1469029,'Nefarius\' hate has made me stronger than ever before! You should have fled while you could, mortals! The fury of Blackrock courses through my veins!',8285,1,0,0,'vaelastrasz SAY_HALFLIFE'),
(-1469030,'Forgive me, $N! Your death only adds to my failure!',8284,1,0,0,'vaelastrasz SAY_KILLTARGET'),

(-1469031,'Death Knights, get over here!',0,1,0,0,'nefarian SAY_DEATH_KNIGHT'),

(-1469032,'Get up, little red wyrm...and destroy them!',0,1,0,1,'victor_nefarius SAY_NEFARIUS_CORRUPT_2'),

(-1469033,'%s flee as the controlling power of the orb is drained.',0,2,0,0,'razorgore EMOTE_TROOPS_FLEE'),

(-1469034,'Run! They are coming.',0,1,0,0,'blackwing technician SAY_TECHNICIAN_RUN'),

(-1469035,'Orb of Domination loses power and shuts off!',0,2,0,0,'razorgore EMOTE_ORB_SHUT_OFF'),

(-1469036,'Looking for this: [Red Scepter Shard]? Come and get it...',0,6,0,0,'nefarius YELL_REDSHARD_TAUNT_1'),
(-1469037,'I wonder, heroes, what will you do when I manage to destroy this shard? Will the world be beyond Salvation?',0,6,0,0,'nefarius YELL_REDSHARD_TAUNT_2'),
(-1469038,'%s\'s laughter echoes through the halls of Blackwing.',0,7,0,0,'nefarius EMOTE_REDSHARD_TAUNT_1'),
(-1469039,'What\'s this? A chip in the shard? Finally, a weakness exposed... It won\'t be long now.',0,6,0,0,'nefarius YELL_REDSHARD_TAUNT_3'),
(-1469040,'Not even my father could have planned such tragedy... such chaos. You will never make it! Time is running out!',0,6,0,0,'nefarius YELL_REDSHARD_TAUNT_4'),
(-1469041,'Would it not be glorious if you somehow managed to defeat me but not before I could destroy the only hope Kalimdor may have? I am giddy with anticipation. Soon... Soon it will be dust.',0,6,0,0,'nefarius YELL_REDSHARD_TAUNT_5'),
(-1469042,'Ah, the protective layer is nearly gone. Do you hear that, heroes? That... That is the sound of inevitability. Of your hopes and dreams being crushed under the weight of my being.',0,6,0,0,'nefarius YELL_REDSHARD_TAUNT_6'),
(-1469044,'%s\'laughter can once more be heard through the halls of Blackwing.',0,7,0,0,'nefarius EMOTE_REDSHARD_TAUNT_2'),
(-1469043,'The scepter shard is no more! Your beloved Kalimdor shall soon be a desolate wasteland. I will, of course, turn the Eastern Kingdoms into an ashen, lifeless void...',0,6,0,0,'nefarius YELL_REDSHARD_TAUNT_7');

-- -1 509 000 RUINS OF AHN'QIRAJ
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1509000,'%s senses your fear.',0,2,0,0,'moam EMOTE_AGGRO'),
(-1509001,'%s bristles with energy!',0,2,0,0,'moan EMOTE_MANA_FULL'),
(-1509028,'%s drains your mana and turns to stone.',0,2,0,0,'moam EMOTE_ENERGIZING'),

(-1509002,'%s sets eyes on $N!',0,2,0,0,'buru EMOTE_TARGET'),

(-1509003,'They come now. Try not to get yourself killed, young blood.',0,1,0,22,'andorov SAY_ANDOROV_INTRO_3'),
(-1509004,'Remember, Rajaxx, when I said I\'d kill you last?',0,1,0,0,'andorov SAY_ANDOROV_INTRO_1'),

(-1509005,'The time of our retribution is at hand! Let darkness reign in the hearts of our enemies!',8612,1,0,0,'rajaxx SAY_WAVE3'),
(-1509006,'No longer will we wait behind barred doors and walls of stone! No longer will our vengeance be denied! The dragons themselves will tremble before our wrath!',8610,1,0,0,'rajaxx SAY_WAVE4'),
(-1509007,'Fear is for the enemy! Fear and death!',8608,1,0,0,'rajaxx SAY_WAVE5'),
(-1509008,'Staghelm will whimper and beg for his life, just as his whelp of a son did! One thousand years of injustice will end this day!',8611,1,0,0,'rajaxx SAY_WAVE6'),
(-1509009,'Fandral! Your time has come! Go and hide in the Emerald Dream and pray we never find you!',8607,1,0,0,'rajaxx SAY_WAVE7'),
(-1509010,'Impudent fool! I will kill you myself!',8609,1,0,0,'rajaxx SAY_INTRO'),
(-1509011,'Attack and make them pay dearly!',8603,1,0,0,'rajaxx SAY_UNK1'),
(-1509012,'Crush them! Drive them out!',8605,1,0,0,'rajaxx SAY_UNK2'),
(-1509013,'Do not hesitate! Destroy them!',8606,1,0,0,'rajaxx SAY_UNK3'),
(-1509014,'Warriors! Captains! Continue the fight!',8613,1,0,0,'rajaxx SAY_UNK4'),
(-1509015,'You are not worth my time $N!',8614,1,0,0,'rajaxx SAY_DEAGGRO'),
(-1509016,'Breathe your last!',8604,1,0,0,'rajaxx SAY_KILLS_ANDOROV'),
(-1509017,'Soon you will know the price of your meddling, mortals... The master is nearly whole... And when he rises, your world will cease!',0,1,0,0,'rajaxx SAY_COMPLETE_QUEST'),

(-1509018,'I am rejuvenated!',8593,1,0,0,'ossirian SAY_SURPREME1'),
(-1509019,'My powers are renewed!',8595,1,0,0,'ossirian SAY_SURPREME2'),
(-1509020,'My powers return!',8596,1,0,0,'ossirian SAY_SURPREME3'),
(-1509021,'Protect the city at all costs!',8597,1,0,0,'ossirian SAY_RAND_INTRO1'),
(-1509022,'The walls have been breached!',8599,6,0,0,'ossirian SAY_RAND_INTRO2'),
(-1509023,'To your posts. Defend the city.',8600,1,0,0,'ossirian SAY_RAND_INTRO3'),
(-1509024,'Tresspassers will be terminated.',8601,1,0,0,'ossirian SAY_RAND_INTRO4'),
(-1509025,'Sands of the desert rise and block out the sun!',8598,1,0,0,'ossirian SAY_AGGRO'),
(-1509026,'You are terminated.',8602,1,0,0,'ossirian SAY_SLAY'),
(-1509027,'I...have...failed.',8594,1,0,0,'ossirian SAY_DEATH'),
-- 28 (above) = EMOTE_ENERGIZING
(-1509029,'Come get some!',0,0,0,0,'andorov SAY_ANDOROV_INTRO_4'),
(-1509030,'Kill first, ask questions later... Incoming!',0,1,0,0,'andorov SAY_ANDOROV_ATTACK_START'),
(-1509031,'I lied...',0,1,0,0,'andorov SAY_ANDOROV_INTRO_2');

-- -1 531 000 TEMPLE OF AHN'QIRAJ
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1531000,'Are you so eager to die? I would be happy to accomodate you.',8615,1,0,0,'skeram SAY_AGGRO'),
(-1531001,'Let your death serve as an example!',8617,1,0,0,'skeram SAY_SLAY'),
(-1531002,'You only delay... the inevitable.',8622,1,0,0,'skeram SAY_DEATH'),

(-1531003,'REUSE_ME',0,0,0,0,'REUSE'),
(-1531004,'REUSE_ME',0,0,0,0,'REUSE'),
(-1531005,'REUSE_ME',0,0,0,0,'REUSE'),
(-1531006,'REUSE_ME',0,0,0,0,'REUSE'),
(-1531007,'REUSE_ME',0,0,0,0,'REUSE'),

(-1531008,'You will be judged for defiling these sacred grounds! The laws of the Ancients will not be challenged! Trespassers will be annihilated!',8646,1,0,0,'sartura SAY_AGGRO'),
(-1531009,'I sentence you to death!',8647,1,0,0,'sartura SAY_SLAY'),
(-1531010,'I serve to the last!',8648,1,0,0,'sartura SAY_DEATH'),

(-1531011,'%s is weakened!',0,2,0,0,'cthun EMOTE_WEAKENED'),

(-1531012,'The massive floating eyeball in the center of the chamber turns its gaze upon you. You stand before a god.',0,7,0,0,'eye cthun EMOTE_INTRO'),
(-1531013,'Only flesh and bone. Mortals are such easy prey...',0,1,0,1,'veklor SAY_INTRO_1'),
(-1531014,'Where are your manners, brother. Let us properly welcome our guests.',0,1,0,5,'veknilash SAY_INTRO_2'),
(-1531015,'There will be pain...',0,1,0,0,'veklor SAY_INTRO_3'),
(-1531016,'Oh so much pain...',0,1,0,5,'veknilash SAY_INTRO_4'),
(-1531017,'Come, little ones.',0,1,0,15,'veklor SAY_INTRO_5'),
(-1531018,'The feast of souls begins now...',0,1,0,15,'veknilash SAY_INTRO_6'),

(-1531019,'It\'s too late to turn away.',8623,1,0,0,'veklor SAY_AGGRO_1'),
(-1531020,'Prepare to embrace oblivion!',8626,1,0,0,'veklor SAY_AGGRO_2'),
(-1531021,'Like a fly in a web.',8624,1,0,0,'veklor SAY_AGGRO_3'),
(-1531022,'Your brash arrogance!',8628,1,0,0,'veklor SAY_AGGRO_4'),
(-1531023,'You will not escape death!',8629,1,0,0,'veklor SAY_SLAY'),
(-1531024,'My brother...NO!',8625,1,0,0,'veklor SAY_DEATH'),
(-1531025,'To decorate our halls!',8627,1,0,0,'veklor SAY_SPECIAL'),

(-1531026,'Ah, lambs to the slaughter!',8630,1,0,0,'veknilash SAY_AGGRO_1'),
(-1531027,'Let none survive!',8632,1,0,0,'veknilash SAY_AGGRO_2'),
(-1531028,'Join me brother, there is blood to be shed!',8631,1,0,0,'veknilash SAY_AGGRO_3'),
(-1531029,'Look brother, fresh blood!',8633,1,0,0,'veknilash SAY_AGGRO_4'),
(-1531030,'Your fate is sealed!',8635,1,0,0,'veknilash SAY_SLAY'),
(-1531031,'Vek\'lor, I feel your pain!',8636,1,0,0,'veknilash SAY_DEATH'),
(-1531032,'Shall be your undoing!',8634,1,0,0,'veknilash SAY_SPECIAL'),

(-1531033,'Death is close...',8580,4,0,0,'cthun SAY_WHISPER_1'),
(-1531034,'You are already dead.',8581,4,0,0,'cthun SAY_WHISPER_2'),
(-1531035,'Your courage will fail.',8582,4,0,0,'cthun SAY_WHISPER_3'),
(-1531036,'Your friends will abandon you.',8583,4,0,0,'cthun SAY_WHISPER_4'),
(-1531037,'You will betray your friends.',8584,4,0,0,'cthun SAY_WHISPER_5'),
(-1531038,'You will die.',8585,4,0,0,'cthun SAY_WHISPER_6'),
(-1531039,'You are weak.',8586,4,0,0,'cthun SAY_WHISPER_7'),
(-1531040,'Your heart will explode.',8587,4,0,0,'cthun SAY_WHISPER_8'),

(-1531041,'%s begins to slow!',0,2,0,0,'viscidus EMOTE_SLOW'),
(-1531042,'%s is freezing up!',0,2,0,0,'viscidus EMOTE_FREEZE'),
(-1531043,'%s is frozen solid!',0,2,0,0,'viscidus EMOTE_FROZEN'),
(-1531044,'%s begins to crack!',0,2,0,0,'viscidus EMOTE_CRACK'),
(-1531045,'%s looks ready to shatter!',0,2,0,0,'viscidus EMOTE_SHATTER'),
(-1531046,'%s explodes!',0,2,0,0,'viscidus EMOTE_EXPLODE'),

(-1531047,'%s shares his powers with his brethren.',0,2,0,0,'anubisath sentinelle EMOTE_SHARE'),

(-1531048,'%s is devoured!',0,2,0,0,'silithid royalty EMOTE_CONSUMED');

-- -1 533 000 NAXXRAMAS
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1533000,'Ahh... welcome to my parlor.',8788,1,0,0,'anubrekhan SAY_GREET1'),
(-1533001,'Just a little taste...',8785,1,0,0,'anubrekhan SAY_AGGRO1'),
(-1533002,'There is no way out.',8786,1,0,0,'anubrekhan SAY_AGGRO2'),
(-1533003,'Yes, run! It makes the blood pump faster!',8787,1,0,0,'anubrekhan SAY_AGGRO3'),
(-1533004,'I hear little hearts beating. Yesss... beating faster now. Soon the beating will stop.',8790,1,0,0,'anubrekhan SAY_GREET2'),
(-1533005,'Where to go? What to do? So many choices that all end in pain, end in death.',8791,1,0,0,'anubrekhan SAY_GREET3'),
(-1533006,'Which one shall I eat first? So difficult to choose. They all smell so delicious...',8792,1,0,0,'anubrekhan SAY_GREET4'),
(-1533007,'Closer now... tasty morsels. I\'ve been too long without food. Without blood to drink.',8793,1,0,0,'anubrekhan SAY_GREET5'),
(-1533008,'Shhh... it will all be over soon.',8789,1,0,0,'anubrekhan SAY_SLAY'),

(-1533009,'Your old lives, your mortal desires, mean nothing. You are acolytes of the master now, and you will serve the cause without question! The greatest glory is to die in the master\'s service!',8799,1,0,0,'faerlina SAY_GREET'),
(-1533010,'Slay them in the master\'s name!',8794,1,0,0,'faerlina SAY_AGGRO1'),
(-1533011,'You cannot hide from me!',8795,1,0,0,'faerlina SAY_ENRAGE_1'),
(-1533012,'Kneel before me, worm!',8796,1,0,0,'faerlina SAY_ENRAGE_2'),
(-1533013,'Run while you still can!',8797,1,0,0,'faerlina SAY_ENRAGE_3'),
(-1533014,'You have failed!',8800,1,0,0,'faerlina SAY_SLAY1'),
(-1533015,'Pathetic wretch!',8801,1,0,0,'faerlina SAY_SLAY2'),
(-1533016,'The master... will avenge me!',8798,1,0,0,'faerlina SAY_DEATH'),

(-1533017,'Patchwerk want to play!',8909,1,0,0,'patchwerk SAY_AGGRO1'),
(-1533018,'Kel\'Thuzad make Patchwerk his Avatar of War!',8910,1,0,0,'patchwerk SAY_AGGRO2'),
(-1533019,'No more play?',8912,1,0,0,'patchwerk SAY_SLAY'),
(-1533020,'What happened to... Patch...',8911,1,0,0,'patchwerk SAY_DEATH'),

(-1533021,'%s sprays slime across the room!',0,3,0,0,'grobbulus EMOTE_SPRAY_SLIME'),

(-1533022,'%s lifts off into the air!',0,3,0,0,'sapphiron EMOTE_FLY'),

(-1533023,'Stalagg crush you!',8864,1,0,0,'stalagg SAY_STAL_AGGRO'),
(-1533024,'Stalagg kill!',8866,1,0,0,'stalagg SAY_STAL_SLAY'),
(-1533025,'Master save me...',8865,1,0,0,'stalagg SAY_STAL_DEATH'),
(-1533026,'Feed you to master!',8802,1,0,0,'feugen SAY_FEUG_AGGRO'),
(-1533027,'Feugen make master happy!',8804,1,0,0,'feugen SAY_FEUG_SLAY'),
(-1533028,'No... more... Feugen...',8803,1,0,0,'feugen SAY_FEUG_DEATH'),

(-1533029,'You are too late... I... must... OBEY!',8872,1,0,0,'thaddius SAY_GREET'),
(-1533030,'KILL!',8867,1,0,0,'thaddius SAY_AGGRO1'),
(-1533031,'EAT YOUR BONES!',8868,1,0,0,'thaddius SAY_AGGRO2'),
(-1533032,'BREAK YOU!',8869,1,0,0,'thaddius SAY_AGGRO3'),
(-1533033,'You die now!',8877,1,0,0,'thaddius SAY_SLAY'),
(-1533034,'Now YOU feel pain!',8871,1,0,0,'thaddius SAY_ELECT'),
(-1533035,'Thank... you...',8870,1,0,0,'thaddius SAY_DEATH'),
(-1533036,'REUSE_ME',0,0,0,0,'REUSE'),
(-1533037,'REUSE_ME',0,0,0,0,'REUSE'),
(-1533038,'REUSE_ME',0,0,0,0,'REUSE'),
(-1533039,'REUSE_ME',0,0,0,0,'REUSE'),

(-1533040,'Foolishly you have sought your own demise.',8807,1,0,0,'gothik SAY_SPEECH_1'),
(-1533041,'Death is the only escape.',8806,1,0,0,'gothik SAY_KILL'),
(-1533042,'I... am... undone!',8805,1,0,0,'gothik SAY_DEATH'),
(-1533043,'I have waited long enough! Now, you face the harvester of souls!',8808,1,0,0,'gothik SAY_TELEPORT'),

(-1533044,'Defend yourself!',8892,0,0,0,'blaumeux SAY_BLAU_AGGRO'),
(-1533045,'Come, Zeliek, do not drive them out. Not before we\'ve had our fun.',8896,6,0,0,'blaumeux SAY_BLAU_TAUNT1'),
(-1533046,'I do hope they stay alive long enough for me to... introduce myself.',8897,6,0,0,'blaumeux SAY_BLAU_TAUNT2'),
(-1533047,'The first kill goes to me! Anyone care to wager?',8898,6,0,0,'blaumeux SAY_BLAU_TAUNT3'),
(-1533048,'Your life is mine!',8895,1,0,0,'blaumeux SAY_BLAU_SPECIAL'),
(-1533049,'Who\'s next?',8894,0,0,0,'blaumeux SAY_BLAU_SLAY'),
(-1533050,'Tou... che!',8893,0,0,0,'blaumeux SAY_BLAU_DEATH'),

(-1533051,'Come out and fight, ye wee ninny!',8899,0,0,0,'korthazz SAY_KORT_AGGRO'),
(-1533052,'To arms, ye roustabouts! We\'ve got company!',8903,6,0,0,'korthazz SAY_KORT_TAUNT1'),
(-1533053,'I heard about enough of yer sniveling. Shut yer fly trap \'afore I shut it for ye!',8904,6,0,0,'korthazz SAY_KORT_TAUNT2'),
(-1533054,'I\'m gonna enjoy killin\' these slack-jawed daffodils!',8905,6,0,0,'korthazz SAY_KORT_TAUNT3'),
(-1533055,'I like my meat extra crispy!',8902,0,0,0,'korthazz SAY_KORT_SPECIAL'),
(-1533056,'Next time, bring more friends!',8901,0,0,0,'korthazz SAY_KORT_SLAY'),
(-1533057,'What a bloody waste this is!',8900,0,0,0,'korthazz SAY_KORT_DEATH'),

(-1533058,'Flee, before it\'s too late!',8913,0,0,0,'zeliek SAY_ZELI_AGGRO'),
(-1533059,'Invaders, cease this foolish venture at once! Turn away while you still can!',8917,6,0,0,'zeliek SAY_ZELI_TAUNT1'),
(-1533060,'Perhaps they will come to their senses, and run away as fast as they can!',8918,6,0,0,'zeliek SAY_ZELI_TAUNT2'),
(-1533061,'Do not continue! Turn back while there\'s still time!',8919,6,0,0,'zeliek SAY_ZELI_TAUNT3'),
(-1533062,'I- I have no choice but to obey!',8916,0,0,0,'zeliek SAY_ZELI_SPECIAL'),
(-1533063,'Forgive me!',8915,0,0,0,'zeliek SAY_ZELI_SLAY'),
(-1533064,'It is... as it should be.',8914,0,0,0,'zeliek SAY_ZELI_DEATH'),

(-1533065,'You seek death?',8835,0,0,0,'mograine SAY_MORG_AGGRO1'),
(-1533066,'None shall pass!',8836,0,0,0,'mograine SAY_MORG_AGGRO2'),
(-1533067,'Be still!',8837,0,0,0,'mograine SAY_MORG_AGGRO3'),
(-1533068,'You will find no peace in death.',8839,0,0,0,'mograine SAY_MORG_SLAY1'),
(-1533069,'The master\'s will is done.',8840,0,0,0,'mograine SAY_MORG_SLAY2'),
(-1533070,'Bow to the might of the Highlord!',8841,0,0,0,'mograine SAY_MORG_SPECIAL'),
(-1533071,'Enough prattling. Let them come! We shall grind their bones to dust.',8842,6,0,0,'mograine SAY_MORG_TAUNT1'),
(-1533072,'Conserve your anger! Harness your rage! You will all have outlets for your frustration soon enough.',8843,6,0,0,'mograine SAY_MORG_TAUNT2'),
(-1533073,'Life is meaningless. It is in death that we are truly tested.',8844,6,0,0,'mograine SAY_MORG_TAUNT3'),
(-1533074,'I...am...released! Perhaps it\'s not too late to-NOO! I need...more time...',8838,0,0,0,'mograine SAY_MORG_DEATH'),

(-1533075,'Glory to the master!',8845,1,0,0,'noth SAY_AGGRO1'),
(-1533076,'Your life is forfeit!',8846,1,0,0,'noth SAY_AGGRO2'),
(-1533077,'Die, trespasser!',8847,1,0,0,'noth SAY_AGGRO3'),
(-1533078,'Rise, my soldiers! Rise and fight once more!',8851,1,0,0,'noth SAY_SUMMON'),
(-1533079,'My task is done!',8849,1,0,0,'noth SAY_SLAY1'),
(-1533080,'Breathe no more!',8850,1,0,0,'noth SAY_SLAY2'),
(-1533081,'I will serve the master... in... death!',8848,1,0,0,'noth SAY_DEATH'),

(-1533082,'%s takes in a deep breath...',0,3,0,0,'sapphiron EMOTE_BREATH'),
(-1533083,'%s resumes his attacks!',0,3,0,0,'sapphiron EMOTE_GROUND'),

(-1533084,'Our preparations continue as planned master.',8878,6,0,0,'kelthuzad SAY_SAPP_DIALOG1'),
(-1533085,'It is good that you serve me so faithfully. Soon... all will serve the Lich King and in the end, you shall be rewarded. So long as you do not falter.',8881,6,0,0,'lich_king SAY_SAPP_DIALOG2_LICH'),
(-1533086,'I foresee no complications at this... wait! What is this!?',8879,6,0,0,'kelthuzad SAY_SAPP_DIALOG3'),
(-1533087,'Your security measures have failed! See to this interruption immediately!',8882,6,0,0,'lich_king SAY_SAPP_DIALOG4_LICH'),
(-1533088,'Yes, master!',8880,6,0,0,'kelthuzad SAY_SAPP_DIALOG5'),
(-1533089,'No!!! A curse upon you, interlopers! The armies of the Lich King will hunt you down. You will not escape your fate...',0,6,0,0,'kelthuzad SAY_CAT_DIED'),
(-1533090,'Who dares violate the sanctity of my domain? Be warned, all who trespass here are doomed.',8820,6,0,0,'kelthuzad SAY_TAUNT1'),
(-1533091,'Fools, you think yourselves triumphant? You have only taken one step closer to the abyss! ',8821,6,0,0,'kelthuzad SAY_TAUNT2'),
(-1533092,'I grow tired of these games. Proceed, and I will banish your souls to oblivion!',8822,6,0,0,'kelthuzad SAY_TAUNT3'),
(-1533093,'You have no idea what horrors lie ahead. You have seen nothing! The frozen heart of Naxxramas awaits you!',8823,6,0,0,'kelthuzad SAY_TAUNT4'),
(-1533094,'Pray for mercy!',8809,1,0,0,'kelthuzad SAY_AGGRO1'),
(-1533095,'Scream your dying breath!',8810,1,0,0,'kelthuzad SAY_AGGRO2'),
(-1533096,'The end is upon you!',8811,1,0,0,'kelthuzad SAY_AGGRO3'),
(-1533097,'The dark void awaits you!',8817,1,0,0,'kelthuzad SAY_SLAY1'),
(-1533098,'<Kel\'Thuzad cackles maniacally!>',8818,1,0,0,'kelthuzad SAY_SLAY2'),
(-1533099,'AAAAGHHH!... Do not rejoice... your victory is a hollow one... for I shall return with powers beyond your imagining!',8814,1,0,0,'kelthuzad SAY_DEATH'),
(-1533100,'Your soul, is bound to me now!',8812,1,0,0,'kelthuzad SAY_CHAIN1'),
(-1533101,'There will be no escape!',8813,1,0,0,'kelthuzad SAY_CHAIN2'),
(-1533102,'I will freeze the blood in your veins!',8815,1,0,0,'kelthuzad SAY_FROST_BLAST'),
(-1533103,'Master! I require aid! ',8816,1,0,0,'kelthuzad SAY_REQUEST_AID'),
(-1533104,'Very well... warriors of the frozen wastes, rise up! I command you to fight, kill, and die for your master. Let none survive...',8824,1,0,0,'kelthuzad SAY_ANSWER_REQUEST'),
(-1533105,'Minions, servants, soldiers of the cold dark, obey the call of Kel\'Thuzad!',8819,1,0,0,'kelthuzad SAY_SUMMON_MINIONS'),
(-1533106,'Your petty magics are no challenge to the might of the Scourge! ',9088,1,0,0,'kelthuzad SAY_SPECIAL1_MANA_DET'),
(-1533107,'Enough! I grow tired of these distractions! ',9090,1,0,0,'kelthuzad SAY_SPECIAL3_MANA_DET'),
(-1533108,'Fools, you have spread your powers too thin. Be free, my minions!',9089,1,0,0,'kelthuzad SAY_SPECIAL2_DISPELL'),

(-1533109,'You are mine now!',8825,1,0,0,'heigan SAY_AGGRO1'),
(-1533110,'I see you!',8826,1,0,0,'heigan SAY_AGGRO2'),
(-1533111,'You...are next!',8827,1,0,0,'heigan SAY_AGGRO3'),
(-1533112,'Close your eyes... sleep!',8829,1,0,0,'heigan SAY_SLAY'),
(-1533113,'The races of the world will perish. It is only a matter of time.',8830,1,0,0,'heigan SAY_TAUNT1'),
(-1533114,'I see endless suffering, I see torment, I see rage. I see... everything!',8831,1,0,0,'heigan SAY_TAUNT2'),
(-1533115,'Soon... the world will tremble!',8832,1,0,0,'heigan SAY_TAUNT3'),
(-1533116,'The end is upon you.',8833,1,0,0,'heigan SAY_CHANNELING'),
(-1533117,'Hungry worms will feast on your rotten flesh!',8834,1,0,0,'heigan SAY_TAUNT4'),
(-1533118,'Noo... o...',8828,1,0,0,'heigan SAY_DEATH'),

(-1533119,'%s spots a nearby Zombie to devour!',0,3,0,0,'gluth EMOTE_ZOMBIE'),

(-1533120,'Do as I taught you!',8855,1,0,0,'razuvious SAY_AGGRO1'),
(-1533121,'Show them no mercy!',8856,1,0,0,'razuvious SAY_AGGRO2'),
(-1533122,'The time for practice is over! Show me what you\'ve learned!',8859,1,0,0,'razuvious SAY_AGGRO3'),
(-1533123,'Sweep the leg! Do you have a problem with that?',8861,1,0,0,'razuvious SAY_AGGRO4'),
(-1533124,'You should have stayed home!',8862,1,0,0,'razuvious SAY_SLAY1'),
(-1533125,'You disappoint me, students!',8858,1,0,0,'razuvious SAY_SLAY2'),
(-1533126,'Hah hah, I\'m just getting warmed up!',8852,1,0,0,'razuvious SAY_TRIUMPHANT1'),
(-1533127,'Stand and fight!',8853,1,0,0,'razuvious SAY_TRIUMPHANT2'),
(-1533128,'Show me what you\'ve got!',8854,1,0,0,'razuvious SAY_TRIUMPHANT3'),
(-1533129,'An honorable... death...',8860,1,0,0,'razuvious SAY_DEATH'),

(-1533130,'%s summons forth Skeletal Warriors!',0,3,0,0,'noth EMOTE_WARRIOR'),
(-1533131,'%s raises more skeletons!',0,3,0,0,'noth EMOTE_SKELETON'),
(-1533132,'%s teleports to the balcony above!',0,3,0,0,'noth EMOTE_TELEPORT'),
(-1533133,'%s teleports back into the battle!',0,3,0,0,'noth EMOTE_TELEPORT_RETURN'),

(-1533134,'A Guardian of Icecrown enters the fight!',0,3,0,0,'kelthuzad EMOTE_GUARDIAN'),
(-1533135,'%s strikes!',0,3,0,0,'kelthuzad EMOTE_PHASE2'),

(-1533136,'%s teleports and begins to channel a spell!',0,3,0,0,'heigan EMOTE_TELEPORT'),
(-1533137,'%s rushes to attack once more!',0,3,0,0,'heigan EMOTE_RETURN'),

(-1533138,'%s teleports into the fray!',0,3,0,0,'gothik EMOTE_TO_FRAY'),
(-1533139,'The central gate opens!',0,3,0,0,'gothik EMOTE_GATE'),
(-1533140,'Brazenly you have disregarded powers beyond your understanding.',0,1,0,0,'gothik SAY_SPEECH_2'),
(-1533141,'You have fought hard to invade the realm of the harvester.',0,1,0,0,'gothik SAY_SPEECH_3'),
(-1533142,'Now there is only one way out - to walk the lonely path of the damned.',0,1,0,0,'gothik SAY_SPEECH_4'),

(-1533143,'An aura of necrotic energy blocks all healing!',0,3,0,0,'Loatheb EMOTE_AURA_BLOCKING'),
(-1533144,'The power of Necrotic Aura begins to wane!',0,3,0,0,'Loatheb EMOTE_AURA_WANE'),
(-1533145,'The aura fades away, allowing healing once more!',0,3,0,0,'Loatheb EMOTE_AURA_FADING'),

(-1533146,'%s spins her web into a cocoon!',0,3,0,0,'maexxna EMOTE_SPIN_WEB'),
(-1533147,'Spiderlings appear on the web!',0,3,0,0,'maexxna EMOTE_SPIDERLING'),
(-1533148,'%s sprays strands of web everywhere!',0,3,0,0,'maexxna EMOTE_SPRAY'),

(-1533149,'%s loses its link!',0,3,0,0,'tesla_coil EMOTE_LOSING_LINK'),
(-1533150,'%s overloads!',0,3,0,0,'tesla_coil EMOTE_TESLA_OVERLOAD'),
(-1533151,'The polarity has shifted!',0,3,0,0,'thaddius EMOTE_POLARITY_SHIFT'),

(-1533152,'%s decimates all nearby flesh!',0,3,0,0,'gluth EMOTE_DECIMATE'),

(-1533153,'A %s joins the fight!',0,3,0,0,'crypt_guard EMOTE_CRYPT_GUARD'),
(-1533154,'%s begins to unleash an insect swarm!',0,3,0,0,'anubrekhan EMOTE_INSECT_SWARM'),
(-1533155,'Corpse Scarabs appear from a Crypt Guard\'s corpse!',0,3,0,0,'anubrekhan EMOTE_CORPSE_SCARABS'),

(-1533156,'%s casts Unyielding Pain on everyone!',0,3,0,0,'lady_blaumeux EMOTE_UNYIELDING_PAIN'),
(-1533157,'%s casts Condemation on everyone!',0,3,0,0,'sir_zeliek EMOTE_CONDEMATION');

-- -1 999 900 EXAMPLE TEXT
INSERT INTO script_texts (entry,content_default,sound,type,language,emote,comment) VALUES
(-1999900,'Let the games begin.',8280,1,0,0,'example_creature SAY_AGGRO'),
(-1999901,'I see endless suffering. I see torment. I see rage. I see everything.',8831,1,0,0,'example_creature SAY_RANDOM_0'),
(-1999902,'Muahahahaha',8818,1,0,0,'example_creature SAY_RANDOM_1'),
(-1999903,'These mortal infedels my lord, they have invaded your sanctum and seek to steal your secrets.',8041,1,0,0,'example_creature SAY_RANDOM_2'),
(-1999904,'You are already dead.',8581,1,0,0,'example_creature SAY_RANDOM_3'),
(-1999905,'Where to go? What to do? So many choices that all end in pain, end in death.',8791,1,0,0,'example_creature SAY_RANDOM_4'),
(-1999906,'$N, I sentance you to death!',8588,1,0,0,'example_creature SAY_BESERK'),
(-1999907,'The suffering has just begun!',0,1,0,0,'example_creature SAY_PHASE'),
(-1999908,'I always thought I was a good dancer.',0,0,0,0,'example_creature SAY_DANCE'),
(-1999909,'Move out Soldier!',0,0,0,0,'example_creature SAY_SALUTE'),

(-1999910,'Help $N! I\'m under attack!',0,0,0,0,'example_escort SAY_AGGRO1'),
(-1999911,'Die scum!',0,0,0,0,'example_escort SAY_AGGRO2'),
(-1999912,'Hmm a nice day for a walk alright',0,0,0,0,'example_escort SAY_WP_1'),
(-1999913,'Wild Felboar attack!',0,0,0,0,'example_escort SAY_WP_2'),
(-1999914,'Time for me to go! See ya around $N!',0,0,0,3,'example_escort SAY_WP_3'),
(-1999915,'Bye Bye!',0,0,0,3,'example_escort SAY_WP_4'),
(-1999916,'How dare you leave me like that! I hate you! =*(',0,3,0,0,'example_escort SAY_DEATH_1'),
(-1999917,'...no...how could you let me die $N',0,0,0,0,'example_escort SAY_DEATH_2'),
(-1999918,'ugh...',0,0,0,0,'example_escort SAY_DEATH_3'),
(-1999919,'Taste death!',0,0,0,0,'example_escort SAY_SPELL'),
(-1999920,'Fireworks!',0,0,0,0,'example_escort SAY_RAND_1'),
(-1999921,'Hmm, I think I could use a buff.',0,0,0,0,'example_escort SAY_RAND_2'),

(-1999922,'Normal select, guess you\'re not interested.',0,0,0,0,'example_gossip_codebox SAY_NOT_INTERESTED'),
(-1999923,'Wrong!',0,0,0,0,'example_gossip_codebox SAY_WRONG'),
(-1999924,'You\'re right, you are allowed to see my inner secrets.',0,0,0,0,'example_gossip_codebox SAY_CORRECT'),

(-1999925,'Hi!',0,0,0,0,'example_areatrigger SAY_HI');

--
-- GOSSIP TEXTS
--

--
-- Below contains data for table `gossip_texts`
-- valid entries for table are between -3000000 and -3999999
--

TRUNCATE gossip_texts;

-- -3 000 000 RESERVED (up to 100)
INSERT INTO gossip_texts (entry,content_default,comment) VALUES
(-3000000,'[PH] SD2 unknown text','GOSSIP_ID_UNKNOWN_TEXT');

-- -3 000 100 GENERAL MAPS (not instance maps)
INSERT INTO gossip_texts (entry,content_default,comment) VALUES
(-3000105,'Ezekiel said that you might have a certain book...','dirty larry GOSSIP_ITEM_BOOK'),
(-3000106,'Let Marshal Windsor know that I am ready.','squire rowe GOSSIP_ITEM_WINDSOR'),
(-3000107,'I am ready, as are my forces. Let us end this masquerade!','reginald windsor GOSSIP_ITEM_START'),
(-3000108,'I need a moment of your time, sir.','prospector anvilward GOSSIP_ITEM_MOMENT'),
(-3000109,'I am ready, Oronok. Let us destroy Cyrukh and free the elements!','oronok torn-heart GOSSIP_ITEM_FIGHT'),
(-3000110,'Why... yes, of course. I\'ve something to show you right inside this building, Mr. Anvilward.','prospector anvilward GOSSIP_ITEM_SHOW'),
(-3000111,'Turn the key to start the machine.','threshwackonator  GOSSIP_ITEM_TURN_KEY');

-- -3 033 000 SHADOWFANG KEEP
INSERT INTO gossip_texts (entry,content_default,comment) VALUES
(-3033000,'Please unlock the courtyard door.','deathstalker adamant/ sorcerer ashcrombe - GOSSIP_ITEM_DOOR');

-- -3 043 000 WAILING CAVERNS
INSERT INTO gossip_texts (entry,content_default,comment) VALUES
(-3043000,'Let the event begin!','Disciple of Naralex - GOSSIP_ITEM_BEGIN');

-- -3 090 000 GNOMEREGAN
INSERT INTO gossip_texts (entry,content_default,comment) VALUES
(-3090000,'I am ready to begin.','emi shortfuse GOSSIP_ITEM_START');

-- -3 230 000 BLACKROCK DEPTHS
INSERT INTO gossip_texts (entry,content_default,comment) VALUES
(-3230000,'You\'re free, Dughal! Get out of here!','dughal GOSSIP_ITEM_DUGHAL'),
(-3230001,'Get out of here, Tobias, you\'re free!','tobias GOSSIP_ITEM_TOBIAS'),
(-3230002,'Your bondage is at an end, Doom\'rel. I challenge you!','doomrel GOSSIP_ITEM_CHALLENGE'),
(-3230003,'Why don\'t you and Rocknot go find somewhere private...','nagmara GOSSIP_ITEM_NAGMARA');

-- -3 409 000 MOLTEN CORE
INSERT INTO gossip_texts (entry,content_default,comment) VALUES
(-3409000,'Tell me more.','majordomo_executus GOSSIP_ITEM_SUMMON_1'),
(-3409001,'What else do you have to say?','majordomo_executus GOSSIP_ITEM_SUMMON_2'),
(-3409002,'You challenged us and we have come. Where is this master you speak of?','majordomo_executus GOSSIP_ITEM_SUMMON_3');

-- -3 469 000 BLACKWING LAIR
INSERT INTO gossip_texts (entry,content_default,comment) VALUES
(-3469000,'I\'ve made no mistakes.','victor_nefarius GOSSIP_ITEM_NEFARIUS_1'),
(-3469001,'You have lost your mind, Nefarius. You speak in riddles.','victor_nefarius GOSSIP_ITEM_NEFARIUS_2'),
(-3469002,'Please do.','victor_nefarius GOSSIP_ITEM_NEFARIUS_3'),

(-3469003,'I cannot, Vaelastrasz! Surely something can be done to heal you!','vaelastrasz GOSSIP_ITEM_VAEL_1'),
(-3469004,'Vaelastrasz, no!!!','vaelastrasz GOSSIP_ITEM_VAEL_2');

-- -3 509 000 RUINS OF AHN'QIRAJ
INSERT INTO gossip_texts (entry,content_default,comment) VALUES
(-3509000,'Let\'s find out.','andorov GOSSIP_ITEM_START'),
(-3509001,'Let\'s see what you have.','andorov GOSSIP_ITEM_TRADE');

--
-- Below just for beautiful view in table, run at own desire
--

-- ALTER TABLE script_texts ORDER BY entry desc;
-- ALTER TABLE gossip_texts ORDER BY entry desc;

--
-- Below contains all waypoints used by escortAI scripts
-- Entry is entry == creature_template.entry
--

TRUNCATE script_waypoint;
INSERT INTO script_waypoint VALUES
(349,0,1,-8769.59,-2185.73,141.975,0,0,0,''),
(349,0,2,-8776.54,-2193.78,140.96,0,0,0,''),
(349,0,3,-8783.29,-2194.82,140.462,0,0,0,''),
(349,0,4,-8792.52,-2188.8,142.078,0,0,0,''),
(349,0,5,-8807.55,-2186.1,141.504,0,0,0,''),
(349,0,6,-8818,-2184.8,139.153,0,0,0,''),
(349,0,7,-8825.81,-2188.84,138.459,0,0,0,''),
(349,0,8,-8827.52,-2199.81,139.622,0,0,0,''),
(349,0,9,-8821.14,-2212.64,143.126,0,0,0,''),
(349,0,10,-8809.18,-2230.46,143.438,0,0,0,''),
(349,0,11,-8797.04,-2240.72,146.548,0,0,0,''),
(349,0,12,-8795.24,-2251.81,146.808,0,0,0,''),
(349,0,13,-8780.16,-2258.62,148.554,0,0,0,''),
(349,0,14,-8762.65,-2259.56,151.144,0,0,0,''),
(349,0,15,-8754.36,-2253.74,152.243,0,0,0,''),
(349,0,16,-8741.87,-2251,154.486,0,0,0,''),
(349,0,17,-8733.22,-2251.01,154.36,0,0,0,''),
(349,0,18,-8717.47,-2245.04,154.686,0,0,0,''),
(349,0,19,-8712.24,-2246.83,154.709,0,0,0,''),
(349,0,20,-8693.84,-2240.41,152.91,0,0,0,''),
(349,0,21,-8681.82,-2245.33,155.518,0,0,0,''),
(349,0,22,-8669.86,-2252.77,154.854,0,0,0,''),
(349,0,23,-8670.56,-2264.69,156.978,0,0,0,''),
(349,0,24,-8676.56,-2269.2,155.411,0,0,0,''),
(349,0,25,-8673.34,-2288.65,157.054,0,0,0,''),
(349,0,26,-8677.76,-2302.56,155.917,0,16000,0,'Corp. Keeshan - Short Break Outside'),
(349,0,27,-8682.46,-2321.69,155.917,0,0,0,''),
(349,0,28,-8690.4,-2331.78,155.971,0,0,0,''),
(349,0,29,-8715.1,-2353.95,156.188,0,0,0,''),
(349,0,30,-8748.04,-2370.7,157.988,0,0,0,''),
(349,0,31,-8780.9,-2421.37,156.109,0,0,0,''),
(349,0,32,-8792.01,-2453.38,142.746,0,0,0,''),
(349,0,33,-8804.78,-2472.43,134.192,0,0,0,''),
(349,0,34,-8841.35,-2503.63,132.276,0,0,0,''),
(349,0,35,-8867.57,-2529.89,134.739,0,0,0,''),
(349,0,36,-8870.67,-2542.08,131.044,0,0,0,''),
(349,0,37,-8922.05,-2585.31,132.446,0,0,0,''),
(349,0,38,-8949.08,-2596.87,132.537,0,0,0,''),
(349,0,39,-8993.46,-2604.04,130.756,0,0,0,''),
(349,0,40,-9006.71,-2598.47,127.966,0,0,0,''),
(349,0,41,-9038.96,-2572.71,124.748,0,0,0,''),
(349,0,42,-9046.92,-2560.64,124.447,0,0,0,''),
(349,0,43,-9066.69,-2546.63,123.11,0,0,0,''),
(349,0,44,-9077.54,-2541.67,121.17,0,0,0,''),
(349,0,45,-9125.32,-2490.06,116.057,0,0,0,''),
(349,0,46,-9145.06,-2442.24,108.232,0,0,0,''),
(349,0,47,-9158.2,-2425.36,105.5,0,0,0,''),
(349,0,48,-9151.92,-2393.67,100.856,0,0,0,''),
(349,0,49,-9165.19,-2376.03,94.8215,0,0,0,''),
(349,0,50,-9187.1,-2360.52,89.9231,0,0,0,''),
(349,0,51,-9235.44,-2305.24,77.9253,0,0,0,''),
(349,0,52,-9264.73,-2292.92,70.0089,0,0,0,''),
(349,0,53,-9277.47,-2296.19,68.0896,0,2500,0,'Corp. Keeshan - quest-finish'),
(349,0,54,-9277.47,-2296.19,68.0896,0,0,0,'Corp. Keeshan - Say Goodbye'),
(467,0,0,-10508.4,1068,55.21,0,0,0,''),
(467,0,1,-10518.3,1074.84,53.96,0,0,0,''),
(467,0,2,-10534.8,1081.92,49.88,0,0,0,''),
(467,0,3,-10546.5,1084.88,50.13,0,0,0,''),
(467,0,4,-10555.3,1084.45,45.75,0,0,0,''),
(467,0,5,-10566.6,1083.53,42.1,0,0,0,''),
(467,0,6,-10575.8,1082.34,39.46,0,0,0,''),
(467,0,7,-10585.7,1081.08,37.77,0,0,0,''),
(467,0,8,-10600.1,1078.19,36.23,0,0,0,''),
(467,0,9,-10608.7,1076.08,35.88,0,0,0,''),
(467,0,10,-10621.3,1073,35.4,0,0,0,''),
(467,0,11,-10638.1,1060.18,33.61,0,0,0,''),
(467,0,12,-10655.9,1038.99,33.48,0,0,0,''),
(467,0,13,-10664.7,1030.54,32.7,0,0,0,''),
(467,0,14,-10708.7,1033.86,33.32,0,0,0,''),
(467,0,15,-10754.4,1017.93,32.79,0,0,0,''),
(467,0,16,-10802.3,1018.01,32.16,0,0,0,''),
(467,0,17,-10832.6,1009.04,32.71,0,0,0,''),
(467,0,18,-10866.6,1006.51,31.71,0,0,0,''),
(467,0,19,-10880,1005.1,32.84,0,0,0,''),
(467,0,20,-10892.5,1001.32,34.46,0,0,0,''),
(467,0,21,-10906.1,997.11,36.15,0,0,0,''),
(467,0,22,-10922.3,1002.23,35.74,0,0,0,''),
(467,0,23,-10936.3,1023.38,36.52,0,0,0,''),
(467,0,24,-10933.3,1052.61,35.85,0,0,0,''),
(467,0,25,-10940.2,1077.66,36.49,0,0,0,''),
(467,0,26,-10957.1,1099.33,36.83,0,0,0,''),
(467,0,27,-10956.5,1119.9,36.73,0,0,0,''),
(467,0,28,-10939.3,1150.75,37.42,0,0,0,''),
(467,0,29,-10915.1,1202.09,36.55,0,0,0,''),
(467,0,30,-10892.6,1257.03,33.37,0,0,0,''),
(467,0,31,-10891.9,1306.66,35.45,0,0,0,''),
(467,0,32,-10896.2,1327.86,37.77,0,0,0,''),
(467,0,33,-10906,1368.05,40.91,0,0,0,''),
(467,0,34,-10910.2,1389.33,42.62,0,0,0,''),
(467,0,35,-10915.4,1417.72,42.93,0,0,0,''),
(467,0,36,-10926.4,1421.18,43.04,0,0,0,'walk here and say'),
(467,0,37,-10952.3,1421.74,43.4,0,0,0,''),
(467,0,38,-10980,1411.38,42.79,0,0,0,''),
(467,0,39,-11006.1,1420.47,43.26,0,0,0,''),
(467,0,40,-11022,1450.59,43.09,0,0,0,''),
(467,0,41,-11025.4,1491.59,43.15,0,0,0,''),
(467,0,42,-11036.1,1508.32,43.28,0,0,0,''),
(467,0,43,-11060.7,1526.72,43.19,0,0,0,''),
(467,0,44,-11072.8,1527.77,43.2,0,5000,0,'say and quest credit'),
(1379,0,1,-5751.12,-3441.01,301.743,0,0,0,''),
(1379,0,2,-5738.58,-3485.14,302.41,0,0,0,''),
(1379,0,3,-5721.62,-3507.85,304.011,0,0,0,''),
(1379,0,4,-5710.21,-3527.97,304.708,0,0,0,''),
(1379,0,5,-5706.92,-3542.89,304.871,0,0,0,''),
(1379,0,6,-5701.53,-3551.24,305.962,0,0,0,''),
(1379,0,7,-5699.53,-3555.69,306.505,0,0,0,''),
(1379,0,8,-5690.56,-3571.98,309.035,0,0,0,''),
(1379,0,9,-5678.61,-3587.17,310.607,0,0,0,''),
(1379,0,10,-5677.05,-3594.35,311.527,0,0,0,''),
(1379,0,11,-5674.39,-3605.19,312.239,0,0,0,''),
(1379,0,12,-5674.45,-3614.39,312.337,0,0,0,''),
(1379,0,13,-5673.05,-3630.56,311.105,0,0,0,''),
(1379,0,14,-5680.34,-3645.44,315.185,0,0,0,''),
(1379,0,15,-5684.46,-3650.05,314.687,0,0,0,''),
(1379,0,16,-5693.9,-3674.14,313.03,0,0,0,''),
(1379,0,17,-5701.43,-3712.54,313.959,0,0,0,''),
(1379,0,18,-5698.79,-3720.88,316.943,0,0,0,''),
(1379,0,19,-5699.95,-3733.63,318.597,0,0,0,'Protecting the Shipment - Ambush'),
(1379,0,20,-5698.61,-3754.74,322.047,0,0,0,''),
(1379,0,21,-5688.68,-3769,323.957,0,0,0,''),
(1379,0,22,-5688.14,-3782.65,322.667,0,0,0,''),
(1379,0,23,-5699.23,-3792.65,322.448,0,30000,0,'Protecting the Shipment - End'),
(1379,0,24,-5700.8,-3792.78,322.588,0,0,0,''),
(1840,0,0,2689.68,-1937.47,72.14,0,0,0,''),
(1840,0,1,2683.11,-1926.82,72.14,0,0,0,''),
(1840,0,2,2678.73,-1919.42,68.86,0,0,0,'escort paused'),
(1842,0,0,2941.75,-1391.82,167.237,0,0,0,'SAY_ESCORT_START'),
(1842,0,1,2940.56,-1393.64,165.943,0,0,0,''),
(1842,0,2,2932.19,-1410.66,165.943,0,0,0,''),
(1842,0,3,2921.81,-1405.09,165.943,0,0,0,''),
(1842,0,4,2916.48,-1402.58,165.943,0,0,0,''),
(1842,0,5,2918.52,-1398.12,165.943,0,0,0,''),
(1842,0,6,2922.8,-1389.49,160.842,0,0,0,''),
(1842,0,7,2924.93,-1385.65,160.842,0,0,0,''),
(1842,0,8,2930.93,-1388.65,160.842,0,0,0,''),
(1842,0,9,2946.7,-1396.65,160.842,0,0,0,''),
(1842,0,10,2948.72,-1392.79,160.842,0,0,0,''),
(1842,0,11,2951.98,-1386.62,155.948,0,0,0,''),
(1842,0,12,2953.84,-1383.33,155.948,0,0,0,''),
(1842,0,13,2951.19,-1381.74,155.948,0,0,0,''),
(1842,0,14,2946.68,-1379.29,152.02,0,0,0,''),
(1842,0,15,2942.79,-1377.66,152.02,0,0,0,''),
(1842,0,16,2935.49,-1392.52,152.02,0,0,0,''),
(1842,0,17,2921.17,-1384.8,152.02,0,0,0,''),
(1842,0,18,2915.33,-1395.35,152.02,0,0,0,''),
(1842,0,19,2926.25,-1401.26,152.028,0,0,0,''),
(1842,0,20,2930.32,-1403.48,150.521,0,0,0,''),
(1842,0,21,2933.94,-1405.36,150.521,0,0,0,''),
(1842,0,22,2929.22,-1415.79,150.504,0,0,0,''),
(1842,0,23,2921.17,-1431.68,150.781,0,0,0,''),
(1842,0,24,2917.47,-1438.78,150.781,0,0,0,''),
(1842,0,25,2913.05,-1453.52,148.098,0,0,0,'SAY_TAELAN_MOUNT'),
(1842,0,26,2913.83,-1474.93,146.224,0,0,0,''),
(1842,0,27,2906.81,-1487.06,146.224,0,0,0,''),
(1842,0,28,2900.64,-1496.57,146.306,0,0,0,''),
(1842,0,29,2885.25,-1501.58,146.02,0,0,0,''),
(1842,0,30,2863.88,-1500.38,146.681,0,0,0,''),
(1842,0,31,2846.51,-1487.18,146.332,0,0,0,''),
(1842,0,32,2823.75,-1490.99,145.782,0,0,0,''),
(1842,0,33,2800.98,-1510.91,145.049,0,0,0,''),
(1842,0,34,2789.49,-1525.21,143.729,0,0,0,''),
(1842,0,35,2776.96,-1542.31,139.435,0,0,0,''),
(1842,0,36,2762.03,-1561.8,133.763,0,0,0,''),
(1842,0,37,2758.74,-1569.6,131.514,0,0,0,''),
(1842,0,38,2765.49,-1588.79,129.721,0,0,0,''),
(1842,0,39,2779.61,-1613.12,129.132,0,0,0,''),
(1842,0,40,2757.65,-1638.03,128.236,0,0,0,''),
(1842,0,41,2741.31,-1659.79,126.457,0,0,0,''),
(1842,0,42,2729.8,-1677.57,126.499,0,0,0,''),
(1842,0,43,2716.78,-1694.65,126.301,0,0,0,''),
(1842,0,44,2706.66,-1709.47,123.42,0,0,0,''),
(1842,0,45,2699.51,-1720.57,120.265,0,0,0,''),
(1842,0,46,2691.98,-1738.47,114.994,0,0,0,''),
(1842,0,47,2690.51,-1757.05,108.764,0,0,0,''),
(1842,0,48,2691.95,-1780.31,99.89,0,0,0,''),
(1842,0,49,2689.34,-1803.26,89.13,0,0,0,''),
(1842,0,50,2697.85,-1820.55,80.681,0,0,0,''),
(1842,0,51,2701.93,-1836.71,73.7,0,0,0,''),
(1842,0,52,2698.09,-1853.87,68.999,0,0,0,''),
(1842,0,53,2693.66,-1870.24,66.882,0,0,0,''),
(1842,0,54,2682.35,-1885.25,66.009,0,0,0,''),
(1842,0,55,2668.23,-1900.8,66.256,0,0,0,'SAY_REACH_TOWER - escort paused'),
(1978,0,0,1406.32,1083.1,52.55,0,0,0,''),
(1978,0,1,1400.49,1080.42,52.5,0,0,0,'SAY_START_2'),
(1978,0,2,1388.48,1083.1,52.52,0,0,0,''),
(1978,0,3,1370.16,1084.02,52.3,0,0,0,''),
(1978,0,4,1359.02,1080.85,52.46,0,0,0,''),
(1978,0,5,1341.43,1087.39,52.69,0,0,0,''),
(1978,0,6,1321.93,1090.51,50.66,0,0,0,''),
(1978,0,7,1312.98,1095.91,47.49,0,0,0,''),
(1978,0,8,1301.09,1102.94,47.76,0,0,0,''),
(1978,0,9,1297.73,1106.35,50.18,0,0,0,''),
(1978,0,10,1295.49,1124.32,50.49,0,0,0,''),
(1978,0,11,1294.84,1137.25,51.75,0,0,0,''),
(1978,0,12,1292.89,1158.99,52.65,0,0,0,''),
(1978,0,13,1290.75,1168.67,52.56,0,2000,0,'quest complete SAY_END'),
(1978,0,14,1287.12,1203.49,52.66,0,5000,0,'SAY_RANE'),
(1978,0,15,1288.3,1203.89,52.68,0,5000,0,'SAY_RANE_REPLY'),
(1978,0,16,1288.3,1203.89,52.68,0,5000,0,'SAY_CHECK_NEXT'),
(1978,0,17,1290.72,1207.44,52.69,0,0,0,''),
(1978,0,18,1297.5,1207.18,53.74,0,0,0,''),
(1978,0,19,1301.32,1220.9,53.74,0,0,0,''),
(1978,0,20,1298.55,1220.43,53.74,0,0,0,''),
(1978,0,21,1297.38,1212.87,58.51,0,0,0,''),
(1978,0,22,1297.8,1210.04,58.51,0,0,0,''),
(1978,0,23,1305.01,1206.1,58.51,0,0,0,''),
(1978,0,24,1310.51,1207.36,58.51,0,5000,0,'SAY_QUINN'),
(1978,0,25,1312.59,1207.21,58.51,0,5000,0,'SAY_QUINN_REPLY'),
(1978,0,26,1312.59,1207.21,58.51,0,30000,0,'SAY_BYE'),
(2713,0,0,-1416.91,-3044.12,36.21,0,0,0,''),
(2713,0,1,-1408.43,-3051.35,37.79,0,0,0,''),
(2713,0,2,-1399.45,-3069.2,31.25,0,0,0,''),
(2713,0,3,-1400.28,-3083.14,27.06,0,0,0,''),
(2713,0,4,-1405.3,-3096.72,26.36,0,0,0,''),
(2713,0,5,-1406.12,-3105.95,24.82,0,0,0,''),
(2713,0,6,-1417.41,-3106.8,16.61,0,0,0,''),
(2713,0,7,-1433.06,-3101.55,12.56,0,0,0,''),
(2713,0,8,-1439.86,-3086.36,12.29,0,0,0,''),
(2713,0,9,-1450.48,-3065.16,12.58,0,5000,0,'SAY_REACH_BOTTOM'),
(2713,0,10,-1456.15,-3055.53,12.54,0,0,0,''),
(2713,0,11,-1459.41,-3035.16,12.11,0,0,0,''),
(2713,0,12,-1472.47,-3034.18,12.44,0,0,0,''),
(2713,0,13,-1495.57,-3034.48,12.55,0,0,0,''),
(2713,0,14,-1524.91,-3035.47,13.15,0,0,0,''),
(2713,0,15,-1549.05,-3037.77,12.98,0,0,0,''),
(2713,0,16,-1555.69,-3028.02,13.64,0,3000,0,'SAY_WATCH_BACK'),
(2713,0,17,-1555.69,-3028.02,13.64,0,5000,0,'SAY_DATA_FOUND'),
(2713,0,18,-1555.69,-3028.02,13.64,0,2000,0,'SAY_ESCAPE'),
(2713,0,19,-1551.19,-3037.78,12.96,0,0,0,''),
(2713,0,20,-1584.6,-3048.77,13.67,0,0,0,''),
(2713,0,21,-1602.14,-3042.82,15.12,0,0,0,''),
(2713,0,22,-1610.68,-3027.42,17.22,0,0,0,''),
(2713,0,23,-1601.65,-3007.97,24.65,0,0,0,''),
(2713,0,24,-1581.05,-2992.32,30.85,0,0,0,''),
(2713,0,25,-1559.95,-2979.51,34.3,0,0,0,''),
(2713,0,26,-1536.51,-2969.78,32.64,0,0,0,''),
(2713,0,27,-1511.81,-2961.09,29.12,0,0,0,''),
(2713,0,28,-1484.83,-2960.87,32.54,0,0,0,''),
(2713,0,29,-1458.23,-2966.8,40.52,0,0,0,''),
(2713,0,30,-1440.2,-2971.2,43.15,0,0,0,''),
(2713,0,31,-1427.85,-2989.15,38.09,0,0,0,''),
(2713,0,32,-1420.27,-3008.91,35.01,0,0,0,''),
(2713,0,33,-1427.58,-3032.53,32.31,0,5000,0,'SAY_FINISH'),
(2713,0,34,-1427.4,-3035.17,32.26,0,0,0,''),
(2768,0,0,-2077.73,-2091.17,9.49,0,0,0,''),
(2768,0,1,-2077.99,-2105.33,13.24,0,0,0,''),
(2768,0,2,-2074.6,-2109.67,14.24,0,0,0,''),
(2768,0,3,-2076.6,-2117.46,16.67,0,0,0,''),
(2768,0,4,-2073.51,-2123.46,18.42,0,2000,0,''),
(2768,0,5,-2073.51,-2123.46,18.42,0,4000,0,''),
(2768,0,6,-2066.6,-2131.85,21.56,0,0,0,''),
(2768,0,7,-2053.85,-2143.19,20.31,0,0,0,''),
(2768,0,8,-2043.49,-2153.73,20.2,0,10000,0,''),
(2768,0,9,-2043.49,-2153.73,20.2,0,20000,0,''),
(2768,0,10,-2043.49,-2153.73,20.2,0,10000,0,''),
(2768,0,11,-2043.49,-2153.73,20.2,0,2000,0,''),
(2768,0,12,-2053.85,-2143.19,20.31,0,0,0,''),
(2768,0,13,-2066.6,-2131.85,21.56,0,0,0,''),
(2768,0,14,-2073.51,-2123.46,18.42,0,0,0,''),
(2768,0,15,-2076.6,-2117.46,16.67,0,0,0,''),
(2768,0,16,-2074.6,-2109.67,14.24,0,0,0,''),
(2768,0,17,-2077.99,-2105.33,13.24,0,0,0,''),
(2768,0,18,-2077.73,-2091.17,9.49,0,0,0,''),
(2768,0,19,-2066.41,-2086.21,8.97,0,6000,0,''),
(2768,0,20,-2066.41,-2086.21,8.97,0,2000,0,''),
(2917,0,0,4675.81,598.615,17.6457,0,0,0,'SAY_REM_START'),
(2917,0,1,4672.84,599.325,16.4176,0,0,0,''),
(2917,0,2,4663.45,607.43,10.4948,0,0,0,''),
(2917,0,3,4655.97,613.761,8.52327,0,0,0,''),
(2917,0,4,4640.8,623.999,8.37705,0,0,0,''),
(2917,0,5,4631.68,630.801,6.415,0,5000,0,'SAY_REM_RAMP1_1'),
(2917,0,6,4633.53,632.476,6.50983,0,0,0,'ambush'),
(2917,0,7,4639.41,637.121,13.3381,0,0,0,''),
(2917,0,8,4642.35,637.668,13.4374,0,0,0,''),
(2917,0,9,4645.08,634.464,13.4372,0,5000,0,'SAY_REM_RAMP1_2'),
(2917,0,10,4642.35,637.585,13.4352,0,0,0,''),
(2917,0,11,4639.63,637.234,13.3398,0,0,0,''),
(2917,0,12,4633.36,632.462,6.48844,0,0,0,''),
(2917,0,13,4624.71,631.724,6.26403,0,0,0,''),
(2917,0,14,4623.53,629.719,6.20134,0,5000,0,'SAY_REM_BOOK'),
(2917,0,15,4623.45,630.37,6.21894,0,0,0,'SAY_REM_TENT1_1'),
(2917,0,16,4622.62,637.222,6.31285,0,0,0,'ambush'),
(2917,0,17,4619.76,637.386,6.31205,0,5000,0,'SAY_REM_TENT1_2'),
(2917,0,18,4620.03,637.368,6.31205,0,0,0,''),
(2917,0,19,4624.15,637.56,6.3139,0,0,0,''),
(2917,0,20,4622.97,634.016,6.29498,0,0,0,''),
(2917,0,21,4616.93,630.303,6.23919,0,0,0,''),
(2917,0,22,4614.55,616.983,5.68764,0,0,0,''),
(2917,0,23,4610.28,610.029,5.44254,0,0,0,''),
(2917,0,24,4601.15,604.112,2.05486,0,0,0,''),
(2917,0,25,4589.62,597.686,1.05715,0,0,0,''),
(2917,0,26,4577.59,592.146,1.12019,0,0,0,'SAY_REM_MOSS (?)'),
(2917,0,27,4569.85,592.177,1.26087,0,5000,0,'EMOTE_REM_MOSS (?)'),
(2917,0,28,4568.79,590.871,1.21134,0,3000,0,'SAY_REM_MOSS_PROGRESS (?)'),
(2917,0,29,4566.72,564.078,1.34308,0,0,0,'ambush'),
(2917,0,30,4568.27,551.958,5.0042,0,0,0,''),
(2917,0,31,4566.73,551.558,5.42631,0,5000,0,'SAY_REM_PROGRESS'),
(2917,0,32,4566.74,560.768,1.70326,0,0,0,''),
(2917,0,33,4573.92,582.566,0.749801,0,0,0,''),
(2917,0,34,4594.21,598.533,1.03406,0,0,0,''),
(2917,0,35,4601.19,604.283,2.06015,0,0,0,''),
(2917,0,36,4609.54,610.845,5.40222,0,0,0,''),
(2917,0,37,4624.8,618.076,5.85154,0,0,0,''),
(2917,0,38,4632.41,623.778,7.28624,0,0,0,''),
(2917,0,39,4645.92,621.984,8.57997,0,0,0,''),
(2917,0,40,4658.67,611.093,8.89175,0,0,0,''),
(2917,0,41,4671.92,599.752,16.0124,0,5000,0,'SAY_REM_REMEMBER'),
(2917,0,42,4676.98,600.65,17.8257,0,5000,0,'EMOTE_REM_END'),
(3439,0,0,1105.09,-3101.25,82.706,0,1000,0,'SAY_STARTUP1'),
(3439,0,1,1103.2,-3104.35,83.113,0,1000,0,''),
(3439,0,2,1107.82,-3106.5,82.739,0,1000,0,''),
(3439,0,3,1104.73,-3100.83,82.747,0,1000,0,''),
(3439,0,4,1103.24,-3106.27,83.133,0,1000,0,''),
(3439,0,5,1112.81,-3106.29,82.32,0,1000,0,''),
(3439,0,6,1112.83,-3108.91,82.377,0,1000,0,''),
(3439,0,7,1108.05,-3115.16,82.894,0,0,0,''),
(3439,0,8,1108.36,-3104.37,82.377,0,5000,0,''),
(3439,0,9,1100.31,-3097.54,83.15,0,0,0,'SAY_STARTUP2'),
(3439,0,10,1100.56,-3082.72,82.768,0,0,0,''),
(3439,0,11,1097.51,-3069.23,82.206,0,0,0,''),
(3439,0,12,1092.96,-3053.11,82.351,0,0,0,''),
(3439,0,13,1094.01,-3036.96,82.888,0,0,0,''),
(3439,0,14,1095.62,-3025.76,83.392,0,0,0,''),
(3439,0,15,1107.66,-3013.53,85.653,0,0,0,''),
(3439,0,16,1119.65,-3006.93,87.019,0,0,0,''),
(3439,0,17,1129.99,-3002.41,91.232,0,7000,0,'SAY_MERCENARY'),
(3439,0,18,1133.33,-2997.71,91.675,0,1000,0,'SAY_PROGRESS_1'),
(3439,0,19,1131.8,-2987.95,91.976,0,1000,0,''),
(3439,0,20,1122.03,-2993.4,91.536,0,0,0,''),
(3439,0,21,1116.61,-2981.92,92.103,0,0,0,''),
(3439,0,22,1102.24,-2994.25,92.074,0,0,0,''),
(3439,0,23,1096.37,-2978.31,91.873,0,0,0,''),
(3439,0,24,1091.97,-2985.92,91.73,0,40000,0,'SAY_PROGRESS_2'),
(3465,0,0,-2095.84,-3650,61.716,0,0,0,''),
(3465,0,1,-2100.19,-3613.95,61.604,0,0,0,''),
(3465,0,2,-2098.55,-3601.56,59.154,0,0,0,''),
(3465,0,3,-2093.8,-3595.23,56.658,0,0,0,''),
(3465,0,4,-2072.58,-3578.83,48.844,0,0,0,''),
(3465,0,5,-2023.86,-3568.15,24.636,0,0,0,''),
(3465,0,6,-2013.58,-3571.5,22.203,0,0,0,''),
(3465,0,7,-2009.81,-3580.55,21.791,0,0,0,''),
(3465,0,8,-2015.3,-3597.39,21.76,0,0,0,''),
(3465,0,9,-2020.68,-3610.3,21.759,0,0,0,''),
(3465,0,10,-2019.99,-3640.16,21.759,0,0,0,''),
(3465,0,11,-2016.11,-3664.13,21.758,0,0,0,''),
(3465,0,12,-1999.4,-3679.44,21.316,0,0,0,''),
(3465,0,13,-1987.46,-3688.31,18.495,0,0,0,''),
(3465,0,14,-1973.97,-3687.67,14.996,0,0,0,''),
(3465,0,15,-1949.16,-3678.05,11.293,0,0,0,''),
(3465,0,16,-1934.09,-3682.86,9.897,0,30000,0,'SAY_GIL_AT_LAST'),
(3465,0,17,-1935.38,-3682.32,10.029,0,1500,0,'SAY_GIL_PROCEED'),
(3465,0,18,-1879.04,-3699.5,6.582,0,7500,0,'SAY_GIL_FREEBOOTERS'),
(3465,0,19,-1852.73,-3703.78,6.875,0,0,0,''),
(3465,0,20,-1812.99,-3718.5,10.572,0,0,0,''),
(3465,0,21,-1788.17,-3722.87,9.663,0,0,0,''),
(3465,0,22,-1767.21,-3739.92,10.082,0,0,0,''),
(3465,0,23,-1750.19,-3747.39,10.39,0,0,0,''),
(3465,0,24,-1729.34,-3776.67,11.779,0,0,0,''),
(3465,0,25,-1716,-3802.4,12.618,0,0,0,''),
(3465,0,26,-1690.71,-3829.26,13.905,0,0,0,''),
(3465,0,27,-1674.7,-3842.4,13.872,0,0,0,''),
(3465,0,28,-1632.73,-3846.11,14.401,0,0,0,''),
(3465,0,29,-1592.73,-3842.23,14.981,0,0,0,''),
(3465,0,30,-1561.61,-3839.32,19.118,0,0,0,''),
(3465,0,31,-1544.57,-3834.39,18.761,0,0,0,''),
(3465,0,32,-1512.51,-3831.72,22.914,0,0,0,''),
(3465,0,33,-1486.89,-3836.64,23.964,0,0,0,''),
(3465,0,34,-1434.19,-3852.7,18.843,0,0,0,''),
(3465,0,35,-1405.79,-3854.49,17.276,0,0,0,''),
(3465,0,36,-1366.59,-3852.38,19.273,0,0,0,''),
(3465,0,37,-1337.36,-3837.83,17.352,0,2000,0,'SAY_GIL_ALMOST'),
(3465,0,38,-1299.74,-3810.69,20.801,0,0,0,''),
(3465,0,39,-1277.14,-3782.79,25.918,0,0,0,''),
(3465,0,40,-1263.69,-3781.25,26.447,0,0,0,''),
(3465,0,41,-1243.67,-3786.33,25.281,0,0,0,''),
(3465,0,42,-1221.88,-3784.12,24.051,0,0,0,''),
(3465,0,43,-1204.01,-3775.94,24.437,0,0,0,''),
(3465,0,44,-1181.71,-3768.93,23.368,0,0,0,''),
(3465,0,45,-1156.91,-3751.56,21.074,0,0,0,''),
(3465,0,46,-1138.83,-3741.81,17.843,0,0,0,''),
(3465,0,47,-1080.1,-3738.78,19.805,0,0,0,'SAY_GIL_SWEET'),
(3465,0,48,-1069.07,-3735.01,19.302,0,0,0,''),
(3465,0,49,-1061.94,-3724.06,21.086,0,0,0,''),
(3465,0,50,-1053.59,-3697.61,27.32,0,0,0,''),
(3465,0,51,-1044.11,-3690.13,24.856,0,0,0,''),
(3465,0,52,-1040.26,-3690.74,25.342,0,0,0,''),
(3465,0,53,-1028.15,-3688.72,23.843,0,7500,0,'SAY_GIL_FREED'),
(3584,0,0,4520.4,420.235,33.5284,0,2000,0,''),
(3584,0,1,4512.26,408.881,32.9308,0,0,0,''),
(3584,0,2,4507.94,396.47,32.9476,0,0,0,''),
(3584,0,3,4507.53,383.781,32.995,0,0,0,''),
(3584,0,4,4512.1,374.02,33.166,0,0,0,''),
(3584,0,5,4519.75,373.241,33.1574,0,0,0,''),
(3584,0,6,4592.41,369.127,31.4893,0,0,0,''),
(3584,0,7,4598.55,364.801,31.4947,0,0,0,''),
(3584,0,8,4602.76,357.649,32.9265,0,0,0,''),
(3584,0,9,4597.88,352.629,34.0317,0,0,0,''),
(3584,0,10,4590.23,350.9,36.2977,0,0,0,''),
(3584,0,11,4581.5,348.254,38.3878,0,0,0,''),
(3584,0,12,4572.05,348.059,42.3539,0,0,0,''),
(3584,0,13,4564.75,344.041,44.2463,0,0,0,''),
(3584,0,14,4556.63,341.003,47.6755,0,0,0,''),
(3584,0,15,4554.38,334.968,48.8003,0,0,0,''),
(3584,0,16,4557.63,329.783,49.9532,0,0,0,''),
(3584,0,17,4563.32,316.829,53.2409,0,0,0,''),
(3584,0,18,4566.09,303.127,55.0396,0,0,0,''),
(3584,0,19,4561.65,295.456,57.0984,0,4000,0,'SAY_THERYLUNE_FINISH'),
(3584,0,20,4551.03,293.333,57.1534,0,2000,0,''),
(3678,0,0,-134.925,125.468,-78.16,0,0,0,''),
(3678,0,1,-125.684,132.937,-78.42,0,0,0,''),
(3678,0,2,-113.812,139.295,-80.98,0,0,0,''),
(3678,0,3,-109.854,157.538,-80.2,0,0,0,''),
(3678,0,4,-108.64,175.207,-79.74,0,0,0,''),
(3678,0,5,-108.668,195.457,-80.64,0,0,0,''),
(3678,0,6,-111.007,219.007,-86.58,0,0,0,''),
(3678,0,7,-102.408,232.821,-91.52,0,0,0,'first corner SAY_FIRST_CORNER'),
(3678,0,8,-92.434,227.742,-90.75,0,0,0,''),
(3678,0,9,-82.456,224.853,-93.57,0,0,0,''),
(3678,0,10,-67.789,208.073,-93.34,0,0,0,''),
(3678,0,11,-43.343,205.295,-96.37,0,0,0,''),
(3678,0,12,-34.676,221.394,-95.82,0,0,0,''),
(3678,0,13,-32.582,238.573,-93.51,0,0,0,''),
(3678,0,14,-42.149,258.672,-92.88,0,0,0,''),
(3678,0,15,-55.257,274.696,-92.83,0,0,0,'circle of flames SAY_CIRCLE_BANISH'),
(3678,0,16,-48.604,287.584,-92.46,0,0,0,''),
(3678,0,17,-47.236,296.093,-90.88,0,0,0,''),
(3678,0,18,-35.618,309.067,-89.73,0,0,0,''),
(3678,0,19,-23.573,311.376,-88.6,0,0,0,''),
(3678,0,20,-8.692,302.389,-87.43,0,0,0,''),
(3678,0,21,-1.237,293.268,-85.55,0,0,0,''),
(3678,0,22,10.398,279.294,-85.86,0,0,0,''),
(3678,0,23,23.108,264.693,-86.69,0,0,0,''),
(3678,0,24,31.996,251.436,-87.62,0,0,0,''),
(3678,0,25,43.374,233.073,-87.61,0,0,0,''),
(3678,0,26,54.438,212.048,-89.5,0,3000,0,'chamber entrance SAY_NARALEX_CHAMBER'),
(3678,0,27,78.794,208.895,-92.84,0,0,0,''),
(3678,0,28,88.392,225.231,-94.46,0,0,0,''),
(3678,0,29,98.758,233.938,-95.84,0,0,0,''),
(3678,0,30,107.248,233.054,-95.98,0,0,0,''),
(3678,0,31,112.825,233.907,-96.39,0,0,0,''),
(3678,0,32,114.634,236.969,-96.04,0,1000,0,'naralex SAY_BEGIN_RITUAL'),
(3678,0,33,127.385,252.279,-90.07,0,0,0,''),
(3678,0,34,121.595,264.488,-91.55,0,0,0,''),
(3678,0,35,115.472,264.253,-91.5,0,0,0,''),
(3678,0,36,99.988,252.79,-91.51,0,0,0,''),
(3678,0,37,96.347,245.038,-90.34,0,0,0,''),
(3678,0,38,82.201,216.273,-86.1,0,0,0,''),
(3678,0,39,75.112,206.494,-84.8,0,0,0,''),
(3678,0,40,27.174,201.064,-72.31,0,0,0,''),
(3678,0,41,-41.114,204.149,-78.94,0,0,0,''),
(3692,0,1,4608.43,-6.32,69.74,0,1000,0,'stand up'),
(3692,0,2,4608.43,-6.32,69.74,0,4000,0,'SAY_START'),
(3692,0,3,4604.54,-5.17,69.51,0,0,0,''),
(3692,0,4,4604.26,-2.02,69.42,0,0,0,''),
(3692,0,5,4607.75,3.79,70.13,0,1000,0,'first ambush'),
(3692,0,6,4607.75,3.79,70.13,0,0,0,'SAY_FIRST_AMBUSH'),
(3692,0,7,4619.77,27.47,70.4,0,0,0,''),
(3692,0,8,4626.28,42.46,68.75,0,0,0,''),
(3692,0,9,4633.13,51.17,67.4,0,0,0,''),
(3692,0,10,4639.67,79.03,61.74,0,0,0,''),
(3692,0,11,4647.54,94.25,59.92,0,0,0,'second ambush'),
(3692,0,12,4682.08,113.47,54.83,0,0,0,''),
(3692,0,13,4705.28,137.81,53.36,0,0,0,'last ambush'),
(3692,0,14,4730.3,158.76,52.33,0,0,0,''),
(3692,0,15,4756.47,195.65,53.61,0,10000,0,'SAY_END'),
(3692,0,16,4608.43,-6.32,69.74,0,1000,0,'bow'),
(3692,0,17,4608.43,-6.32,69.74,0,4000,0,'SAY_ESCAPE'),
(3692,0,18,4608.43,-6.32,69.74,0,4000,0,'SPELL_MOONSTALKER_FORM'),
(3692,0,19,4604.54,-5.17,69.51,0,0,0,''),
(3692,0,20,4604.26,-2.02,69.42,0,0,0,''),
(3692,0,21,4607.75,3.79,70.13,0,0,0,''),
(3692,0,22,4607.75,3.79,70.13,0,0,0,''),
(3692,0,23,4619.77,27.47,70.4,0,0,0,''),
(3692,0,24,4640.33,33.74,68.22,0,0,0,'quest complete'),
(3849,0,0,-250.923,2116.26,81.179,0,0,0,'SAY_FREE_AD'),
(3849,0,1,-255.049,2119.39,81.179,0,0,0,''),
(3849,0,2,-254.129,2123.45,81.179,0,0,0,''),
(3849,0,3,-253.898,2130.87,81.179,0,0,0,''),
(3849,0,4,-249.889,2142.31,86.972,0,0,0,''),
(3849,0,5,-248.205,2144.02,87.013,0,0,0,''),
(3849,0,6,-240.553,2140.55,87.012,0,0,0,''),
(3849,0,7,-237.514,2142.07,87.012,0,0,0,''),
(3849,0,8,-235.638,2149.23,90.587,0,0,0,''),
(3849,0,9,-237.188,2151.95,90.624,0,0,0,''),
(3849,0,10,-241.162,2153.65,90.624,0,0,0,'SAY_OPEN_DOOR_AD'),
(3849,0,11,-241.13,2154.56,90.624,0,2000,0,'SAY_UNLOCK_DOOR_AD'),
(3849,0,12,-241.13,2154.56,90.624,0,3000,0,''),
(3849,0,13,-241.13,2154.56,90.624,0,5000,0,'SAY_POST1_DOOR_AD'),
(3849,0,14,-241.13,2154.56,90.624,0,0,0,'SAY_POST2_DOOR_AD'),
(3849,0,15,-208.764,2141.6,90.6257,0,0,0,''),
(3849,0,16,-206.441,2143.51,90.4287,0,0,0,''),
(3849,0,17,-203.715,2145.85,88.7052,0,0,0,''),
(3849,0,18,-199.199,2144.88,86.501,0,0,0,''),
(3849,0,19,-195.798,2143.58,86.501,0,0,0,''),
(3849,0,20,-190.029,2141.38,83.2712,0,0,0,''),
(3849,0,21,-189.353,2138.65,83.1102,0,0,0,''),
(3849,0,22,-190.304,2135.73,81.5288,0,0,0,''),
(3849,0,23,-207.325,2112.43,81.0548,0,0,0,''),
(3849,0,24,-208.754,2109.9,81.0527,0,0,0,''),
(3849,0,25,-206.248,2108.62,81.0555,0,0,0,''),
(3849,0,26,-202.017,2106.64,78.6836,0,0,0,''),
(3849,0,27,-200.928,2104.49,78.5569,0,0,0,''),
(3849,0,28,-201.845,2101.17,76.9256,0,0,0,''),
(3849,0,29,-202.844,2100.11,76.8911,0,0,0,''),
(3849,0,30,-213.326,2105.83,76.8925,0,0,0,''),
(3849,0,31,-226.993,2111.47,76.8892,0,0,0,''),
(3849,0,32,-227.955,2112.34,76.8895,0,0,0,''),
(3849,0,33,-230.05,2106.64,76.8895,0,0,0,''),
(3850,0,0,-241.817,2122.9,81.179,0,0,0,'SAY_FREE_AS'),
(3850,0,1,-247.139,2124.89,81.179,0,0,0,''),
(3850,0,2,-253.179,2127.41,81.179,0,0,0,''),
(3850,0,3,-253.898,2130.87,81.179,0,0,0,''),
(3850,0,4,-249.889,2142.31,86.972,0,0,0,''),
(3850,0,5,-248.205,2144.02,87.013,0,0,0,''),
(3850,0,6,-240.553,2140.55,87.012,0,0,0,''),
(3850,0,7,-237.514,2142.07,87.012,0,0,0,''),
(3850,0,8,-235.638,2149.23,90.587,0,0,0,''),
(3850,0,9,-237.188,2151.95,90.624,0,0,0,''),
(3850,0,10,-241.162,2153.65,90.624,0,0,0,'SAY_OPEN_DOOR_AS'),
(3850,0,11,-241.13,2154.56,90.624,0,5000,0,'cast'),
(3850,0,12,-241.13,2154.56,90.624,0,0,0,''),
(3850,0,13,-241.13,2154.56,90.624,0,5000,0,'SAY_POST_DOOR_AS'),
(3850,0,14,-241.13,2154.56,90.624,0,2500,0,'cast'),
(3850,0,15,-241.13,2154.56,90.624,0,0,0,'SAY_VANISH_AS'),
(4484,0,0,3178.57,188.52,4.27,0,0,0,'SAY_QUEST_START'),
(4484,0,1,3189.82,198.56,5.62,0,0,0,''),
(4484,0,2,3215.21,185.78,6.43,0,0,0,''),
(4484,0,3,3224.05,183.08,6.74,0,0,0,''),
(4484,0,4,3228.11,194.97,7.51,0,0,0,''),
(4484,0,5,3225.33,201.78,7.25,0,0,0,''),
(4484,0,6,3233.33,226.88,10.18,0,0,0,''),
(4484,0,7,3274.12,225.83,10.72,0,0,0,''),
(4484,0,8,3321.63,209.82,12.36,0,0,0,''),
(4484,0,9,3369.66,226.21,11.69,0,0,0,''),
(4484,0,10,3402.35,227.2,9.48,0,0,0,''),
(4484,0,11,3441.92,224.75,10.85,0,0,0,''),
(4484,0,12,3453.87,220.31,12.52,0,0,0,''),
(4484,0,13,3472.51,213.68,13.26,0,0,0,''),
(4484,0,14,3515.49,212.96,9.76,0,5000,0,'SAY_FIRST_AMBUSH_START'),
(4484,0,15,3516.21,212.84,9.52,0,20000,0,'SAY_FIRST_AMBUSH_END'),
(4484,0,16,3548.22,217.12,7.34,0,0,0,''),
(4484,0,17,3567.57,219.43,5.22,0,0,0,''),
(4484,0,18,3659.85,209.68,2.27,0,0,0,''),
(4484,0,19,3734.9,177.64,6.75,0,0,0,''),
(4484,0,20,3760.24,162.51,7.49,0,5000,0,'SAY_SECOND_AMBUSH_START'),
(4484,0,21,3761.58,161.14,7.37,0,20000,0,'SAY_SECOND_AMBUSH_END'),
(4484,0,22,3801.17,129.87,9.38,0,0,0,''),
(4484,0,23,3815.53,118.53,10.14,0,0,0,''),
(4484,0,24,3894.58,44.88,15.49,0,0,0,''),
(4484,0,25,3972.83,0.42,17.34,0,0,0,''),
(4484,0,26,4026.41,-7.63,16.77,0,0,0,''),
(4484,0,27,4086.24,12.32,16.12,0,0,0,''),
(4484,0,28,4158.79,50.67,25.86,0,0,0,''),
(4484,0,29,4223.48,99.52,35.47,0,5000,0,'SAY_FINAL_AMBUSH_START'),
(4484,0,30,4224.28,100.02,35.49,0,10000,0,'SAY_QUEST_END'),
(4484,0,31,4243.45,117.44,38.83,0,0,0,''),
(4484,0,32,4264.18,134.22,42.96,0,0,0,''),
(4500,0,0,-3125.6,-2885.67,34.731,0,2500,0,''),
(4500,0,1,-3120.26,-2877.83,34.917,0,0,0,''),
(4500,0,2,-3116.49,-2850.67,34.869,0,0,0,''),
(4500,0,3,-3093.47,-2819.19,34.432,0,0,0,''),
(4500,0,4,-3104.73,-2802.02,33.954,0,0,0,''),
(4500,0,5,-3105.91,-2780.23,34.469,0,0,0,''),
(4500,0,6,-3116.08,-2757.9,34.734,0,0,0,''),
(4500,0,7,-3125.23,-2733.96,33.189,0,0,0,''),
(4508,0,0,2194.38,1791.65,65.48,0,5000,0,''),
(4508,0,1,2188.56,1805.87,64.45,0,0,0,''),
(4508,0,2,2186.2,1836.28,59.859,0,5000,0,'SAY_WILLIX_1'),
(4508,0,3,2163.27,1851.67,56.73,0,0,0,''),
(4508,0,4,2140.22,1845.02,48.32,0,0,0,''),
(4508,0,5,2131.5,1804.29,46.85,0,0,0,''),
(4508,0,6,2096.18,1789.03,51.13,0,3000,0,'SAY_WILLIX_2'),
(4508,0,7,2074.46,1780.09,55.64,0,0,0,''),
(4508,0,8,2055.12,1768.67,58.46,0,0,0,''),
(4508,0,9,2037.83,1748.62,60.27,0,5000,0,'SAY_WILLIX_3'),
(4508,0,10,2037.51,1728.94,60.85,0,0,0,''),
(4508,0,11,2044.7,1711.71,59.71,0,0,0,''),
(4508,0,12,2067.66,1701.84,57.77,0,0,0,''),
(4508,0,13,2078.91,1704.54,56.77,0,0,0,''),
(4508,0,14,2097.65,1715.24,54.74,0,3000,0,'SAY_WILLIX_4'),
(4508,0,15,2106.44,1720.98,54.41,0,0,0,''),
(4508,0,16,2123.96,1732.56,52.27,0,0,0,''),
(4508,0,17,2153.82,1728.73,51.92,0,0,0,''),
(4508,0,18,2163.49,1706.33,54.42,0,0,0,''),
(4508,0,19,2158.75,1695.98,55.7,0,0,0,''),
(4508,0,20,2142.6,1680.72,58.24,0,0,0,''),
(4508,0,21,2118.31,1671.54,59.21,0,0,0,''),
(4508,0,22,2086.02,1672.04,61.24,0,0,0,''),
(4508,0,23,2068.81,1658.93,61.24,0,0,0,''),
(4508,0,24,2062.82,1633.31,64.35,0,0,0,''),
(4508,0,25,2060.92,1600.11,62.41,0,3000,0,'SAY_WILLIX_5'),
(4508,0,26,2063.05,1589.16,63.26,0,0,0,''),
(4508,0,27,2063.67,1577.22,65.89,0,0,0,''),
(4508,0,28,2057.94,1560.68,68.4,0,0,0,''),
(4508,0,29,2052.56,1548.05,73.35,0,0,0,''),
(4508,0,30,2045.22,1543.4,76.65,0,0,0,''),
(4508,0,31,2034.35,1543.01,79.7,0,0,0,''),
(4508,0,32,2029.95,1542.94,80.79,0,0,0,''),
(4508,0,33,2021.34,1538.67,80.8,0,0,0,'SAY_WILLIX_6'),
(4508,0,34,2012.45,1549.48,79.93,0,0,0,''),
(4508,0,35,2008.05,1554.92,80.44,0,0,0,''),
(4508,0,36,2006.54,1562.72,81.11,0,0,0,''),
(4508,0,37,2003.8,1576.43,81.57,0,0,0,''),
(4508,0,38,2000.57,1590.06,80.62,0,0,0,''),
(4508,0,39,1998.96,1596.87,80.22,0,0,0,''),
(4508,0,40,1991.19,1600.82,79.39,0,0,0,''),
(4508,0,41,1980.71,1601.44,79.77,0,0,0,''),
(4508,0,42,1967.22,1600.18,80.62,0,0,0,''),
(4508,0,43,1956.43,1596.97,81.75,0,0,0,''),
(4508,0,44,1954.87,1592.02,82.18,0,3000,0,'SAY_WILLIX_7'),
(4508,0,45,1948.35,1571.35,80.96,0,30000,0,'SAY_WILLIX_END'),
(4508,0,46,1947.02,1566.42,81.8,0,30000,0,''),
(4880,0,0,-2670.22,-3446.19,34.085,0,0,0,''),
(4880,0,1,-2683.96,-3451.09,34.707,0,0,0,''),
(4880,0,2,-2703.24,-3454.82,33.395,0,0,0,''),
(4880,0,3,-2721.61,-3457.41,33.626,0,0,0,''),
(4880,0,4,-2739.98,-3459.84,33.329,0,0,0,''),
(4880,0,5,-2756.24,-3460.52,32.037,0,5000,0,'SAY_STINKY_FIRST_STOP'),
(4880,0,6,-2764.52,-3472.71,33.75,0,0,0,''),
(4880,0,7,-2773.68,-3482.91,32.84,0,0,0,''),
(4880,0,8,-2781.39,-3490.61,32.598,0,0,0,''),
(4880,0,9,-2788.31,-3492.9,30.761,0,0,0,''),
(4880,0,10,-2794.58,-3489.19,31.119,0,5000,0,'SAY_SECOND_STOP'),
(4880,0,11,-2789.43,-3498.04,31.05,0,0,0,''),
(4880,0,12,-2786.97,-3508.17,31.983,0,0,0,''),
(4880,0,13,-2786.77,-3519.95,31.079,0,0,0,''),
(4880,0,14,-2789.36,-3525.02,31.831,0,0,0,''),
(4880,0,15,-2797.95,-3523.69,31.697,0,0,0,''),
(4880,0,16,-2812.97,-3519.84,29.864,0,0,0,''),
(4880,0,17,-2818.33,-3521.4,30.563,0,0,0,''),
(4880,0,18,-2824.77,-3528.73,32.399,0,0,0,''),
(4880,0,19,-2830.7,-3539.88,32.505,0,0,0,''),
(4880,0,20,-2836.24,-3549.96,31.18,0,0,0,''),
(4880,0,21,-2837.58,-3561.05,30.74,0,0,0,''),
(4880,0,22,-2834.45,-3568.26,30.751,0,0,0,''),
(4880,0,23,-2827.35,-3569.81,31.316,0,0,0,''),
(4880,0,24,-2817.38,-3566.96,30.947,0,5000,0,'SAY_THIRD_STOP_1'),
(4880,0,25,-2817.38,-3566.96,30.947,0,2000,0,'SAY_THIRD_STOP_2'),
(4880,0,26,-2817.38,-3566.96,30.947,0,0,0,'SAY_THIRD_STOP_3'),
(4880,0,27,-2818.81,-3579.42,28.525,0,0,0,''),
(4880,0,28,-2820.21,-3590.64,30.269,0,0,0,''),
(4880,0,29,-2820.85,-3593.94,31.15,0,3000,0,''),
(4880,0,30,-2820.85,-3593.94,31.15,0,3000,0,'SAY_PLANT_GATHERED'),
(4880,0,31,-2834.21,-3592.04,33.79,0,0,0,''),
(4880,0,32,-2840.31,-3586.21,36.288,0,0,0,''),
(4880,0,33,-2847.49,-3576.42,37.66,0,0,0,''),
(4880,0,34,-2855.72,-3565.18,39.39,0,0,0,''),
(4880,0,35,-2861.78,-3552.9,41.243,0,0,0,''),
(4880,0,36,-2869.54,-3545.58,40.701,0,0,0,''),
(4880,0,37,-2877.78,-3538.37,37.274,0,0,0,''),
(4880,0,38,-2882.68,-3534.17,34.844,0,0,0,''),
(4880,0,39,-2888.57,-3534.12,34.298,0,4000,0,'SAY_STINKY_END'),
(4880,0,40,-2888.57,-3534.12,34.298,0,0,0,''),
(4962,0,0,-3804.44,-828.048,10.0931,0,0,0,''),
(4962,0,1,-3803.93,-835.772,10.0777,0,0,0,''),
(4962,0,2,-3792.63,-835.671,9.65566,0,0,0,''),
(4962,0,3,-3772.43,-835.346,10.869,0,0,0,''),
(4962,0,4,-3765.94,-840.129,10.8856,0,0,0,''),
(4962,0,5,-3738.63,-830.997,11.0574,0,0,0,''),
(4962,0,6,-3690.22,-862.262,9.96045,0,0,0,''),
(4983,0,0,-3322.65,-3124.63,33.842,0,0,0,''),
(4983,0,1,-3326.34,-3126.83,34.426,0,0,0,''),
(4983,0,2,-3336.98,-3129.61,30.692,0,0,0,''),
(4983,0,3,-3342.6,-3132.15,30.422,0,0,0,''),
(4983,0,4,-3355.83,-3140.95,29.534,0,0,0,''),
(4983,0,5,-3365.83,-3144.28,35.176,0,0,0,''),
(4983,0,6,-3368.9,-3147.27,36.091,0,0,0,''),
(4983,0,7,-3369.36,-3169.83,36.325,0,0,0,''),
(4983,0,8,-3371.44,-3183.91,33.454,0,0,0,''),
(4983,0,9,-3373.82,-3190.86,34.717,0,5000,0,'SAY_OGR_SPOT'),
(4983,0,10,-3368.53,-3198.21,34.926,0,0,0,'SAY_OGR_RET_WHAT'),
(4983,0,11,-3366.27,-3210.87,33.733,0,5000,0,'pause'),
(4983,0,12,-3368.53,-3198.21,34.926,0,0,0,''),
(4983,0,13,-3373.82,-3190.86,34.717,0,0,0,''),
(4983,0,14,-3371.44,-3183.91,33.454,0,0,0,''),
(4983,0,15,-3369.36,-3169.83,36.325,0,0,0,''),
(4983,0,16,-3368.9,-3147.27,36.091,0,0,0,''),
(4983,0,17,-3365.83,-3144.28,35.176,0,0,0,''),
(4983,0,18,-3355.83,-3140.95,29.534,0,0,0,''),
(4983,0,19,-3342.6,-3132.15,30.422,0,0,0,''),
(4983,0,20,-3336.98,-3129.61,30.692,0,0,0,''),
(4983,0,21,-3326.34,-3126.83,34.426,0,0,0,''),
(4983,0,22,-3322.65,-3124.63,33.842,0,0,0,''),
(5391,0,0,-9901.12,-3727.29,22.11,0,3000,0,''),
(5391,0,1,-9909.27,-3727.81,23.25,0,0,0,''),
(5391,0,2,-9935.25,-3729.02,22.11,0,0,0,''),
(5391,0,3,-9945.83,-3719.34,21.68,0,0,0,''),
(5391,0,4,-9963.41,-3710.18,21.71,0,0,0,''),
(5391,0,5,-9972.75,-3690.13,21.68,0,0,0,''),
(5391,0,6,-9989.7,-3669.67,21.67,0,0,0,''),
(5391,0,7,-9989.21,-3647.76,23,0,0,0,''),
(5391,0,8,-9992.27,-3633.74,21.67,0,0,0,''),
(5391,0,9,-10002.3,-3611.67,22.26,0,0,0,''),
(5391,0,10,-9999.25,-3586.33,21.85,0,0,0,''),
(5391,0,11,-10006.5,-3571.99,21.67,0,0,0,''),
(5391,0,12,-10014.3,-3545.24,21.67,0,0,0,''),
(5391,0,13,-10018.9,-3525.03,21.68,0,0,0,''),
(5391,0,14,-10030.2,-3514.77,21.67,0,0,0,''),
(5391,0,15,-10045.1,-3501.49,21.67,0,0,0,''),
(5391,0,16,-10052.9,-3479.13,21.67,0,0,0,''),
(5391,0,17,-10060.7,-3460.31,21.67,0,0,0,''),
(5391,0,18,-10074.7,-3436.85,20.97,0,0,0,''),
(5391,0,19,-10074.7,-3436.85,20.97,0,0,0,''),
(5391,0,20,-10072.9,-3408.92,20.43,0,15000,0,''),
(5391,0,21,-10108,-3406.05,22.06,0,0,0,''),
(5644,0,1,-339.679,1752.04,139.482,0,0,0,''),
(5644,0,2,-328.957,1734.95,139.327,0,0,0,''),
(5644,0,3,-338.29,1731.36,139.327,0,0,0,''),
(5644,0,4,-350.747,1731.12,139.338,0,0,0,''),
(5644,0,5,-365.064,1739.04,139.376,0,0,0,''),
(5644,0,6,-371.105,1746.03,139.374,0,0,0,''),
(5644,0,7,-383.141,1738.62,138.93,0,0,0,''),
(5644,0,8,-390.445,1733.98,136.353,0,0,0,''),
(5644,0,9,-401.368,1726.77,131.071,0,0,0,''),
(5644,0,10,-416.016,1721.19,129.807,0,0,0,''),
(5644,0,11,-437.139,1709.82,126.342,0,0,0,''),
(5644,0,12,-455.83,1695.61,119.305,0,0,0,''),
(5644,0,13,-459.862,1687.92,116.059,0,0,0,''),
(5644,0,14,-463.565,1679.1,111.653,0,0,0,''),
(5644,0,15,-461.485,1670.94,109.033,0,0,0,''),
(5644,0,16,-471.786,1647.34,102.862,0,0,0,''),
(5644,0,17,-477.146,1625.69,98.342,0,0,0,''),
(5644,0,18,-475.815,1615.81,97.07,0,0,0,''),
(5644,0,19,-474.329,1590.01,94.4982,0,0,0,''),
(6182,0,0,-11480.7,1545.09,49.8986,0,0,0,''),
(6182,0,1,-11466.8,1530.15,50.2636,0,0,0,''),
(6182,0,2,-11465.2,1528.34,50.9544,0,0,0,'entrance hut'),
(6182,0,3,-11463,1525.24,50.9377,0,0,0,''),
(6182,0,4,-11461,1526.61,50.9377,0,5000,0,'pick up rifle'),
(6182,0,5,-11463,1525.24,50.9377,0,0,0,''),
(6182,0,6,-11465.2,1528.34,50.9544,0,0,0,''),
(6182,0,7,-11468.4,1535.08,50.4009,0,15000,0,'hold, prepare for wave1'),
(6182,0,8,-11468.4,1535.08,50.4009,0,15000,0,'hold, prepare for wave2'),
(6182,0,9,-11468.4,1535.08,50.4009,0,10000,0,'hold, prepare for wave3'),
(6182,0,10,-11467.9,1532.46,50.3489,0,0,0,'we are done'),
(6182,0,11,-11466.1,1529.86,50.2094,0,0,0,''),
(6182,0,12,-11463,1525.24,50.9377,0,0,0,''),
(6182,0,13,-11461,1526.61,50.9377,0,5000,0,'deliver rifle'),
(6182,0,14,-11463,1525.24,50.9377,0,0,0,''),
(6182,0,15,-11465.2,1528.34,50.9544,0,0,0,''),
(6182,0,16,-11470.3,1537.28,50.3785,0,0,0,''),
(6182,0,17,-11475.6,1548.68,50.1844,0,0,0,'complete quest'),
(6182,0,18,-11482.3,1557.41,48.6245,0,0,0,''),
(6575,0,0,1945.81,-431.54,16.36,0,0,0,''),
(6575,0,1,1946.21,-436.41,16.36,0,0,0,''),
(6575,0,2,1950.01,-444.11,14.63,0,0,0,''),
(6575,0,3,1956.08,-449.34,13.12,0,0,0,''),
(6575,0,4,1966.59,-450.55,11.27,0,0,0,''),
(6575,0,5,1976.09,-447.51,11.27,0,0,0,''),
(6575,0,6,1983.42,-435.85,11.27,0,0,0,''),
(6575,0,7,1978.17,-428.81,11.27,0,0,0,''),
(6575,0,8,1973.97,-422.08,9.04,0,0,0,''),
(6575,0,9,1963.84,-418.9,6.17,0,0,0,''),
(6575,0,10,1961.22,-422.74,6.17,0,0,0,''),
(6575,0,11,1964.8,-431.26,6.17,0,300000,0,''),
(7780,0,0,261.059,-2757.88,122.553,0,0,0,''),
(7780,0,1,259.812,-2758.25,122.555,0,0,0,'SAY_RIN_FREE'),
(7780,0,2,253.823,-2758.62,122.562,0,0,0,''),
(7780,0,3,241.395,-2769.75,123.309,0,0,0,''),
(7780,0,4,218.916,-2783.4,123.355,0,0,0,''),
(7780,0,5,209.088,-2789.68,122.001,0,0,0,''),
(7780,0,6,204.454,-2792.21,120.62,0,0,0,''),
(7780,0,7,182.013,-2810,113.887,0,0,0,'summon'),
(7780,0,8,164.412,-2825.16,107.779,0,0,0,''),
(7780,0,9,149.728,-2833.7,106.224,0,0,0,''),
(7780,0,10,142.448,-2838.81,109.665,0,0,0,''),
(7780,0,11,133.275,-2845.14,112.606,0,0,0,''),
(7780,0,12,111.247,-2861.07,116.305,0,0,0,''),
(7780,0,13,96.1041,-2874.89,114.397,0,0,0,'summon'),
(7780,0,14,73.3699,-2881.18,117.666,0,0,0,''),
(7780,0,15,58.5792,-2889.15,116.253,0,0,0,''),
(7780,0,16,33.2142,-2906.34,115.083,0,0,0,''),
(7780,0,17,19.5865,-2908.71,117.276,0,7500,0,'SAY_RIN_COMPLETE'),
(7780,0,18,10.2825,-2911.61,118.394,0,0,0,''),
(7780,0,19,-37.5804,-2942.73,117.145,0,0,0,''),
(7780,0,20,-68.5994,-2953.69,116.685,0,0,0,''),
(7780,0,21,-102.054,-2956.97,116.677,0,0,0,''),
(7780,0,22,-135.994,-2955.74,115.788,0,0,0,''),
(7780,0,23,-171.562,-2951.42,115.451,0,0,0,''),
(7784,0,0,-8845.65,-4373.98,43.87,0,5000,0,'SAY_START'),
(7784,0,1,-8840.79,-4373.73,44.24,0,0,0,''),
(7784,0,2,-8837.43,-4373.56,45.6,0,0,0,''),
(7784,0,3,-8832.74,-4373.32,45.68,0,0,0,''),
(7784,0,4,-8829.37,-4373.14,44.33,0,0,0,''),
(7784,0,5,-8817.38,-4372.41,35.58,0,0,0,''),
(7784,0,6,-8803.47,-4371.6,30.34,0,0,0,''),
(7784,0,7,-8795.1,-4365.61,26.08,0,0,0,''),
(7784,0,8,-8766.78,-4367.13,25.15,0,0,0,''),
(7784,0,9,-8755.63,-4367.54,24.63,0,0,0,''),
(7784,0,10,-8754.42,-4365.59,24.15,0,0,0,''),
(7784,0,11,-8728.82,-4353.13,20.9,0,0,0,''),
(7784,0,12,-8706.6,-4356.55,17.93,0,0,0,''),
(7784,0,13,-8679,-4380.23,12.64,0,0,0,''),
(7784,0,14,-8642.96,-4393.82,12.52,0,0,0,''),
(7784,0,15,-8611.19,-4399.11,9.55,0,0,0,''),
(7784,0,16,-8554.87,-4409.32,13.05,0,0,0,''),
(7784,0,17,-8531.64,-4411.96,11.2,0,0,0,''),
(7784,0,18,-8510.4,-4414.38,12.84,0,0,0,''),
(7784,0,19,-8476.92,-4418.34,9.71,0,0,0,''),
(7784,0,20,-8435.89,-4426.74,9.67,0,0,0,''),
(7784,0,21,-8381.89,-4446.4,10.23,0,0,0,''),
(7784,0,22,-8351.15,-4447.79,9.99,0,5000,0,'first ambush SAY_AMBUSH'),
(7784,0,23,-8324.18,-4445.05,9.71,0,0,0,''),
(7784,0,24,-8138.94,-4384.78,10.92,0,0,0,''),
(7784,0,25,-8036.87,-4443.38,9.65,0,0,0,''),
(7784,0,26,-7780.92,-4761.81,9.5,0,0,0,''),
(7784,0,27,-7587.67,-4765.01,8.96,0,0,0,''),
(7784,0,28,-7497.65,-4792.86,10.01,0,0,0,'second ambush SAY_AMBUSH'),
(7784,0,29,-7391.54,-4774.26,12.44,0,0,0,''),
(7784,0,30,-7308.42,-4739.87,12.65,0,0,0,''),
(7784,0,31,-7016.11,-4751.12,10.06,0,0,0,''),
(7784,0,32,-6985.52,-4777.41,10.26,0,0,0,''),
(7784,0,33,-6953.02,-4786,6.32,0,0,0,''),
(7784,0,34,-6940.37,-4831.03,0.67,0,10000,0,'quest complete SAY_END'),
(7806,0,0,495.404,-3478.35,114.837,0,0,0,''),
(7806,0,1,492.705,-3486.11,108.627,0,0,0,''),
(7806,0,2,487.25,-3485.76,107.89,0,0,0,''),
(7806,0,3,476.852,-3489.88,99.985,0,0,0,''),
(7806,0,4,467.212,-3493.36,99.819,0,0,0,''),
(7806,0,5,460.017,-3496.98,104.481,0,0,0,''),
(7806,0,6,439.619,-3500.73,110.534,0,0,0,''),
(7806,0,7,428.326,-3495.87,118.662,0,0,0,''),
(7806,0,8,424.664,-3489.38,121.999,0,0,0,''),
(7806,0,9,424.137,-3470.95,124.333,0,0,0,''),
(7806,0,10,421.791,-3449.24,119.126,0,0,0,''),
(7806,0,11,404.247,-3429.38,117.644,0,0,0,''),
(7806,0,12,335.465,-3430.72,116.456,0,0,0,''),
(7806,0,13,317.16,-3426.71,116.226,0,0,0,''),
(7806,0,14,331.18,-3464,117.143,0,0,0,''),
(7806,0,15,336.394,-3501.88,118.201,0,0,0,''),
(7806,0,16,337.251,-3544.76,117.284,0,0,0,''),
(7806,0,17,337.749,-3565.42,116.797,0,0,0,''),
(7806,0,18,336.011,-3597.36,118.225,0,0,0,''),
(7806,0,19,324.619,-3622.88,119.811,0,0,0,''),
(7806,0,20,308.027,-3648.6,123.047,0,0,0,''),
(7806,0,21,276.325,-3685.74,128.356,0,0,0,''),
(7806,0,22,239.981,-3717.33,131.874,0,0,0,''),
(7806,0,23,224.951,-3730.17,132.125,0,0,0,''),
(7806,0,24,198.708,-3768.29,129.42,0,0,0,''),
(7806,0,25,183.758,-3791.07,128.045,0,0,0,''),
(7806,0,26,178.111,-3801.58,128.37,0,3000,0,'SAY_OOX_DANGER'),
(7806,0,27,162.215,-3827.01,129.424,0,0,0,''),
(7806,0,28,141.665,-3864.52,131.419,0,0,0,''),
(7806,0,29,135.302,-3880.09,132.12,0,0,0,''),
(7806,0,30,122.461,-3910.07,135.605,0,0,0,''),
(7806,0,31,103.376,-3937.73,137.342,0,0,0,''),
(7806,0,32,81.4145,-3958.61,138.469,0,0,0,''),
(7806,0,33,55.3781,-3982,136.52,0,0,0,''),
(7806,0,34,13.9831,-4013.95,126.903,0,0,0,''),
(7806,0,35,-21.658,-4048.71,118.068,0,0,0,''),
(7806,0,36,-52.4431,-4081.21,117.477,0,0,0,''),
(7806,0,37,-102.711,-4116.76,118.666,0,0,0,''),
(7806,0,38,-92.9962,-4135.85,119.31,0,0,0,''),
(7806,0,39,-86.3913,-4153.33,122.502,0,0,0,''),
(7806,0,40,-85.7461,-4163.6,121.892,0,0,0,''),
(7806,0,41,-90.544,-4183.58,117.587,0,0,0,''),
(7806,0,42,-110.224,-4205.86,121.878,0,0,0,''),
(7806,0,43,-115.258,-4211.96,121.878,0,3000,0,'SAY_OOX_DANGER'),
(7806,0,44,-128.595,-4233.34,117.766,0,0,0,''),
(7806,0,45,-135.359,-4258.12,117.562,0,0,0,''),
(7806,0,46,-156.832,-4258.96,120.059,0,0,0,''),
(7806,0,47,-167.12,-4274.1,117.062,0,0,0,''),
(7806,0,48,-176.291,-4287.59,118.721,0,0,0,''),
(7806,0,49,-196.993,-4315.82,117.588,0,0,0,''),
(7806,0,50,-209.329,-4331.67,115.142,0,0,0,''),
(7806,0,51,-232.292,-4356.02,108.543,0,0,0,''),
(7806,0,52,-232.16,-4370.9,102.815,0,0,0,''),
(7806,0,53,-210.271,-4389.9,84.167,0,0,0,''),
(7806,0,54,-187.94,-4407.53,70.987,0,0,0,''),
(7806,0,55,-181.354,-4418.77,64.778,0,0,0,''),
(7806,0,56,-170.53,-4440.44,58.943,0,0,0,''),
(7806,0,57,-141.429,-4465.32,45.963,0,0,0,''),
(7806,0,58,-120.994,-4487.09,32.075,0,0,0,''),
(7806,0,59,-104.135,-4501.84,25.051,0,0,0,''),
(7806,0,60,-84.1547,-4529.44,11.952,0,0,0,''),
(7806,0,61,-88.6989,-4544.63,9.055,0,0,0,''),
(7806,0,62,-100.603,-4575.03,11.388,0,0,0,''),
(7806,0,63,-106.909,-4600.41,11.046,0,0,0,''),
(7806,0,64,-106.832,-4620.5,11.057,0,3000,0,'SAY_OOX_COMPLETE'),
(7807,0,0,-4943.74,1715.74,62.74,0,0,0,'SAY_START'),
(7807,0,1,-4944.93,1706.66,63.16,0,0,0,''),
(7807,0,2,-4942.82,1690.22,64.25,0,0,0,''),
(7807,0,3,-4946.47,1669.62,63.84,0,0,0,''),
(7807,0,4,-4955.93,1651.88,63,0,0,0,''),
(7807,0,5,-4967.58,1643.86,64.31,0,0,0,''),
(7807,0,6,-4978.12,1607.9,64.3,0,0,0,''),
(7807,0,7,-4975.38,1596.16,64.7,0,0,0,''),
(7807,0,8,-4972.82,1581.89,61.75,0,0,0,''),
(7807,0,9,-4958.65,1581.05,61.81,0,0,0,''),
(7807,0,10,-4936.72,1594.89,65.96,0,0,0,''),
(7807,0,11,-4885.69,1598.1,67.45,0,4000,0,'first ambush SAY_AMBUSH'),
(7807,0,12,-4874.2,1601.73,68.54,0,0,0,''),
(7807,0,13,-4816.64,1594.47,78.2,0,0,0,''),
(7807,0,14,-4802.2,1571.92,87.01,0,0,0,''),
(7807,0,15,-4746.4,1576.11,84.59,0,0,0,''),
(7807,0,16,-4739.72,1707.16,94.04,0,0,0,''),
(7807,0,17,-4674.03,1840.44,89.17,0,0,0,''),
(7807,0,18,-4667.94,1864.11,85.18,0,0,0,''),
(7807,0,19,-4668.08,1886.39,81.14,0,0,0,''),
(7807,0,20,-4679.43,1932.32,73.76,0,0,0,''),
(7807,0,21,-4674.17,1946.66,70.83,0,5000,0,'second ambush SAY_AMBUSH'),
(7807,0,22,-4643.94,1967.45,65.27,0,0,0,''),
(7807,0,23,-4595.6,2010.75,52.1,0,0,0,''),
(7807,0,24,-4562.65,2029.28,45.41,0,0,0,''),
(7807,0,25,-4538.56,2032.65,45.28,0,0,0,''),
(7807,0,26,-4531.96,2034.15,48.34,0,0,0,''),
(7807,0,27,-4507.75,2039.32,51.57,0,0,0,''),
(7807,0,28,-4482.74,2045.67,48.15,0,0,0,''),
(7807,0,29,-4460.87,2051.54,45.55,0,0,0,''),
(7807,0,30,-4449.97,2060.03,45.51,0,10000,0,'third ambush SAY_AMBUSH'),
(7807,0,31,-4448.99,2079.05,44.64,0,0,0,''),
(7807,0,32,-4436.64,2134.48,28.83,0,0,0,''),
(7807,0,33,-4429.25,2170.2,15.44,0,0,0,''),
(7807,0,34,-4424.83,2186.11,11.48,0,0,0,''),
(7807,0,35,-4416.71,2209.76,7.36,0,0,0,''),
(7807,0,36,-4405.25,2231.77,5.94,0,0,0,''),
(7807,0,37,-4377.61,2265.45,6.71,0,15000,0,'complete quest SAY_END'),
(7998,0,1,-510.13,-132.69,-152.5,0,0,0,''),
(7998,0,2,-511.099,-129.74,-153.845,0,0,0,''),
(7998,0,3,-511.79,-127.476,-155.551,0,0,0,''),
(7998,0,4,-512.969,-124.926,-156.115,0,5000,0,''),
(7998,0,5,-513.972,-120.236,-156.116,0,0,0,''),
(7998,0,6,-514.388,-115.19,-156.117,0,0,0,''),
(7998,0,7,-514.304,-111.478,-155.52,0,0,0,''),
(7998,0,8,-514.84,-107.663,-154.893,0,0,0,''),
(7998,0,9,-518.994,-101.416,-154.648,0,27000,0,''),
(7998,0,10,-526.998,-98.1488,-155.625,0,0,0,''),
(7998,0,11,-534.569,-105.41,-155.989,0,30000,0,''),
(7998,0,12,-535.534,-104.695,-155.971,0,0,0,''),
(7998,0,13,-541.63,-98.6583,-155.858,0,25000,0,''),
(7998,0,14,-535.092,-99.9175,-155.974,0,0,0,''),
(7998,0,15,-519.01,-101.51,-154.677,0,3000,0,''),
(7998,0,16,-504.466,-97.848,-150.955,0,30000,0,''),
(7998,0,17,-506.907,-89.1474,-151.083,0,23000,0,''),
(7998,0,18,-512.758,-101.902,-153.198,0,0,0,''),
(7998,0,19,-519.988,-124.848,-156.128,0,86400000,0,'this npc should not reset on wp end'),
(8284,0,0,-7007.21,-1749.16,234.182,0,3000,0,'stand up'),
(8284,0,1,-7007.32,-1729.85,234.162,0,0,0,''),
(8284,0,2,-7006.39,-1726.52,234.099,0,0,0,''),
(8284,0,3,-7003.26,-1726.9,234.594,0,0,0,''),
(8284,0,4,-6994.78,-1733.57,238.281,0,0,0,''),
(8284,0,5,-6987.9,-1735.94,240.727,0,0,0,''),
(8284,0,6,-6978.7,-1736.99,241.809,0,0,0,''),
(8284,0,7,-6964.26,-1740.25,241.713,0,0,0,''),
(8284,0,8,-6946.7,-1746.28,241.667,0,0,0,''),
(8284,0,9,-6938.75,-1749.38,240.744,0,0,0,''),
(8284,0,10,-6927,-1768.78,240.744,0,0,0,''),
(8284,0,11,-6909.45,-1791.26,240.744,0,0,0,''),
(8284,0,12,-6898.23,-1804.87,240.744,0,0,0,''),
(8284,0,13,-6881.28,-1821.79,240.744,0,0,0,''),
(8284,0,14,-6867.65,-1832.67,240.706,0,0,0,''),
(8284,0,15,-6850.18,-1839.25,243.006,0,0,0,''),
(8284,0,16,-6829.38,-1847.64,244.19,0,0,0,''),
(8284,0,17,-6804.62,-1857.54,244.209,0,0,0,''),
(8284,0,18,-6776.42,-1868.88,244.142,0,0,0,''),
(8284,0,19,-6753.47,-1876.91,244.17,0,10000,0,'stop'),
(8284,0,20,-6753.47,-1876.91,244.17,0,0,0,'ambush'),
(8284,0,21,-6731.03,-1884.94,244.144,0,0,0,''),
(8284,0,22,-6705.74,-1896.78,244.144,0,0,0,''),
(8284,0,23,-6678.96,-1909.61,244.369,0,0,0,''),
(8284,0,24,-6654.26,-1916.76,244.145,0,0,0,''),
(8284,0,25,-6620.6,-1917.61,244.149,0,0,0,''),
(8284,0,26,-6575.96,-1922.41,244.149,0,0,0,''),
(8284,0,27,-6554.81,-1929.88,244.162,0,0,0,''),
(8284,0,28,-6521.86,-1947.32,244.151,0,0,0,''),
(8284,0,29,-6493.32,-1962.65,244.151,0,0,0,''),
(8284,0,30,-6463.35,-1975.54,244.213,0,0,0,''),
(8284,0,31,-6435.43,-1983.85,244.548,0,0,0,''),
(8284,0,32,-6418.38,-1985.78,246.554,0,0,0,''),
(8284,0,33,-6389.78,-1986.54,246.771,0,30000,0,'quest complete'),
(8516,0,1,2603.18,725.259,54.6927,0,0,0,''),
(8516,0,2,2587.13,734.392,55.231,0,0,0,''),
(8516,0,3,2570.69,753.572,54.5855,0,0,0,''),
(8516,0,4,2558.51,747.66,54.4482,0,0,0,''),
(8516,0,5,2544.23,772.924,47.9255,0,0,0,''),
(8516,0,6,2530.08,797.475,45.97,0,0,0,''),
(8516,0,7,2521.83,799.127,44.3061,0,0,0,''),
(8516,0,8,2502.61,789.222,39.5074,0,0,0,''),
(8516,0,9,2495.25,789.406,39.499,0,0,0,''),
(8516,0,10,2488.07,802.455,42.9834,0,0,0,''),
(8516,0,11,2486.64,826.649,43.6363,0,0,0,''),
(8516,0,12,2492.64,835.166,45.1427,0,0,0,''),
(8516,0,13,2505.02,847.564,47.6487,0,0,0,''),
(8516,0,14,2538.96,877.362,47.6781,0,0,0,''),
(8516,0,15,2546.07,885.672,47.6789,0,0,0,''),
(8516,0,16,2548.02,897.584,47.7277,0,0,0,''),
(8516,0,17,2544.29,909.116,46.2506,0,0,0,''),
(8516,0,18,2523.6,920.306,45.8717,0,0,0,''),
(8516,0,19,2522.69,933.546,47.5769,0,0,0,''),
(8516,0,20,2531.63,959.893,49.4111,0,0,0,''),
(8516,0,21,2540.23,973.338,50.1241,0,0,0,''),
(8516,0,22,2547.21,977.489,49.9759,0,0,0,''),
(8516,0,23,2558.75,969.243,50.7353,0,0,0,''),
(8516,0,24,2575.6,950.138,52.846,0,0,0,''),
(8516,0,25,2575.6,950.138,52.846,0,0,0,''),
(9023,0,1,316.336,-225.528,-77.7258,0,2000,0,'SAY_WINDSOR_START'),
(9023,0,2,322.96,-207.13,-77.87,0,0,0,''),
(9023,0,3,281.05,-172.16,-75.12,0,0,0,''),
(9023,0,4,272.19,-139.14,-70.61,0,0,0,''),
(9023,0,5,283.62,-116.09,-70.21,0,0,0,''),
(9023,0,6,296.18,-94.3,-74.08,0,0,0,''),
(9023,0,7,294.57,-93.11,-74.08,0,0,0,'escort paused - SAY_WINDSOR_CELL_DUGHAL_1'),
(9023,0,8,294.57,-93.11,-74.08,0,10000,0,''),
(9023,0,9,294.57,-93.11,-74.08,0,3000,0,'SAY_WINDSOR_CELL_DUGHAL_3'),
(9023,0,10,314.31,-74.31,-76.09,0,0,0,''),
(9023,0,11,360.22,-62.93,-66.77,0,0,0,''),
(9023,0,12,383.38,-69.4,-63.25,0,0,0,''),
(9023,0,13,389.99,-67.86,-62.57,0,0,0,''),
(9023,0,14,400.98,-72.01,-62.31,0,0,0,'SAY_WINDSOR_EQUIPMENT_1'),
(9023,0,15,404.22,-62.3,-63.5,0,2000,0,''),
(9023,0,16,404.22,-62.3,-63.5,0,1500,0,'open supply door'),
(9023,0,17,407.65,-51.86,-63.96,0,0,0,''),
(9023,0,18,403.61,-51.71,-63.92,0,1000,0,'SAY_WINDSOR_EQUIPMENT_2'),
(9023,0,19,403.61,-51.71,-63.92,0,2000,0,''),
(9023,0,20,403.61,-51.71,-63.92,0,1000,0,'open supply crate'),
(9023,0,21,403.61,-51.71,-63.92,0,1000,0,'update entry to Reginald Windsor'),
(9023,0,22,403.61,-52.71,-63.92,0,4000,0,'SAY_WINDSOR_EQUIPMENT_3'),
(9023,0,23,403.61,-52.71,-63.92,0,4000,0,'SAY_WINDSOR_EQUIPMENT_4'),
(9023,0,24,406.33,-54.87,-63.95,0,0,0,''),
(9023,0,25,403.86,-73.88,-62.02,0,0,0,''),
(9023,0,26,428.8,-81.34,-64.91,0,0,0,''),
(9023,0,27,557.03,-119.71,-61.83,0,0,0,''),
(9023,0,28,573.4,-124.39,-65.07,0,0,0,''),
(9023,0,29,593.91,-130.29,-69.25,0,0,0,''),
(9023,0,30,593.21,-132.16,-69.25,0,0,0,'escort paused - SAY_WINDSOR_CELL_JAZ_1'),
(9023,0,31,593.21,-132.16,-69.25,0,1000,0,''),
(9023,0,32,593.21,-132.16,-69.25,0,3000,0,'SAY_WINDSOR_CELL_JAZ_2'),
(9023,0,33,622.81,-135.55,-71.92,0,0,0,''),
(9023,0,34,634.68,-151.29,-70.32,0,0,0,''),
(9023,0,35,635.06,-153.25,-70.32,0,0,0,'escort paused - SAY_WINDSOR_CELL_SHILL_1'),
(9023,0,36,635.06,-153.25,-70.32,0,3000,0,''),
(9023,0,37,635.06,-153.25,-70.32,0,5000,0,'SAY_WINDSOR_CELL_SHILL_2'),
(9023,0,38,635.06,-153.25,-70.32,0,2000,0,'SAY_WINDSOR_CELL_SHILL_3'),
(9023,0,39,655.25,-172.39,-73.72,0,0,0,''),
(9023,0,40,654.79,-226.3,-83.06,0,0,0,''),
(9023,0,41,622.85,-268.85,-83.96,0,0,0,''),
(9023,0,42,579.45,-275.56,-80.44,0,0,0,''),
(9023,0,43,561.19,-266.85,-75.59,0,0,0,''),
(9023,0,44,547.91,-253.92,-70.34,0,0,0,''),
(9023,0,45,549.2,-252.4,-70.34,0,0,0,'escort paused - SAY_WINDSOR_CELL_CREST_1'),
(9023,0,46,549.2,-252.4,-70.34,0,1000,0,''),
(9023,0,47,549.2,-252.4,-70.34,0,4000,0,'SAY_WINDSOR_CELL_CREST_2'),
(9023,0,48,555.33,-269.16,-74.4,0,0,0,''),
(9023,0,49,554.31,-270.88,-74.4,0,0,0,'escort paused - SAY_WINDSOR_CELL_TOBIAS_1'),
(9023,0,50,554.31,-270.88,-74.4,0,10000,0,''),
(9023,0,51,554.31,-270.88,-74.4,0,4000,0,'SAY_WINDSOR_CELL_TOBIAS_2'),
(9023,0,52,536.1,-249.6,-67.47,0,0,0,''),
(9023,0,53,520.94,-216.65,-59.28,0,0,0,''),
(9023,0,54,505.99,-148.74,-62.17,0,0,0,''),
(9023,0,55,484.21,-56.24,-62.43,0,0,0,''),
(9023,0,56,470.39,-6.01,-70.1,0,0,0,''),
(9023,0,57,452.45,29.85,-70.37,0,1500,0,'SAY_WINDSOR_FREE_1'),
(9023,0,58,452.45,29.85,-70.37,0,15000,0,'SAY_WINDSOR_FREE_2'),
(9502,0,0,847.848,-230.067,-43.614,0,0,0,''),
(9502,0,1,868.122,-223.884,-43.695,0,0,0,'YELL_PHALANX_AGGRO'),
(9503,0,0,885.185,-194.007,-43.4584,0,0,0,''),
(9503,0,1,885.185,-194.007,-43.4584,0,0,0,''),
(9503,0,2,872.764,-185.606,-43.7037,0,5000,0,'b1'),
(9503,0,3,867.923,-188.006,-43.7037,0,5000,0,'b2'),
(9503,0,4,863.296,-190.795,-43.7037,0,5000,0,'b3'),
(9503,0,5,856.14,-194.653,-43.7037,0,5000,0,'b4'),
(9503,0,6,851.879,-196.928,-43.7037,0,15000,0,'b5'),
(9503,0,7,877.035,-187.048,-43.7037,0,0,0,''),
(9503,0,8,891.198,-197.924,-43.6204,0,0,0,'home'),
(9503,0,9,876.935,-189.007,-43.4584,0,0,0,'Nagmara escort'),
(9503,0,10,885.185,-194.007,-43.4584,0,0,0,''),
(9503,0,11,869.124,-202.852,-43.7088,0,0,0,''),
(9503,0,12,869.465,-202.878,-43.4588,0,0,0,''),
(9503,0,13,864.244,-210.826,-43.459,0,0,0,''),
(9503,0,14,866.824,-220.959,-43.4472,0,0,0,''),
(9503,0,15,867.074,-221.959,-43.4472,0,0,0,''),
(9503,0,16,870.419,-225.675,-43.5566,0,0,0,'open door'),
(9503,0,17,872.169,-227.425,-43.5566,0,0,0,''),
(9503,0,18,872.919,-228.175,-43.5566,0,0,0,''),
(9503,0,19,875.919,-230.925,-43.5566,0,0,0,''),
(9503,0,20,876.919,-230.175,-43.5566,0,0,0,''),
(9503,0,21,877.919,-229.425,-43.5566,0,0,0,''),
(9503,0,22,882.395,-225.949,-46.7405,0,0,0,''),
(9503,0,23,885.895,-223.699,-49.2405,0,0,0,''),
(9503,0,24,887.645,-222.449,-49.2405,0,0,0,''),
(9503,0,25,885.937,-223.351,-49.2954,0,0,0,''),
(9503,0,26,887.437,-222.351,-49.2954,0,0,0,''),
(9503,0,27,888.937,-221.601,-49.5454,0,0,0,''),
(9503,0,28,887.687,-220.101,-49.5454,0,0,0,''),
(9503,0,29,886.687,-218.851,-49.5454,0,0,0,''),
(9503,0,30,887.567,-220.04,-49.7059,0,0,0,''),
(9503,0,31,886.567,-218.79,-49.7059,0,0,0,''),
(9503,0,32,886.067,-218.29,-49.7059,0,0,0,''),
(9503,0,33,880.825,-221.389,-49.9562,0,0,0,'stop'),
(9520,0,1,-7699.62,-1444.29,139.87,0,4000,0,'SAY_START'),
(9520,0,2,-7670.67,-1458.25,140.74,0,0,0,''),
(9520,0,3,-7675.26,-1465.58,140.74,0,0,0,''),
(9520,0,4,-7685.84,-1472.66,140.75,0,0,0,''),
(9520,0,5,-7700.08,-1473.41,140.79,0,0,0,''),
(9520,0,6,-7712.55,-1470.19,140.79,0,0,0,''),
(9520,0,7,-7717.27,-1481.7,140.72,0,5000,0,'SAY_PAY'),
(9520,0,8,-7726.23,-1500.78,132.99,0,0,0,''),
(9520,0,9,-7744.61,-1531.61,132.69,0,0,0,''),
(9520,0,10,-7763.08,-1536.22,131.93,0,0,0,''),
(9520,0,11,-7815.32,-1522.61,134.16,0,0,0,''),
(9520,0,12,-7850.26,-1516.87,138.17,0,0,0,'SAY_FIRST_AMBUSH_START'),
(9520,0,13,-7850.26,-1516.87,138.17,0,3000,0,'SAY_FIRST_AMBUSH_END'),
(9520,0,14,-7881.01,-1508.49,142.37,0,0,0,''),
(9520,0,15,-7888.91,-1458.09,144.79,0,0,0,''),
(9520,0,16,-7889.18,-1430.21,145.31,0,0,0,''),
(9520,0,17,-7900.53,-1427.01,150.26,0,0,0,''),
(9520,0,18,-7904.15,-1429.91,150.27,0,0,0,''),
(9520,0,19,-7921.48,-1425.47,140.54,0,0,0,''),
(9520,0,20,-7941.43,-1413.1,134.35,0,0,0,''),
(9520,0,21,-7964.85,-1367.45,132.99,0,0,0,''),
(9520,0,22,-7989.95,-1319.12,133.71,0,0,0,''),
(9520,0,23,-8010.43,-1270.23,133.42,0,0,0,''),
(9520,0,24,-8025.62,-1243.78,133.91,0,0,0,'SAY_SEC_AMBUSH_START'),
(9520,0,25,-8025.62,-1243.78,133.91,0,3000,0,'SAY_SEC_AMBUSH_END'),
(9520,0,26,-8015.22,-1196.98,146.76,0,0,0,''),
(9520,0,27,-7994.68,-1151.38,160.7,0,0,0,''),
(9520,0,28,-7970.91,-1132.81,170.16,0,0,0,'summon Searscale Drakes'),
(9520,0,29,-7927.59,-1122.79,185.86,0,0,0,''),
(9520,0,30,-7897.67,-1126.67,194.32,0,0,0,'SAY_THIRD_AMBUSH_START'),
(9520,0,31,-7897.67,-1126.67,194.32,0,3000,0,'SAY_THIRD_AMBUSH_END'),
(9520,0,32,-7864.11,-1135.98,203.29,0,0,0,''),
(9520,0,33,-7837.31,-1137.73,209.63,0,0,0,''),
(9520,0,34,-7808.72,-1134.9,214.84,0,0,0,''),
(9520,0,35,-7786.85,-1127.24,214.84,0,0,0,''),
(9520,0,36,-7746.58,-1125.16,215.08,0,5000,0,'EMOTE_LAUGH'),
(9520,0,37,-7746.41,-1103.62,215.62,0,0,0,''),
(9520,0,38,-7740.25,-1090.51,216.69,0,0,0,''),
(9520,0,39,-7730.97,-1085.55,217.12,0,0,0,''),
(9520,0,40,-7697.89,-1089.43,217.62,0,0,0,''),
(9520,0,41,-7679.3,-1059.15,220.09,0,0,0,''),
(9520,0,42,-7661.39,-1038.24,226.24,0,0,0,''),
(9520,0,43,-7634.49,-1020.96,234.3,0,0,0,''),
(9520,0,44,-7596.22,-1013.16,244.03,0,0,0,''),
(9520,0,45,-7556.53,-1021.74,253.21,0,0,0,'SAY_LAST_STAND'),
(9537,0,0,854.977,-150.308,-49.671,0,0,0,''),
(9537,0,1,855.816,-149.763,-49.671,0,0,0,'YELL_HURLEY_SPAWN'),
(9537,0,2,882.63,-148.166,-49.7597,0,0,0,''),
(9537,0,3,883.96,-148.087,-49.76,0,0,0,''),
(9537,0,4,896.846,-147.319,-49.7575,0,0,0,''),
(9537,0,5,896.846,-147.319,-49.7575,0,0,0,''),
(9598,0,0,6004.27,-1180.49,376.377,0,0,0,'SAY_ESCORT_START'),
(9598,0,1,6002.51,-1157.29,381.407,0,0,0,''),
(9598,0,2,6029.23,-1139.72,383.127,0,0,0,''),
(9598,0,3,6042.48,-1128.96,386.582,0,0,0,''),
(9598,0,4,6062.82,-1115.52,386.85,0,0,0,''),
(9598,0,5,6089.19,-1111.91,383.105,0,0,0,''),
(9598,0,6,6104.39,-1114.56,380.49,0,0,0,''),
(9598,0,7,6115.08,-1128.57,375.779,0,0,0,''),
(9598,0,8,6119.35,-1147.31,372.518,0,0,0,''),
(9598,0,9,6119,-1176.08,371.072,0,0,0,''),
(9598,0,10,6120.98,-1198.41,373.432,0,0,0,''),
(9598,0,11,6123.52,-1226.64,374.119,0,0,0,''),
(9598,0,12,6127.74,-1246.04,373.574,0,0,0,''),
(9598,0,13,6133.43,-1253.64,369.1,0,0,0,''),
(9598,0,14,6150.79,-1269.15,369.24,0,0,0,''),
(9598,0,15,6155.81,-1275.03,373.627,0,0,0,''),
(9598,0,16,6163.54,-1307.13,376.234,0,0,0,''),
(9598,0,17,6174.8,-1340.89,379.039,0,0,0,''),
(9598,0,18,6191.14,-1371.26,378.453,0,0,0,''),
(9598,0,19,6215.65,-1397.52,376.012,0,0,0,''),
(9598,0,20,6243.78,-1407.68,371.594,0,0,0,''),
(9598,0,21,6280.77,-1408.26,370.554,0,0,0,''),
(9598,0,22,6325.36,-1406.69,370.082,0,0,0,''),
(9598,0,23,6355,-1404.34,370.646,0,0,0,''),
(9598,0,24,6374.68,-1399.18,372.105,0,0,0,''),
(9598,0,25,6395.8,-1367.06,374.91,0,0,0,''),
(9598,0,26,6408.57,-1333.49,376.616,0,0,0,''),
(9598,0,27,6409.08,-1312.17,379.598,0,0,0,''),
(9598,0,28,6418.69,-1277.7,381.638,0,0,0,''),
(9598,0,29,6441.69,-1247.32,387.246,0,0,0,''),
(9598,0,30,6462.14,-1226.32,397.61,0,0,0,''),
(9598,0,31,6478.02,-1216.26,408.284,0,0,0,''),
(9598,0,32,6499.63,-1217.09,419.461,0,0,0,''),
(9598,0,33,6523.17,-1220.78,430.549,0,0,0,''),
(9598,0,34,6542.72,-1217,437.788,0,0,0,''),
(9598,0,35,6557.28,-1211.12,441.452,0,0,0,''),
(9598,0,36,6574.57,-1204.59,443.216,0,0,0,'SAY_EXIT_IRONTREE'),
(9623,0,0,-6383.07,-1964.37,-258.709,0,0,0,'SAY_AME_START'),
(9623,0,1,-6393.65,-1949.57,-261.449,0,0,0,''),
(9623,0,2,-6397.85,-1931.1,-263.366,0,0,0,''),
(9623,0,3,-6397.5,-1921.47,-263.876,0,0,0,''),
(9623,0,4,-6389.63,-1910,-259.601,0,0,0,''),
(9623,0,5,-6380.07,-1905.45,-255.858,0,0,0,''),
(9623,0,6,-6373.44,-1900.28,-254.774,0,0,0,''),
(9623,0,7,-6372.87,-1893.5,-255.678,0,0,0,''),
(9623,0,8,-6379.73,-1877.63,-259.654,0,0,0,''),
(9623,0,9,-6380.26,-1871.14,-260.617,0,0,0,''),
(9623,0,10,-6373.83,-1855.62,-259.566,0,0,0,''),
(9623,0,11,-6368.82,-1847.77,-259.246,0,0,0,''),
(9623,0,12,-6370.9,-1835.04,-260.212,0,0,0,''),
(9623,0,13,-6376.59,-1821.59,-260.856,0,0,0,''),
(9623,0,14,-6381.93,-1810.43,-266.18,0,0,0,''),
(9623,0,15,-6396.71,-1807.12,-269.329,0,0,0,''),
(9623,0,16,-6400.27,-1795.05,-269.744,0,0,0,''),
(9623,0,17,-6402.68,-1747.51,-272.961,0,0,0,''),
(9623,0,18,-6397,-1710.05,-273.719,0,0,0,''),
(9623,0,19,-6388.11,-1676.33,-272.133,0,5000,0,'SAY_AME_PROGRESS'),
(9623,0,20,-6370.71,-1638.64,-272.031,0,0,0,''),
(9623,0,21,-6366.71,-1592.65,-272.201,0,0,0,''),
(9623,0,22,-6333.87,-1534.6,-270.493,0,0,0,''),
(9623,0,23,-6305.36,-1477.91,-269.518,0,0,0,''),
(9623,0,24,-6311.59,-1419.02,-267.622,0,0,0,''),
(9623,0,25,-6330.01,-1400.06,-266.425,0,0,0,''),
(9623,0,26,-6356.02,-1392.61,-267.123,0,0,0,''),
(9623,0,27,-6370.86,-1386.18,-270.218,0,0,0,''),
(9623,0,28,-6381.53,-1369.78,-272.11,0,0,0,''),
(9623,0,29,-6405.38,-1321.52,-271.699,0,0,0,''),
(9623,0,30,-6406.58,-1307.57,-271.802,0,0,0,''),
(9623,0,31,-6386.33,-1286.85,-272.074,0,0,0,''),
(9623,0,32,-6364.25,-1264.71,-269.075,0,0,0,''),
(9623,0,33,-6343.64,-1239.84,-268.364,0,0,0,''),
(9623,0,34,-6335.57,-1202.45,-271.515,0,0,0,''),
(9623,0,35,-6325.62,-1184.46,-270.461,0,0,0,''),
(9623,0,36,-6317.8,-1177.67,-269.792,0,0,0,''),
(9623,0,37,-6303.02,-1180.25,-269.332,0,0,0,'SAY_AME_END'),
(9623,0,38,-6301.98,-1184.79,-269.371,0,1000,0,''),
(9623,0,39,-6297.58,-1186.41,-268.962,0,5000,0,''),
(10096,0,0,604.803,-191.082,-54.0586,0,0,0,'ring'),
(10096,0,1,604.073,-222.107,-52.7438,0,0,0,'first gate'),
(10096,0,2,621.4,-214.499,-52.8145,0,0,0,'hiding in corner'),
(10096,0,3,601.301,-198.557,-53.9503,0,0,0,'ring'),
(10096,0,4,631.818,-180.548,-52.6548,0,0,0,'second gate'),
(10096,0,5,627.39,-201.076,-52.6929,0,0,0,'hiding in corner'),
(10300,0,1,5728.81,-4801.15,778.18,0,0,0,''),
(10300,0,2,5730.22,-4818.34,777.11,0,0,0,''),
(10300,0,3,5728.12,-4835.76,778.15,0,1000,0,'SAY_ENTER_OWL_THICKET'),
(10300,0,4,5718.85,-4865.62,787.56,0,0,0,''),
(10300,0,5,5697.13,-4909.12,801.53,0,0,0,''),
(10300,0,6,5684.2,-4913.75,801.6,0,0,0,''),
(10300,0,7,5674.67,-4915.78,802.13,0,0,0,''),
(10300,0,8,5665.61,-4919.22,804.85,0,0,0,''),
(10300,0,9,5638.22,-4897.58,804.97,0,0,0,''),
(10300,0,10,5632.67,-4892.05,805.44,0,0,0,'Cavern 1 - EMOTE_CHANT_SPELL'),
(10300,0,11,5664.58,-4921.84,804.91,0,0,0,''),
(10300,0,12,5684.21,-4943.81,802.8,0,0,0,''),
(10300,0,13,5724.92,-4983.69,808.25,0,0,0,''),
(10300,0,14,5753.39,-4990.73,809.84,0,0,0,''),
(10300,0,15,5765.62,-4994.89,809.42,0,0,0,'Cavern 2 - EMOTE_CHANT_SPELL'),
(10300,0,16,5724.94,-4983.58,808.29,0,0,0,''),
(10300,0,17,5699.61,-4989.82,808.03,0,0,0,''),
(10300,0,18,5686.8,-5012.17,807.27,0,0,0,''),
(10300,0,19,5691.43,-5037.43,807.73,0,0,0,''),
(10300,0,20,5694.24,-5054.64,808.85,0,0,0,'Cavern 3 - EMOTE_CHANT_SPELL'),
(10300,0,21,5686.88,-5012.18,807.23,0,0,0,''),
(10300,0,22,5664.94,-5001.12,807.78,0,0,0,''),
(10300,0,23,5647.12,-5002.84,807.54,0,0,0,''),
(10300,0,24,5629.23,-5014.88,807.94,0,0,0,''),
(10300,0,25,5611.26,-5025.62,808.36,0,0,0,'Cavern 4 - EMOTE_CHANT_SPELL'),
(10300,0,26,5647.13,-5002.85,807.57,0,0,0,''),
(10300,0,27,5641.12,-4973.22,809.39,0,0,0,''),
(10300,0,28,5622.97,-4953.58,811.12,0,0,0,''),
(10300,0,29,5601.52,-4939.49,820.77,0,0,0,''),
(10300,0,30,5571.87,-4936.22,831.35,0,0,0,''),
(10300,0,31,5543.23,-4933.67,838.33,0,0,0,''),
(10300,0,32,5520.86,-4942.05,843.02,0,0,0,''),
(10300,0,33,5509.15,-4946.31,849.36,0,0,0,''),
(10300,0,34,5498.45,-4950.08,849.98,0,0,0,''),
(10300,0,35,5485.78,-4963.4,850.43,0,0,0,''),
(10300,0,36,5467.92,-4980.67,851.89,0,0,0,'Cavern 5 - EMOTE_CHANT_SPELL'),
(10300,0,37,5498.68,-4950.45,849.96,0,0,0,''),
(10300,0,38,5518.68,-4921.94,844.65,0,0,0,''),
(10300,0,39,5517.66,-4920.82,845.12,0,0,0,'SAY_REACH_ALTAR_1'),
(10300,0,40,5518.38,-4913.47,845.57,0,0,0,''),
(10300,0,41,5511.31,-4913.82,847.17,0,5000,0,'light the spotlights'),
(10300,0,42,5511.31,-4913.82,847.17,0,0,0,'start altar cinematic - SAY_RANSHALLA_ALTAR_2'),
(10300,0,43,5510.36,-4921.17,846.33,0,0,0,''),
(10300,0,44,5517.66,-4920.82,845.12,0,0,0,'escort paused'),
(10427,0,0,-5185.46,-1185.93,45.951,0,0,0,''),
(10427,0,1,-5184.88,-1154.21,45.035,0,0,0,''),
(10427,0,2,-5175.88,-1126.53,43.701,0,0,0,''),
(10427,0,3,-5138.65,-1111.87,44.024,0,0,0,''),
(10427,0,4,-5134.73,-1104.8,47.365,0,0,0,''),
(10427,0,5,-5129.68,-1097.88,49.449,0,2500,0,''),
(10427,0,6,-5125.3,-1080.57,47.033,0,0,0,''),
(10427,0,7,-5146.67,-1053.69,28.415,0,0,0,''),
(10427,0,8,-5147.46,-1027.54,13.818,0,0,0,''),
(10427,0,9,-5139.24,-1018.89,8.22,0,0,0,''),
(10427,0,10,-5121.17,-1013.13,-0.619,0,0,0,''),
(10427,0,11,-5091.92,-1014.21,-4.902,0,0,0,''),
(10427,0,12,-5069.24,-994.299,-4.631,0,0,0,''),
(10427,0,13,-5059.98,-944.112,-5.377,0,0,0,''),
(10427,0,14,-5013.55,-906.184,-5.49,0,0,0,''),
(10427,0,15,-4992.46,-920.983,-4.98,0,5000,0,'SAY_WYVERN'),
(10427,0,16,-4976.35,-1003,-5.38,0,0,0,''),
(10427,0,17,-4958.48,-1033.19,-5.433,0,0,0,''),
(10427,0,18,-4953.35,-1052.21,-10.836,0,0,0,''),
(10427,0,19,-4937.45,-1056.35,-22.139,0,0,0,''),
(10427,0,20,-4908.46,-1050.43,-33.458,0,0,0,''),
(10427,0,21,-4905.53,-1056.89,-33.722,0,0,0,''),
(10427,0,22,-4920.83,-1073.28,-45.515,0,0,0,''),
(10427,0,23,-4933.37,-1082.7,-50.186,0,0,0,''),
(10427,0,24,-4935.31,-1092.35,-52.785,0,0,0,''),
(10427,0,25,-4929.55,-1101.27,-50.637,0,0,0,''),
(10427,0,26,-4920.68,-1100.03,-51.944,0,10000,0,'SAY_COMPLETE'),
(10427,0,27,-4920.68,-1100.03,-51.944,0,0,0,'quest complete'),
(10638,0,0,-4903.52,-1368.34,-52.611,0,5000,0,'SAY_KAN_START'),
(10638,0,1,-4906,-1367.05,-52.611,0,0,0,''),
(10646,0,0,-4792.4,-2137.78,82.423,0,0,0,''),
(10646,0,1,-4813.51,-2141.54,80.774,0,0,0,''),
(10646,0,2,-4828.63,-2154.31,82.074,0,0,0,''),
(10646,0,3,-4833.77,-2149.18,81.676,0,0,0,''),
(10646,0,4,-4846.42,-2136.05,77.871,0,0,0,''),
(10646,0,5,-4865.08,-2116.55,76.483,0,0,0,''),
(10646,0,6,-4888.43,-2090.73,80.907,0,0,0,''),
(10646,0,7,-4893.07,-2085.47,82.094,0,0,0,''),
(10646,0,8,-4907.26,-2074.93,84.437,0,5000,0,'SAY_LAKO_LOOK_OUT'),
(10646,0,9,-4899.9,-2062.14,83.78,0,0,0,''),
(10646,0,10,-4897.76,-2056.52,84.184,0,0,0,''),
(10646,0,11,-4888.33,-2033.18,83.654,0,0,0,''),
(10646,0,12,-4876.34,-2003.92,90.887,0,0,0,''),
(10646,0,13,-4872.23,-1994.17,91.513,0,0,0,''),
(10646,0,14,-4879.57,-1976.99,92.185,0,5000,0,'SAY_LAKO_HERE_COME'),
(10646,0,15,-4879.05,-1964.35,92.001,0,0,0,''),
(10646,0,16,-4874.72,-1956.94,90.737,0,0,0,''),
(10646,0,17,-4869.47,-1952.61,89.206,0,0,0,''),
(10646,0,18,-4842.47,-1929,84.147,0,0,0,''),
(10646,0,19,-4804.44,-1897.3,89.362,0,0,0,''),
(10646,0,20,-4798.07,-1892.38,89.368,0,0,0,''),
(10646,0,21,-4779.45,-1882.76,90.169,0,5000,0,'SAY_LAKO_MORE'),
(10646,0,22,-4762.08,-1866.53,89.481,0,0,0,''),
(10646,0,23,-4766.27,-1861.87,87.847,0,0,0,''),
(10646,0,24,-4782.93,-1852.17,78.354,0,0,0,''),
(10646,0,25,-4793.61,-1850.96,77.658,0,0,0,''),
(10646,0,26,-4803.32,-1855.1,78.958,0,0,0,''),
(10646,0,27,-4807.97,-1854.5,77.743,0,0,0,''),
(10646,0,28,-4837.21,-1848.49,64.488,0,0,0,''),
(10646,0,29,-4884.62,-1840.4,56.219,0,0,0,''),
(10646,0,30,-4889.71,-1839.62,54.417,0,0,0,''),
(10646,0,31,-4893.9,-1843.69,53.012,0,0,0,''),
(10646,0,32,-4903.14,-1872.38,32.266,0,0,0,''),
(10646,0,33,-4910.94,-1879.86,29.94,0,0,0,''),
(10646,0,34,-4920.05,-1880.94,30.597,0,0,0,''),
(10646,0,35,-4924.46,-1881.45,29.292,0,0,0,''),
(10646,0,36,-4966.12,-1886.03,10.977,0,0,0,''),
(10646,0,37,-4999.37,-1890.85,4.43,0,0,0,''),
(10646,0,38,-5007.27,-1891.67,2.771,0,0,0,''),
(10646,0,39,-5013.33,-1879.59,-1.947,0,0,0,''),
(10646,0,40,-5023.33,-1855.96,-17.103,0,0,0,''),
(10646,0,41,-5038.51,-1825.99,-35.821,0,0,0,''),
(10646,0,42,-5048.73,-1809.8,-46.457,0,0,0,''),
(10646,0,43,-5053.19,-1791.68,-57.186,0,0,0,''),
(10646,0,44,-5062.09,-1794.4,-56.515,0,0,0,''),
(10646,0,45,-5052.66,-1797.04,-54.734,0,5000,0,'SAY_LAKO_END'),
(11016,0,0,5004.98,-440.237,319.059,0,4000,0,'SAY_ESCORT_START'),
(11016,0,1,4992.22,-449.964,317.057,0,0,0,''),
(11016,0,2,4988.55,-457.438,316.289,0,0,0,''),
(11016,0,3,4989.98,-464.297,316.846,0,0,0,''),
(11016,0,4,4994.04,-467.754,318.055,0,0,0,''),
(11016,0,5,5002.31,-466.318,319.965,0,0,0,''),
(11016,0,6,5011.8,-462.889,321.501,0,0,0,''),
(11016,0,7,5020.53,-460.797,321.97,0,0,0,''),
(11016,0,8,5026.84,-463.171,321.345,0,0,0,''),
(11016,0,9,5028.66,-476.805,318.726,0,0,0,''),
(11016,0,10,5029.5,-487.131,318.179,0,0,0,''),
(11016,0,11,5031.18,-497.678,316.533,0,0,0,''),
(11016,0,12,5032.72,-504.748,314.744,0,0,0,''),
(11016,0,13,5035,-513.138,314.372,0,0,0,''),
(11016,0,14,5037.49,-521.733,313.221,0,6000,0,'SAY_FIRST_STOP'),
(11016,0,15,5049.06,-519.546,313.221,0,0,0,''),
(11016,0,16,5059.17,-522.93,313.221,0,0,0,''),
(11016,0,17,5062.75,-529.933,313.221,0,0,0,''),
(11016,0,18,5063.9,-538.827,313.221,0,0,0,''),
(11016,0,19,5062.22,-545.635,313.221,0,0,0,''),
(11016,0,20,5061.69,-552.333,313.101,0,0,0,''),
(11016,0,21,5060.33,-560.349,310.873,0,0,0,''),
(11016,0,22,5055.62,-565.541,308.737,0,0,0,''),
(11016,0,23,5049.8,-567.604,306.537,0,0,0,''),
(11016,0,24,5043.01,-564.946,303.682,0,0,0,''),
(11016,0,25,5038.22,-559.823,301.463,0,0,0,''),
(11016,0,26,5039.46,-548.675,297.824,0,0,0,''),
(11016,0,27,5043.44,-538.807,297.801,0,0,0,''),
(11016,0,28,5056.4,-528.954,297.801,0,0,0,''),
(11016,0,29,5064.4,-521.904,297.801,0,0,0,''),
(11016,0,30,5067.62,-512.999,297.196,0,0,0,''),
(11016,0,31,5065.99,-505.329,297.214,0,0,0,''),
(11016,0,32,5062.24,-499.086,297.448,0,0,0,''),
(11016,0,33,5065.09,-492.069,298.054,0,0,0,''),
(11016,0,34,5071.19,-491.173,297.666,0,5000,0,'SAY_SECOND_STOP'),
(11016,0,35,5087.47,-496.478,296.677,0,0,0,''),
(11016,0,36,5095.55,-508.639,296.677,0,0,0,''),
(11016,0,37,5104.3,-521.014,296.677,0,0,0,''),
(11016,0,38,5110.13,-532.123,296.677,0,4000,0,'open equipment chest'),
(11016,0,39,5110.13,-532.123,296.677,0,4000,0,'cast SPELL_STRENGHT_ARKONARIN'),
(11016,0,40,5110.13,-532.123,296.677,0,4000,0,'SAY_EQUIPMENT'),
(11016,0,41,5110.13,-532.123,296.677,0,0,0,'SAY_ESCAPE'),
(11016,0,42,5099.75,-510.823,296.677,0,0,0,''),
(11016,0,43,5091.94,-497.516,296.677,0,0,0,''),
(11016,0,44,5079.38,-486.811,297.638,0,0,0,''),
(11016,0,45,5069.21,-488.77,298.082,0,0,0,''),
(11016,0,46,5064.24,-496.051,297.275,0,0,0,''),
(11016,0,47,5065.08,-505.239,297.361,0,0,0,''),
(11016,0,48,5067.82,-515.245,297.125,0,0,0,''),
(11016,0,49,5064.62,-521.17,297.801,0,0,0,''),
(11016,0,50,5053.22,-530.739,297.801,0,0,0,''),
(11016,0,51,5045.73,-538.311,297.801,0,0,0,''),
(11016,0,52,5039.69,-548.112,297.801,0,0,0,''),
(11016,0,53,5038.78,-557.588,300.787,0,0,0,''),
(11016,0,54,5042.01,-566.749,303.838,0,0,0,''),
(11016,0,55,5050.56,-568.149,306.782,0,0,0,''),
(11016,0,56,5056.98,-564.674,309.342,0,0,0,''),
(11016,0,57,5060.79,-556.801,311.936,0,0,0,''),
(11016,0,58,5059.58,-551.626,313.221,0,0,0,''),
(11016,0,59,5062.83,-541.994,313.221,0,0,0,''),
(11016,0,60,5063.55,-531.288,313.221,0,0,0,''),
(11016,0,61,5057.93,-523.088,313.221,0,0,0,''),
(11016,0,62,5049.47,-519.36,313.221,0,0,0,''),
(11016,0,63,5040.79,-519.809,313.221,0,0,0,''),
(11016,0,64,5034.3,-515.361,313.948,0,0,0,''),
(11016,0,65,5032,-505.532,314.663,0,0,0,''),
(11016,0,66,5029.92,-495.645,316.821,0,0,0,''),
(11016,0,67,5028.87,-487,318.179,0,0,0,''),
(11016,0,68,5028.11,-475.531,318.839,0,0,0,''),
(11016,0,69,5027.76,-465.442,320.643,0,0,0,''),
(11016,0,70,5019.96,-460.892,321.969,0,0,0,''),
(11016,0,71,5009.43,-464.793,321.248,0,0,0,''),
(11016,0,72,4999.57,-468.062,319.426,0,0,0,''),
(11016,0,73,4992.03,-468.128,317.894,0,0,0,''),
(11016,0,74,4988.17,-461.293,316.369,0,0,0,''),
(11016,0,75,4990.62,-447.459,317.104,0,0,0,''),
(11016,0,76,4993.48,-438.643,318.272,0,0,0,''),
(11016,0,77,4995.45,-430.178,318.462,0,0,0,''),
(11016,0,78,4993.56,-422.876,318.864,0,0,0,''),
(11016,0,79,4985.4,-420.864,320.205,0,0,0,''),
(11016,0,80,4976.52,-426.168,323.112,0,0,0,''),
(11016,0,81,4969.83,-429.755,325.029,0,0,0,''),
(11016,0,82,4960.7,-425.44,325.834,0,0,0,''),
(11016,0,83,4955.45,-418.765,327.433,0,0,0,''),
(11016,0,84,4949.7,-408.796,328.004,0,0,0,''),
(11016,0,85,4940.02,-403.222,329.956,0,0,0,''),
(11016,0,86,4934.98,-401.475,330.898,0,0,0,''),
(11016,0,87,4928.69,-399.302,331.744,0,0,0,''),
(11016,0,88,4926.94,-398.436,333.079,0,0,0,''),
(11016,0,89,4916.16,-393.822,333.729,0,0,0,''),
(11016,0,90,4908.39,-396.217,333.217,0,0,0,''),
(11016,0,91,4905.61,-396.535,335.05,0,0,0,''),
(11016,0,92,4897.88,-395.245,337.346,0,0,0,''),
(11016,0,93,4895.21,-388.203,339.295,0,0,0,''),
(11016,0,94,4896.94,-382.429,341.04,0,0,0,''),
(11016,0,95,4901.88,-378.799,342.771,0,0,0,''),
(11016,0,96,4908.09,-380.635,344.597,0,0,0,''),
(11016,0,97,4911.91,-385.818,346.491,0,0,0,''),
(11016,0,98,4910.1,-393.444,348.798,0,0,0,''),
(11016,0,99,4903.5,-396.947,350.812,0,0,0,''),
(11016,0,100,4898.08,-394.226,351.821,0,0,0,''),
(11016,0,101,4891.33,-393.436,351.801,0,0,0,''),
(11016,0,102,4881.2,-395.211,351.59,0,0,0,''),
(11016,0,103,4877.84,-395.536,349.713,0,0,0,''),
(11016,0,104,4873.97,-394.919,349.844,0,5000,0,'SAY_FRESH_AIR'),
(11016,0,105,4873.97,-394.919,349.844,0,3000,0,'SAY_BETRAYER'),
(11016,0,106,4873.97,-394.919,349.844,0,2000,0,'SAY_TREY'),
(11016,0,107,4873.97,-394.919,349.844,0,0,0,'SAY_ATTACK_TREY'),
(11016,0,108,4873.97,-394.919,349.844,0,5000,0,'SAY_ESCORT_COMPLETE'),
(11016,0,109,4873.97,-394.919,349.844,0,1000,0,''),
(11016,0,110,4863.02,-394.521,350.65,0,0,0,''),
(11016,0,111,4848.7,-397.612,351.215,0,0,0,''),
(11625,0,0,-1802.8,2194.02,59.8226,0,0,0,''),
(11625,0,1,-1800.41,2180.08,59.8226,0,0,0,''),
(11625,0,2,-1800.26,2166.08,60.1822,0,0,0,''),
(11625,0,3,-1801.62,2148.85,62.344,0,0,0,''),
(11625,0,4,-1801.15,2134.86,63.1766,0,0,0,''),
(11625,0,5,-1802.26,2110.11,63.6737,0,0,0,''),
(11625,0,6,-1805.87,2096.6,63.1784,0,0,0,''),
(11625,0,7,-1809.25,2083.01,63.0772,0,0,0,''),
(11625,0,8,-1812.63,2069.43,63.043,0,0,0,''),
(11625,0,9,-1816.72,2056.04,61.8496,0,0,0,'FIRST_AMBUSH'),
(11625,0,10,-1822.89,2032.23,60.6524,0,0,0,''),
(11625,0,11,-1822.76,2027.39,60.3783,0,0,0,''),
(11625,0,12,-1815.56,2003.46,59.4022,0,0,0,''),
(11625,0,13,-1814.41,1983.18,58.9549,0,0,0,''),
(11625,0,14,-1811.8,1967.01,59.4735,0,0,0,''),
(11625,0,15,-1803.12,1951.78,60.7154,0,0,0,''),
(11625,0,16,-1793.24,1941.87,60.8439,0,0,0,'SECOND_AMBUSH'),
(11625,0,17,-1775.92,1926.82,59.3033,0,0,0,''),
(11625,0,18,-1759.93,1918.92,58.9613,0,0,0,''),
(11625,0,19,-1751.9,1917.2,59.0003,0,0,0,''),
(11625,0,20,-1737.91,1917.04,59.0673,0,0,0,''),
(11625,0,21,-1712.18,1914.85,60.4394,0,0,0,''),
(11625,0,22,-1701.72,1911.02,61.0949,0,0,0,''),
(11625,0,23,-1694.06,1904.03,61.03,0,0,0,''),
(11625,0,24,-1687.1,1886.34,59.7501,0,0,0,''),
(11625,0,25,-1684.12,1872.66,59.0354,0,0,0,''),
(11625,0,26,-1673.14,1845.28,58.9273,0,0,0,'THIRD_AMBUSH'),
(11625,0,27,-1657.63,1821.97,58.9273,0,0,0,''),
(11625,0,28,-1649.83,1810.34,58.9273,0,0,0,''),
(11625,0,29,-1634.24,1787.08,58.9252,0,0,0,''),
(11625,0,30,-1626.45,1775.45,58.9252,0,0,0,''),
(11625,0,31,-1605.77,1750.66,58.9256,0,0,0,''),
(11625,0,32,-1594.91,1741.83,58.9256,0,0,0,'QUEST_COMPLETION'),
(11626,0,0,-1802.8,2194.02,59.8226,0,0,0,''),
(11626,0,1,-1800.41,2180.08,59.8226,0,0,0,''),
(11626,0,2,-1800.26,2166.08,60.1822,0,0,0,''),
(11626,0,3,-1801.62,2148.85,62.344,0,0,0,''),
(11626,0,4,-1801.15,2134.86,63.1766,0,0,0,''),
(11626,0,5,-1802.26,2110.11,63.6737,0,0,0,''),
(11626,0,6,-1805.87,2096.6,63.1784,0,0,0,''),
(11626,0,7,-1809.25,2083.01,63.0772,0,0,0,''),
(11626,0,8,-1812.63,2069.43,63.043,0,0,0,''),
(11626,0,9,-1816.72,2056.04,61.8496,0,0,0,'FIRST_AMBUSH'),
(11626,0,10,-1822.89,2032.23,60.6524,0,0,0,''),
(11626,0,11,-1822.76,2027.39,60.3783,0,0,0,''),
(11626,0,12,-1815.56,2003.46,59.4022,0,0,0,''),
(11626,0,13,-1814.41,1983.18,58.9549,0,0,0,''),
(11626,0,14,-1811.8,1967.01,59.4735,0,0,0,''),
(11626,0,15,-1803.12,1951.78,60.7154,0,0,0,''),
(11626,0,16,-1793.24,1941.87,60.8439,0,0,0,'SECOND_AMBUSH'),
(11626,0,17,-1775.92,1926.82,59.3033,0,0,0,''),
(11626,0,18,-1759.93,1918.92,58.9613,0,0,0,''),
(11626,0,19,-1751.9,1917.2,59.0003,0,0,0,''),
(11626,0,20,-1737.91,1917.04,59.0673,0,0,0,''),
(11626,0,21,-1712.18,1914.85,60.4394,0,0,0,''),
(11626,0,22,-1701.72,1911.02,61.0949,0,0,0,''),
(11626,0,23,-1694.06,1904.03,61.03,0,0,0,''),
(11626,0,24,-1687.1,1886.34,59.7501,0,0,0,''),
(11626,0,25,-1684.12,1872.66,59.0354,0,0,0,''),
(11626,0,26,-1673.14,1845.28,58.9273,0,0,0,'THIRD_AMBUSH'),
(11626,0,27,-1657.63,1821.97,58.9273,0,0,0,''),
(11626,0,28,-1649.83,1810.34,58.9273,0,0,0,''),
(11626,0,29,-1634.24,1787.08,58.9252,0,0,0,''),
(11626,0,30,-1626.45,1775.45,58.9252,0,0,0,''),
(11626,0,31,-1605.77,1750.66,58.9256,0,0,0,''),
(11626,0,32,-1594.91,1741.83,58.9256,0,0,0,'QUEST_COMPLETION'),
(11832,0,0,7848.39,-2216.36,470.888,0,15000,0,'SAY_REMULOS_INTRO_1'),
(11832,0,1,7848.39,-2216.36,470.888,0,5000,0,'SAY_REMULOS_INTRO_2'),
(11832,0,2,7829.79,-2244.84,463.853,0,0,0,''),
(11832,0,3,7819.01,-2304.34,455.957,0,0,0,''),
(11832,0,4,7931.1,-2314.35,473.054,0,0,0,''),
(11832,0,5,7943.55,-2324.69,477.677,0,0,0,''),
(11832,0,6,7952.02,-2351.14,485.235,0,0,0,''),
(11832,0,7,7963.67,-2412.99,488.953,0,0,0,''),
(11832,0,8,7975.18,-2551.6,490.08,0,0,0,''),
(11832,0,9,7948.05,-2570.83,489.751,0,0,0,''),
(11832,0,10,7947.16,-2583.4,490.066,0,0,0,''),
(11832,0,11,7951.09,-2596.22,489.831,0,0,0,''),
(11832,0,12,7948.27,-2610.06,492.34,0,0,0,''),
(11832,0,13,7928.52,-2625.95,492.448,0,0,0,'escort paused - SAY_REMULOS_INTRO_3'),
(11832,0,14,7948.27,-2610.06,492.34,0,0,0,''),
(11832,0,15,7952.32,-2594.12,490.07,0,0,0,''),
(11832,0,16,7913.99,-2567,488.331,0,0,0,''),
(11832,0,17,7835.45,-2571.1,489.289,0,0,0,'escort paused - SAY_REMULOS_DEFEND_2'),
(11832,0,18,7897.28,-2560.65,487.461,0,0,0,'escort paused'),
(11856,0,0,113.91,-350.13,4.55,0,0,0,''),
(11856,0,1,109.54,-350.08,3.74,0,0,0,''),
(11856,0,2,106.95,-353.4,3.6,0,0,0,''),
(11856,0,3,100.28,-338.89,2.97,0,0,0,''),
(11856,0,4,110.11,-320.26,3.47,0,0,0,''),
(11856,0,5,109.78,-287.8,5.3,0,0,0,''),
(11856,0,6,105.02,-269.71,4.71,0,0,0,''),
(11856,0,7,86.71,-251.81,5.34,0,0,0,''),
(11856,0,8,64.1,-246.38,5.91,0,0,0,''),
(11856,0,9,-2.55,-243.58,6.3,0,0,0,''),
(11856,0,10,-27.78,-267.53,-1.08,0,0,0,''),
(11856,0,11,-31.27,-283.54,-4.36,0,0,0,''),
(11856,0,12,-28.96,-322.44,-9.19,0,0,0,''),
(11856,0,13,-35.63,-360.03,-16.59,0,0,0,''),
(11856,0,14,-58.3,-412.26,-30.6,0,0,0,''),
(11856,0,15,-58.88,-474.17,-44.54,0,0,0,''),
(11856,0,16,-45.92,-496.57,-46.26,0,5000,0,'AMBUSH'),
(11856,0,17,-40.25,-510.07,-46.05,0,0,0,''),
(11856,0,18,-38.88,-520.72,-46.06,0,5000,0,'END'),
(12126,0,0,2631.23,-1917.93,72.59,0,0,0,''),
(12126,0,1,2643.53,-1914.07,71,0,0,0,''),
(12126,0,2,2653.83,-1907.54,69.34,0,0,0,'escort paused'),
(12277,0,1,-1154.87,2708.16,111.123,0,1000,0,'SAY_MELIZZA_START'),
(12277,0,2,-1162.62,2712.86,111.549,0,0,0,''),
(12277,0,3,-1183.37,2709.45,111.601,0,0,0,''),
(12277,0,4,-1245.09,2676.43,111.572,0,0,0,''),
(12277,0,5,-1260.54,2672.48,111.55,0,0,0,''),
(12277,0,6,-1272.71,2666.38,111.555,0,0,0,''),
(12277,0,7,-1342.95,2580.82,111.557,0,0,0,''),
(12277,0,8,-1362.24,2561.74,110.848,0,0,0,''),
(12277,0,9,-1376.56,2514.06,95.6146,0,0,0,''),
(12277,0,10,-1379.06,2510.88,93.3256,0,0,0,''),
(12277,0,11,-1383.14,2489.17,89.009,0,0,0,''),
(12277,0,12,-1395.34,2426.15,88.6607,0,0,0,'SAY_MELIZZA_FINISH'),
(12277,0,13,-1366.23,2317.17,91.8086,0,0,0,''),
(12277,0,14,-1353.81,2213.52,90.726,0,0,0,''),
(12277,0,15,-1354.19,2208.28,88.7386,0,0,0,''),
(12277,0,16,-1354.59,2193.77,77.6702,0,0,0,''),
(12277,0,17,-1367.62,2160.64,67.1482,0,0,0,''),
(12277,0,18,-1379.44,2132.77,64.1326,0,0,0,''),
(12277,0,19,-1404.81,2088.68,61.8162,0,0,0,'SAY_MELIZZA_1'),
(12277,0,20,-1417.15,2082.65,62.4112,0,0,0,''),
(12277,0,21,-1423.28,2074.19,62.2046,0,0,0,''),
(12277,0,22,-1432.99,2070.56,61.7811,0,0,0,''),
(12277,0,23,-1469.27,2078.68,63.1141,0,0,0,''),
(12277,0,24,-1507.21,2115.12,62.3578,0,0,0,''),
(12423,0,0,-9509.72,-147.03,58.74,0,0,0,''),
(12423,0,1,-9517.07,-172.82,58.66,0,0,0,''),
(12427,0,0,-5689.2,-456.44,391.08,0,0,0,''),
(12427,0,1,-5700.37,-450.77,393.19,0,0,0,''),
(12428,0,0,2454.09,361.26,31.51,0,0,0,''),
(12428,0,1,2472.03,378.08,30.98,0,0,0,''),
(12429,0,0,9654.19,909.58,1272.11,0,0,0,''),
(12429,0,1,9642.53,908.11,1269.1,0,0,0,''),
(12430,0,0,161.65,-4779.34,14.64,0,0,0,''),
(12430,0,1,140.71,-4813.56,17.04,0,0,0,''),
(12580,0,0,-8997.63,486.402,96.622,0,0,0,''),
(12580,0,1,-8971.08,507.541,96.349,0,0,0,'SAY_DIALOG_1'),
(12580,0,2,-8953.17,518.537,96.355,0,0,0,''),
(12580,0,3,-8936.33,501.777,94.066,0,0,0,''),
(12580,0,4,-8922.52,498.45,93.869,0,0,0,''),
(12580,0,5,-8907.64,509.941,93.84,0,0,0,''),
(12580,0,6,-8925.26,542.51,94.274,0,0,0,''),
(12580,0,7,-8832.28,622.285,93.686,0,0,0,''),
(12580,0,8,-8824.8,621.713,94.084,0,0,0,''),
(12580,0,9,-8796.46,590.922,97.466,0,0,0,''),
(12580,0,10,-8769.85,607.883,97.118,0,0,0,''),
(12580,0,11,-8737.14,574.741,97.398,0,0,0,'reset jonathan'),
(12580,0,12,-8746.27,563.446,97.399,0,0,0,''),
(12580,0,13,-8745.5,557.877,97.704,0,0,0,''),
(12580,0,14,-8730.95,541.477,101.12,0,0,0,''),
(12580,0,15,-8713.16,520.692,97.227,0,0,0,''),
(12580,0,16,-8677.09,549.614,97.438,0,0,0,''),
(12580,0,17,-8655.72,552.732,96.941,0,0,0,''),
(12580,0,18,-8641.68,540.516,98.972,0,0,0,''),
(12580,0,19,-8620.08,520.12,102.812,0,0,0,''),
(12580,0,20,-8591.09,492.553,104.032,0,0,0,''),
(12580,0,21,-8562.45,463.583,104.517,0,0,0,''),
(12580,0,22,-8548.63,467.38,104.517,0,0,0,'SAY_WINDSOR_BEFORE_KEEP'),
(12580,0,23,-8487.77,391.44,108.386,0,0,0,''),
(12580,0,24,-8455.95,351.225,120.88,0,0,0,''),
(12580,0,25,-8446.87,339.904,121.33,0,0,0,'SAY_WINDSOR_KEEP_1'),
(12580,0,26,-8446.87,339.904,121.33,0,10000,0,''),
(12717,0,0,3346.25,1007.88,3.59,0,0,0,'SAY_MUG_START2'),
(12717,0,1,3367.39,1011.51,3.72,0,0,0,''),
(12717,0,2,3418.64,1013.96,2.905,0,0,0,''),
(12717,0,3,3426.84,1015.1,3.449,0,0,0,''),
(12717,0,4,3437.03,1020.79,2.742,0,0,0,''),
(12717,0,5,3460.56,1024.26,1.353,0,0,0,''),
(12717,0,6,3479.87,1037.96,1.023,0,0,0,''),
(12717,0,7,3490.53,1043.35,3.338,0,0,0,''),
(12717,0,8,3504.28,1047.77,8.205,0,0,0,''),
(12717,0,9,3510.73,1049.79,12.143,0,0,0,''),
(12717,0,10,3514.41,1051.17,13.235,0,0,0,''),
(12717,0,11,3516.94,1052.91,12.918,0,0,0,''),
(12717,0,12,3523.64,1056.3,7.563,0,0,0,''),
(12717,0,13,3531.94,1059.86,6.175,0,0,0,''),
(12717,0,14,3535.48,1069.96,1.697,0,0,0,''),
(12717,0,15,3546.98,1093.49,0.68,0,0,0,''),
(12717,0,16,3549.73,1101.88,-1.123,0,0,0,''),
(12717,0,17,3555.14,1116.99,-4.326,0,0,0,''),
(12717,0,18,3571.94,1132.18,-0.634,0,0,0,''),
(12717,0,19,3574.28,1137.58,3.684,0,0,0,''),
(12717,0,20,3579.31,1137.25,8.205,0,0,0,''),
(12717,0,21,3590.22,1143.65,8.291,0,0,0,''),
(12717,0,22,3595.97,1145.83,6.773,0,0,0,''),
(12717,0,23,3603.65,1146.92,9.763,0,0,0,''),
(12717,0,24,3607.08,1146.01,10.692,0,5000,0,'SAY_MUG_BRAZIER'),
(12717,0,25,3614.52,1142.63,10.248,0,0,0,''),
(12717,0,26,3616.66,1140.84,10.682,0,3000,0,'SAY_MUG_PATROL'),
(12717,0,27,3621.08,1138.11,10.369,0,0,0,'SAY_MUG_RETURN'),
(12717,0,28,3615.48,1145.53,9.614,0,0,0,''),
(12717,0,29,3607.19,1152.72,8.871,0,0,0,''),
(12818,0,0,3347.25,-694.701,159.926,0,0,0,''),
(12818,0,1,3341.53,-694.726,161.125,0,4000,0,''),
(12818,0,2,3338.35,-686.088,163.444,0,0,0,''),
(12818,0,3,3352.74,-677.722,162.316,0,0,0,''),
(12818,0,4,3370.29,-669.367,160.751,0,0,0,''),
(12818,0,5,3381.48,-659.449,162.545,0,0,0,''),
(12818,0,6,3389.55,-648.5,163.652,0,0,0,''),
(12818,0,7,3396.65,-641.509,164.216,0,0,0,''),
(12818,0,8,3410.5,-634.3,165.773,0,0,0,''),
(12818,0,9,3418.46,-631.792,166.478,0,0,0,''),
(12818,0,10,3429.5,-631.589,166.921,0,0,0,''),
(12818,0,11,3434.95,-629.245,168.334,0,0,0,''),
(12818,0,12,3438.93,-618.503,171.503,0,0,0,''),
(12818,0,13,3444.22,-609.294,173.078,0,1000,0,'Ambush 1'),
(12818,0,14,3460.51,-593.794,174.342,0,0,0,''),
(12818,0,15,3480.28,-578.21,176.652,0,0,0,''),
(12818,0,16,3492.91,-562.335,181.396,0,0,0,''),
(12818,0,17,3495.23,-550.978,184.652,0,0,0,''),
(12818,0,18,3496.25,-529.194,188.172,0,0,0,''),
(12818,0,19,3497.62,-510.411,188.345,0,1000,0,'Ambush 2'),
(12818,0,20,3498.5,-497.788,185.806,0,0,0,''),
(12818,0,21,3484.22,-489.718,182.39,0,4000,0,''),
(12858,0,0,1782.63,-2241.11,109.73,0,5000,0,''),
(12858,0,1,1788.88,-2240.17,111.71,0,0,0,''),
(12858,0,2,1797.49,-2238.11,112.31,0,0,0,''),
(12858,0,3,1803.83,-2232.77,111.22,0,0,0,''),
(12858,0,4,1806.65,-2217.83,107.36,0,0,0,''),
(12858,0,5,1811.81,-2208.01,107.45,0,0,0,''),
(12858,0,6,1820.85,-2190.82,100.49,0,0,0,''),
(12858,0,7,1829.6,-2177.49,96.44,0,0,0,''),
(12858,0,8,1837.98,-2164.19,96.71,0,0,0,'prepare'),
(12858,0,9,1839.99,-2149.29,96.78,0,0,0,''),
(12858,0,10,1835.14,-2134.98,96.8,0,0,0,''),
(12858,0,11,1823.57,-2118.27,97.43,0,0,0,''),
(12858,0,12,1814.99,-2110.35,98.38,0,0,0,''),
(12858,0,13,1806.6,-2103.09,99.19,0,0,0,''),
(12858,0,14,1798.27,-2095.77,100.04,0,0,0,''),
(12858,0,15,1783.59,-2079.92,100.81,0,0,0,''),
(12858,0,16,1776.79,-2069.48,101.77,0,0,0,''),
(12858,0,17,1776.82,-2054.59,109.82,0,0,0,''),
(12858,0,18,1776.88,-2047.56,109.83,0,0,0,''),
(12858,0,19,1776.86,-2036.55,109.83,0,0,0,''),
(12858,0,20,1776.9,-2024.56,109.83,0,0,0,'win'),
(12858,0,21,1776.87,-2028.31,109.83,0,60000,0,'stay'),
(12858,0,22,1776.9,-2028.3,109.83,0,0,0,''),
(17804,0,0,-9054.86,443.58,93.05,0,0,0,''),
(17804,0,1,-9079.33,424.49,92.52,0,0,0,''),
(17804,0,2,-9086.21,419.02,92.32,0,3000,0,''),
(17804,0,3,-9086.21,419.02,92.32,0,1000,0,''),
(17804,0,4,-9079.33,424.49,92.52,0,0,0,''),
(17804,0,5,-9054.38,436.3,93.05,0,0,0,''),
(17804,0,6,-9042.23,434.24,93.37,0,5000,0,'SAY_SIGNAL_SENT');

-- EOF

