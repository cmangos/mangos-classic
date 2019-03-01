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

#ifndef __SPELL_H
#define __SPELL_H

#include "Common.h"
#include "Maps/GridDefines.h"
#include "Globals/SharedDefines.h"
#include "Server/DBCEnums.h"
#include "Entities/ObjectGuid.h"
#include "Entities/Unit.h"
#include "Entities/Player.h"
#include "Server/SQLStorages.h"
#include "Spells/SpellEffectDefines.h"
#include "Spells/SpellMgr.h"

class WorldSession;
class WorldPacket;
class DynamicObj;
class Item;
class GameObject;
class Group;
class Aura;
struct SpellTargetEntry;

enum SpellCastFlags
{
    CAST_FLAG_NONE              = 0x00000000,
    CAST_FLAG_HIDDEN_COMBATLOG  = 0x00000001,               // hide in combat log?
    CAST_FLAG_UNKNOWN2          = 0x00000002,
    CAST_FLAG_UNKNOWN3          = 0x00000004,
    CAST_FLAG_UNKNOWN4          = 0x00000008,
    CAST_FLAG_PERSISTENT_AA     = 0x00000010,               // Spell has Persistent AA effect
    CAST_FLAG_AMMO              = 0x00000020,               // Projectiles visual
    CAST_FLAG_UNKNOWN7          = 0x00000040,               // !0x41 mask used to call CGTradeSkillInfo::DoRecast
    CAST_FLAG_UNKNOWN8          = 0x00000080,
    CAST_FLAG_UNKNOWN9          = 0x00000100,
};

enum SpellNotifyPushType
{
    PUSH_CONE,
    PUSH_SELF_CENTER,
    PUSH_SRC_CENTER,
    PUSH_DEST_CENTER,
    PUSH_TARGET_CENTER
};

enum CheckException
{
    EXCEPTION_NONE,
    EXCEPTION_MAGNET,
};

bool IsQuestTameSpell(uint32 spellId);

namespace MaNGOS
{
    struct SpellNotifierPlayer;
    struct SpellNotifierCreatureAndPlayer;
}

class SpellCastTargets;

struct SpellCastTargetsReader
{
    explicit SpellCastTargetsReader(SpellCastTargets& _targets, Unit* _caster) : targets(_targets), caster(_caster) {}

    SpellCastTargets& targets;
    Unit* caster;
};

class SpellCastTargets
{
    public:
        SpellCastTargets();
        ~SpellCastTargets();

        void read(ByteBuffer& data, Unit* caster);
        void write(ByteBuffer& data) const;

        SpellCastTargetsReader ReadForCaster(Unit* caster) { return SpellCastTargetsReader(*this, caster); }

        SpellCastTargets& operator=(const SpellCastTargets& target)
        {
            m_unitTarget = target.m_unitTarget;
            m_itemTarget = target.m_itemTarget;
            m_GOTarget   = target.m_GOTarget;

            m_unitTargetGUID    = target.m_unitTargetGUID;
            m_GOTargetGUID      = target.m_GOTargetGUID;
            m_CorpseTargetGUID  = target.m_CorpseTargetGUID;
            m_itemTargetGUID    = target.m_itemTargetGUID;

            m_itemTargetEntry  = target.m_itemTargetEntry;

            m_srcX = target.m_srcX;
            m_srcY = target.m_srcY;
            m_srcZ = target.m_srcZ;

            m_destX = target.m_destX;
            m_destY = target.m_destY;
            m_destZ = target.m_destZ;

            m_strTarget = target.m_strTarget;

            m_targetMask = target.m_targetMask;

            return *this;
        }

        void setUnitTarget(Unit* target);
        ObjectGuid getUnitTargetGuid() const { return m_unitTargetGUID; }
        Unit* getUnitTarget() const { return m_unitTarget; }

        void setDestination(float x, float y, float z);
        void setSource(float x, float y, float z);
        void getDestination(float& x, float& y, float& z) const { x = m_destX; y = m_destY; z = m_destZ; }
        void getDestination(WorldLocation& loc) { loc.coord_x = m_destX; loc.coord_y = m_destY; loc.coord_z = m_destZ; }
        void getSource(float& x, float& y, float& z) const { x = m_srcX; y = m_srcY, z = m_srcZ; }

        void setGOTarget(GameObject* target);
        ObjectGuid getGOTargetGuid() const { return m_GOTargetGUID; }
        GameObject* getGOTarget() const { return m_GOTarget; }

        void setCorpseTarget(Corpse* corpse);
        ObjectGuid getCorpseTargetGuid() const { return m_CorpseTargetGUID; }
        Corpse* getCorpseTarget() const { return m_CorpseTarget; }

        void setItemTarget(Item* item);
        ObjectGuid getItemTargetGuid() const { return m_itemTargetGUID; }
        Item* getItemTarget() const { return m_itemTarget; }
        uint32 getItemTargetEntry() const { return m_itemTargetEntry; }

        void setTradeItemTarget(Player* caster);

