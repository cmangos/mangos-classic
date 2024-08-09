/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MANGOSSERVER_CREATURE_H
#define MANGOSSERVER_CREATURE_H

#include "Common.h"
#include "Entities/Unit.h"
#include "Globals/SharedDefines.h"
#include "Entities/CreatureDefines.h"
#include "Server/DBCEnums.h"
#include "Util/Util.h"
#include "Entities/CreatureSpellList.h"
#include "Entities/CreatureSettings.h"

#include <list>
#include <memory>
#include <optional>

struct SpellEntry;

class UnitAI;
class Group;
class Quest;
class Player;
class WorldSession;
class CreatureGroup;

struct GameEventCreatureData;
enum class VisibilityDistanceType : uint32;

enum CreatureFlagsExtra
{
    CREATURE_EXTRA_FLAG_INSTANCE_BIND          = 0x00000001,       // 1 creature kill bind instance with killer and killer's group
    CREATURE_EXTRA_FLAG_NO_AGGRO_ON_SIGHT      = 0x00000002,       // 2 no aggro (ignore faction/reputation hostility)
    CREATURE_EXTRA_FLAG_NO_PARRY               = 0x00000004,       // 4 creature can't parry
    CREATURE_EXTRA_FLAG_NO_PARRY_HASTEN        = 0x00000008,       // 8 creature can't counter-attack at parry
    CREATURE_EXTRA_FLAG_NO_BLOCK               = 0x00000010,       // 16 creature can't block
    CREATURE_EXTRA_FLAG_UNUSED                 = 0x00000020,       // 32
    CREATURE_EXTRA_FLAG_UNUSED2                = 0x00000040,       // 64
    CREATURE_EXTRA_FLAG_INVISIBLE              = 0x00000080,       // 128 creature is always invisible for player (mostly trigger creatures)
    CREATURE_EXTRA_FLAG_UNUSED3                = 0x00000100,       // 256
    CREATURE_EXTRA_FLAG_AGGRO_ZONE             = 0x00000200,       // 512 creature sets itself in combat with zone on aggro
    CREATURE_EXTRA_FLAG_GUARD                  = 0x00000400,       // 1024 creature is a guard
    CREATURE_EXTRA_FLAG_NO_CALL_ASSIST         = 0x00000800,       // 2048 creature shouldn't call for assistance on aggro
    CREATURE_EXTRA_FLAG_ACTIVE                 = 0x00001000,       // 4096 creature is active object. Grid of this creature will be loaded and creature set as active
    CREATURE_EXTRA_FLAG_MMAP_FORCE_ENABLE      = 0x00002000,       // 8192 creature is forced to use MMaps
    CREATURE_EXTRA_FLAG_MMAP_FORCE_DISABLE     = 0x00004000,       // 16384 creature is forced to NOT use MMaps
    CREATURE_EXTRA_FLAG_WALK_IN_WATER          = 0x00008000,       // 32768 creature is forced to walk in water even it can swim
    CREATURE_EXTRA_FLAG_CIVILIAN               = 0x00010000,       // 65536 CreatureInfo->civilian substitute (for new expansions)
    CREATURE_EXTRA_FLAG_NO_MELEE               = 0x00020000,       // 131072 creature can't melee
    CREATURE_EXTRA_FLAG_FORCE_ATTACKING_CAPABILITY = 0x00080000,   // 524288 SetForceAttackingCapability(true); for nonattackable, nontargetable creatures that should be able to attack nontheless
    CREATURE_EXTRA_FLAG_DYNGUID                = 0x00100000,       // 1048576 Temporary transition flag - spawns of this entry use dynguid system
    CREATURE_EXTRA_FLAG_COUNT_SPAWNS           = 0x00200000,       // 2097152 count creature spawns in Map*
    CREATURE_EXTRA_FLAG_UNUSED4                = 0x00400000,       // 4194304
    CREATURE_EXTRA_FLAG_DUAL_WIELD_FORCED      = 0x00800000,       // 8388606 creature is alwyas dual wielding (even if unarmed)
    CREATURE_EXTRA_FLAG_UNUSED5                = 0x01000000,       // 16777216
};

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push,N), also any gcc version not support it at some platform
#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

#define MAX_KILL_CREDIT 2
#define MAX_CREATURE_MODEL 4                                // only single send to client in static data
#define USE_DEFAULT_DATABASE_LEVEL  0                       // just used to show we don't want to force the new creature level and use the level stored in db
#define MINIMUM_LOOTING_TIME (2 * MINUTE * IN_MILLISECONDS) // give player enough time to pick loot

