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

#ifndef _PLAYERBOTAI_H
#define _PLAYERBOTAI_H

#include "Common.h"
#include "../../Entities/ObjectGuid.h"
#include "../../Entities/Unit.h"
#include "../../GameEvents/GameEventMgr.h"
#include "../../Quests/QuestDef.h"

class WorldPacket;
class WorldObject;
class Player;
class Unit;
class Object;
class Item;
class PlayerbotClassAI;
class PlayerbotMgr;

enum RacialTraits
{
    BERSERKING_ALL                 = 26297,
    BLOOD_FURY_ALL                 = 20572,
    ESCAPE_ARTIST_ALL              = 20589,
    PERCEPTION_ALL                 = 20600,
    SHADOWMELD_ALL                 = 20580,
    STONEFORM_ALL                  = 20594,
    WAR_STOMP_ALL                  = 20549,
    WILL_OF_THE_FORSAKEN_ALL       = 7744
};

enum ProfessionSpells
{
    ALCHEMY_1                      = 2259,
    BLACKSMITHING_1                = 2018,
    COOKING_1                      = 2550,
    ENCHANTING_1                   = 7411,
    ENGINEERING_1                  = 4036,
    FIRST_AID_1                    = 3273,
    FISHING_1                      = 7620,
    HERB_GATHERING_1               = 2366,
    MINING_1                       = 2575,
    SKINNING_1                     = 8613,
    TAILORING_1                    = 3908
};

enum NotableItems
{
    // Skeleton Keys
    SILVER_SKELETON_KEY            = 15869,
    GOLDEN_SKELETON_KEY            = 15870,
    TRUESILVER_SKELETON_KEY        = 15871,
    ARCANITE_SKELETON_KEY          = 15872,
    // Lock Charges
    SMALL_SEAFORIUM_CHARGE         = 4367,
    LARGE_SEAFORIUM_CHARGE         = 4398,
    POWERFUL_SEAFORIUM_CHARGE      = 18594
};

enum SharpeningStoneDisplayId
{
    ROUGH_SHARPENING_DISPLAYID          = 24673,
    COARSE_SHARPENING_DISPLAYID         = 24674,
    HEAVY_SHARPENING_DISPLAYID          = 24675,
    SOLID_SHARPENING_DISPLAYID          = 24676,
    DENSE_SHARPENING_DISPLAYID          = 24677,
    CONSECRATED_SHARPENING_DISPLAYID    = 24674,    // will not be used because bot can not know if it will face undead targets
    ELEMENTAL_SHARPENING_DISPLAYID      = 21072
};

enum WeightStoneDisplayId
{
    ROUGH_WEIGHTSTONE_DISPLAYID         = 24683,
    COARSE_WEIGHTSTONE_DISPLAYID        = 24684,
    HEAVY_WEIGHTSTONE_DISPLAYID         = 24685,
    SOLID_WEIGHTSTONE_DISPLAYID         = 24686,
    DENSE_WEIGHTSTONE_DISPLAYID         = 24687
};

enum ManaPotionsId
{
    MINOR_MANA_POTION                   = 15715,
    LESSER_MANA_POTION                  = 15716,
    MANA_POTION                         = 15717,
    GREATER_MANA_POTION                 = 15718,
    SUPERIOR_MANA_POTION                = 24151,
    MAJOR_MANA_POTION                   = 21672,
    MINOR_REJUVENATION_POTION           = 2345,
    MAJOR_REJUVENATION_POTION           = 18253
};

enum ManaRunesId
{
    DEMONIC_RUNE                        = 22952,
    DARK_RUNE                           = 32905
};

enum HealingItemDisplayId
{
    MAJOR_HEALING_POTION                = 24152,
    WHIPPER_ROOT_TUBER                  = 21974,
    NIGHT_DRAGON_BREATH                 = 21975,
    LIMITED_INVULNERABILITY_POTION      = 24213,
    GREATER_DREAMLESS_SLEEP_POTION      = 17403,
    SUPERIOR_HEALING_POTION             = 15714,
    CRYSTAL_RESTORE                     = 2516,
    DREAMLESS_SLEEP_POTION              = 17403,
    GREATER_HEALING_POTION              = 15713,
    HEALING_POTION                      = 15712,
    LESSER_HEALING_POTION               = 15711,
    DISCOLORED_HEALING_POTION           = 15736,
    MINOR_HEALING_POTION                = 15710
};