        void updateTradeSlotItem()
        {
            if (m_itemTarget && (m_targetMask & TARGET_FLAG_TRADE_ITEM))
            {
                m_itemTargetGUID = m_itemTarget->GetObjectGuid();
                m_itemTargetEntry = m_itemTarget->GetEntry();
            }
        }

        bool IsEmpty() const { return !m_GOTargetGUID && !m_unitTargetGUID && !m_itemTarget && !m_CorpseTargetGUID; }

        void Update(Unit* caster);

        float m_srcX, m_srcY, m_srcZ;
        float m_destX, m_destY, m_destZ;
        std::string m_strTarget;

        uint16 m_targetMask;

    private:
        // objects (can be used at spell creating and after Update at casting
        Unit* m_unitTarget;
        GameObject* m_GOTarget;
        Item* m_itemTarget;
        Corpse* m_CorpseTarget;

        // object GUID/etc, can be used always
        ObjectGuid m_unitTargetGUID;
        ObjectGuid m_GOTargetGUID;
        ObjectGuid m_CorpseTargetGUID;
        ObjectGuid m_itemTargetGUID;
        uint32 m_itemTargetEntry;
};

inline ByteBuffer& operator<< (ByteBuffer& buf, SpellCastTargets const& targets)
{
    targets.write(buf);
    return buf;
}

inline ByteBuffer& operator>> (ByteBuffer& buf, SpellCastTargetsReader const& targets)
{
    targets.targets.read(buf, targets.caster);
    return buf;
}

enum SpellState
{
    SPELL_STATE_CREATED    = 0,                             // just created
    SPELL_STATE_TARGETING  = 1,                             // doing initial check
    SPELL_STATE_CASTING    = 2,                             // cast time delay period, non channeled spell
    SPELL_STATE_DELAYED    = 3,                             // spell is delayed (cast time pushed back) TODO: need to be implemented properly
    SPELL_STATE_TRAVELING  = 4,                             // spell casted but need time to hit target(s)
    SPELL_STATE_LANDING    = 5,                             // processing the effects
    SPELL_STATE_CHANNELING = 6,                             // channeled time period spell casting state
    SPELL_STATE_FINISHED   = 7,                             // cast finished to success or fail
};

enum SpellTargets
{
    SPELL_TARGETS_ASSISTABLE,
    SPELL_TARGETS_AOE_ATTACKABLE,
    SPELL_TARGETS_ALL
};

typedef std::multimap<uint64, uint64> SpellTargetTimeMap;

// SpellLog class to manage spells logs that have to be sent to clients
class SpellLog
{
    public:
        SpellLog(Spell* spell) :
            m_spell(spell), m_spellLogDataEffectsCounterPos(0), m_spellLogDataEffectsCounter(0),
            m_spellLogDataTargetsCounterPos(0), m_spellLogDataTargetsCounter(0), m_currentEffect(MAX_SPELL_EFFECTS) {}
        SpellLog() = delete;
        SpellLog(const SpellLog&) = delete;

        void Initialize();

        // Variadic template to add log data (warnings, devs should respect the correct packet structure)
        template<typename... Args>
        void AddLog(uint32 spellEffect, Args... args)
        {
            SetCurrentEffect(spellEffect);
            AddLogData(args...);
            ++m_spellLogDataEffectsCounter;
        }

        // Send collected logs
        void SendToSet();

    private:
        // Finalize previous log if need by setting total targets amount
        void FinalizePrevious();

        // Handle multi targets cases by adjusting targets counter if need
        void SetCurrentEffect(uint32 effect);

        // Variadic template to method to handle multi arguments passed with different types
        template<typename T>
        void AddLogData(T const& data) { m_spellLogData << data; }

        template<typename T, typename... Args>
        void AddLogData(T const& data, Args const& ... args)
        {
            AddLogData(data);
            AddLogData(args...);
        }

        Spell* m_spell;
        WorldPacket m_spellLogData;
        size_t m_spellLogDataEffectsCounterPos;
        uint32 m_spellLogDataEffectsCounter;
        size_t m_spellLogDataTargetsCounterPos;
        uint32 m_spellLogDataTargetsCounter;
        uint32 m_currentEffect;
};

class Spell
{
        friend struct MaNGOS::SpellNotifierPlayer;
        friend struct MaNGOS::SpellNotifierCreatureAndPlayer;
        friend void Unit::SetCurrentCastedSpell(Spell* pSpell);
    public:

        void EffectEmpty(SpellEffectIndex eff_idx);
        void EffectNULL(SpellEffectIndex eff_idx);
        void EffectUnused(SpellEffectIndex eff_idx);
        void EffectDistract(SpellEffectIndex eff_idx);
        void EffectPull(SpellEffectIndex eff_idx);
        void EffectSchoolDMG(SpellEffectIndex eff_idx);
        void EffectEnvironmentalDMG(SpellEffectIndex eff_idx);
        void EffectInstaKill(SpellEffectIndex eff_idx);
        void EffectDummy(SpellEffectIndex eff_idx);
        void EffectTeleportUnits(SpellEffectIndex eff_idx);
        void EffectApplyAura(SpellEffectIndex eff_idx);
        void EffectSendEvent(SpellEffectIndex eff_idx);
        void EffectPowerBurn(SpellEffectIndex eff_idx);
        void EffectPowerDrain(SpellEffectIndex eff_idx);
        void EffectHeal(SpellEffectIndex eff_idx);
        void EffectBind(SpellEffectIndex eff_idx);
        void EffectHealthLeech(SpellEffectIndex eff_idx);
        void EffectQuestComplete(SpellEffectIndex eff_idx);
        void EffectCreateItem(SpellEffectIndex eff_idx);
        void EffectPersistentAA(SpellEffectIndex eff_idx);
        void EffectEnergize(SpellEffectIndex eff_idx);
        void EffectOpenLock(SpellEffectIndex eff_idx);
        void EffectSummonChangeItem(SpellEffectIndex eff_idx);
        void EffectProficiency(SpellEffectIndex eff_idx);
        void EffectApplyAreaAura(SpellEffectIndex eff_idx);
        void EffectSummon(SpellEffectIndex eff_idx);
        void EffectLearnSpell(SpellEffectIndex eff_idx);
        void EffectDispel(SpellEffectIndex eff_idx);
        void EffectDualWield(SpellEffectIndex eff_idx);
        void EffectPickPocket(SpellEffectIndex eff_idx);
        void EffectAddFarsight(SpellEffectIndex eff_idx);
        void EffectSummonPossessed(SpellEffectIndex eff_idx);
        void EffectSummonWild(SpellEffectIndex eff_idx);
        void EffectSummonGuardian(SpellEffectIndex eff_idx);
        void EffectHealMechanical(SpellEffectIndex eff_idx);
        void EffectTeleUnitsFaceCaster(SpellEffectIndex eff_idx);
        void EffectLearnSkill(SpellEffectIndex eff_idx);
        void EffectAddHonor(SpellEffectIndex eff_idx);
        void EffectSpawn(SpellEffectIndex eff_idx);
        void EffectTradeSkill(SpellEffectIndex eff_idx);
        void EffectEnchantItemPerm(SpellEffectIndex eff_idx);
        void EffectEnchantItemTmp(SpellEffectIndex eff_idx);
        void EffectTameCreature(SpellEffectIndex eff_idx);
        void EffectSummonPet(SpellEffectIndex eff_idx);
        void EffectLearnPetSpell(SpellEffectIndex eff_idx);
        void EffectWeaponDmg(SpellEffectIndex eff_idx);
        void EffectTriggerSpell(SpellEffectIndex eff_idx);
        void EffectTriggerMissileSpell(SpellEffectIndex eff_idx);
        void EffectThreat(SpellEffectIndex eff_idx);
        void EffectHealMaxHealth(SpellEffectIndex eff_idx);
        void EffectInterruptCast(SpellEffectIndex eff_idx);
        void EffectSummonObjectWild(SpellEffectIndex eff_idx);
        void EffectScriptEffect(SpellEffectIndex eff_idx);
        void EffectSanctuary(SpellEffectIndex eff_idx);
        void EffectAddComboPoints(SpellEffectIndex eff_idx);
        void EffectDuel(SpellEffectIndex eff_idx);
        void EffectStuck(SpellEffectIndex eff_idx);
        void EffectSummonPlayer(SpellEffectIndex eff_idx);
        void EffectActivateObject(SpellEffectIndex eff_idx);
        void EffectSummonTotem(SpellEffectIndex eff_idx);
        void EffectEnchantHeldItem(SpellEffectIndex eff_idx);
        void EffectSummonObject(SpellEffectIndex eff_idx);
        void EffectResurrect(SpellEffectIndex eff_idx);
        void EffectParry(SpellEffectIndex eff_idx);
        void EffectBlock(SpellEffectIndex eff_idx);
        void EffectLeapForward(SpellEffectIndex eff_idx);
        void EffectTransmitted(SpellEffectIndex eff_idx);
        void EffectDisEnchant(SpellEffectIndex eff_idx);
        void EffectInebriate(SpellEffectIndex eff_idx);
        void EffectFeedPet(SpellEffectIndex eff_idx);
        void EffectDismissPet(SpellEffectIndex eff_idx);
        void EffectReputation(SpellEffectIndex eff_idx);
        void EffectSelfResurrect(SpellEffectIndex eff_idx);
        void EffectSkinning(SpellEffectIndex eff_idx);
        void EffectCharge(SpellEffectIndex eff_idx);
        void EffectSendTaxi(SpellEffectIndex eff_idx);
        void EffectSummonCritter(SpellEffectIndex eff_idx);
        void EffectKnockBack(SpellEffectIndex eff_idx);
        void EffectPlayerPull(SpellEffectIndex eff_idx);
        void EffectDispelMechanic(SpellEffectIndex eff_idx);
        void EffectSummonDeadPet(SpellEffectIndex eff_idx);
        void EffectDestroyAllTotems(SpellEffectIndex eff_idx);
        void EffectDurabilityDamage(SpellEffectIndex eff_idx);
        void EffectSkill(SpellEffectIndex eff_idx);
        void EffectTaunt(SpellEffectIndex eff_idx);
        void EffectDurabilityDamagePCT(SpellEffectIndex eff_idx);
        void EffectModifyThreatPercent(SpellEffectIndex eff_idx);
        void EffectResurrectNew(SpellEffectIndex eff_idx);
        void EffectAddExtraAttacks(SpellEffectIndex eff_idx);
        void EffectSpiritHeal(SpellEffectIndex eff_idx);
        void EffectSkinPlayerCorpse(SpellEffectIndex eff_idx);
        void EffectSummonDemon(SpellEffectIndex eff_idx);