// from `creature_template` table
struct CreatureInfo
{
    uint32  Entry;
    char*   Name;
    char*   SubName;
    uint32  MinLevel;
    uint32  MaxLevel;
    uint32  DisplayId[MAX_CREATURE_MODEL];
    uint32  DisplayIdProbability[MAX_CREATURE_MODEL];
    uint32  Faction;
    float   Scale;
    uint32  Family;                                         // enum CreatureFamily values (optional)
    uint32  CreatureType;                                   // enum CreatureType values
    uint32  InhabitType;
    uint32  RegenerateStats;
    bool    RacialLeader;
    uint32  NpcFlags;
    uint32  UnitFlags;                                      // enum UnitFlags mask values
    uint32  DynamicFlags;
    uint32  ExtraFlags;
    uint32  TypeFlags;                                      // enum TypeFlags mask values
    uint32  StaticFlags;
    uint32  StaticFlags2;
    uint32  StaticFlags3;
    uint32  StaticFlags4;
    float   SpeedWalk;
    float   SpeedRun;
    uint32  Detection;                                      // Detection Range for Line of Sight aggro
    uint32  CallForHelp;
    uint32  Pursuit;
    uint32  Leash;
    uint32  Timeout;
    uint32  UnitClass;                                      // enum Classes. Note only 4 classes are known for creatures.
    uint32  Rank;
    float   HealthMultiplier;
    float   PowerMultiplier;
    float   DamageMultiplier;
    float   DamageVariance;
    float   ArmorMultiplier;
    float   ExperienceMultiplier;
    float   StrengthMultiplier;
    float   AgilityMultiplier;
    float   StaminaMultiplier;
    float   IntellectMultiplier;
    float   SpiritMultiplier;
    uint32  MinLevelHealth;
    uint32  MaxLevelHealth;
    uint32  MinLevelMana;
    uint32  MaxLevelMana;
    float   MinMeleeDmg;
    float   MaxMeleeDmg;
    float   MinRangedDmg;
    float   MaxRangedDmg;
    uint32  Armor;
    uint32  MeleeAttackPower;
    uint32  RangedAttackPower;
    uint32  MeleeBaseAttackTime;
    uint32  RangedBaseAttackTime;
    uint32  DamageSchool;
    uint32  MinLootGold;
    uint32  MaxLootGold;
    uint32  LootId;
    uint32  PickpocketLootId;
    uint32  SkinningLootId;
    uint32  KillCredit[MAX_KILL_CREDIT];
    uint32  MechanicImmuneMask;
    uint32  SchoolImmuneMask;
    int32   ResistanceHoly;
    int32   ResistanceFire;
    int32   ResistanceNature;
    int32   ResistanceFrost;
    int32   ResistanceShadow;
    int32   ResistanceArcane;
    uint32  PetSpellDataId;
    uint32  MovementType;
    uint32  TrainerType;
    uint32  TrainerSpell;
    uint32  TrainerClass;
    uint32  TrainerRace;
    uint32  TrainerTemplateId;
    uint32  VendorTemplateId;
    uint32  GossipMenuId;
    uint32  InteractionPauseTimer;
    uint32  CorpseDelay;
    uint32  SpellList;
    uint32  CharmedSpellList;
    uint32  StringID1;
    uint32  StringID2;
    uint32  EquipmentTemplateId;
    uint32  Civilian;
    char const* AIName;
    uint32  ScriptID;

    // helpers
    static HighGuid GetHighGuid()
    {
        return HIGHGUID_UNIT;                               // in pre-3.x always HIGHGUID_UNIT
    }

    ObjectGuid GetObjectGuid(uint32 lowguid) const { return ObjectGuid(GetHighGuid(), Entry, lowguid); }

    SkillType GetRequiredLootSkill() const
    {
        if (HasFlag(CreatureTypeFlags::SKIN_WITH_HERBALISM))
            return SKILL_HERBALISM;
        if (HasFlag(CreatureTypeFlags::SKIN_WITH_MINING))
            return SKILL_MINING;

        return SKILL_SKINNING;                          // normal case
    }

    bool isTameable() const
    {
        return CreatureType == CREATURE_TYPE_BEAST && Family != 0 && HasFlag(CreatureTypeFlags::TAMEABLE);
    }

    bool HasFlag(CreatureTypeFlags flags) const
    {
        return bool(CreatureTypeFlags(TypeFlags) & flags);
    }
};

struct CreatureCooldowns
{
    uint32 entry;
    uint32 spellId;
    uint32 cooldownMin;
    uint32 cooldownMax;
};

struct EquipmentInfo
{
    uint32 entry;
    uint32 equipentry[3];
};

// depricated old way
struct EquipmentInfoRaw
{
    uint32  entry;
    uint32  equipmodel[3];
    uint32  equipinfo[3];
    uint32  equipslot[3];
};

enum SpawnFlags
{
    SPAWN_FLAG_RUN_ON_SPAWN = 0x01,
    SPAWN_FLAG_HOVER        = 0x02,
};

struct CreatureSpawnTemplate
{
    uint32 entry;
    int32 npcFlags;
    int64 unitFlags;
    uint32 faction;
    uint32 modelId;
    int32 equipmentId;
    uint32 curHealth;
    uint32 curMana;
    uint32 spawnFlags;
    uint32 relayId;
    uint32 stringId;

    bool IsRunning() const { return (spawnFlags & SPAWN_FLAG_RUN_ON_SPAWN) != 0; }
    bool IsHovering() const { return (spawnFlags & SPAWN_FLAG_HOVER) != 0; }
};

// from `creature` table
struct CreatureData
{
    uint32 id;                                              // entry in creature_template
    uint16 mapid;
    float posX;
    float posY;
    float posZ;
    float orientation;
    uint32 spawntimesecsmin;
    uint32 spawntimesecsmax;
    float spawndist;
    uint8 movementType;
    uint8 spawnMask;
    int16 gameEvent;
    uint16 GuidPoolId;
    uint16 EntryPoolId;
    uint16 OriginalZoneId;
    CreatureSpawnTemplate const* spawnTemplate;

    // helper function
    ObjectGuid GetObjectGuid(uint32 lowguid) const { return ObjectGuid(CreatureInfo::GetHighGuid(), id, lowguid); }
    uint32 GetRandomRespawnTime() const { return urand(spawntimesecsmin, spawntimesecsmax); }

    // return false if it should be handled by GameEventMgr or PoolMgr
    bool IsNotPartOfPoolOrEvent() const { return (!gameEvent && !GuidPoolId && !EntryPoolId); }
};

