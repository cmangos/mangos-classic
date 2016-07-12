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
#include "Unit.h"
#include "SharedDefines.h"
#include "DBCEnums.h"
#include "Cell.h"

#include <list>

struct SpellEntry;

class CreatureAI;
class Group;
class Quest;
class Player;
class WorldSession;

struct GameEventCreatureData;

enum CreatureFlagsExtra
{
    CREATURE_EXTRA_FLAG_INSTANCE_BIND          = 0x00000001,       // creature kill bind instance with killer and killer's group
    CREATURE_EXTRA_FLAG_NO_AGGRO               = 0x00000002,       // not aggro (ignore faction/reputation hostility)
    CREATURE_EXTRA_FLAG_NO_PARRY               = 0x00000004,       // creature can't parry
    CREATURE_EXTRA_FLAG_NO_PARRY_HASTEN        = 0x00000008,       // creature can't counter-attack at parry
    CREATURE_EXTRA_FLAG_NO_BLOCK               = 0x00000010,       // creature can't block
    CREATURE_EXTRA_FLAG_NO_CRUSH               = 0x00000020,       // creature can't do crush attacks
    CREATURE_EXTRA_FLAG_NO_XP_AT_KILL          = 0x00000040,       // creature kill not provide XP
    CREATURE_EXTRA_FLAG_INVISIBLE              = 0x00000080,       // creature is always invisible for player (mostly trigger creatures)
    CREATURE_EXTRA_FLAG_NOT_TAUNTABLE          = 0x00000100,       // creature is immune to taunt auras and effect attack me
    CREATURE_EXTRA_FLAG_AGGRO_ZONE             = 0x00000200,       // creature sets itself in combat with zone on aggro
    CREATURE_EXTRA_FLAG_GUARD                  = 0x00000400,       // creature is a guard
    CREATURE_EXTRA_FLAG_NO_CALL_ASSIST         = 0x00000800,       // creature shouldn't call for assistance on aggro
    CREATURE_EXTRA_FLAG_ACTIVE                 = 0x00001000,       // creature is active object. Grid of this creature will be loaded and creature set as active
    CREATURE_EXTRA_FLAG_MMAP_FORCE_ENABLE      = 0x00002000,       // creature is forced to use MMaps
    CREATURE_EXTRA_FLAG_MMAP_FORCE_DISABLE     = 0x00004000,       // creature is forced to NOT use MMaps
    CREATURE_EXTRA_FLAG_WALK_IN_WATER          = 0x00008000,       // creature is forced to walk in water even it can swim
    CREATURE_EXTRA_FLAG_HAVE_NO_SWIM_ANIMATION = 0x00010000,       // we have to not set "swim" animation or creature will have "no animation"
    CREATURE_EXTRA_FLAG_NO_MELEE               = 0x00020000,       // creature can't melee
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

// from `creature_template` table
struct CreatureInfo
{
    uint32  Entry;
    char*   Name;
    char*   SubName;
    uint32  MinLevel;
    uint32  MaxLevel;
    uint32  ModelId[MAX_CREATURE_MODEL];
    uint32  FactionAlliance;
    uint32  FactionHorde;
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
    uint32  CreatureTypeFlags;                              // enum CreatureTypeFlags mask values
    float   SpeedWalk;
    float   SpeedRun;
    uint32  UnitClass;                                      // enum Classes. Note only 4 classes are known for creatures.
    uint32  Rank;
    float   HealthMultiplier;
    float   PowerMultiplier;
    float   DamageMultiplier;
    float   DamageVariance;
    float   ArmorMultiplier;
    float   ExperienceMultiplier;
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
    uint32  EquipmentTemplateId;
    uint32  civilian;
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
        if (CreatureTypeFlags & CREATURE_TYPEFLAGS_HERBLOOT)
            return SKILL_HERBALISM;
        else if (CreatureTypeFlags & CREATURE_TYPEFLAGS_MININGLOOT)
            return SKILL_MINING;
        else
            return SKILL_SKINNING;                          // normal case
    }

