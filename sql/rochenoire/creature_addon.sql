-- PREPARE WORK
UPDATE tbcaowow.dbc_areatable SET areaLevel = 0;
REPLACE INTO tbcaowow.dbc_areatable (areaId, enUs, areaLevel, areaBit, areaFlags)
(
	SELECT c.areaId, area.enUS, ROUND(AVG(ct.MinLevel)) AS areaLevel, area.areaBit, area.areaFlags
	FROM creature c, creature_template ct, tbcaowow.dbc_areatable AREA
	WHERE c.id = ct.Entry
	AND ct.CreatureType IN (1,2,3,4,5,6,7,9)
	AND area.areaId = c.areaId
	AND ct.NpcFlags = 0
	AND ct.ExtraFlags = 0
	AND ct.UnitFlags = 0
	AND ct.`Faction` != 35 -- no friendly
	AND area.areaFlags != 0x138 -- no capitals
	GROUP BY c.areaId
	LIMIT 999999
);

-- DO THE WORK
SET sql_mode = 'NO_UNSIGNED_SUBTRACTION';
UPDATE creature_addon ca SET ca.lvar = 0;

INSERT IGNORE INTO creature_addon (guid) (SELECT guid FROM creature);

UPDATE creature c, creature_addon ca, creature_template ct, tbcaowow.dbc_areatable AREA
SET ca.lvar = (ct.MinLevel - area.areaLevel)
WHERE c.guid = ca.guid
AND c.id = ct.Entry
AND c.areaId = area.areaId
AND ct.CreatureType IN (1,2,3,4,5,6,7,9)
AND ct.NpcFlags = 0
AND ct.ExtraFlags = 0
AND ct.UnitFlags = 0
AND ct.`Faction` != 35 -- no friendly
AND area.areaFlags != 0x138; -- no capitals

DELETE FROM creature_addon WHERE lvar = 0 AND mount = 0 AND bytes1 = 0 AND b2_0_sheath = 0 AND b2_1_flags = 0 AND emote = 0 AND moveflags = 0 AND ISNULL(auras);