enum SplineFlags
{
    SPLINEFLAG_WALKMODE     = 0x0000100,
    SPLINEFLAG_FLYING       = 0x0000200,
};

// from `creature_addon` and `creature_template_addon`tables
struct CreatureDataAddon
{
    uint32 guidOrEntry;
    uint32 mount;
    uint8  standState;
    uint8  sheath_state;                                    // SheathState
    uint32 emote;
    uint32 move_flags;
    uint32 const* auras;                                    // loaded as char* "spell1 spell2 ... "
};

// Bases values for given Level and UnitClass
struct CreatureClassLvlStats
{
    uint32  BaseHealth;
    uint32  BaseMana;
    float   BaseDamage;
    float   BaseMeleeAttackPower;
    float   BaseRangedAttackPower;
    uint32  BaseArmor;
    uint32  Strength;
    uint32  Agility;
    uint32  Stamina;
    uint32  Intellect;
    uint32  Spirit;
};

struct CreatureModelInfo
{
    uint32 modelid;
    float bounding_radius;
    float combat_reach;
    float SpeedWalk;
    float SpeedRun;
    uint8 gender;
    uint32 modelid_other_gender;                            // The oposite gender for this modelid (male/female)
    uint32 modelid_alternative;                              // The oposite team. Generally for alliance totem
};

struct CreatureConditionalSpawn
{
    uint32 Guid;
    uint32 EntryAlliance;
    uint32 EntryHorde;
    // Note: future condition flags to be added
};

// GCC have alternative #pragma pack() syntax and old gcc version not support pack(pop), also any gcc version not support it at some platform
#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

struct CreatureLocale
{
    std::vector<std::string> Name;
    std::vector<std::string> SubName;
};

struct GossipMenuItemsLocale
{
    std::vector<std::string> OptionText;
    std::vector<std::string> BoxText;
};

struct PointOfInterestLocale
{
    std::vector<std::string> IconName;
};

struct AreaTriggerLocale
{
    std::vector<std::string> StatusFailed;
};

enum InhabitTypeValues
{
    INHABIT_GROUND = 1,
    INHABIT_WATER  = 2,
    INHABIT_AIR    = 4,
    INHABIT_ANYWHERE = INHABIT_GROUND | INHABIT_WATER | INHABIT_AIR
};

// Enums used by StringTextData::Type (CreatureEventAI)
enum ChatType
{
    CHAT_TYPE_SAY               = 0,
    CHAT_TYPE_YELL              = 1,
    CHAT_TYPE_TEXT_EMOTE        = 2,
    CHAT_TYPE_BOSS_EMOTE        = 3,
    CHAT_TYPE_WHISPER           = 4,
    CHAT_TYPE_BOSS_WHISPER      = 5,
    CHAT_TYPE_ZONE_YELL         = 6,
    CHAT_TYPE_ZONE_EMOTE        = 7,
    CHAT_TYPE_PARTY             = 8,
    CHAT_TYPE_MAX
};

enum RegenStatsFlags
{
    REGEN_FLAG_HEALTH_IN_COMBAT     = 0x001, // placeholder for future
    REGEN_FLAG_HEALTH               = 0x002,
    REGEN_FLAG_POWER_IN_COMBAT      = 0x004,
    REGEN_FLAG_POWER                = 0x008,
};

// Vendors
struct VendorItem
{
    VendorItem(uint32 _item, uint32 _maxcount, uint32 _incrtime, uint16 _conditionId)
        : item(_item), maxcount(_maxcount), incrtime(_incrtime), conditionId(_conditionId) {}

    uint32 item;
    uint32 maxcount;                                        // 0 for infinity item amount
    uint32 incrtime;                                        // time for restore items amount if maxcount != 0
    uint16 conditionId;                                     // condition to check for this item
};
typedef std::vector<VendorItem*> VendorItemList;

struct VendorItemData
{
    VendorItemList m_items;

    VendorItem* GetItem(uint32 slot) const
    {
        if (slot >= m_items.size()) return nullptr;
        return m_items[slot];
    }
    bool Empty() const { return m_items.empty(); }
    uint8 GetItemCount() const { return m_items.size(); }
    void AddItem(uint32 item, uint32 maxcount, uint32 ptime, uint16 conditonId)
    {
        m_items.push_back(new VendorItem(item, maxcount, ptime, conditonId));
    }
    bool RemoveItem(uint32 item_id);
    VendorItem const* FindItem(uint32 item_id) const;
    size_t FindItemSlot(uint32 item_id) const;

    void Clear()
    {
        for (VendorItemList::const_iterator itr = m_items.begin(); itr != m_items.end(); ++itr)
            delete (*itr);
        m_items.clear();
    }
};

struct VendorItemCount
{
    explicit VendorItemCount(uint32 _item, uint32 _count)
        : itemId(_item), count(_count), lastIncrementTime(time(nullptr)) {}

    uint32 itemId;
    uint32 count;
    time_t lastIncrementTime;
};

typedef std::list<VendorItemCount> VendorItemCounts;

struct TrainerSpell
{
    TrainerSpell() : spell(0), spellCost(0), reqSkill(0), reqSkillValue(0), reqLevel(0), learnedSpell(0), conditionId(0), isProvidedReqLevel(false) {}

    TrainerSpell(uint32 _spell, uint32 _spellCost, uint32 _reqSkill, uint32 _reqSkillValue, uint32 _reqLevel, uint32 _learnedspell, bool _isProvidedReqLevel, uint32 _conditionId)
        : spell(_spell), spellCost(_spellCost), reqSkill(_reqSkill), reqSkillValue(_reqSkillValue), reqLevel(_reqLevel), learnedSpell(_learnedspell), conditionId(_conditionId), isProvidedReqLevel(_isProvidedReqLevel) {}