enum MainSpec
{
    MAGE_SPEC_FIRE              = 41,
    MAGE_SPEC_FROST             = 61,
    MAGE_SPEC_ARCANE            = 81,
    WARRIOR_SPEC_ARMS           = 161,
    WARRIOR_SPEC_PROTECTION     = 163,
    WARRIOR_SPEC_FURY           = 164,
    ROGUE_SPEC_COMBAT           = 181,
    ROGUE_SPEC_ASSASSINATION    = 182,
    ROGUE_SPEC_SUBTELTY         = 183,
    PRIEST_SPEC_DISCIPLINE      = 201,
    PRIEST_SPEC_HOLY            = 202,
    PRIEST_SPEC_SHADOW          = 203,
    SHAMAN_SPEC_ELEMENTAL       = 261,
    SHAMAN_SPEC_RESTORATION     = 262,
    SHAMAN_SPEC_ENHANCEMENT     = 263,
    DRUID_SPEC_FERAL            = 281,
    DRUID_SPEC_RESTORATION      = 282,
    DRUID_SPEC_BALANCE          = 283,
    WARLOCK_SPEC_DESTRUCTION    = 301,
    WARLOCK_SPEC_AFFLICTION     = 302,
    WARLOCK_SPEC_DEMONOLOGY     = 303,
    HUNTER_SPEC_BEASTMASTERY    = 361,
    HUNTER_SPEC_SURVIVAL        = 362,
    HUNTER_SPEC_MARKSMANSHIP    = 363,
    PALADIN_SPEC_RETRIBUTION    = 381,
    PALADIN_SPEC_HOLY           = 382,
    PALADIN_SPEC_PROTECTION     = 383
};

enum CombatManeuverReturns
{
    // TODO: RETURN_NO_ACTION_UNKNOWN is not part of ANY_OK or ANY_ERROR. It's also bad form and should be eliminated ASAP.
    RETURN_NO_ACTION_OK                 = 0x01, // No action taken during this combat maneuver, as intended (just wait, etc...)
    RETURN_NO_ACTION_UNKNOWN            = 0x02, // No action taken during this combat maneuver, unknown reason
    RETURN_NO_ACTION_ERROR              = 0x04, // No action taken due to error
    RETURN_NO_ACTION_INVALIDTARGET      = 0x08, // No action taken - invalid target
    RETURN_FINISHED_FIRST_MOVES         = 0x10, // Last action of first-combat-maneuver finished, continue onto next-combat-maneuver
    RETURN_CONTINUE                     = 0x20, // Continue first moves; normal return value for next-combat-maneuver
    RETURN_NO_ACTION_INSUFFICIENT_POWER = 0x40, // No action taken due to insufficient power (rage, focus, mana, runes)
    RETURN_ANY_OK                       = 0x31, // All the OK values bitwise OR'ed
    RETURN_ANY_ACTION                   = 0x30, // All returns that result in action (which should also be 'OK')
    RETURN_ANY_ERROR                    = 0x4C  // All the ERROR values bitwise OR'ed
};

class PlayerbotAI
{
    public:
        enum ScenarioType
        {
            SCENARIO_PVE,
            SCENARIO_PVE_ELITE, // group (5 members max) when an elite is near - most likely instance (can happen in open world)
            SCENARIO_PVE_RAID,
            SCENARIO_PVP_DUEL,
            SCENARIO_PVP_BG,    // You'll probably want to expand this to suit goal? (capture the flag, assault, domination, ...)
            SCENARIO_PVP_ARENA,
            SCENARIO_PVP_OPENWORLD
        };

        enum CombatStyle
        {
            COMBAT_MELEE                = 0x01,             // class melee attacker
            COMBAT_RANGED               = 0x02              // class is ranged attacker
        };