        Spell(Unit* caster, SpellEntry const* info, uint32 triggeredFlags, ObjectGuid originalCasterGUID = ObjectGuid(), SpellEntry const* triggeredBy = nullptr);
        ~Spell();

        SpellCastResult SpellStart(SpellCastTargets const* targets, Aura* triggeredByAura = nullptr);

        void cancel();

        void update(uint32 difftime);
        void cast(bool skipCheck = false);
        void finish(bool ok = true);
        void TakePower();
        void TakeAmmo() const;
        void TakeReagents();
        void TakeCastItem();

        SpellCastResult CheckCast(bool strict);
        SpellCastResult CheckPetCast(Unit* target);

        // handlers
        void handle_immediate();
        uint64 handle_delayed(uint64 t_offset);
        // handler helpers
        void _handle_immediate_phase();
        void _handle_finish_phase();

        SpellCastResult CheckItems();
        SpellCastResult CheckRange(bool strict);
        SpellCastResult CheckPower();
        SpellCastResult CheckCasterAuras() const;

        int32 CalculateDamage(SpellEffectIndex i, Unit* target) const { return m_caster->CalculateSpellDamage(target, m_spellInfo, i, &m_currentBasePoints[i]); }
        static uint32 CalculatePowerCost(SpellEntry const* spellInfo, Unit* caster, Spell const* spell = nullptr, Item* castItem = nullptr);

        bool HaveTargetsForEffect(SpellEffectIndex effect) const;
        void Delayed();
        void DelayedChannel();
        uint32 getState() const { return m_spellState; }
        void setState(uint32 state) { m_spellState = state; }

        uint32 GetUsableHealthStoneItemType(Unit* unitTarget);
        bool DoCreateItem(SpellEffectIndex eff_idx, uint32 itemtype);

        void WriteSpellGoTargets(WorldPacket& data);
        void WriteAmmoToPacket(WorldPacket& data) const;

        template<typename T> WorldObject* FindCorpseUsing();

        bool CheckTargetGOScript(GameObject* target, SpellEffectIndex eff) const;
        bool CheckTargetScript(Unit* target, SpellEffectIndex eff) const;
        bool CheckTarget(Unit* target, SpellEffectIndex eff, bool targetB, CheckException exception = EXCEPTION_NONE) const;
        bool CanAutoCast(Unit* target);

        static void SendCastResult(Player const* caster, SpellEntry const* spellInfo, SpellCastResult result, bool isPetCastResult = false);
        void SendCastResult(SpellCastResult result) const;
        void SendSpellStart() const;
        void SendSpellGo();
        void SendSpellCooldown();
        void SendInterrupted(SpellCastResult result) const;
        void SendChannelUpdate(uint32 time, uint32 lastTick = 0) const;
        void SendChannelStart(uint32 duration);
        void SendResurrectRequest(Player* target) const;

        void StopCast(SpellCastResult castResult);

        void HandleEffects(Unit* pUnitTarget, Item* pItemTarget, GameObject* pGOTarget, SpellEffectIndex i, float DamageMultiplier = 1.0);
        void HandleThreatSpells();
        // void HandleAddAura(Unit* Target);

        SpellEntry const* m_spellInfo;
        SpellEntry const* m_triggeredBySpellInfo;
        int32 m_currentBasePoints[MAX_EFFECT_INDEX];        // cache SpellEntry::CalculateSimpleValue and use for set custom base points

        ObjectGuid m_CastItemGuid;
        Item* m_CastItem;

        SpellCastTargets m_targets;

        // Trigger flag system
        bool m_IsTriggeredSpell;
        bool m_ignoreHitResult;
        bool m_ignoreCastTime;
        bool m_ignoreUnselectableTarget;
        bool m_ignoreUnattackableTarget;
        bool m_triggerAutorepeat;
        bool m_doNotProc;
        bool m_petCast;
        bool m_notifyAI;