    bool isTameable() const
    {
        return CreatureType == CREATURE_TYPE_BEAST && Family != 0 && (CreatureTypeFlags & CREATURE_TYPEFLAGS_TAMEABLE);
    }
};

struct CreatureTemplateSpells
{
    uint32 entry;
    uint32 spells[CREATURE_MAX_SPELLS];
};

struct EquipmentInfo
{
    uint32  entry;
    uint32  equipentry[3];
};

// depricated old way
struct EquipmentInfoRaw
{
    uint32  entry;
    uint32  equipmodel[3];
    uint32  equipinfo[3];
    uint32  equipslot[3];
};

// from `creature` table
struct CreatureData
{
    uint32 id;                                              // entry in creature_template
    uint16 mapid;
    uint32 modelid_override;                                // overrides any model defined in creature_template
    int32 equipmentId;
    float posX;
    float posY;
    float posZ;
    float orientation;
    uint32 spawntimesecs;
    float spawndist;
    uint32 currentwaypoint;
    uint32 curhealth;
    uint32 curmana;
    bool  is_dead;
    uint8 movementType;

    // helper function
    ObjectGuid GetObjectGuid(uint32 lowguid) const { return ObjectGuid(CreatureInfo::GetHighGuid(), id, lowguid); }
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
    uint32 bytes1;
    uint8  sheath_state;                                    // SheathState
    uint8  flags;                                           // UnitBytes2_Flags
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
};

struct CreatureModelInfo
{
    uint32 modelid;
    float bounding_radius;
    float combat_reach;
    uint8 gender;
    uint32 modelid_other_gender;                            // The oposite gender for this modelid (male/female)
    uint32 modelid_other_team;                              // The oposite team. Generally for alliance totem
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
    CHAT_TYPE_ZONE_YELL         = 6
};

// Selection method used by SelectAttackingTarget
enum AttackingTarget
{
    ATTACKING_TARGET_RANDOM = 0,                            // Just selects a random target
    ATTACKING_TARGET_TOPAGGRO,                              // Selects targes from top aggro to bottom
    ATTACKING_TARGET_BOTTOMAGGRO,                           // Selects targets from bottom aggro to top
};

enum SelectFlags
{
    SELECT_FLAG_IN_LOS              = 0x001,                // Default Selection Requirement for Spell-targets
    SELECT_FLAG_PLAYER              = 0x002,
    SELECT_FLAG_POWER_MANA          = 0x004,                // For Energy based spells, like manaburn
    SELECT_FLAG_POWER_RAGE          = 0x008,
    SELECT_FLAG_POWER_ENERGY        = 0x010,
    SELECT_FLAG_IN_MELEE_RANGE      = 0x040,
    SELECT_FLAG_NOT_IN_MELEE_RANGE  = 0x080,
};

enum RegenStatsFlags
{
    REGEN_FLAG_HEALTH               = 0x001,
    REGEN_FLAG_POWER                = 0x002,
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
            delete(*itr);
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
    TrainerSpell() : spell(0), spellCost(0), reqSkill(0), reqSkillValue(0), reqLevel(0), isProvidedReqLevel(false) {}

    TrainerSpell(uint32 _spell, uint32 _spellCost, uint32 _reqSkill, uint32 _reqSkillValue, uint32 _reqLevel, bool _isProvidedReqLevel)
        : spell(_spell), spellCost(_spellCost), reqSkill(_reqSkill), reqSkillValue(_reqSkillValue), reqLevel(_reqLevel), isProvidedReqLevel(_isProvidedReqLevel)
    {}