    uint32 spell;
    uint32 spellCost;
    uint32 reqSkill;
    uint32 reqSkillValue;
    uint32 reqLevel;
    uint32 learnedSpell;
    std::array<std::optional<uint32>, 3> reqAbility;
    uint32 conditionId;
    bool isProvidedReqLevel;

    // helpers
    bool IsCastable() const { return learnedSpell != spell; }
};

typedef std::unordered_map < uint32 /*spellid*/, TrainerSpell > TrainerSpellMap;

struct TrainerSpellData
{
    TrainerSpellData() : trainerType(0) {}

    TrainerSpellMap spellList;
    uint32 trainerType;                                     // trainer type based at trainer spells, can be different from creature_template value.
    // req. for correct show non-prof. trainers like weaponmaster, allowed values 0 and 2.
    TrainerSpell const* Find(uint32 spell_id) const;
    void Clear() { spellList.clear(); }
};

// max different by z coordinate for creature aggro reaction
#define CREATURE_Z_ATTACK_RANGE_MELEE  3
#define CREATURE_Z_ATTACK_RANGE_RANGED 15

#define MAX_VENDOR_ITEMS 128                                // Limitation in item count field size in SMSG_LIST_INVENTORY - different in vanilla

enum VirtualItemSlot
{
    VIRTUAL_ITEM_SLOT_0 = 0,
    VIRTUAL_ITEM_SLOT_1 = 1,
    VIRTUAL_ITEM_SLOT_2 = 2,
};

#define MAX_VIRTUAL_ITEM_SLOT 3

enum VirtualItemInfoByteOffset
{
    VIRTUAL_ITEM_INFO_0_OFFSET_CLASS         = 0,
    VIRTUAL_ITEM_INFO_0_OFFSET_SUBCLASS      = 1,
    VIRTUAL_ITEM_INFO_0_OFFSET_MATERIAL      = 2,
    VIRTUAL_ITEM_INFO_0_OFFSET_INVENTORYTYPE = 3,

    VIRTUAL_ITEM_INFO_1_OFFSET_SHEATH        = 0,
};

struct CreatureCreatePos
{
    public:
        // exactly coordinates used
        CreatureCreatePos(Map* map, float x, float y, float z, float o)
            : m_map(map), m_closeObject(nullptr), m_angle(0.0f), m_dist(0.0f) { m_pos.x = x; m_pos.y = y; m_pos.z = z; m_pos.o = o; }
        // if dist == 0.0f -> exactly object coordinates used, in other case close point to object (CONTACT_DIST can be used as minimal distances)
        CreatureCreatePos(WorldObject* closeObject, float ori, float dist = 0.0f, float angle = 0.0f)
            : m_map(closeObject->GetMap()),
              m_closeObject(closeObject), m_angle(angle), m_dist(dist) { m_pos.o = ori; }
    public:
        Map* GetMap() const { return m_map; }
        void SelectFinalPoint(Creature* cr, bool staticSpawn);
        bool Relocate(Creature* cr) const;

        // read only after SelectFinalPoint
        Position m_pos;
    private:
        Map* m_map;
        WorldObject* m_closeObject;
        float m_angle;
        float m_dist;
};

enum CreatureSubtype
{
    CREATURE_SUBTYPE_GENERIC,                               // new Creature
    CREATURE_SUBTYPE_PET,                                   // new Pet
    CREATURE_SUBTYPE_TOTEM,                                 // new Totem
    CREATURE_SUBTYPE_TEMPORARY_SUMMON,                      // new TemporarySummon
};

enum TemporaryFactionFlags                                  // Used at real faction changes
{
    TEMPFACTION_NONE                    = 0x00,             // When no flag is used in temporary faction change, faction will be persistent. It will then require manual change back to default/another faction when changed once
    TEMPFACTION_RESTORE_RESPAWN         = 0x01,             // Default faction will be restored at respawn
    TEMPFACTION_RESTORE_COMBAT_STOP     = 0x02,             // ... at CombatStop() (happens at creature death, at evade or custom scripte among others)
    TEMPFACTION_RESTORE_REACH_HOME      = 0x04,             // ... at reaching home in home movement (evade), if not already done at CombatStop()
    TEMPFACTION_TOGGLE_NON_ATTACKABLE   = 0x08,             // Remove UNIT_FLAG_SPAWNING(0x02) when faction is changed (reapply when temp-faction is removed)
    TEMPFACTION_TOGGLE_IMMUNE_TO_PLAYER = 0x10,             // Remove UNIT_FLAG_IMMUNE_TO_PLAYER(0x100, 256) when faction is changed (reapply when temp-faction is removed)
    TEMPFACTION_TOGGLE_IMMUNE_TO_NPC    = 0x20,             // Remove UNIT_FLAG_IMMUNE_TO_NPC(0x200, 512) when faction is changed (reapply when temp-faction is removed)
    TEMPFACTION_TOGGLE_PACIFIED         = 0x40,             // Remove UNIT_FLAG_PACIFIED(0x20000) when faction is changed (reapply when temp-faction is removed)
    TEMPFACTION_TOGGLE_NOT_SELECTABLE   = 0x80,             // Remove UNIT_FLAG_UNINTERACTIBLE(0x2000000) when faction is changed (reapply when temp-faction is removed)
    TEMPFACTION_ALL,
};

class Creature : public Unit
{
    public:

        explicit Creature(CreatureSubtype subtype = CREATURE_SUBTYPE_GENERIC);
        virtual ~Creature();