        int32 GetCastTime() const { return m_casttime; }
        uint32 GetCastedTime() const { return m_timer; }
        bool IsAutoRepeat() const { return m_autoRepeat; }
        void SetAutoRepeat(bool rep) { m_autoRepeat = rep; }
        void ReSetTimer() { m_timer = m_casttime > 0 ? m_casttime : 0; }
        bool IsNextMeleeSwingSpell() const
        {
            return m_spellInfo->HasAttribute(SPELL_ATTR_ON_NEXT_SWING_1) || m_spellInfo->HasAttribute(SPELL_ATTR_ON_NEXT_SWING_2);
        }
        bool IsRangedSpell() const
        {
            return  m_spellInfo->HasAttribute(SPELL_ATTR_RANGED);
        }
        bool IsChannelActive() const { return m_caster->GetUInt32Value(UNIT_CHANNEL_SPELL) != 0; }
        bool IsMeleeAttackResetSpell() const { return !m_IsTriggeredSpell && (m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_INTERRUPT);  }
        bool IsRangedAttackResetSpell() const { return !m_IsTriggeredSpell && IsRangedSpell() && (m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_INTERRUPT); }
        bool IsEffectWithImplementedMultiplier(uint32 effectId) const;

        bool IsDeletable() const { return !m_referencedFromCurrentSpell && !m_executedCurrently; }
        void SetReferencedFromCurrent(bool yes) { m_referencedFromCurrentSpell = yes; }
        void SetExecutedCurrently(bool yes) { m_executedCurrently = yes; }
        bool IsExecutedCurrently() const { return m_executedCurrently; }
        uint64 GetDelayStart() const { return m_delayStart; }
        void SetDelayStart(uint64 m_time) { m_delayStart = m_time; }
        uint64 GetDelayMoment() const { return m_delayMoment; }

        bool IsNeedSendToClient() const;                    // use for hide spell cast for client in case when cast not have client side affect (animation or log entries)
        bool IsTriggeredSpellWithRedundantCastTime() const; // use for ignore some spell data for triggered spells like cast time, some triggered spells have redundent copy data from main spell for client use purpose
        bool IsTriggeredByAura() const { return m_triggeredByAuraSpell != nullptr; }

        CurrentSpellTypes GetCurrentContainer() const;

        // caster types:
        // formal spell caster, in game source of spell affects cast
        Unit* GetCaster() const { return m_caster; }
        // real source of cast affects, explicit caster, or DoT/HoT applier, or GO owner, or wild GO itself. Can be nullptr
        WorldObject* GetAffectiveCasterObject() const;
        // limited version returning nullptr in cases wild gameobject caster object, need for Aura (auras currently not support non-Unit caster)
        Unit* GetAffectiveCaster() const { return m_originalCasterGUID ? m_originalCaster : m_caster; }
        // m_originalCasterGUID can store GO guid, and in this case this is visual caster
        WorldObject* GetCastingObject() const;

        uint32 GetPowerCost() const { return m_powerCost; }
        // channels are generally instant - until more research provided all need to have speed 0
        float GetSpellSpeed() const;

        void UpdatePointers();                              // must be used at call Spell code after time delay (non triggered spell cast/update spell call/etc)

        bool CheckTargetCreatureType(Unit* target) const;

        void AddTriggeredSpell(SpellEntry const* spellInfo) { m_TriggerSpells.push_back(spellInfo); }
        void AddPrecastSpell(SpellEntry const* spellInfo) { m_preCastSpells.push_back(spellInfo); }
        void AddTriggeredSpell(uint32 spellId);
        void AddPrecastSpell(uint32 spellId);
        void CastPreCastSpells(Unit* target);
        void CastTriggerSpells();

        void CleanupTargetList();
        void ClearCastItem();

        void ProcSpellAuraTriggers();

        bool CanBeInterrupted() const { return m_spellState <= SPELL_STATE_DELAYED || m_spellState == SPELL_STATE_CHANNELING; }

        uint64 GetScriptValue() const { return m_scriptValue; }
        void SetScriptValue(uint64 value) { m_scriptValue = value; }

        // Spell Script hooks
        void OnSuccessfulSpellStart();
        void OnSuccessfulSpellFinish();
        SpellCastResult OnCheckCast(bool strict);

    protected:
        void SendLoot(ObjectGuid guid, LootType loottype, LockType lockType);
        bool IgnoreItemRequirements() const;                // some item use spells have unexpected reagent data
        void UpdateOriginalCasterPointer();

        SpellCastResult PreCastCheck(Aura* triggeredByAura = nullptr);
        void Prepare();

        // Spell_C_GetMinMaxRange client equivalent - do not change
        std::pair<float, float> GetMinMaxRange(bool strict);

        Unit* m_caster;

        ObjectGuid m_originalCasterGUID;                    // real source of cast (aura caster/etc), used for spell targets selection
        // e.g. damage around area spell trigered by victim aura and da,age emeies of aura caster
        Unit* m_originalCaster;                             // cached pointer for m_originalCaster, updated at Spell::UpdatePointers()

        // Spell data
        SpellSchoolMask m_spellSchoolMask;                  // Spell school (can be overwrite for some spells (wand shoot for example)
        WeaponAttackType m_attackType;                      // For weapon based attack
        uint32 m_powerCost;                                 // Calculated spell cost     initialized only in Spell::prepare
        int32 m_casttime;                                   // Calculated spell cast time initialized only in Spell::prepare
        int32 m_duration;
        bool m_reflectable;                                  // can reflect this spell?
        bool m_autoRepeat;
        float m_maxRange;                                   // For channeled spell check