    uint32 spell;
    uint32 spellCost;
    uint32 reqSkill;
    uint32 reqSkillValue;
    uint32 reqLevel;
    bool isProvidedReqLevel;
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

typedef std::map<uint32, time_t> CreatureSpellCooldowns;

// max different by z coordinate for creature aggro reaction
#define CREATURE_Z_ATTACK_RANGE 3

#define MAX_VENDOR_ITEMS 255                                // Limitation in item count field size in SMSG_LIST_INVENTORY

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
        void SelectFinalPoint(Creature* cr);
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
    TEMPFACTION_TOGGLE_NON_ATTACKABLE   = 0x08,             // Remove UNIT_FLAG_NON_ATTACKABLE(0x02) when faction is changed (reapply when temp-faction is removed)
    TEMPFACTION_TOGGLE_OOC_NOT_ATTACK   = 0x10,             // Remove UNIT_FLAG_OOC_NOT_ATTACKABLE(0x100) when faction is changed (reapply when temp-faction is removed)
    TEMPFACTION_TOGGLE_PASSIVE          = 0x20,             // Remove UNIT_FLAG_PASSIVE(0x200) when faction is changed (reapply when temp-faction is removed)
    TEMPFACTION_TOGGLE_PACIFIED         = 0x40,             // Remove UNIT_FLAG_PACIFIED(0x20000) when faction is changed (reapply when temp-faction is removed)
    TEMPFACTION_TOGGLE_NOT_SELECTABLE   = 0x80,             // Remove UNIT_FLAG_NOT_SELECTABLE(0x2000000) when faction is changed (reapply when temp-faction is removed)
    TEMPFACTION_ALL,
};

class MANGOS_DLL_SPEC Creature : public Unit
{
        CreatureAI* i_AI;

    public:

        explicit Creature(CreatureSubtype subtype = CREATURE_SUBTYPE_GENERIC);
        virtual ~Creature();

        void AddToWorld() override;
        void RemoveFromWorld() override;

        bool Create(uint32 guidlow, CreatureCreatePos& cPos, CreatureInfo const* cinfo, Team team = TEAM_NONE, const CreatureData* data = nullptr, GameEventCreatureData const* eventData = nullptr);
        bool LoadCreatureAddon(bool reload);

        // SelectLevel set creature bases stats for given level or for default levels stored in db
        void SelectLevel(uint32 forcedLevel = USE_DEFAULT_DATABASE_LEVEL);
        void LoadEquipment(uint32 equip_entry, bool force = false);

        bool HasStaticDBSpawnData() const;                  // listed in `creature` table and have fixed in DB guid

        char const* GetSubName() const { return GetCreatureInfo()->SubName; }

        void Update(uint32 update_diff, uint32 time) override;  // overwrite Unit::Update

        virtual void RegenerateAll(uint32 update_diff);
        uint32 GetEquipmentId() const { return m_equipmentId; }

        CreatureSubtype GetSubtype() const { return m_subtype; }
        bool IsPet() const { return m_subtype == CREATURE_SUBTYPE_PET; }
        bool IsTotem() const { return m_subtype == CREATURE_SUBTYPE_TOTEM; }
        bool IsTemporarySummon() const { return m_subtype == CREATURE_SUBTYPE_TEMPORARY_SUMMON; }

        bool IsCorpse() const { return getDeathState() ==  CORPSE; }
        bool IsDespawned() const { return getDeathState() ==  DEAD; }
        void SetCorpseDelay(uint32 delay) { m_corpseDelay = delay; }
        bool IsRacialLeader() const { return GetCreatureInfo()->RacialLeader; }
        bool IsCivilian() const { return !!GetCreatureInfo()->civilian; }
        bool IsGuard() const { return !!(GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_GUARD); }

        bool CanWalk() const { return !!(GetCreatureInfo()->InhabitType & INHABIT_GROUND); }
        bool CanSwim() const { return !!(GetCreatureInfo()->InhabitType & INHABIT_WATER); }
        bool IsSwimming() const { return (m_movementInfo.HasMovementFlag((MovementFlags)(MOVEFLAG_SWIMMING))); }
        bool CanFly() const { return (GetCreatureInfo()->InhabitType & INHABIT_AIR) || m_movementInfo.HasMovementFlag((MovementFlags)(MOVEFLAG_LEVITATING | MOVEFLAG_CAN_FLY)); }
        bool IsFlying() const { return (m_movementInfo.HasMovementFlag((MovementFlags)(MOVEFLAG_FLYING | MOVEFLAG_LEVITATING))); }
        bool IsTrainerOf(Player* player, bool msg) const;
        bool CanInteractWithBattleMaster(Player* player, bool msg) const;
        bool CanTrainAndResetTalentsOf(Player* pPlayer) const;