        void AddToWorld() override;
        void RemoveFromWorld() override;
        void CleanupsBeforeDelete() override;

        bool Create(uint32 dbGuid, uint32 guidlow, CreatureCreatePos& cPos, CreatureInfo const* cinfo, const CreatureData* data = nullptr, GameEventCreatureData const* eventData = nullptr);
        bool LoadCreatureAddon(bool reload);

        float GetNativeScale() const override;

        // SelectLevel set creature bases stats for given level or for default levels stored in db
        void SelectLevel(uint32 forcedLevel = USE_DEFAULT_DATABASE_LEVEL);
        void LoadEquipment(uint32 equip_entry, bool force = false);

        bool HasStaticDBSpawnData() const;                  // listed in `creature` table and have fixed in DB guid

        char const* GetSubName() const { return GetCreatureInfo()->SubName; }

        void Update(const uint32 diff) override;  // overwrite Unit::Update

        virtual void RegenerateAll(uint32 update_diff);
        uint32 GetEquipmentId() const { return m_equipmentId; }

        CreatureSubtype GetSubtype() const { return m_subtype; }
        bool IsPet() const { return m_subtype == CREATURE_SUBTYPE_PET; }
        bool IsTotem() const { return m_subtype == CREATURE_SUBTYPE_TOTEM; }
        bool IsTemporarySummon() const { return m_subtype == CREATURE_SUBTYPE_TEMPORARY_SUMMON; }
        bool IsCritter() const { return m_creatureInfo->CreatureType == CREATURE_TYPE_CRITTER; }

#ifdef BUILD_DEPRECATED_PLAYERBOT
        // Adds functionality to load/unload bots from NPC, also need to apply SQL scripts
        void LoadBotMenu(Player* pPlayer);
#endif

        bool IsCorpse() const { return GetDeathState() == CORPSE; }
        bool IsDespawned() const { return GetDeathState() == DEAD; }
        void SetCorpseDelay(uint32 delay) { m_corpseDelay = delay; }
        void ReduceCorpseDecayTimer();
        TimePoint GetCorpseDecayTimer() const { return m_corpseExpirationTime; }
        bool CanRestockPickpocketLoot() const;
        void StartPickpocketRestockTimer();
        bool IsRacialLeader() const { return GetCreatureInfo()->RacialLeader; }
        bool IsCivilian() const { return GetCreatureInfo()->Civilian != 0; }
        bool IsNoAggroOnSight() const { return (GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_NO_AGGRO_ON_SIGHT) != 0; }
        bool IsGuard() const { return (GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_GUARD) != 0; }

        bool CanWalk() const override { return (GetCreatureInfo()->InhabitType & INHABIT_GROUND) != 0; }
        bool CanSwim() const override { return (GetCreatureInfo()->InhabitType & INHABIT_WATER) != 0; }
        bool IsSwimming() const { return m_movementInfo.HasMovementFlag(MOVEFLAG_SWIMMING); }
        bool CanFly() const override { return (GetCreatureInfo()->InhabitType & INHABIT_AIR) || m_movementInfo.HasMovementFlag((MovementFlags)(MOVEFLAG_LEVITATING | MOVEFLAG_HOVER | MOVEFLAG_CAN_FLY)); }
        bool IsFlying() const override { return m_movementInfo.HasMovementFlag((MovementFlags)(MOVEFLAG_FLYING | MOVEFLAG_HOVER | MOVEFLAG_LEVITATING)); }
        bool IsTrainerOf(Player* pPlayer, bool msg) const;
        bool CanInteractWithBattleMaster(Player* pPlayer, bool msg) const;
        bool CanTrainAndResetTalentsOf(Player* pPlayer) const;

        void FillGuidsListFromThreatList(GuidVector& guids, uint32 maxamount = 0);

        bool IsImmuneToSpell(SpellEntry const* spellInfo, bool castOnSelf, uint8 effectMask, WorldObject const* caster) override;
        bool IsImmuneToDamage(SpellSchoolMask meleeSchoolMask) override;
        bool IsImmuneToSpellEffect(SpellEntry const* spellInfo, SpellEffectIndex index, bool castOnSelf) const override;

        bool IsElite() const
        {
            if (IsPet())
                return false;

            uint32 rank = GetCreatureInfo()->Rank;
            return rank != CREATURE_ELITE_NORMAL && rank != CREATURE_ELITE_RARE;
        }

        bool IsWorldBoss() const
        {
            if (IsPet())
                return false;

            return GetCreatureInfo()->Rank == CREATURE_ELITE_WORLDBOSS;
        }

        uint32 GetLevelForTarget(Unit const* target) const override; // overwrite Unit::GetLevelForTarget for boss level support

        bool AIM_Initialize();

        virtual UnitAI* AI() override
        {
            if (m_charmInfo && m_charmInfo->GetAI()) return m_charmInfo->GetAI();
            return m_ai.get();
        }

        CombatData* GetCombatData() override
        {
            if (m_charmInfo && m_charmInfo->GetCombatData())
                return m_charmInfo->GetCombatData();
            return m_combatData;
        }

        CombatData const* GetCombatData() const override
        {
            if (m_charmInfo && m_charmInfo->GetCombatData())
                return m_charmInfo->GetCombatData();
            return m_combatData;
        }

        void SetWalk(bool enable, bool asDefault = true);

        // TODO: Research mob shield block values
        uint32 GetShieldBlockValue() const override { return (GetLevel() / 2 + uint32(GetStat(STAT_STRENGTH) / 20)); }