        uint8 m_delayAtDamageCount;
        int32 GetNextDelayAtDamageMsTime() { return m_delayAtDamageCount < 5 ? 1000 - (m_delayAtDamageCount++) * 200 : 200; }

        // Delayed spells system
        uint64 m_delayStart;                                // time of spell delay start, filled by event handler, zero = just started
        uint64 m_delayMoment;                               // moment of next delay call, used internally

        // These vars are used in both delayed spell system and modified immediate spell system
        bool m_referencedFromCurrentSpell;                  // mark as references to prevent deleted and access by dead pointers
        bool m_executedCurrently;                           // mark as executed to prevent deleted and access by dead pointers
        bool m_needSpellLog;                                // need to send spell log?
        uint8 m_applyMultiplierMask;                        // by effect: damage multiplier needed?
        float m_damageMultipliers[3];                       // by effect: damage multiplier

        // Current targets, to be used in SpellEffects (MUST BE USED ONLY IN SPELL EFFECTS)
        Unit* unitTarget;
        Item* itemTarget;
        GameObject* gameObjTarget;
        SpellAuraHolder* m_spellAuraHolder;                 // spell aura holder for current target, created only if spell has aura applying effect
        int32 damage;

        // this is set in Spell Hit, but used in Apply Aura handler
        DiminishingLevels m_diminishLevel;
        DiminishingGroup m_diminishGroup;

        // -------------------------------------------
        GameObject* focusObject;

        // Damage and healing in effects need just calculate
        int32 m_damage;                                     // Damage in effects count here
        int32 m_damagePerEffect[MAX_EFFECT_INDEX];
        int32 m_healing;                                    // Healing in effects count here
        int32 m_healingPerEffect[MAX_EFFECT_INDEX];
        int32 m_healthLeech;                                // Health leech in effects for all targets count here

        //******************************************
        // Spell trigger system
        //******************************************
        bool   m_canTrigger;                                // Can start trigger (m_IsTriggeredSpell can`t use for this)
        uint8  m_negativeEffectMask;                        // Use for avoid sent negative spell procs for additional positive effects only targets
        void prepareDataForTriggerSystem();
        void PrepareMasksForProcSystem(uint8 effectMask, uint32& procAttacker, uint32& procVictim, WorldObject* caster, WorldObject* target);

        //*****************************************
        // Spell target filling
        //*****************************************
        struct TempTargetData
        {
            UnitList tmpUnitList[2];
            GameObjectList tmpGOList[2];
            std::list<Item*> tempItemList;
        };
        struct TempTargetingData
        {
            TempTargetData data[MAX_EFFECT_INDEX];
            bool magnet;
        };
        void FillTargetMap();
        void SetTargetMap(SpellEffectIndex effIndex, uint32 targetMode, bool targetB, TempTargetingData& targetingData);
        bool CheckAndAddMagnetTarget(Unit* unitTarget, SpellEffectIndex effIndex, bool targetB, TempTargetingData& data);
        static void CheckSpellScriptTargets(SQLMultiStorage::SQLMSIteratorBounds<SpellTargetEntry>& bounds, UnitList& tempTargetUnitMap, UnitList& targetUnitMap, SpellEffectIndex effIndex);
        void FilterTargetMap(UnitList& filterUnitList, SpellEffectIndex effIndex);

        void FillAreaTargets(UnitList& targetUnitMap, float radius, float cone, SpellNotifyPushType pushType, SpellTargets spellTargets, WorldObject* originalCaster = nullptr);
        void FillRaidOrPartyTargets(UnitList& targetUnitMap, Unit* member, float radius, bool raid, bool withPets, bool withcaster) const;

        // Returns a target that was filled by SPELL_SCRIPT_TARGET (or selected victim) Can return nullptr
        Unit* GetPrefilledUnitTargetOrUnitTarget(SpellEffectIndex effIndex) const;
        void GetSpellRangeAndRadius(SpellEffectIndex effIndex, float& radius, bool targetB, uint32& EffectChainTarget) const;
        float GetCone();

        //*****************************************
        // Spell target subsystem
        //*****************************************
        // Targets store structures and data
        struct TargetInfo
        {
            ObjectGuid targetGUID;
            uint64 timeDelay;
            uint32 HitInfo;
            uint32 damage;
            SpellMissInfo missCondition: 8;
            SpellMissInfo reflectResult: 8;
            uint8  effectHitMask : 8; // Used for all effects which actually hit a target
            uint8  effectMask: 8; // Used for all effects a certain target was evaluated for
            bool   processed: 1;
            bool   magnet: 1;
        };
        uint8 m_needAliveTargetMask;                        // Mask req. alive targets

        struct GOTargetInfo
        {
            ObjectGuid targetGUID;
            uint64 timeDelay;
            uint8  effectMask: 8;
            bool   processed: 1;
        };

        struct ItemTargetInfo
        {
            Item* item;
            uint8 effectMask;
        };