        // masters orders that should be obeyed by the AI during the updteAI routine
        // the master will auto set the target of the bot
        enum CombatOrderType
        {
            ORDERS_NONE                 = 0x0000,   // no special orders given
            ORDERS_TANK                 = 0x0001,   // bind attackers by gaining threat
            ORDERS_ASSIST               = 0x0002,   // assist someone (dps type)
            ORDERS_HEAL                 = 0x0004,   // concentrate on healing (no attacks, only self defense)
            ORDERS_NODISPEL             = 0x0008,   // Dont dispel anything
            ORDERS_PROTECT              = 0x0010,   // combinable state: check if protectee is attacked
            ORDERS_PASSIVE              = 0x0020,   // bots do nothing
            ORDERS_TEMP_WAIT_TANKAGGRO  = 0x0040,   // Wait on tank to build aggro - expect healing to continue, disable setting when tank loses focus
            ORDERS_TEMP_WAIT_OOC        = 0x0080,   // Wait but only while OOC - wait only - combat will resume healing, dps, tanking, ...
            ORDERS_RESIST_FIRE          = 0x0100,   // resist fire
            ORDERS_RESIST_NATURE        = 0x0200,   // resist nature
            ORDERS_RESIST_FROST         = 0x0400,   // resist frost
            ORDERS_RESIST_SHADOW        = 0x0800,   // resist shadow
            ORDERS_MAIN_TANK            = 0x1000,   // main attackers binder by gaining threat in raid situation
            ORDERS_MAIN_HEAL            = 0x2000,   // concentrate on healing the main tank (will ignore other targets as long as MT needs healing)
            ORDERS_NOT_MAIN_HEAL        = 0x4000,   // concentrate on healing except the main tank that will be ignored

            // Cumulative orders
            ORDERS_PRIMARY              = 0x7007,
            ORDERS_SECONDARY            = 0x0F78,
            ORDERS_RESIST               = 0x0F00,
            ORDERS_TEMP                 = 0x00C0,   // All orders NOT to be saved, turned off by bots (or logoff, reset, ...)
            ORDERS_RESET                = 0xFFFF
        };

        enum CombatTargetType
        {
            TARGET_NORMAL               = 0x00,
            TARGET_THREATEN             = 0x01
        };

        enum BotState
        {
            BOTSTATE_LOADING,           // loading state during world load
            BOTSTATE_NORMAL,            // normal AI routines are processed
            BOTSTATE_COMBAT,            // bot is in combat
            BOTSTATE_DEAD,              // we are dead and wait for becoming ghost
            BOTSTATE_DEADRELEASED,      // we released as ghost and wait to revive
            BOTSTATE_LOOTING,           // looting mode, used just after combat
            BOTSTATE_FLYING,             // bot is flying
            BOTSTATE_DELAYED            // bot delay action
        };

        enum CollectionFlags
        {
            COLLECT_FLAG_NOTHING    = 0x00,     // skip looting of anything
            COLLECT_FLAG_COMBAT     = 0x01,     // loot after combat
            COLLECT_FLAG_QUEST      = 0x02,     // quest and needed items
            COLLECT_FLAG_PROFESSION = 0x04,     // items related to skills
            COLLECT_FLAG_LOOT       = 0x08,     // all loot on corpses
            COLLECT_FLAG_SKIN       = 0x10,     // skin creatures if available
            COLLECT_FLAG_NEAROBJECT = 0x20      // collect specified nearby object
        };

        enum MovementOrderType
        {
            MOVEMENT_NONE               = 0x00,
            MOVEMENT_FOLLOW             = 0x01,
            MOVEMENT_STAY               = 0x02
        };

        enum TaskFlags
        {
            NONE                        = 0x00,  // do nothing
            SELL                        = 0x01,  // sell items
            REPAIR                      = 0x02,  // repair items
            ADD                         = 0x03,  // add auction
            REMOVE                      = 0x04,  // remove auction
            RESET                       = 0x05,  // reset all talents
            WITHDRAW                    = 0x06,  // withdraw item from bank
            DEPOSIT                     = 0x07,  // deposit item in bank
            LIST                        = 0x08,  // list quests
            END                         = 0x09,  // turn in quests
            TAKE                        = 0x0A   // take quest
        };