        bool HasSpell(uint32 spellID) const override;
        void UpdateSpell(int32 index, int32 newSpellId);
        void SetSpellList(uint32 spellSet);
        void UpdateImmunitiesSet(uint32 immunitySet);

        bool UpdateEntry(uint32 Entry, const CreatureData* data = nullptr, GameEventCreatureData const* eventData = nullptr, bool preserveHPAndPower = true);
        void ResetEntry(bool respawn = false);

        void ApplyGameEventSpells(GameEventCreatureData const* eventData, bool activated);
        bool UpdateStats(Stats stat) override;
        bool UpdateAllStats() override;
        void UpdateResistances(uint32 school) override;
        void UpdateArmor() override;
        void UpdateAttackPowerAndDamage(bool ranged = false) override;
        void UpdateDamagePhysical(WeaponAttackType attType) override;
        virtual float GetConditionalTotalPhysicalDamageModifier(WeaponAttackType type) const;
        float GetHealthBonusFromStamina() const override;

        uint32 GetCurrentEquipmentId() const { return m_equipmentId; }

        static float _GetHealthMod(int32 Rank);             ///< Get custom factor to scale health (default 1, CONFIG_FLOAT_RATE_CREATURE_*_HP)
        static float _GetDamageMod(int32 Rank);             ///< Get custom factor to scale damage (default 1, CONFIG_FLOAT_RATE_*_DAMAGE)
        static float _GetSpellDamageMod(int32 Rank);        ///< Get custom factor to scale spell damage (default 1, CONFIG_FLOAT_RATE_*_SPELLDAMAGE)

        VendorItemData const* GetVendorItems() const;
        VendorItemData const* GetVendorTemplateItems() const;
        uint32 GetVendorItemCurrentCount(VendorItem const* vItem);
        uint32 UpdateVendorItemCurrentCount(VendorItem const* vItem, uint32 used_count);

        TrainerSpellData const* GetTrainerTemplateSpells() const;
        TrainerSpellData const* GetTrainerSpells() const;

        CreatureInfo const* GetCreatureInfo() const { return m_creatureInfo; }
        CreatureDataAddon const* GetCreatureAddon() const;

        static uint32 ChooseDisplayId(const CreatureInfo* cinfo, const CreatureData* data = nullptr, GameEventCreatureData const* eventData = nullptr);

        std::string GetAIName() const;
        std::string GetScriptName() const;
        uint32 GetScriptId() const;

        // overwrite WorldObject function for proper name localization
        const char* GetNameForLocaleIdx(int32 loc_idx) const override;

        void SetDeathState(DeathState s) override;          // overwrite virtual Unit::SetDeathState

        bool LoadFromDB(uint32 dbGuid, Map* map, uint32 newGuid, uint32 forcedEntry, GenericTransport* transport = nullptr);
        virtual void SaveToDB();
        // overwrited in Pet
        virtual void SaveToDB(uint32 mapid);
        virtual void DeleteFromDB();                        // overwrited in Pet
        static void DeleteFromDB(uint32 lowguid, CreatureData const* data);

        void PrepareBodyLootState(Unit* killer);
        CreatureLootStatus GetLootStatus() const { return m_lootStatus; }
        virtual void InspectingLoot() override;
        void SetLootStatus(CreatureLootStatus status, bool forced = false);
        bool IsTappedBy(Player* plr) const;
        ObjectGuid GetLootRecipientGuid() const { return m_lootRecipientGuid; }
        uint32 GetLootGroupRecipientId() const { return m_lootGroupRecipientId; }
        Player* GetLootRecipient() const;                   // use group cases as prefered
        Group* GetGroupLootRecipient() const;
        void SetCorpseAccelerationDelay(uint32 delay) { m_corpseAccelerationDecayDelay = delay; } // in miliseconds

        bool HasLootRecipient() const { return m_lootGroupRecipientId || m_lootRecipientGuid; }
        bool IsGroupLootRecipient() const { return m_lootGroupRecipientId != 0; }
        void SetLootRecipient(Unit* unit);
        Player* GetOriginalLootRecipient() const;           // ignore group changes/etc, not for looting

        SpellEntry const* ReachWithSpellAttack(Unit* pVictim);
        SpellEntry const* ReachWithSpellCure(Unit* pVictim);

        void CallForHelp(float radius);
        void CallAssistance();
        void CallAssistance(Unit* enemy);
        void SetNoCallAssistance(bool val) { m_AlreadyCallAssistance = val; }
        bool CanAssistTo(const Unit* u, const Unit* enemy, bool checkfaction = true) const;
        bool CanInitiateAttack() const;
        bool CanCallForAssistance() const override { return m_canCallForAssistance; }
        void SetCanCallForAssistance(bool state) { m_canCallForAssistance = state; }
        bool IsInGroup(Unit const* other, bool party/* = false*/, bool ignoreCharms/* = false*/) const override;

        MovementGeneratorType GetDefaultMovementType() const { return m_defaultMovementType; }
        void SetDefaultMovementType(MovementGeneratorType mgt) { m_defaultMovementType = mgt; }

        bool IsVisibleInGridForPlayer(Player* pl) const override;

        void RemoveCorpse(bool inPlace = false);

        virtual void ForcedDespawn(uint32 timeMSToDespawn = 0, bool onlyAlive = false);
        virtual void ForcedDespawn(std::chrono::milliseconds timeToDespawn, bool onlyAlive = false) { ForcedDespawn(timeToDespawn.count(), onlyAlive); }