        struct DestTargetInfo
        {
            uint32 effectMask;
            uint64 timeDelay;
            bool processed;
            DestTargetInfo() : effectMask(0), timeDelay(0), processed(false) {}
        };

        typedef std::list<TargetInfo>     TargetList;
        typedef std::list<GOTargetInfo>   GOTargetList;
        typedef std::list<ItemTargetInfo> ItemTargetList;

        TargetList     m_UniqueTargetInfo;
        GOTargetList   m_UniqueGOTargetInfo;
        ItemTargetList m_UniqueItemInfo;
        uint32         m_targetlessMask;
        DestTargetInfo m_destTargetInfo;

        void AddUnitTarget(Unit* target, uint8 effectMask, CheckException exception = EXCEPTION_NONE);
        void AddGOTarget(GameObject* target, uint8 effectMask);
        void AddItemTarget(Item* item, uint8 effectMask);
        void AddDestExecution(SpellEffectIndex effIndex);
        void DoAllEffectOnTarget(TargetInfo* target);
        void HandleDelayedSpellLaunch(TargetInfo* target);
        void InitializeDamageMultipliers();
        void ResetEffectDamageAndHeal();
        void DoSpellHitOnUnit(Unit* unit, uint32 effectMask, bool isReflected = false);
        void DoAllTargetlessEffects(bool dest);
        void DoAllEffectOnTarget(GOTargetInfo* target);
        void DoAllEffectOnTarget(ItemTargetInfo* target);
        bool IsAliveUnitPresentInTargetList();
        bool IsValidDeadOrAliveTarget(Unit const* unit) const;
        SpellCastResult CanOpenLock(SpellEffectIndex effIndex, uint32 lockid, SkillType& skillid, int32& reqSkillValue, int32& skillValue);
        // -------------------------------------------

        // List For Triggered Spells
        typedef std::list<SpellEntry const*> SpellInfoList;
        SpellInfoList m_TriggerSpells;                      // casted by caster to same targets settings in m_targets at success finish of current spell
        SpellInfoList m_preCastSpells;                      // casted by caster to each target at spell hit before spell effects apply

        //*****************************************
        // Spell scripting subsystem
        //*****************************************
        // persistent value to enable storing in script
        uint64 m_scriptValue;

        uint32 m_spellState;
        uint32 m_timer;

        float m_castPositionX;
        float m_castPositionY;
        float m_castPositionZ;
        float m_castOrientation;

        uint32 m_affectedTargetCount;

        // if need this can be replaced by Aura copy
        // we can't store original aura link to prevent access to deleted auras
        // and in same time need aura data and after aura deleting.
        SpellEntry const* m_triggeredByAuraSpell;

        // needed to store all log for this spell
        SpellLog m_spellLog;
};

enum ReplenishType
{
    REPLENISH_UNDEFINED = 0,
    REPLENISH_HEALTH    = 20,
    REPLENISH_MANA      = 21,
    REPLENISH_RAGE      = 22
};

namespace MaNGOS
{
    struct SpellNotifierPlayer              // Currently unused. When put to use this one requires handling for source-location (smilar to below)
    {
        UnitList& i_data;
        Spell& i_spell;
        const uint32& i_index;
        float i_radius;
        WorldObject* i_originalCaster;

        SpellNotifierPlayer(Spell& spell, UnitList& data, const uint32& i, float radius)
            : i_data(data), i_spell(spell), i_index(i), i_radius(radius)
        {
            i_originalCaster = i_spell.GetAffectiveCasterObject();
        }

        void Visit(PlayerMapType& m)
        {
            if (!i_originalCaster)
                return;

            for (auto& itr : m)
            {
                Player* pPlayer = itr.getSource();
                if (!pPlayer->isAlive() || pPlayer->IsTaxiFlying())
                    continue;

                if (!i_originalCaster->CanAttackSpell(pPlayer, i_spell.m_spellInfo))
                    continue;

                if (pPlayer->IsWithinDist3d(i_spell.m_targets.m_destX, i_spell.m_targets.m_destY, i_spell.m_targets.m_destZ, i_radius))
                    i_data.push_back(pPlayer);
            }
        }
        template<class SKIP> void Visit(GridRefManager<SKIP>&) {}
    };

    struct SpellNotifierCreatureAndPlayer
    {
        UnitList& i_data;
        Spell& i_spell;
        SpellNotifyPushType i_push_type;
        float i_radius;
        float i_cone;
        SpellTargets i_TargetType;
        WorldObject* i_originalCaster;
        WorldObject* i_castingObject;
        bool i_playerControlled;
        float i_centerX;
        float i_centerY;
        float i_centerZ;

        float GetCenterX() const { return i_centerX; }
        float GetCenterY() const { return i_centerY; }