        bool IsOutOfThreatArea(Unit* pVictim) const;
        void FillGuidsListFromThreatList(GuidVector& guids, uint32 maxamount = 0);

        bool IsImmuneToSpell(SpellEntry const* spellInfo, bool castOnSelf) override;
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

        bool IsInEvadeMode() const;

        bool AIM_Initialize();

        CreatureAI* AI() { return i_AI; }

        void SetWalk(bool enable, bool asDefault = true);
        void SetLevitate(bool enable) override;
        void SetSwim(bool enable) override;
        void SetCanFly(bool enable) override;
        void SetFeatherFall(bool enable) override;
        void SetHover(bool enable) override;
        void SetRoot(bool enable) override;
        void SetWaterWalk(bool enable) override;

        uint32 GetShieldBlockValue() const override         // dunno mob block value
        {
            return (getLevel() / 2 + uint32(GetStat(STAT_STRENGTH) / 20));
        }

        SpellSchoolMask GetMeleeDamageSchoolMask() const override { return m_meleeDamageSchoolMask; }
        void SetMeleeDamageSchool(SpellSchools school) { m_meleeDamageSchoolMask = GetSchoolMask(school); }

        void _AddCreatureSpellCooldown(uint32 spell_id, time_t end_time);
        void _AddCreatureCategoryCooldown(uint32 category, time_t apply_time);
        void AddCreatureSpellCooldown(uint32 spellid);
        bool HasSpellCooldown(uint32 spell_id) const;
        bool HasCategoryCooldown(uint32 spell_id) const;

        bool HasSpell(uint32 spellID) const override;

        bool UpdateEntry(uint32 entry, Team team = ALLIANCE, const CreatureData* data = nullptr, GameEventCreatureData const* eventData = nullptr, bool preserveHPAndPower = true);

        void ApplyGameEventSpells(GameEventCreatureData const* eventData, bool activated);
        bool UpdateStats(Stats stat) override;
        bool UpdateAllStats() override;
        void UpdateResistances(uint32 school) override;
        void UpdateArmor() override;
        void UpdateMaxHealth() override;
        void UpdateMaxPower(Powers power) override;
        void UpdateAttackPowerAndDamage(bool ranged = false) override;
        void UpdateDamagePhysical(WeaponAttackType attType) override;
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
        const char* GetNameForLocaleIdx(int32 locale_idx) const override;

        void SetDeathState(DeathState s) override;          // overwrite virtual Unit::SetDeathState

        bool LoadFromDB(uint32 guid, Map* map);
        virtual void SaveToDB();
        // overwrited in Pet
        virtual void SaveToDB(uint32 mapid);
        virtual void DeleteFromDB();                        // overwrited in Pet
        static void DeleteFromDB(uint32 lowguid, CreatureData const* data);

        void PrepareBodyLootState();
        CreatureLootStatus GetLootStatus() const { return m_lootStatus; }
        void SetLootStatus(CreatureLootStatus status);
        bool IsTappedBy(Player* plr) const;
        ObjectGuid GetLootRecipientGuid() const { return m_lootRecipientGuid; }
        uint32 GetLootGroupRecipientId() const { return m_lootGroupRecipientId; }
        Player* GetLootRecipient() const;                   // use group cases as prefered
        Group* GetGroupLootRecipient() const;
        bool HasLootRecipient() const { return m_lootGroupRecipientId || m_lootRecipientGuid; }
        bool IsGroupLootRecipient() const { return !!m_lootGroupRecipientId; }
        void SetLootRecipient(Unit* unit);
        Player* GetOriginalLootRecipient() const;           // ignore group changes/etc, not for looting