        enum AnnounceFlags
        {
            NOTHING                     = 0x00,
            INVENTORY_FULL              = 0x01,
            CANT_AFFORD                 = 0x02
        };

        typedef std::pair<enum TaskFlags, uint32> taskPair;
        typedef std::list<taskPair> BotTaskList;
        typedef std::list<enum NPCFlags> BotNPCList;
        typedef std::map<uint32, uint32> BotNeedItem;
        typedef std::pair<uint32, uint32> talentPair;
        typedef std::list<ObjectGuid> BotObjectList;
        typedef std::list<uint32> BotEntryList;
        typedef std::vector<uint32> BotTaxiNode;
        typedef std::set<ObjectGuid> BotObjectSet;

        // attacker query used in PlayerbotAI::FindAttacker()
        enum ATTACKERINFOTYPE
        {
            AIT_NONE                    = 0x00,
            AIT_LOWESTTHREAT            = 0x01,
            AIT_HIGHESTTHREAT           = 0x02,
            AIT_VICTIMSELF              = 0x04,
            AIT_VICTIMNOTSELF           = 0x08      // could/should use victim param in FindAttackers
        };
        struct AttackerInfo
        {
            Unit*    attacker;            // reference to the attacker
            Unit*    victim;              // combatant's current victim
            float threat;                 // own threat on this combatant
            float threat2;                // highest threat not caused by bot
            uint32 count;                 // number of units attacking
            uint32 source;                // 1=bot, 2=master, 3=group
        };
        typedef std::map<ObjectGuid, AttackerInfo> AttackerInfoList;
        typedef std::map<uint32, float> SpellRanges;

        enum HELPERLINKABLES
        {
            HL_NONE,
            HL_PROFESSION,
            HL_ITEM,
            HL_TALENT,
            HL_SKILL,
            HL_OPTION,
            HL_PETAGGRO,
            HL_QUEST,
            HL_GAMEOBJECT,
            HL_SPELL,
            HL_TARGET,
            HL_NAME,
            HL_AUCTION
        };

    public:
        PlayerbotAI(PlayerbotMgr& mgr, Player* const bot, bool debugWhisper);
        virtual ~PlayerbotAI();

        // This is called from Unit.cpp and is called every second (I think)
        void UpdateAI(const uint32 p_time);

        // This is called from ChatHandler.cpp when there is an incoming message to the bot
        // from a whisper or from the party channel
        void HandleCommand(const std::string& text, Player& fromPlayer);

        // This is called by WorldSession.cpp
        // It provides a view of packets normally sent to the client.
        // Since there is no client at the other end, the packets are dropped of course.
        // For a list of opcodes that can be caught see Opcodes.cpp (SMSG_* opcodes only)
        void HandleBotOutgoingPacket(const WorldPacket& packet);

        // Returns what kind of situation we are in so the ai can react accordingly
        ScenarioType GetScenarioType() { return m_ScenarioType; }
        CombatStyle GetCombatStyle() { return m_combatStyle; }
        void SetCombatStyle(CombatStyle cs) { m_combatStyle = cs; }

        PlayerbotClassAI* GetClassAI() { return m_classAI; }
        void ReloadAI();

        // finds spell ID for matching substring args
        // in priority of full text match, spells not taking reagents, and highest rank
        uint32 getSpellId(const char* args, bool master = false) const;
        uint32 getPetSpellId(const char* args) const;
        // Initialize spell using rank 1 spell id
        uint32 initSpell(uint32 spellId);
        uint32 initPetSpell(uint32 spellIconId);

        // extract quest ids from links
        void extractQuestIds(const std::string& text, std::list<uint32>& questIds) const;

        // extract auction ids from links
        void extractAuctionIds(const std::string& text, std::list<uint32>& auctionIds) const;

        // extracts talent ids to list
        void extractTalentIds(const std::string& text, std::list<talentPair>& talentIds) const;