        time_t const& GetRespawnTime() const { return m_respawnTime; }
        time_t GetRespawnTimeEx() const;
        void SetRespawnTime(uint32 respawn) { m_respawnTime = respawn ? time(nullptr) + respawn : 0; } // do not use this from scripts
        void Respawn();
        void SaveRespawnTime() override;

        uint32 GetRespawnDelay() const override { return m_respawnDelay; }
        void SetRespawnDelay(uint32 delay, bool once = false) { m_respawnDelay = delay; m_respawnOverriden = true; m_respawnOverrideOnce = once; } // in seconds
        void SetRespawnDelay(std::chrono::seconds delay, bool once = false) { SetRespawnDelay(delay.count(), once); }

        float GetRespawnRadius() const { return m_respawnradius; }
        void SetRespawnRadius(float dist) { m_respawnradius = dist; }

        void SendZoneUnderAttackMessage(Player* attacker) const;

        void SetInCombatWithZone(bool checkAttackability = true);

        bool HasQuest(uint32 quest_id) const override;
        bool HasInvolvedQuest(uint32 quest_id)  const override;

        void SetDefaultGossipMenuId(uint32 menuId) { m_gossipMenuId = menuId; }
        uint32 GetDefaultGossipMenuId() const { return m_gossipMenuId; }
        uint32 GetInteractionPauseTimer() const { return m_interactionPauseTimer; }

        GridReference<Creature>& GetGridRef() { return m_gridRef; }
        bool IsRegeneratingHealth() const { return (GetCreatureInfo()->RegenerateStats & REGEN_FLAG_HEALTH) != 0 && !(GetCreatureInfo()->HasFlag(CreatureTypeFlags::ALLOW_INTERACTION_WHILE_IN_COMBAT)); }
        bool IsRegeneratingPower() const;
        virtual uint8 GetPetAutoSpellSize() const { return CREATURE_MAX_SPELLS; }
        virtual uint32 GetPetAutoSpellOnPos(uint8 pos) const
        {
            if (pos >= CREATURE_MAX_SPELLS || m_charmInfo->GetCharmSpell(pos)->GetType() != ACT_ENABLED)
                return 0;
            return m_charmInfo->GetCharmSpell(pos)->GetAction();
        }

        void SetCombatStartPosition(Position const& pos) { m_combatStartPos = pos; }
        void GetCombatStartPosition(Position& pos) const { pos = m_combatStartPos; }
        Position const& GetCombatStartPosition() const { return m_combatStartPos; }
        uint32 GetPursuit() const override { return GetCreatureInfo()->Pursuit; }

        void SetRespawnCoord(CreatureCreatePos const& pos) { m_respawnPos = pos.m_pos; }
        void SetRespawnCoord(float x, float y, float z, float ori) { m_respawnPos.x = x; m_respawnPos.y = y; m_respawnPos.z = z; m_respawnPos.o = ori; }
        void GetRespawnCoord(float& x, float& y, float& z, float* ori = nullptr, float* dist = nullptr) const;
        Position const& GetRespawnPosition() const { return m_respawnPos; }
        void ResetRespawnCoord();

        void SetFactionTemporary(uint32 factionId, uint32 tempFactionFlags = TEMPFACTION_ALL);
        void ClearTemporaryFaction();
        uint32 GetTemporaryFactionFlags() const { return m_temporaryFactionFlags; }

        void SendAreaSpiritHealerQueryOpcode(Player* pl);

        void SetVirtualItem(VirtualItemSlot slot, uint32 item_id);

        bool hasWeapon(WeaponAttackType type) const override;
        bool hasWeaponForAttack(WeaponAttackType type) const override { return (Unit::hasWeaponForAttack(type) && hasWeapon(type)); }
        virtual void SetCanDualWield(bool value) override;

        void SetInvisible(bool invisible) { m_isInvisible = invisible; }
        bool IsInvisible() const { return m_isInvisible; }

        void SetIgnoreMMAP(bool ignore) { m_ignoreMMAP = ignore; }
        virtual MmapForcingStatus IsIgnoringMMAP() const override;

        void OnEventHappened(uint16 eventId, bool activate, bool resume) override { return AI()->OnEventHappened(eventId, activate, resume); }

        void SetForceAttackingCapability(bool state) { m_forceAttackingCapability = state; }
        bool GetForceAttackingCapability() const { return m_forceAttackingCapability; }

        void SetSpawnCounting(bool state) { m_countSpawns = state; }

        uint32 GetDetectionRange() const override { return m_detectionRange; }
        void SetDetectionRange(uint32 range) { m_detectionRange = range; }

        void SetBaseWalkSpeed(float speed) override;
        void SetBaseRunSpeed(float speed, bool force = true) override;

        void LockOutSpells(SpellSchoolMask schoolMask, uint32 duration) override;

        bool CanAggro() const { return m_canAggro; }
        void SetCanAggro(bool canAggro) { m_canAggro = canAggro; }

        bool CanCheckForHelp() const override { return m_checkForHelp; }
        void SetCanCheckForHelp(bool state) { m_checkForHelp = state; }

        void SetNoRewards();
        bool IsNoXp();
        void SetNoXP(bool state);
        bool IsNoLoot();
        void SetNoLoot(bool state);
        bool IsNoReputation() { return m_noReputation; }
        void SetNoReputation(bool state) { m_noReputation = state; }
        bool IsIgnoringFeignDeath() const override;
        void SetIgnoreFeignDeath(bool state);

        void SetNoWoundedSlowdown(bool state);
        bool IsNoWoundedSlowdown() const;
        bool IsSlowedInCombat() const override;

        void SetNoWeaponSkillGain(bool state);
        bool IsNoWeaponSkillGain() const override;