        SpellEntry const* ReachWithSpellAttack(Unit* pVictim);
        SpellEntry const* ReachWithSpellCure(Unit* pVictim);

        uint32 m_spells[CREATURE_MAX_SPELLS];
        CreatureSpellCooldowns m_CreatureSpellCooldowns;
        CreatureSpellCooldowns m_CreatureCategoryCooldowns;

        float GetAttackDistance(Unit const* pl) const;

        void SendAIReaction(AiReaction reactionType);

        void DoFleeToGetAssistance();
        void CallForHelp(float fRadius);
        void CallAssistance();
        void SetNoCallAssistance(bool val) { m_AlreadyCallAssistance = val; }
        void SetNoSearchAssistance(bool val) { m_AlreadySearchedAssistance = val; }
        bool HasSearchedAssistance() { return m_AlreadySearchedAssistance; }
        bool CanAssistTo(const Unit* u, const Unit* enemy, bool checkfaction = true) const;
        bool CanInitiateAttack();

        MovementGeneratorType GetDefaultMovementType() const { return m_defaultMovementType; }
        void SetDefaultMovementType(MovementGeneratorType mgt) { m_defaultMovementType = mgt; }

        // for use only in LoadHelper, Map::Add Map::CreatureCellRelocation
        Cell const& GetCurrentCell() const { return m_currentCell; }
        void SetCurrentCell(Cell const& cell) { m_currentCell = cell; }

        bool IsVisibleInGridForPlayer(Player* pl) const override;

        void RemoveCorpse(bool inPlace = false);
        bool IsDeadByDefault() const { return m_isDeadByDefault; };

        void ForcedDespawn(uint32 timeMSToDespawn = 0);

        time_t const& GetRespawnTime() const { return m_respawnTime; }
        time_t GetRespawnTimeEx() const;
        void SetRespawnTime(uint32 respawn) { m_respawnTime = respawn ? time(nullptr) + respawn : 0; }
        void Respawn();
        void SaveRespawnTime() override;

        uint32 GetRespawnDelay() const { return m_respawnDelay; }
        void SetRespawnDelay(uint32 delay) { m_respawnDelay = delay; }

        float GetRespawnRadius() const { return m_respawnradius; }
        void SetRespawnRadius(float dist) { m_respawnradius = dist; }

        // Functions spawn/remove creature with DB guid in all loaded map copies (if point grid loaded in map)
        static void AddToRemoveListInMaps(uint32 db_guid, CreatureData const* data);
        static void SpawnInMaps(uint32 db_guid, CreatureData const* data);

        void SendZoneUnderAttackMessage(Player* attacker);

        void SetInCombatWithZone();

        Unit* SelectAttackingTarget(AttackingTarget target, uint32 position, uint32 uiSpellEntry, uint32 selectFlags = 0) const;
        Unit* SelectAttackingTarget(AttackingTarget target, uint32 position, SpellEntry const* pSpellInfo = nullptr, uint32 selectFlags = 0) const;

        bool HasQuest(uint32 quest_id) const override;
        bool HasInvolvedQuest(uint32 quest_id)  const override;

        GridReference<Creature>& GetGridRef() { return m_gridRef; }
        bool IsRegeneratingHealth() const { return !!(GetCreatureInfo()->RegenerateStats & REGEN_FLAG_HEALTH); }
        bool IsRegeneratingPower() const { return !!(GetCreatureInfo()->RegenerateStats & REGEN_FLAG_POWER); }
        virtual uint8 GetPetAutoSpellSize() const { return CREATURE_MAX_SPELLS; }
        virtual uint32 GetPetAutoSpellOnPos(uint8 pos) const
        {
            if (pos >= CREATURE_MAX_SPELLS || m_charmInfo->GetCharmSpell(pos)->GetType() != ACT_ENABLED)
                return 0;
            else
                return m_charmInfo->GetCharmSpell(pos)->GetAction();
        }