        // extracts item ids from links
        void extractItemIds(const std::string& text, std::list<uint32>& itemIds) const;

        // extract spellid from links
        void extractSpellId(const std::string& text, uint32& spellId) const;

        // extract spellids from links to list
        void extractSpellIdList(const std::string& text, BotEntryList& m_spellsToLearn) const;

        // extracts currency from a string as #g#s#c and returns the total in copper
        uint32 extractMoney(const std::string& text) const;

        // extracts gameobject info from link
        void extractGOinfo(const std::string& text, BotObjectList& m_lootTargets) const;

        // finds items in bots equipment and adds them to foundItemList, removes found items from itemIdSearchList
        void findItemsInEquip(std::list<uint32>& itemIdSearchList, std::list<Item*>& foundItemList) const;
        // finds items in bots inventory and adds them to foundItemList, removes found items from itemIdSearchList
        void findItemsInInv(std::list<uint32>& itemIdSearchList, std::list<Item*>& foundItemList) const;
        // finds nearby game objects that are specified in m_collectObjects then adds them to the m_lootTargets list
        void findNearbyGO();
        // finds nearby creatures, whose UNIT_NPC_FLAGS match the flags specified in item list m_itemIds
        void findNearbyCreature();
        // finds nearby corpse that is lootable
        void findNearbyCorpse();
        bool IsElite(Unit* target, bool isWorldBoss = false) const;
        // Used by bots to check if their target is neutralized (polymorph, shackle or the like). Useful to avoid breaking crowd control
        static bool IsNeutralized(Unit* target);
        // Make the bots face their target
        void FaceTarget(Unit* target);
        // Used by bot to check if target is immune to a specific damage school before using an ability
        static bool IsImmuneToSchool(Unit* pTarget, SpellSchoolMask SchoolMask);

        void MakeSpellLink(const SpellEntry* sInfo, std::ostringstream& out);
        void MakeWeaponSkillLink(const SpellEntry* sInfo, std::ostringstream& out, uint32 skillid);

        // currently bots only obey commands from the master
        bool canObeyCommandFrom(const Player& player) const;

        // get current casting spell (will return NULL if no spell!)
        Spell* GetCurrentSpell() const;
        uint32 GetCurrentSpellId() { return m_CurrentlyCastingSpellId; }

        bool HasAura(uint32 spellId, const Unit& player) const;
        bool HasAura(const char* spellName, const Unit& player) const;
        bool HasAura(const char* spellName) const;

        bool CanReceiveSpecificSpell(uint8 spec, Unit* target) const;

        bool PickPocket(Unit* pTarget);
        bool HasTool(uint32 TC);        // TODO implement this for opening lock
        bool HasSpellReagents(uint32 spellId);

        uint8 GetHealthPercent(const Unit& target) const;
        uint8 GetHealthPercent() const;
        uint8 GetManaPercent(const Unit& target) const;
        uint8 GetManaPercent() const;
        uint8 GetRageAmount(const Unit& target) const;
        uint8 GetRageAmount() const;
        uint8 GetEnergyAmount(const Unit& target) const;
        uint8 GetEnergyAmount() const;

        Item* FindFood() const;
        Item* FindDrink() const;
        Item* FindBandage() const;
        Item* FindMount(uint32 matchingRidingSkill) const;
        Item* FindItem(uint32 ItemId);
        Item* FindItemInBank(uint32 ItemId);
        Item* FindKeyForLockValue(uint32 reqSkillValue);
        Item* FindBombForLockValue(uint32 reqSkillValue);
        Item* FindConsumable(uint32 displayId) const;
        Item* FindStoneFor(Item* weapon) const;
        Item* FindManaRegenItem() const;
        bool  FindAmmo() const;
        bool CanStore();