        bool IsPreventingDeath() const override;

        virtual void AddCooldown(SpellEntry const& spellEntry, ItemPrototype const* itemProto = nullptr, bool permanent = false, uint32 forcedDuration = 0, bool ignoreCat = false) override;

        // spell scripting persistency
        bool HasBeenHitBySpell(uint32 spellId);
        void RegisterHitBySpell(uint32 spellId);
        void ResetSpellHitCounter();

        HighGuid GetParentHigh() const override { return HIGHGUID_UNIT; }

        void Heartbeat() override;

        // Spell Lists
        CreatureSpellList const& GetSpellList() const { return m_spellList; }
        std::vector<uint32> GetCharmSpells() const override;
        enum CooldownResult
        {
            COOLDOWN_RESULT_NOT_FOUND       = 0,
            COOLDOWN_RESULT_FOUND           = 1,
            COOLDOWN_RESULT_CATEGORY_FOUND  = 2,
        };
        CooldownResult GetSpellCooldown(uint32 spellId, uint32& cooldown) const;

        void SetCreatureGroup(CreatureGroup* group);
        void ClearCreatureGroup();
        CreatureGroup* GetCreatureGroup() const { return m_creatureGroup; }

        CreatureSettings const& GetSettings() const { return m_settings; }
        CreatureSettings& GetSettings() { return m_settings; }

        void StartCooldown(Unit* owner);

        ObjectGuid GetKillerGuid() const { return m_killer; }
        void SetKillerGuid(ObjectGuid guid) { m_killer = guid; }

        CreatureInfo const* GetMountInfo() const override{ return m_mountInfo; }
        void SetMountInfo(CreatureInfo const* info) override;

        void SetModelRunSpeed(float runSpeed) override { m_modelRunSpeed = runSpeed; }

    protected:
        bool CreateFromProto(uint32 dbGuid, uint32 guidlow, CreatureInfo const* cinfo, const CreatureData* data = nullptr, GameEventCreatureData const* eventData = nullptr);
        bool InitEntry(uint32 Entry, const CreatureData* data = nullptr, GameEventCreatureData const* eventData = nullptr);

        uint32 GetCreatureConditionalSpawnEntry(uint32 guidlow, Map* map) const;

        void UnsummonCleanup(); // cleans up data before unsummon of various creatures

        bool IsCorpseExpired() const;

        // vendor items
        VendorItemCounts m_vendorItemCounts;

        uint32 m_gossipMenuId;
        uint32 m_lootMoney;
        ObjectGuid m_lootRecipientGuid;                     // player who will have rights for looting if m_lootGroupRecipient==0 or group disbanded
        uint32 m_lootGroupRecipientId;                      // group who will have rights for looting if set and exist
        CreatureLootStatus m_lootStatus;                    // loot status (used to know when we could loot, pickpocket or skin)
        uint32 m_corpseAccelerationDecayDelay;              // time for ReduceCorpseDecayTimer

        /// Timers
        TimePoint m_corpseExpirationTime;                   // (msecs) time point of corpse decay
        time_t m_respawnTime;                               // (secs) time of next respawn
        uint32 m_respawnDelay;                              // (secs) delay between corpse disappearance and respawning
        bool m_respawnOverriden;
        bool m_respawnOverrideOnce;
        uint32 m_corpseDelay;                               // (secs) delay between death and corpse disappearance
        TimePoint m_pickpocketRestockTime;                  // (msecs) time point of pickpocket restock
        bool m_canAggro;                                    // controls response of creature to attacks
        bool m_checkForHelp;                                // controls checkforhelp in ai
        float m_respawnradius;
        uint32 m_interactionPauseTimer;                     // (msecs) waypoint pause time when interacted with

        CreatureSubtype m_subtype;                          // set in Creatures subclasses for fast it detect without dynamic_cast use
        void RegeneratePower(float timerMultiplier);
        virtual void RegenerateHealth();
        MovementGeneratorType m_defaultMovementType;
        uint32 m_equipmentId;
        uint32 m_detectionRange;

        // below fields has potential for optimization
        bool m_AlreadyCallAssistance;
        bool m_canCallForAssistance;
        uint32 m_temporaryFactionFlags;                     // used for real faction changes (not auras etc)

        uint32 m_originalEntry;

        Position m_combatStartPos;                          // after combat contains last position
        Position m_respawnPos;

        uint32 m_gameEventVendorId;                         // game event creature data vendor id override

        uint32 m_immunitySet;

        std::unique_ptr<UnitAI> m_ai;
        bool m_isInvisible;
        bool m_ignoreMMAP;
        bool m_forceAttackingCapability;                    // can attack even if not selectable/not attackable
        bool m_noReputation;

        CreatureSettings m_settings;

        // Script logic
        bool m_countSpawns;

        // spell scripting persistency
        std::set<uint32> m_hitBySpells;

        // Spell Lists
        CreatureSpellList m_spellList;

        CreatureGroup* m_creatureGroup;

        ObjectGuid m_killer;

        bool m_imposedCooldown;

    private:
        GridReference<Creature> m_gridRef;
        CreatureInfo const* m_creatureInfo;

        CreatureInfo const* m_mountInfo;
        float m_modelRunSpeed;
};

class ForcedDespawnDelayEvent : public BasicEvent
{
    public:
        ForcedDespawnDelayEvent(Creature& owner, bool onlyAlive = false) : BasicEvent(), m_owner(owner), m_onlyAlive(onlyAlive) { }
        bool Execute(uint64 e_time, uint32 p_time) override;

    private:
        Creature& m_owner;
        bool m_onlyAlive;
};

#endif