        void SetCombatStartPosition(float x, float y, float z) { m_combatStartX = x; m_combatStartY = y; m_combatStartZ = z; }
        void GetCombatStartPosition(float& x, float& y, float& z) { x = m_combatStartX; y = m_combatStartY; z = m_combatStartZ; }

        void SetRespawnCoord(CreatureCreatePos const& pos) { m_respawnPos = pos.m_pos; }
        void SetRespawnCoord(float x, float y, float z, float ori) { m_respawnPos.x = x; m_respawnPos.y = y; m_respawnPos.z = z; m_respawnPos.o = ori; }
        void GetRespawnCoord(float& x, float& y, float& z, float* ori = nullptr, float* dist = nullptr) const;
        void ResetRespawnCoord();

        void SetDeadByDefault(bool death_state) { m_isDeadByDefault = death_state; }

        void SetFactionTemporary(uint32 factionId, uint32 tempFactionFlags = TEMPFACTION_ALL);
        void ClearTemporaryFaction();
        uint32 GetTemporaryFactionFlags() { return m_temporaryFactionFlags; }

        void SendAreaSpiritHealerQueryOpcode(Player* pl);

        void SetVirtualItem(VirtualItemSlot slot, uint32 item_id);
        void SetVirtualItemRaw(VirtualItemSlot slot, uint32 display_id, uint32 info0, uint32 info1);
    protected:
        bool MeetsSelectAttackingRequirement(Unit* pTarget, SpellEntry const* pSpellInfo, uint32 selectFlags) const;

        bool CreateFromProto(uint32 guidlow, CreatureInfo const* cinfo, Team team, const CreatureData* data = nullptr, GameEventCreatureData const* eventData = nullptr);
        bool InitEntry(uint32 entry, Team team = ALLIANCE, const CreatureData* data = nullptr, GameEventCreatureData const* eventData = nullptr);

        // vendor items
        VendorItemCounts m_vendorItemCounts;

        uint32 m_lootMoney;
        ObjectGuid m_lootRecipientGuid;                     // player who will have rights for looting if m_lootGroupRecipient==0 or group disbanded
        uint32 m_lootGroupRecipientId;                      // group who will have rights for looting if set and exist
        CreatureLootStatus m_lootStatus;                    // loot status (used to know when we could loot, pickpocket or skin)

        /// Timers
        uint32 m_corpseDecayTimer;                          // (msecs)timer for death or corpse disappearance
        time_t m_respawnTime;                               // (secs) time of next respawn
        uint32 m_respawnDelay;                              // (secs) delay between corpse disappearance and respawning
        uint32 m_corpseDelay;                               // (secs) delay between death and corpse disappearance
        uint32 m_aggroDelay;                                // (msecs)delay between respawn and aggro due to movement
        float m_respawnradius;

        CreatureSubtype m_subtype;                          // set in Creatures subclasses for fast it detect without dynamic_cast use
        void RegeneratePower();
        void RegenerateHealth();
        MovementGeneratorType m_defaultMovementType;
        Cell m_currentCell;                                 // store current cell where creature listed
        uint32 m_equipmentId;

        // below fields has potential for optimization
        bool m_AlreadyCallAssistance;
        bool m_AlreadySearchedAssistance;
        bool m_AI_locked;
        bool m_isDeadByDefault;
        uint32 m_temporaryFactionFlags;                     // used for real faction changes (not auras etc)

        SpellSchoolMask m_meleeDamageSchoolMask;
        uint32 m_originalEntry;

        float m_combatStartX;
        float m_combatStartY;
        float m_combatStartZ;

        Position m_respawnPos;

    private:
        GridReference<Creature> m_gridRef;
        CreatureInfo const* m_creatureInfo;
};

class ForcedDespawnDelayEvent : public BasicEvent
{
    public:
        ForcedDespawnDelayEvent(Creature& owner) : BasicEvent(), m_owner(owner) { }
        bool Execute(uint64 e_time, uint32 p_time) override;

    private:
        Creature& m_owner;
};

#endif