        // ******* Actions ****************************************
        // Your handlers can call these actions to make the bot do things.
        void TellMaster(const std::string& text) const;
        void TellMaster(const char* fmt, ...) const;
        void SendWhisper(const std::string& text, Player& player) const;
        SpellCastResult CastSpell(const char* args);
        SpellCastResult CastSpell(uint32 spellId);
        SpellCastResult CastSpell(uint32 spellId, Unit& target);
        SpellCastResult CheckBotCast(const SpellEntry* sInfo);
        SpellCastResult CastPetSpell(uint32 spellId, Unit* target = nullptr);
        SpellCastResult Buff(uint32 spellId, Unit* target, void (*beforeCast)(Player*) = nullptr);
        SpellCastResult SelfBuff(uint32 spellId);
        bool In_Range(Unit* Target, uint32 spellId);
        bool In_Reach(Unit* Target, uint32 spellId);
        bool CanReachWithSpellAttack(Unit* target);

        void UseItem(Item* item, uint16 targetFlag, ObjectGuid targetGUID);
        void UseItem(Item* item, uint8 targetInventorySlot);
        void UseItem(Item* item, Unit* target);
        void UseItem(Item* item);

        void TryEmergency(Unit* pAttacker);

        void PlaySound(uint32 soundid);
        void Announce(AnnounceFlags msg);

        void EquipItem(Item* src_Item);
        //void Stay();
        //bool Follow(Player& player);
        void SendNotEquipList(Player& player);

        uint8  m_DelayAttack;
        time_t m_DelayAttackInit;
        Unit* gPrimtarget;
        Unit* gSectarget;
        uint32 gQuestFetch;
        void BotDataRestore();
        void CombatOrderRestore();
        void InterruptCurrentCastingSpell();
        void Attack(Unit* forcedTarget = nullptr);
        void GetCombatTarget(Unit* forcedTarget = 0);
        void GetDuelTarget(Unit* forcedTarget);
        Unit* GetCurrentTarget() { return m_targetCombat; };
        void DoNextCombatManeuver();
        void DoCombatMovement();
        void SetIgnoreUpdateTime(uint8 t = 0) { m_ignoreAIUpdatesUntilTime = time(nullptr) + t; };
        time_t CurrentTime() { return time(nullptr); };

        Player* GetPlayerBot() const { return m_bot; }
        Player* GetPlayer() const { return m_bot; }
        Player* GetMaster() const;

        BotState GetState() { return m_botState; };
        void SetState(BotState state);
        void SetQuestNeedItems();
        void SetQuestNeedCreatures();
        void SendQuestNeedList();
        bool IsInQuestItemList(uint32 itemid) { return m_needItemList.find(itemid) != m_needItemList.end(); };
        bool IsInQuestCreatureList(uint32 id) { return m_needCreatureOrGOList.find(id) != m_needCreatureOrGOList.end(); };
        bool IsItemUseful(uint32 itemid);
        void SendOrders(Player& player);
        bool DoTeleport(WorldObject& obj);
        void DoLoot();
        void DoFlight();
        void GetTaxi(ObjectGuid guid, BotTaxiNode& nodes);

        bool HasCollectFlag(uint8 flag) { return (m_collectionFlags & flag) ? true : false; }
        void SetCollectFlag(uint8 flag)
        {
            if (HasCollectFlag(flag)) m_collectionFlags &= ~flag;
            else m_collectionFlags |= flag;
        }

        uint32 EstRepairAll();
        uint32 EstRepair(uint16 pos);

        void AcceptQuest(Quest const* qInfo, Player* pGiver);
        void TurnInQuests(WorldObject* questgiver);
        void ListQuests(WorldObject* questgiver);
        bool AddQuest(const uint32 entry, WorldObject* questgiver);

