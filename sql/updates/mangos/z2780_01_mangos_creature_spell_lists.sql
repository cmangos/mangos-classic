ALTER TABLE db_version CHANGE COLUMN required_z2779_01_mangos_dbscript_data_flags_widening required_z2780_01_mangos_creature_spell_lists bit;

DROP TABLE IF EXISTS creature_spell_list_entry;
CREATE TABLE creature_spell_list_entry (
Id INT NOT NULL COMMENT 'List ID',
Name VARCHAR(200) NOT NULL COMMENT 'Description of usage',
ChanceSupportAction INT NOT NULL COMMENT 'Chance of support action per tick',
ChanceRangedAttack INT NOT NULL COMMENT 'Chance of ranged attack per tick',
PRIMARY KEY(Id)
);

DROP TABLE IF EXISTS creature_spell_list;
CREATE TABLE creature_spell_list (
Id INT NOT NULL COMMENT 'List ID',
Position INT NOT NULL COMMENT 'Position on list',
SpellId INT NOT NULL COMMENT 'Spell ID',
Flags INT NOT NULL COMMENT 'Spell Flags',
TargetId INT NOT NULL COMMENT 'Targeting ID',
ScriptId INT NOT NULL COMMENT 'Dbscript to be launched on success',
Availability INT NOT NULL COMMENT 'Chance on spawn for spell to be included',
Probability INT NOT NULL COMMENT 'Weight of spell when multiple are available',
InitialMin INT NOT NULL COMMENT 'Initial delay minimum',
InitialMax INT NOT NULL COMMENT 'Initial delay maximum',
RepeatMin INT NOT NULL COMMENT 'Repeated delay minimum',
RepeatMax INT NOT NULL COMMENT 'Repeated delay maximum',
Comments VARCHAR(255) NOT NULL COMMENT 'Description of spell use',
PRIMARY KEY(Id, Position)
);

DROP TABLE IF EXISTS creature_spell_targeting;
CREATE TABLE creature_spell_targeting (
Id INT NOT NULL COMMENT 'Targeting ID',
Type INT NOT NULL COMMENT 'Type of targeting ID',
Param1 INT NOT NULL COMMENT 'First parameter',
Param2 INT NOT NULL COMMENT 'Second parameter',
Param3 INT NOT NULL COMMENT 'Third parameter',
Comments VARCHAR(255) NOT NULL COMMENT 'Description of target',
PRIMARY KEY(Id)
);

ALTER TABLE creature_template ADD COLUMN SpellList INT NOT NULL DEFAULT '0' COMMENT 'creature_spell_list_entry' AFTER CorpseDecay;

REPLACE INTO creature_spell_targeting(Id, Type, Param1, Param2, Param3, Comments) VALUES
(0, 0, 0, 0, 0, 'Hardcoded - none'),
(1, 0, 0, 0, 0, 'Hardcoded - current'),
(2, 0, 0, 0, 0, 'Hardcoded - self'),
(3, 0, 0, 0, 0, 'Hardcoded - eligible friendly dispel'),
(4, 0, 0, 0, 0, 'Hardcoded - eligible friendly dispel - skip self'),
(100, 1, 0, 0, 0x0002, 'Attack - random player');

