-- delete original GOSSIP_MENU_BOT
delete FROM gossip_menu_option where option_id = 99;
  
  DROP TABLE IF EXISTS `ai_playerbot_rpg_races`;
  
  CREATE TABLE `ai_playerbot_rpg_races` (
    `id` bigint(20) NOT NULL AUTO_INCREMENT,
    `entry` bigint(20),
    `race` bigint(20),
    `minl` bigint(20),
    `maxl` bigint(20),
    PRIMARY KEY (`id`),
    KEY `entry` (`entry`)
  ) ENGINE=InnoDB DEFAULT CHARSET=utf8;
  
  DELETE FROM `ai_playerbot_rpg_races`;
  
  -- say
  
  INSERT INTO `ai_playerbot_rpg_races` VALUES
  --
  --       DRAENEI
  --
  -- Draenei Azumeryst Isle
  (NULL, 16553, 11, 1, 10),
  -- Draenei Bloodmyst Isle
  (NULL, 17553, 11, 10, 20),
  --
  --       HUMANS
  --
  -- Innkeeper Farley, Goldshire
  (NULL, 295, 1, 1, 10),
  -- Innkeeper Heather, Westfall
  (NULL, 8931, 1, 10, 20),
  --
  --       DWARVES & GNOMES
  --
  -- Innkeeper Belm, Kharanos
  (NULL, 1247, 3, 1, 10),
  (NULL, 1247, 7, 1, 10),
  -- Innkeeper Hearthstove, Loch Modan
  (NULL, 6734, 3, 10, 20),
  (NULL, 6734, 7, 10, 20),
  --
  --       NIGHT ELVES
  --
  -- Innkeeper Keldamyr, Dolanaar
  (NULL, 6736, 4, 1, 10),
  -- Innkeeper Shaussiy, Auberdine
  (NULL, 6737, 4, 10, 20),
  --
  --       ALLIANCE CITIES
  --
  -- Innkeeper Saelienne, Darnassus
  (NULL, 6735, 4, 10, 80), -- elves
  --
  -- Innkeeper Firebrew, Ironforge
  (NULL, 5111, 3, 10, 80), -- dwarves
  (NULL, 5111, 7, 10, 80), -- gnomes
  (NULL, 5111, 4, 10, 80), -- elves
  (NULL, 5111, 1, 10, 80), -- human
  (NULL, 5111, 11, 20, 80), -- draenei
  --
  -- Innkeeper Allison, Stormwind
  (NULL, 6740, 1, 10, 80), -- human
  (NULL, 6740, 3, 10, 80), -- dwarves
  (NULL, 6740, 4, 10, 80), -- elves
  (NULL, 6740, 7, 10, 80), -- gnomes
  (NULL, 6740, 11, 20, 80), -- draenei
  --
  -- Caregiver Breel Exodar
  (NULL, 16739, 11, 10, 80), -- draenei
  (NULL, 16739, 1, 60, 80), -- human
  (NULL, 16739, 3, 60, 80), -- dwarves
  (NULL, 16739, 4, 60, 80), -- elves
  (NULL, 16739, 7, 60, 80), -- gnomes
  --
  --       ALLIANCE CONTESTED LOCATIONS
  --
  -- Innkeeper Kimlya, Astranaar
  --
  (NULL, 6738, 4, 15, 30), -- elves
  (NULL, 6738, 11, 20, 30), -- draenei
  --
  -- Innkeeper Faralia, Stonetalon Peak
  --
  (NULL, 16458, 4, 15, 27), -- elves
  --
  -- Innkeeper Lyshaerya, Desolase
  --
  (NULL, 11103, 4, 30, 40), -- elves
  --
  -- Innkeeper Shyria, Feathermoon, Feralas
  (NULL, 7736, 4, 40, 50), -- elves
  --
  -- Falfindel Waywarder, Feralas elf camp
  (NULL, 4048, 4, 40, 50), -- elves
  --
  -- Innkeeper Helbrek, Wetlands
  --
  (NULL, 1464, 3, 20, 30), -- dwarves
  (NULL, 1464, 7, 20, 30), -- gnomes
  --
  -- Innkeeper Trelayne, Duskwood
  --
  (NULL, 6790, 1, 18, 30), -- human
  (NULL, 6790, 3, 18, 30), -- dwarves
  (NULL, 6790, 7, 18, 30), -- gnomes
  (NULL, 6790, 11, 20, 30), -- draenei
  --
  -- Innkeeper Brianna, Redridge Mountains
  --
  (NULL, 6727, 1, 15, 25), -- human
  --
  -- Innkeeper Anderson, Southshore, Hillsbrad
  (NULL, 2352, 1, 20, 30), -- human
  (NULL, 2352, 3, 20, 30), -- dwarves
  (NULL, 2352, 7, 20, 30), -- gnomes
  (NULL, 2352, 11, 20, 30), -- draenei
  --
  -- Captain Nials, Refuge Pointe, Arathi
  (NULL, 2700, 1, 30, 40), -- human
  (NULL, 2700, 3, 30, 40), -- dwarves
  (NULL, 2700, 7, 30, 40), -- gnomes
  (NULL, 2700, 4, 30, 40), -- elves
  (NULL, 2700, 11, 30, 40), -- draenei
  --
  -- Lt. Doren, Stranglethorn Vale
  (NULL, 469, 1, 30, 45), -- human
  (NULL, 469, 3, 30, 45), -- dwarves
  (NULL, 469, 4, 30, 45), -- elves
  (NULL, 469, 7, 30, 45), -- gnomes
  (NULL, 469, 11, 30, 45), -- draenei
  --
  -- Innkeeper Janene, Theramore
  (NULL, 6272, 1, 35, 45), -- human
  (NULL, 6272, 3, 35, 45), -- dwarves
  (NULL, 6272, 7, 35, 45), -- gnomes
  (NULL, 6272, 11, 35, 45), -- draenei
  --
  -- Innkeeper Prospector Ryedol, Badlands Q-giver
  (NULL, 2910, 3, 35, 45), -- dwarves
  (NULL, 2910, 7, 35, 45), -- gnomes
  --
  -- Innkeeper Thulfram, Hinterlands, Dwarven Outpost
  (NULL, 7744, 3, 40, 50), -- dwarves
  (NULL, 7744, 1, 40, 50), -- human
  (NULL, 7744, 7, 40, 50), -- gnomes
  --
  -- Loh'atu, Azshara alliance camp Q-giver 11548
  (NULL, 11548, 4, 45, 55), -- elves
  (NULL, 11548, 1, 45, 55), -- human
  (NULL, 11548, 11, 45, 55), -- draenei
  --
  -- Thadius Grimshade, Nethergarde Keep, Blasted Lands
  (NULL, 8022, 1, 45, 55), -- human
  (NULL, 8022, 3, 45, 55), -- dwarves
  (NULL, 8022, 4, 45, 55), -- elves
  (NULL, 8022, 7, 45, 55), -- gnomes
  (NULL, 8022, 11, 45, 55), -- draenei
  --
  -- Gothine the Hooded, Felwood Alliance camp
  (NULL, 9465, 4, 48, 55), -- elves
  (NULL, 9465, 1, 48, 55), -- human
  (NULL, 9465, 3, 48, 55), -- dwarves
  (NULL, 9465, 7, 48, 55), -- gnomes
  (NULL, 9465, 11, 48, 55), -- draenei
  --
  -- Muigin, Alliance Q-giver, Un'Goro
  (NULL, 9119, 1, 48, 55), -- human
  (NULL, 9119, 3, 48, 55), -- dwarves
  (NULL, 9119, 4, 48, 55), -- elves
  (NULL, 9119, 7, 48, 55), -- gnomes
  (NULL, 9119, 11, 48, 55), -- draenei
  --
  -- Alchemist Arbington, West Plaguelands, Human
  (NULL, 11056, 1, 51, 58), -- human
  (NULL, 11056, 3, 51, 58), -- dwarves
  (NULL, 11056, 4, 51, 58), -- elves
  (NULL, 11056, 7, 51, 58), -- gnomes
  (NULL, 11056, 11, 51, 58), -- draenei
  --
  -- Borgus Stourarm, Alliance Taxi, Burning Steppes
  (NULL, 2299, 1, 50, 60), -- human
  (NULL, 2299, 3, 50, 60), -- dwarves
  (NULL, 2299, 4, 50, 60), -- elves
  (NULL, 2299, 7, 50, 60), -- gnomes
  (NULL, 2299, 11, 50, 60), -- draenei
  --
  -- Marshal Bluewall, Alliance camp, Silithus
  (NULL, 17080, 1, 55, 60), -- human
  (NULL, 17080, 3, 55, 60), -- dwarves
  (NULL, 17080, 4, 55, 60), -- elves
  (NULL, 17080, 7, 55, 60), -- gnomes
  (NULL, 17080, 11, 55, 60), -- draenei
  --
  --           OUTLAND
  --
  -- Commander Duron, Dark Portal
  (NULL, 19229, 1, 58, 59), -- human
  (NULL, 19229, 3, 58, 59), -- dwarves
  (NULL, 19229, 4, 58, 59), -- elves
  (NULL, 19229, 7, 58, 59), -- gnomes
  (NULL, 19229, 11, 58, 59), -- draenei
  --
  -- Sid Limbardi, Honor Hold, Hellfire
  (NULL, 16826, 1, 58, 63), -- human
  (NULL, 16826, 3, 58, 63), -- dwarves
  (NULL, 16826, 4, 58, 63), -- elves
  (NULL, 16826, 7, 58, 63), -- gnomes
  (NULL, 16826, 11, 58, 63), -- draenei
  --
  -- Caregiver Ophera Windfury, Draenei, Hellfire
  (NULL, 18906, 1, 60, 63), -- human
  (NULL, 18906, 3, 60, 63), -- dwarves
  (NULL, 18906, 4, 60, 63), -- elves
  (NULL, 18906, 7, 60, 63), -- gnomes
  (NULL, 18906, 11, 60, 63), -- draenei
  --
  -- Caregiver Abidaar, Telredor, Zangarmarsh
  (NULL, 18251, 1, 60, 63), -- human
  (NULL, 18251, 3, 60, 63), -- dwarves
  (NULL, 18251, 4, 60, 63), -- elves
  (NULL, 18251, 7, 60, 63), -- gnomes
  (NULL, 18251, 11, 60, 63), -- draenei
  --
  -- Caregiver Kerp, Orebor, Zangarmarsh
  (NULL, 18908, 1, 61, 64), -- human
  (NULL, 18908, 3, 61, 64), -- dwarves
  (NULL, 18908, 4, 61, 64), -- elves
  (NULL, 18908, 7, 61, 64), -- gnomes
  (NULL, 18908, 11, 61, 64), -- draenei
  --
  -- Innkeeper Biribi, Terrokar
  (NULL, 19296, 1, 62, 65), -- human
  (NULL, 19296, 3, 62, 65), -- dwarves
  (NULL, 19296, 4, 62, 65), -- elves
  (NULL, 19296, 7, 62, 65), -- gnomes
  (NULL, 19296, 11, 62, 65), -- draenei
  --
  -- Caregiver Isel, Telaar, Nagrand
  (NULL, 18914, 1, 64, 67), -- human
  (NULL, 18914, 3, 64, 67), -- dwarves
  (NULL, 18914, 4, 64, 67), -- elves
  (NULL, 18914, 7, 64, 67), -- gnomes
  (NULL, 18914, 11, 64, 67), -- draenei
  --
  -- Innkeeper Shaunessy, Sylvanaar, Blade's Edge
  (NULL, 19495, 1, 65, 68), -- human
  (NULL, 19495, 3, 65, 68), -- dwarves
  (NULL, 19495, 4, 65, 68), -- elves
  (NULL, 19495, 7, 65, 68), -- gnomes
  (NULL, 19495, 11, 65, 68), -- draenei
  --
  -- Innkeeper Fizir Doc Clocktock, Blade's Edge
  (NULL, 21110, 1, 65, 68), -- human
  (NULL, 21110, 3, 65, 68), -- dwarves
  (NULL, 21110, 4, 65, 68), -- elves
  (NULL, 21110, 7, 65, 68), -- gnomes
  (NULL, 21110, 11, 65, 68), -- draenei
  --
  -- Innkeeper Dreg Cloudsweeper, Shadowmoon
  (NULL, 19352, 1, 67, 70), -- human
  (NULL, 19352, 3, 67, 70), -- dwarves
  (NULL, 19352, 4, 67, 70), -- elves
  (NULL, 19352, 7, 67, 70), -- gnomes
  (NULL, 19352, 11, 67, 70), -- draenei
  --
  --           NORTHREND
  --
  -- Isirami Fairwind, Dalaran
  (NULL, 32413, 1, 72, 80), -- human
  (NULL, 32413, 3, 72, 80), -- dwarves
  (NULL, 32413, 4, 72, 80), -- elves
  (NULL, 32413, 7, 72, 80), -- gnomes
  (NULL, 32413, 11, 72, 80), -- draenei
  --
  -- James Deacon, Valiance Keep, Borean Tundra
  (NULL, 25245, 1, 68, 72), -- human
  (NULL, 25245, 3, 68, 72), -- dwarves
  (NULL, 25245, 4, 68, 72), -- elves
  (NULL, 25245, 7, 68, 72), -- gnomes
  (NULL, 25245, 11, 68, 72), -- draenei
  --
  -- "Charlie" Northtop, Fizzcrank Airstrip, Borean Tundra
  (NULL, 26596, 1, 69, 72), -- human
  (NULL, 26596, 3, 69, 72), -- dwarves
  (NULL, 26596, 4, 69, 72), -- elves
  (NULL, 26596, 7, 69, 72), -- gnomes
  (NULL, 26596, 11, 69, 72), -- draenei
  --
  -- Innkeeper Hazel Lagras, Valgarde, Howling Fjord
  (NULL, 23731, 1, 68, 72), -- human
  (NULL, 23731, 3, 68, 72), -- dwarves
  (NULL, 23731, 4, 68, 72), -- elves
  (NULL, 23731, 7, 68, 72), -- gnomes
  (NULL, 23731, 11, 68, 72), -- draenei
  --
  -- Innkeeper Celeste Goodhutch, Westguard Keep, Howling Fjord
  (NULL, 23937, 1, 69, 72), -- human
  (NULL, 23937, 3, 69, 72), -- dwarves
  (NULL, 23937, 4, 69, 72), -- elves
  (NULL, 23937, 7, 69, 72), -- gnomes
  (NULL, 23937, 11, 69, 72), -- draenei
  --
  -- Christina Daniels, Fort Wildervar, Howling Fjord
  (NULL, 24057, 1, 70, 72), -- human
  (NULL, 24057, 3, 70, 72), -- dwarves
  (NULL, 24057, 4, 70, 72), -- elves
  (NULL, 24057, 7, 70, 72), -- gnomes
  (NULL, 24057, 11, 70, 72), -- draenei
  --
  -- Jennifer Bell, Amberpine Lodge, Grizzly Hills
  (NULL, 27066, 1, 70, 74), -- human
  (NULL, 27066, 3, 70, 74), -- dwarves
  (NULL, 27066, 4, 70, 74), -- elves
  (NULL, 27066, 7, 70, 74), -- gnomes
  (NULL, 27066, 11, 70, 74), -- draenei
  --
  -- Quartermaster McCarty, Westfall Brigade Encampment, Grizzly Hills
  (NULL, 26375, 1, 70, 74), -- human
  (NULL, 26375, 3, 70, 74), -- dwarves
  (NULL, 26375, 4, 70, 74), -- elves
  (NULL, 26375, 7, 70, 74), -- gnomes
  (NULL, 26375, 11, 70, 74), -- draenei
  --
  -- Illusia Lune, Wintergarde Keep, Dragonblight
  (NULL, 27042, 1, 71, 75), -- human
  (NULL, 27042, 3, 71, 75), -- dwarves
  (NULL, 27042, 4, 71, 75), -- elves
  (NULL, 27042, 7, 71, 75), -- gnomes
  (NULL, 27042, 11, 71, 75), -- draenei
  --
  -- Naohain, Stars' Rest, Dragonblight
  (NULL, 27052, 1, 71, 75), -- human
  (NULL, 27052, 3, 71, 75), -- dwarves
  (NULL, 27052, 4, 71, 75), -- elves
  (NULL, 27052, 7, 71, 75), -- gnomes
  (NULL, 27052, 11, 71, 75), -- draenei
  --
  -- Gunda Boldhammer, Frosthold, Storm Peaks
  (NULL, 29926, 1, 77, 80), -- human
  (NULL, 29926, 3, 77, 80), -- dwarves
  (NULL, 29926, 4, 77, 80), -- elves
  (NULL, 29926, 7, 77, 80), -- gnomes
  (NULL, 29926, 11, 77, 80), -- draenei
  --
  -- Caris Sunlance, Argent Tournament, Icecrown
  (NULL, 33970, 1, 80, 80), -- human
  (NULL, 33970, 3, 80, 80), -- dwarves
  (NULL, 33970, 4, 80, 80), -- elves
  (NULL, 33970, 7, 80, 80), -- gnomes
  (NULL, 33970, 11, 80, 80), -- draenei
  --
  --       ALLIANCE MOUNT VENDORS
  --
  -- Milli Featherwhistle, Gnome Mechanostrider merchant, Dun Morogh
  -- (NULL, 7955, 3),
  -- (NULL, 7955, 7),
  --
  -- Lelanai, Night Elf Night Saber vendor, Darnassus
  -- (NULL, 4730, 4),
  --
  -- Katie Hunter, Human Horse vendor, Elwynn Forest
  -- (NULL, 384, 1),
  --
  --       HORDE
  --
  --       ORCS & TROLLS
  --
  -- Innkeeper Grosk, Durotar
  (NULL, 6928, 2, 1, 10),
  (NULL, 6928, 8, 1, 10),
  --
  --       TAUREN
  --
  -- Innkeeper Pala, Mulgore
  (NULL, 6746, 6, 1, 10),
  --
  --       UNDEAD
  --
  -- Innkeeper Renee, Brill, Tirisfal Glades
  (NULL, 5688, 5, 1, 10),
  -- Innkeeper Bates, The Sepulcher, Silverpine Forest
  (NULL, 6739, 5, 10, 20),
  --
  --       BLOOD ELVES
  --
  -- Blood Elves Eversong Woods
  (NULL, 15397, 10, 1, 10),
  -- Blood Elves Ghostlands
  (NULL, 16542, 10, 10, 20),
  --
  --       HORDE CITIES
  --
  -- Innkeeper Gryshka, Orgrimmar
  (NULL, 6929, 2, 10, 80), -- orcs
  (NULL, 6929, 8, 10, 80), -- trolls
  (NULL, 6929, 6, 10, 80), -- tauren
  (NULL, 6929, 5, 20, 80), -- undead
  (NULL, 6929, 10, 20, 80), -- blood elves
  --
  -- Innkeeper Pala, Thunder Bluff, Mulgore
  (NULL, 6746, 6, 10, 80), -- tauren
  --
  -- Innkeeper Norman, Undercity
  (NULL, 6741, 5, 10, 80), -- undead
  (NULL, 6741, 10, 20, 80), -- blood elves
  --
  -- Innkeeper Velandra Silvermoon
  (NULL, 16618, 10, 10, 80), -- blood elves
  -- Innkeeper Jovia Silvermoon
  (NULL, 17630, 2, 60, 80), -- orcs
  (NULL, 17630, 5, 60, 80), -- undead
  (NULL, 17630, 6, 60, 80), -- tauren
  (NULL, 17630, 8, 60, 80), -- trolls
  -- Innkeeper Delaniel Silvermoon Entrance
  (NULL, 15433, 10, 5, 7), -- blood elves
  --
  --       HORDE CONTESTED LOCATIONS
  --
  -- Innkeeper Boorand Plainswind, Crossroads, Barrens
  (NULL, 3934, 2, 10, 25), -- orcs
  (NULL, 3934, 6, 10, 25), -- tauren
  (NULL, 3934, 8, 10, 25), -- trolls
  (NULL, 3934, 10, 20, 25), -- blood elves
  -- (NULL, 3934, 5, 15, 25), -- undead
  --
  -- Innkeeper Byula, Camp Taurajo, Barrens
  (NULL, 7714, 2, 10, 25), -- orcs
  (NULL, 7714, 6, 10, 25), -- tauren
  (NULL, 7714, 8, 10, 25), -- trolls
  --
  -- Innkeeper Jayka, Stonetalon, Red Rock Retreat
  (NULL, 7731, 2, 15, 27), -- orcs
  (NULL, 7731, 6, 15, 27), -- tauren
  (NULL, 7731, 8, 15, 27), -- trolls
  (NULL, 7731, 10, 20, 27), -- blood elves
  --
  -- Innkeeper Abeqwa, Thousand Needles
  (NULL, 11116, 2, 25, 35), -- orcs
  (NULL, 11116, 6, 25, 35), -- tauren
  (NULL, 11116, 8, 25, 35), -- trolls
  (NULL, 11116, 10, 25, 35), -- blood elves
  --
  -- Innkeeper Shay, Tarren Mill, Hillsbrad
  (NULL, 2388, 5, 20, 30), -- undead
  (NULL, 2388, 10, 20, 30), -- blood elves
  --
  -- Innkeeper Greul, Feralas, Horde
  (NULL, 7737, 6, 40, 50), -- tauren
  --
  -- Innkeeper Kaylisk, Splitertree, Ashenvale
  (NULL, 12196, 2, 18, 30), -- orcs
  (NULL, 12196, 8, 18, 30), -- trolls
  (NULL, 12196, 10, 20, 30), -- blood elves
  --
  -- Marukai, Zoram'gar, Ashenvale
  (NULL, 12719, 2, 18, 30), -- orcs
  (NULL, 12719, 8, 18, 30), -- trolls
  --
  -- Innkeeper Sikewa, Desolace
  (NULL, 11106, 2, 30, 40), -- orcs
  (NULL, 11106, 6, 30, 40), -- tauren
  (NULL, 11106, 8, 30, 40), -- trolls
  --
  -- Innkeeper Adegwa, Arathi, Hammerfall
  (NULL, 9501, 2, 30, 40), -- orcs
  (NULL, 9501, 5, 30, 40), -- undead
  (NULL, 9501, 6, 30, 40), -- tauren
  (NULL, 9501, 8, 30, 40), -- trolls
  (NULL, 9501, 10, 30, 40), -- blood elves
  --
  -- Innkeeper Lard, Revantusk Village , Hinterlands
  (NULL, 14731, 2, 40, 50), -- orcs
  (NULL, 14731, 5, 40, 50), -- undead
  (NULL, 14731, 6, 40, 50), -- tauren
  (NULL, 14731, 8, 40, 50), -- trolls
  (NULL, 14731, 10, 40, 50), -- blood elves
  --
  -- Innkeeper Shul'kar, Kargath Outpost, Badlands
  (NULL, 9356, 2, 35, 45), -- orcs
  (NULL, 9356, 5, 35, 45), -- undead
  (NULL, 9356, 6, 35, 45), -- tauren
  (NULL, 9356, 8, 35, 45), -- trolls
  (NULL, 9356, 10, 35, 45), -- blood elves
  --
  -- Innkeeper Karakul, Swamp of Sorrows
  (NULL, 6930, 2, 35, 45), -- orcs
  (NULL, 6930, 5, 35, 45), -- undead
  (NULL, 6930, 6, 35, 45), -- tauren
  (NULL, 6930, 8, 35, 45), -- trolls
  (NULL, 6930, 10, 35, 45), -- blood elves
  --
  -- Innkeeper Thulbek, Grom Gol, Stranglethorn Vale
  (NULL, 5814, 2, 30, 45), -- orcs
  (NULL, 5814, 5, 30, 45), -- undead
  (NULL, 5814, 6, 30, 45), -- tauren
  (NULL, 5814, 8, 30, 45), -- trolls
  (NULL, 5814, 10, 30, 45), -- blood elves
  --
  -- Overlord Mok'Morokk, Dustwallow Marsh
  (NULL, 4500, 2, 35, 45), -- orcs
  (NULL, 4500, 6, 35, 45), -- tauren
  (NULL, 4500, 8, 35, 45), -- trolls
  --
  -- Jediga, Azshara horde camp
  (NULL, 8587, 2, 45, 55), -- orcs
  (NULL, 8587, 8, 45, 55), -- trolls
  (NULL, 8587, 6, 45, 55), -- tauren
  (NULL, 8587, 10, 45, 55), -- blood elves
  --
  -- Winna Hazzard, Felwood horde camp
  (NULL, 9996, 2, 48, 55), -- orcs
  (NULL, 9996, 6, 48, 55), -- tauren
  (NULL, 9996, 8, 48, 55), -- trolls
  (NULL, 9996, 5, 48, 55), -- undead
  (NULL, 9996, 10, 48, 55), -- blood elves
  --
  -- Larion, Horde Q-giver, Un'Goro
  (NULL, 9118, 2, 48, 55), -- orcs
  (NULL, 9118, 6, 48, 55), -- tauren
  (NULL, 9118, 8, 48, 55), -- trolls
  (NULL, 9118, 5, 48, 55), -- undead
  (NULL, 9118, 10, 48, 55), -- blood elves
  --
  -- Vahgruk, Horde Taxi, Burning Steppes
  (NULL, 13177, 2, 50, 60), -- orcs
  (NULL, 13177, 5, 50, 60), -- undead
  (NULL, 13177, 6, 50, 60), -- tauren
  (NULL, 13177, 8, 50, 60), -- trolls
  (NULL, 13177, 10, 50, 60), -- blood elves
  --
  -- General Kirika, Horde camp, Silithus
  (NULL, 17079, 2, 55, 60), -- orcs
  (NULL, 17079, 5, 55, 60), -- undead
  (NULL, 17079, 6, 55, 60), -- tauren
  (NULL, 17079, 8, 55, 60), -- trolls
  (NULL, 17079, 10, 55, 60), -- blood elves
  --
  --        OUTLAND
  --
  -- Lieutenant General Orion, Dark Portal
  (NULL, 19253, 2, 58, 59), -- orcs
  (NULL, 19253, 5, 58, 59), -- undead
  (NULL, 19253, 6, 58, 59), -- tauren
  (NULL, 19253, 8, 58, 59), -- trolls
  (NULL, 19253, 10, 58, 59), -- blood elves
  --
  -- Floyd Pinkus, Thrallmar, Hellfire
  (NULL, 16602, 2, 58, 63), -- orcs
  (NULL, 16602, 5, 58, 63), -- undead
  (NULL, 16602, 6, 58, 63), -- tauren
  (NULL, 16602, 8, 58, 63), -- trolls
  (NULL, 16602, 10, 58, 63), -- blood elves
  --
  -- Innkeeper Bazil, Falcon Watch, Hellfire
  (NULL, 18905, 2, 60, 63), -- orcs
  (NULL, 18905, 5, 60, 63), -- undead
  (NULL, 18905, 6, 60, 63), -- tauren
  (NULL, 18905, 8, 60, 63), -- trolls
  (NULL, 18905, 10, 60, 63), -- blood elves
  --
  -- Innkeeper Merajit, Zabra'jin, Zangarmarsh
  (NULL, 18245, 2, 60, 64), -- orcs
  (NULL, 18245, 5, 60, 64), -- undead
  (NULL, 18245, 6, 60, 64), -- tauren
  (NULL, 18245, 8, 60, 64), -- trolls
  (NULL, 18245, 10, 60, 64), -- blood elves
  --
  -- Innkeeper Grilka, Terrokar
  (NULL, 18957, 2, 62, 65), -- orcs
  (NULL, 18957, 5, 62, 65), -- undead
  (NULL, 18957, 6, 62, 65), -- tauren
  (NULL, 18957, 8, 62, 65), -- trolls
  (NULL, 18957, 10, 62, 65), -- blood elves
  --
  -- Matron Tikkit, Garadar, Nagrand
  (NULL, 18913, 2, 62, 65), -- orcs
  (NULL, 18913, 5, 62, 65), -- undead
  (NULL, 18913, 6, 62, 65), -- tauren
  (NULL, 18913, 8, 62, 65), -- trolls
  (NULL, 18913, 10, 62, 65), -- blood elves
  --
  -- Innkeeper Matron Varah, Mok'Nathal, Blade's Edge
  (NULL, 21088, 2, 65, 68), -- orcs
  (NULL, 21088, 5, 65, 68), -- undead
  (NULL, 21088, 6, 65, 68), -- tauren
  (NULL, 21088, 8, 65, 68), -- trolls
  (NULL, 21088, 10, 65, 68), -- blood elves
  --
  -- Innkeeper Gholah, Thunderlord, Blade's Edge
  (NULL, 19470, 2, 65, 68), -- orcs
  (NULL, 19470, 5, 65, 68), -- undead
  (NULL, 19470, 6, 65, 68), -- tauren
  (NULL, 19470, 8, 65, 68), -- trolls
  (NULL, 19470, 10, 65, 68), -- blood elves
  --
  -- Innkeeper Darg Bloodclaw, Shadowmoon Village
  (NULL, 19319, 2, 67, 70), -- orcs
  (NULL, 19319, 5, 67, 70), -- undead
  (NULL, 19319, 6, 67, 70), -- tauren
  (NULL, 19319, 8, 67, 70), -- trolls
  (NULL, 19319, 10, 67, 70), -- blood elves
  --
  --        NORTHREND
  --
  -- Uda the Beast, Dalaran
  (NULL, 31557, 2, 72, 80), -- orcs
  (NULL, 31557, 5, 72, 80), -- undead
  (NULL, 31557, 6, 72, 80), -- tauren
  (NULL, 31557, 8, 72, 80), -- trolls
  (NULL, 31557, 10, 72, 80), -- blood elves
  --
  -- Williamson, Warsong Hold, Borean Tundra
  (NULL, 25278, 2, 68, 72), -- orcs
  (NULL, 25278, 5, 68, 72), -- undead
  (NULL, 25278, 6, 68, 72), -- tauren
  (NULL, 25278, 8, 68, 72), -- trolls
  (NULL, 25278, 10, 68, 72), -- blood elves
  --
  -- Pahu Frosthoof, Taunka'le Village, Borean Tundra
  (NULL, 26709, 2, 69, 72), -- orcs
  (NULL, 26709, 5, 69, 72), -- undead
  (NULL, 26709, 6, 69, 72), -- tauren
  (NULL, 26709, 8, 69, 72), -- trolls
  (NULL, 26709, 10, 69, 72), -- blood elves
  --
  -- Matron Magah, Bor'Gorok Outpost, Borean Tundra
  (NULL, 26709, 2, 70, 72), -- orcs
  (NULL, 26709, 5, 70, 72), -- undead
  (NULL, 26709, 6, 70, 72), -- tauren
  (NULL, 26709, 8, 70, 72), -- trolls
  (NULL, 26709, 10, 70, 72), -- blood elves
  --
  -- Basil Osgood, New Agamand, Howling Fjord
  (NULL, 24149, 2, 68, 72), -- orcs
  (NULL, 24149, 5, 68, 72), -- undead
  (NULL, 24149, 6, 68, 72), -- tauren
  (NULL, 24149, 8, 68, 72), -- trolls
  (NULL, 24149, 10, 68, 72), -- blood elves
  --
  -- Timothy Holland, Vengeance Landing, Howling Fjord
  (NULL, 24342, 2, 69, 72), -- orcs
  (NULL, 24342, 5, 69, 72), -- undead
  (NULL, 24342, 6, 69, 72), -- tauren
  (NULL, 24342, 8, 69, 72), -- trolls
  (NULL, 24342, 10, 69, 72), -- blood elves
  --
  -- Bori Wintertotem, Camp Winterhoof, Howling Fjord
  (NULL, 24033, 2, 70, 72), -- orcs
  (NULL, 24033, 5, 70, 72), -- undead
  (NULL, 24033, 6, 70, 72), -- tauren
  (NULL, 24033, 8, 70, 72), -- trolls
  (NULL, 24033, 10, 70, 72), -- blood elves
  --
  -- Barracks Master Rhekku, Conquest Hold, Grizzly Hills
  (NULL, 27125, 2, 70, 74), -- orcs
  (NULL, 27125, 5, 70, 74), -- undead
  (NULL, 27125, 6, 70, 74), -- tauren
  (NULL, 27125, 8, 70, 74), -- trolls
  (NULL, 27125, 10, 70, 74), -- blood elves
  --
  -- Aiyan Coldwind, Capm Onequah, Grizzly Hills
  (NULL, 26680, 2, 70, 74), -- orcs
  (NULL, 26680, 5, 70, 74), -- undead
  (NULL, 26680, 6, 70, 74), -- tauren
  (NULL, 26680, 8, 70, 74), -- trolls
  (NULL, 26680, 10, 70, 74), -- blood elves
  --
  -- Mrs. Winterby, Venomspite, Dragonblight
  (NULL, 27027, 2, 71, 75), -- orcs
  (NULL, 27027, 5, 71, 75), -- undead
  (NULL, 27027, 6, 71, 75), -- tauren
  (NULL, 27027, 8, 71, 75), -- trolls
  (NULL, 27027, 10, 71, 75), -- blood elves
  --
  -- Barracks Master Harga, Agmar's Hammer, Dragonblight
  (NULL, 26985, 2, 71, 75), -- orcs
  (NULL, 26985, 5, 71, 75), -- undead
  (NULL, 26985, 6, 71, 75), -- tauren
  (NULL, 26985, 8, 71, 75), -- trolls
  (NULL, 26985, 10, 71, 75), -- blood elves
  --
  -- Wabada Whiteflower, Camp Tunka'lo, Storm Peaks
  (NULL, 29971, 2, 77, 80), -- orcs
  (NULL, 29971, 5, 77, 80), -- undead
  (NULL, 29971, 6, 77, 80), -- tauren
  (NULL, 29971, 8, 77, 80), -- trolls
  (NULL, 29971, 10, 77, 80), -- blood elves
  --
  -- Jarin Dawnglow, Argent Tournament, Icecrown
  (NULL, 33971, 2, 80, 80), -- orcs
  (NULL, 33971, 5, 80, 80), -- undead
  (NULL, 33971, 6, 80, 80), -- tauren
  (NULL, 33971, 8, 80, 80), -- trolls
  (NULL, 33971, 10, 80, 80), -- blood elves
  --
  --        NEUTRAL AREAS
  --
  -- Innkeeper Skindle, Booty Bay 6807 (Neutral)
  (NULL, 6807, 0, 30, 45),
  -- Innkeeper Wiley, Ratchet 6791 (Neutral)
  (NULL, 6791, 2, 10, 25),
  (NULL, 6791, 8, 10, 25),
  -- Innkeeper Fizzgrimble, Tanaris 7733 (Neutral)
  (NULL, 7733, 0, 40, 50),
  -- Master Smith Burninate, Searing Gorge
  (NULL, 14624, 0, 45, 50),
  -- Marin Noggenfogger 7564 (Neutral)
  -- Innkeeper Vizzie, Everlook 11118 (Neutral)
  (NULL, 11118, 0, 53, 60),
  -- Calandrath, Silithus 15174 (Neutral)
  (NULL, 15174, 0, 55, 60),
  -- Jessica Chambers, East Plaguelands 16256 (Neutral)
  (NULL, 16256, 0, 53, 60),
  --
  --           OUTLAND
  --
  -- Innkeeper Coryth Stoktron, Cenarion Refuge (Neutral)
  (NULL, 18907, 0, 60, 64),
  -- Minalei, Aldors, Shattrath
  (NULL, 19046, 0, 65, 70),
  -- Innkeeper Haelthol, Scryers, Shattrath
  (NULL, 19232, 0, 65, 70),
  -- Shaarubo, World End Tavern
  (NULL, 19182, 0, 65, 70),
  -- Innkeeper Aelerya, Blade's Edge (Neutral)
  (NULL, 22922, 0, 65, 68),
  -- Innkeeper Eyonix, Stormspire, Netherstorm
  (NULL, 19531, 0, 67, 70),
  -- Innkeeper Remi Dodoso, Area 52, Netherstorm
  (NULL, 19571, 0, 67, 70),
  -- Caregiver Inaara, Isle if Quel'Danas
  (NULL, 25036, 0, 69, 70),
  --
  --           NORTHREND
  --
  -- Amisi Azuregaze, Dalaran Inn
  (NULL, 28687, 0, 72, 80),
  -- Afsaneh Asrar, Dalaran Underbelly
  (NULL, 32411, 0, 77, 80),
  -- Caregiver Poallu, Kaluak Camp, Borean Tundra
  (NULL, 27187, 0, 68, 72),
  -- Caregiver Iqniq, Kamagua, Howling Fjord
  (NULL, 27187, 0, 68, 72),
  -- Caregiver Mumik, Mo'Aki Harbor, Dragonblight
  (NULL, 27174, 0, 71, 73),
  -- Marissa Everwatch, The Argent Stand, Zul'Drak
  (NULL, 28791, 0, 73, 75),
  -- Pan'ya, Zim'Torga, Zul'Drak
  (NULL, 29583, 0, 75, 77),
  -- Purser Boulian, Nesingwary Base Camp, Sholazar Basin
  (NULL, 29583, 0, 75, 79),
  -- Smilin' Slirk Brassknob, K3, The Storm Peaks
  (NULL, 29904, 0, 77, 80),
  -- Magorn, Snow Drift Plains, The Storm Peaks
  (NULL, 29963, 0, 77, 80),
  -- Initiate Brenners, The Argent Stand, Zul'Drak
  -- (NULL, 30308, 0, 77, 80),
  --
  --
  --       UNUSED
  --
  --
  -- Bashana Runetotem, Thunder Bluff (Tauren npc in TB)
  -- (NULL, 9087, 6),
  --
  -- Alchemist Arbington, West Plaguelands, Human
  -- (NULL, 11056, 1),
  -- (NULL, 11056, 3),
  -- (NULL, 11056, 4),
  -- (NULL, 11056, 7),
  --
  -- Lokhtos Darkbargainer, Blackrock Depths, 12944
  --
  -- Gregan Brewspewer, Feralas, Dwarf (Some swarf Q-giver in Feralas)
  -- (NULL, 7775, 3),
  --
  -- Augustus the Touched, East Plaguelands, Undead (Some undead vendor near stratholme)
  -- (NULL, 12384, 5),
  --
  -- ENDING PLACEHOLDER
  (NULL, 6807, 15, 90, 90)
  ;