        bool IsInCombat();
        bool IsRegenerating();
        bool IsGroupReady();
        Player* GetGroupTank(); // TODO: didn't want to pollute non-playerbot code but this should really go in group.cpp
        void SetGroupCombatOrder(CombatOrderType co);
        void ClearGroupCombatOrder(CombatOrderType co);
        void SetGroupIgnoreUpdateTime(uint8 t);
        bool GroupHoTOnTank();
        bool CanPull(Player& fromPlayer);
        bool CastPull();
        bool GroupTankHoldsAggro();
        bool CastNeutralize();
        void UpdateAttackerInfo();
        Unit* FindAttacker(ATTACKERINFOTYPE ait = AIT_NONE, Unit* victim = 0);
        uint32 GetAttackerCount() { return m_attackerInfo.size(); };
        void SetCombatOrderByStr(std::string str, Unit* target = 0);
        void SetCombatOrder(CombatOrderType co, Unit* target = 0);
        void ClearCombatOrder(CombatOrderType co);
        CombatOrderType GetCombatOrder() { return this->m_combatOrder; }
        bool IsMainTank() { return (m_combatOrder & ORDERS_MAIN_TANK) ? true : false; }
        bool IsTank() { return (m_combatOrder & ORDERS_TANK) || IsMainTank() ? true : false; }
        bool IsMainHealer() { return (m_combatOrder & ORDERS_MAIN_HEAL) ? true : false; }
        bool IsHealer() { return (m_combatOrder & (ORDERS_HEAL | ORDERS_NOT_MAIN_HEAL)) || IsMainHealer() ? true : false; }
        bool HasDispelOrder() { return !(m_combatOrder & ORDERS_NODISPEL); }
        bool IsDPS() { return (m_combatOrder & ORDERS_ASSIST) ? true : false; }
        bool Impulse() { srand(time(nullptr)); return (((rand() % 100) > 50) ? true : false); }
        void SetMovementOrder(MovementOrderType mo, Unit* followTarget = 0);
        MovementOrderType GetMovementOrder() { return this->m_movementOrder; }
        void MovementReset();
        void MovementClear();

        void SetInFront(const Unit* obj);

        void ItemLocalization(std::string& itemName, const uint32 itemID) const;
        void QuestLocalization(std::string& questTitle, const uint32 questID) const;
        void CreatureLocalization(std::string& creatureName, const uint32 entry) const;
        void GameObjectLocalization(std::string& gameobjectName, const uint32 entry) const;

        uint8 GetFreeBagSpace() const;
        void SellGarbage(bool listNonTrash = true, bool bDetailTrashSold = false, bool verbose = true);
        void Sell(const uint32 itemid);
        void AddAuction(const uint32 itemid, Creature* aCreature);
        void ListAuctions();
        bool RemoveAuction(const uint32 auctionid);
        void Repair(const uint32 itemid, Creature* rCreature);
        bool Talent(Creature* tCreature);
        void InspectUpdate();
        bool Withdraw(const uint32 itemid);
        bool Deposit(const uint32 itemid);
        void BankBalance();
        std::string Cash(uint32 copper);

    private:
        bool ExtractCommand(const std::string sLookingFor, std::string& text, bool bUseShort = false);
        // outsource commands for code clarity
        void _HandleCommandReset(std::string& text, Player& fromPlayer);
        void _HandleCommandReport(std::string& text, Player& fromPlayer);
        void _HandleCommandOrders(std::string& text, Player& fromPlayer);
        void _HandleCommandFollow(std::string& text, Player& fromPlayer);
        void _HandleCommandStay(std::string& text, Player& fromPlayer);
        void _HandleCommandAttack(std::string& text, Player& fromPlayer);
        void _HandleCommandPull(std::string& text, Player& fromPlayer);
        void _HandleCommandNeutralize(std::string& text, Player& fromPlayer);
        void _HandleCommandCast(std::string& text, Player& fromPlayer);
        void _HandleCommandSell(std::string& text, Player& fromPlayer);
        void _HandleCommandRepair(std::string& text, Player& fromPlayer);
        void _HandleCommandAuction(std::string& text, Player& fromPlayer);
        void _HandleCommandBank(std::string& text, Player& fromPlayer);
        void _HandleCommandUse(std::string& text, Player& fromPlayer);
        void _HandleCommandEquip(std::string& text, Player& fromPlayer);
        void _HandleCommandFind(std::string& text, Player& fromPlayer);
        void _HandleCommandGet(std::string& text, Player& fromPlayer);
        void _HandleCommandCollect(std::string& text, Player& fromPlayer);
        void _HandleCommandQuest(std::string& text, Player& fromPlayer);
        void _HandleCommandPet(std::string& text, Player& fromPlayer);
        void _HandleCommandSpells(std::string& text, Player& fromPlayer);
        void _HandleCommandSurvey(std::string& text, Player& fromPlayer);
        void _HandleCommandSkill(std::string& text, Player& fromPlayer);
        void _HandleCommandStats(std::string& text, Player& fromPlayer);
        void _HandleCommandHelp(std::string& text, Player& fromPlayer);
        void _HandleCommandHelp(const char* szText, Player& fromPlayer) { std::string text = szText; _HandleCommandHelp(text, fromPlayer); }
        std::string _HandleCommandHelpHelper(std::string sCommand, std::string sExplain, HELPERLINKABLES reqLink = HL_NONE, bool bReqLinkMultiples = false, bool bCommandShort = false);