        SpellNotifierCreatureAndPlayer(Spell& spell, UnitList& data, float radius, float cone, SpellNotifyPushType type,
                                       SpellTargets TargetType = SPELL_TARGETS_AOE_ATTACKABLE, WorldObject* originalCaster = nullptr)
            : i_data(data), i_spell(spell), i_push_type(type), i_radius(radius), i_cone(cone), i_TargetType(TargetType),
              i_originalCaster(originalCaster), i_castingObject(i_spell.GetCastingObject())
        {
            if (!i_originalCaster)
                i_originalCaster = i_spell.GetAffectiveCasterObject();
            i_playerControlled = i_originalCaster  ? i_originalCaster->IsControlledByPlayer() : false;

            switch (i_push_type)
            {
                case PUSH_CONE:
                case PUSH_SELF_CENTER:
                    if (i_castingObject)
                    {
                        i_centerX = i_castingObject->GetPositionX();
                        i_centerY = i_castingObject->GetPositionY();
                    }
                    break;
                case PUSH_SRC_CENTER:
                    if (i_spell.m_targets.m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
                        i_spell.m_targets.getSource(i_centerX, i_centerY, i_centerZ);
                    break;
                case PUSH_DEST_CENTER:
                    if (i_spell.m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
                        i_spell.m_targets.getDestination(i_centerX, i_centerY, i_centerZ);
                    break;
                case PUSH_TARGET_CENTER:
                    if (Unit* target = i_spell.m_targets.getUnitTarget())
                    {
                        i_centerX = target->GetPositionX();
                        i_centerY = target->GetPositionY();
                        i_centerZ = target->GetPositionZ();
                    }
                    break;
                default:
                    sLog.outError("SpellNotifierCreatureAndPlayer: unsupported PUSH_* case %u.", i_push_type);
            }
        }

        template<class T> inline void Visit(GridRefManager<T>&  m)
        {
            if (!i_originalCaster || !i_castingObject)
                return;

            for (typename GridRefManager<T>::iterator itr = m.begin(); itr != m.end(); ++itr)
            {
                // there are still more spells which can be casted on dead, but
                // they are no AOE and don't have such a nice SPELL_ATTR flag
                // mostly phase check
                if (!itr->getSource()->IsInMap(i_originalCaster) || itr->getSource()->IsTaxiFlying())
                    continue;

                switch (i_TargetType)
                {
                    case SPELL_TARGETS_ASSISTABLE:
                        if (itr->getSource()->GetTypeId() == TYPEID_UNIT && ((Creature*)itr->getSource())->IsTotem())
                            continue;

                        if (!i_originalCaster->CanAssistSpell(itr->getSource(), i_spell.m_spellInfo))
                            continue;
                        break;
                    case SPELL_TARGETS_AOE_ATTACKABLE:
                    {
                        if (itr->getSource()->GetTypeId() == TYPEID_UNIT && ((Creature*)itr->getSource())->IsTotem())
                            continue;

                        if (!i_originalCaster->CanAttackSpell(itr->getSource(), i_spell.m_spellInfo, true))
                            continue;
                    }
                    break;
                    case SPELL_TARGETS_ALL:
                        break;
                    default: continue;
                }

                // we don't need to check InMap here, it's already done some lines above
                switch (i_push_type)
                {
                    case PUSH_CONE:
                        if (i_cone >= 0.f)
                        {
                            if (i_castingObject->isInFront((Unit*)(itr->getSource()), i_radius, i_cone))
                                i_data.push_back(itr->getSource());
                        }
                        else
                        {
                            if (i_castingObject->isInBack((Unit*)(itr->getSource()), i_radius, -i_cone))
                                i_data.push_back(itr->getSource());
                        }
                        break;
                    case PUSH_SELF_CENTER:
                        if (itr->getSource()->GetDistance2d(i_centerX, i_centerY, DIST_CALC_COMBAT_REACH) <= i_radius)
                            i_data.push_back(itr->getSource());
                        break;
                    case PUSH_SRC_CENTER:
                    case PUSH_DEST_CENTER:
                    case PUSH_TARGET_CENTER:
                        if (itr->getSource()->GetDistance(i_centerX, i_centerY, i_centerZ, DIST_CALC_COMBAT_REACH) <= i_radius)
                            i_data.push_back(itr->getSource());
                        break;
                }
            }
        }

#ifdef _MSC_VER
        template<> inline void Visit(CorpseMapType&) {}
        template<> inline void Visit(GameObjectMapType&) {}
        template<> inline void Visit(DynamicObjectMapType&) {}
        template<> inline void Visit(CameraMapType&) {}
#endif
    };

#ifndef _MSC_VER
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(CorpseMapType&) {}
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(GameObjectMapType&) {}
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(DynamicObjectMapType&) {}
    template<> inline void SpellNotifierCreatureAndPlayer::Visit(CameraMapType&) {}
#endif
}

typedef void(Spell::*pEffect)(SpellEffectIndex eff_idx);

class SpellEvent : public BasicEvent
{
    public:
        SpellEvent(Spell* spell);
        virtual ~SpellEvent();

        virtual bool Execute(uint64 e_time, uint32 p_time) override;
        virtual void Abort(uint64 e_time) override;
        virtual bool IsDeletable() const override;

        Spell* GetSpell() const { return m_Spell; }
    protected:
        Spell* m_Spell;
};
#endif