        // ****** Closed Actions ********************************
        // These actions may only be called at special times.
        // Trade methods are only applicable when the trade window is open
        // and are only called from within HandleCommand.
        bool TradeItem(const Item& item, int8 slot = -1);
        bool TradeCopper(uint32 copper);

        // Helper routines not needed by class AIs.
        void UpdateAttackersForTarget(Unit* victim);

        void _doSellItem(Item* const item, std::ostringstream& report, std::ostringstream& canSell, uint32& TotalCost, uint32& TotalSold);
        void MakeItemLink(const Item* item, std::ostringstream& out, bool IncludeQuantity = true);
        void MakeItemLink(const ItemPrototype* item, std::ostringstream& out);

        // it is safe to keep these back reference pointers because m_bot
        // owns the "this" object and m_master owns m_bot. The owner always cleans up.
        PlayerbotMgr& m_mgr;
        Player* const m_bot;
        PlayerbotClassAI* m_classAI;

        // ignores AI updates until time specified
        // no need to waste CPU cycles during casting etc
        time_t m_ignoreAIUpdatesUntilTime;

        CombatStyle m_combatStyle;
        CombatOrderType m_combatOrder;
        MovementOrderType m_movementOrder;

        ScenarioType m_ScenarioType;

        // defines the state of behaviour of the bot
        BotState m_botState;

        // list of items, creatures or gameobjects needed to fullfill quests
        BotNeedItem m_needItemList;
        BotNeedItem m_needCreatureOrGOList;

        // list of creatures we recently attacked and want to loot
        BotNPCList m_findNPC;               // list of NPCs
        BotTaskList m_tasks;                // list of tasks
        BotObjectList m_lootTargets;        // list of targets
        BotEntryList m_spellsToLearn;       // list of spells
        ObjectGuid m_lootCurrent;           // current remains of interest
        ObjectGuid m_lootPrev;              // previous loot
        BotEntryList m_collectObjects;      // object entries searched for in findNearbyGO
        BotTaxiNode m_taxiNodes;            // flight node chain;

        uint8 m_collectionFlags;            // what the bot should look for to loot
        uint32 m_collectDist;               // distance to collect objects
        bool m_inventory_full;

        uint32 m_CurrentlyCastingSpellId;
        //bool m_IsFollowingMaster;

        // if master commands bot to do something, store here until updateAI
        // can do it
        uint32 m_spellIdCommand;
        ObjectGuid m_targetGuidCommand;
        ObjectGuid m_taxiMaster;

        BotObjectSet m_ignorePlayersChat;  // list of players that the bot will not respond to

        AttackerInfoList m_attackerInfo;

        // Force bot to pick a neutralised unit as combat target when told so
        bool m_ignoreNeutralizeEffect;
        bool m_targetChanged;
        CombatTargetType m_targetType;

        Unit* m_targetCombat;       // current combat target
        Unit* m_targetAssist;       // get new target by checking attacker list of assisted player
        Unit* m_targetProtect;      // check

        Unit* m_followTarget;       // whom to follow in non combat situation?

        uint32 FISHING,
               HERB_GATHERING,
               MINING,
               SKINNING;

        SpellRanges m_spellRangeMap;

        float m_destX, m_destY, m_destZ; // latest coordinates for chase and point movement types
        bool m_debugWhisper = false;
};

#endif
