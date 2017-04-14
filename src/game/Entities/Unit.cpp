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

#include "Entities/Unit.h"
#include "Log.h"
#include "Server/Opcodes.h"
#include "WorldPacket.h"
#include "Server/WorldSession.h"
#include "World/World.h"
#include "Globals/ObjectMgr.h"
#include "Entities/ObjectGuid.h"
#include "Spells/SpellMgr.h"
#include "Entities/Player.h"
#include "Entities/Creature.h"
#include "Spells/Spell.h"
#include "Groups/Group.h"
#include "Spells/SpellAuras.h"
#include "Globals/ObjectAccessor.h"
#include "AI/CreatureAISelector.h"
#include "Entities/TemporarySummon.h"
#include "Entities/Pet.h"
#include "Util.h"
#include "Entities/Totem.h"
#include "BattleGround/BattleGround.h"
#include "Maps/InstanceData.h"
#include "OutdoorPvP/OutdoorPvP.h"
#include "Maps/MapPersistentStateMgr.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "MotionGenerators/MovementGenerator.h"
#include "Movement/MoveSplineInit.h"
#include "Movement/MoveSpline.h"
#include "Entities/CreatureLinkingMgr.h"

#include <math.h>
#include <array>

float baseMoveSpeed[MAX_MOVE_TYPE] =
{
    2.5f,                                                   // MOVE_WALK
    7.0f,                                                   // MOVE_RUN
    4.5f,                                                   // MOVE_RUN_BACK
    4.722222f,                                              // MOVE_SWIM
    2.5f,                                                   // MOVE_SWIM_BACK
    3.141594f,                                              // MOVE_TURN_RATE
};

typedef std::array<uint32, NUM_SPELL_PARTIAL_RESISTS> SpellPartialResistChanceEntry;
typedef std::vector<SpellPartialResistChanceEntry> SpellPartialResistDistribution;
static inline SpellPartialResistDistribution InitSpellPartialResistDistribution()
{
    // Precalculated chances for 0-100% mitigation
    // We use integer random instead of floats, so each chance is premultiplied by 100 (100.00 becomes 10000)
    const SpellPartialResistDistribution precalculated = {
        {{10000,0,0,0,0}},
        {{9700,200,100,0,0}},
        {{9400,400,200,0,0}},
        {{9000,800,200,0,0}},
        {{8700,1000,300,0,0}},
        {{8400,1200,400,0,0}},
        {{8200,1300,400,100,0}},
        {{7900,1500,500,100,0}},
        {{7600,1700,600,100,0}},
        {{7300,1900,700,100,0}},
        {{6900,2300,700,100,0}},
        {{6600,2500,800,100,0}},
        {{6300,2700,900,100,0}},
        {{6000,2900,1000,100,0}},
        {{5800,3000,1000,200,0}},
        {{5400,3300,1100,200,0}},
        {{5100,3600,1100,200,0}},
        {{4800,3800,1200,200,0}},
        {{4400,4200,1200,200,0}},
        {{4100,4400,1300,200,0}},
        {{3700,4800,1300,200,0}},
        {{3400,5000,1400,200,0}},
        {{3100,5200,1500,200,0}},
        {{3000,5200,1500,200,100}},
        {{2800,5300,1500,300,100}},
        {{2500,5500,1600,300,100}},
        {{2400,5400,1700,400,100}},
        {{2300,5300,1800,500,100}},
        {{2200,5100,2100,500,100}},
        {{2100,5000,2200,600,100}},
        {{2000,4900,2400,600,100}},
        {{1900,4700,2600,700,100}},
        {{1800,4600,2700,800,100}},
        {{1700,4400,3000,800,100}},
        {{1600,4300,3100,900,100}},
        {{1500,4200,3200,1000,100}},
        {{1400,4100,3300,1100,100}},
        {{1300,3900,3600,1100,100}},
        {{1300,3600,3800,1200,100}},
        {{1200,3500,3900,1300,100}},
        {{1100,3400,4000,1400,100}},
        {{1000,3300,4100,1500,100}},
        {{900,3100,4400,1500,100}},
        {{800,3000,4500,1600,100}},
        {{800,2700,4700,1700,100}},
        {{700,2600,4800,1800,100}},
        {{600,2500,4900,1900,100}},
        {{600,2300,5000,1900,200}},
        {{500,2200,5100,2000,200}},
        {{300,2200,5300,2000,200}},
        {{200,2100,5400,2100,200}},
        {{200,2000,5300,2200,300}},
        {{200,2000,5100,2200,500}},
        {{200,1900,5000,2300,600}},
        {{100,1900,4900,2500,600}},
        {{100,1800,4800,2600,700}},
        {{100,1700,4700,2700,800}},
        {{100,1600,4500,3000,800}},
        {{100,1500,4400,3100,900}},
        {{100,1500,4100,3300,1000}},
        {{100,1400,4000,3400,1100}},
        {{100,1300,3900,3500,1200}},
        {{100,1200,3800,3600,1300}},
        {{100,1100,3600,3900,1300}},
        {{100,1100,3300,4100,1400}},
        {{100,1000,3200,4200,1500}},
        {{100,900,3100,4300,1600}},
        {{100,800,3000,4400,1700}},
        {{100,800,2700,4600,1800}},
        {{100,700,2600,4700,1900}},
        {{100,600,2400,4900,2000}},
        {{100,600,2200,5000,2100}},
        {{100,500,2100,5100,2200}},
        {{100,500,1800,5300,2300}},
        {{100,400,1700,5400,2400}},
        {{100,300,1600,5500,2500}},
        {{100,300,1500,5300,2800}},
        {{100,200,1500,5200,3000}},
        {{0,200,1500,5200,3100}},
        {{0,200,1400,5000,3400}},
        {{0,200,1300,4800,3700}},
        {{0,200,1300,4400,4100}},
        {{0,200,1200,4200,4400}},
        {{0,200,1200,3800,4800}},
        {{0,200,1100,3600,5100}},
        {{0,200,1100,3300,5400}},
        {{0,200,1000,3000,5800}},
        {{0,100,1000,2900,6000}},
        {{0,100,900,2700,6300}},
        {{0,100,800,2500,6600}},
        {{0,100,700,2300,6900}},
        {{0,100,700,1900,7300}},
        {{0,100,600,1700,7600}},
        {{0,100,500,1500,7900}},
        {{0,100,400,1300,8200}},
        {{0,0,400,1200,8400}},
        {{0,0,300,1000,8700}},
        {{0,0,200,800,9000}},
        {{0,0,200,400,9400}},
        {{0,0,100,200,9700}},
        {{0,0,0,0,10000}},
    };
    // Inflate up to two decimal places of chance %: add all intermediate values and 100% value (100*100 + 1)
    SpellPartialResistDistribution inflated(10001, {{}});
    for (size_t row = 0; row < precalculated.size(); ++row)
    {
        const size_t next = (row + 1);
        const size_t shift = (row * 100);
        const auto &source = precalculated.at(row);
        // Check if this is the last one first
        if (next == precalculated.size())
        {
            for (uint8 column = SPELL_PARTIAL_RESIST_NONE; column < NUM_SPELL_PARTIAL_RESISTS; ++column)
                inflated[shift][column] = source.at(column);
            break;
        }
        const auto &ahead = precalculated.at(next);
        for (size_t intermediate = 0; intermediate < 100; ++intermediate)
        {
            const size_t index = (shift + intermediate);
            auto &values = inflated[index];
            for (uint8 column = SPELL_PARTIAL_RESIST_NONE; column < NUM_SPELL_PARTIAL_RESISTS; ++column)
            {
                const uint32 base = source.at(column);
                const uint32 upcoming = ahead.at(column);
                const int64 diff = (int64(upcoming) - base);
                // Use bigger types signed math to avoid potential erratic behavior on some compilers...
                values[column] = uint32(std::max(0.0, (base + ::round(diff * (intermediate / double(100.0))))));
            }
        }
    }
    return inflated;
}

static const SpellPartialResistDistribution SPELL_PARTIAL_RESIST_DISTRIBUTION = InitSpellPartialResistDistribution();

////////////////////////////////////////////////////////////
// Methods of class MovementInfo

void MovementInfo::Read(ByteBuffer& data)
{
    data >> moveFlags;
    data >> time;
    data >> pos.x;
    data >> pos.y;
    data >> pos.z;
    data >> pos.o;

    if (HasMovementFlag(MOVEFLAG_ONTRANSPORT))
    {
        data >> t_guid;
        data >> t_pos.x;
        data >> t_pos.y;
        data >> t_pos.z;
        data >> t_pos.o;
    }
    if (HasMovementFlag(MOVEFLAG_SWIMMING))
    {
        data >> s_pitch;
    }

    data >> fallTime;

    if (HasMovementFlag(MOVEFLAG_FALLING))
    {
        data >> jump.velocity;
        data >> jump.sinAngle;
        data >> jump.cosAngle;
        data >> jump.xyspeed;
    }

    if (HasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
    {
        data >> u_unk1;                                     // unknown
    }
}

void MovementInfo::Write(ByteBuffer& data) const
{
    data << moveFlags;
    data << time;
    data << pos.x;
    data << pos.y;
    data << pos.z;
    data << pos.o;

    if (HasMovementFlag(MOVEFLAG_ONTRANSPORT))
    {
        data << t_guid;
        data << t_pos.x;
        data << t_pos.y;
        data << t_pos.z;
        data << t_pos.o;
    }
    if (HasMovementFlag(MOVEFLAG_SWIMMING))
    {
        data << s_pitch;
    }

    data << fallTime;

    if (HasMovementFlag(MOVEFLAG_FALLING))
    {
        data << jump.velocity;
        data << jump.sinAngle;
        data << jump.cosAngle;
        data << jump.xyspeed;
    }

    if (HasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
    {
        data << u_unk1;                                     // unknown
    }
}

////////////////////////////////////////////////////////////
// Methods of class GlobalCooldownMgr

bool GlobalCooldownMgr::HasGlobalCooldown(SpellEntry const* spellInfo) const
{
    GlobalCooldownList::const_iterator itr = m_GlobalCooldowns.find(spellInfo->StartRecoveryCategory);
    return itr != m_GlobalCooldowns.end() && itr->second.duration && WorldTimer::getMSTimeDiff(itr->second.cast_time, WorldTimer::getMSTime()) < itr->second.duration;
}

void GlobalCooldownMgr::AddGlobalCooldown(SpellEntry const* spellInfo, uint32 gcd)
{
    m_GlobalCooldowns[spellInfo->StartRecoveryCategory] = GlobalCooldown(gcd, WorldTimer::getMSTime());
}

void GlobalCooldownMgr::CancelGlobalCooldown(SpellEntry const* spellInfo)
{
    m_GlobalCooldowns[spellInfo->StartRecoveryCategory].duration = 0;
}

////////////////////////////////////////////////////////////
// Methods of class Unit

Unit::Unit() :
    movespline(new Movement::MoveSpline()),
    m_charmInfo(nullptr),
    i_motionMaster(this),
    m_regenTimer(0),
    m_combatData(new CombatData(this))
{
    m_objectType |= TYPEMASK_UNIT;
    m_objectTypeId = TYPEID_UNIT;
    m_updateFlag = (UPDATEFLAG_ALL | UPDATEFLAG_LIVING | UPDATEFLAG_HAS_POSITION);

    m_attackTimer[BASE_ATTACK]   = 0;
    m_attackTimer[OFF_ATTACK]    = 0;
    m_attackTimer[RANGED_ATTACK] = 0;
    m_modAttackSpeedPct[BASE_ATTACK] = 1.0f;
    m_modAttackSpeedPct[OFF_ATTACK] = 1.0f;
    m_modAttackSpeedPct[RANGED_ATTACK] = 1.0f;

    m_extraAttacks = 0;

    m_state = 0;
    m_deathState = ALIVE;

    for (uint32 i = 0; i < CURRENT_MAX_SPELL; ++i)
        m_currentSpells[i] = nullptr;

    m_castCounter = 0;

    // m_Aura = nullptr;
    // m_AurasCheck = 2000;
    // m_removeAuraTimer = 4;
    m_spellAuraHoldersUpdateIterator = m_spellAuraHolders.end();
    m_AuraFlags = 0;

    m_Visibility = VISIBILITY_ON;
    m_AINotifyScheduled = false;

    m_detectInvisibilityMask = 0;
    m_invisibilityMask = 0;
    m_transform = 0;
    m_canModifyStats = false;

    for (int i = 0; i < MAX_SPELL_IMMUNITY; ++i)
        m_spellImmune[i].clear();
    for (int i = 0; i < UNIT_MOD_END; ++i)
    {
        m_auraModifiersGroup[i][BASE_VALUE] = 0.0f;
        m_auraModifiersGroup[i][BASE_EXCLUSIVE] = 0.0f;
        m_auraModifiersGroup[i][BASE_PCT] = 1.0f;
        m_auraModifiersGroup[i][TOTAL_VALUE] = 0.0f;
        m_auraModifiersGroup[i][TOTAL_PCT] = 1.0f;
    }

    // implement 50% base damage from offhand
    m_auraModifiersGroup[UNIT_MOD_DAMAGE_OFFHAND][TOTAL_PCT] = 0.5f;

    for (int i = 0; i < MAX_ATTACK; ++i)
    {
        for (int j = 0; j < MAX_ITEM_PROTO_DAMAGES; j++)
        {
            m_weaponDamage[i][j].damage[MINDAMAGE] = (j == 0) ? BASE_MINDAMAGE : 0;
            m_weaponDamage[i][j].damage[MAXDAMAGE] = (j == 0) ? BASE_MAXDAMAGE : 0;
            m_weaponDamage[i][j].school = SPELL_SCHOOL_NORMAL;
        }

        m_weaponDamageCount[i] = 1;
    }

    for (int i = 0; i < MAX_STATS; ++i)
        m_createStats[i] = 0.0f;

    m_attacking = nullptr;
    m_modMeleeHitChance = 0.0f;
    m_modRangedHitChance = 0.0f;
    m_modSpellHitChance = 0.0f;
    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        m_modSpellCritChance[i] = 0.0f;

    for (int i = 0; i < MAX_ATTACK; ++i)
        m_modCritChance[i] = 0.0f;
    m_modDodgeChance = 0.0f;
    m_modParryChance = 0.0f;
    m_modBlockChance = 0.0f;

    m_canDodge = true;
    m_canParry = true;
    m_canBlock = true;

    m_CombatTimer = 0;
    m_lastManaUseTimer = 0;

    // m_victimThreat = 0.0f;
    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        m_threatModifier[i] = 1.0f;
    m_isSorted = true;
    for (int i = 0; i < MAX_MOVE_TYPE; ++i)
        m_speed_rate[i] = 1.0f;

    // remove aurastates allowing special moves
    for (int i = 0; i < MAX_REACTIVE; ++i)
        m_reactiveTimer[i] = 0;

    m_isCreatureLinkingTrigger = false;
    m_isSpawningLinked = false;
    m_dummyCombatState = false;
}

Unit::~Unit()
{
    // set current spells as deletable
    for (uint32 i = 0; i < CURRENT_MAX_SPELL; ++i)
    {
        if (m_currentSpells[i])
        {
            m_currentSpells[i]->SetReferencedFromCurrent(false);
            m_currentSpells[i] = nullptr;
        }
    }

    delete m_combatData;
    delete m_charmInfo;
    delete movespline;

    // those should be already removed at "RemoveFromWorld()" call
    MANGOS_ASSERT(m_gameObj.size() == 0);
    MANGOS_ASSERT(m_dynObjGUIDs.size() == 0);
    MANGOS_ASSERT(m_deletedAuras.size() == 0);
    MANGOS_ASSERT(m_deletedHolders.size() == 0);
}

void Unit::Update(uint32 update_diff, uint32 p_time)
{
    if (!IsInWorld())
        return;

    /*if(p_time > m_AurasCheck)
    {
    m_AurasCheck = 2000;
    _UpdateAura();
    }else
    m_AurasCheck -= p_time;*/

    // WARNING! Order of execution here is important, do not change.
    // Spells must be processed with event system BEFORE they go to _UpdateSpells.
    // Or else we may have some SPELL_STATE_FINISHED spells stalled in pointers, that is bad.
    m_Events.Update(update_diff);
    _UpdateSpells(update_diff);

    CleanupDeletedAuras();

    if (m_lastManaUseTimer)
    {
        if (update_diff >= m_lastManaUseTimer)
            m_lastManaUseTimer = 0;
        else
            m_lastManaUseTimer -= update_diff;
    }

    if (!CanHaveThreatList() && isInCombat())
    {
        // update combat timer only for players and pets (they have no threat list)
        // Check UNIT_STAT_MELEE_ATTACKING or UNIT_STAT_CHASE (without UNIT_STAT_FOLLOW in this case) so pets can reach far away
        // targets without stopping half way there and running off.
        // These flags are reset after target dies or another command is given.
        if (getHostileRefManager().isEmpty())
        {
            // m_CombatTimer set at aura start and it will be freeze until aura removing
            if (m_CombatTimer <= update_diff)
                CombatStop();
            else
                m_CombatTimer -= update_diff;
        }
    }

    if (uint32 base_att = getAttackTimer(BASE_ATTACK))
    {
        setAttackTimer(BASE_ATTACK, (update_diff >= base_att ? 0 : base_att - update_diff));
    }

    if (uint32 base_att = getAttackTimer(OFF_ATTACK))
    {
        setAttackTimer(OFF_ATTACK, (update_diff >= base_att ? 0 : base_att - update_diff));
    }

    // update abilities available only for fraction of time
    UpdateReactives(update_diff);

    UpdateSplineMovement(p_time);
    i_motionMaster.UpdateMotion(p_time);

    if (AI() && isAlive())
        AI()->UpdateAI(p_time);   // AI not react good at real update delays (while freeze in non-active part of map)
    
    if (isAlive())
        ModifyAuraState(AURA_STATE_HEALTHLESS_20_PERCENT, GetHealth() < GetMaxHealth() * 0.20f);
}

bool Unit::UpdateMeleeAttackingState()
{
    Unit* victim = getVictim();
    if (!victim || IsNonMeleeSpellCasted(false))
        return false;

    if (!isAttackReady(BASE_ATTACK) && !(isAttackReady(OFF_ATTACK) && haveOffhandWeapon()))
        return false;

    uint8 swingError = 0;
    if (!CanReachWithMeleeAttack(victim))
    {
        setAttackTimer(BASE_ATTACK, 100);
        setAttackTimer(OFF_ATTACK, 100);
        swingError = 1;
    }
    // 120 degrees of radiant range
    else if (!HasInArc(2 * M_PI_F / 3, victim))
    {
        setAttackTimer(BASE_ATTACK, 100);
        setAttackTimer(OFF_ATTACK, 100);
        swingError = 2;
    }
    else
    {
        if (isAttackReady(BASE_ATTACK))
        {
            // prevent base and off attack in same time, delay attack at 0.2 sec
            if (haveOffhandWeapon())
            {
                if (getAttackTimer(OFF_ATTACK) < ATTACK_DISPLAY_DELAY)
                    setAttackTimer(OFF_ATTACK, ATTACK_DISPLAY_DELAY);
            }
            AttackerStateUpdate(victim, BASE_ATTACK);
            resetAttackTimer(BASE_ATTACK);
        }
        if (haveOffhandWeapon() && isAttackReady(OFF_ATTACK))
        {
            // prevent base and off attack in same time, delay attack at 0.2 sec
            uint32 base_att = getAttackTimer(BASE_ATTACK);
            if (base_att < ATTACK_DISPLAY_DELAY)
                setAttackTimer(BASE_ATTACK, ATTACK_DISPLAY_DELAY);
            // do attack
            AttackerStateUpdate(victim, OFF_ATTACK);
            resetAttackTimer(OFF_ATTACK);
        }
    }

    Player* player = (GetTypeId() == TYPEID_PLAYER ? (Player*)this : nullptr);
    if (player && swingError != player->LastSwingErrorMsg())
    {
        if (swingError == 1)
            player->SendAttackSwingNotInRange();
        else if (swingError == 2)
            player->SendAttackSwingBadFacingAttack();
        player->SwingErrorMsg(swingError);
    }

    return !!swingError;
}

bool Unit::haveOffhandWeapon() const
{
    if (!CanUseEquippedWeapon(OFF_ATTACK))
        return false;

    if (GetTypeId() == TYPEID_PLAYER)
        return !!((Player*)this)->GetWeaponForAttack(OFF_ATTACK, true, true);
    else
    {
        uint8 itemClass = GetByteValue(UNIT_VIRTUAL_ITEM_INFO + (1 * 2) + 0, VIRTUAL_ITEM_INFO_0_OFFSET_CLASS);
        if (itemClass == ITEM_CLASS_WEAPON)
            return true;

        return false;
    }
}

void Unit::SendHeartBeat()
{
    m_movementInfo.UpdateTime(WorldTimer::getMSTime());
    WorldPacket data(MSG_MOVE_HEARTBEAT, 31);
    data << GetPackGUID();
    data << m_movementInfo;
    SendMessageToSet(data, true);
}

void Unit::resetAttackTimer(WeaponAttackType type)
{
    m_attackTimer[type] = uint32(GetAttackTime(type) * m_modAttackSpeedPct[type]);
}

float Unit::GetCombatReach(Unit const* pVictim, bool forMeleeRange /*=true*/, float flat_mod /*=0.0f*/) const
{
    // The measured values show BASE_MELEE_OFFSET in (1.3224, 1.342)
    float reach = GetFloatValue(UNIT_FIELD_COMBATREACH) + pVictim->GetFloatValue(UNIT_FIELD_COMBATREACH) +
                  BASE_MELEERANGE_OFFSET + flat_mod;

    if (forMeleeRange && reach < ATTACK_DISTANCE)
        reach = ATTACK_DISTANCE;

    return reach;
}

float Unit::GetCombatDistance(Unit const* target, bool forMeleeRange) const
{
    float radius = GetCombatReach(target, forMeleeRange);

    float dx = GetPositionX() - target->GetPositionX();
    float dy = GetPositionY() - target->GetPositionY();
    float dz = GetPositionZ() - target->GetPositionZ();
    float dist = sqrt((dx * dx) + (dy * dy) + (dz * dz)) - radius;

    return (dist > 0.0f ? dist : 0.0f);
}

bool Unit::CanReachWithMeleeAttack(Unit const* pVictim, float flat_mod /*= 0.0f*/) const
{
    MANGOS_ASSERT(pVictim);

    float reach = GetCombatReach(pVictim, true, flat_mod);

    // This check is not related to bounding radius
    float dx = GetPositionX() - pVictim->GetPositionX();
    float dy = GetPositionY() - pVictim->GetPositionY();
    float dz = GetPositionZ() - pVictim->GetPositionZ();

    return dx * dx + dy * dy + dz * dz < reach * reach;
}

void Unit::RemoveSpellsCausingAura(AuraType auraType)
{
    for (AuraList::const_iterator iter = m_modAuras[auraType].begin(); iter != m_modAuras[auraType].end();)
    {
        RemoveAurasDueToSpell((*iter)->GetId());
        iter = m_modAuras[auraType].begin();
    }
}

void Unit::RemoveSpellsCausingAura(AuraType auraType, SpellAuraHolder* except)
{
    for (AuraList::const_iterator iter = m_modAuras[auraType].begin(); iter != m_modAuras[auraType].end();)
    {
        // skip `except` aura
        if ((*iter)->GetHolder() == except)
        {
            ++iter;
            continue;
        }

        RemoveAurasDueToSpell((*iter)->GetId(), except);
        iter = m_modAuras[auraType].begin();
    }
}

void Unit::RemoveSpellsCausingAura(AuraType auraType, ObjectGuid casterGuid)
{
    for (AuraList::const_iterator iter = m_modAuras[auraType].begin(); iter != m_modAuras[auraType].end();)
    {
        if ((*iter)->GetCasterGuid() == casterGuid)
        {
            RemoveAuraHolderFromStack((*iter)->GetId(), 1, casterGuid);
            iter = m_modAuras[auraType].begin();
        }
        else
            ++iter;
    }
}

void Unit::DealDamageMods(Unit* pVictim, uint32& damage, uint32* absorb, DamageEffectType damagetype, SpellEntry const* spellProto)
{
    if (!pVictim->isAlive() || pVictim->IsTaxiFlying() || (pVictim->GetTypeId() == TYPEID_UNIT && ((Creature*)pVictim)->IsInEvadeMode()))
    {
        if (absorb)
            *absorb += damage;
        damage = 0;
        return;
    }

    uint32 originalDamage = damage;

    // Script Event damage Deal
    if (AI())
        AI()->DamageDeal(pVictim, damage, damagetype, spellProto);
    // Script Event damage taken
    if (pVictim->AI())
        pVictim->AI()->DamageTaken(this, damage, damagetype, spellProto);

    if (absorb && originalDamage > damage)
        *absorb += (originalDamage - damage);
}

uint32 Unit::DealDamage(Unit* pVictim, uint32 damage, CleanDamage const* cleanDamage, DamageEffectType damagetype, SpellSchoolMask damageSchoolMask, SpellEntry const* spellProto, bool durabilityLoss)
{
    // remove affects from attacker at any non-DoT damage (including 0 damage)
    if (damagetype != DOT)
    {
        if (damagetype != SELF_DAMAGE_ROGUE_FALL)
            RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
        RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

        if (pVictim->GetTypeId() == TYPEID_PLAYER && !pVictim->IsStandState() && !pVictim->hasUnitState(UNIT_STAT_STUNNED))
            pVictim->SetStandState(UNIT_STAND_STATE_STAND);
    }

    if (!damage)
    {
        // Rage from physical damage received .
        if (cleanDamage && cleanDamage->damage && (damageSchoolMask & SPELL_SCHOOL_MASK_NORMAL) && pVictim->GetTypeId() == TYPEID_PLAYER && (pVictim->GetPowerType() == POWER_RAGE))
            ((Player*)pVictim)->RewardRage(cleanDamage->damage, false);

        return 0;
    }

    DEBUG_FILTER_LOG(LOG_FILTER_DAMAGE, "DealDamageStart");

    uint32 health = pVictim->GetHealth();
    DEBUG_FILTER_LOG(LOG_FILTER_DAMAGE, "deal dmg:%d to health:%d ", damage, health);


    // Rage from Damage made (only from direct weapon damage)
    if (cleanDamage && damagetype == DIRECT_DAMAGE && this != pVictim && GetTypeId() == TYPEID_PLAYER && GetPowerType() == POWER_RAGE && cleanDamage->attackType != RANGED_ATTACK)
        ((Player*)this)->RewardRage(damage, true);

    // no xp,health if type 8 /critters/
    if (pVictim->GetTypeId() == TYPEID_UNIT && pVictim->GetCreatureType() == CREATURE_TYPE_CRITTER)
    {
        // TODO: fix this part
        // Critter may not die of damage taken, instead expect it to run away (no fighting back)
        // If (this) is TYPEID_PLAYER, (this) will enter combat w/victim, but after some time, automatically leave combat.
        // It is unclear how it should work for other cases.

        DEBUG_FILTER_LOG(LOG_FILTER_DAMAGE, "DealDamage critter, critter dies");

        ((Creature*)pVictim)->SetLootRecipient(this);

        JustKilledCreature((Creature*)pVictim, nullptr);
        pVictim->SetHealth(0);

        return damage;
    }

    // duel ends when player has 1 or less hp
    bool duel_hasEnded = false;
    if (pVictim->GetTypeId() == TYPEID_PLAYER && ((Player*)pVictim)->duel && damage >= (health - 1))
    {
        // prevent kill only if killed in duel and killed by opponent or opponent controlled creature
        if (((Player*)pVictim)->duel->opponent == this || ((Player*)pVictim)->duel->opponent->GetObjectGuid() == GetOwnerGuid())
            damage = health - 1;

        duel_hasEnded = true;
    }

    // Get in CombatState
    if (pVictim != this && damagetype != DOT && (!spellProto || (!IsPositiveSpellTargetMode(spellProto, this, pVictim) && !spellProto->HasAttribute(SPELL_ATTR_EX3_NO_INITIAL_AGGRO))))
    {
        SetInCombatWith(pVictim);
        pVictim->SetInCombatWith(this);

        if (Player* attackedPlayer = pVictim->GetCharmerOrOwnerPlayerOrPlayerItself())
            SetContestedPvP(attackedPlayer);
    }

    if (pVictim->GetTypeId() == TYPEID_UNIT && !((Creature*)pVictim)->IsPet() && !((Creature*)pVictim)->HasLootRecipient())
        ((Creature*)pVictim)->SetLootRecipient(this);

    if (health <= damage)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_DAMAGE, "DealDamage %s Killed %s", GetGuidStr().c_str(), pVictim->GetGuidStr().c_str());

        /*
         *                      Preparation: Who gets credit for killing whom, invoke SpiritOfRedemtion?
         */
        // for loot will be used only if group_tap == nullptr
        Player* player_tap = GetCharmerOrOwnerPlayerOrPlayerItself();
        Group* group_tap = nullptr;

        // in creature kill case group/player tap stored for creature
        if (pVictim->GetTypeId() == TYPEID_UNIT)
        {
            group_tap = ((Creature*)pVictim)->GetGroupLootRecipient();

            if (Player* recipient = ((Creature*)pVictim)->GetOriginalLootRecipient())
                player_tap = recipient;
        }
        // in player kill case group tap selected by player_tap (killer-player itself, or charmer, or owner, etc)
        else
        {
            if (player_tap)
                group_tap = player_tap->GetGroup();
        }

        // Spirit of Redemtion Talent
        bool damageFromSpiritOfRedemtionTalent = spellProto && spellProto->Id == 27795;
        // if talent known but not triggered (check priest class for speedup check)
        Aura* spiritOfRedemtionTalentReady = nullptr;
        if (!damageFromSpiritOfRedemtionTalent &&           // not called from SPELL_AURA_SPIRIT_OF_REDEMPTION
                pVictim->GetTypeId() == TYPEID_PLAYER && pVictim->getClass() == CLASS_PRIEST)
        {
            AuraList const& vDummyAuras = pVictim->GetAurasByType(SPELL_AURA_DUMMY);
            for (AuraList::const_iterator itr = vDummyAuras.begin(); itr != vDummyAuras.end(); ++itr)
            {
                if ((*itr)->GetSpellProto()->SpellIconID == 1654)
                {
                    spiritOfRedemtionTalentReady = *itr;
                    break;
                }
            }
        }

        /*
         *                      Generic Actions (ProcEvents, Combat-Log, Kill Rewards, Stop Combat)
         */
        // call kill spell proc event (before real die and combat stop to triggering auras removed at death/combat stop)
        if (player_tap && player_tap != pVictim)
        {
            player_tap->ProcDamageAndSpell(pVictim, PROC_FLAG_KILL, PROC_FLAG_KILLED, PROC_EX_NONE, 0);

            WorldPacket data(SMSG_PARTYKILLLOG, (8 + 8));   // send event PARTY_KILL
            data << player_tap->GetObjectGuid();            // player with killing blow
            data << pVictim->GetObjectGuid();               // victim

            if (group_tap)
                group_tap->BroadcastPacket(data, false, group_tap->GetMemberGroup(player_tap->GetObjectGuid()), player_tap->GetObjectGuid());

            player_tap->SendDirectMessage(data);
        }
        else if (GetTypeId() == TYPEID_UNIT && this != pVictim)
            ProcDamageAndSpell(pVictim, PROC_FLAG_KILL, PROC_FLAG_KILLED, PROC_EX_NONE, 0);

        // Reward player, his pets, and group/raid members
        if (player_tap != pVictim)
        {
            if (group_tap)
                group_tap->RewardGroupAtKill(pVictim, player_tap);
            else if (player_tap)
                player_tap->RewardSinglePlayerAtKill(pVictim);
        }

        /*
         *                      Actions for the killer
         */
        if (spiritOfRedemtionTalentReady)
        {
            DEBUG_FILTER_LOG(LOG_FILTER_DAMAGE, "DealDamage: Spirit of Redemtion ready");

            // save value before aura remove
            uint32 ressSpellId = pVictim->GetUInt32Value(PLAYER_SELF_RES_SPELL);
            if (!ressSpellId)
                ressSpellId = ((Player*)pVictim)->GetResurrectionSpellId();

            // Remove all expected to remove at death auras (most important negative case like DoT or periodic triggers)
            pVictim->RemoveAllAurasOnDeath();

            // restore for use at real death
            pVictim->SetUInt32Value(PLAYER_SELF_RES_SPELL, ressSpellId);

            // FORM_SPIRITOFREDEMPTION and related auras
            pVictim->CastSpell(pVictim, 27827, TRIGGERED_OLD_TRIGGERED, nullptr, spiritOfRedemtionTalentReady);
        }
        else
            pVictim->SetHealth(0);

        // Call KilledUnit for creatures
        if (AI())
            AI()->KilledUnit(pVictim);

        // Call AI OwnerKilledUnit (for any current summoned minipet/guardian/protector)
        PetOwnerKilledUnit(pVictim);

        /*
         *                      Actions for the victim
         */
        if (pVictim->GetTypeId() == TYPEID_PLAYER)          // Killed player
        {
            Player* playerVictim = (Player*)pVictim;

            // remember victim PvP death for corpse type and corpse reclaim delay
            // at original death (not at SpiritOfRedemtionTalent timeout)
            if (!damageFromSpiritOfRedemtionTalent)
                playerVictim->SetPvPDeath(player_tap != nullptr);

            // 10% durability loss on death
            // only if not player and not controlled by player pet. And not at BG
            if (durabilityLoss && !player_tap && !playerVictim->InBattleGround())
            {
                DEBUG_LOG("DealDamage: Killed %s, looing 10 percents durability", pVictim->GetGuidStr().c_str());
                playerVictim->DurabilityLossAll(0.10f, false);
                // durability lost message
                WorldPacket data(SMSG_DURABILITY_DAMAGE_DEATH, 0);
                playerVictim->GetSession()->SendPacket(data);
            }

            if (!spiritOfRedemtionTalentReady)              // Before informing Battleground
            {
                DEBUG_FILTER_LOG(LOG_FILTER_DAMAGE, "SET JUST_DIED");
                pVictim->SetDeathState(JUST_DIED);
            }

            // playerVictim was in duel, duel must be interrupted
            // last damage from non duel opponent or non opponent controlled creature
            if (duel_hasEnded)
            {
                playerVictim->duel->opponent->CombatStopWithPets(true);
                playerVictim->CombatStopWithPets(true);

                playerVictim->DuelComplete(DUEL_INTERRUPTED);
            }

            if (player_tap)                                 // PvP kill
            {
                if (BattleGround* bg = playerVictim->GetBattleGround())
                {
                    bg->HandleKillPlayer(playerVictim, player_tap);
                }
                else if (pVictim != this)
                {
                    // selfkills are not handled in outdoor pvp scripts
                    if (OutdoorPvP* outdoorPvP = sOutdoorPvPMgr.GetScript(playerVictim->GetCachedZoneId()))
                        outdoorPvP->HandlePlayerKill(player_tap, playerVictim);
                }
            }
        }
        else                                                // Killed creature
            JustKilledCreature((Creature*)pVictim, player_tap);

        // stop combat
        DEBUG_FILTER_LOG(LOG_FILTER_DAMAGE, "DealDamageAttackStop");
        pVictim->CombatStop();
        pVictim->getHostileRefManager().deleteReferences();
    }
    else                                                    // if (health <= damage)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_DAMAGE, "DealDamageAlive");

        pVictim->ModifyHealth(- (int32)damage);

        if (damagetype != DOT)
        {
            if (!getVictim())
            {
                // if not have main target then attack state with target (including AI call)
                // start melee attacks only after melee hit
                Attack(pVictim, (damagetype == DIRECT_DAMAGE));
            }

            // if damage pVictim call AI reaction
            pVictim->AttackedBy(this);
        }

        if (damagetype == DIRECT_DAMAGE || damagetype == SPELL_DIRECT_DAMAGE)
        {
            if (!spellProto || !(spellProto->AuraInterruptFlags & AURA_INTERRUPT_FLAG_DIRECT_DAMAGE))
                pVictim->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_DIRECT_DAMAGE);
        }
        if (pVictim->GetTypeId() != TYPEID_PLAYER)
        {
            float threat = damage * sSpellMgr.GetSpellThreatMultiplier(spellProto);
            pVictim->AddThreat(this, threat, (cleanDamage && cleanDamage->hitOutCome == MELEE_HIT_CRIT), damageSchoolMask, spellProto);
        }
        else                                                // victim is a player
        {
            // Rage from damage received
            if (this != pVictim && pVictim->GetPowerType() == POWER_RAGE)
            {
                uint32 rage_damage = damage + (cleanDamage ? cleanDamage->damage : 0);
                ((Player*)pVictim)->RewardRage(rage_damage, false);
            }

            // random durability for items (HIT TAKEN)
            if (roll_chance_f(sWorld.getConfig(CONFIG_FLOAT_RATE_DURABILITY_LOSS_DAMAGE)))
            {
                EquipmentSlots slot = EquipmentSlots(urand(0, EQUIPMENT_SLOT_END - 1));
                ((Player*)pVictim)->DurabilityPointLossForEquipSlot(slot);
            }
        }

        if (GetTypeId() == TYPEID_PLAYER)
        {
            // random durability for items (HIT DONE)
            if (roll_chance_f(sWorld.getConfig(CONFIG_FLOAT_RATE_DURABILITY_LOSS_DAMAGE)))
            {
                EquipmentSlots slot = EquipmentSlots(urand(0, EQUIPMENT_SLOT_END - 1));
                ((Player*)this)->DurabilityPointLossForEquipSlot(slot);
            }
        }

        if ((damagetype == DIRECT_DAMAGE || damagetype == SPELL_DIRECT_DAMAGE || damagetype == DOT)
            && !(spellProto && spellProto->HasAttribute(SPELL_ATTR_EX4_DAMAGE_DOESNT_BREAK_AURAS)))
        {
            int32 auraInterruptFlags = AURA_INTERRUPT_FLAG_DAMAGE;
            if (damagetype != DOT)
                auraInterruptFlags = (auraInterruptFlags | AURA_INTERRUPT_FLAG_DIRECT_DAMAGE);

            SpellAuraHolderMap& vInterrupts = pVictim->GetSpellAuraHolderMap();
            std::vector<uint32> cleanupHolder;

            for (auto aura : vInterrupts)
            {
                if (spellProto && spellProto->Id == aura.second->GetId()) // Not drop auras added by self
                    continue;

                const SpellEntry* se = aura.second->GetSpellProto();

                if (!se)
                    continue;

                if (se->AuraInterruptFlags & auraInterruptFlags)
                    cleanupHolder.push_back(aura.second->GetId());
            }

            for (auto aura : cleanupHolder)
                pVictim->RemoveAurasDueToSpell(aura);
        }

        if (damagetype != NODAMAGE && damage && pVictim->GetTypeId() == TYPEID_PLAYER)
        {
            if (damagetype != DOT)
            {
                for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; ++i)
                {
                    // skip channeled spell (processed differently below)
                    if (i == CURRENT_CHANNELED_SPELL)
                        continue;

                    if (Spell* spell = pVictim->GetCurrentSpell(CurrentSpellTypes(i)))
                    {
                        if (spell->getState() == SPELL_STATE_PREPARING)
                        {
                            if (spell->m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_ABORT_ON_DMG)
                                pVictim->InterruptSpell(CurrentSpellTypes(i));
                            else
                                spell->Delayed();
                        }
                    }
                }
            }

            if (Spell* spell = pVictim->m_currentSpells[CURRENT_CHANNELED_SPELL])
            {
                if (spell->getState() == SPELL_STATE_CASTING)
                {
                    uint32 channelInterruptFlags = spell->m_spellInfo->ChannelInterruptFlags;
                    if (channelInterruptFlags & CHANNEL_FLAG_DELAY)
                    {
                        if (pVictim != this)                // don't shorten the duration of channeling if you damage yourself
                            spell->DelayedChannel();
                    }
                    else if ((channelInterruptFlags & (CHANNEL_FLAG_DAMAGE | CHANNEL_FLAG_DAMAGE2)))
                    {
                        DETAIL_LOG("Spell %u canceled at damage!", spell->m_spellInfo->Id);
                        pVictim->InterruptSpell(CURRENT_CHANNELED_SPELL);
                    }
                }
                else if (spell->getState() == SPELL_STATE_DELAYED)
                    // break channeled spell in delayed state on damage
                {
                    DETAIL_LOG("Spell %u canceled at damage!", spell->m_spellInfo->Id);
                    pVictim->InterruptSpell(CURRENT_CHANNELED_SPELL);
                }
            }
        }

        // last damage from duel opponent
        if (duel_hasEnded)
        {
            MANGOS_ASSERT(pVictim->GetTypeId() == TYPEID_PLAYER);
            Player* he = (Player*)pVictim;

            MANGOS_ASSERT(he->duel);

            he->SetHealth(1);

            he->duel->opponent->CombatStopWithPets(true);
            he->CombatStopWithPets(true);

            he->CastSpell(he, 7267, TRIGGERED_OLD_TRIGGERED);                  // beg
            he->DuelComplete(DUEL_WON);
        }
    }

    DEBUG_FILTER_LOG(LOG_FILTER_DAMAGE, "DealDamageEnd returned %d damage", damage);

    return damage;
}

struct PetOwnerKilledUnitHelper
{
    explicit PetOwnerKilledUnitHelper(Unit* pVictim) : m_victim(pVictim) {}
    void operator()(Unit* pTarget) const
    {
        if (pTarget->GetTypeId() == TYPEID_UNIT)
        {
            if (pTarget->AI())
                pTarget->AI()->OwnerKilledUnit(m_victim);
        }
    }

    Unit* m_victim;
};

void Unit::JustKilledCreature(Creature* victim, Player* responsiblePlayer)
{
    victim->m_deathState = DEAD;                            // so that isAlive, isDead return expected results in the called hooks of JustKilledCreature
    // must be used only shortly before SetDeathState(JUST_DIED) and only for Creatures or Pets

    // some critters required for quests (need normal entry instead possible heroic in any cases)
    if (victim->GetCreatureType() == CREATURE_TYPE_CRITTER && GetTypeId() == TYPEID_PLAYER)
    {
        if (CreatureInfo const* normalInfo = ObjectMgr::GetCreatureTemplate(victim->GetEntry()))
            ((Player*)this)->KilledMonster(normalInfo, victim->GetObjectGuid());
    }

    // Interrupt channeling spell when a Possessed Summoned is killed
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(victim->GetUInt32Value(UNIT_CREATED_BY_SPELL));
    if (spellInfo && spellInfo->HasAttribute(SPELL_ATTR_EX_FARSIGHT) && spellInfo->HasAttribute(SPELL_ATTR_EX_CHANNELED_1))
    {
        Unit* creator = GetMap()->GetUnit(victim->GetCreatorGuid());
        if (creator && creator->GetCharmGuid() == victim->GetObjectGuid())
        {
            Spell* channeledSpell = creator->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
            if (channeledSpell && channeledSpell->m_spellInfo->Id == spellInfo->Id)
                creator->InterruptNonMeleeSpells(false);
        }
    }

    /* ******************************* Inform various hooks ************************************ */
    // Inform victim's AI
    if (victim->AI())
        victim->AI()->JustDied(this);

    // Inform Owner
    Unit* pOwner = victim->GetCharmerOrOwner();
    if (victim->IsTemporarySummon())
    {
        TemporarySummon* pSummon = (TemporarySummon*)victim;
        if (pSummon->GetSummonerGuid().IsCreature())
            if (Creature* pSummoner = victim->GetMap()->GetCreature(pSummon->GetSummonerGuid()))
                if (pSummoner->AI())
                    pSummoner->AI()->SummonedCreatureJustDied(victim);
    }
    else if (pOwner && pOwner->AI())
            pOwner->AI()->SummonedCreatureJustDied(victim);

    // Inform Instance Data and Linking
    if (InstanceData* mapInstance = victim->GetInstanceData())
        mapInstance->OnCreatureDeath(victim);

    if (responsiblePlayer)                                  // killedby Player, inform BG
        if (BattleGround* bg = responsiblePlayer->GetBattleGround())
            bg->HandleKillUnit(victim, responsiblePlayer);

    // Notify the outdoor pvp script
    if (OutdoorPvP* outdoorPvP = sOutdoorPvPMgr.GetScript(responsiblePlayer ? responsiblePlayer->GetCachedZoneId() : GetZoneId()))
        outdoorPvP->HandleCreatureDeath(victim);

    // Start creature death script
    GetMap()->ScriptsStart(sCreatureDeathScripts, victim->GetEntry(), victim, responsiblePlayer ? responsiblePlayer : this);

    if (victim->IsLinkingEventTrigger())
        victim->GetMap()->GetCreatureLinkingHolder()->DoCreatureLinkingEvent(LINKING_EVENT_DIE, victim);

    // Dungeon specific stuff
    if (victim->GetInstanceId())
    {
        Map* m = victim->GetMap();
        Player* creditedPlayer = GetCharmerOrOwnerPlayerOrPlayerItself();
        // TODO: do instance binding anyway if the charmer/owner is offline

        if (m->IsDungeon() && creditedPlayer)
        {
            if (m->IsRaid())
            {
                if (victim->GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_INSTANCE_BIND)
                    ((DungeonMap*)m)->PermBindAllPlayers(creditedPlayer);
            }
            else
            {
                DungeonPersistentState* save = ((DungeonMap*)m)->GetPersistanceState();
                // the reset time is set but not added to the scheduler
                // until the players leave the instance
                time_t resettime = victim->GetRespawnTimeEx() + 2 * HOUR;
                if (save->GetResetTime() < resettime)
                    save->SetResetTime(resettime);
            }

            ((DungeonMap*)m)->GetPersistanceState()->UpdateEncounterState(ENCOUNTER_CREDIT_KILL_CREATURE, victim->GetEntry());
        }
    }

    bool isPet = victim->IsPet();

    /* ********************************* Set Death finally ************************************* */
    DEBUG_FILTER_LOG(LOG_FILTER_DAMAGE, "SET JUST_DIED");
    victim->SetDeathState(JUST_DIED);                       // if !spiritOfRedemtionTalentReady always true for unit

    if (isPet)
        return;                                             // Pets might have been unsummoned at this place, do not handle them further!

    /* ******************************** Prepare loot if can ************************************ */
    victim->DeleteThreatList();

    // only lootable if it has loot or can drop gold
    victim->PrepareBodyLootState();
}

void Unit::PetOwnerKilledUnit(Unit* pVictim)
{
    // for minipet and guardians (including protector)
    CallForAllControlledUnits(PetOwnerKilledUnitHelper(pVictim), CONTROLLED_MINIPET | CONTROLLED_GUARDIANS);
}

void Unit::CastStop(uint32 except_spellid)
{
    for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; ++i)
        if (m_currentSpells[i] && m_currentSpells[i]->m_spellInfo->Id != except_spellid)
            InterruptSpell(CurrentSpellTypes(i), false);
}

SpellCastResult Unit::CastSpell(Unit* Victim, uint32 spellId, uint32 triggeredFlags, Item* castItem, Aura* triggeredByAura, ObjectGuid originalCaster, SpellEntry const* triggeredBy)
{
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

    if (!spellInfo)
    {
        if (triggeredByAura)
            sLog.outError("CastSpell: unknown spell id %i by caster: %s triggered by aura %u (eff %u)", spellId, GetGuidStr().c_str(), triggeredByAura->GetId(), triggeredByAura->GetEffIndex());
        else
            sLog.outError("CastSpell: unknown spell id %i by caster: %s", spellId, GetGuidStr().c_str());
        return SPELL_NOT_FOUND;
    }

    return CastSpell(Victim, spellInfo, triggeredFlags, castItem, triggeredByAura, originalCaster, triggeredBy);
}

SpellCastResult Unit::CastSpell(Unit* Victim, SpellEntry const* spellInfo, uint32 triggeredFlags, Item* castItem, Aura* triggeredByAura, ObjectGuid originalCaster, SpellEntry const* triggeredBy)
{
    if (!spellInfo)
    {
        if (triggeredByAura)
            sLog.outError("CastSpell: unknown spell by caster: %s triggered by aura %u (eff %u)", GetGuidStr().c_str(), triggeredByAura->GetId(), triggeredByAura->GetEffIndex());
        else
            sLog.outError("CastSpell: unknown spell by caster: %s", GetGuidStr().c_str());

        return SPELL_NOT_FOUND;
    }

    if (castItem)
        DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "WORLD: cast Item spellId - %i", spellInfo->Id);

    if (triggeredByAura)
    {
        if (!originalCaster)
            originalCaster = triggeredByAura->GetCasterGuid();

        triggeredBy = triggeredByAura->GetSpellProto();
    }

    Spell* spell = new Spell(this, spellInfo, triggeredFlags, originalCaster, triggeredBy);

    SpellCastTargets targets;
    targets.setUnitTarget(Victim);

    if (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
        targets.setDestination(Victim->GetPositionX(), Victim->GetPositionY(), Victim->GetPositionZ());
    if (spellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION)
        if (WorldObject* caster = spell->GetCastingObject())
            targets.setSource(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ());

    spell->m_CastItem = castItem;
    return spell->SpellStart(&targets, triggeredByAura);
}

SpellCastResult Unit::CastCustomSpell(Unit* Victim, uint32 spellId, int32 const* bp0, int32 const* bp1, int32 const* bp2, uint32 triggeredFlags, Item* castItem, Aura* triggeredByAura, ObjectGuid originalCaster, SpellEntry const* triggeredBy)
{
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

    if (!spellInfo)
    {
        if (triggeredByAura)
            sLog.outError("CastCustomSpell: unknown spell id %i by caster: %s triggered by aura %u (eff %u)", spellId, GetGuidStr().c_str(), triggeredByAura->GetId(), triggeredByAura->GetEffIndex());
        else
            sLog.outError("CastCustomSpell: unknown spell id %i by caster: %s", spellId, GetGuidStr().c_str());
        return SPELL_NOT_FOUND;
    }

    return CastCustomSpell(Victim, spellInfo, bp0, bp1, bp2, triggeredFlags, castItem, triggeredByAura, originalCaster, triggeredBy);
}

SpellCastResult Unit::CastCustomSpell(Unit* Victim, SpellEntry const* spellInfo, int32 const* bp0, int32 const* bp1, int32 const* bp2, uint32 triggeredFlags, Item* castItem, Aura* triggeredByAura, ObjectGuid originalCaster, SpellEntry const* triggeredBy)
{
    if (!spellInfo)
    {
        if (triggeredByAura)
            sLog.outError("CastCustomSpell: unknown spell by caster: %s triggered by aura %u (eff %u)", GetGuidStr().c_str(), triggeredByAura->GetId(), triggeredByAura->GetEffIndex());
        else
            sLog.outError("CastCustomSpell: unknown spell by caster: %s", GetGuidStr().c_str());
        return SPELL_NOT_FOUND;
    }

    if (castItem)
        DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "WORLD: cast Item spellId - %i", spellInfo->Id);

    if (triggeredByAura)
    {
        if (!originalCaster)
            originalCaster = triggeredByAura->GetCasterGuid();

        triggeredBy = triggeredByAura->GetSpellProto();
    }

    Spell* spell = new Spell(this, spellInfo, triggeredFlags, originalCaster, triggeredBy);

    if (bp0)
        spell->m_currentBasePoints[EFFECT_INDEX_0] = *bp0;

    if (bp1)
        spell->m_currentBasePoints[EFFECT_INDEX_1] = *bp1;

    if (bp2)
        spell->m_currentBasePoints[EFFECT_INDEX_2] = *bp2;

    SpellCastTargets targets;
    targets.setUnitTarget(Victim);
    spell->m_CastItem = castItem;

    if (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
        targets.setDestination(Victim->GetPositionX(), Victim->GetPositionY(), Victim->GetPositionZ());
    if (spellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION)
        if (WorldObject* caster = spell->GetCastingObject())
            targets.setSource(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ());

    return spell->SpellStart(&targets, triggeredByAura);
}

// used for scripting
SpellCastResult Unit::CastSpell(float x, float y, float z, uint32 spellId, uint32 triggeredFlags, Item* castItem, Aura* triggeredByAura, ObjectGuid originalCaster, SpellEntry const* triggeredBy)
{
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

    if (!spellInfo)
    {
        if (triggeredByAura)
            sLog.outError("CastSpell(x,y,z): unknown spell id %i by caster: %s triggered by aura %u (eff %u)", spellId, GetGuidStr().c_str(), triggeredByAura->GetId(), triggeredByAura->GetEffIndex());
        else
            sLog.outError("CastSpell(x,y,z): unknown spell id %i by caster: %s", spellId, GetGuidStr().c_str());
        return SPELL_NOT_FOUND;
    }

    return CastSpell(x, y, z, spellInfo, triggeredFlags, castItem, triggeredByAura, originalCaster, triggeredBy);
}

// used for scripting
SpellCastResult Unit::CastSpell(float x, float y, float z, SpellEntry const* spellInfo, uint32 triggeredFlags, Item* castItem, Aura* triggeredByAura, ObjectGuid originalCaster, SpellEntry const* triggeredBy)
{
    if (!spellInfo)
    {
        if (triggeredByAura)
            sLog.outError("CastSpell(x,y,z): unknown spell by caster: %s triggered by aura %u (eff %u)", GetGuidStr().c_str(), triggeredByAura->GetId(), triggeredByAura->GetEffIndex());
        else
            sLog.outError("CastSpell(x,y,z): unknown spell by caster: %s", GetGuidStr().c_str());
        return SPELL_NOT_FOUND;
    }

    if (castItem)
        DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "WORLD: cast Item spellId - %i", spellInfo->Id);

    if (triggeredByAura)
    {
        if (!originalCaster)
            originalCaster = triggeredByAura->GetCasterGuid();

        triggeredBy = triggeredByAura->GetSpellProto();
    }

    Spell* spell = new Spell(this, spellInfo, triggeredFlags, originalCaster, triggeredBy);

    SpellCastTargets targets;

    if (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
        targets.setDestination(x, y, z);
    if (spellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION)
        targets.setSource(x, y, z);

    // Spell cast with x,y,z but without dbc target-mask, set destination
    if (!(targets.m_targetMask & (TARGET_FLAG_DEST_LOCATION | TARGET_FLAG_SOURCE_LOCATION)))
        targets.setDestination(x, y, z);

    spell->m_CastItem = castItem;
    return spell->SpellStart(&targets, triggeredByAura);
}

// Obsolete func need remove, here only for comotability vs another patches
uint32 Unit::SpellNonMeleeDamageLog(Unit* pVictim, uint32 spellID, uint32 damage)
{
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellID);
    SpellNonMeleeDamage damageInfo(this, pVictim, spellInfo->Id, SpellSchools(spellInfo->School));
    CalculateSpellDamage(&damageInfo, damage, spellInfo);
    damageInfo.target->CalculateAbsorbResistBlock(this, &damageInfo, spellInfo);
    DealDamageMods(damageInfo.target, damageInfo.damage, &damageInfo.absorb, SPELL_DIRECT_DAMAGE, spellInfo);
    SendSpellNonMeleeDamageLog(&damageInfo);
    DealSpellDamage(&damageInfo, true);
    return damageInfo.damage;
}

void Unit::CalculateSpellDamage(SpellNonMeleeDamage* damageInfo, int32 damage, SpellEntry const* spellInfo, WeaponAttackType attackType)
{
    SpellSchoolMask damageSchoolMask = GetSchoolMask(damageInfo->school);
    Unit* pVictim = damageInfo->target;

    if (damage < 0)
        return;

    if (!this || !pVictim)
        return;

    // units which are not alive cannot deal damage except for dying creatures
    if ((!this->isAlive() || !pVictim->isAlive()) && (this->GetTypeId() != TYPEID_UNIT || this->getDeathState() != DEAD))
        return;

    // Check spell crit chance
    bool crit = RollSpellCritOutcome(pVictim, damageSchoolMask, spellInfo);

    // damage bonus (per damage class)
    switch (spellInfo->DmgClass)
    {
        // Melee and Ranged Spells
        case SPELL_DAMAGE_CLASS_RANGED:
        case SPELL_DAMAGE_CLASS_MELEE:
        {
            // Calculate damage bonus
            damage = MeleeDamageBonusDone(pVictim, damage, attackType, spellInfo, SPELL_DIRECT_DAMAGE);
            damage = pVictim->MeleeDamageBonusTaken(this, damage, attackType, spellInfo, SPELL_DIRECT_DAMAGE);
        }
        break;
        // Magical Attacks
        case SPELL_DAMAGE_CLASS_NONE:
        case SPELL_DAMAGE_CLASS_MAGIC:
        {
            // Calculate damage bonus
            damage = SpellDamageBonusDone(pVictim, spellInfo, damage, SPELL_DIRECT_DAMAGE);
            damage = pVictim->SpellDamageBonusTaken(this, spellInfo, damage, SPELL_DIRECT_DAMAGE);
        }
        break;
    }

    // if crit add critical bonus
    if (crit)
    {
        damageInfo->HitInfo |= SPELL_HIT_TYPE_CRIT;
        damage = CalculateCritAmount(pVictim, damage, spellInfo);
    }

    // damage mitigation
    if (damage > 0)
    {
        // physical damage => armor
        if (damageSchoolMask & SPELL_SCHOOL_MASK_NORMAL)
            damage = CalcArmorReducedDamage(pVictim, damage);
    }
    else
        damage = 0;
    damageInfo->damage = damage;
}

void Unit::DealSpellDamage(SpellNonMeleeDamage* damageInfo, bool durabilityLoss)
{
    if (!damageInfo)
        return;

    Unit* pVictim = damageInfo->target;

    if (!this || !pVictim)
        return;

    if (!pVictim->isAlive() || pVictim->IsTaxiFlying() || (pVictim->GetTypeId() == TYPEID_UNIT && ((Creature*)pVictim)->IsInEvadeMode()))
        return;

    SpellEntry const* spellProto = sSpellTemplate.LookupEntry<SpellEntry>(damageInfo->SpellID);
    if (spellProto == nullptr)
    {
        sLog.outError("Unit::DealSpellDamage have wrong damageInfo->SpellID: %u", damageInfo->SpellID);
        return;
    }

    // Call default DealDamage (send critical in hit info for threat calculation)
    CleanDamage cleanDamage(0, BASE_ATTACK, damageInfo->HitInfo & SPELL_HIT_TYPE_CRIT ? MELEE_HIT_CRIT : MELEE_HIT_NORMAL);
    DealDamage(pVictim, damageInfo->damage, &cleanDamage, SPELL_DIRECT_DAMAGE, GetSchoolMask(damageInfo->school), spellProto, durabilityLoss);
}

// TODO for melee need create structure as in
void Unit::CalculateMeleeDamage(Unit* pVictim, CalcDamageInfo* damageInfo, WeaponAttackType attackType /*= BASE_ATTACK*/)
{
    damageInfo->attacker         = this;
    damageInfo->target           = pVictim;
    damageInfo->attackType       = attackType;
    damageInfo->totalDamage      = 0;
    damageInfo->cleanDamage      = 0;
    damageInfo->blocked_amount   = 0;

    damageInfo->TargetState      = VICTIMSTATE_UNAFFECTED;
    damageInfo->HitInfo          = HITINFO_NORMALSWING;
    damageInfo->procAttacker     = PROC_FLAG_NONE;
    damageInfo->procVictim       = PROC_FLAG_NONE;
    damageInfo->procEx           = PROC_EX_NONE;
    damageInfo->hitOutCome       = MELEE_HIT_EVADE;

    if (!this || !pVictim)
        return;
    if (!this->isAlive() || !pVictim->isAlive())
        return;

    // Select HitInfo/procAttacker/procVictim flag based on attack type
    switch (attackType)
    {
        case BASE_ATTACK:
            damageInfo->procAttacker = PROC_FLAG_SUCCESSFUL_MELEE_HIT;
            damageInfo->procVictim   = PROC_FLAG_TAKEN_MELEE_HIT;
            damageInfo->HitInfo      = HITINFO_NORMALSWING2;
            break;
        case OFF_ATTACK:
            damageInfo->procAttacker = PROC_FLAG_SUCCESSFUL_MELEE_HIT | PROC_FLAG_SUCCESSFUL_OFFHAND_HIT;
            damageInfo->procVictim   = PROC_FLAG_TAKEN_MELEE_HIT;//|PROC_FLAG_TAKEN_OFFHAND_HIT // not used
            damageInfo->HitInfo = HITINFO_LEFTSWING;
            break;
        case RANGED_ATTACK:
            damageInfo->procAttacker = PROC_FLAG_SUCCESSFUL_RANGED_HIT;
            damageInfo->procVictim   = PROC_FLAG_TAKEN_RANGED_HIT;
            damageInfo->HitInfo = HITINFO_UNK3;             // test (dev note: test what? HitInfo flag possibly not confirmed.)
            break;
        default:
            break;
    }

    bool immune = true;

    for (uint8 i = 0; i < m_weaponDamageCount[damageInfo->attackType]; i++)
    {
        SubDamageInfo *subDamage = &damageInfo->subDamage[i];

        subDamage->damageSchoolMask = GetTypeId() == TYPEID_PLAYER
            ? GetSchoolMask(GetWeaponDamageSchool(damageInfo->attackType, i))
            : GetMeleeDamageSchoolMask();

        if (damageInfo->target->IsImmuneToDamage(subDamage->damageSchoolMask))
        {
            subDamage->damage = 0;
            continue;
        }
        else
            immune = false;

        subDamage->damage = CalculateDamage(damageInfo->attackType, false, i);
        // Add melee damage bonus
        subDamage->damage = MeleeDamageBonusDone(damageInfo->target, subDamage->damage, damageInfo->attackType, nullptr, DIRECT_DAMAGE, 1, i == 0);
        subDamage->damage = damageInfo->target->MeleeDamageBonusTaken(this, subDamage->damage, damageInfo->attackType, nullptr, DIRECT_DAMAGE, 1, i == 0);

        // Calculate armor reduction
        if(subDamage->damageSchoolMask == SPELL_SCHOOL_MASK_NORMAL)
        {
            damageInfo->cleanDamage += subDamage->damage;
            subDamage->damage = CalcArmorReducedDamage(damageInfo->target, subDamage->damage);
            damageInfo->cleanDamage -= subDamage->damage;
        }

        damageInfo->totalDamage += subDamage->damage;
    }

    // Physical Immune check
    if (immune)
    {
        damageInfo->HitInfo |= HITINFO_NORMALSWING;
        damageInfo->TargetState = VICTIMSTATE_IS_IMMUNE;

        damageInfo->procEx |= PROC_EX_IMMUNE;
        damageInfo->totalDamage = 0;
        damageInfo->cleanDamage = 0;
        return;
    }

    // FIXME: Fix individual school results later when appropriate API is ready
    uint32 mask = SPELL_SCHOOL_MASK_NORMAL;
    for (uint8 i = 0; i < m_weaponDamageCount[damageInfo->attackType]; ++i)
    {
        SubDamageInfo &subDamage = damageInfo->subDamage[i];
        mask |= subDamage.damageSchoolMask;
    }
    damageInfo->hitOutCome = RollMeleeOutcomeAgainst(damageInfo->target, damageInfo->attackType, SpellSchoolMask(mask));

    // Disable parry or dodge for ranged attack
    if (damageInfo->attackType == RANGED_ATTACK)
    {
        if (damageInfo->hitOutCome == MELEE_HIT_PARRY) damageInfo->hitOutCome = MELEE_HIT_NORMAL;
        if (damageInfo->hitOutCome == MELEE_HIT_DODGE) damageInfo->hitOutCome = MELEE_HIT_MISS;
    }

    switch (damageInfo->hitOutCome)
    {
        case MELEE_HIT_EVADE:
        {
            damageInfo->HitInfo |= HITINFO_MISS | HITINFO_SWINGNOHITSOUND;
            damageInfo->TargetState = VICTIMSTATE_EVADES;

            damageInfo->procEx |= PROC_EX_EVADE;
            damageInfo->totalDamage = 0;
            damageInfo->cleanDamage = 0;

            for (uint8 i = 0; i < m_weaponDamageCount[damageInfo->attackType]; i++)
                damageInfo->subDamage[i].damage = 0;

            return;
        }
        case MELEE_HIT_MISS:
        {
            damageInfo->HitInfo |= HITINFO_MISS;
            damageInfo->TargetState = VICTIMSTATE_UNAFFECTED;

            damageInfo->procEx |= PROC_EX_MISS;
            damageInfo->totalDamage = 0;
            damageInfo->cleanDamage = 0;

            for (uint8 i = 0; i < m_weaponDamageCount[damageInfo->attackType]; i++)
                damageInfo->subDamage[i].damage = 0;

            break;
        }
        case MELEE_HIT_NORMAL:
        {
            damageInfo->TargetState = VICTIMSTATE_NORMAL;
            damageInfo->procEx |= PROC_EX_NORMAL_HIT;
            break;
        }
        case MELEE_HIT_CRIT:
        {
            damageInfo->HitInfo |= HITINFO_CRITICALHIT;
            damageInfo->TargetState = VICTIMSTATE_NORMAL;
            damageInfo->procEx |= PROC_EX_CRITICAL_HIT;
            damageInfo->totalDamage = CalculateCritAmount(damageInfo);
            break;
        }
        case MELEE_HIT_PARRY:
        {
            damageInfo->TargetState = VICTIMSTATE_PARRY;
            damageInfo->procEx |= PROC_EX_PARRY;
            damageInfo->cleanDamage += damageInfo->totalDamage;
            damageInfo->totalDamage = 0;

            for (uint8 i = 0; i < m_weaponDamageCount[damageInfo->attackType]; i++)
                damageInfo->subDamage[i].damage = 0;

            break;
        }
        case MELEE_HIT_DODGE:
        {
            damageInfo->TargetState = VICTIMSTATE_DODGE;
            damageInfo->procEx |= PROC_EX_DODGE;
            damageInfo->cleanDamage += damageInfo->totalDamage;
            damageInfo->totalDamage = 0;

            for (uint8 i = 0; i < m_weaponDamageCount[damageInfo->attackType]; i++)
                damageInfo->subDamage[i].damage = 0;

            break;
        }
        case MELEE_HIT_BLOCK:
        {
            damageInfo->TargetState = VICTIMSTATE_NORMAL;
            damageInfo->procEx |= PROC_EX_BLOCK;
            damageInfo->blocked_amount = damageInfo->target->GetShieldBlockValue();

            if (damageInfo->blocked_amount >= damageInfo->subDamage[0].damage)
            {
                damageInfo->TargetState = VICTIMSTATE_BLOCKS;
                damageInfo->blocked_amount = damageInfo->subDamage[0].damage;
            }
            else
                damageInfo->procEx |= PROC_EX_NORMAL_HIT;   // Partial blocks can still cause attacker procs

            damageInfo->totalDamage -= damageInfo->blocked_amount;
            damageInfo->subDamage[0].damage -= damageInfo->blocked_amount;
            damageInfo->cleanDamage += damageInfo->blocked_amount;
            break;
        }
        case MELEE_HIT_GLANCING:
        {
            damageInfo->HitInfo |= HITINFO_GLANCING;
            damageInfo->TargetState = VICTIMSTATE_NORMAL;
            damageInfo->procEx |= PROC_EX_NORMAL_HIT;
            // calculate base values and mods
            float baseLowEnd = 1.3f;
            float baseHighEnd = 1.2f;
            switch (getClass())                             // lowering base values for casters
            {
                case CLASS_SHAMAN:
                case CLASS_PRIEST:
                case CLASS_MAGE:
                case CLASS_WARLOCK:
                case CLASS_DRUID:
                    baseLowEnd  -= 0.7f;
                    baseHighEnd -= 0.3f;
                    break;
            }

            float maxLowEnd = 0.6f;
            switch (getClass())                             // upper for melee classes
            {
                case CLASS_WARRIOR:
                case CLASS_ROGUE:
                    maxLowEnd = 0.91f;                      // If the attacker is a melee class then instead the lower value of 0.91
            }

            // calculate values
            int32 diff = damageInfo->target->GetDefenseSkillValue() - GetWeaponSkillValue(damageInfo->attackType);
            float lowEnd  = baseLowEnd - (0.05f * diff);
            float highEnd = baseHighEnd - (0.03f * diff);

            // apply max/min bounds
            if (lowEnd < 0.01f)                             // the low end must not go bellow 0.01f
                lowEnd = 0.01f;
            else if (lowEnd > maxLowEnd)                    // the smaller value of this and 0.6 is kept as the low end
                lowEnd = maxLowEnd;

            if (highEnd < 0.2f)                             // high end limits
                highEnd = 0.2f;
            if (highEnd > 0.99f)
                highEnd = 0.99f;

            if (lowEnd > highEnd)                           // prevent negative range size
                lowEnd = highEnd;

            float reducePercent = lowEnd + rand_norm_f() * (highEnd - lowEnd);

            damageInfo->cleanDamage += uint32((1.0f - reducePercent) * damageInfo->totalDamage);
            damageInfo->totalDamage = uint32(reducePercent * damageInfo->totalDamage);

            for (uint8 i = 0; i < m_weaponDamageCount[damageInfo->attackType]; i++)
                damageInfo->subDamage[i].damage = uint32(reducePercent * damageInfo->subDamage[i].damage);

            break;
        }
        case MELEE_HIT_CRUSHING:
        {
            damageInfo->HitInfo |= HITINFO_CRUSHING;
            damageInfo->TargetState = VICTIMSTATE_NORMAL;
            damageInfo->procEx |= PROC_EX_NORMAL_HIT;

            // 150% of normal damage
            damageInfo->totalDamage += damageInfo->totalDamage / 2;

            for (uint8 i = 0; i < m_weaponDamageCount[damageInfo->attackType]; i++)
                damageInfo->subDamage[i].damage += damageInfo->subDamage[i].damage / 2;

            break;
        }
        default:
            break;
    }

    // Calculate absorb resist
    if (int32(damageInfo->totalDamage) > 0)
    {
        damageInfo->procVictim |= PROC_FLAG_TAKEN_ANY_DAMAGE;

        // Calculate absorb & resists
        for (uint8 i = 0; i < m_weaponDamageCount[damageInfo->attackType]; i++)
        {
            SubDamageInfo *subDamage = &damageInfo->subDamage[i];

            damageInfo->target->CalculateDamageAbsorbAndResist(this, subDamage->damageSchoolMask, DIRECT_DAMAGE, subDamage->damage, &subDamage->absorb, &subDamage->resist, true);
            damageInfo->totalDamage -= subDamage->absorb + subDamage->resist;
            subDamage->damage -= subDamage->absorb + subDamage->resist;

            if (subDamage->absorb)
            {
                damageInfo->HitInfo |= HITINFO_ABSORB;
                damageInfo->procEx |= PROC_EX_ABSORB;
            }

            if (subDamage->resist)
                damageInfo->HitInfo |= HITINFO_RESIST;
        }
    }
    else
        damageInfo->totalDamage = 0;
}

void Unit::DealMeleeDamage(CalcDamageInfo* damageInfo, bool durabilityLoss)
{
    if (!damageInfo)
        return;

    Unit* pVictim = damageInfo->target;

    if (!this || !pVictim)
        return;

    if (!pVictim->isAlive() || pVictim->IsTaxiFlying() || (pVictim->GetTypeId() == TYPEID_UNIT && ((Creature*)pVictim)->IsInEvadeMode()))
        return;

    // Hmmmm dont like this emotes client must by self do all animations
    if (damageInfo->HitInfo & HITINFO_CRITICALHIT)
        pVictim->HandleEmoteCommand(EMOTE_ONESHOT_WOUNDCRITICAL);
    if (damageInfo->blocked_amount && damageInfo->TargetState != VICTIMSTATE_BLOCKS)
        pVictim->HandleEmoteCommand(EMOTE_ONESHOT_PARRYSHIELD);

    if (damageInfo->TargetState == VICTIMSTATE_PARRY)
    {
        if (pVictim->GetTypeId() != TYPEID_UNIT ||
            !(((Creature*)pVictim)->GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_NO_PARRY_HASTEN))
        {
            // Get attack timers
            float offtime = float(pVictim->getAttackTimer(OFF_ATTACK));
            float basetime = float(pVictim->getAttackTimer(BASE_ATTACK));
            // Reduce attack time
            if (pVictim->haveOffhandWeapon() && offtime < basetime)
            {
                float percent20 = pVictim->GetAttackTime(OFF_ATTACK) * 0.20f;
                float percent60 = 3.0f * percent20;
                if (offtime > percent20 && offtime <= percent60)
                {
                    pVictim->setAttackTimer(OFF_ATTACK, uint32(percent20));
                }
                else if (offtime > percent60)
                {
                    offtime -= 2.0f * percent20;
                    pVictim->setAttackTimer(OFF_ATTACK, uint32(offtime));
                }
            }
            else
            {
                float percent20 = pVictim->GetAttackTime(BASE_ATTACK) * 0.20f;
                float percent60 = 3.0f * percent20;
                if (basetime > percent20 && basetime <= percent60)
                {
                    pVictim->setAttackTimer(BASE_ATTACK, uint32(percent20));
                }
                else if (basetime > percent60)
                {
                    basetime -= 2.0f * percent20;
                    pVictim->setAttackTimer(BASE_ATTACK, uint32(basetime));
                }
            }
        }
    }

    // Call default DealDamage
    CleanDamage cleanDamage(damageInfo->cleanDamage, damageInfo->attackType, damageInfo->hitOutCome);
    DealDamage(pVictim, damageInfo->totalDamage, &cleanDamage, DIRECT_DAMAGE, SpellSchoolMask(damageInfo->subDamage[0].damageSchoolMask), nullptr, durabilityLoss);

    // If this attack by an NPC dealt some damage from behind to a player, it has a chance to daze victim
    if (damageInfo->totalDamage && CanDazeInCombat(pVictim) && roll_chance_f(CalculateEffectiveDazeChance(pVictim, damageInfo->attackType)))
        CastSpell(pVictim, 1604, TRIGGERED_OLD_TRIGGERED);

    // update at damage Judgement aura duration that applied by attacker at victim
    if (damageInfo->totalDamage)
    {
        SpellAuraHolderMap const& vAuras = pVictim->GetSpellAuraHolderMap();
        for (SpellAuraHolderMap::const_iterator itr = vAuras.begin(); itr != vAuras.end(); ++itr)
        {
            SpellEntry const* spellInfo = (*itr).second->GetSpellProto();
            if (spellInfo->HasAttribute(SPELL_ATTR_EX3_CANT_MISS) && spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN && ((*itr).second->GetCasterGuid() == GetObjectGuid()))
                (*itr).second->RefreshHolder();
        }
    }

    // If not miss
    if (!(damageInfo->HitInfo & HITINFO_MISS))
    {
        // on weapon hit casts
        if (GetTypeId() == TYPEID_PLAYER && pVictim->isAlive())
            ((Player*)this)->CastItemCombatSpell(pVictim, damageInfo->attackType);

        // If not immune
        if (damageInfo->TargetState != VICTIMSTATE_IS_IMMUNE)
        {
            // victim's damage shield
            std::set<Aura*> alreadyDone;
            AuraList const& vDamageShields = pVictim->GetAurasByType(SPELL_AURA_DAMAGE_SHIELD);
            for (AuraList::const_iterator i = vDamageShields.begin(); i != vDamageShields.end();)
            {
                if (alreadyDone.find(*i) == alreadyDone.end())
                {
                    alreadyDone.insert(*i);
                    uint32 damage = (*i)->GetModifier()->m_amount;
                    SpellEntry const* i_spellProto = (*i)->GetSpellProto();
                    // Calculate absorb resist ??? no data in opcode for this possibly unable to absorb or resist?
                    // uint32 absorb;
                    // uint32 resist;
                    // CalcAbsorbResist(pVictim, SpellSchools(spellProto->School), SPELL_DIRECT_DAMAGE, damage, &absorb, &resist);
                    // damage-=absorb + resist;

                    pVictim->DealDamageMods(this, damage, nullptr, SPELL_DIRECT_DAMAGE, i_spellProto);

                    WorldPacket data(SMSG_SPELLDAMAGESHIELD, (8 + 8 + 4 + 4));
                    data << pVictim->GetObjectGuid();
                    data << GetObjectGuid();
                    data << uint32(damage);
                    data << uint32(i_spellProto->School);
                    pVictim->SendMessageToSet(data, true);

                    pVictim->DealDamage(this, damage, nullptr, SPELL_DIRECT_DAMAGE, GetSpellSchoolMask(i_spellProto), i_spellProto, true);

                    i = vDamageShields.begin();
                }
                else
                    ++i;
            }
        }
    }
}

void Unit::HandleEmoteCommand(uint32 emote_id)
{
    WorldPacket data(SMSG_EMOTE, 4 + 8);
    data << uint32(emote_id);
    data << GetObjectGuid();
    SendMessageToSet(data, true);
}

void Unit::HandleEmoteState(uint32 emote_id)
{
    SetUInt32Value(UNIT_NPC_EMOTESTATE, emote_id);
}

void Unit::HandleEmote(uint32 emote_id)
{
    if (!emote_id)
        HandleEmoteState(0);
    else if (EmotesEntry const* emoteEntry = sEmotesStore.LookupEntry(emote_id))
    {
        if (emoteEntry->EmoteType)                          // 1,2 states, 0 command
            HandleEmoteState(emote_id);
        else
            HandleEmoteCommand(emote_id);
    }
}

uint32 Unit::CalcArmorReducedDamage(Unit* pVictim, const uint32 damage)
{
    float armor = (float)pVictim->GetArmor();

    // Ignore enemy armor by armor penetration
    armor -= GetResistancePenetration(SPELL_SCHOOL_NORMAL);

    if (armor < 0.0f)
        armor = 0.0f;

    float levelModifier = (float)getLevel();

    float tmpvalue = 0.1f * armor / (8.5f * levelModifier + 40);
    tmpvalue = tmpvalue / (1.0f + tmpvalue);

    if (tmpvalue < 0.0f)
        tmpvalue = 0.0f;
    if (tmpvalue > 0.75f)
        tmpvalue = 0.75f;

    uint32 newdamage = uint32(damage - (damage * tmpvalue));

    return (newdamage > 1) ? newdamage : 1;
}

void Unit::CalculateDamageAbsorbAndResist(Unit* pCaster, SpellSchoolMask schoolMask, DamageEffectType damagetype, const uint32 damage, uint32* absorb, uint32* resist, bool canReflect, bool ignoreResists, bool binary)
{
    if (!pCaster || !isAlive() || !damage)
        return;

    // Magic damage, check for resists
    if (!ignoreResists && (schoolMask & SPELL_SCHOOL_MASK_MAGIC) && (!binary || damagetype == DOT))
    {
        const float percent = CalculateEffectiveMagicResistancePercent(pCaster, schoolMask);
        const SpellPartialResistChanceEntry &chances = SPELL_PARTIAL_RESIST_DISTRIBUTION.at(uint32(percent * 100));
        // We choose which portion of damage is resisted below, none by default
        uint8 portion = SPELL_PARTIAL_RESIST_NONE;
        // If we got to this point, we already rolled for full resist on hit
        // We do a roll between remaining chances
        const uint8 outcomes = (NUM_SPELL_PARTIAL_RESISTS - 1);
        const uint32 roll = urand(1, (10000 - chances.at(SPELL_PARTIAL_RESIST_PCT_100)));
        Die<SpellPartialResist, SPELL_PARTIAL_RESIST_NONE, outcomes> die;
        for (uint8 outcome = SPELL_PARTIAL_RESIST_NONE; outcome < outcomes; ++outcome)
            die.chance[outcome] = chances.at(outcome);
        portion = die.roll(roll);
        const uint32 amount = uint32(damage * (portion * (1.0f / float(outcomes))));
        // We already rolled for full resist on hit, so we need to deal at least *some* amount of damage...
        *resist = (amount >= damage) ? (damage - 1) : amount;
    }
    else
        *resist = 0;

    int32 RemainingDamage = damage - *resist;

    // full absorb cases (by chance)
    /* none cases, but preserve for better backporting conflict resolve
    AuraList const& vAbsorb = GetAurasByType(SPELL_AURA_SCHOOL_ABSORB);
    for(AuraList::const_iterator i = vAbsorb.begin(); i != vAbsorb.end() && RemainingDamage > 0; ++i)
    {
        // only work with proper school mask damage
        Modifier* i_mod = (*i)->GetModifier();
        if (!(i_mod->m_miscvalue & schoolMask))
            continue;

        SpellEntry const* i_spellProto = (*i)->GetSpellProto();
    }
    */

    // Need remove expired auras after
    bool existExpired = false;

    // absorb without mana cost
    AuraList const& vSchoolAbsorb = GetAurasByType(SPELL_AURA_SCHOOL_ABSORB);
    for (AuraList::const_iterator i = vSchoolAbsorb.begin(); i != vSchoolAbsorb.end() && RemainingDamage > 0; ++i)
    {
        Modifier* mod = (*i)->GetModifier();
        if (!(mod->m_miscvalue & schoolMask))
            continue;

        SpellEntry const* spellProto = (*i)->GetSpellProto();

        // Max Amount can be absorbed by this aura
        int32  currentAbsorb = mod->m_amount;

        // Found empty aura (impossible but..)
        if (currentAbsorb <= 0)
        {
            existExpired = true;
            continue;
        }

        // currentAbsorb - damage can be absorbed by shield
        // If need absorb less damage
        if (RemainingDamage < currentAbsorb)
            currentAbsorb = RemainingDamage;

        RemainingDamage -= currentAbsorb;

        // Reduce shield amount
        mod->m_amount -= currentAbsorb;
        if ((*i)->GetHolder()->DropAuraCharge())
            mod->m_amount = 0;
        // Need remove it later
        if (mod->m_amount <= 0)
            existExpired = true;
    }

    // Remove all expired absorb auras
    if (existExpired)
    {
        for (AuraList::const_iterator i = vSchoolAbsorb.begin(); i != vSchoolAbsorb.end();)
        {
            if ((*i)->GetModifier()->m_amount <= 0)
            {
                RemoveAurasDueToSpell((*i)->GetId(), nullptr, AURA_REMOVE_BY_SHIELD_BREAK);
                i = vSchoolAbsorb.begin();
            }
            else
                ++i;
        }
    }

    // absorb by mana cost
    AuraList const& vManaShield = GetAurasByType(SPELL_AURA_MANA_SHIELD);
    for (AuraList::const_iterator i = vManaShield.begin(), next; i != vManaShield.end() && RemainingDamage > 0; i = next)
    {
        next = i; ++next;

        // check damage school mask
        if (((*i)->GetModifier()->m_miscvalue & schoolMask) == 0)
            continue;

        int32 currentAbsorb;
        if (RemainingDamage >= (*i)->GetModifier()->m_amount)
            currentAbsorb = (*i)->GetModifier()->m_amount;
        else
            currentAbsorb = RemainingDamage;

        if (float manaMultiplier = (*i)->GetSpellProto()->EffectMultipleValue[(*i)->GetEffIndex()])
        {
            if (Player* modOwner = GetSpellModOwner())
                modOwner->ApplySpellMod((*i)->GetId(), SPELLMOD_MULTIPLE_VALUE, manaMultiplier);

            int32 maxAbsorb = int32(GetPower(POWER_MANA) / manaMultiplier);
            if (currentAbsorb > maxAbsorb)
                currentAbsorb = maxAbsorb;

            int32 manaReduction = int32(currentAbsorb * manaMultiplier);
            ApplyPowerMod(POWER_MANA, manaReduction, false);
        }

        (*i)->GetModifier()->m_amount -= currentAbsorb;
        if ((*i)->GetModifier()->m_amount <= 0)
        {
            RemoveAurasDueToSpell((*i)->GetId());
            next = vManaShield.begin();
        }

        RemainingDamage -= currentAbsorb;
    }

    // only split damage if not damaging yourself
    if (pCaster != this)
    {
        AuraList const& vSplitDamageFlat = GetAurasByType(SPELL_AURA_SPLIT_DAMAGE_FLAT);
        for (AuraList::const_iterator i = vSplitDamageFlat.begin(), next; i != vSplitDamageFlat.end() && RemainingDamage >= 0; i = next)
        {
            next = i; ++next;

            // check damage school mask
            if (((*i)->GetModifier()->m_miscvalue & schoolMask) == 0)
                continue;

            // Damage can be splitted only if aura has an alive caster
            Unit* caster = (*i)->GetCaster();
            if (!caster || caster == this || !caster->IsInWorld() || !caster->isAlive())
                continue;

            int32 currentAbsorb;
            if (RemainingDamage >= (*i)->GetModifier()->m_amount)
                currentAbsorb = (*i)->GetModifier()->m_amount;
            else
                currentAbsorb = RemainingDamage;

            RemainingDamage -= currentAbsorb;

            if (caster->IsImmuneToDamage(schoolMask))
            {
                SendSpellMiss(caster, (*i)->GetSpellProto()->Id, SPELL_MISS_IMMUNE);
                return;
            }

            uint32 splitted = currentAbsorb;
            uint32 splitted_absorb = 0;
            pCaster->DealDamageMods(caster, splitted, &splitted_absorb, DIRECT_DAMAGE, (*i)->GetSpellProto());

            pCaster->SendSpellNonMeleeDamageLog(caster, (*i)->GetSpellProto()->Id, splitted, schoolMask, splitted_absorb, 0, false, 0, false);

            CleanDamage cleanDamage = CleanDamage(splitted, BASE_ATTACK, MELEE_HIT_NORMAL);
            pCaster->DealDamage(caster, splitted, &cleanDamage, DIRECT_DAMAGE, schoolMask, (*i)->GetSpellProto(), false);
        }

        AuraList const& vSplitDamagePct = GetAurasByType(SPELL_AURA_SPLIT_DAMAGE_PCT);
        for (AuraList::const_iterator i = vSplitDamagePct.begin(), next; i != vSplitDamagePct.end() && RemainingDamage >= 0; i = next)
        {
            next = i; ++next;

            // check damage school mask
            if (((*i)->GetModifier()->m_miscvalue & schoolMask) == 0)
                continue;

            // Damage can be splitted only if aura has an alive caster
            Unit* caster = (*i)->GetCaster();
            if (!caster || caster == this || !caster->IsInWorld() || !caster->isAlive())
                continue;

            uint32 splitted = uint32(RemainingDamage * (*i)->GetModifier()->m_amount / 100.0f);

            RemainingDamage -=  int32(splitted);

            if (caster->IsImmuneToDamage(schoolMask))
            {
                SendSpellMiss(caster, (*i)->GetSpellProto()->Id, SPELL_MISS_IMMUNE);
                return;
            }

            uint32 split_absorb = 0;
            pCaster->DealDamageMods(caster, splitted, &split_absorb, DIRECT_DAMAGE, (*i)->GetSpellProto());

            pCaster->SendSpellNonMeleeDamageLog(caster, (*i)->GetSpellProto()->Id, splitted, schoolMask, split_absorb, 0, false, 0, false);

            CleanDamage cleanDamage = CleanDamage(splitted, BASE_ATTACK, MELEE_HIT_NORMAL);
            pCaster->DealDamage(caster, splitted, &cleanDamage, DIRECT_DAMAGE, schoolMask, (*i)->GetSpellProto(), false);
        }
    }

    *absorb = damage - RemainingDamage - *resist;
}

void Unit::CalculateAbsorbResistBlock(Unit* pCaster, SpellNonMeleeDamage* damageInfo, SpellEntry const* spellProto, WeaponAttackType attType)
{
    if (RollAbilityPartialBlockOutcome(pCaster, attType, spellProto))
    {
        damageInfo->blocked = std::min(GetShieldBlockValue(), damageInfo->damage);
        damageInfo->damage -= damageInfo->blocked;
    }

    CalculateDamageAbsorbAndResist(pCaster, GetSpellSchoolMask(spellProto), SPELL_DIRECT_DAMAGE, damageInfo->damage, &damageInfo->absorb, &damageInfo->resist, IsReflectableSpell(spellProto), spellProto->HasAttribute(SPELL_ATTR_EX4_IGNORE_RESISTANCES), IsBinarySpell(spellProto));
    damageInfo->damage -= damageInfo->absorb + damageInfo->resist;
}

void Unit::AttackerStateUpdate(Unit* pVictim, WeaponAttackType attType, bool extra)
{
    if (hasUnitState(UNIT_STAT_CAN_NOT_REACT) || HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED))
        return;

    if (!pVictim->isAlive())
        return;

    if (IsNonMeleeSpellCasted(false))
        return;

    if (attType == RANGED_ATTACK)
        return;                                             // ignore ranged case

    uint32 extraAttacks = m_extraAttacks;

    // melee attack spell casted at main hand attack only
    if (attType == BASE_ATTACK && m_currentSpells[CURRENT_MELEE_SPELL])
    {
        m_currentSpells[CURRENT_MELEE_SPELL]->cast();
        return;
    }

    RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_MELEE_ATTACK);
    CalcDamageInfo damageInfo;
    CalculateMeleeDamage(pVictim, &damageInfo, attType);

    // Send log damage message to client
    for (uint8 i = 0; i < m_weaponDamageCount[attType]; i++)
    {
        damageInfo.totalDamage -= damageInfo.subDamage[i].damage;
        DealDamageMods(pVictim, damageInfo.subDamage[i].damage, &damageInfo.subDamage[i].absorb, DIRECT_DAMAGE);
        damageInfo.totalDamage += damageInfo.subDamage[i].damage;
    }

    SendAttackStateUpdate(&damageInfo);
    ProcDamageAndSpell(damageInfo.target, damageInfo.procAttacker, damageInfo.procVictim, damageInfo.procEx, damageInfo.totalDamage, damageInfo.attackType);
    DealMeleeDamage(&damageInfo, true);

    uint32 totalAbsorb = 0;
    uint32 totalResist = 0;

    for (uint8 i = 0; i < m_weaponDamageCount[attType]; i++)
    {
        totalAbsorb += damageInfo.subDamage[i].absorb;
        totalResist += damageInfo.subDamage[i].resist;
    }

    if (GetTypeId() == TYPEID_PLAYER)
        DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "AttackerStateUpdate: (Player) %u attacked %u (TypeId: %u) for %u dmg, absorbed %u, blocked %u, resisted %u.",
                         GetGUIDLow(), pVictim->GetGUIDLow(), pVictim->GetTypeId(), damageInfo.totalDamage, totalAbsorb, damageInfo.blocked_amount, totalResist);
    else
        DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "AttackerStateUpdate: (NPC)    %u attacked %u (TypeId: %u) for %u dmg, absorbed %u, blocked %u, resisted %u.",
                         GetGUIDLow(), pVictim->GetGUIDLow(), pVictim->GetTypeId(), damageInfo.totalDamage, totalAbsorb, damageInfo.blocked_amount, totalResist);

    if (Unit* owner = GetOwner())
        if (owner->GetTypeId() == TYPEID_UNIT)
        {
            owner->SetInCombatWith(pVictim);
            owner->AddThreat(pVictim);
            pVictim->SetInCombatWith(owner);
        }

    for (GuidSet::const_iterator itr = m_guardianPets.begin(); itr != m_guardianPets.end(); ++itr)
        if (Unit* pet = (Unit*)GetMap()->GetPet(*itr))
        {
            pet->SetInCombatWith(pVictim);
            pet->AddThreat(pVictim);
            pVictim->SetInCombatWith(pet);
        }

    // if damage pVictim call AI reaction
    pVictim->AttackedBy(this);
}

void Unit::DoExtraAttacks(Unit* pVictim)
{
    while (m_extraAttacks)
    {
        AttackerStateUpdate(pVictim, BASE_ATTACK, true);
        if (m_extraAttacks > 0)
            --m_extraAttacks;
    }
}

MeleeHitOutcome Unit::RollMeleeOutcomeAgainst(const Unit* pVictim, WeaponAttackType attType, SpellSchoolMask schoolMask) const
{
    if (pVictim->GetTypeId() == TYPEID_UNIT && ((Creature*)pVictim)->IsInEvadeMode())
        return MELEE_HIT_EVADE;

    Die<UnitCombatDieSide, UNIT_COMBAT_DIE_HIT, NUM_UNIT_COMBAT_DIE_SIDES> die;
    die.set(UNIT_COMBAT_DIE_MISS, CalculateEffectiveMissChance(pVictim, attType));
    if (pVictim->GetTypeId() == TYPEID_PLAYER && !pVictim->IsStandState() && CanCrit(attType))
    {
        die.chance[UNIT_COMBAT_DIE_CRIT] = 10000;
        DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "RollMeleeOutcomeAgainst: New attack die: [MISS:%u, CRIT:%u] (target is sitting)",
                         die.chance[UNIT_COMBAT_DIE_MISS], die.chance[UNIT_COMBAT_DIE_CRIT]);
    }
    else
    {
        if (pVictim->CanReactInCombat())
        {
            if (pVictim->CanDodgeInCombat(this))
                die.set(UNIT_COMBAT_DIE_DODGE, pVictim->CalculateEffectiveDodgeChance(this, attType));
            if (pVictim->CanParryInCombat(this))
                die.set(UNIT_COMBAT_DIE_PARRY, pVictim->CalculateEffectiveParryChance(this, attType));
            if (pVictim->CanBlockInCombat(this, schoolMask))
                die.set(UNIT_COMBAT_DIE_BLOCK, pVictim->CalculateEffectiveBlockChance(this, attType));
        }
        if (CanGlanceInCombat(pVictim))
            die.set(UNIT_COMBAT_DIE_GLANCE, CalculateEffectiveGlanceChance(pVictim, attType));
        die.set(UNIT_COMBAT_DIE_CRIT, CalculateEffectiveCritChance(pVictim, attType));
        if (CanCrushInCombat(pVictim))
            die.set(UNIT_COMBAT_DIE_CRUSH, CalculateEffectiveCrushChance(pVictim, attType));
        DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "RollMeleeOutcomeAgainst: New attack die: [MISS:%u, DODGE:%u, PARRY:%u, BLOCK:%u, GLANCE:%u, CRIT:%u, CRUSH:%u]",
                         die.chance[UNIT_COMBAT_DIE_MISS], die.chance[UNIT_COMBAT_DIE_DODGE], die.chance[UNIT_COMBAT_DIE_PARRY], die.chance[UNIT_COMBAT_DIE_BLOCK],
                         die.chance[UNIT_COMBAT_DIE_GLANCE], die.chance[UNIT_COMBAT_DIE_CRIT], die.chance[UNIT_COMBAT_DIE_CRUSH]);
    }

    const uint32 random = urand(1, 10000);
    const UnitCombatDieSide side = die.roll(random);
    if (side != UNIT_COMBAT_DIE_HIT)
        DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "RollMeleeOutcomeAgainst: Rolled %u, result: %s (chance %u)", random, UnitCombatDieSideText(side), die.chance[side]);
    switch (uint32(side))
    {
        case UNIT_COMBAT_DIE_MISS:   return MELEE_HIT_MISS;
        case UNIT_COMBAT_DIE_DODGE:  return MELEE_HIT_DODGE;
        case UNIT_COMBAT_DIE_PARRY:  return MELEE_HIT_PARRY;
        case UNIT_COMBAT_DIE_BLOCK:  return MELEE_HIT_BLOCK;
        case UNIT_COMBAT_DIE_GLANCE: return MELEE_HIT_GLANCING;
        case UNIT_COMBAT_DIE_CRIT:   return MELEE_HIT_CRIT;
        case UNIT_COMBAT_DIE_CRUSH:  return MELEE_HIT_CRUSHING;
    }
    DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "RollMeleeOutcomeAgainst: Rolled %u, result: HIT", random);
    return MELEE_HIT_NORMAL;
}

uint32 Unit::CalculateDamage(WeaponAttackType attType, bool normalized, uint8 index)
{
    float min_damage, max_damage;

    if (GetTypeId() != TYPEID_PLAYER && index != 0)
        return 0;

    if ((normalized || index != 0) && GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->CalculateMinMaxDamage(attType, normalized, min_damage, max_damage, index);
    else
    {
        switch (attType)
        {
            case RANGED_ATTACK:
                min_damage = GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE);
                max_damage = GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE);
                break;
            case BASE_ATTACK:
                min_damage = GetFloatValue(UNIT_FIELD_MINDAMAGE);
                max_damage = GetFloatValue(UNIT_FIELD_MAXDAMAGE);
                break;
            case OFF_ATTACK:
                min_damage = GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE);
                max_damage = GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE);
                break;
            // Just for good manner
            default:
                min_damage = 0.0f;
                max_damage = 0.0f;
                break;
        }
    }

    if (min_damage > max_damage)
    {
        std::swap(min_damage, max_damage);
    }

    if (max_damage == 0.0f)
        max_damage = 5.0f;

    return urand((uint32)min_damage, (uint32)max_damage);
}

float Unit::CalculateLevelPenalty(SpellEntry const* spellProto) const
{
    return spellProto->spellLevel < 20 && spellProto->spellLevel > 0 ? (1.0f - ((20.0f - spellProto->spellLevel) * 0.0375f)) : 1.0f;
}

void Unit::SendMeleeAttackStart(Unit* pVictim) const
{
    WorldPacket data(SMSG_ATTACKSTART, 8 + 8);
    data << GetObjectGuid();
    data << pVictim->GetObjectGuid();

    SendMessageToSet(data, true);
    DEBUG_LOG("WORLD: Sent SMSG_ATTACKSTART");
}

void Unit::SendMeleeAttackStop(Unit* victim) const
{
    if (!victim)
        return;

    WorldPacket data(SMSG_ATTACKSTOP, (4 + 16));            // we guess size
    data << GetPackGUID();
    data << victim->GetPackGUID();                          // can be 0x00...
    data << uint32(0);                                      // can be 0x1
    SendMessageToSet(data, true);
    DETAIL_FILTER_LOG(LOG_FILTER_COMBAT, "%s %u stopped attacking %s %u", (GetTypeId() == TYPEID_PLAYER ? "player" : "creature"), GetGUIDLow(), (victim->GetTypeId() == TYPEID_PLAYER ? "player" : "creature"), victim->GetGUIDLow());

    /*if(victim->GetTypeId() == TYPEID_UNIT)
    ((Creature*)victim)->AI().EnterEvadeMode(this);*/
}

SpellMissInfo Unit::MeleeSpellHitResult(Unit* pVictim, SpellEntry const* spell)
{
    Die<UnitCombatDieSide, UNIT_COMBAT_DIE_HIT, NUM_UNIT_COMBAT_DIE_SIDES> die;
    die.set(UNIT_COMBAT_DIE_MISS, CalculateSpellMissChance(pVictim, SPELL_SCHOOL_MASK_NORMAL, spell));
    die.set(UNIT_COMBAT_DIE_RESIST, CalculateSpellResistChance(pVictim, SPELL_SCHOOL_MASK_NORMAL, spell));
    if (pVictim->CanReactOnAbility(spell))
    {
        const WeaponAttackType attackType = GetWeaponAttackType(spell);
        if (pVictim->CanDodgeAbility(this, spell))
           die.set(UNIT_COMBAT_DIE_DODGE, pVictim->CalculateEffectiveDodgeChance(this, attackType, spell));
        if (pVictim->CanParryAbility(this, spell))
           die.set(UNIT_COMBAT_DIE_PARRY, pVictim->CalculateEffectiveParryChance(this, attackType, spell));
        if (pVictim->CanDeflectAbility(this, spell))
           die.set(UNIT_COMBAT_DIE_DEFLECT, pVictim->CalculateAbilityDeflectChance(this, spell));
        if (pVictim->CanBlockAbility(this, spell, true))
           die.set(UNIT_COMBAT_DIE_BLOCK, pVictim->CalculateEffectiveBlockChance(this, attackType, spell));
    }
    DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "MeleeSpellHitResult: New ability hit die: %u [MISS:%u, RESIST:%u, DODGE:%u, PARRY:%u, DEFLECT:%u, BLOCK:%u]", spell->Id,
                     die.chance[UNIT_COMBAT_DIE_MISS], die.chance[UNIT_COMBAT_DIE_RESIST], die.chance[UNIT_COMBAT_DIE_DODGE],
                     die.chance[UNIT_COMBAT_DIE_PARRY], die.chance[UNIT_COMBAT_DIE_DEFLECT], die.chance[UNIT_COMBAT_DIE_BLOCK]);
    const uint32 random = urand(1, 10000);
    const UnitCombatDieSide side = die.roll(random);
    if (side != UNIT_COMBAT_DIE_HIT)
        DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "MeleeSpellHitResult: Rolled %u, result: %s (chance %u)", random, UnitCombatDieSideText(side), die.chance[side]);
    switch (uint32(side))
    {
        case UNIT_COMBAT_DIE_MISS:      return SPELL_MISS_MISS;
        case UNIT_COMBAT_DIE_RESIST:    return SPELL_MISS_RESIST;
        case UNIT_COMBAT_DIE_DODGE:     return SPELL_MISS_DODGE;
        case UNIT_COMBAT_DIE_PARRY:     return SPELL_MISS_PARRY;
        case UNIT_COMBAT_DIE_DEFLECT:   return SPELL_MISS_DEFLECT;
        case UNIT_COMBAT_DIE_BLOCK:     return SPELL_MISS_BLOCK;
    }
    DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "MeleeSpellHitResult: Rolled %u, result: HIT", random);
    return SPELL_MISS_NONE;
}

SpellMissInfo Unit::MagicSpellHitResult(Unit* pVictim, SpellEntry const* spell, SpellSchoolMask schoolMask)
{
    Die<UnitCombatDieSide, UNIT_COMBAT_DIE_HIT, NUM_UNIT_COMBAT_DIE_SIDES> die;
    die.set(UNIT_COMBAT_DIE_MISS, CalculateSpellMissChance(pVictim, schoolMask, spell));
    die.set(UNIT_COMBAT_DIE_RESIST, CalculateSpellResistChance(pVictim, schoolMask, spell));
    /* Deflect for spells is currently unused up until WotLK, commented out for performance
    if (pVictim->CanDeflectAbility(this, spell))
       die.set(UNIT_COMBAT_DIE_DEFLECT, pVictim->CalculateAbilityDeflectChance(this, spell));
    */
    DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "MagicSpellHitResult: New spell hit die: %u [MISS:%u, RESIST:%u, DEFLECT:%u]", spell->Id,
                     die.chance[UNIT_COMBAT_DIE_MISS], die.chance[UNIT_COMBAT_DIE_RESIST], die.chance[UNIT_COMBAT_DIE_DEFLECT]);
    const uint32 random = urand(1, 10000);
    const UnitCombatDieSide side = die.roll(random);
    if (side != UNIT_COMBAT_DIE_HIT)
        DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "MagicSpellHitResult: Rolled %u, result: %s (chance %u)", random, UnitCombatDieSideText(side), die.chance[side]);
    switch (uint32(side))
    {
        case UNIT_COMBAT_DIE_MISS:      // Shows up as "Resist" up until WotLK
        case UNIT_COMBAT_DIE_RESIST:    return SPELL_MISS_RESIST;
        case UNIT_COMBAT_DIE_DEFLECT:   return SPELL_MISS_DEFLECT;
    }
    DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "MagicSpellHitResult: Rolled %u, result: HIT", random);
    return SPELL_MISS_NONE;
}

SpellMissInfo Unit::SpellHitResult(Unit* pVictim, SpellEntry const* spell, bool reflectable)
{
    // Dead units can't be missed, can't resist, reflect, etc
    if (!pVictim->isAlive())
        return SPELL_MISS_NONE;
    // Return evade for units in evade mode
    if (pVictim->GetTypeId() == TYPEID_UNIT && ((Creature*)pVictim)->IsInEvadeMode())
        return SPELL_MISS_EVADE;
    // All positive spells can`t miss
    // TODO: client not show miss log for this spells - so need find info for this in dbc and use it!
    if (IsPositiveSpell(spell, this, pVictim))
        return SPELL_MISS_NONE;
    SpellSchoolMask schoolMask = GetSpellSchoolMask(spell);
    // wand case
    bool wand = spell->Id == 5019;
    if (wand && !!(getClassMask() & CLASSMASK_WAND_USERS) && GetTypeId() == TYPEID_PLAYER)
        schoolMask = GetSchoolMask(GetWeaponDamageSchool(RANGED_ATTACK));
    // Reflect (when available)
    if (reflectable)
    {
        const float chance = pVictim->GetReflectChance(schoolMask);
        if (roll_chance_combat(chance))
            return SPELL_MISS_REFLECT;
    }
    if (!spell->HasAttribute(SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY))
    {
        // TODO: improve for partial application
        // Check for immune
        if (!wand && pVictim->IsImmuneToSpell(spell, (this == pVictim)))
            return SPELL_MISS_IMMUNE;
        // Check for immune (use charges)
        if (pVictim->IsImmuneToDamage(schoolMask))
            return SPELL_MISS_IMMUNE;
    }
    switch (spell->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MELEE:
        case SPELL_DAMAGE_CLASS_RANGED:
            return MeleeSpellHitResult(pVictim, spell);
        case SPELL_DAMAGE_CLASS_MAGIC:
            return MagicSpellHitResult(pVictim, spell, schoolMask);
        case SPELL_DAMAGE_CLASS_NONE:
            // Usually never misses, but needs more research for some spells
            break;
    }
    return SPELL_MISS_NONE;
}

uint32 Unit::GetDefenseSkillValue(Unit const* target) const
{
    if (GetTypeId() == TYPEID_PLAYER)
    {
        // in PvP always use full skill (and bonuses on top of it if present)
        uint32 value = (target && target->GetTypeId() == TYPEID_PLAYER)
                       ? std::max(((Player*)this)->GetMaxSkillValue(SKILL_DEFENSE), ((Player*)this)->GetSkillValue(SKILL_DEFENSE))
                       : ((Player*)this)->GetSkillValue(SKILL_DEFENSE);
        return value;
    }
    else
        return GetUnitMeleeSkill(target);
}

bool Unit::CanCrush() const
{
    // Generally, only npcs and npc-controlled players/units are eligible to deal crushing blows
    if (GetTypeId() == TYPEID_PLAYER || HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE))
        return GetCharmerGuid().IsCreature();
    return !GetCharmerOrOwnerGuid().IsPlayer();
}

bool Unit::CanGlance() const
{
    // Generally, only players and player-controlled units are eligible to deal glancing blows
    if (GetTypeId() == TYPEID_PLAYER || HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE))
        return !GetCharmerGuid().IsCreature();
    return false;
}

bool Unit::CanDaze() const
{
    // Generally, only npcs are able to daze targets in melee
    return (GetTypeId() == TYPEID_UNIT && !HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE));
}

void Unit::SetCanDodge(const bool flag)
{
    if (m_canDodge == flag)
        return;

    m_canDodge = flag;
    if (GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->UpdateDodgePercentage();
}

void Unit::SetCanParry(const bool flag)
{
    if (m_canParry == flag)
        return;

    m_canParry = flag;
    if (GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->UpdateParryPercentage();
}

void Unit::SetCanBlock(const bool flag)
{
    if (m_canBlock == flag)
        return;

    m_canBlock = flag;
    if (GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->UpdateBlockPercentage();
}

bool Unit::CanDodgeInCombat() const
{
    if (!CanDodge() || !CanReactInCombat())
        return false;
    // Own chance appears to be zero / below zero / unmeaningful for some reason (debuffs?): unit is incapable
    if (GetDodgeChance() < 0.005f)
        return false;
     return true;
}

bool Unit::CanDodgeInCombat(const Unit *attacker) const
{
    if (!attacker || !CanDodgeInCombat())
        return false;
    // Players can't dodge attacks from behind
    if (GetTypeId() == TYPEID_PLAYER)
        return attacker->IsFacingTargetsFront(this);
    return true;
}

bool Unit::CanParryInCombat() const
{
    if (!CanParry() || !CanReactInCombat() || HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED) || IsNonMeleeSpellCasted(false))
        return false;
    // Own chance appears to be zero / below zero / unmeaningful for some reason (debuffs?): unit is incapable
    if (GetParryChance() < 0.005f)
        return false;
    // Players need a melee weapon they can use to parry an attack
    if (GetTypeId() == TYPEID_PLAYER)
    {
        const Player* player = (const Player*)this;
        return (player->GetWeaponForAttack(BASE_ATTACK, true, true) || player->GetWeaponForAttack(OFF_ATTACK, true, true));
    }
    return true;
}

bool Unit::CanParryInCombat(const Unit *attacker) const
{
    return (attacker && CanParryInCombat() && attacker->IsFacingTargetsFront(this));
}

bool Unit::CanBlockInCombat(SpellSchoolMask weaponSchoolMask) const
{
    // Pre-WotLK: cannot block elemental and magic melee auto-attacks
    if (weaponSchoolMask & SPELL_SCHOOL_MASK_MAGIC)
        return false;
    if (!CanBlock() || !CanReactInCombat() || IsNonMeleeSpellCasted(false))
        return false;
    // Own chance appears to be zero / below zero / unmeaningful for some reason (debuffs?): unit is incapable
    if (GetBlockChance() < 0.005f)
        return false;
    // Players need a shield equipped to block an attack
    if (GetTypeId() == TYPEID_PLAYER)
    {
        const Player* player = (const Player*)this;
        if (player->CanUseEquippedWeapon(OFF_ATTACK))
        {
            const Item* offhand = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
            return (offhand && !offhand->IsBroken() && offhand->GetProto()->Block);
        }
        return false;
    }
    return true;
}

bool Unit::CanBlockInCombat(const Unit *attacker, SpellSchoolMask weaponSchoolMask) const
{
    return (attacker && CanBlockInCombat(weaponSchoolMask) && attacker->IsFacingTargetsFront(this));
}

bool Unit::CanCrushInCombat() const
{
    if (!CanCrush())
        return false;
    if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_NO_CRUSH)
        return false;
    return true;
}

bool Unit::CanCrushInCombat(const Unit *victim) const
{
    return (victim && CanCrushInCombat() && victim->GetCharmerOrOwnerOrOwnGuid().IsPlayer());
}

bool Unit::CanGlanceInCombat(const Unit *victim) const
{
    return (victim && CanGlanceInCombat() && victim->GetCharmerOrOwnerOrOwnGuid().IsCreature() && victim->GetLevelForTarget(this) > 10);
}

bool Unit::CanDazeInCombat(const Unit *victim) const
{
    return (victim && CanDazeInCombat() && (victim->GetTypeId() == TYPEID_PLAYER) && IsFacingTargetsBack(victim));
}

bool Unit::CanReactOnAbility(const SpellEntry *ability) const
{
    return (ability && !ability->HasAttribute(SPELL_ATTR_IMPOSSIBLE_DODGE_PARRY_BLOCK));
}

bool Unit::CanDodgeAbility(const Unit *attacker, const SpellEntry *ability) const
{
    // Can't dodge unavoidable and ranged attacks
    if (!CanReactOnAbility(ability) || ability->DmgClass != SPELL_DAMAGE_CLASS_MELEE)
        return false;
    // Check if attacker can be dodged at the moment
    if (!CanDodgeInCombat(attacker))
        return false;
    return true;
}

bool Unit::CanParryAbility(const Unit *attacker, const SpellEntry *ability) const
{
    // Can't parry unavoidable attacks
    if (!CanReactOnAbility(ability))
        return false;
    // Only physical melee attacks can be parried, deflect for magical and ranged
    if (ability->DmgClass != SPELL_DAMAGE_CLASS_MELEE || (GetSpellSchoolMask(ability) & SPELL_SCHOOL_MASK_MAGIC))
        return false;
    // Only close range melee attacks can be parried, longer range abilities use deflect
    if (ability->rangeIndex != SPELL_RANGE_IDX_COMBAT && ability->rangeIndex != SPELL_RANGE_IDX_SELF_ONLY)
        return false;
    // Check if attacker can be parried at the moment
    if (!CanParryInCombat(attacker))
        return false;
    return true;
}

bool Unit::CanBlockAbility(const Unit *attacker, const SpellEntry *ability, bool miss) const
{
    // Can't block unavoidable attacks
    if (!CanReactOnAbility(ability))
        return false;
    // Melee and ranged spells only
    switch (ability->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MELEE:
        case SPELL_DAMAGE_CLASS_RANGED:
            break;
        default:
            return false;
    }
    // There are 2 types of ability blocks: partial and full
    // Fully blockable spells have a specific attribute, which generates a miss instead of a partial block
    // Spells with an attribute must be rolled for block once on spell hit die
    // Spells without an attribute must be rolled for partial block only inside damage calculation
    // This function can query both scenarios via an argument flag
    if (miss != ability->HasAttribute(SPELL_ATTR_EX3_BLOCKABLE_SPELL))
        return false;
    // Check if attacker can be blocked at the moment
    if (!CanBlockInCombat(attacker))
        return false;
    return true;
}

bool Unit::CanDeflectAbility(const Unit *attacker, const SpellEntry *ability) const
{
    // Can't deflect unavoidable attacks
    if (!CanReactOnAbility(ability))
        return false;
    // Only magical melee attacks, ranged attacks and spells can be deflected, parry for the rest
    switch (ability->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_RANGED:
        case SPELL_DAMAGE_CLASS_MAGIC:
            break;
        case SPELL_DAMAGE_CLASS_MELEE:
            if (!(GetSpellSchoolMask(ability) & SPELL_SCHOOL_MASK_MAGIC))
                return false;
            if (ability->rangeIndex == SPELL_RANGE_IDX_COMBAT || ability->rangeIndex == SPELL_RANGE_IDX_SELF_ONLY)
                return false;
            break;
        default:
            return false;
    }
    // Check if attacker can be parried/deflected at the moment
    if (!CanParryInCombat(attacker))
        return false;
    return true;
}

float Unit::CalculateEffectiveDodgeChance(const Unit *attacker, WeaponAttackType attType, const SpellEntry *ability) const
{
    float chance = 0.0f;

    chance += GetDodgeChance();
    // Own chance appears to be zero / below zero / unmeaningful for some reason (debuffs?): skip calculation, unit is incapable
    if (chance < 0.005f)
        return 0.0f;
    const bool weapon = (!ability || ability->EquippedItemClass == ITEM_CLASS_WEAPON);
    // Skill difference can be negative (and reduce our chance to mitigate an attack), which means:
    // a) Attacker's level is higher
    // b) Attacker has +skill bonuses
    const bool isPlayerOrPet = (GetTypeId() == TYPEID_PLAYER || GetOwnerGuid().IsPlayer());
    const uint32 skill = (weapon ? attacker->GetWeaponSkillValue(attType, this) : attacker->GetMaxSkillValueForLevel(this));
    int32 difference = int32(GetDefenseSkillValue(attacker) - skill);
    // Defense/weapon skill factor: for players and NPCs
    float factor = 0.04f;
    // NPCs gain additional bonus dodge chance based on positive skill difference
    if (!isPlayerOrPet && difference > 0)
        factor = 0.1f;
    chance += (difference * factor);
    return std::max(0.0f, std::min(chance, 100.0f));
}

float Unit::CalculateEffectiveParryChance(const Unit *attacker, WeaponAttackType attType, const SpellEntry *ability) const
{
    float chance = 0.0f;

    if (attType == RANGED_ATTACK)
        return 0.0f;

    chance += GetParryChance();
    // Own chance appears to be zero / below zero / unmeaningful for some reason (debuffs?): skip calculation, unit is incapable
    if (chance < 0.005f)
        return 0.0f;
    const bool weapon = (!ability || ability->EquippedItemClass == ITEM_CLASS_WEAPON);
    // Skill difference can be negative (and reduce our chance to mitigate an attack), which means:
    // a) Attacker's level is higher
    // b) Attacker has +skill bonuses
    const bool isPlayerOrPet = (GetTypeId() == TYPEID_PLAYER || GetOwnerGuid().IsPlayer());
    const uint32 skill = (weapon ? attacker->GetWeaponSkillValue(attType, this) : attacker->GetMaxSkillValueForLevel(this));
    int32 difference = int32(GetDefenseSkillValue(attacker) - skill);
    // Defense/weapon skill factor: for players and NPCs
    float factor = 0.04f;
    // NPCs gain additional bonus parry chance based on positive skill difference (same value as bonus miss rate)
    if (!isPlayerOrPet && difference > 0)
    {
        if (difference > 10)
        {
            // First 10 points of difference (2 levels): usual decrease
            chance += (10 * 0.1f);
            difference -= 10;
            // Each additional point of difference:
            factor = 0.4f;
            chance += (difference * 0.2f); // Pre-WotLK: Additional 1% miss chance for each level above 2 (total @ 3% per level)
        }
        else
            factor = 0.1f;
    }
    chance += (difference * factor);
    return std::max(0.0f, std::min(chance, 100.0f));
}

float Unit::CalculateEffectiveBlockChance(const Unit *attacker, WeaponAttackType attType, const SpellEntry *ability) const
{
    float chance = 0.0f;

    chance += GetBlockChance();
    // Own chance appears to be zero / below zero / unmeaningful for some reason (debuffs?): skip calculation, unit is incapable
    if (chance < 0.005f)
        return 0.0f;
    const bool weapon = (!ability || ability->EquippedItemClass == ITEM_CLASS_WEAPON);
    // Skill difference can be negative (and reduce our chance to mitigate an attack), which means:
    // a) Attacker's level is higher
    // b) Attacker has +skill bonuses
    const bool isPlayerOrPet = (GetTypeId() == TYPEID_PLAYER || GetOwnerGuid().IsPlayer());
    const uint32 skill = (weapon ? attacker->GetWeaponSkillValue(attType, this) : attacker->GetMaxSkillValueForLevel(this));
    const int32 difference = int32(GetDefenseSkillValue(attacker) - skill);
    // Defense/weapon skill factor: for players and NPCs
    float factor = 0.04f;
    // NPCs cannot gain bonus block chance based on positive skill difference
    if (!isPlayerOrPet && difference > 0)
        factor = 0.0f;
    chance += (difference * factor);
    return std::max(0.0f, std::min(chance, 100.0f));
}

float Unit::CalculateEffectiveCrushChance(const Unit *victim, WeaponAttackType attType) const
{
    float chance = 0.0f;

    // Crushing blow chance is present when attacker's weapon skill is >= 15 over victim's own capped defense skill
    // The chance starts at 15% and increased by 2% per each additional skill point of defense deficit
    const uint32 cap = victim->GetMaxSkillValueForLevel();
    const uint32 defense = std::min(victim->GetDefenseSkillValue(this), cap);
    const int32 deficit = (int32(GetWeaponSkillValue(attType, victim)) - int32(defense));
    if (deficit >= 15)
        chance += ((2 * deficit) - 15);
    return std::max(0.0f, std::min(chance, 100.0f));
}

float Unit::CalculateEffectiveGlanceChance(const Unit *victim, WeaponAttackType attType) const
{
    float chance = 0.0f;

    // Glancing blow chance starts at 2% when victim's defense skill is > 10 lower than attacker's own capped weapon skill
    // The chance is increased for each point of skill diffrernce
    const uint32 cap = GetMaxSkillValueForLevel();
    const uint32 skill = std::min(GetWeaponSkillValue(attType, victim), cap);
    const int32 level = int32(GetLevelForTarget(victim));
    const uint32 defense = victim->GetDefenseSkillValue(this);
    // Penalty for caster classes in Classic
    if (GetTypeId() == TYPEID_PLAYER && (getClassMask() & CLASSMASK_WAND_USERS))
        chance += (level < 30 ) ? (level + (int32(defense - skill) * 2)) : (30 + (int32(defense - skill) * 2));
    else
        chance += (10 + (int32(defense - skill) * 2));
    return std::max(0.0f, std::min(chance, 100.0f));
}

float Unit::CalculateEffectiveDazeChance(const Unit *victim, WeaponAttackType attType) const
{
    float chance = 0.0f;

    // Base chance starts at 20% and is modified by difference between victim's defense skill and attacker's weapon skill
    const uint32 level = victim->GetLevelForTarget(this);
    // Base chance is reduced for lower levels
    if (level < 30)
        chance += (0.65f * level) + 0.5f;
    else
        chance += 20.0f;
    // Same mechanics as miss chance, but premultiplied by 4 (20% base instead of 5% base)
    chance += (int32(GetWeaponSkillValue(attType, victim)) - int32(victim->GetDefenseSkillValue(this))) * 0.04f * 4;
    // Chance is capped at 40%
    return std::max(0.0f, std::min(chance, 40.0f));
}

float Unit::CalculateAbilityDeflectChance(const Unit *attacker, const SpellEntry *ability) const
{
    switch (ability->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MELEE:
        case SPELL_DAMAGE_CLASS_RANGED: return CalculateEffectiveParryChance(attacker, GetWeaponAttackType(ability), ability);
        case SPELL_DAMAGE_CLASS_MAGIC:  return CalculateEffectiveParryChance(attacker, RANGED_ATTACK, ability);
    }
    return 0.0f;
}

bool Unit::RollAbilityPartialBlockOutcome(const Unit *attacker, WeaponAttackType attType, const SpellEntry *ability) const
{
    if (!CanBlockAbility(attacker, ability))
        return false;
    const float chance = CalculateEffectiveBlockChance(attacker, attType, ability);
    return roll_chance_combat(chance);
}

float Unit::GetDodgeChance() const
{
    float chance = 0.0f;
    if (GetTypeId() == TYPEID_UNIT)
        chance += 5.0f;
    chance += m_modDodgeChance;
    return chance;
}

float Unit::GetParryChance() const
{
    float chance = 0.0f;
    // FIXME: Verify which creatures are eligible for parry together with DB team and implement in a more clean way
    if (GetTypeId() == TYPEID_UNIT)
        chance += 5.0f;
    chance += m_modParryChance;
    return chance;
}

float Unit::GetBlockChance() const
{
    float chance = 0.0f;
    // FIXME: Verify which creatures are eligible for block together with DB team and implement in a more clean way
    if (GetTypeId() == TYPEID_UNIT)
        chance += 5.0f;
    chance += m_modBlockChance;
    return chance;
}

float Unit::GetReflectChance(SpellSchoolMask schoolMask) const
{
    float chance = 0.0f;
    chance += GetTotalAuraModifier(SPELL_AURA_REFLECT_SPELLS);
    // Reflect works in victim's favour when multi-school spells are involved
    chance += GetTotalAuraModifierByMiscMask(SPELL_AURA_REFLECT_SPELLS_SCHOOL, schoolMask);
    return chance;
}

bool Unit::CanCrit(const SpellEntry *entry, SpellSchoolMask schoolMask, WeaponAttackType attType) const
{
    if (!IsSpellAbleToCrit(entry))
        return false;
    switch (entry->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MELEE:
        case SPELL_DAMAGE_CLASS_RANGED: return CanCrit(attType);
        case SPELL_DAMAGE_CLASS_NONE:
        case SPELL_DAMAGE_CLASS_MAGIC:  return CanCrit(schoolMask);
    }
    return false;
}

float Unit::GetCritChance(WeaponAttackType attType) const
{
    float chance = 0.0f;
    if (GetTypeId() == TYPEID_UNIT)
        // Base crit chance (needed for units only, already included for players)
        chance += 5.0f;
    chance += m_modCritChance[(attType == OFF_ATTACK) ? BASE_ATTACK : attType];
    return chance;
}

float Unit::GetCritChance(SpellSchoolMask schoolMask) const
{
    float chance = 0.0f;

    uint32 mask = uint32(schoolMask);
    if (!mask)
        return 0.0f;

    if (GetTypeId() == TYPEID_UNIT)
        // Base crit chance (needed for units only, already included for players)
        chance += 5.0f;
    // Pick highest spell crit available for given school mask
    for (uint8 school = 0; mask; ++school)
    {
        if (mask & 1)
        {
            const float crit = m_modSpellCritChance[school];
            if (crit > chance)
                chance = crit;
        }
        mask >>= 1;
    }
    return chance;
}

float Unit::GetCritChance(const SpellEntry *entry, SpellSchoolMask schoolMask) const
{
    float chance = 0.0f;

    if (!entry)
        return 0.0f;

    // Add own hit chance
    switch (entry->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MAGIC:
            chance += GetCritChance(schoolMask);
            break;
        case SPELL_DAMAGE_CLASS_MELEE:
        case SPELL_DAMAGE_CLASS_RANGED:
            chance += GetCritChance(GetWeaponAttackType(entry));
            break;
    }
    // Add mod
    if (Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(entry->Id, SPELLMOD_CRITICAL_CHANCE, chance);
    return chance;
}

float Unit::GetCritTakenChance(SpellSchoolMask dmgSchoolMask, SpellDmgClass dmgClass, bool heal) const
{
    float chance = 0.0f;
    // Resilience and resilience-like auras:
    if (!heal)
    {
        switch (uint32(dmgClass))
        {
            case SPELL_DAMAGE_CLASS_MELEE:
                chance += GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_CHANCE);
                break;
            case SPELL_DAMAGE_CLASS_RANGED:
                chance += GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_CHANCE);
                break;
            case SPELL_DAMAGE_CLASS_MAGIC:
                chance += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE, dmgSchoolMask);
                break;
        }
    }
    return chance;
}

float Unit::GetCritMultiplier(SpellSchoolMask dmgSchoolMask, uint32 creatureTypeMask, const SpellEntry *spell, bool heal) const
{
    const float pct = 0.01f;
    // Default crit multiplier (attacks): 2x
    float multiplier = 2.0f;
    // Default crit multiplier for spells: 1.5x
    if (spell)
    {
        switch (spell->DmgClass)
        {
            case SPELL_DAMAGE_CLASS_NONE:
            case SPELL_DAMAGE_CLASS_MAGIC:
                multiplier = 1.5f;
                break;
        }
    }
    // Multiplier bonus coefficient for damage
    if (!heal)
    {
        float bonus = 0;
        if (dmgSchoolMask)
            bonus += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_CRIT_DAMAGE_BONUS, dmgSchoolMask); // Gems, talents
        if (creatureTypeMask)
            bonus += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_CRIT_PERCENT_VERSUS, creatureTypeMask); // Creature slaying
        multiplier *= (1.0f + (bonus * pct));
    }
    // Apply spell mods
    if (spell)
    {
        if (Player* modOwner = GetSpellModOwner())
        {
            // Extract crit bonus part only from multiplier (exclude base of 1) and modify it
            float bonus = (multiplier - 1.0f);
            modOwner->ApplySpellMod(spell->Id, SPELLMOD_CRIT_DAMAGE_BONUS, bonus);
            // Reconstruct final multiplier now with increased bonus part
            multiplier = (1.0f + bonus);
        }
    }
    return std::max(0.0f, multiplier);
}

float Unit::GetCritTakenMultiplier(SpellSchoolMask /*dmgSchoolMask*/, SpellDmgClass /*dmgClass*/, float /*ignorePct*/, bool /*heal*/) const
{
    // Default incoming crit multiplier: 1x (receive full damage)
    float multiplier = 1.0f;
    return std::max(0.0f, multiplier);
}

uint32 Unit::CalculateCritAmount(const Unit *victim, uint32 amount, const SpellEntry *entry, bool heal) const
{
    if (!entry)
        return 0;
    const SpellSchoolMask schoolMask = GetSpellSchoolMask(entry);
    const uint32 creatureTypeMask = (victim ? victim->GetCreatureTypeMask() : 0);
    const float multiplier = GetCritMultiplier(schoolMask, creatureTypeMask, entry, heal);
    const SpellDmgClass dmgClass = SpellDmgClass(entry->DmgClass);
    const float ignoreReduction = 0.0f;
    const float taken = (victim ? victim->GetCritTakenMultiplier(schoolMask, dmgClass, ignoreReduction, heal) : 1.0f);
    const uint32 outgoing = uint32(amount * multiplier);
    // Crit amount can't be less than normal hit
    const uint32 result = std::max(amount, uint32(outgoing * taken));
    return result;
}

uint32 Unit::CalculateCritAmount(CalcDamageInfo *meleeInfo) const
{
    if (!meleeInfo)
        return 0;
    uint32 clean = 0;
    uint32 total = 0;
    const Unit* victim = meleeInfo->target;
    const uint32 creatureTypeMask = (victim ? victim->GetCreatureTypeMask() : 0);
    const SpellDmgClass dmgClass = ((meleeInfo->attackType == RANGED_ATTACK) ? SPELL_DAMAGE_CLASS_RANGED : SPELL_DAMAGE_CLASS_MELEE);
    const float ignoreReduction = 0.0f;
    for (uint8 i = 0; i < m_weaponDamageCount[meleeInfo->attackType]; i++)
    {
        SubDamageInfo &subDamage = meleeInfo->subDamage[i];
        const uint32 amount = subDamage.damage;
        const SpellSchoolMask schoolMask = subDamage.damageSchoolMask;
        const float multiplier = GetCritMultiplier(schoolMask, creatureTypeMask);
        const float taken = (victim ? victim->GetCritTakenMultiplier(schoolMask, dmgClass, ignoreReduction) : 1.0f);
        const uint32 outgoing = uint32(amount * multiplier);
        // Crit amount can't be less than normal hit
        const uint32 result = std::max(amount, uint32(outgoing * taken));
        subDamage.damage = result;
        total += result;
        clean += std::max(result, outgoing);
    }
    meleeInfo->cleanDamage = clean;
    return total;
}

float Unit::GetHitChance(WeaponAttackType attackType) const
{
    if (attackType == RANGED_ATTACK)
        return m_modRangedHitChance;
    return m_modMeleeHitChance;
}

float Unit::GetHitChance(SpellSchoolMask schoolMask) const
{
    float chance = 0.0f;

    if (!uint32(schoolMask))
        return 0.0f;

    chance += m_modSpellHitChance;
    return chance;
}

float Unit::GetHitChance(const SpellEntry *entry, SpellSchoolMask schoolMask) const
{
    float chance = 0.0f;

    if (!entry)
        return 0.0f;

    // Add own hit chance
    switch (entry->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MAGIC:
            chance += GetHitChance(schoolMask);
            break;
        case SPELL_DAMAGE_CLASS_MELEE:
        case SPELL_DAMAGE_CLASS_RANGED:
            chance += GetHitChance(GetWeaponAttackType(entry));
            break;
    }
    // Add mod
    if (Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(entry->Id, SPELLMOD_RESIST_MISS_CHANCE, chance);
    return chance;
}

float Unit::GetMissChance(WeaponAttackType /*attackType*/) const
{
    // Totems have no inherit miss chance
    if (GetTypeId() == TYPEID_UNIT && ((const Creature*)this)->IsTotem())
        return 0.0f;
    // Default base chance to be missed for all acting units: 5%
    float chance = 5.0f;
    return chance;
}

float Unit::GetMissChance(SpellSchoolMask /*schoolMask*/) const
{
    // Totems have no inherit miss chance
    if (GetTypeId() == TYPEID_UNIT && ((const Creature*)this)->IsTotem())
        return 0.0f;
    // Default base chance to be missed by a spell for all acting units: 4%
    float chance = 4.0f;
    return chance;
}

float Unit::GetMissChance(const SpellEntry *entry, SpellSchoolMask schoolMask) const
{
    float chance = 0.0f;

    if (!entry)
        return 0.0f;

    // Add own miss chance
    switch (entry->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MELEE:
        case SPELL_DAMAGE_CLASS_RANGED:
            chance += GetMissChance(GetWeaponAttackType(entry));
            break;
        // By default, we use spell miss chance even for spells without dmgclass
        default:
            chance += GetMissChance(schoolMask);
            break;
    }
    // AoE avoidance
    if (IsAreaOfEffectSpell(entry))
        chance += GetTotalAuraModifier(SPELL_AURA_MOD_AOE_AVOIDANCE);
    return chance;
}

float Unit::CalculateEffectiveCritChance(const Unit* victim, WeaponAttackType attType, const SpellEntry *ability) const
{
    float chance = 0.0f;
    chance += (ability ? GetCritChance(ability, SPELL_SCHOOL_MASK_NORMAL) : GetCritChance(attType));
    // Own chance appears to be zero / below zero / unmeaningful for some reason (debuffs?): skip calculation, unit is incapable
    if (chance < 0.005f)
        return 0.0f;
    const bool ranged = (attType == RANGED_ATTACK);
    // Skip victim calculation if positive ability
    if (ability && IsPositiveSpell(ability, this, victim))
        return std::max(0.0f, std::min(chance, 100.0f));
    const bool weapon = (!ability || ability->EquippedItemClass == ITEM_CLASS_WEAPON);
    // Skill difference can be both negative and positive.
    // a) Positive means that attacker's level is higher or additional weapon +skill bonuses
    // b) Negative means that victim's level is higher or additional +defense bonuses
    const uint32 skill = (weapon ? GetWeaponSkillValue(attType, victim) : GetMaxSkillValueForLevel(victim));
    const int32 difference = int32(skill - victim->GetDefenseSkillValue(this));
    // Weapon skill factor: for players and NPCs
    float factor = 0.04f;
    chance += (difference * factor);
    // Victim's crit taken chance
    const SpellDmgClass dmgClass = (ranged ? SPELL_DAMAGE_CLASS_RANGED : SPELL_DAMAGE_CLASS_MELEE);
    chance += victim->GetCritTakenChance(SPELL_SCHOOL_MASK_NORMAL, dmgClass);
    return std::max(0.0f, std::min(chance, 100.0f));
}

float Unit::CalculateEffectiveMissChance(const Unit *victim, WeaponAttackType attType, const SpellEntry *ability) const
{
    float chance = 0.0f;

    chance += (ability ? victim->GetMissChance(ability, SPELL_SCHOOL_MASK_NORMAL) : victim->GetMissChance(attType));
    // Victim's own chance appears to be zero / below zero / unmeaningful for some reason (debuffs?): skip calculation, unit can't be missed
    if (chance < 0.005f)
        return 0.0f;
    const bool ranged = (attType == RANGED_ATTACK);
    const bool weapon = (!ability || ability->EquippedItemClass == ITEM_CLASS_WEAPON);
    // Check if dual wielding, add additional miss penalty
    if (!ability && !ranged && haveOffhandWeapon())
        chance += 19.0f;
    // Skill difference can be both negative and positive. Positive difference means that:
    // a) Victim's level is higher
    // b) Victim has additional defense skill bonuses
    const bool vsPlayerOrPet = (victim->GetTypeId() == TYPEID_PLAYER || victim->GetOwnerGuid().IsPlayer());
    const uint32 skill = (weapon ? GetWeaponSkillValue(attType, victim) : GetMaxSkillValueForLevel(victim));
    int32 difference = int32(victim->GetDefenseSkillValue(this) - skill);
    // Defense/weapon skill factor: for players and NPCs
    float factor = 0.04f;
    // NPCs gain additional bonus to incoming hit chance reduction based on positive skill difference (same value as bonus parry rate)
    if (!vsPlayerOrPet && difference > 0)
    {
        if (difference > 10)
        {
            // First 10 points of difference (2 levels): usual decrease
            chance += (10 * 0.1f);
            difference -= 10;
            // Each additional point of difference:
            factor = 0.4f;
            chance += (difference * 0.2f); // Pre-WotLK: Additional 1% miss chance for each level (final @ 3% per level)
        }
        else
            factor = 0.1f;
    }
    chance += (difference * factor);
    // Victim's auras affecting attacker's hit contribution:
    chance -= victim->GetTotalAuraModifier(ranged ? SPELL_AURA_MOD_ATTACKER_RANGED_HIT_CHANCE: SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE);
    // Finally, take hit chance
    chance -= (ability ? GetHitChance(ability, SPELL_SCHOOL_MASK_NORMAL) : GetHitChance(attType));
    return std::max(0.0f, std::min(chance, 100.0f));
}

float Unit::CalculateSpellCritChance(const Unit *victim, SpellSchoolMask schoolMask, const SpellEntry *spell) const
{
    if (!spell || spell->HasAttribute(SPELL_ATTR_EX2_CANT_CRIT))
        return 0.0f;

    float chance = 0.0f;

    switch (spell->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MELEE:
        case SPELL_DAMAGE_CLASS_RANGED:
            return CalculateEffectiveCritChance(victim, GetWeaponAttackType(spell), spell);
    }
    chance += GetCritChance(spell, schoolMask);
    // Own chance appears to be zero / below zero / unmeaningful for some reason (debuffs?): skip calculation, unit is incapable
    if (chance < 0.005f)
        return 0.0f;
    // Modify by victim in case of hostile hit
    if (!IsPositiveSpell(spell, this, victim))
        chance += victim->GetCritTakenChance(schoolMask, SpellDmgClass(spell->DmgClass));
    // TODO: Scripted crit chance auras: class script auras need to be orgnaized and re-implemented in the future as a part of scripting system
    const AuraList &scripts = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    for (AuraList::const_iterator i = scripts.begin(); i != scripts.end(); ++i)
    {
        if (!((*i)->isAffectedOnSpell(spell)))
            continue;
        switch ((*i)->GetModifier()->m_miscvalue)
        {
            // Shatter
            case 849: if (victim->isFrozen()) chance += 10.0f; break;
            case 910: if (victim->isFrozen()) chance += 20.0f; break;
            case 911: if (victim->isFrozen()) chance += 30.0f; break;
            case 912: if (victim->isFrozen()) chance += 40.0f; break;
            case 913: if (victim->isFrozen()) chance += 50.0f; break;
            default:
                break;
        }
    }
    return std::max(0.0f, std::min(chance, 100.0f));
}

float Unit::CalculateSpellMissChance(const Unit *victim, SpellSchoolMask schoolMask, const SpellEntry *spell) const
{
    if (!spell || spell->HasAttribute(SPELL_ATTR_EX3_CANT_MISS))
        return 0.0f;

    float chance = 0.0f;
    const float minimum = 1.0f; // Pre-WotLK: unavoidable spell miss is at least 1%

    switch (spell->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MELEE:
        case SPELL_DAMAGE_CLASS_RANGED:
            return CalculateEffectiveMissChance(victim, GetWeaponAttackType(spell), spell);
    }
    chance += victim->GetMissChance(spell, schoolMask);
    // Victim's own chance appears to be zero / below zero / unmeaningful for some reason (debuffs?): skip calculation, unit can't be missed
    if (chance < 0.005f)
        return 0.0f;
    // Level difference: positive adds to miss chance, negative substracts
    const bool vsPlayerOrPet = (victim->GetTypeId() == TYPEID_PLAYER || victim->GetOwnerGuid().IsPlayer());
    int32 difference = int32(victim->GetLevelForTarget(this) - GetLevelForTarget(victim));
    // Level difference factor: 1% per level
    uint8 factor = 1;
    // NPCs and players gain additional bonus to incoming spell hit chance reduction based on positive level difference
    if (difference > 2)
    {
        chance += (2 * factor);
        // Miss bonus for each additional level of difference above 2
        factor = (vsPlayerOrPet ? 7 : 11);
        chance += ((difference - 2) * factor);
    }
    else
        chance += (difference * factor);
    // Reduce by caster's spell hit chance
    chance -= GetHitChance(spell, schoolMask);
    // Reduce (or increase) by victim SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE auras
    chance -= victim->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE, schoolMask);
    return std::max(minimum, std::min(chance, 100.0f));
}

bool Unit::RollSpellCritOutcome(const Unit *victim, SpellSchoolMask schoolMask, const SpellEntry *spell) const
{
    if (!CanCrit(spell, schoolMask, GetWeaponAttackType(spell)))
        return false;

    const float chance = CalculateSpellCritChance(victim, schoolMask, spell);
    return roll_chance_combat(chance);
}

int32 Unit::GetResistancePenetration(SpellSchools school) const
{
    // Technically, it is target resistance mod, but it's used as penetration (negative sign) 99.9% of the time
    // So, let's logically reverse the sign and use it as such
    return -(GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_TARGET_RESISTANCE, (1 << school)));
}

float Unit::CalculateEffectiveMagicResistancePercent(const Unit *attacker, SpellSchoolMask schoolMask, bool binary) const
{
    // Non-magical or contains Physical as a component (such as Chaos) - ignores resistances completely
    if (schoolMask & SPELL_SCHOOL_MASK_NORMAL)
        return 0.0f;
    // Completely ignore holy resistance value (since beta stages)
    uint32 schools = uint32(schoolMask & ~uint32(SPELL_SCHOOL_MASK_HOLY));
    // Select a resistance value matching given school mask, prefer mininal for multischool spells
    int32 resistance = 0;
    for (uint32 school = 0; schools; ++school)
    {
        if (schools & 1)
        {
            // Base victim resistance
            int32 amount = int32(GetResistance(SpellSchools(school)));
            // Modify by penetration
            amount -= attacker->GetResistancePenetration(SpellSchools(school));
            // If unit has no resistance for this school: use it!
            if (amount < 1)
            {
                resistance = 0; // Resistance can't be negative since early stages of development
                break;
            }
            // Else if first school encountered or more vulnerable: memorize and continue
            else if (!resistance || amount < resistance)
                resistance = amount;
        }
        schools >>= 1;
    }
    // Attacker's level based skill, penalize when calculating for low levels (< 20):
    const uint32 skill = std::max(attacker->GetMaxSkillValueForLevel(this), uint16(100));
    float percent = float(float(resistance) / float(skill)) * 100 * 0.75f;
    // Bonus resistance by level difference when calculating damage hit for NPCs only
    if (!binary && GetTypeId() == TYPEID_UNIT && GetCharmerOrOwnerOrOwnGuid().IsCreature())
        percent += (0.4f * std::max(int32(GetMaxSkillValueForLevel(attacker) - skill), 0));
    // Magic resistance percentage cap
    const float cap = 75.0f; // Pre-WotLK: 75%
    return std::max(0.0f, std::min(percent, cap));
}

float Unit::CalculateSpellResistChance(const Unit *victim, SpellSchoolMask schoolMask, const SpellEntry *spell) const
{
    float chance = 0.0f;

    // Chance to fully resist a spell by magic resistance
    if (spell->DmgClass == SPELL_DAMAGE_CLASS_MAGIC && !spell->HasAttribute(SPELL_ATTR_EX4_IGNORE_RESISTANCES))
    {
        const bool binary = IsBinarySpell(spell);
        const float percent = victim->CalculateEffectiveMagicResistancePercent(this, schoolMask, binary);
        if (binary)
            chance += percent;
        else
            chance += float(SPELL_PARTIAL_RESIST_DISTRIBUTION.at(uint32(percent * 100)).at(SPELL_PARTIAL_RESIST_PCT_100)) * 0.01f;
    }
    // Chance to fully resist entire spell by it's dispel type
    if (const int32 type = int32(spell->Dispel))
        chance += victim->GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_DEBUFF_RESISTANCE, type);
    // Chance to fully resist entire spell by it's mechanic
    if (const int32 mechanic = int32(spell->Mechanic))
        chance += victim->GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_MECHANIC_RESISTANCE, mechanic);

    // TODO: Move it in the future, effect mechanic resist must be processed differently
    float effects = 0.0f;
    for (uint32 i = EFFECT_INDEX_0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (!spell->Effect[i])
            continue;
        if (const int32 mechanic = int32(spell->EffectMechanic[i]))
        {
            const float resist = victim->GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_MECHANIC_RESISTANCE, mechanic);
            if (resist > effects)
                effects = resist;
        }
    }
    chance += effects;
    //
    return std::max(0.0f, std::min(chance, 100.0f));
}

uint32 Unit::GetWeaponSkillValue(WeaponAttackType attType, Unit const* target) const
{
    uint32 value;
    if (GetTypeId() == TYPEID_PLAYER)
    {
        Item* item = ((Player*)this)->GetWeaponForAttack(attType, true, true);

        // feral or unarmed skill only for base attack
        if (attType != BASE_ATTACK && !item)
            return 0;

        if (IsInFeralForm())
            return GetMaxSkillValueForLevel();              // always maximized SKILL_FERAL_COMBAT in fact

        // weapon skill or (unarmed for base attack)
        uint32 skill = item ? item->GetSkill() : uint32(SKILL_UNARMED);

        // in PvP always use full skill (and bonuses on top of it if present)
        value = (target && target->GetTypeId() == TYPEID_PLAYER)
                ? std::max(((Player*)this)->GetMaxSkillValue(skill), ((Player*)this)->GetSkillValue(skill))
                : ((Player*)this)->GetSkillValue(skill);
    }
    else
        value = GetUnitMeleeSkill(target);
    return value;
}

void Unit::_UpdateSpells(uint32 time)
{
    if (m_currentSpells[CURRENT_AUTOREPEAT_SPELL])
        _UpdateAutoRepeatSpell();

    // remove finished spells from current pointers
    for (uint32 i = 0; i < CURRENT_MAX_SPELL; ++i)
    {
        if (m_currentSpells[i] && m_currentSpells[i]->getState() == SPELL_STATE_FINISHED)
        {
            m_currentSpells[i]->SetReferencedFromCurrent(false);
            m_currentSpells[i] = nullptr;                      // remove pointer
        }
    }

    // update auras
    // m_AurasUpdateIterator can be updated in inderect called code at aura remove to skip next planned to update but removed auras
    for (m_spellAuraHoldersUpdateIterator = m_spellAuraHolders.begin(); m_spellAuraHoldersUpdateIterator != m_spellAuraHolders.end();)
    {
        SpellAuraHolder* i_holder = m_spellAuraHoldersUpdateIterator->second;
        ++m_spellAuraHoldersUpdateIterator;                 // need shift to next for allow update if need into aura update
        i_holder->UpdateHolder(time);
    }

    // remove expired auras
    for (SpellAuraHolderMap::iterator iter = m_spellAuraHolders.begin(); iter != m_spellAuraHolders.end();)
    {
        SpellAuraHolder* holder = iter->second;

        if (!(holder->IsPermanent() || holder->IsPassive()) && holder->GetAuraDuration() == 0)
        {
            RemoveSpellAuraHolder(holder, AURA_REMOVE_BY_EXPIRE);
            iter = m_spellAuraHolders.begin();
        }
        else
            ++iter;
    }

    if (!m_gameObj.empty())
    {
        GameObjectList::iterator ite1, dnext1;
        for (ite1 = m_gameObj.begin(); ite1 != m_gameObj.end(); ite1 = dnext1)
        {
            dnext1 = ite1;
            //(*i)->Update( difftime );
            if (!(*ite1)->isSpawned())
            {
                (*ite1)->SetOwnerGuid(ObjectGuid());
                (*ite1)->SetRespawnTime(0);
                (*ite1)->Delete();
                dnext1 = m_gameObj.erase(ite1);
            }
            else
                ++dnext1;
        }
    }
}

void Unit::_UpdateAutoRepeatSpell()
{
    // check "realtime" interrupts
    if ((GetTypeId() == TYPEID_PLAYER && ((Player*)this)->IsMoving()) || IsNonMeleeSpellCasted(false, false, true))
    {
        // cancel wand shoot
        if (m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo->Category == 351)
            InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
        m_AutoRepeatFirstCast = true;
        return;
    }

    // apply delay
    if (m_AutoRepeatFirstCast && getAttackTimer(RANGED_ATTACK) < 500)
        setAttackTimer(RANGED_ATTACK, 500);
    m_AutoRepeatFirstCast = false;

    // castroutine
    if (isAttackReady(RANGED_ATTACK))
    {
        // Check if able to cast
        if (m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->CheckCast(true) != SPELL_CAST_OK)
        {
            InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
            return;
        }

        // we want to shoot
        Spell* spell = new Spell(this, m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo, true);
        spell->SpellStart(&(m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_targets));

        // all went good, reset attack
        resetAttackTimer(RANGED_ATTACK);
    }
}

void Unit::SetCurrentCastedSpell(Spell* pSpell)
{
    MANGOS_ASSERT(pSpell);                                  // nullptr may be never passed here, use InterruptSpell or InterruptNonMeleeSpells

    CurrentSpellTypes CSpellType = pSpell->GetCurrentContainer();

    if (pSpell == m_currentSpells[CSpellType]) return;      // avoid breaking self

    // break same type spell if it is not delayed
    InterruptSpell(CSpellType, false);

    // special breakage effects:
    switch (CSpellType)
    {
        case CURRENT_GENERIC_SPELL:
        {
            // generic spells always break channeled not delayed spells
            InterruptSpell(CURRENT_CHANNELED_SPELL, false);

            // autorepeat breaking
            if (m_currentSpells[CURRENT_AUTOREPEAT_SPELL])
            {
                // break autorepeat if not Auto Shot
                if (m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo->Category == 351)
                    InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
                m_AutoRepeatFirstCast = true;
            }
        } break;

        case CURRENT_CHANNELED_SPELL:
        {
            // channel spells always break generic non-delayed and any channeled spells
            InterruptSpell(CURRENT_GENERIC_SPELL, false);
            InterruptSpell(CURRENT_CHANNELED_SPELL);

            // it also does break autorepeat if not Auto Shot
            if (m_currentSpells[CURRENT_AUTOREPEAT_SPELL] &&
                    m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo->Category == 351)
                InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
        } break;

        case CURRENT_AUTOREPEAT_SPELL:
        {
            // only Auto Shoot does not break anything
            if (pSpell->m_spellInfo->Category == 351)
            {
                // generic autorepeats break generic non-delayed and channeled non-delayed spells
                InterruptSpell(CURRENT_GENERIC_SPELL, false);
                InterruptSpell(CURRENT_CHANNELED_SPELL, false);
            }
            // special action: set first cast flag
            m_AutoRepeatFirstCast = true;
        } break;

        default:
        {
            // other spell types don't break anything now
        } break;
    }

    // current spell (if it is still here) may be safely deleted now
    if (m_currentSpells[CSpellType])
        m_currentSpells[CSpellType]->SetReferencedFromCurrent(false);

    // set new current spell
    m_currentSpells[CSpellType] = pSpell;
    pSpell->SetReferencedFromCurrent(true);
}

void Unit::InterruptSpell(CurrentSpellTypes spellType, bool withDelayed)
{
    if (m_currentSpells[spellType] && (withDelayed || m_currentSpells[spellType]->getState() != SPELL_STATE_DELAYED))
    {
        // send autorepeat cancel message for autorepeat spells
        if (spellType == CURRENT_AUTOREPEAT_SPELL)
        {
            if (GetTypeId() == TYPEID_PLAYER)
                ((Player*)this)->SendAutoRepeatCancel();
        }

        if (m_currentSpells[spellType]->getState() != SPELL_STATE_FINISHED)
            m_currentSpells[spellType]->cancel();

        // cancel can interrupt spell already (caster cancel ->target aura remove -> caster iterrupt)
        if (m_currentSpells[spellType])
        {
            m_currentSpells[spellType]->SetReferencedFromCurrent(false);
            m_currentSpells[spellType] = nullptr;
        }
    }
}

void Unit::FinishSpell(CurrentSpellTypes spellType, bool ok /*= true*/)
{
    Spell* spell = m_currentSpells[spellType];
    if (!spell)
        return;

    if (spellType == CURRENT_CHANNELED_SPELL)
        spell->SendChannelUpdate(0);

    spell->finish(ok);
}

bool Unit::IsNonMeleeSpellCasted(bool withDelayed, bool skipChanneled, bool skipAutorepeat) const
{
    // We don't do loop here to explicitly show that melee spell is excluded.
    // Maybe later some special spells will be excluded too.

    // generic spells are casted when they are not finished and not delayed
    if (m_currentSpells[CURRENT_GENERIC_SPELL] &&
            (m_currentSpells[CURRENT_GENERIC_SPELL]->getState() != SPELL_STATE_FINISHED) &&
            (withDelayed || m_currentSpells[CURRENT_GENERIC_SPELL]->getState() != SPELL_STATE_DELAYED))
        return true;

    // channeled spells may be delayed, but they are still considered casted
    else if (!skipChanneled && m_currentSpells[CURRENT_CHANNELED_SPELL] &&
             (m_currentSpells[CURRENT_CHANNELED_SPELL]->getState() != SPELL_STATE_FINISHED))
        return true;

    // autorepeat spells may be finished or delayed, but they are still considered casted
    else if (!skipAutorepeat && m_currentSpells[CURRENT_AUTOREPEAT_SPELL])
        return true;

    return false;
}

void Unit::InterruptNonMeleeSpells(bool withDelayed, uint32 spell_id)
{
    // generic spells are interrupted if they are not finished or delayed
    if (m_currentSpells[CURRENT_GENERIC_SPELL] && (!spell_id || m_currentSpells[CURRENT_GENERIC_SPELL]->m_spellInfo->Id == spell_id))
        InterruptSpell(CURRENT_GENERIC_SPELL, withDelayed);

    // autorepeat spells are interrupted if they are not finished or delayed
    if (m_currentSpells[CURRENT_AUTOREPEAT_SPELL] && (!spell_id || m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo->Id == spell_id))
        InterruptSpell(CURRENT_AUTOREPEAT_SPELL, withDelayed);

    // channeled spells are interrupted if they are not finished, even if they are delayed
    if (m_currentSpells[CURRENT_CHANNELED_SPELL] && (!spell_id || m_currentSpells[CURRENT_CHANNELED_SPELL]->m_spellInfo->Id == spell_id))
        InterruptSpell(CURRENT_CHANNELED_SPELL, true);
}

Spell* Unit::FindCurrentSpellBySpellId(uint32 spell_id) const
{
    for (uint32 i = 0; i < CURRENT_MAX_SPELL; ++i)
        if (m_currentSpells[i] && m_currentSpells[i]->m_spellInfo->Id == spell_id)
            return m_currentSpells[i];
    return nullptr;
}

void Unit::SetInFront(Unit const* target)
{
    SetOrientation(GetAngle(target));
}

void Unit::SetFacingTo(float ori)
{
    Movement::MoveSplineInit init(*this);
    init.SetFacing(ori);
    init.Launch();
}

void Unit::SetFacingToObject(WorldObject* pObject)
{
    // never face when already moving
    if (!IsStopped())
        return;

    // TODO: figure out under what conditions creature will move towards object instead of facing it where it currently is.
    SetFacingTo(GetAngle(pObject));
}

bool Unit::isInAccessablePlaceFor(Unit const* unit) const
{
    if (IsInWater())
        return unit->CanSwim();
    else
        return unit->CanWalk() || unit->CanFly();
}

bool Unit::IsInWater() const
{
    return GetTerrain()->IsInWater(GetPositionX(), GetPositionY(), GetPositionZ());
}

bool Unit::IsUnderWater() const
{
    return GetTerrain()->IsUnderWater(GetPositionX(), GetPositionY(), GetPositionZ());
}

void Unit::DeMorph()
{
    SetDisplayId(GetNativeDisplayId());
}

int32 Unit::GetTotalAuraModifier(AuraType auratype) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin(); i != mTotalAuraList.end(); ++i)
        modifier += (*i)->GetModifier()->m_amount;

    return modifier;
}

float Unit::GetTotalAuraMultiplier(AuraType auratype) const
{
    float multiplier = 1.0f;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin(); i != mTotalAuraList.end(); ++i)
        multiplier *= (100.0f + (*i)->GetModifier()->m_amount) / 100.0f;

    return multiplier;
}

int32 Unit::GetMaxPositiveAuraModifier(AuraType auratype) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin(); i != mTotalAuraList.end(); ++i)
        if ((*i)->GetModifier()->m_amount > modifier)
            modifier = (*i)->GetModifier()->m_amount;

    return modifier;
}

int32 Unit::GetMaxNegativeAuraModifier(AuraType auratype) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin(); i != mTotalAuraList.end(); ++i)
        if ((*i)->GetModifier()->m_amount < modifier)
            modifier = (*i)->GetModifier()->m_amount;

    return modifier;
}

int32 Unit::GetTotalAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const
{
    if (!misc_mask)
        return 0;

    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin(); i != mTotalAuraList.end(); ++i)
    {
        Modifier* mod = (*i)->GetModifier();
        if (mod->m_miscvalue & misc_mask)
            modifier += mod->m_amount;
    }
    return modifier;
}

float Unit::GetTotalAuraMultiplierByMiscMask(AuraType auratype, uint32 misc_mask) const
{
    if (!misc_mask)
        return 1.0f;

    float multiplier = 1.0f;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin(); i != mTotalAuraList.end(); ++i)
    {
        Modifier* mod = (*i)->GetModifier();
        if (mod->m_miscvalue & misc_mask)
            multiplier *= (100.0f + mod->m_amount) / 100.0f;
    }
    return multiplier;
}

int32 Unit::GetMaxPositiveAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const
{
    if (!misc_mask)
        return 0;

    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin(); i != mTotalAuraList.end(); ++i)
    {
        Modifier* mod = (*i)->GetModifier();
        if (mod->m_miscvalue & misc_mask && mod->m_amount > modifier)
            modifier = mod->m_amount;
    }

    return modifier;
}

int32 Unit::GetMaxNegativeAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const
{
    if (!misc_mask)
        return 0;

    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin(); i != mTotalAuraList.end(); ++i)
    {
        Modifier* mod = (*i)->GetModifier();
        if (mod->m_miscvalue & misc_mask && mod->m_amount < modifier)
            modifier = mod->m_amount;
    }

    return modifier;
}

int32 Unit::GetTotalAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin(); i != mTotalAuraList.end(); ++i)
    {
        Modifier* mod = (*i)->GetModifier();
        if (mod->m_miscvalue == misc_value)
            modifier += mod->m_amount;
    }
    return modifier;
}

float Unit::GetTotalAuraMultiplierByMiscValue(AuraType auratype, int32 misc_value) const
{
    float multiplier = 1.0f;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin(); i != mTotalAuraList.end(); ++i)
    {
        Modifier* mod = (*i)->GetModifier();
        if (mod->m_miscvalue == misc_value)
            multiplier *= (100.0f + mod->m_amount) / 100.0f;
    }
    return multiplier;
}

int32 Unit::GetMaxPositiveAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin(); i != mTotalAuraList.end(); ++i)
    {
        Modifier* mod = (*i)->GetModifier();
        if (mod->m_miscvalue == misc_value && mod->m_amount > modifier)
            modifier = mod->m_amount;
    }

    return modifier;
}

int32 Unit::GetMaxNegativeAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (AuraList::const_iterator i = mTotalAuraList.begin(); i != mTotalAuraList.end(); ++i)
    {
        Modifier* mod = (*i)->GetModifier();
        if (mod->m_miscvalue == misc_value && mod->m_amount < modifier)
            modifier = mod->m_amount;
    }

    return modifier;
}

bool Unit::AddSpellAuraHolder(SpellAuraHolder* holder)
{
    SpellEntry const* aurSpellInfo = holder->GetSpellProto();

    // ghost spell check, allow apply any auras at player loading in ghost mode (will be cleanup after load)
    if (!isAlive() && !IsDeathPersistentSpell(aurSpellInfo) &&
        !IsDeathOnlySpell(aurSpellInfo) && !aurSpellInfo->HasAttribute(SPELL_ATTR_EX2_CAN_TARGET_DEAD) &&
        (GetTypeId() != TYPEID_PLAYER || !((Player*)this)->GetSession()->PlayerLoading()))
    {
        return false;
    }

    if (holder->GetTarget() != this)
    {
        sLog.outError("Holder (spell %u) add to spell aura holder list of %s (lowguid: %u) but spell aura holder target is %s (lowguid: %u)",
                      holder->GetId(), (GetTypeId() == TYPEID_PLAYER ? "player" : "creature"), GetGUIDLow(),
                      (holder->GetTarget()->GetTypeId() == TYPEID_PLAYER ? "player" : "creature"), holder->GetTarget()->GetGUIDLow());
        return false;
    }

    // passive and persistent auras can stack with themselves any number of times
    if ((!holder->IsPassive() && !holder->IsPersistent()) || holder->IsAreaAura())
    {
        SpellAuraHolderBounds spair = GetSpellAuraHolderBounds(aurSpellInfo->Id);

        // take out same spell
        for (SpellAuraHolderMap::iterator iter = spair.first; iter != spair.second; ++iter)
        {
            SpellAuraHolder* foundHolder = iter->second;
            if (foundHolder->GetCasterGuid() == holder->GetCasterGuid())
            {
                // Aura can stack on self -> Stack it;
                if (aurSpellInfo->StackAmount)
                {
                    // can be created with >1 stack by some spell mods
                    foundHolder->ModStackAmount(holder->GetStackAmount());
                    return false;
                }

                // Check for coexisting Weapon-proced Auras
                if (holder->IsWeaponBuffCoexistableWith(foundHolder))
                    continue;

                // can be only single
                RemoveSpellAuraHolder(foundHolder, AURA_REMOVE_BY_STACK);
                break;
            }
            else
            {
                //any stackable case with amount should mod existing stack amount
                if (aurSpellInfo->StackAmount && aurSpellInfo->rangeIndex != SPELL_RANGE_IDX_SELF_ONLY && !aurSpellInfo->HasAttribute(SPELL_ATTR_EX3_STACK_FOR_DIFF_CASTERS))
                {
                    foundHolder->ModStackAmount(holder->GetStackAmount());
                    return false;
                }
                else if (!IsStackableSpell(aurSpellInfo, foundHolder->GetSpellProto(), holder->GetTarget()))
                {
                    RemoveSpellAuraHolder(foundHolder, AURA_REMOVE_BY_STACK);
                    break;
                }
            }
        }
    }

    // normal spell or passive auras not stackable with other ranks
    if (!IsPassiveSpell(aurSpellInfo) || !IsPassiveSpellStackableWithRanks(aurSpellInfo))
    {
        if (!RemoveNoStackAurasDueToAuraHolder(holder))
        {
            return false;                                   // couldn't remove conflicting aura with higher rank
        }
    }

    // update tracked aura targets list (before aura add to aura list, to prevent unexpected remove recently added aura)
    if (TrackedAuraType trackedType = holder->GetTrackedAuraType())
    {
        if (Unit* caster = holder->GetCaster())             // caster not in world
        {
            // Only compare TrackedAuras of same tracking type
            TrackedAuraTargetMap& scTargets = caster->GetTrackedAuraTargets(trackedType);
            for (TrackedAuraTargetMap::iterator itr = scTargets.begin(); itr != scTargets.end();)
            {
                SpellEntry const* itr_spellEntry = itr->first;
                ObjectGuid itr_targetGuid = itr->second;    // Target on whom the tracked aura is

                if (itr_targetGuid == GetObjectGuid())      // Note: I don't understand this check (based on old aura concepts, kept when adding holders)
                {
                    ++itr;
                    continue;
                }

                bool removed = false;
                switch (trackedType)
                {
                    case TRACK_AURA_TYPE_SINGLE_TARGET:
                        if (IsSingleTargetSpells(itr_spellEntry, aurSpellInfo))
                        {
                            removed = true;
                            // remove from target if target found
                            if (Unit* itr_target = GetMap()->GetUnit(itr_targetGuid))
                                itr_target->RemoveAurasDueToSpell(itr_spellEntry->Id); // TODO AURA_REMOVE_BY_TRACKING (might require additional work elsewhere)
                            else                            // Normally the tracking will be removed by the AuraRemoval
                                scTargets.erase(itr);
                        }
                        break;
                    case TRACK_AURA_TYPE_NOT_TRACKED:       // These two can never happen
                    case MAX_TRACKED_AURA_TYPES:
                        MANGOS_ASSERT(false);
                        break;
                }

                if (removed)
                {
                    itr = scTargets.begin();                // list can be chnaged at remove aura
                    continue;
                }

                ++itr;
            }

            // TODO remove this switch after confirming that we dont need other cases
            switch (trackedType)
            {
                case TRACK_AURA_TYPE_SINGLE_TARGET:         // Register spell holder single target
                    scTargets[aurSpellInfo] = GetObjectGuid();
                    break;
                default:
                    break;
            }
        }
    }

    // add aura, register in lists and arrays
    holder->_AddSpellAuraHolder();
    m_spellAuraHolders.insert(SpellAuraHolderMap::value_type(holder->GetId(), holder));

    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (Aura* aur = holder->GetAuraByEffectIndex(SpellEffectIndex(i)))
            AddAuraToModList(aur);

    holder->ApplyAuraModifiers(true, true);                 // This is the place where auras are actually applied onto the target
    DEBUG_LOG("Holder of spell %u now is in use", holder->GetId());

    // if aura deleted before boosts apply ignore
    // this can be possible it it removed indirectly by triggered spell effect at ApplyModifier
    if (!holder->IsDeleted())
        holder->HandleSpellSpecificBoosts(true);

    return true;
}

void Unit::AddAuraToModList(Aura* aura)
{
    if (aura->GetModifier()->m_auraname < TOTAL_AURAS)
        m_modAuras[aura->GetModifier()->m_auraname].push_back(aura);
}

void Unit::RemoveRankAurasDueToSpell(uint32 spellId)
{
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo)
        return;
    SpellAuraHolderMap::const_iterator i, next;
    for (i = m_spellAuraHolders.begin(); i != m_spellAuraHolders.end(); i = next)
    {
        next = i;
        ++next;
        uint32 i_spellId = (*i).second->GetId();
        if ((*i).second && i_spellId && i_spellId != spellId)
        {
            if (sSpellMgr.IsRankSpellDueToSpell(spellInfo, i_spellId))
            {
                RemoveAurasDueToSpell(i_spellId);

                if (m_spellAuraHolders.empty())
                    break;
                else
                    next =  m_spellAuraHolders.begin();
            }
        }
    }
}

bool Unit::RemoveNoStackAurasDueToAuraHolder(SpellAuraHolder* holder)
{
    if (!holder)
        return false;

    SpellEntry const* spellProto = holder->GetSpellProto();
    if (!spellProto)
        return false;

    // passive spell special case (only non stackable with ranks)
    if (IsPassiveSpell(spellProto))
    {
        if (IsPassiveSpellStackableWithRanks(spellProto))
            return true;
    }

    const uint32 spellId = holder->GetId();
    const SpellSpecific specific = GetSpellSpecific(spellId);
    SpellEntry const* triggeredBy = holder->GetTriggeredBy();

    SpellAuraHolderMap::iterator i, next;
    for (i = m_spellAuraHolders.begin(); i != m_spellAuraHolders.end(); i = next)
    {
        next = i;
        ++next;
        if (!(*i).second) continue;

        SpellAuraHolder* existing = (*i).second;
        SpellEntry const* existingSpellProto = existing->GetSpellProto();

        if (!existingSpellProto)
            continue;

        const uint32 existingSpellId = existingSpellProto->Id;
        const SpellSpecific existingSpecific = GetSpellSpecific(existingSpellId);
        const bool own = (holder->GetCasterGuid() == existing->GetCasterGuid());

        // early checks that spellId is passive non stackable spell
        if (IsPassiveSpell(existingSpellProto))
        {
            // passive non-stackable spells not stackable only for same caster
            if (!own)
                continue;

            // passive non-stackable spells not stackable only with another rank of same spell
            if (!sSpellMgr.IsRankSpellDueToSpell(spellProto, existingSpellId))
                continue;
        }

        // prevent triggering aura of removing aura that triggered it
        if ((existing->GetTriggeredBy() && existing->GetTriggeredBy()->Id == spellId)
            || (holder->GetTriggeredBy() && holder->GetTriggeredBy()->Id == existingSpellId))
            continue;

        bool unique = false;
        bool personal = false;
        if (specific && existingSpecific && IsSpellSpecificIdentical(specific, existingSpecific))


        {
            personal = IsSpellSpecificUniquePerCaster(specific);
            unique = (personal || IsSpellSpecificUniquePerTarget(specific));
        }

        const bool stackable = !sSpellMgr.IsNoStackSpellDueToSpell(spellProto, existingSpellProto);
        if (unique || !stackable)
        {
            // check if this spell can be triggered by any talent aura
            if (!unique && triggeredBy && sSpellMgr.IsSpellCanAffectSpell(triggeredBy, existingSpellProto))
                continue;

            if (!own)
            {
                if (personal && stackable)
                    continue;
                // holder cannot remove higher rank if it isn't from the same caster
                if (IsSimilarExistingAuraStronger(holder, existing) || (sSpellMgr.IsRankSpellDueToSpell(spellProto, existingSpellId) && sSpellMgr.IsHighRankOfSpell(existingSpellId, spellId)))
                    return false;
            }

            // Its a parent aura (create this aura in ApplyModifier)
            if (existing->IsInUse())
            {
                sLog.outError("SpellAuraHolder (Spell %u) is in process but attempt removed at SpellAuraHolder (Spell %u) adding, need add stack rule for Unit::RemoveNoStackAurasDueToAuraHolder", i->second->GetId(), holder->GetId());
                continue;
            }

            if (personal && stackable)
                RemoveAurasByCasterSpell(existingSpellId, holder->GetCasterGuid());
            else
                RemoveAurasDueToSpell(existingSpellId);

            if (m_spellAuraHolders.empty())
                break;
            else
                next = m_spellAuraHolders.begin();
        }
    }
    return true;
}

void Unit::RemoveAura(uint32 spellId, SpellEffectIndex effindex, Aura* except)
{
    SpellAuraHolderBounds spair = GetSpellAuraHolderBounds(spellId);
    for (SpellAuraHolderMap::iterator iter = spair.first; iter != spair.second;)
    {
        Aura* aur = iter->second->m_auras[effindex];
        if (aur && aur != except)
        {
            RemoveSingleAuraFromSpellAuraHolder(iter->second, effindex);
            // may remove holder
            spair = GetSpellAuraHolderBounds(spellId);
            iter = spair.first;
        }
        else
            ++iter;
    }
}
void Unit::RemoveAurasByCasterSpell(uint32 spellId, ObjectGuid casterGuid)
{
    SpellAuraHolderBounds spair = GetSpellAuraHolderBounds(spellId);
    for (SpellAuraHolderMap::iterator iter = spair.first; iter != spair.second;)
    {
        if (iter->second->GetCasterGuid() == casterGuid)
        {
            RemoveSpellAuraHolder(iter->second);
            spair = GetSpellAuraHolderBounds(spellId);
            iter = spair.first;
        }
        else
            ++iter;
    }
}

void Unit::RemoveSingleAuraFromSpellAuraHolder(uint32 spellId, SpellEffectIndex effindex, ObjectGuid casterGuid, AuraRemoveMode mode)
{
    SpellAuraHolderBounds spair = GetSpellAuraHolderBounds(spellId);
    for (SpellAuraHolderMap::iterator iter = spair.first; iter != spair.second;)
    {
        Aura* aur = iter->second->m_auras[effindex];
        if (aur && aur->GetCasterGuid() == casterGuid)
        {
            RemoveSingleAuraFromSpellAuraHolder(iter->second, effindex, mode);
            spair = GetSpellAuraHolderBounds(spellId);
            iter = spair.first;
        }
        else
            ++iter;
    }
}

void Unit::RemoveAuraHolderDueToSpellByDispel(uint32 spellId, uint32 stackAmount, ObjectGuid casterGuid, Unit* dispeller)
{
    RemoveAuraHolderFromStack(spellId, stackAmount, casterGuid, AURA_REMOVE_BY_DISPEL);
}

void Unit::RemoveAurasDueToSpellBySteal(uint32 spellId, ObjectGuid casterGuid, Unit* stealer)
{
    SpellAuraHolder* holder = GetSpellAuraHolder(spellId, casterGuid);
    SpellEntry const* spellProto = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    SpellAuraHolder* new_holder = CreateSpellAuraHolder(spellProto, stealer, this);

    // set its duration and maximum duration
    // max duration 2 minutes (in msecs)
    int32 dur = holder->GetAuraDuration();
    int32 max_dur = 2 * MINUTE * IN_MILLISECONDS;
    int32 new_max_dur = max_dur > dur ? dur : max_dur;
    new_holder->SetAuraMaxDuration(new_max_dur);
    new_holder->SetAuraDuration(new_max_dur);

    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        Aura* aur = holder->GetAuraByEffectIndex(SpellEffectIndex(i));

        if (!aur)
            continue;

        int32 basePoints = aur->GetBasePoints();
        // construct the new aura for the attacker - will never return nullptr, it's just a wrapper for
        // some different constructors
        Aura* new_aur = CreateAura(aur->GetSpellProto(), aur->GetEffIndex(), &basePoints, new_holder, stealer, this);

        // set periodic to do at least one tick (for case when original aura has been at last tick preparing)
        int32 periodic = aur->GetModifier()->periodictime;
        new_aur->GetModifier()->periodictime = periodic < new_max_dur ? periodic : new_max_dur;

        // add the new aura to stealer
        new_holder->AddAura(new_aur, new_aur->GetEffIndex());
    }

    if (holder->ModStackAmount(-1))
        // Remove aura as dispel
        RemoveSpellAuraHolder(holder, AURA_REMOVE_BY_DISPEL);

    // strange but intended behaviour: Stolen single target auras won't be treated as single targeted
    new_holder->SetTrackedAuraType(TRACK_AURA_TYPE_NOT_TRACKED);

    if (!stealer->AddSpellAuraHolder(new_holder))
        delete new_holder;
}

void Unit::RemoveAurasDueToSpellByCancel(uint32 spellId)
{
    SpellAuraHolderBounds spair = GetSpellAuraHolderBounds(spellId);
    for (SpellAuraHolderMap::iterator iter = spair.first; iter != spair.second;)
    {
        RemoveSpellAuraHolder(iter->second, AURA_REMOVE_BY_CANCEL);
        spair = GetSpellAuraHolderBounds(spellId);
        iter = spair.first;
    }
}

void Unit::RemoveAurasTriggeredBySpell(uint32 spellId, ObjectGuid casterGuid /*= ObjectGuid()*/)
{
    if (!spellId)
        return;

    SpellAuraHolderMap& auras = GetSpellAuraHolderMap();
    for (SpellAuraHolderMap::iterator itr = auras.begin(); itr != auras.end();)
    {
        const SpellEntry* entry = itr->second->GetTriggeredBy();
        if ((entry && entry->Id == spellId) && (casterGuid.IsEmpty() || casterGuid == itr->second->GetCasterGuid()))
        {
            RemoveSpellAuraHolder(itr->second);
            itr = auras.begin();
        }
        else
            ++itr;
    }
}

void Unit::RemoveAurasWithDispelType(DispelType type, ObjectGuid casterGuid)
{
    // Create dispel mask by dispel type
    uint32 dispelMask = GetDispellMask(type);
    // Dispel all existing auras vs current dispel type
    SpellAuraHolderMap& auras = GetSpellAuraHolderMap();
    for (SpellAuraHolderMap::iterator itr = auras.begin(); itr != auras.end();)
    {
        SpellEntry const* spell = itr->second->GetSpellProto();
        if (((1 << spell->Dispel) & dispelMask) && (!casterGuid || casterGuid == itr->second->GetCasterGuid()))
        {
            // Dispel aura
            RemoveAurasDueToSpell(spell->Id);
            itr = auras.begin();
        }
        else
            ++itr;
    }
}

void Unit::RemoveAuraHolderFromStack(uint32 spellId, uint32 stackAmount, ObjectGuid casterGuid, AuraRemoveMode mode)
{
    SpellAuraHolderBounds spair = GetSpellAuraHolderBounds(spellId);
    for (SpellAuraHolderMap::iterator iter = spair.first; iter != spair.second; ++iter)
    {
        if (!casterGuid || iter->second->GetCasterGuid() == casterGuid)
        {
            if (iter->second->ModStackAmount(-int32(stackAmount)))
            {
                RemoveSpellAuraHolder(iter->second, mode);
                break;
            }
        }
    }
}

void Unit::RemoveAurasDueToSpell(uint32 spellId, SpellAuraHolder* except, AuraRemoveMode mode)
{
    SpellAuraHolderBounds bounds = GetSpellAuraHolderBounds(spellId);
    for (SpellAuraHolderMap::iterator iter = bounds.first; iter != bounds.second;)
    {
        if (iter->second != except)
        {
            RemoveSpellAuraHolder(iter->second, mode);
            bounds = GetSpellAuraHolderBounds(spellId);
            iter = bounds.first;
        }
        else
            ++iter;
    }
}

void Unit::RemoveAurasDueToItemSpell(Item* castItem, uint32 spellId)
{
    SpellAuraHolderBounds bounds = GetSpellAuraHolderBounds(spellId);
    for (SpellAuraHolderMap::iterator iter = bounds.first; iter != bounds.second;)
    {
        if (iter->second->GetCastItemGuid() == castItem->GetObjectGuid())
        {
            RemoveSpellAuraHolder(iter->second);
            bounds = GetSpellAuraHolderBounds(spellId);
            iter = bounds.first;
        }
        else
            ++iter;
    }
}

void Unit::RemoveAurasWithInterruptFlags(uint32 flags)
{
    for (SpellAuraHolderMap::iterator iter = m_spellAuraHolders.begin(); iter != m_spellAuraHolders.end();)
    {
        if (iter->second->GetSpellProto()->AuraInterruptFlags & flags)
        {
            RemoveSpellAuraHolder(iter->second);
            iter = m_spellAuraHolders.begin();
        }
        else
            ++iter;
    }
}

void Unit::RemoveAurasWithAttribute(uint32 flags)
{
    for (SpellAuraHolderMap::iterator iter = m_spellAuraHolders.begin(); iter != m_spellAuraHolders.end();)
    {
        if (iter->second->GetSpellProto()->HasAttribute((SpellAttributes)flags))
        {
            RemoveSpellAuraHolder(iter->second);
            iter = m_spellAuraHolders.begin();
        }
        else
            ++iter;
    }
}

void Unit::RemoveAurasOnCast(SpellEntry const* castedSpellEntry)
{
    for (SpellAuraHolderMap::iterator iter = m_spellAuraHolders.begin(); iter != m_spellAuraHolders.end();)
    {
        SpellAuraHolder* holder = iter->second;
        SpellEntry const* spellEntry = holder->GetSpellProto();
        bool removeThisHolder = false;

        if (spellEntry->AuraInterruptFlags & AURA_INTERRUPT_FLAG_UNK2)
        {
            if (castedSpellEntry->HasAttribute(SPELL_ATTR_EX_NOT_BREAK_STEALTH))
            {
                bool foundStealth = false;
                for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
                {
                    if (Aura* aura = holder->m_auras[i])
                    {
                        if (aura->GetModifier()->m_auraname == SPELL_AURA_MOD_STEALTH)
                        {
                            foundStealth = true;
                            break;
                        }
                    }
                }
                removeThisHolder = !foundStealth;
            }
            else
                removeThisHolder = true;
        }

        if (removeThisHolder)
        {
            RemoveSpellAuraHolder(iter->second);
            iter = m_spellAuraHolders.begin();
        }
        else
            ++iter;
    }
}

void Unit::RemoveNotOwnTrackedTargetAuras()
{
    // tracked aura targets from other casters are removed if the phase does no more fit
    for (SpellAuraHolderMap::iterator iter = m_spellAuraHolders.begin(); iter != m_spellAuraHolders.end();)
    {
        TrackedAuraType trackedType = iter->second->GetTrackedAuraType();
        if (!trackedType)
        {
            ++iter;
            continue;
        }

        if (iter->second->GetCasterGuid() != GetObjectGuid())
        {
            RemoveSpellAuraHolder(iter->second);
            iter = m_spellAuraHolders.begin();
            continue;
        }

        ++iter;
    }

    // tracked aura targets at other targets
    for (uint8 type = TRACK_AURA_TYPE_SINGLE_TARGET; type < MAX_TRACKED_AURA_TYPES; ++type)
    {
        TrackedAuraTargetMap& scTargets = GetTrackedAuraTargets(TrackedAuraType(type));
        for (TrackedAuraTargetMap::iterator itr = scTargets.begin(); itr != scTargets.end();)
        {
            SpellEntry const* itr_spellEntry = itr->first;
            ObjectGuid itr_targetGuid = itr->second;

            if (itr_targetGuid != GetObjectGuid())
            {
                scTargets.erase(itr);                           // remove for caster in any case

                // remove from target if target found
                if (Unit* itr_target = GetMap()->GetUnit(itr_targetGuid))
                    itr_target->RemoveAurasByCasterSpell(itr_spellEntry->Id, GetObjectGuid());

                itr = scTargets.begin();                        // list can be changed at remove aura
                continue;
            }

            ++itr;
        }
    }
}

void Unit::RemoveSpellAuraHolder(SpellAuraHolder* holder, AuraRemoveMode mode)
{
    // Statue unsummoned at holder remove
    SpellEntry const* AurSpellInfo = holder->GetSpellProto();
    Totem* statue = nullptr;
    Unit* caster = holder->GetCaster();
    if (IsChanneledSpell(AurSpellInfo) && caster)
        if (caster->GetTypeId() == TYPEID_UNIT && ((Creature*)caster)->IsTotem() && ((Totem*)caster)->GetTotemType() == TOTEM_STATUE)
            statue = ((Totem*)caster);

    if (m_spellAuraHoldersUpdateIterator != m_spellAuraHolders.end() && m_spellAuraHoldersUpdateIterator->second == holder)
        ++m_spellAuraHoldersUpdateIterator;

    SpellAuraHolderBounds bounds = GetSpellAuraHolderBounds(holder->GetId());
    for (SpellAuraHolderMap::iterator itr = bounds.first; itr != bounds.second; ++itr)
    {
        if (itr->second == holder)
        {
            m_spellAuraHolders.erase(itr);
            break;
        }
    }

    holder->SetRemoveMode(mode);
    holder->UnregisterAndCleanupTrackedAuras();

    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (Aura* aura = holder->m_auras[i])
            RemoveAura(aura, mode);
    }

    holder->_RemoveSpellAuraHolder();

    if (mode != AURA_REMOVE_BY_DELETE)
        holder->HandleSpellSpecificBoosts(false);

    if (statue)
        statue->UnSummon();

    // If holder in use (removed from code that plan access to it data after return)
    // store it in holder list with delayed deletion
    if (holder->IsInUse())
    {
        holder->SetDeleted();
        m_deletedHolders.push_back(holder);
    }
    else
        delete holder;

    if (mode != AURA_REMOVE_BY_EXPIRE && IsChanneledSpell(AurSpellInfo) && !IsAreaOfEffectSpell(AurSpellInfo) &&
            caster && caster->GetObjectGuid() != GetObjectGuid())
    {
        caster->InterruptSpell(CURRENT_CHANNELED_SPELL);
    }
}

void Unit::RemoveSingleAuraFromSpellAuraHolder(SpellAuraHolder* holder, SpellEffectIndex index, AuraRemoveMode mode)
{
    Aura* aura = holder->GetAuraByEffectIndex(index);
    if (!aura)
        return;

    if (aura->IsLastAuraOnHolder())
        RemoveSpellAuraHolder(holder, mode);
    else
        RemoveAura(aura, mode);
}

void Unit::RemoveAura(Aura* Aur, AuraRemoveMode mode)
{
    // remove from list before mods removing (prevent cyclic calls, mods added before including to aura list - use reverse order)
    if (Aur->GetModifier()->m_auraname < TOTAL_AURAS)
    {
        m_modAuras[Aur->GetModifier()->m_auraname].remove(Aur);
    }

    // Set remove mode
    Aur->SetRemoveMode(mode);

    // some ShapeshiftBoosts at remove trigger removing other auras including parent Shapeshift aura
    // remove aura from list before to prevent deleting it before
    /// m_Auras.erase(i);

    DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Aura %u now is remove mode %d", Aur->GetModifier()->m_auraname, mode);

    // aura _MUST_ be remove from holder before unapply.
    // un-apply code expected that aura not find by diff searches
    // in another case it can be double removed for example, if target die/etc in un-apply process.
    Aur->GetHolder()->RemoveAura(Aur->GetEffIndex());

    // some auras also need to apply modifier (on caster) on remove
    if (mode == AURA_REMOVE_BY_DELETE)
    {
        switch (Aur->GetModifier()->m_auraname)
        {
            // need properly undo any auras with player-caster mover set (or will crash at next caster move packet)
            case SPELL_AURA_MOD_POSSESS:
            case SPELL_AURA_MOD_POSSESS_PET:
                Aur->ApplyModifier(false, true);
                break;
            default: break;
        }
    }
    else
        Aur->ApplyModifier(false, true);

    // If aura in use (removed from code that plan access to it data after return)
    // store it in aura list with delayed deletion
    if (Aur->IsInUse())
        m_deletedAuras.push_back(Aur);
    else
        delete Aur;
}

void Unit::RemoveAllAuras(AuraRemoveMode mode /*= AURA_REMOVE_BY_DEFAULT*/)
{
    while (!m_spellAuraHolders.empty())
    {
        SpellAuraHolderMap::iterator iter = m_spellAuraHolders.begin();
        RemoveSpellAuraHolder(iter->second, mode);
    }
}

void Unit::RemoveAllAurasOnDeath()
{
    // used just after dieing to remove all visible auras
    // and disable the mods for the passive ones
    for (SpellAuraHolderMap::iterator iter = m_spellAuraHolders.begin(); iter != m_spellAuraHolders.end();)
    {
        if (!iter->second->IsPassive() && !iter->second->IsDeathPersistent())
        {
            RemoveSpellAuraHolder(iter->second, AURA_REMOVE_BY_DEATH);
            iter = m_spellAuraHolders.begin();
        }
        else
            ++iter;
    }
}

void Unit::RemoveAllAurasOnEvade()
{
    // used when evading to remove all auras except some special auras
    // Linked and flying auras should not be removed on evade
    for (SpellAuraHolderMap::iterator iter = m_spellAuraHolders.begin(); iter != m_spellAuraHolders.end();)
    {
        // Note: for the moment this part of the function is used as a placeholder to keep in sync with master branch
        /*SpellEntry const* proto = iter->second->GetSpellProto();
        if (!IsSpellHaveAura(proto, SPELL_AURA_CONTROL_VEHICLE))
        {
            RemoveSpellAuraHolder(iter->second, AURA_REMOVE_BY_DEFAULT);
            iter = m_spellAuraHolders.begin();
        }
        else
            ++iter;*/

        RemoveSpellAuraHolder(iter->second, AURA_REMOVE_BY_DEFAULT);
        iter = m_spellAuraHolders.begin();
    }
}

void Unit::DelaySpellAuraHolder(uint32 spellId, int32 delaytime, ObjectGuid casterGuid)
{
    SpellAuraHolderBounds bounds = GetSpellAuraHolderBounds(spellId);
    for (SpellAuraHolderMap::iterator iter = bounds.first; iter != bounds.second; ++iter)
    {
        SpellAuraHolder* holder = iter->second;

        if (casterGuid != holder->GetCasterGuid())
            continue;

        if (holder->GetAuraDuration() < delaytime)
            holder->SetAuraDuration(0);
        else
            holder->SetAuraDuration(holder->GetAuraDuration() - delaytime);

        holder->UpdateAuraDuration();

        DEBUG_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Spell %u partially interrupted on %s, new duration: %u ms", spellId, GetGuidStr().c_str(), holder->GetAuraDuration());
    }
}

void Unit::_RemoveAllAuraMods()
{
    for (SpellAuraHolderMap::const_iterator i = m_spellAuraHolders.begin(); i != m_spellAuraHolders.end(); ++i)
    {
        (*i).second->ApplyAuraModifiers(false);
    }
}

void Unit::_ApplyAllAuraMods()
{
    for (SpellAuraHolderMap::const_iterator i = m_spellAuraHolders.begin(); i != m_spellAuraHolders.end(); ++i)
    {
        (*i).second->ApplyAuraModifiers(true);
    }
}

bool Unit::HasAuraType(AuraType auraType) const
{
    return !GetAurasByType(auraType).empty();
}

bool Unit::HasAffectedAura(AuraType auraType, SpellEntry const* spellProto) const
{
    Unit::AuraList const& auras = GetAurasByType(auraType);

    for (Unit::AuraList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
    {
        if ((*itr)->isAffectedOnSpell(spellProto))
            return true;
    }

    return false;
}

Aura* Unit::GetAura(uint32 spellId, SpellEffectIndex effindex)
{
    SpellAuraHolderBounds bounds = GetSpellAuraHolderBounds(spellId);
    if (bounds.first != bounds.second)
        return bounds.first->second->GetAuraByEffectIndex(effindex);
    return nullptr;
}

Aura* Unit::GetAura(AuraType type, SpellFamily family, uint64 familyFlag, ObjectGuid casterGuid)
{
    AuraList const& auras = GetAurasByType(type);
    for (AuraList::const_iterator i = auras.begin(); i != auras.end(); ++i)
        if ((*i)->GetSpellProto()->IsFitToFamily(family, familyFlag) &&
                (!casterGuid || (*i)->GetCasterGuid() == casterGuid))
            return *i;

    return nullptr;
}

bool Unit::HasAura(uint32 spellId, SpellEffectIndex effIndex) const
{
    SpellAuraHolderConstBounds spair = GetSpellAuraHolderBounds(spellId);
    for (SpellAuraHolderMap::const_iterator i_holder = spair.first; i_holder != spair.second; ++i_holder)
        if (i_holder->second->GetAuraByEffectIndex(effIndex))
            return true;

    return false;
}

void Unit::AddDynObject(DynamicObject* dynObj)
{
    m_dynObjGUIDs.push_back(dynObj->GetObjectGuid());
}

void Unit::RemoveDynObject(uint32 spellid)
{
    if (m_dynObjGUIDs.empty())
        return;
    for (GuidList::iterator i = m_dynObjGUIDs.begin(); i != m_dynObjGUIDs.end();)
    {
        DynamicObject* dynObj = GetMap()->GetDynamicObject(*i);
        if (!dynObj)
        {
            i = m_dynObjGUIDs.erase(i);
        }
        else if (spellid == 0 || dynObj->GetSpellId() == spellid)
        {
            dynObj->Delete();
            i = m_dynObjGUIDs.erase(i);
        }
        else
            ++i;
    }
}

void Unit::RemoveAllDynObjects()
{
    while (!m_dynObjGUIDs.empty())
    {
        if (DynamicObject* dynObj = GetMap()->GetDynamicObject(*m_dynObjGUIDs.begin()))
            dynObj->Delete();
        m_dynObjGUIDs.erase(m_dynObjGUIDs.begin());
    }
}

DynamicObject* Unit::GetDynObject(uint32 spellId, SpellEffectIndex effIndex)
{
    for (GuidList::iterator i = m_dynObjGUIDs.begin(); i != m_dynObjGUIDs.end();)
    {
        DynamicObject* dynObj = GetMap()->GetDynamicObject(*i);
        if (!dynObj)
        {
            i = m_dynObjGUIDs.erase(i);
            continue;
        }

        if (dynObj->GetSpellId() == spellId && dynObj->GetEffIndex() == effIndex)
            return dynObj;
        ++i;
    }
    return nullptr;
}

DynamicObject* Unit::GetDynObject(uint32 spellId)
{
    for (GuidList::iterator i = m_dynObjGUIDs.begin(); i != m_dynObjGUIDs.end();)
    {
        DynamicObject* dynObj = GetMap()->GetDynamicObject(*i);
        if (!dynObj)
        {
            i = m_dynObjGUIDs.erase(i);
            continue;
        }

        if (dynObj->GetSpellId() == spellId)
            return dynObj;
        ++i;
    }
    return nullptr;
}

GameObject* Unit::GetGameObject(uint32 spellId) const
{
    for (GameObjectList::const_iterator i = m_gameObj.begin(); i != m_gameObj.end(); ++i)
        if ((*i)->GetSpellId() == spellId)
            return *i;

    WildGameObjectMap::const_iterator find = m_wildGameObjs.find(spellId);
    if (find != m_wildGameObjs.end())
        return GetMap()->GetGameObject(find->second);       // Can be nullptr

    return nullptr;
}

void Unit::AddGameObject(GameObject* gameObj)
{
    MANGOS_ASSERT(gameObj && !gameObj->GetOwnerGuid());
    m_gameObj.push_back(gameObj);
    gameObj->SetOwnerGuid(GetObjectGuid());

    if (GetTypeId() == TYPEID_PLAYER && gameObj->GetSpellId())
    {
        SpellEntry const* createBySpell = sSpellTemplate.LookupEntry<SpellEntry>(gameObj->GetSpellId());
        // Need disable spell use for owner
        if (createBySpell && createBySpell->HasAttribute(SPELL_ATTR_DISABLED_WHILE_ACTIVE))
            // note: item based cooldowns and cooldown spell mods with charges ignored (unknown existing cases)
            ((Player*)this)->AddSpellAndCategoryCooldowns(createBySpell, 0, nullptr, true);
    }
}

void Unit::AddWildGameObject(GameObject* gameObj)
{
    MANGOS_ASSERT(gameObj && gameObj->GetOwnerGuid().IsEmpty());
    m_wildGameObjs[gameObj->GetSpellId()] = gameObj->GetObjectGuid();

    // As of 335 there are no wild-summon spells with SPELL_ATTR_DISABLED_WHILE_ACTIVE

    // Remove outdated wild summoned GOs
    for (WildGameObjectMap::iterator itr = m_wildGameObjs.begin(); itr != m_wildGameObjs.end();)
    {
        GameObject* pGo = GetMap()->GetGameObject(itr->second);
        if (pGo)
            ++itr;
        else
            m_wildGameObjs.erase(itr++);
    }
}

void Unit::RemoveGameObject(GameObject* gameObj, bool del)
{
    MANGOS_ASSERT(gameObj && gameObj->GetOwnerGuid() == GetObjectGuid());

    gameObj->SetOwnerGuid(ObjectGuid());

    // GO created by some spell
    if (uint32 spellid = gameObj->GetSpellId())
    {
        RemoveAurasDueToSpell(spellid);

        if (GetTypeId() == TYPEID_PLAYER)
        {
            SpellEntry const* createBySpell = sSpellTemplate.LookupEntry<SpellEntry>(spellid);
            // Need activate spell use for owner
            if (createBySpell && createBySpell->HasAttribute(SPELL_ATTR_DISABLED_WHILE_ACTIVE))
                // note: item based cooldowns and cooldown spell mods with charges ignored (unknown existing cases)
                ((Player*)this)->SendCooldownEvent(createBySpell);
        }
    }

    m_gameObj.remove(gameObj);

    if (del)
    {
        gameObj->SetRespawnTime(0);
        gameObj->Delete();
    }
}

void Unit::RemoveGameObject(uint32 spellid, bool del)
{
    if (m_gameObj.empty())
        return;

    GameObjectList::iterator i, next;
    for (i = m_gameObj.begin(); i != m_gameObj.end(); i = next)
    {
        next = i;
        if (spellid == 0 || (*i)->GetSpellId() == spellid)
        {
            (*i)->SetOwnerGuid(ObjectGuid());
            if (del)
            {
                (*i)->SetRespawnTime(0);
                (*i)->Delete();
            }

            next = m_gameObj.erase(i);
        }
        else
            ++next;
    }
}

void Unit::RemoveAllGameObjects()
{
    // remove references to unit
    for (GameObjectList::iterator i = m_gameObj.begin(); i != m_gameObj.end();)
    {
        (*i)->SetOwnerGuid(ObjectGuid());
        (*i)->SetRespawnTime(0);
        (*i)->Delete();
        i = m_gameObj.erase(i);
    }

    // wild summoned GOs - only remove references, do not remove GOs
    m_wildGameObjs.clear();
}

void Unit::SendSpellNonMeleeDamageLog(SpellNonMeleeDamage* log) const
{
    WorldPacket data(SMSG_SPELLNONMELEEDAMAGELOG, (16 + 4 + 4 + 1 + 4 + 4 + 1 + 1 + 4 + 4 + 1)); // we guess size
    data << log->target->GetPackGUID();
    data << log->attacker->GetPackGUID();
    data << uint32(log->SpellID);
    data << uint32(log->damage);                            // damage amount
    data << uint8(log->school);                             // damage school
    data << uint32(log->absorb);                            // AbsorbedDamage
    data << uint32(log->resist);                            // resist
    data << uint8(log->physicalLog);                        // if 1, then client show spell name (example: %s's ranged shot hit %s for %u school or %s suffers %u school damage from %s's spell_name
    data << uint8(log->unused);                             // unused
    data << uint32(log->blocked);                           // blocked
    data << uint32(log->HitInfo);
    data << uint8(0);                                       // flag to use extend data
    SendMessageToSet(data, true);
}

void Unit::SendSpellNonMeleeDamageLog(Unit* target, uint32 SpellID, uint32 Damage, SpellSchoolMask damageSchoolMask, uint32 AbsorbedDamage, uint32 Resist, bool PhysicalDamage, uint32 Blocked, bool CriticalHit)
{
    SpellNonMeleeDamage log(this, target, SpellID, GetFirstSchoolInMask(damageSchoolMask));
    log.damage = Damage - AbsorbedDamage - Resist - Blocked;
    log.absorb = AbsorbedDamage;
    log.resist = Resist;
    log.physicalLog = PhysicalDamage;
    log.blocked = Blocked;
    log.HitInfo = SPELL_HIT_TYPE_UNK1 | SPELL_HIT_TYPE_UNK3 | SPELL_HIT_TYPE_UNK6;
    if (CriticalHit)
        log.HitInfo |= SPELL_HIT_TYPE_CRIT;
    SendSpellNonMeleeDamageLog(&log);
}

void Unit::SendPeriodicAuraLog(SpellPeriodicAuraLogInfo* pInfo) const
{
    Aura* aura = pInfo->aura;
    Modifier* mod = aura->GetModifier();

    WorldPacket data(SMSG_PERIODICAURALOG, 30);
    data << aura->GetTarget()->GetPackGUID();
    data << aura->GetCasterGuid().WriteAsPacked();
    data << uint32(aura->GetId());                          // spellId
    data << uint32(1);                                      // count
    data << uint32(mod->m_auraname);                        // auraId
    switch (mod->m_auraname)
    {
        case SPELL_AURA_PERIODIC_DAMAGE:
        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
            data << uint32(pInfo->damage);                  // damage
            data << uint32(aura->GetSpellProto()->School);
            data << uint32(pInfo->absorb);                  // absorb
            data << uint32(pInfo->resist);                  // resist
            break;
        case SPELL_AURA_PERIODIC_HEAL:
        case SPELL_AURA_OBS_MOD_HEALTH:
            data << uint32(pInfo->damage);                  // damage
            break;
        case SPELL_AURA_OBS_MOD_MANA:
        case SPELL_AURA_PERIODIC_ENERGIZE:
            data << uint32(mod->m_miscvalue);               // power type
            data << uint32(pInfo->damage);                  // damage
            break;
        case SPELL_AURA_PERIODIC_MANA_LEECH:
            data << uint32(mod->m_miscvalue);               // power type
            data << uint32(pInfo->damage);                  // amount
            data << float(pInfo->multiplier);               // gain multiplier
            break;
        default:
            sLog.outError("Unit::SendPeriodicAuraLog: unknown aura %u", uint32(mod->m_auraname));
            return;
    }

    aura->GetTarget()->SendMessageToSet(data, true);
}

void Unit::ProcDamageAndSpell(Unit* pVictim, uint32 procAttacker, uint32 procVictim, uint32 procExtra, uint32 amount, WeaponAttackType attType, SpellEntry const* procSpell)
{
    // Not much to do if no flags are set.
    if (procAttacker)
        ProcDamageAndSpellFor(false, pVictim, procAttacker, procExtra, attType, procSpell, amount);
    // Now go on with a victim's events'n'auras
    // Not much to do if no flags are set or there is no victim
    if (pVictim && pVictim->isAlive() && procVictim)
        pVictim->ProcDamageAndSpellFor(true, this, procVictim, procExtra, attType, procSpell, amount);
}

void Unit::SendSpellMiss(Unit* target, uint32 spellID, SpellMissInfo missInfo) const
{
    WorldPacket data(SMSG_SPELLLOGMISS, (4 + 8 + 1 + 4 + 8 + 1));
    data << uint32(spellID);
    data << GetObjectGuid();
    data << uint8(0);                                       // can be 0 or 1
    data << uint32(1);                                      // target count
    // for(i = 0; i < target count; ++i)
    data << target->GetObjectGuid();                        // target GUID
    data << uint8(missInfo);
    // end loop
    SendMessageToSet(data, true);
}

void Unit::SendAttackStateUpdate(CalcDamageInfo* damageInfo) const
{
    DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "WORLD: Sending SMSG_ATTACKERSTATEUPDATE");

    WorldPacket data(SMSG_ATTACKERSTATEUPDATE, (16 + 45));  // we guess size

    data << uint32(damageInfo->HitInfo);
    data << GetPackGUID();
    data << damageInfo->target->GetPackGUID();
    data << uint32(damageInfo->totalDamage);    // Total damage

    data << uint8(m_weaponDamageCount[damageInfo->attackType]);         // Sub damage count

    // Sub damage description
    for (uint8 i = 0; i < m_weaponDamageCount[damageInfo->attackType]; i++)
    {
        SubDamageInfo *subDamage = &damageInfo->subDamage[i];

        data << uint32(GetFirstSchoolInMask(subDamage->damageSchoolMask));
        data << float(subDamage->damage)/ float(damageInfo->totalDamage);        // Float coefficient of sub damage
        data << uint32(subDamage->damage);
        data << uint32(subDamage->absorb);
        data << uint32(subDamage->resist);
    }
    data << uint32(damageInfo->TargetState);
    data << uint32(0);
    data << uint32(0);                                      // spell id, seen with heroic strike and disarm as examples.
    // HITINFO_NOACTION normally set if spell
    data << uint32(damageInfo->blocked_amount);

    SendMessageToSet(data, true);
}

void Unit::SendAttackStateUpdate(uint32 HitInfo, Unit* target, SpellSchoolMask damageSchoolMask, uint32 Damage, uint32 AbsorbDamage, uint32 Resist, VictimState TargetState, uint32 BlockedAmount)
{
    CalcDamageInfo dmgInfo;
    dmgInfo.HitInfo = HitInfo;
    dmgInfo.attacker = this;
    dmgInfo.target = target;
    dmgInfo.attackType = BASE_ATTACK;
    dmgInfo.totalDamage = Damage - AbsorbDamage - Resist - BlockedAmount;
    dmgInfo.subDamage[0].damage = dmgInfo.totalDamage;
    dmgInfo.subDamage[0].damageSchoolMask = damageSchoolMask;
    dmgInfo.subDamage[0].absorb = AbsorbDamage;
    dmgInfo.subDamage[0].resist = Resist;
    dmgInfo.TargetState = TargetState;
    dmgInfo.blocked_amount = BlockedAmount;
    SendAttackStateUpdate(&dmgInfo);
}

void Unit::SetPowerType(Powers new_powertype)
{
    // set power type
    SetByteValue(UNIT_FIELD_BYTES_0, 3, new_powertype);

    // group updates
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (((Player*)this)->GetGroup())
            ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_POWER_TYPE);
    }
    else if (((Creature*)this)->IsPet())
    {
        Pet* pet = ((Pet*)this);
        if (pet->isControlled())
        {
            Unit* owner = GetOwner();
            if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && ((Player*)owner)->GetGroup())
                ((Player*)owner)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_POWER_TYPE);
        }
    }

    // special cases for power type switching (druid and pets only)
    if (GetTypeId() == TYPEID_PLAYER || (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->IsPet()))
    {
        uint32 maxValue = GetCreatePowers(new_powertype);
        uint32 curValue = maxValue;

        // special cases with current power = 0
        if (new_powertype == POWER_RAGE)
            curValue = 0;

        // set power (except for mana)
        if (new_powertype != POWER_MANA)
        {
            SetMaxPower(new_powertype, maxValue);
            SetPower(new_powertype, curValue);
        }
    }
}

FactionTemplateEntry const* Unit::getFactionTemplateEntry() const
{
    FactionTemplateEntry const* entry = sFactionTemplateStore.LookupEntry(getFaction());
    if (!entry)
    {
        static ObjectGuid guid;                             // prevent repeating spam same faction problem

        if (GetObjectGuid() != guid)
        {
            guid = GetObjectGuid();

            if (guid.GetHigh() == HIGHGUID_PET)
                sLog.outError("%s (base creature entry %u) have invalid faction template id %u, owner %s", GetGuidStr().c_str(), GetEntry(), getFaction(), ((Pet*)this)->GetOwnerGuid().GetString().c_str());
            else
                sLog.outError("%s have invalid faction template id %u", GetGuidStr().c_str(), getFaction());
        }
    }
    return entry;
}

bool Unit::IsHostileTo(Unit const* unit) const
{
    // always non-hostile to self
    if (unit == this)
        return false;

    // always non-hostile to GM in GM mode
    if (unit->GetTypeId() == TYPEID_PLAYER && ((Player const*)unit)->isGameMaster())
        return false;

    // always hostile to enemy
    if (getVictim() == unit || unit->getVictim() == this)
        return true;

    // test pet/charm masters instead pers/charmeds
    Unit const* testerOwner = GetCharmerOrOwner();
    Unit const* targetOwner = unit->GetCharmerOrOwner();

    // always hostile to owner's enemy
    if (testerOwner && (testerOwner->getVictim() == unit || unit->getVictim() == testerOwner))
        return true;

    // always hostile to enemy owner
    if (targetOwner && (getVictim() == targetOwner || targetOwner->getVictim() == this))
        return true;

    // always hostile to owner of owner's enemy
    if (testerOwner && targetOwner && (testerOwner->getVictim() == targetOwner || targetOwner->getVictim() == testerOwner))
        return true;

    Unit const* tester = testerOwner ? testerOwner : this;
    Unit const* target = targetOwner ? targetOwner : unit;

    // always non-hostile to target with common owner, or to owner/pet
    if (tester == target)
        return false;

    // special cases (Duel, etc)
    if (tester->GetTypeId() == TYPEID_PLAYER && target->GetTypeId() == TYPEID_PLAYER)
    {
        Player const* pTester = (Player const*)tester;
        Player const* pTarget = (Player const*)target;

        // Duel
        if (pTester->IsInDuelWith(pTarget))
            return true;

        // Group
        if (pTester->GetGroup() && pTester->GetGroup() == pTarget->GetGroup())
            return false;

        // Sanctuary
        if (pTarget->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY) && pTester->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY))
            return false;

        // PvP FFA state
        if (pTester->IsFFAPvP() && pTarget->IsFFAPvP())
            return true;

        //= PvP states
        // Green/Blue (can't attack)
        if (pTester->GetTeam() == pTarget->GetTeam())
            return false;

        // Red (can attack) if true, Blue/Yellow (can't attack) in another case
        return pTester->IsPvP() && pTarget->IsPvP();
    }

    // faction base cases
    FactionTemplateEntry const* tester_faction = tester->getFactionTemplateEntry();
    FactionTemplateEntry const* target_faction = target->getFactionTemplateEntry();
    if (!tester_faction || !target_faction)
        return false;

    if (target->isAttackingPlayer() && tester->IsContestedGuard())
        return true;

    // PvC forced reaction and reputation case
    if (tester->GetTypeId() == TYPEID_PLAYER)
    {
        if (target_faction->faction)
        {
            // forced reaction
            if (ReputationRank const* force = ((Player*)tester)->GetReputationMgr().GetForcedRankIfAny(target_faction))
                return *force <= REP_HOSTILE;

            // if faction have reputation then hostile state for tester at 100% dependent from at_war state
            if (FactionEntry const* raw_target_faction = sFactionStore.LookupEntry(target_faction->faction))
                if (FactionState const* factionState = ((Player*)tester)->GetReputationMgr().GetState(raw_target_faction))
                    return !!(factionState->Flags & FACTION_FLAG_AT_WAR);
        }
    }
    // CvP forced reaction and reputation case
    else if (target->GetTypeId() == TYPEID_PLAYER)
    {
        if (tester_faction->faction)
        {
            // forced reaction
            if (ReputationRank const* force = ((Player*)target)->GetReputationMgr().GetForcedRankIfAny(tester_faction))
                return *force <= REP_HOSTILE;

            // apply reputation state
            FactionEntry const* raw_tester_faction = sFactionStore.LookupEntry(tester_faction->faction);
            if (raw_tester_faction && raw_tester_faction->reputationListID >= 0)
                return ((Player const*)target)->GetReputationMgr().GetRank(raw_tester_faction) <= REP_HOSTILE;
        }
    }

    // common faction based case (CvC,PvC,CvP)
    return tester_faction->IsHostileTo(*target_faction);
}

bool Unit::IsFriendlyTo(Unit const* unit) const
{
    // always friendly to self
    if (unit == this)
        return true;

    // always friendly to GM in GM mode
    if (unit->GetTypeId() == TYPEID_PLAYER && ((Player const*)unit)->isGameMaster())
        return true;

    // always non-friendly to enemy
    if (getVictim() == unit || unit->getVictim() == this)
        return false;

    // test pet/charm masters instead pers/charmeds
    Unit const* testerOwner = GetCharmerOrOwner();
    Unit const* targetOwner = unit->GetCharmerOrOwner();

    // always non-friendly to owner's enemy
    if (testerOwner && (testerOwner->getVictim() == unit || unit->getVictim() == testerOwner))
        return false;

    // always non-friendly to enemy owner
    if (targetOwner && (getVictim() == targetOwner || targetOwner->getVictim() == this))
        return false;

    // always non-friendly to owner of owner's enemy
    if (testerOwner && targetOwner && (testerOwner->getVictim() == targetOwner || targetOwner->getVictim() == testerOwner))
        return false;

    Unit const* tester = testerOwner ? testerOwner : this;
    Unit const* target = targetOwner ? targetOwner : unit;

    // always friendly to target with common owner, or to owner/pet
    if (tester == target)
        return true;

    // special cases (Duel)
    if (tester->GetTypeId() == TYPEID_PLAYER && target->GetTypeId() == TYPEID_PLAYER)
    {
        Player const* pTester = (Player const*)tester;
        Player const* pTarget = (Player const*)target;

        // Duel
        if (pTester->IsInDuelWith(pTarget))
            return false;

        // Group
        if (pTester->GetGroup() && pTester->GetGroup() == pTarget->GetGroup())
            return true;

        // Sanctuary
        if (pTarget->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY) && pTester->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY))
            return true;

        // PvP FFA state
        if (pTester->IsFFAPvP() && pTarget->IsFFAPvP())
            return false;

        //= PvP states
        // Green/Blue (non-attackable)
        if (pTester->GetTeam() == pTarget->GetTeam())
            return true;

        // Blue (friendly/non-attackable) if not PVP, or Yellow/Red in another case (attackable)
        return !pTarget->IsPvP();
    }

    // faction base cases
    FactionTemplateEntry const* tester_faction = tester->getFactionTemplateEntry();
    FactionTemplateEntry const* target_faction = target->getFactionTemplateEntry();
    if (!tester_faction || !target_faction)
        return false;

    if (target->isAttackingPlayer() && tester->IsContestedGuard())
        return false;

    // PvC forced reaction and reputation case
    if (tester->GetTypeId() == TYPEID_PLAYER)
    {
        if (target_faction->faction)
        {
            // forced reaction
            if (ReputationRank const* force = ((Player*)tester)->GetReputationMgr().GetForcedRankIfAny(target_faction))
                return *force >= REP_FRIENDLY;

            // if faction have reputation then friendly state for tester at 100% dependent from at_war state
            if (FactionEntry const* raw_target_faction = sFactionStore.LookupEntry(target_faction->faction))
                if (FactionState const* factionState = ((Player*)tester)->GetReputationMgr().GetState(raw_target_faction))
                    return !(factionState->Flags & FACTION_FLAG_AT_WAR);
        }
    }
    // CvP forced reaction and reputation case
    else if (target->GetTypeId() == TYPEID_PLAYER)
    {
        if (tester_faction->faction)
        {
            // forced reaction
            if (ReputationRank const* force = ((Player*)target)->GetReputationMgr().GetForcedRankIfAny(tester_faction))
                return *force >= REP_FRIENDLY;

            // apply reputation state
            if (FactionEntry const* raw_tester_faction = sFactionStore.LookupEntry(tester_faction->faction))
                if (raw_tester_faction->reputationListID >= 0)
                    return ((Player const*)target)->GetReputationMgr().GetRank(raw_tester_faction) >= REP_FRIENDLY;
        }
    }

    // common faction based case (CvC,PvC,CvP)
    return tester_faction->IsFriendlyTo(*target_faction);
}

bool Unit::IsHostileToPlayers() const
{
    FactionTemplateEntry const* my_faction = getFactionTemplateEntry();
    if (!my_faction || !my_faction->faction)
        return false;

    FactionEntry const* raw_faction = sFactionStore.LookupEntry(my_faction->faction);
    if (raw_faction && raw_faction->reputationListID >= 0)
        return false;

    return my_faction->IsHostileToPlayers();
}

bool Unit::IsNeutralToAll() const
{
    FactionTemplateEntry const* my_faction = getFactionTemplateEntry();
    if (!my_faction || !my_faction->faction)
        return true;

    FactionEntry const* raw_faction = sFactionStore.LookupEntry(my_faction->faction);
    if (raw_faction && raw_faction->reputationListID >= 0)
        return false;

    return my_faction->IsNeutralToAll();
}

bool Unit::Attack(Unit* victim, bool meleeAttack)
{
    if (!victim || victim == this)
        return false;

    // dead units can neither attack nor be attacked
    if (!isAlive() || !victim->IsInWorld() || !victim->isAlive())
        return false;

    // player cannot attack in mount state
    if (GetTypeId() == TYPEID_PLAYER && IsMounted())
        return false;

    // nobody can attack GM in GM-mode
    if (victim->GetTypeId() == TYPEID_PLAYER)
    {
        if (((Player*)victim)->isGameMaster())
            return false;
    }
    else
    {
        if (((Creature*)victim)->IsInEvadeMode())
            return false;
    }

    // remove SPELL_AURA_MOD_UNATTACKABLE at attack (in case non-interruptible spells stun aura applied also that not let attack)
    if (HasAuraType(SPELL_AURA_MOD_UNATTACKABLE))
        RemoveSpellsCausingAura(SPELL_AURA_MOD_UNATTACKABLE);

    // in fighting already
    if (m_attacking)
    {
        if (m_attacking == victim)
        {
            // switch to melee attack from ranged/magic
            if (meleeAttack)
            {
                if (!hasUnitState(UNIT_STAT_MELEE_ATTACKING))
                {
                    addUnitState(UNIT_STAT_MELEE_ATTACKING);
                    SendMeleeAttackStart(victim);
                }
                return true;
            }
            return false;
        }

        // remove old target data
        AttackStop(true);
    }
    // new battle
    else
    {
        // set position before any AI calls/assistance
        if (GetTypeId() == TYPEID_UNIT)
            ((Creature*)this)->SetCombatStartPosition(GetPositionX(), GetPositionY(), GetPositionZ());
    }

    // Set our target
    SetTargetGuid(victim->GetObjectGuid());

    if (meleeAttack)
        addUnitState(UNIT_STAT_MELEE_ATTACKING);

    m_attacking = victim;
    m_attacking->_addAttacker(this);

    if (GetTypeId() == TYPEID_UNIT)
    {
        ((Creature*)this)->SendAIReaction(AI_REACTION_HOSTILE);
        ((Creature*)this)->CallAssistance();
    }

    // delay offhand weapon attack to next attack time
    if (haveOffhandWeapon())
        resetAttackTimer(OFF_ATTACK);

    if (meleeAttack)
        SendMeleeAttackStart(victim);

    return true;
}

void Unit::AttackedBy(Unit* attacker)
{
    // trigger AI reaction
    if (AI())
        AI()->AttackedBy(attacker);

    // do not pet reaction for self inflicted damage (like environmental)
    if (attacker == this)
        return;

    // trigger pet AI reaction
    if (Pet* pet = GetPet())
        pet->AttackedBy(attacker);
}

bool Unit::AttackStop(bool targetSwitch /*= false*/, bool includingCast /*= false*/)
{
    // interrupt cast only id includingCast == true and we have something to interrupt.
    if (includingCast && IsNonMeleeSpellCasted(false))
        InterruptNonMeleeSpells(false);

    // Clear our target only if targetSwitch == true
    if (targetSwitch)
        SetTargetGuid(ObjectGuid());

    clearUnitState(UNIT_STAT_MELEE_ATTACKING);

    InterruptSpell(CURRENT_MELEE_SPELL);

    if (m_attacking)
    {
        SendMeleeAttackStop(m_attacking);

        m_attacking->_removeAttacker(this);
        m_attacking = nullptr;
        return true;
    }
    return false;
}

void Unit::CombatStop(bool includingCast)
{
    AttackStop(true, includingCast);
    RemoveAllAttackers();

    if (GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->SendAttackSwingCancelAttack();     // melee and ranged forced attack cancel
    else
    {
        ((Creature*)this)->SetNoCallAssistance(false);

        if (((Creature*)this)->HasSearchedAssistance())
        {
            ((Creature*)this)->SetNoSearchAssistance(false);
            UpdateSpeed(MOVE_RUN, false);
        }

        if (((Creature*)this)->GetTemporaryFactionFlags() & TEMPFACTION_RESTORE_COMBAT_STOP)
            ((Creature*)this)->ClearTemporaryFaction();
    }

    if (AI())
        AI()->CombatStop();

    ClearInCombat();
}

struct CombatStopWithPetsHelper
{
    explicit CombatStopWithPetsHelper(bool _includingCast) : includingCast(_includingCast) {}
    void operator()(Unit* unit) const { unit->CombatStop(includingCast); }
    bool includingCast;
};

void Unit::CombatStopWithPets(bool includingCast)
{
    CombatStop(includingCast);
    CallForAllControlledUnits(CombatStopWithPetsHelper(includingCast), CONTROLLED_PET | CONTROLLED_GUARDIANS | CONTROLLED_CHARM);
}

struct IsAttackingPlayerHelper
{
    explicit IsAttackingPlayerHelper() {}
    bool operator()(Unit const* unit) const { return unit->isAttackingPlayer(); }
};

bool Unit::isAttackingPlayer() const
{
    if (hasUnitState(UNIT_STAT_ATTACK_PLAYER))
        return true;

    return CheckAllControlledUnits(IsAttackingPlayerHelper(), CONTROLLED_PET | CONTROLLED_TOTEMS | CONTROLLED_GUARDIANS | CONTROLLED_CHARM);
}

void Unit::RemoveAllAttackers()
{
    while (!m_attackers.empty())
    {
        AttackerSet::iterator iter = m_attackers.begin();
        (*iter)->AttackStop();

        if (m_attacking)
        {
            sLog.outError("WORLD: Unit has an attacker that isn't attacking it!");
            m_attackers.erase(iter);
        }
    }
}

void Unit::ModifyAuraState(AuraState flag, bool apply)
{
    if (apply)
    {
        if (!HasFlag(UNIT_FIELD_AURASTATE, 1 << (flag - 1)))
        {
            SetFlag(UNIT_FIELD_AURASTATE, 1 << (flag - 1));
            if (GetTypeId() == TYPEID_PLAYER)
            {
                const PlayerSpellMap& sp_list = ((Player*)this)->GetSpellMap();
                for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
                {
                    if (itr->second.state == PLAYERSPELL_REMOVED) continue;
                    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(itr->first);
                    if (!spellInfo || !IsPassiveSpell(spellInfo)) continue;
                    if (AuraState(spellInfo->CasterAuraState) == flag)
                        CastSpell(this, itr->first, TRIGGERED_OLD_TRIGGERED, nullptr);
                }
            }
        }
    }
    else
    {
        if (HasFlag(UNIT_FIELD_AURASTATE, 1 << (flag - 1)))
        {
            RemoveFlag(UNIT_FIELD_AURASTATE, 1 << (flag - 1));

            Unit::SpellAuraHolderMap& tAuras = GetSpellAuraHolderMap();
            for (Unit::SpellAuraHolderMap::iterator itr = tAuras.begin(); itr != tAuras.end();)
            {
                SpellEntry const* spellProto = (*itr).second->GetSpellProto();
                if (spellProto->CasterAuraState == flag)
                {
                    // exceptions (applied at state but not removed at state change)
                    // Rampage
                    if (spellProto->SpellIconID == 2006 && spellProto->IsFitToFamilyMask(uint64(0x0000000000100000)))
                    {
                        ++itr;
                        continue;
                    }

                    RemoveSpellAuraHolder(itr->second);
                    itr = tAuras.begin();
                }
                else
                    ++itr;
            }
        }
    }
}

Unit* Unit::GetOwner() const
{
    if (ObjectGuid ownerid = GetOwnerGuid())
        return ObjectAccessor::GetUnit(*this, ownerid);
    return nullptr;
}

Unit* Unit::GetCharmer() const
{
    if (ObjectGuid charmerid = GetCharmerGuid())
        return ObjectAccessor::GetUnit(*this, charmerid);
    return nullptr;
}

bool Unit::IsCharmerOrOwnerPlayerOrPlayerItself() const
{
    if (GetTypeId() == TYPEID_PLAYER)
        return true;

    return GetCharmerOrOwnerGuid().IsPlayer();
}

Player* Unit::GetCharmerOrOwnerPlayerOrPlayerItself()
{
    ObjectGuid guid = GetCharmerOrOwnerGuid();
    if (guid.IsPlayer())
        return ObjectAccessor::FindPlayer(guid);

    return GetTypeId() == TYPEID_PLAYER ? (Player*)this : nullptr;
}

Player const* Unit::GetCharmerOrOwnerPlayerOrPlayerItself() const
{
    ObjectGuid guid = GetCharmerOrOwnerGuid();
    if (guid.IsPlayer())
        return ObjectAccessor::FindPlayer(guid);

    return GetTypeId() == TYPEID_PLAYER ? (Player const*)this : nullptr;
}

Pet* Unit::GetPet() const
{
    if (ObjectGuid pet_guid = GetPetGuid())
    {
        if (Pet* pet = GetMap()->GetPet(pet_guid))
            return pet;

        sLog.outError("Unit::GetPet: %s not exist.", pet_guid.GetString().c_str());
        const_cast<Unit*>(this)->SetPet(nullptr);
    }

    return nullptr;
}

Pet* Unit::_GetPet(ObjectGuid guid) const
{
    return GetMap()->GetPet(guid);
}

Unit* Unit::GetCharm() const
{
    if (ObjectGuid charm_guid = GetCharmGuid())
    {
        if (Unit* pet = ObjectAccessor::GetUnit(*this, charm_guid))
            return pet;

        sLog.outError("Unit::GetCharm: Charmed %s not exist.", charm_guid.GetString().c_str());
        const_cast<Unit*>(this)->SetCharm(nullptr);
    }

    return nullptr;
}

void Unit::Uncharm()
{
    if (Unit* charm = GetCharm())
    {
        charm->RemoveSpellsCausingAura(SPELL_AURA_MOD_CHARM);
        charm->RemoveSpellsCausingAura(SPELL_AURA_MOD_POSSESS);
        charm->RemoveSpellsCausingAura(SPELL_AURA_MOD_POSSESS_PET);
    }
}

void Unit::SetPet(Pet* pet)
{
    SetPetGuid(pet ? pet->GetObjectGuid() : ObjectGuid());
}

void Unit::SetCharm(Unit* pet)
{
    SetCharmGuid(pet ? pet->GetObjectGuid() : ObjectGuid());
}

void Unit::AddGuardian(Pet* pet)
{
    m_guardianPets.insert(pet->GetObjectGuid());
}

void Unit::RemoveGuardian(Pet* pet)
{
    m_guardianPets.erase(pet->GetObjectGuid());
}

void Unit::RemoveGuardians()
{
    while (!m_guardianPets.empty())
    {
        ObjectGuid guid = *m_guardianPets.begin();

        if (Pet* pet = GetMap()->GetPet(guid))
            pet->Unsummon(PET_SAVE_AS_DELETED, this); // can remove pet guid from m_guardianPets

        m_guardianPets.erase(guid);
    }
}

Pet* Unit::FindGuardianWithEntry(uint32 entry)
{
    for (GuidSet::const_iterator itr = m_guardianPets.begin(); itr != m_guardianPets.end(); ++itr)
        if (Pet* pet = GetMap()->GetPet(*itr))
            if (pet->GetEntry() == entry)
                return pet;

    return nullptr;
}

Unit* Unit::_GetTotem(TotemSlot slot) const
{
    return GetTotem(slot);
}

Totem* Unit::GetTotem(TotemSlot slot) const
{
    if (!IsInWorld() || !m_TotemSlot[slot])
        return nullptr;

    Creature* totem = GetMap()->GetCreature(m_TotemSlot[slot]);
    return totem && totem->IsTotem() ? (Totem*)totem : nullptr;
}

bool Unit::IsAllTotemSlotsUsed() const
{
    for (int i = 0; i < MAX_TOTEM_SLOT; ++i)
        if (!m_TotemSlot[i])
            return false;
    return true;
}

void Unit::_AddTotem(TotemSlot slot, Totem* totem)
{
    m_TotemSlot[slot] = totem->GetObjectGuid();
}

void Unit::_RemoveTotem(Totem* totem)
{
    for (int i = 0; i < MAX_TOTEM_SLOT; ++i)
    {
        if (m_TotemSlot[i] == totem->GetObjectGuid())
        {
            m_TotemSlot[i].Clear();
            break;
        }
    }
}

void Unit::UnsummonAllTotems() const
{
    for (int i = 0; i < MAX_TOTEM_SLOT; ++i)
        if (Totem* totem = GetTotem(TotemSlot(i)))
            totem->UnSummon();
}

int32 Unit::DealHeal(Unit* pVictim, uint32 addhealth, SpellEntry const* spellProto, bool critical)
{
    int32 gain = pVictim->ModifyHealth(int32(addhealth));

    Unit* unit = this;

    if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->IsTotem() && ((Totem*)this)->GetTotemType() != TOTEM_STATUE)
        unit = GetOwner();

    if (unit->GetTypeId() == TYPEID_PLAYER)
        unit->SendHealSpellLog(pVictim, spellProto->Id, addhealth, critical);

    // Script Event HealedBy
    if (pVictim->AI())
        pVictim->AI()->HealedBy(this, addhealth);

    return gain;
}

Unit* Unit::SelectMagnetTarget(Unit* victim, Spell* spell, SpellEffectIndex eff)
{
    if (!victim)
        return nullptr;

    // Magic case
    if (spell && (spell->m_spellInfo->DmgClass == SPELL_DAMAGE_CLASS_NONE || spell->m_spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MAGIC))
    {
        if (spell->m_spellInfo->HasAttribute(SPELL_ATTR_ABILITY) || spell->m_spellInfo->HasAttribute(SPELL_ATTR_EX_CANT_BE_REDIRECTED) || spell->m_spellInfo->HasAttribute(SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY))
            return nullptr;

        Unit::AuraList const& magnetAuras = victim->GetAurasByType(SPELL_AURA_SPELL_MAGNET);
        for (Unit::AuraList::const_iterator itr = magnetAuras.begin(); itr != magnetAuras.end(); ++itr)
        {
            if (Unit* magnet = (*itr)->GetCaster())
            {
                if (magnet->isAlive() && magnet->IsWithinLOSInMap(this) && spell->CheckTarget(magnet, eff))
                {
                    if (SpellAuraHolder* holder = (*itr)->GetHolder())
                        if (holder->DropAuraCharge())
                            victim->RemoveSpellAuraHolder(holder);
                    return magnet;
                }
            }
        }
    }

    return nullptr;
}

void Unit::SendHealSpellLog(Unit* pVictim, uint32 SpellID, uint32 Damage, bool critical)
{
    // we guess size
    WorldPacket data(SMSG_SPELLHEALLOG, (8 + 8 + 4 + 4 + 1));
    data << pVictim->GetPackGUID();
    data << GetPackGUID();
    data << uint32(SpellID);
    data << uint32(Damage);
    data << uint8(critical ? 1 : 0);
    data << uint8(0);                                       // unused in client?
    SendMessageToSet(data, true);
}

void Unit::SendEnergizeSpellLog(Unit* pVictim, uint32 SpellID, uint32 Damage, Powers powertype) const
{
    WorldPacket data(SMSG_SPELLENERGIZELOG, (8 + 8 + 4 + 4 + 4 + 1));
    data << pVictim->GetPackGUID();
    data << GetPackGUID();
    data << uint32(SpellID);
    data << uint32(powertype);
    data << uint32(Damage);
    SendMessageToSet(data, true);
}

void Unit::EnergizeBySpell(Unit* pVictim, uint32 SpellID, uint32 Damage, Powers powertype)
{
    SendEnergizeSpellLog(pVictim, SpellID, Damage, powertype);
    // needs to be called after sending spell log
    pVictim->ModifyPower(powertype, Damage);
}

/** Calculate spell coefficents and level penalties for spell/melee damage or heal
 *
 * this is the caster of the spell/ melee attacker
 * @param spellProto SpellEntry of the used spell
 * @param total current value onto which the Bonus and level penalty will be calculated
 * @param benefit additional benefit from ie spellpower-auras
 * @param ap_benefit additional melee attackpower benefit from auras
 * @param damagetype what kind of damage
 * @param donePart calculate for done or taken
 */
int32 Unit::SpellBonusWithCoeffs(SpellEntry const* spellProto, int32 total, int32 benefit, int32 ap_benefit,  DamageEffectType damagetype, bool donePart)
{
    // Distribute Damage over multiple effects, reduce by AoE
    float coeff = 1.0f;

    // Not apply this to creature casted spells
    if (GetTypeId() == TYPEID_UNIT && !((Creature*)this)->IsPet())
        coeff = 1.0f;
    // Check for table values
    else if (SpellBonusEntry const* bonus = sSpellMgr.GetSpellBonusData(spellProto->Id))
    {
        coeff = damagetype == DOT ? bonus->dot_damage : bonus->direct_damage;

        // apply ap bonus at done part calculation only (it flat total mod so common with taken)
        if (donePart && (bonus->ap_bonus || bonus->ap_dot_bonus))
        {
            float ap_bonus = damagetype == DOT ? bonus->ap_dot_bonus : bonus->ap_bonus;

            total += int32(ap_bonus * (GetTotalAttackPowerValue(IsSpellRequiresRangedAP(spellProto) ? RANGED_ATTACK : BASE_ATTACK) + ap_benefit));
        }
    }
    // Default calculation
    else if (benefit)
        coeff = CalculateDefaultCoefficient(spellProto, damagetype);

    if (benefit)
    {
        float LvlPenalty = CalculateLevelPenalty(spellProto);

        // Spellmod SpellDamage
        if (coeff) // If something has 0 coeff to begin with, do not add spellmod
        {
            if (Player* modOwner = GetSpellModOwner()) // Spellmod SpellDamage
            {
                coeff *= 100.0f;
                modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_SPELL_BONUS_DAMAGE, coeff);
                coeff /= 100.0f;
            }
        }

        total += int32(benefit * coeff * LvlPenalty);
    }

    return total;
};

/**
 * Calculates caster part of spell damage bonuses,
 * also includes different bonuses dependent from target auras
 */
uint32 Unit::SpellDamageBonusDone(Unit* pVictim, SpellEntry const* spellProto, uint32 pdamage, DamageEffectType damagetype, uint32 stack)
{
    if (!spellProto || !pVictim || damagetype == DIRECT_DAMAGE)
        return pdamage;

    // For totems get damage bonus from owner (statue isn't totem in fact)
    if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->IsTotem() && ((Totem*)this)->GetTotemType() != TOTEM_STATUE)
    {
        if (Unit* owner = GetOwner())
            return owner->SpellDamageBonusDone(pVictim, spellProto, pdamage, damagetype);
    }

    uint32 creatureTypeMask = pVictim->GetCreatureTypeMask();
    float DoneTotalMod = 1.0f;
    int32 DoneTotal = 0;

    // Creature damage
    if (GetTypeId() == TYPEID_UNIT && !((Creature*)this)->IsPet())
        DoneTotalMod *= Creature::_GetSpellDamageMod(((Creature*)this)->GetCreatureInfo()->Rank);

    AuraList const& mModDamagePercentDone = GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
    for (AuraList::const_iterator i = mModDamagePercentDone.begin(); i != mModDamagePercentDone.end(); ++i)
    {
        if (((*i)->GetModifier()->m_miscvalue & GetSpellSchoolMask(spellProto)) &&
                (*i)->GetSpellProto()->EquippedItemClass == -1 &&
                // -1 == any item class (not wand then)
                (*i)->GetSpellProto()->EquippedItemInventoryTypeMask == 0)
            // 0 == any inventory type (not wand then)
        {
            DoneTotalMod *= ((*i)->GetModifier()->m_amount + 100.0f) / 100.0f;
        }
    }

    // Add flat bonus from spell damage versus
    DoneTotal += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_FLAT_SPELL_DAMAGE_VERSUS, creatureTypeMask);

    // Add pct bonus from spell damage versus
    DoneTotalMod *= GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_DAMAGE_DONE_VERSUS, creatureTypeMask);

    // Add flat bonus from spell damage creature
    DoneTotal += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_DAMAGE_DONE_CREATURE, creatureTypeMask);

    // done scripted mod (take it from owner)
    Unit* owner = GetOwner();
    if (!owner)
        owner = this;

    AuraList const& mOverrideClassScript = owner->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    for (AuraList::const_iterator i = mOverrideClassScript.begin(); i != mOverrideClassScript.end(); ++i)
    {
        if (!(*i)->isAffectedOnSpell(spellProto))
            continue;

        switch ((*i)->GetModifier()->m_miscvalue)
        {
            case 4418: // Increased Shock Damage
            case 4554: // Increased Lightning Damage
            {
                DoneTotal += (*i)->GetModifier()->m_amount;
                break;
            }
            case 4555: // Improved Moonfire
            {
                DoneTotalMod *= ((*i)->GetModifier()->m_amount + 100.0f) / 100.0f;
                break;
            }
        }
    }

    // Done fixed damage bonus auras
    int32 DoneAdvertisedBenefit = SpellBaseDamageBonusDone(GetSpellSchoolMask(spellProto));

    // Pets just add their bonus damage to their spell damage
    // note that their spell damage is just gain of their own auras
    if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->IsPet())
        DoneAdvertisedBenefit += ((Pet*)this)->GetBonusDamage();

    // apply ap bonus and benefit affected by spell power implicit coeffs and spell level penalties
    DoneTotal = SpellBonusWithCoeffs(spellProto, DoneTotal, DoneAdvertisedBenefit, 0, damagetype, true);

    float tmpDamage = (int32(pdamage) + DoneTotal * int32(stack)) * DoneTotalMod;
    // apply spellmod to Done damage (flat and pct)
    if (Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellProto->Id, damagetype == DOT ? SPELLMOD_DOT : SPELLMOD_DAMAGE, tmpDamage);

    return tmpDamage > 0 ? uint32(tmpDamage) : 0;
}

/**
 * Calculates target part of spell damage bonuses,
 * will be called on each tick for periodic damage over time auras
 */
uint32 Unit::SpellDamageBonusTaken(Unit* pCaster, SpellEntry const* spellProto, uint32 pdamage, DamageEffectType damagetype, uint32 stack)
{
    if (!spellProto || !pCaster || damagetype == DIRECT_DAMAGE)
        return pdamage;

    uint32 schoolMask = GetSpellSchoolMask(spellProto);

    // Taken total percent damage auras
    float TakenTotalMod = 1.0f;
    int32 TakenTotal = 0;

    // ..taken
    TakenTotalMod *= GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, schoolMask);

    // Taken fixed damage bonus auras
    int32 TakenAdvertisedBenefit = SpellBaseDamageBonusTaken(GetSpellSchoolMask(spellProto));

    // apply benefit affected by spell power implicit coeffs and spell level penalties
    TakenTotal = pCaster->SpellBonusWithCoeffs(spellProto, TakenTotal, TakenAdvertisedBenefit, 0, damagetype, false);

    float tmpDamage = (int32(pdamage) + TakenTotal * int32(stack)) * TakenTotalMod;

    return tmpDamage > 0 ? uint32(tmpDamage) : 0;
}

int32 Unit::SpellBaseDamageBonusDone(SpellSchoolMask schoolMask)
{
    int32 DoneAdvertisedBenefit = 0;

    // ..done
    AuraList const& mDamageDone = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE);
    for (AuraList::const_iterator i = mDamageDone.begin(); i != mDamageDone.end(); ++i)
    {
        if (((*i)->GetModifier()->m_miscvalue & schoolMask) != 0 &&
                (*i)->GetSpellProto()->EquippedItemClass == -1 &&                   // -1 == any item class (not wand then)
                (*i)->GetSpellProto()->EquippedItemInventoryTypeMask == 0)          //  0 == any inventory type (not wand then)
            DoneAdvertisedBenefit += (*i)->GetModifier()->m_amount;
    }

    if (GetTypeId() == TYPEID_PLAYER)
    {
        // Damage bonus from stats
        AuraList const& mDamageDoneOfStatPercent = GetAurasByType(SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT);
        for (AuraList::const_iterator i = mDamageDoneOfStatPercent.begin(); i != mDamageDoneOfStatPercent.end(); ++i)
        {
            if ((*i)->GetModifier()->m_miscvalue & schoolMask)
            {
                // stat used stored in miscValueB for this aura
                Stats usedStat = STAT_SPIRIT;
                DoneAdvertisedBenefit += int32(GetStat(usedStat) * (*i)->GetModifier()->m_amount / 100.0f);
            }
        }
    }
    return DoneAdvertisedBenefit;
}

int32 Unit::SpellBaseDamageBonusTaken(SpellSchoolMask schoolMask) const
{
    int32 TakenAdvertisedBenefit = 0;

    // ..taken
    AuraList const& mDamageTaken = GetAurasByType(SPELL_AURA_MOD_DAMAGE_TAKEN);
    for (AuraList::const_iterator i = mDamageTaken.begin(); i != mDamageTaken.end(); ++i)
    {
        if (((*i)->GetModifier()->m_miscvalue & schoolMask) != 0)
            TakenAdvertisedBenefit += (*i)->GetModifier()->m_amount;
    }

    return TakenAdvertisedBenefit;
}

/**
 * Calculates caster part of healing spell bonuses,
 * also includes different bonuses dependent from target auras
 */
uint32 Unit::SpellHealingBonusDone(Unit* pVictim, SpellEntry const* spellProto, int32 healamount, DamageEffectType damagetype, uint32 stack)
{
    // For totems get healing bonus from owner (statue isn't totem in fact)
    if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->IsTotem() && ((Totem*)this)->GetTotemType() != TOTEM_STATUE)
        if (Unit* owner = GetOwner())
            return owner->SpellHealingBonusDone(pVictim, spellProto, healamount, damagetype, stack);

    // No heal amount for this class spells
    if (spellProto->DmgClass == SPELL_DAMAGE_CLASS_NONE)
        return healamount < 0 ? 0 : healamount;

    // Healing Done
    // Done total percent damage auras
    float  DoneTotalMod = 1.0f;
    int32  DoneTotal = 0;

    // Healing done percent
    AuraList const& mHealingDonePct = GetAurasByType(SPELL_AURA_MOD_HEALING_DONE_PERCENT);
    for (AuraList::const_iterator i = mHealingDonePct.begin(); i != mHealingDonePct.end(); ++i)
        DoneTotalMod *= (100.0f + (*i)->GetModifier()->m_amount) / 100.0f;

    // done scripted mod (take it from owner)
    Unit* owner = GetOwner();
    if (!owner) owner = this;
    AuraList const& mOverrideClassScript = owner->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    for (AuraList::const_iterator i = mOverrideClassScript.begin(); i != mOverrideClassScript.end(); ++i)
    {
        if (!(*i)->isAffectedOnSpell(spellProto))
            continue;
        switch ((*i)->GetModifier()->m_miscvalue)
        {
            case 4415: // Increased Rejuvenation Healing
            case 3736: // Hateful Totem of the Third Wind / Increased Lesser Healing Wave / Savage Totem of the Third Wind
                DoneTotal += (*i)->GetModifier()->m_amount;
                break;
            default:
                break;
        }
    }

    // Done fixed damage bonus auras
    int32 DoneAdvertisedBenefit  = SpellBaseHealingBonusDone(GetSpellSchoolMask(spellProto));

    // apply ap bonus and benefit affected by spell power implicit coeffs and spell level penalties
    DoneTotal = SpellBonusWithCoeffs(spellProto, DoneTotal, DoneAdvertisedBenefit, 0, damagetype, true);

    // use float as more appropriate for negative values and percent applying
    float heal = (healamount + DoneTotal * int32(stack)) * DoneTotalMod;
    // apply spellmod to Done amount
    if (Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellProto->Id, damagetype == DOT ? SPELLMOD_DOT : SPELLMOD_DAMAGE, heal);

    return heal < 0 ? 0 : uint32(heal);
}

/**
 * Calculates target part of healing spell bonuses,
 * will be called on each tick for periodic damage over time auras
 */
uint32 Unit::SpellHealingBonusTaken(Unit* pCaster, SpellEntry const* spellProto, int32 healamount, DamageEffectType damagetype, uint32 stack)
{
    float  TakenTotalMod = 1.0f;

    // Healing taken percent
    float minval = float(GetMaxNegativeAuraModifier(SPELL_AURA_MOD_HEALING_PCT));
    if (minval)
        TakenTotalMod *= (100.0f + minval) / 100.0f;

    float maxval = float(GetMaxPositiveAuraModifier(SPELL_AURA_MOD_HEALING_PCT));
    if (maxval)
        TakenTotalMod *= (100.0f + maxval) / 100.0f;

    // No heal amount for this class spells
    if (spellProto->DmgClass == SPELL_DAMAGE_CLASS_NONE)
    {
        healamount = int32(healamount * TakenTotalMod);
        return healamount < 0 ? 0 : healamount;
    }

    // Healing Done
    // Done total percent damage auras
    int32  TakenTotal = 0;

    // Taken fixed damage bonus auras
    int32 TakenAdvertisedBenefit = SpellBaseHealingBonusTaken(GetSpellSchoolMask(spellProto));

    // Blessing of Light dummy effects healing taken from Holy Light and Flash of Light
    if (spellProto->SpellFamilyName == SPELLFAMILY_PALADIN && (spellProto->SpellFamilyFlags & uint64(0x0000000000006000)))
    {
        AuraList const& mDummyAuras = GetAurasByType(SPELL_AURA_DUMMY);
        for (AuraList::const_iterator i = mDummyAuras.begin(); i != mDummyAuras.end(); ++i)
        {
            if ((*i)->GetSpellProto()->SpellVisual == 300 && ((*i)->GetSpellProto()->SpellFamilyFlags & uint64(0x0000000010000000)))
            {
                // Flash of Light
                if ((spellProto->SpellFamilyFlags & uint64(0x0000000000002000)) && (*i)->GetEffIndex() == EFFECT_INDEX_1)
                    TakenTotal += (*i)->GetModifier()->m_amount;
                // Holy Light
                else if ((spellProto->SpellFamilyFlags & uint64(0x0000000000004000)) && (*i)->GetEffIndex() == EFFECT_INDEX_0)
                    TakenTotal += (*i)->GetModifier()->m_amount;
            }
        }
    }

    // apply benefit affected by spell power implicit coeffs and spell level penalties
    TakenTotal = pCaster->SpellBonusWithCoeffs(spellProto, TakenTotal, TakenAdvertisedBenefit, 0, damagetype, false);

    // Taken mods
    // Healing Wave cast
    if (spellProto->SpellFamilyName == SPELLFAMILY_SHAMAN && (spellProto->SpellFamilyFlags & uint64(0x0000000000000040)))
    {
        // Search for Healing Way on Victim
        Unit::AuraList const& auraDummy = GetAurasByType(SPELL_AURA_DUMMY);
        for (Unit::AuraList::const_iterator itr = auraDummy.begin(); itr != auraDummy.end(); ++itr)
            if ((*itr)->GetId() == 29203)
                TakenTotalMod *= ((*itr)->GetModifier()->m_amount + 100.0f) / 100.0f;
    }


    // use float as more appropriate for negative values and percent applying
    float heal = (healamount + TakenTotal * int32(stack)) * TakenTotalMod;

    return heal < 0 ? 0 : uint32(heal);
}

int32 Unit::SpellBaseHealingBonusDone(SpellSchoolMask schoolMask)
{
    int32 AdvertisedBenefit = 0;

    AuraList const& mHealingDone = GetAurasByType(SPELL_AURA_MOD_HEALING_DONE);
    for (AuraList::const_iterator i = mHealingDone.begin(); i != mHealingDone.end(); ++i)
        if (((*i)->GetModifier()->m_miscvalue & schoolMask) != 0)
            AdvertisedBenefit += (*i)->GetModifier()->m_amount;

    // Healing bonus of spirit, intellect and strength
    if (GetTypeId() == TYPEID_PLAYER)
    {
        // Healing bonus from stats
        AuraList const& mHealingDoneOfStatPercent = GetAurasByType(SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT);
        for (AuraList::const_iterator i = mHealingDoneOfStatPercent.begin(); i != mHealingDoneOfStatPercent.end(); ++i)
        {
            // 1.12.* have only 1 stat type support
            Stats usedStat = STAT_SPIRIT;
            AdvertisedBenefit += int32(GetStat(usedStat) * (*i)->GetModifier()->m_amount / 100.0f);
        }
    }
    return AdvertisedBenefit;
}

int32 Unit::SpellBaseHealingBonusTaken(SpellSchoolMask schoolMask) const
{
    int32 AdvertisedBenefit = 0;
    AuraList const& mDamageTaken = GetAurasByType(SPELL_AURA_MOD_HEALING);
    for (AuraList::const_iterator i = mDamageTaken.begin(); i != mDamageTaken.end(); ++i)
        if ((*i)->GetModifier()->m_miscvalue & schoolMask)
            AdvertisedBenefit += (*i)->GetModifier()->m_amount;

    return AdvertisedBenefit;
}

bool Unit::IsImmuneToDamage(SpellSchoolMask shoolMask)
{
    // If m_immuneToSchool type contain this school type, IMMUNE damage.
    SpellImmuneList const& schoolList = m_spellImmune[IMMUNITY_SCHOOL];
    for (SpellImmuneList::const_iterator itr = schoolList.begin(); itr != schoolList.end(); ++itr)
        if (itr->type & shoolMask)
            return true;

    // If m_immuneToDamage type contain magic, IMMUNE damage.
    SpellImmuneList const& damageList = m_spellImmune[IMMUNITY_DAMAGE];
    for (SpellImmuneList::const_iterator itr = damageList.begin(); itr != damageList.end(); ++itr)
        if (itr->type & shoolMask)
            return true;

    return false;
}

bool Unit::IsImmuneToSpell(SpellEntry const* spellInfo, bool /*castOnSelf*/)
{
    if (!spellInfo)
        return false;

    // TODO add spellEffect immunity checks!, player with flag in bg is immune to immunity buffs from other friendly players!
    // SpellImmuneList const& dispelList = m_spellImmune[IMMUNITY_EFFECT];

    SpellImmuneList const& dispelList = m_spellImmune[IMMUNITY_DISPEL];
    for (SpellImmuneList::const_iterator itr = dispelList.begin(); itr != dispelList.end(); ++itr)
        if (itr->type == spellInfo->Dispel)
            return true;

    if (!spellInfo->HasAttribute(SPELL_ATTR_EX_UNAFFECTED_BY_SCHOOL_IMMUNE) &&          // unaffected by school immunity
            !spellInfo->HasAttribute(SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY))           // can remove immune (by dispell or immune it)
    {
        SpellImmuneList const& schoolList = m_spellImmune[IMMUNITY_SCHOOL];
        for (SpellImmuneList::const_iterator itr = schoolList.begin(); itr != schoolList.end(); ++itr)
            if (!(IsPositiveSpell(itr->spellId) && IsPositiveSpell(spellInfo->Id)) &&
                    (itr->type & GetSpellSchoolMask(spellInfo)))
                return true;
    }

    if (uint32 mechanic = spellInfo->Mechanic)
    {
        SpellImmuneList const& mechanicList = m_spellImmune[IMMUNITY_MECHANIC];
        for (SpellImmuneList::const_iterator itr = mechanicList.begin(); itr != mechanicList.end(); ++itr)
            if (itr->type == mechanic)
                return true;

        AuraList const& immuneAuraApply = GetAurasByType(SPELL_AURA_MECHANIC_IMMUNITY_MASK);
        for (AuraList::const_iterator iter = immuneAuraApply.begin(); iter != immuneAuraApply.end(); ++iter)
            if ((*iter)->GetModifier()->m_miscvalue & (1 << (mechanic - 1)))
                return true;
    }

    return false;
}

bool Unit::IsImmuneToSpellEffect(SpellEntry const* spellInfo, SpellEffectIndex index, bool /*castOnSelf*/) const
{
    // If m_immuneToEffect type contain this effect type, IMMUNE effect.
    uint32 effect = spellInfo->Effect[index];
    SpellImmuneList const& effectList = m_spellImmune[IMMUNITY_EFFECT];
    for (SpellImmuneList::const_iterator itr = effectList.begin(); itr != effectList.end(); ++itr)
        if (itr->type == effect)
            return true;

    if (uint32 mechanic = spellInfo->EffectMechanic[index])
    {
        SpellImmuneList const& mechanicList = m_spellImmune[IMMUNITY_MECHANIC];
        for (SpellImmuneList::const_iterator itr = mechanicList.begin(); itr != mechanicList.end(); ++itr)
            if (itr->type == mechanic)
                return true;

        AuraList const& immuneAuraApply = GetAurasByType(SPELL_AURA_MECHANIC_IMMUNITY_MASK);
        for (AuraList::const_iterator iter = immuneAuraApply.begin(); iter != immuneAuraApply.end(); ++iter)
            if ((*iter)->GetModifier()->m_miscvalue & (1 << (mechanic - 1)))
                return true;
    }

    if (uint32 aura = spellInfo->EffectApplyAuraName[index])
    {
        SpellImmuneList const& list = m_spellImmune[IMMUNITY_STATE];
        for (SpellImmuneList::const_iterator itr = list.begin(); itr != list.end(); ++itr)
            if (itr->type == aura)
                return true;
    }
    return false;
}

/**
 * Calculates caster part of melee damage bonuses,
 * also includes different bonuses dependent from target auras
 */
uint32 Unit::MeleeDamageBonusDone(Unit* pVictim, uint32 pdamage, WeaponAttackType attType, SpellEntry const* spellProto, DamageEffectType damagetype, uint32 stack, bool flat)
{
    if (!pVictim || pdamage == 0)
        return pdamage;

    // differentiate for weapon damage based spells
    bool isWeaponDamageBasedSpell = !(spellProto && (damagetype == DOT || IsSpellHaveEffect(spellProto, SPELL_EFFECT_SCHOOL_DAMAGE)));
    Item*  pWeapon          = GetTypeId() == TYPEID_PLAYER ? ((Player*)this)->GetWeaponForAttack(attType, true, false) : nullptr;
    uint32 creatureTypeMask = pVictim->GetCreatureTypeMask();
    uint32 schoolMask       = spellProto ? GetSpellSchoolMask(spellProto) : uint32(GetMeleeDamageSchoolMask());

    // FLAT damage bonus auras
    // =======================
    int32 DoneFlat  = 0;
    int32 APbonus   = 0;

    // ..done flat, already included in weapon damage based spells
    if (!isWeaponDamageBasedSpell)
    {
        AuraList const& mModDamageDone = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE);
        for (AuraList::const_iterator i = mModDamageDone.begin(); i != mModDamageDone.end(); ++i)
        {
            if ((*i)->GetModifier()->m_miscvalue & schoolMask &&                         // schoolmask has to fit with the intrinsic spell school
                    (*i)->GetModifier()->m_miscvalue & GetMeleeDamageSchoolMask() &&         // AND schoolmask has to fit with weapon damage school (essential for non-physical spells)
                    (((*i)->GetSpellProto()->EquippedItemClass == -1) ||                     // general, weapon independent
                     (pWeapon && pWeapon->IsFitToSpellRequirements((*i)->GetSpellProto()))))  // OR used weapon fits aura requirements
            {
                DoneFlat += (*i)->GetModifier()->m_amount;
            }
        }

        // Pets just add their bonus damage to their melee damage
        if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->IsPet())
            DoneFlat += ((Pet*)this)->GetBonusDamage();
    }

    // ..done flat (by creature type mask)
    DoneFlat += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_DAMAGE_DONE_CREATURE, creatureTypeMask);

    // ..done flat (base at attack power for marked target and base at attack power for creature type)
    if (attType == RANGED_ATTACK)
    {
        APbonus += pVictim->GetTotalAuraModifier(SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS);
        APbonus += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_RANGED_ATTACK_POWER_VERSUS, creatureTypeMask);
    }
    else
    {
        APbonus += pVictim->GetTotalAuraModifier(SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS);
        APbonus += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_MELEE_ATTACK_POWER_VERSUS, creatureTypeMask);
    }

    // PERCENT damage auras
    // ====================
    float DonePercent   = 1.0f;

    // ..done pct, already included in weapon damage based spells
    if (!isWeaponDamageBasedSpell)
    {
        AuraList const& mModDamagePercentDone = GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
        for (AuraList::const_iterator i = mModDamagePercentDone.begin(); i != mModDamagePercentDone.end(); ++i)
        {
            if ((*i)->GetModifier()->m_miscvalue & schoolMask &&                         // schoolmask has to fit with the intrinsic spell school
                    (*i)->GetModifier()->m_miscvalue & GetMeleeDamageSchoolMask() &&         // AND schoolmask has to fit with weapon damage school (essential for non-physical spells)
                    (((*i)->GetSpellProto()->EquippedItemClass == -1) ||                     // general, weapon independent
                     (pWeapon && pWeapon->IsFitToSpellRequirements((*i)->GetSpellProto()))))  // OR used weapon fits aura requirements
            {
                DonePercent *= ((*i)->GetModifier()->m_amount + 100.0f) / 100.0f;
            }
        }

        if (attType == OFF_ATTACK)
            DonePercent *= GetModifierValue(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_PCT);                    // no school check required
    }

    // ..done pct (by creature type mask)
    DonePercent *= GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_DAMAGE_DONE_VERSUS, creatureTypeMask);

    // special dummys/class scripts and other effects
    // =============================================
    Unit* owner = GetOwner();
    if (!owner)
        owner = this;

    // final calculation
    // =================

    float DoneTotal = 0.0f;

    // scaling of non weapon based spells
    if (!isWeaponDamageBasedSpell)
    {
        // apply ap bonus and benefit affected by spell power implicit coeffs and spell level penalties
        DoneTotal = SpellBonusWithCoeffs(spellProto, DoneTotal, DoneFlat, APbonus, damagetype, true);
    }
    // weapon damage based spells
    else if (APbonus || DoneFlat)
    {
        bool normalized = spellProto ? IsSpellHaveEffect(spellProto, SPELL_EFFECT_NORMALIZED_WEAPON_DMG) : false;
        DoneTotal += int32(APbonus / 14.0f * GetAPMultiplier(attType, normalized));

        // for weapon damage based spells we still have to apply damage done percent mods
        // (that are already included into pdamage) to not-yet included DoneFlat
        // e.g. from doneVersusCreature, apBonusVs...
        UnitMods unitMod;
        switch (attType)
        {
            default:
            case BASE_ATTACK:   unitMod = UNIT_MOD_DAMAGE_MAINHAND; break;
            case OFF_ATTACK:    unitMod = UNIT_MOD_DAMAGE_OFFHAND;  break;
            case RANGED_ATTACK: unitMod = UNIT_MOD_DAMAGE_RANGED;   break;
        }

        DoneTotal += DoneFlat;

        DoneTotal *= GetModifierValue(unitMod, TOTAL_PCT);
    }

    if (!flat)
        DoneTotal = 0.0f;

    float tmpDamage = float(int32(pdamage) + DoneTotal * int32(stack)) * DonePercent;

    // apply spellmod to Done damage
    if (spellProto)
    {
        if (Player* modOwner = GetSpellModOwner())
            modOwner->ApplySpellMod(spellProto->Id, damagetype == DOT ? SPELLMOD_DOT : SPELLMOD_DAMAGE, tmpDamage);
    }

    // bonus result can be negative
    return tmpDamage > 0 ? uint32(tmpDamage) : 0;
}

/**
 * Calculates target part of melee damage bonuses,
 * will be called on each tick for periodic damage over time auras
 */
uint32 Unit::MeleeDamageBonusTaken(Unit* pCaster, uint32 pdamage, WeaponAttackType attType, SpellEntry const* spellProto, DamageEffectType damagetype, uint32 stack, bool flat)
{
    if (!pCaster)
        return pdamage;

    if (pdamage == 0)
        return pdamage;

    // differentiate for weapon damage based spells
    bool isWeaponDamageBasedSpell = !(spellProto && (damagetype == DOT || IsSpellHaveEffect(spellProto, SPELL_EFFECT_SCHOOL_DAMAGE)));
    uint32 schoolMask       = spellProto ? GetSpellSchoolMask(spellProto) : uint32(GetMeleeDamageSchoolMask());

    // FLAT damage bonus auras
    // =======================
    int32 TakenFlat = 0;

    // ..taken flat (base at attack power for marked target and base at attack power for creature type)
    if (attType == RANGED_ATTACK)
        TakenFlat += GetTotalAuraModifier(SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN);
    else
        TakenFlat += GetTotalAuraModifier(SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN);

    // ..taken flat (by school mask)
    TakenFlat += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_DAMAGE_TAKEN, schoolMask);

    // PERCENT damage auras
    // ====================
    float TakenPercent  = 1.0f;

    // ..taken pct (by school mask)
    TakenPercent *= GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, schoolMask);

    // ..taken pct (melee/ranged)
    if (attType == RANGED_ATTACK)
        TakenPercent *= GetTotalAuraMultiplier(SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT);
    else
        TakenPercent *= GetTotalAuraMultiplier(SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT);

    // final calculation
    // =================

    // scaling of non weapon based spells
    if (!isWeaponDamageBasedSpell)
    {
        // apply benefit affected by spell power implicit coeffs and spell level penalties
        TakenFlat = pCaster->SpellBonusWithCoeffs(spellProto, 0, TakenFlat, 0, damagetype, false);
    }

    if (!flat)
        TakenFlat = 0.0f;

    float tmpDamage = float(int32(pdamage) + TakenFlat * int32(stack)) * TakenPercent;

    // bonus result can be negative
    return tmpDamage > 0 ? uint32(tmpDamage) : 0;
}

void Unit::ApplySpellImmune(uint32 spellId, uint32 op, uint32 type, bool apply)
{
    if (apply)
    {
        for (SpellImmuneList::iterator itr = m_spellImmune[op].begin(), next; itr != m_spellImmune[op].end(); itr = next)
        {
            next = itr; ++next;
            if (itr->type == type)
            {
                m_spellImmune[op].erase(itr);
                next = m_spellImmune[op].begin();
            }
        }
        SpellImmune Immune;
        Immune.spellId = spellId;
        Immune.type = type;
        m_spellImmune[op].push_back(Immune);
    }
    else
    {
        for (SpellImmuneList::iterator itr = m_spellImmune[op].begin(); itr != m_spellImmune[op].end(); ++itr)
        {
            if (itr->spellId == spellId)
            {
                m_spellImmune[op].erase(itr);
                break;
            }
        }
    }
}

void Unit::ApplySpellDispelImmunity(const SpellEntry* spellProto, DispelType type, bool apply)
{
    ApplySpellImmune(spellProto->Id, IMMUNITY_DISPEL, type, apply);

    if (apply && spellProto->HasAttribute(SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY))
        RemoveAurasWithDispelType(type);
}

float Unit::GetWeaponProcChance() const
{
    // normalized proc chance for weapon attack speed
    // (odd formula...)
    if (isAttackReady(BASE_ATTACK))
        return (GetAttackTime(BASE_ATTACK) * 1.8f / 1000.0f);
    else if (haveOffhandWeapon() && isAttackReady(OFF_ATTACK))
        return (GetAttackTime(OFF_ATTACK) * 1.6f / 1000.0f);

    return 0.0f;
}

float Unit::GetPPMProcChance(uint32 WeaponSpeed, float PPM) const
{
    // proc per minute chance calculation
    if (PPM <= 0.0f)
        return 0.0f;
    return WeaponSpeed * PPM / 600.0f;                      // result is chance in percents (probability = Speed_in_sec * (PPM / 60))
}

void Unit::Mount(uint32 mount, uint32 spellId)
{
    if (!mount)
        return;

    RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_MOUNTING);

    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, mount);

    if (GetTypeId() == TYPEID_PLAYER)
    {
        // Called by Taxi system / GM command
        if (!spellId)
            ((Player*)this)->UnsummonPetTemporaryIfAny();
        // Called by mount aura
        else if (Pet* pet = GetPet())
        {
            if (sWorld.getConfig(CONFIG_BOOL_PET_UNSUMMON_AT_MOUNT))
                ((Player*)this)->UnsummonPetTemporaryIfAny();
            else 
                pet->SetModeFlags(PET_MODE_DISABLE_ACTIONS);
        }
    }
}

void Unit::Unmount(bool from_aura)
{
    if (!IsMounted())
        return;

    RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_NOT_MOUNTED);

    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);

    // Called NOT by Taxi system / GM command
    if (from_aura)
    {
        WorldPacket data(SMSG_DISMOUNT, 8);
        data << GetPackGUID();
        SendMessageToSet(data, true);
    }

    // only resummon old pet if the player is already added to a map
    // this prevents adding a pet to a not created map which would otherwise cause a crash
    // (it could probably happen when logging in after a previous crash)
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (Pet* pet = GetPet())
        {
            if (CharmInfo* charmInfo = pet->GetCharmInfo())
                pet->SetModeFlags(PetModeFlags(charmInfo->GetReactState() | charmInfo->GetCommandState() * 0x100));
        }
        else
            ((Player*)this)->ResummonPetTemporaryUnSummonedIfAny();
    }
}

void Unit::SetInCombatWith(Unit* enemy)
{
    Unit* eOwner = enemy->GetCharmerOrOwnerOrSelf();
    if (eOwner->IsPvP())
    {
        SetInCombatState(true, enemy);
        return;
    }

    // check for duel
    if (eOwner->GetTypeId() == TYPEID_PLAYER && ((Player*)eOwner)->duel)
    {
        if (Player const* myOwner = GetCharmerOrOwnerPlayerOrPlayerItself())
        {
            if (myOwner->IsInDuelWith((Player const*)eOwner))
            {
                SetInCombatState(true, enemy);
                return;
            }
        }
    }

    SetInCombatState(false, enemy);
}

void Unit::SetInDummyCombatState(bool state)
{
    if (state)
    {
        m_dummyCombatState = true;
        SetInCombatState(false);
    }
    else
        m_dummyCombatState = false;
}

void Unit::SetInCombatState(bool PvP, Unit* enemy)
{
    // only alive units can be in combat
    if (!isAlive())
        return;

    if (PvP)
        m_CombatTimer = 5000;

    bool creatureNotInCombat = GetTypeId() == TYPEID_UNIT && !HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

    if (isCharmed() || (GetTypeId() != TYPEID_PLAYER && ((Creature*)this)->IsPet()))
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT);

    // interrupt all delayed non-combat casts
    for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; ++i)
        if (Spell* spell = GetCurrentSpell(CurrentSpellTypes(i)))
            if (IsNonCombatSpell(spell->m_spellInfo))
                InterruptSpell(CurrentSpellTypes(i), false);

    if (creatureNotInCombat)
    {
        // should probably be removed for the attacked (+ it's party/group) only, not global
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);

        Creature* pCreature = (Creature*)this;

        if (pCreature->AI())
            pCreature->AI()->EnterCombat(enemy);

        // Some bosses are set into combat with zone
        if (GetMap()->IsDungeon() && (pCreature->GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_AGGRO_ZONE) && enemy && enemy->IsControlledByPlayer())
            pCreature->SetInCombatWithZone();

        if (InstanceData* mapInstance = GetInstanceData())
            mapInstance->OnCreatureEnterCombat(pCreature);

        if (m_isCreatureLinkingTrigger)
            GetMap()->GetCreatureLinkingHolder()->DoCreatureLinkingEvent(LINKING_EVENT_AGGRO, pCreature, enemy);
    }
}

void Unit::ClearInCombat()
{
    m_CombatTimer = 0;
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

    if (isCharmed() || (GetTypeId() != TYPEID_PLAYER && ((Creature*)this)->IsPet()))
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT);

    // Player's state will be cleared in Player::UpdateContestedPvP
    if (GetTypeId() == TYPEID_UNIT)
    {
        Creature* cThis = static_cast<Creature*>(this);
        if (cThis->GetCreatureInfo()->UnitFlags & UNIT_FLAG_OOC_NOT_ATTACKABLE && !(cThis->GetTemporaryFactionFlags() & TEMPFACTION_TOGGLE_OOC_NOT_ATTACK))
            SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);

        clearUnitState(UNIT_STAT_ATTACK_PLAYER);
    }
}

bool Unit::isTargetableForAttack(bool inverseAlive /*=false*/) const
{
    if (!CanBeDetected())
        return false;

    if (GetTypeId() == TYPEID_PLAYER && ((Player*)this)->isGameMaster())
        return false;

    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE))
        return false;

    // to be removed if unit by any reason enter combat
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE))
        return false;

    // inversealive is needed for some spells which need to be casted at dead targets (aoe)
    if (isAlive() == inverseAlive)
        return false;

    return IsInWorld() && !hasUnitState(UNIT_STAT_DIED) && !IsTaxiFlying();
}

int32 Unit::ModifyHealth(int32 dVal)
{
    if (dVal == 0)
        return 0;

    int32 curHealth = (int32)GetHealth();

    int32 val = dVal + curHealth;
    if (val <= 0)
    {
        SetHealth(0);
        return -curHealth;
    }

    int32 maxHealth = (int32)GetMaxHealth();

    int32 gain;
    if (val < maxHealth)
    {
        SetHealth(val);
        gain = val - curHealth;
    }
    else
    {
        SetHealth(maxHealth);
        gain = maxHealth - curHealth;
    }

    return gain;
}

int32 Unit::ModifyPower(Powers power, int32 dVal)
{
    if (dVal == 0)
        return 0;

    int32 curPower = (int32)GetPower(power);

    int32 val = dVal + curPower;
    if (val <= 0)
    {
        SetPower(power, 0);
        return -curPower;
    }

    int32 maxPower = (int32)GetMaxPower(power);

    int32 gain;
    if (val < maxPower)
    {
        SetPower(power, val);
        gain = val - curPower;
    }
    else
    {
        SetPower(power, maxPower);
        gain = maxPower - curPower;
    }

    return gain;
}

bool Unit::isVisibleForOrDetect(Unit const* u, WorldObject const* viewPoint, bool detect, bool inVisibleList, bool is3dDistance) const
{
    if (!u || !IsInMap(u))
        return false;

    // Always can see self
    if (u == this)
        return true;

    // player visible for other player if not logout and at same transport
    // including case when player is out of world
    bool at_same_transport =
        GetTypeId() == TYPEID_PLAYER &&  u->GetTypeId() == TYPEID_PLAYER &&
        !((Player*)this)->GetSession()->PlayerLogout() && !((Player*)u)->GetSession()->PlayerLogout() &&
        !((Player*)this)->GetSession()->PlayerLoading() && !((Player*)u)->GetSession()->PlayerLoading() &&
        ((Player*)this)->GetTransport() && ((Player*)this)->GetTransport() == ((Player*)u)->GetTransport();

    // not in world
    if (!at_same_transport && (!IsInWorld() || !u->IsInWorld()))
        return false;

    // forbidden to seen (while Removing corpse)
    if (m_Visibility == VISIBILITY_REMOVE_CORPSE)
        return false;

    Map& _map = *u->GetMap();
    // Grid dead/alive checks
    if (u->GetTypeId() == TYPEID_PLAYER)
    {
        // non visible at grid for any stealth state
        if (!IsVisibleInGridForPlayer((Player*)u))
            return false;

        // if player is dead then he can't detect anyone in any cases
        if (!u->isAlive())
            detect = false;
    }
    else
    {
        // all dead creatures/players not visible for any creatures
        if (!u->isAlive() || !isAlive())
            return false;
    }

    // different visible distance checks
    if (u->IsTaxiFlying())                                  // what see player in flight
    {
        // use object grey distance for all (only see objects any way)
        if (!IsWithinDistInMap(viewPoint, World::GetMaxVisibleDistanceInFlight() + (inVisibleList ? World::GetVisibleObjectGreyDistance() : 0.0f), is3dDistance))
            return false;
    }
    else if (!at_same_transport)                            // distance for show player/pet/creature (no transport case)
    {
        // Any units far than max visible distance for viewer or not in our map are not visible too
        if (!IsWithinDistInMap(viewPoint, _map.GetVisibilityDistance() + (inVisibleList ? World::GetVisibleUnitGreyDistance() : 0.0f), is3dDistance))
            return false;
    }

    // always seen by owner
    if (GetCharmerOrOwnerGuid() == u->GetObjectGuid())
        return true;

    // isInvisibleForAlive() those units can only be seen by dead or if other
    // unit is also invisible for alive.. if an isinvisibleforalive unit dies we
    // should be able to see it too
    if (u->isAlive() && isAlive() && isInvisibleForAlive() != u->isInvisibleForAlive())
        if (u->GetTypeId() != TYPEID_PLAYER || !((Player*)u)->isGameMaster())
            return false;

    // Visible units, always are visible for all units
    if (m_Visibility == VISIBILITY_ON)
        return true;

    // GMs see any players, not higher GMs and all units
    if (u->GetTypeId() == TYPEID_PLAYER && ((Player*)u)->isGameMaster())
    {
        if (GetTypeId() == TYPEID_PLAYER)
            return ((Player*)this)->GetSession()->GetSecurity() <= ((Player*)u)->GetSession()->GetSecurity();
        else
            return true;
    }

    // non faction visibility non-breakable for non-GMs
    if (m_Visibility == VISIBILITY_OFF)
        return false;

    // grouped players should always see stealthed party members
    if (GetTypeId() == TYPEID_PLAYER && u->GetTypeId() == TYPEID_PLAYER)
        if (((Player*)this)->IsGroupVisibleFor(((Player*)u)) && u->IsFriendlyTo(this))
            return true;

    // raw invisibility
    bool invisible = (m_invisibilityMask != 0 || u->m_invisibilityMask != 0);
    if (u->GetTypeId() == TYPEID_PLAYER) // if object is player with mover, use its visibility masks, so that an invisible player MCing a creature can see stuff
    {
        if (Player* player = (Player*)u)
        {
            if (Unit* mover=player->GetMover())
            {
                invisible= (m_invisibilityMask != 0 || mover->m_invisibilityMask != 0);
            }
        }
    }

    // detectable invisibility case
    if (invisible && (
                // Invisible units, always are visible for units under same invisibility type
                (m_invisibilityMask & u->m_invisibilityMask) != 0 ||
                // Invisible units, always are visible for unit that can detect this invisibility (have appropriate level for detect)
                u->canDetectInvisibilityOf(this)))
    {
        invisible = false;
    }

    // special cases for always overwrite invisibility/stealth
    if (invisible || m_Visibility == VISIBILITY_GROUP_STEALTH)
    {
        if (u->IsHostileTo(this))
        {
            // Hunter mark functionality
            AuraList const& auras = GetAurasByType(SPELL_AURA_MOD_STALKED);
            for (AuraList::const_iterator iter = auras.begin(); iter != auras.end(); ++iter)
                if ((*iter)->GetCasterGuid() == u->GetObjectGuid())
                    return true;
        }

        // none other cases for detect invisibility, so invisible
        if (invisible)
            return false;
    }

    // unit got in stealth in this moment and must ignore old detected state
    if (m_Visibility == VISIBILITY_GROUP_NO_DETECT)
        return false;

    // GM invisibility checks early, invisibility if any detectable, so if not stealth then visible
    if (m_Visibility != VISIBILITY_GROUP_STEALTH)
        return true;

    // NOW ONLY STEALTH CASE

    // if in non-detect mode then invisible for unit
    // mobs always detect players (detect == true)... return 'false' for those mobs which have (detect == false)
    // players detect players only in Player::HandleStealthedUnitsDetection()
    if (!detect)
        return (u->GetTypeId() == TYPEID_PLAYER) ? ((Player*)u)->HaveAtClient(this) : false;

    // Special cases

    // If is attacked then stealth is lost, some creature can use stealth too
    if (!getAttackers().empty())
        return true;

    // If there is collision rogue is seen regardless of level difference
    if (IsWithinDist(u, 0.24f))
        return true;

    // If a mob or player is stunned he will not be able to detect stealth
    if (u->hasUnitState(UNIT_STAT_STUNNED) && (u != this))
        return false;

    // set max ditance
    float visibleDistance = (u->GetTypeId() == TYPEID_PLAYER) ? MAX_PLAYER_STEALTH_DETECT_RANGE : ((Creature const*)u)->GetAttackDistance(this);

    // Always invisible from back (when stealth detection is on), also filter max distance cases
    bool isInFront = viewPoint->isInFrontInMap(this, visibleDistance);
    if (!isInFront)
        return false;

    // Calculation if target is in front

    // Visible distance based on stealth value (stealth rank 4 300MOD, 10.5 - 3 = 7.5)
    visibleDistance = 10.5f - (GetTotalAuraModifier(SPELL_AURA_MOD_STEALTH) / 100.0f);

    // Visible distance is modified by
    //-Level Diff (every level diff = 1.0f in visible distance)
    visibleDistance += int32(u->GetLevelForTarget(this)) - int32(GetLevelForTarget(u));

    // This allows to check talent tree and will add addition stealth dependent on used points)
    int32 stealthMod = GetTotalAuraModifier(SPELL_AURA_MOD_STEALTH_LEVEL);
    if (stealthMod < 0)
        stealthMod = 0;

    //-Stealth Mod(positive like Master of Deception) and Stealth Detection(negative like paranoia)
    // based on wowwiki every 5 mod we have 1 more level diff in calculation
    visibleDistance += (int32(u->GetTotalAuraModifier(SPELL_AURA_MOD_STEALTH_DETECT)) - stealthMod) / 5.0f;
    visibleDistance = visibleDistance > MAX_PLAYER_STEALTH_DETECT_RANGE ? MAX_PLAYER_STEALTH_DETECT_RANGE : visibleDistance;

    // recheck new distance
    if (visibleDistance <= 0 || !IsWithinDist(viewPoint, visibleDistance))
        return false;

    // Now check is target visible with LoS
    float ox, oy, oz;
    viewPoint->GetPosition(ox, oy, oz);
    return IsWithinLOS(ox, oy, oz);
}

void Unit::UpdateVisibilityAndView()
{

    static const AuraType auratypes[] = {SPELL_AURA_BIND_SIGHT, SPELL_AURA_FAR_SIGHT, SPELL_AURA_NONE};
    for (AuraType const* type = &auratypes[0]; *type != SPELL_AURA_NONE; ++type)
    {
        AuraList& alist = m_modAuras[*type];
        if (alist.empty())
            continue;

        for (AuraList::iterator it = alist.begin(); it != alist.end();)
        {
            Aura* aura = (*it);
            Unit* owner = aura->GetCaster();

            if (!owner || !isVisibleForOrDetect(owner, this, false))
            {
                alist.erase(it);
                RemoveAura(aura);
                it = alist.begin();
            }
            else
                ++it;
        }
    }

    GetViewPoint().Call_UpdateVisibilityForOwner();
    UpdateObjectVisibility();
    ScheduleAINotify(0);
    GetViewPoint().Event_ViewPointVisibilityChanged();
}

void Unit::SetVisibility(UnitVisibility x)
{
    m_Visibility = x;

    if (IsInWorld())
        UpdateVisibilityAndView();
}

bool Unit::canDetectInvisibilityOf(Unit const* u) const
{
    if (uint32 mask = (m_detectInvisibilityMask & u->m_invisibilityMask))
    {
        for (int32 i = 0; i < 32; ++i)
        {
            if (((1 << i) & mask) == 0)
                continue;

            // find invisibility level
            int32 invLevel = 0;
            Unit::AuraList const& iAuras = u->GetAurasByType(SPELL_AURA_MOD_INVISIBILITY);
            for (Unit::AuraList::const_iterator itr = iAuras.begin(); itr != iAuras.end(); ++itr)
                if ((*itr)->GetModifier()->m_miscvalue == i && invLevel < (*itr)->GetModifier()->m_amount)
                    invLevel = (*itr)->GetModifier()->m_amount;

            // find invisibility detect level
            int32 detectLevel = 0;
            Unit::AuraList const& dAuras = GetAurasByType(SPELL_AURA_MOD_INVISIBILITY_DETECTION);
            for (Unit::AuraList::const_iterator itr = dAuras.begin(); itr != dAuras.end(); ++itr)
                if ((*itr)->GetModifier()->m_miscvalue == i && detectLevel < (*itr)->GetModifier()->m_amount)
                    detectLevel = (*itr)->GetModifier()->m_amount;

            if (i == 6 && GetTypeId() == TYPEID_PLAYER)     // special drunk detection case
                detectLevel = ((Player*)this)->GetDrunkValue();

            if (invLevel <= detectLevel)
                return true;
        }
    }

    return false;
}

void Unit::UpdateSpeed(UnitMoveType mtype, bool forced, float ratio)
{
    // not in combat pet have same speed as owner
    switch (mtype)
    {
        case MOVE_RUN:
        case MOVE_WALK:
        case MOVE_SWIM:
            if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->IsPet() && hasUnitState(UNIT_STAT_FOLLOW))
            {
                if (Unit* owner = GetOwner())
                {
                    SetSpeedRate(mtype, owner->GetSpeedRate(mtype), forced);
                    return;
                }
            }
            break;
        default:
            break;
    }

    int32 main_speed_mod  = 0;
    float stack_bonus     = 1.0f;
    float non_stack_bonus = 1.0f;

    switch (mtype)
    {
        case MOVE_WALK:
            break;
        case MOVE_RUN:
        {
            if (IsMounted()) // Use on mount auras
            {
                main_speed_mod  = GetMaxPositiveAuraModifier(SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED);
                stack_bonus     = GetTotalAuraMultiplier(SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS);
                non_stack_bonus = (100.0f + GetMaxPositiveAuraModifier(SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK)) / 100.0f;
            }
            else
            {
                main_speed_mod  = GetMaxPositiveAuraModifier(SPELL_AURA_MOD_INCREASE_SPEED);
                stack_bonus     = GetTotalAuraMultiplier(SPELL_AURA_MOD_SPEED_ALWAYS);
                non_stack_bonus = (100.0f + GetMaxPositiveAuraModifier(SPELL_AURA_MOD_SPEED_NOT_STACK)) / 100.0f;
            }
            break;
        }
        case MOVE_RUN_BACK:
            return;
        case MOVE_SWIM:
        {
            main_speed_mod  = GetMaxPositiveAuraModifier(SPELL_AURA_MOD_INCREASE_SWIM_SPEED);
            break;
        }
        case MOVE_SWIM_BACK:
            return;
        default:
            sLog.outError("Unit::UpdateSpeed: Unsupported move type (%d)", mtype);
            return;
    }

    float bonus = non_stack_bonus > stack_bonus ? non_stack_bonus : stack_bonus;
    // now we ready for speed calculation
    float speed  = main_speed_mod ? bonus * (100.0f + main_speed_mod) / 100.0f : bonus;

    switch (mtype)
    {
        case MOVE_RUN:
        case MOVE_SWIM:
        {
            // Normalize speed by 191 aura SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED if need
            // TODO: possible affect only on MOVE_RUN
            if (int32 normalization = GetMaxPositiveAuraModifier(SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED))
            {
                // Use speed from aura
                float max_speed = normalization / baseMoveSpeed[mtype];
                if (speed > max_speed)
                    speed = max_speed;
            }
            break;
        }
        default:
            break;
    }

    // for creature case, we check explicit if mob searched for assistance
    if (GetTypeId() == TYPEID_UNIT)
    {
        if (((Creature*)this)->HasSearchedAssistance())
            speed *= 0.66f;                                 // best guessed value, so this will be 33% reduction. Based off initial speed, mob can then "run", "walk fast" or "walk".
    }
    // for player case, we look for some custom rates
    else
    {
        if (getDeathState() == CORPSE)
            speed *= sWorld.getConfig(((Player*)this)->InBattleGround() ? CONFIG_FLOAT_GHOST_RUN_SPEED_BG : CONFIG_FLOAT_GHOST_RUN_SPEED_WORLD);
    }

    // Apply strongest slow aura mod to speed
    int32 slow = GetMaxNegativeAuraModifier(SPELL_AURA_MOD_DECREASE_SPEED);
    if (slow)
        speed *= (100.0f + slow) / 100.0f;

    if (GetTypeId() == TYPEID_UNIT)
    {
        switch (mtype)
        {
            case MOVE_RUN:
                speed *= ((Creature*)this)->GetCreatureInfo()->SpeedRun;
                break;
            case MOVE_WALK:
                speed *= ((Creature*)this)->GetCreatureInfo()->SpeedWalk;
                break;
            default:
                break;
        }
    }

    SetSpeedRate(mtype, speed * ratio, forced);
}

float Unit::GetSpeed(UnitMoveType mtype) const
{
    return m_speed_rate[mtype] * baseMoveSpeed[mtype];
}

struct SetSpeedRateHelper
{
    explicit SetSpeedRateHelper(UnitMoveType _mtype, bool _forced) : mtype(_mtype), forced(_forced) {}
    void operator()(Unit* unit) const { unit->UpdateSpeed(mtype, forced); }
    UnitMoveType mtype;
    bool forced;
};

void Unit::SetSpeedRate(UnitMoveType mtype, float rate, bool forced)
{
    rate = std::max(rate, 0.01f);

    // Update speed only on change
    if (m_speed_rate[mtype] != rate)
    {
        m_speed_rate[mtype] = rate;

        propagateSpeedChange();

        typedef const uint16 SpeedOpcodePair[2];
        SpeedOpcodePair SetSpeed2Opc_table[MAX_MOVE_TYPE] =
        {
            {SMSG_FORCE_WALK_SPEED_CHANGE,        SMSG_SPLINE_SET_WALK_SPEED},
            {SMSG_FORCE_RUN_SPEED_CHANGE,         SMSG_SPLINE_SET_RUN_SPEED},
            {SMSG_FORCE_RUN_BACK_SPEED_CHANGE,    SMSG_SPLINE_SET_RUN_BACK_SPEED},
            {SMSG_FORCE_SWIM_SPEED_CHANGE,        SMSG_SPLINE_SET_SWIM_SPEED},
            {SMSG_FORCE_SWIM_BACK_SPEED_CHANGE,   SMSG_SPLINE_SET_SWIM_BACK_SPEED},
            {SMSG_FORCE_TURN_RATE_CHANGE,         SMSG_SPLINE_SET_TURN_RATE},
        };

        const SpeedOpcodePair& speedOpcodes = SetSpeed2Opc_table[mtype];
        
        if (forced && GetTypeId() == TYPEID_PLAYER)
        {
            // register forced speed changes for WorldSession::HandleForceSpeedChangeAck
            // and do it only for real sent packets and use run for run/mounted as client expected
            ++((Player*)this)->m_forced_speed_changes[mtype];

            WorldPacket data(speedOpcodes[0], 18);
            data << GetPackGUID();
            data << (uint32)0;                              // moveEvent, NUM_PMOVE_EVTS = 0x39
            data << float(GetSpeed(mtype));
            ((Player*)this)->GetSession()->SendPacket(data);
        }
        WorldPacket data(speedOpcodes[1], 12);
        data << GetPackGUID();
        data << float(GetSpeed(mtype));
        SendMessageToSet(data, false);
    }

    CallForAllControlledUnits(SetSpeedRateHelper(mtype, forced), CONTROLLED_PET | CONTROLLED_GUARDIANS | CONTROLLED_CHARM | CONTROLLED_MINIPET);
}

void Unit::SetDeathState(DeathState s)
{
    if (s == JUST_DIED)
    {
        RemoveAllAurasOnDeath();
        RemoveGuardians();
        UnsummonAllTotems();

        StopMoving(true);
        i_motionMaster.Clear(false, true);
        i_motionMaster.MoveIdle();

        ModifyAuraState(AURA_STATE_HEALTHLESS_20_PERCENT, false);
        // remove aurastates allowing special moves
        ClearAllReactives();
        ClearDiminishings();
    }
    else if (s == JUST_ALIVED)
    {
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);  // clear skinnable for creature and player (at battleground)
    }

    if (s != ALIVE && s != JUST_ALIVED)
    {
        CombatStop();
        DeleteThreatList();
        ClearComboPointHolders();                           // any combo points pointed to unit lost at it death

        if (IsNonMeleeSpellCasted(false))
            InterruptNonMeleeSpells(false);
    }

    // TODO:: what is/was that?
    /*if (m_deathState != ALIVE && s == ALIVE)
    {
        //_ApplyAllAuraMods();
    }*/
    m_deathState = s;
}

/*########################################
########                          ########
########       AGGRO SYSTEM       ########
########                          ########
########################################*/

// Check if this unit is controlled by provided controller or by one of its friendly faction
bool Unit::IsTargetUnderControl(Unit const& target) const
{
    ObjectGuid charmerGuid = GetCharmerGuid();

    if (!charmerGuid)
        return false;
    
    if (target.GetCharmerGuid() == charmerGuid)
        return true;

    // also check if this unit is controlled by another creature from friendly faction
    if (Unit const* charmer = GetMap()->GetUnit(charmerGuid))
    {
        FactionTemplateEntry const* charmerFactionEntry = charmer->getFactionTemplateEntry();
        FactionTemplateEntry const* controllerFactionEntry = target.getFactionTemplateEntry();
        if (controllerFactionEntry && charmerFactionEntry && !controllerFactionEntry->IsHostileTo(*controllerFactionEntry))
            return true;
    }

    return false;
}

bool Unit::CanHaveThreatList(bool ignoreAliveState/*=false*/) const
{
    // only creatures can have threat list
    if (GetTypeId() != TYPEID_UNIT)
        return false;

    // only alive units can have threat list
    if (!isAlive() && !ignoreAliveState)
        return false;

    Creature const* creature = ((Creature const*)this);

    // totems can not have threat list
    if (creature->IsTotem())
        return false;

    // pets can not have a threat list, unless they are controlled by a creature
    if (creature->IsPet() && creature->GetOwnerGuid().IsPlayer())
    {
        Pet const* pet = static_cast<Pet const*>(creature);
        if (pet->getPetType() == GUARDIAN_PET)
            return true;
        return false;
    }

    // charmed units can not have a threat list if charmed by player
    if (creature->GetCharmerGuid().IsPlayer())
        return false;

    return true;
}

//======================================================================

float Unit::ApplyTotalThreatModifier(float threat, SpellSchoolMask schoolMask)
{
    if (!HasAuraType(SPELL_AURA_MOD_THREAT))
        return threat;

    if (schoolMask == SPELL_SCHOOL_MASK_NONE)
        return threat;

    SpellSchools school = GetFirstSchoolInMask(schoolMask);

    return threat * m_threatModifier[school];
}

//======================================================================

void Unit::AddThreat(Unit* pVictim, float threat /*= 0.0f*/, bool crit /*= false*/, SpellSchoolMask schoolMask /*= SPELL_SCHOOL_MASK_NONE*/, SpellEntry const* threatSpell /*= nullptr*/)
{
    // Only mobs can manage threat lists
    if (CanHaveThreatList())
        getThreatManager().addThreat(pVictim, threat, crit, schoolMask, threatSpell);
}

//======================================================================

void Unit::DeleteThreatList()
{
    getThreatManager().clearReferences();
}

//======================================================================

void Unit::TauntApply(Unit* taunter)
{
    MANGOS_ASSERT(GetTypeId() == TYPEID_UNIT);

    if (!taunter || (taunter->GetTypeId() == TYPEID_PLAYER && ((Player*)taunter)->isGameMaster()))
        return;

    if (!CanHaveThreatList())
        return;

    Unit* target = getVictim();

    if (target && target == taunter)
        return;

    // Only attack taunter if this is a valid target
    if (CanReactInCombat() && !IsSecondChoiceTarget(taunter, true, true))
    {
        if (GetTargetGuid() || !target)
            SetInFront(taunter);

        if (AI())
            AI()->AttackStart(taunter);
    }

    getThreatManager().tauntApply(taunter);
}

//======================================================================

void Unit::TauntFadeOut(Unit* taunter)
{
    MANGOS_ASSERT(GetTypeId() == TYPEID_UNIT);

    if (!taunter || (taunter->GetTypeId() == TYPEID_PLAYER && ((Player*)taunter)->isGameMaster()))
        return;

    if (!CanHaveThreatList())
        return;

    Unit* target = getVictim();

    if (!target || target != taunter)
        return;

    if (getThreatManager().isThreatListEmpty())
    {
        m_fixateTargetGuid.Clear();

        if (AI())
            AI()->EnterEvadeMode();

        if (InstanceData* mapInstance = GetInstanceData())
            mapInstance->OnCreatureEvade((Creature*)this);

        if (m_isCreatureLinkingTrigger)
            GetMap()->GetCreatureLinkingHolder()->DoCreatureLinkingEvent(LINKING_EVENT_EVADE, (Creature*)this);

        return;
    }

    getThreatManager().tauntFadeOut(taunter);
    target = getThreatManager().getHostileTarget();

    if (target && target != taunter)
    {
        if (GetTargetGuid())
            SetInFront(target);

        if (AI())
            AI()->AttackStart(target);
    }
}

//======================================================================
/// if pVictim is given, the npc will fixate onto pVictim, if nullptr it will remove current fixation
void Unit::FixateTarget(Unit* pVictim)
{
    if (!pVictim)                                           // Remove Fixation
        m_fixateTargetGuid.Clear();
    else if (pVictim->isTargetableForAttack())              // Apply Fixation
        m_fixateTargetGuid = pVictim->GetObjectGuid();

    // Start attacking the fixated target or the next proper one
    SelectHostileTarget();
}

//======================================================================

bool Unit::IsSecondChoiceTarget(Unit* pTarget, bool taunt, bool checkThreatArea)
{
    MANGOS_ASSERT(pTarget);

    // little hack before handling threatarea in unit instead of creature as charmed players will act like creature
    Creature* thisCreature = GetTypeId() == TYPEID_UNIT ? static_cast<Creature*>(this) : nullptr;

    return
        pTarget->IsTargetUnderControl(*this) ||
        (!taunt && pTarget->IsImmuneToDamage(GetMeleeDamageSchoolMask())) ||
        pTarget->hasNegativeAuraWithInterruptFlag(AURA_INTERRUPT_FLAG_DAMAGE) ||
        (thisCreature && checkThreatArea && thisCreature->IsOutOfThreatArea(pTarget));
}

//======================================================================

bool Unit::SelectHostileTarget()
{
    // function provides main threat functionality
    // next-victim-selection algorithm and evade mode are called
    // threat list sorting etc.

    MANGOS_ASSERT(GetTypeId() == TYPEID_UNIT);

    if (!this->isAlive())
        return false;

    // This function only useful once AI has been initialized
    if (!AI())
        return false;

    Unit* target = nullptr;
    Unit* oldTarget = getVictim();

    // first check if we should fixate a target
    if (m_fixateTargetGuid)
    {
        if (oldTarget && oldTarget->GetObjectGuid() == m_fixateTargetGuid)
            target = oldTarget;
        else
        {
            Unit* pFixateTarget = GetMap()->GetUnit(m_fixateTargetGuid);
            if (pFixateTarget && pFixateTarget->isAlive() && !IsSecondChoiceTarget(pFixateTarget, false, true))
                target = pFixateTarget;
        }
    }
    // then checking if we have some taunt on us
    if (!target)
    {
        const AuraList& tauntAuras = GetAurasByType(SPELL_AURA_MOD_TAUNT);
        Unit* caster;

        // Find first available taunter target
        // Auras are pushed_back, last caster will be on the end
        for (AuraList::const_reverse_iterator aura = tauntAuras.rbegin(); aura != tauntAuras.rend(); ++aura)
        {
            if ((caster = (*aura)->GetCaster()) && caster->IsInMap(this) &&
                    caster->isTargetableForAttack() && caster->isInAccessablePlaceFor((Creature*)this) &&
                    !IsSecondChoiceTarget(caster, true, true))
            {
                target = caster;
                break;
            }
        }
    }

    // No valid fixate target, taunt aura or taunt aura caster is dead, standard target selection
    if (!target && !getThreatManager().isThreatListEmpty())
        target = getThreatManager().getHostileTarget();

    if (target)
    {
        if (CanReactInCombat())
        {
            SetInFront(target);
            if (oldTarget != target)
                AI()->AttackStart(target);

            // check if currently selected target is reachable
            // NOTE: path alrteady generated from AttackStart()
            if (!GetMotionMaster()->GetCurrent()->IsReachable())
            {
                // remove all taunts
                RemoveSpellsCausingAura(SPELL_AURA_MOD_TAUNT);

                if (getThreatManager().getThreatList().size() < 2)
                {
                    // only one target in list, we have to evade after timer
                    // TODO: make timer - inside Creature class
                    AI()->EnterEvadeMode();
                }
                else
                {
                    // remove unreachable target from our threat list
                    // next iteration we will select next possible target
                    getHostileRefManager().deleteReference(target);
                    getThreatManager().modifyThreatPercent(target, -101);
                    // remove target from current attacker, do not exit combat settings
                    AttackStop(true);
                }

                return false;
            }
        }
        return true;
    }

    // no target but something prevent go to evade mode
    if (!isInCombat() || HasAuraType(SPELL_AURA_MOD_TAUNT))
        return false;

    // last case when creature don't must go to evade mode:
    // it in combat but attacker not make any damage and not enter to aggro radius to have record in threat list
    // for example at owner command to pet attack some far away creature
    // Note: creature not have targeted movement generator but have attacker in this case
    if (GetMotionMaster()->GetCurrentMovementGeneratorType() != CHASE_MOTION_TYPE)
    {
        for (AttackerSet::const_iterator itr = m_attackers.begin(); itr != m_attackers.end(); ++itr)
        {
            if ((*itr)->IsInMap(this) && (*itr)->isTargetableForAttack() && (*itr)->isInAccessablePlaceFor((Creature*)this))
                return false;
        }
    }

    // enter in evade mode in other case
    m_fixateTargetGuid.Clear();
    AI()->EnterEvadeMode();

    if (InstanceData* mapInstance = GetInstanceData())
        mapInstance->OnCreatureEvade((Creature*)this);

    if (m_isCreatureLinkingTrigger)
        GetMap()->GetCreatureLinkingHolder()->DoCreatureLinkingEvent(LINKING_EVENT_EVADE, (Creature*)this);

    return false;
}

//======================================================================
//======================================================================
//======================================================================

int32 Unit::CalculateSpellDamage(Unit const* target, SpellEntry const* spellProto, SpellEffectIndex effect_index, int32 const* effBasePoints)
{
    Player* unitPlayer = (GetTypeId() == TYPEID_PLAYER) ? (Player*)this : nullptr;

    uint8 comboPoints = unitPlayer ? unitPlayer->GetComboPoints() : 0;

    int32 level = int32(getLevel());
    if (level > (int32)spellProto->maxLevel && spellProto->maxLevel > 0)
        level = (int32)spellProto->maxLevel;
    else if (level < (int32)spellProto->baseLevel)
        level = (int32)spellProto->baseLevel;
    level -= (int32)spellProto->spellLevel;

    int32 baseDice = int32(spellProto->EffectBaseDice[effect_index]);
    float basePointsPerLevel = spellProto->EffectRealPointsPerLevel[effect_index];
    float randomPointsPerLevel = spellProto->EffectDicePerLevel[effect_index];
    int32 basePoints = effBasePoints
                       ? *effBasePoints - baseDice
                       : spellProto->EffectBasePoints[effect_index];

    basePoints += int32(level * basePointsPerLevel);
    int32 randomPoints = int32(spellProto->EffectDieSides[effect_index] + level * randomPointsPerLevel);
    float comboDamage = spellProto->EffectPointsPerComboPoint[effect_index];

    switch (randomPoints)
    {
        case 0:
        case 1: basePoints += baseDice; break;              // range 1..1
        default:
        {
            // range can have positive (1..rand) and negative (rand..1) values, so order its for irand
            int32 randvalue = baseDice >= randomPoints
                              ? irand(randomPoints, baseDice)
                              : irand(baseDice, randomPoints);

            basePoints += randvalue;
            break;
        }
    }

    int32 value = basePoints;

    // random damage
    if (comboDamage != 0.0f && unitPlayer && target && (target->GetObjectGuid() == unitPlayer->GetComboTargetGuid() || IsOnlySelfTargeting(spellProto)))
        value += (int32)(comboDamage * comboPoints);

    if (Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_ALL_EFFECTS, value);

    if (spellProto->HasAttribute(SPELL_ATTR_LEVEL_DAMAGE_CALCULATION) && spellProto->spellLevel)
    {
        if (spellProto->EffectApplyAuraName[effect_index])
            value += int32(std::max(0, int32(getLevel() - spellProto->maxLevel)) * basePointsPerLevel);
        else if (spellProto->Effect[effect_index] != SPELL_EFFECT_WEAPON_PERCENT_DAMAGE &&
                 spellProto->Effect[effect_index] != SPELL_EFFECT_KNOCK_BACK &&
                 !IsControlledByPlayer())
        {
            value = int32(value * 0.25f * exp(getLevel() * (70 - spellProto->spellLevel) / 1000.0f));
        }
    }
    return value;
}

DiminishingLevels Unit::GetDiminishing(DiminishingGroup group)
{
    for (Diminishing::iterator i = m_Diminishing.begin(); i != m_Diminishing.end(); ++i)
    {
        if (i->DRGroup != group)
            continue;

        if (!i->hitCount)
            return DIMINISHING_LEVEL_1;

        if (!i->hitTime)
            return DIMINISHING_LEVEL_1;

        // If last spell was casted more than 15 seconds ago - reset the count.
        if (i->stack == 0 && WorldTimer::getMSTimeDiff(i->hitTime, WorldTimer::getMSTime()) > 15 * IN_MILLISECONDS)
        {
            i->hitCount = DIMINISHING_LEVEL_1;
            return DIMINISHING_LEVEL_1;
        }
        // or else increase the count.
        else
        {
            return DiminishingLevels(i->hitCount);
        }
    }
    return DIMINISHING_LEVEL_1;
}

void Unit::IncrDiminishing(DiminishingGroup group)
{
    // Checking for existing in the table
    for (Diminishing::iterator i = m_Diminishing.begin(); i != m_Diminishing.end(); ++i)
    {
        if (i->DRGroup != group)
            continue;
        if (i->hitCount < DIMINISHING_LEVEL_IMMUNE)
            i->hitCount += 1;
        return;
    }
    m_Diminishing.push_back(DiminishingReturn(group, WorldTimer::getMSTime(), DIMINISHING_LEVEL_2));
}

void Unit::ApplyDiminishingToDuration(DiminishingGroup group, int32& duration, Unit* caster, DiminishingLevels Level, bool isReflected)
{
    if (duration == -1 || group == DIMINISHING_NONE || (!isReflected && caster->IsFriendlyTo(this)))
        return;

    float mod = 1.0f;

    // Some diminishings applies to mobs too (for example, Stun)
    if ((GetDiminishingReturnsGroupType(group) == DRTYPE_PLAYER && GetTypeId() == TYPEID_PLAYER) || GetDiminishingReturnsGroupType(group) == DRTYPE_ALL)
    {
        DiminishingLevels diminish = Level;
        switch (diminish)
        {
            case DIMINISHING_LEVEL_1: break;
            case DIMINISHING_LEVEL_2: mod = 0.5f; break;
            case DIMINISHING_LEVEL_3: mod = 0.25f; break;
            case DIMINISHING_LEVEL_IMMUNE: mod = 0.0f; break;
            default: break;
        }
    }

    duration = int32(duration * mod);
}

void Unit::ApplyDiminishingAura(DiminishingGroup group, bool apply)
{
    // Checking for existing in the table
    for (Diminishing::iterator i = m_Diminishing.begin(); i != m_Diminishing.end(); ++i)
    {
        if (i->DRGroup != group)
            continue;

        if (apply)
            i->stack += 1;
        else if (i->stack)
        {
            i->stack -= 1;
            // Remember time after last aura from group removed
            if (i->stack == 0)
                i->hitTime = WorldTimer::getMSTime();
        }
        break;
    }
}

bool Unit::isVisibleForInState(Player const* u, WorldObject const* viewPoint, bool inVisibleList) const
{
    return isVisibleForOrDetect(u, viewPoint, false, inVisibleList, false);
}

/// returns true if creature can't be seen by alive units
bool Unit::isInvisibleForAlive() const
{
    if (m_AuraFlags & UNIT_AURAFLAG_ALIVE_INVISIBLE)
        return true;
    // TODO: maybe spiritservices also have just an aura
    return isSpiritService();
}

uint32 Unit::GetCreatureType() const
{
    if (GetTypeId() == TYPEID_PLAYER)
    {
        SpellShapeshiftFormEntry const* ssEntry = sSpellShapeshiftFormStore.LookupEntry(GetShapeshiftForm());
        if (ssEntry && ssEntry->creatureType > 0)
            return ssEntry->creatureType;
        else
            return CREATURE_TYPE_HUMANOID;
    }
    else
        return ((Creature*)this)->GetCreatureInfo()->CreatureType;
}

/*#######################################
########                         ########
########       STAT SYSTEM       ########
########                         ########
#######################################*/

bool Unit::HandleStatModifier(UnitMods unitMod, UnitModifierType modifierType, float amount, bool apply)
{
    if (unitMod >= UNIT_MOD_END || modifierType >= MODIFIER_TYPE_END)
    {
        sLog.outError("ERROR in HandleStatModifier(): nonexistent UnitMods or wrong UnitModifierType!");
        return false;
    }

    float val;

    switch (modifierType)
    {
        case BASE_VALUE:
        case BASE_EXCLUSIVE:
        case TOTAL_VALUE:
            m_auraModifiersGroup[unitMod][modifierType] += apply ? amount : -amount;
            break;
        case BASE_PCT:
        case TOTAL_PCT:
            if (amount <= -100.0f)                          // small hack-fix for -100% modifiers
                amount = -200.0f;

            val = (100.0f + amount) / 100.0f;
            m_auraModifiersGroup[unitMod][modifierType] *= apply ? val : (1.0f / val);
            break;

        default:
            break;
    }

    if (!CanModifyStats())
        return false;

    switch (unitMod)
    {
        case UNIT_MOD_STAT_STRENGTH:
        case UNIT_MOD_STAT_AGILITY:
        case UNIT_MOD_STAT_STAMINA:
        case UNIT_MOD_STAT_INTELLECT:
        case UNIT_MOD_STAT_SPIRIT:         UpdateStats(GetStatByAuraGroup(unitMod));  break;

        case UNIT_MOD_ARMOR:               UpdateArmor();           break;
        case UNIT_MOD_HEALTH:              UpdateMaxHealth();       break;

        case UNIT_MOD_MANA:
        case UNIT_MOD_RAGE:
        case UNIT_MOD_FOCUS:
        case UNIT_MOD_ENERGY:
        case UNIT_MOD_HAPPINESS:           UpdateMaxPower(GetPowerTypeByAuraGroup(unitMod)); break;

        case UNIT_MOD_RESISTANCE_HOLY:
        case UNIT_MOD_RESISTANCE_FIRE:
        case UNIT_MOD_RESISTANCE_NATURE:
        case UNIT_MOD_RESISTANCE_FROST:
        case UNIT_MOD_RESISTANCE_SHADOW:
        case UNIT_MOD_RESISTANCE_ARCANE:   UpdateResistances(GetSpellSchoolByAuraGroup(unitMod)); break;

        case UNIT_MOD_ATTACK_POWER:        UpdateAttackPowerAndDamage();         break;
        case UNIT_MOD_ATTACK_POWER_RANGED: UpdateAttackPowerAndDamage(true);     break;

        case UNIT_MOD_DAMAGE_MAINHAND:     UpdateDamagePhysical(BASE_ATTACK);    break;
        case UNIT_MOD_DAMAGE_OFFHAND:      UpdateDamagePhysical(OFF_ATTACK);     break;
        case UNIT_MOD_DAMAGE_RANGED:       UpdateDamagePhysical(RANGED_ATTACK);  break;

        default:
            break;
    }

    return true;
}

float Unit::GetModifierValue(UnitMods unitMod, UnitModifierType modifierType) const
{
    if (unitMod >= UNIT_MOD_END || modifierType >= MODIFIER_TYPE_END)
    {
        sLog.outError("attempt to access nonexistent modifier value from UnitMods!");
        return 0.0f;
    }

    if (modifierType == TOTAL_PCT && m_auraModifiersGroup[unitMod][modifierType] <= 0.0f)
        return 0.0f;

    return m_auraModifiersGroup[unitMod][modifierType];
}

float Unit::GetTotalStatValue(Stats stat) const
{
    UnitMods unitMod = UnitMods(UNIT_MOD_STAT_START + stat);

    if (m_auraModifiersGroup[unitMod][TOTAL_PCT] <= 0.0f)
        return 0.0f;

    // value = ((base_value * base_pct) + total_value) * total_pct
    float value  = m_auraModifiersGroup[unitMod][BASE_VALUE] + GetCreateStat(stat);
    value *= m_auraModifiersGroup[unitMod][BASE_PCT];
    value += m_auraModifiersGroup[unitMod][TOTAL_VALUE];
    value *= m_auraModifiersGroup[unitMod][TOTAL_PCT];

    return value;
}

float Unit::GetTotalAuraModValue(UnitMods unitMod) const
{
    if (unitMod >= UNIT_MOD_END)
    {
        sLog.outError("attempt to access nonexistent UnitMods in GetTotalAuraModValue()!");
        return 0.0f;
    }

    if (m_auraModifiersGroup[unitMod][TOTAL_PCT] <= 0.0f)
        return 0.0f;

    float value  = m_auraModifiersGroup[unitMod][BASE_VALUE];
    value += m_auraModifiersGroup[unitMod][BASE_EXCLUSIVE];
    value *= m_auraModifiersGroup[unitMod][BASE_PCT];
    value += m_auraModifiersGroup[unitMod][TOTAL_VALUE];
    value *= m_auraModifiersGroup[unitMod][TOTAL_PCT];

    return value;
}

SpellSchools Unit::GetSpellSchoolByAuraGroup(UnitMods unitMod) const
{
    SpellSchools school = SPELL_SCHOOL_NORMAL;

    switch (unitMod)
    {
        case UNIT_MOD_RESISTANCE_HOLY:     school = SPELL_SCHOOL_HOLY;          break;
        case UNIT_MOD_RESISTANCE_FIRE:     school = SPELL_SCHOOL_FIRE;          break;
        case UNIT_MOD_RESISTANCE_NATURE:   school = SPELL_SCHOOL_NATURE;        break;
        case UNIT_MOD_RESISTANCE_FROST:    school = SPELL_SCHOOL_FROST;         break;
        case UNIT_MOD_RESISTANCE_SHADOW:   school = SPELL_SCHOOL_SHADOW;        break;
        case UNIT_MOD_RESISTANCE_ARCANE:   school = SPELL_SCHOOL_ARCANE;        break;

        default:
            break;
    }

    return school;
}

Stats Unit::GetStatByAuraGroup(UnitMods unitMod) const
{
    Stats stat = STAT_STRENGTH;

    switch (unitMod)
    {
        case UNIT_MOD_STAT_STRENGTH:    stat = STAT_STRENGTH;      break;
        case UNIT_MOD_STAT_AGILITY:     stat = STAT_AGILITY;       break;
        case UNIT_MOD_STAT_STAMINA:     stat = STAT_STAMINA;       break;
        case UNIT_MOD_STAT_INTELLECT:   stat = STAT_INTELLECT;     break;
        case UNIT_MOD_STAT_SPIRIT:      stat = STAT_SPIRIT;        break;

        default:
            break;
    }

    return stat;
}

Powers Unit::GetPowerTypeByAuraGroup(UnitMods unitMod) const
{
    switch (unitMod)
    {
        case UNIT_MOD_MANA:       return POWER_MANA;
        case UNIT_MOD_RAGE:       return POWER_RAGE;
        case UNIT_MOD_FOCUS:      return POWER_FOCUS;
        case UNIT_MOD_ENERGY:     return POWER_ENERGY;
        case UNIT_MOD_HAPPINESS:  return POWER_HAPPINESS;
        default:                  return POWER_MANA;
    }
}

float Unit::GetTotalAttackPowerValue(WeaponAttackType attType) const
{
    if (attType == RANGED_ATTACK)
    {
        int32 ap = GetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER) + GetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS);
        if (ap < 0)
            return 0.0f;
        return ap * (1.0f + GetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER));
    }
    else
    {
        int32 ap = GetInt32Value(UNIT_FIELD_ATTACK_POWER) + GetInt32Value(UNIT_FIELD_ATTACK_POWER_MODS);
        if (ap < 0)
            return 0.0f;
        return ap * (1.0f + GetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER));
    }
}

float Unit::GetWeaponDamageRange(WeaponAttackType attType, WeaponDamageRange damageRange, uint8 index) const
{
    if (attType == OFF_ATTACK && !haveOffhandWeapon())
        return 0.0f;

    return m_weaponDamage[attType][index].damage[damageRange];
}

void Unit::SetLevel(uint32 lvl)
{
    SetUInt32Value(UNIT_FIELD_LEVEL, lvl);

    // group update
    if ((GetTypeId() == TYPEID_PLAYER) && ((Player*)this)->GetGroup())
        ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_LEVEL);
}

void Unit::SetHealth(uint32 val)
{
    uint32 maxHealth = GetMaxHealth();
    if (maxHealth < val)
        val = maxHealth;

    SetUInt32Value(UNIT_FIELD_HEALTH, val);

    // group update
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (((Player*)this)->GetGroup())
            ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_CUR_HP);
    }
    else if (((Creature*)this)->IsPet())
    {
        Pet* pet = ((Pet*)this);
        if (pet->isControlled())
        {
            Unit* owner = GetOwner();
            if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && ((Player*)owner)->GetGroup())
                ((Player*)owner)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_CUR_HP);
        }
    }
}

void Unit::SetMaxHealth(uint32 val)
{
    uint32 health = GetHealth();
    SetUInt32Value(UNIT_FIELD_MAXHEALTH, val);

    // group update
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (((Player*)this)->GetGroup())
            ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_MAX_HP);
    }
    else if (((Creature*)this)->IsPet())
    {
        Pet* pet = ((Pet*)this);
        if (pet->isControlled())
        {
            Unit* owner = GetOwner();
            if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && ((Player*)owner)->GetGroup())
                ((Player*)owner)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_MAX_HP);
        }
    }

    if (val < health)
        SetHealth(val);
}

void Unit::SetHealthPercent(float percent)
{
    uint32 newHealth = GetMaxHealth() * percent / 100.0f;
    SetHealth(newHealth);
}

void Unit::SetPower(Powers power, uint32 val)
{
    if (GetPower(power) == val)
        return;

    uint32 maxPower = GetMaxPower(power);
    if (maxPower < val)
        val = maxPower;

    SetStatInt32Value(UNIT_FIELD_POWER1 + power, val);

    // group update
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (((Player*)this)->GetGroup())
            ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_CUR_POWER);
    }
    else if (((Creature*)this)->IsPet())
    {
        Pet* pet = ((Pet*)this);
        if (pet->isControlled())
        {
            Unit* owner = GetOwner();
            if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && ((Player*)owner)->GetGroup())
                ((Player*)owner)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_CUR_POWER);
        }

        // Update the pet's character sheet with happiness damage bonus
        if (pet->getPetType() == HUNTER_PET && power == POWER_HAPPINESS)
            pet->UpdateDamagePhysical(BASE_ATTACK);
    }
}

void Unit::SetMaxPower(Powers power, uint32 val)
{
    uint32 cur_power = GetPower(power);
    SetStatInt32Value(UNIT_FIELD_MAXPOWER1 + power, val);

    // group update
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (((Player*)this)->GetGroup())
            ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_MAX_POWER);
    }
    else if (((Creature*)this)->IsPet())
    {
        Pet* pet = ((Pet*)this);
        if (pet->isControlled())
        {
            Unit* owner = GetOwner();
            if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && ((Player*)owner)->GetGroup())
                ((Player*)owner)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_MAX_POWER);
        }
    }

    if (val < cur_power)
        SetPower(power, val);
}

void Unit::ApplyPowerMod(Powers power, uint32 val, bool apply)
{
    ApplyModUInt32Value(UNIT_FIELD_POWER1 + power, val, apply);

    // group update
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (((Player*)this)->GetGroup())
            ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_CUR_POWER);
    }
    else if (((Creature*)this)->IsPet())
    {
        Pet* pet = ((Pet*)this);
        if (pet->isControlled())
        {
            Unit* owner = GetOwner();
            if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && ((Player*)owner)->GetGroup())
                ((Player*)owner)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_CUR_POWER);
        }
    }
}

void Unit::ApplyMaxPowerMod(Powers power, uint32 val, bool apply)
{
    ApplyModUInt32Value(UNIT_FIELD_MAXPOWER1 + power, val, apply);

    // group update
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (((Player*)this)->GetGroup())
            ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_MAX_POWER);
    }
    else if (((Creature*)this)->IsPet())
    {
        Pet* pet = ((Pet*)this);
        if (pet->isControlled())
        {
            Unit* owner = GetOwner();
            if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && ((Player*)owner)->GetGroup())
                ((Player*)owner)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_MAX_POWER);
        }
    }
}

void Unit::ApplyAuraProcTriggerDamage(Aura* aura, bool apply)
{
    AuraList& tAuraProcTriggerDamage = m_modAuras[SPELL_AURA_PROC_TRIGGER_DAMAGE];
    if (apply)
        tAuraProcTriggerDamage.push_back(aura);
    else
        tAuraProcTriggerDamage.remove(aura);
}

uint32 Unit::GetCreatePowers(Powers power) const
{
    switch (power)
    {
        case POWER_HEALTH:      return 0;                   // is it really should be here?
        case POWER_MANA:        return GetCreateMana();
        case POWER_RAGE:        return POWER_RAGE_DEFAULT;
        case POWER_FOCUS:       return (GetTypeId() == TYPEID_PLAYER || !((Creature const*)this)->IsPet() || ((Pet const*)this)->getPetType() != HUNTER_PET ? 0 : POWER_FOCUS_DEFAULT);
        case POWER_ENERGY:      return POWER_ENERGY_DEFAULT;
        case POWER_HAPPINESS:   return (GetTypeId() == TYPEID_PLAYER || !((Creature const*)this)->IsPet() || ((Pet const*)this)->getPetType() != HUNTER_PET ? 0 : POWER_HAPPINESS_DEFAULT);
    }

    return 0;
}

void Unit::AddToWorld()
{
    Object::AddToWorld();
    ScheduleAINotify(0);
}

void Unit::RemoveFromWorld()
{
    // cleanup
    if (IsInWorld())
    {
        Uncharm();
        RemoveNotOwnTrackedTargetAuras();
        RemoveGuardians();
        RemoveAllGameObjects();
        RemoveAllDynObjects();
        CleanupDeletedAuras();
        GetViewPoint().Event_RemovedFromWorld();
    }

    Object::RemoveFromWorld();
}

void Unit::CleanupsBeforeDelete()
{
    if (m_uint32Values)                                     // only for fully created object
    {
        InterruptNonMeleeSpells(true);
        m_Events.KillAllEvents(false);                      // non-delatable (currently casted spells) will not deleted now but it will deleted at call in Map::RemoveAllObjectsInRemoveList
        CombatStop();
        ClearComboPointHolders();
        DeleteThreatList();
        if (GetTypeId() == TYPEID_PLAYER)
            getHostileRefManager().setOnlineOfflineState(false);
        else
            getHostileRefManager().deleteReferences();
        RemoveAllAuras(AURA_REMOVE_BY_DELETE);
    }
    WorldObject::CleanupsBeforeDelete();
}

CharmInfo* Unit::InitCharmInfo(Unit* charm)
{
    delete m_charmInfo;
    m_charmInfo = new CharmInfo(charm);

    return m_charmInfo;
}

CharmInfo::CharmInfo(Unit* unit) :
    m_unit(unit), m_ai(nullptr), m_combatData(nullptr),
    m_CommandState(COMMAND_FOLLOW), m_reactState(REACT_PASSIVE),
    m_petnumber(0), m_retreating(false), m_stayPosSet(false),
    m_stayPosX(0), m_stayPosY(0), m_stayPosZ(0), m_stayPosO(0),
    m_opener(0), m_openerMinRange(0), m_openerMaxRange(0),
    m_unitFieldBytes2_1(0)
{
    for (int i = 0; i < CREATURE_MAX_SPELLS; ++i)
        m_charmspells[i].SetActionAndType(0, ACT_DISABLED);
}

CharmInfo::~CharmInfo()
{
    delete m_combatData;
    delete m_ai;
}

void CharmInfo::SetCharmState(std::string const & ainame /*= "PetAI"*/, bool withNewThreatList /*= true*/)
{
    if (!ainame.empty())
        m_ai = FactorySelector::GetSpecificAI(m_unit, ainame);
    else
        m_ai = FactorySelector::GetSpecificAI(m_unit, "PetAI");

    if (withNewThreatList)
        m_combatData = new CombatData(m_unit);

    m_unitFieldBytes2_1 = m_unit->GetByteValue(UNIT_FIELD_BYTES_2, 1);
}

void CharmInfo::ResetCharmState()
{
    m_unit->SetByteValue(UNIT_FIELD_BYTES_2, 1, m_unitFieldBytes2_1);
}

void CharmInfo::InitPetActionBar()
{
    // the first 3 SpellOrActions are attack, follow and stay
    for (uint32 i = 0; i < ACTION_BAR_INDEX_PET_SPELL_START - ACTION_BAR_INDEX_START; ++i)
        SetActionBar(ACTION_BAR_INDEX_START + i, COMMAND_ATTACK - i, ACT_COMMAND);

    // middle 4 SpellOrActions are spells/special attacks/abilities
    for (uint32 i = 0; i < ACTION_BAR_INDEX_PET_SPELL_END - ACTION_BAR_INDEX_PET_SPELL_START; ++i)
        SetActionBar(ACTION_BAR_INDEX_PET_SPELL_START + i, 0, ACT_DISABLED);

    // last 3 SpellOrActions are reactions
    for (uint32 i = 0; i < ACTION_BAR_INDEX_END - ACTION_BAR_INDEX_PET_SPELL_END; ++i)
        SetActionBar(ACTION_BAR_INDEX_PET_SPELL_END + i, COMMAND_ATTACK - i, ACT_REACTION);
}

void CharmInfo::InitEmptyActionBar()
{
    for (uint32 x = ACTION_BAR_INDEX_START + 1; x < ACTION_BAR_INDEX_END; ++x)
        SetActionBar(x, 0, ACT_PASSIVE);
}

void CharmInfo::InitPossessCreateSpells()
{
    InitEmptyActionBar();                                   // charm action bar

    SetActionBar(ACTION_BAR_INDEX_START, COMMAND_ATTACK, ACT_COMMAND);

    if (m_unit->GetTypeId() == TYPEID_PLAYER)               // possessed players don't have spells, keep the action bar empty
        return;

    for (uint32 x = 0; x < CREATURE_MAX_SPELLS; ++x)
    {
        if (IsPassiveSpell(((Creature*)m_unit)->m_spells[x]))
            m_unit->CastSpell(m_unit, ((Creature*)m_unit)->m_spells[x], TRIGGERED_OLD_TRIGGERED);
        else
            AddSpellToActionBar(((Creature*)m_unit)->m_spells[x], ACT_PASSIVE);
    }
}

void CharmInfo::InitCharmCreateSpells()
{
    InitPetActionBar();

    SetActionBar(ACTION_BAR_INDEX_START, COMMAND_ATTACK, ACT_COMMAND);

    if (m_unit->GetTypeId() == TYPEID_PLAYER)               // charmed players don't have spells
        return;

    for (uint32 x = 0; x < CREATURE_MAX_SPELLS; ++x)
    {
        uint32 spellId = ((Creature*)m_unit)->m_spells[x];

        if (!spellId)
        {
            m_charmspells[x].SetActionAndType(spellId, ACT_DISABLED);
            continue;
        }

        if (IsPassiveSpell(spellId))
        {
            m_unit->CastSpell(m_unit, spellId, TRIGGERED_OLD_TRIGGERED);
            m_charmspells[x].SetActionAndType(spellId, ACT_PASSIVE);
        }
        else
        {
            m_charmspells[x].SetActionAndType(spellId, ACT_DISABLED);

            ActiveStates newstate;
            bool onlyselfcast = true;
            SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

            for (uint32 i = 0; i < 3 && onlyselfcast; ++i)  // nonexistent spell will not make any problems as onlyselfcast would be false -> break right away
            {
                if (spellInfo->EffectImplicitTargetA[i] != TARGET_SELF && spellInfo->EffectImplicitTargetA[i] != 0)
                    onlyselfcast = false;
            }

            if (onlyselfcast || !IsPositiveSpell(spellId))  // only self cast and spells versus enemies are autocastable
                newstate = ACT_DISABLED;
            else
                newstate = ACT_PASSIVE;

            AddSpellToActionBar(spellId, newstate);
        }
    }
}

bool CharmInfo::AddSpellToActionBar(uint32 spellId, ActiveStates newstate)
{
    uint32 first_id = sSpellMgr.GetFirstSpellInChain(spellId);

    // new spell rank can be already listed
    for (uint8 i = 0; i < MAX_UNIT_ACTION_BAR_INDEX; ++i)
    {
        if (uint32 action = PetActionBar[i].GetAction())
        {
            if (PetActionBar[i].IsActionBarForSpell() && sSpellMgr.GetFirstSpellInChain(action) == first_id)
            {
                PetActionBar[i].SetAction(spellId);
                return true;
            }
        }
    }

    // or use empty slot in other case
    for (uint8 i = 0; i < MAX_UNIT_ACTION_BAR_INDEX; ++i)
    {
        if (!PetActionBar[i].GetAction() && PetActionBar[i].IsActionBarForSpell())
        {
            SetActionBar(i, spellId, newstate == ACT_DECIDE ? IsAutocastable(spellId) ? ACT_DISABLED : ACT_PASSIVE : newstate);
            return true;
        }
    }
    return false;
}

bool CharmInfo::RemoveSpellFromActionBar(uint32 spell_id)
{
    uint32 first_id = sSpellMgr.GetFirstSpellInChain(spell_id);

    for (uint8 i = 0; i < MAX_UNIT_ACTION_BAR_INDEX; ++i)
    {
        if (uint32 action = PetActionBar[i].GetAction())
        {
            if (PetActionBar[i].IsActionBarForSpell() && sSpellMgr.GetFirstSpellInChain(action) == first_id)
            {
                SetActionBar(i, 0, ACT_DISABLED);
                return true;
            }
        }
    }

    return false;
}

void CharmInfo::ToggleCreatureAutocast(uint32 spellid, bool apply)
{
    if (IsPassiveSpell(spellid))
        return;

    for (uint32 x = 0; x < CREATURE_MAX_SPELLS; ++x)
        if (spellid == m_charmspells[x].GetAction())
            m_charmspells[x].SetType(apply ? ACT_ENABLED : ACT_DISABLED);
}

void CharmInfo::SetPetNumber(uint32 petnumber, bool statwindow)
{
    m_petnumber = petnumber;
    if (statwindow)
        m_unit->SetUInt32Value(UNIT_FIELD_PETNUMBER, m_petnumber);
    else
        m_unit->SetUInt32Value(UNIT_FIELD_PETNUMBER, 0);
}

void CharmInfo::SetCommandState(CommandStates st)
{
    m_CommandState = st;

    switch (st)
    {
        case COMMAND_STAY:
            SetIsRetreating();
            SetStayPosition(true);
            SetSpellOpener();
            break;

        case COMMAND_FOLLOW:
            SetStayPosition();
            SetSpellOpener();
            break;

        default:
            break;
    }
}

void CharmInfo::LoadPetActionBar(const std::string& data)
{
    InitPetActionBar();

    Tokens tokens = StrSplit(data, " ");

    if (tokens.size() != (ACTION_BAR_INDEX_END - ACTION_BAR_INDEX_START) * 2)
        return;                                             // non critical, will reset to default

    int index;
    Tokens::iterator iter;
    for (iter = tokens.begin(), index = ACTION_BAR_INDEX_START; index < ACTION_BAR_INDEX_END; ++iter, ++index)
    {
        // use unsigned cast to avoid sign negative format use at long-> ActiveStates (int) conversion
        uint8 type  = (uint8)std::stoul((*iter).c_str());
        ++iter;
        uint32 action = std::stoul((*iter).c_str());

        PetActionBar[index].SetActionAndType(action, ActiveStates(type));

        // check correctness
        if (PetActionBar[index].IsActionBarForSpell())
        {
            SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(PetActionBar[index].GetAction());
            if (!spellInfo)
                SetActionBar(index, 0, ACT_DISABLED);
            else if (!IsAutocastable(spellInfo))
                SetActionBar(index, PetActionBar[index].GetAction(), ACT_PASSIVE);
        }
    }
}

void CharmInfo::BuildActionBar(WorldPacket& data) const
{
    for (uint32 i = 0; i < MAX_UNIT_ACTION_BAR_INDEX; ++i)
        data << uint32(PetActionBar[i].packedData);
}

void CharmInfo::SetSpellAutocast(uint32 spell_id, bool state)
{
    for (int i = 0; i < MAX_UNIT_ACTION_BAR_INDEX; ++i)
    {
        if (spell_id == PetActionBar[i].GetAction() && PetActionBar[i].IsActionBarForSpell())
        {
            PetActionBar[i].SetType(state ? ACT_ENABLED : ACT_DISABLED);
            break;
        }
    }
}

void CharmInfo::SetStayPosition(bool stay)
{
    if (stay)
    {
        m_stayPosX = m_unit->GetPositionX();
        m_stayPosY = m_unit->GetPositionY();
        m_stayPosZ = m_unit->GetPositionZ();
        m_stayPosO = m_unit->GetOrientation();
    }
    else
    {
        m_stayPosX = 0;
        m_stayPosY = 0;
        m_stayPosZ = 0;
        m_stayPosO = 0;
    }

    m_stayPosSet = stay;
}

bool Unit::isFrozen() const
{
    return HasAuraState(AURA_STATE_FROZEN);
}

struct ProcTriggeredData
{
    ProcTriggeredData(SpellProcEventEntry const* _spellProcEvent, SpellAuraHolder* _triggeredByHolder)
        : spellProcEvent(_spellProcEvent), triggeredByHolder(_triggeredByHolder)
    {}
    SpellProcEventEntry const* spellProcEvent;
    SpellAuraHolder* triggeredByHolder;
};

typedef std::list< ProcTriggeredData > ProcTriggeredList;
typedef std::list< uint32> RemoveSpellList;

uint32 createProcExtendMask(SpellNonMeleeDamage* damageInfo, SpellMissInfo missCondition)
{
    uint32 procEx = PROC_EX_NONE;
    // Check victim state
    if (missCondition != SPELL_MISS_NONE)
        switch (missCondition)
        {
            case SPELL_MISS_MISS:    procEx |= PROC_EX_MISS;   break;
            case SPELL_MISS_RESIST:  procEx |= PROC_EX_RESIST; break;
            case SPELL_MISS_DODGE:   procEx |= PROC_EX_DODGE;  break;
            case SPELL_MISS_PARRY:   procEx |= PROC_EX_PARRY;  break;
            case SPELL_MISS_BLOCK:   procEx |= PROC_EX_BLOCK;  break;
            case SPELL_MISS_EVADE:   procEx |= PROC_EX_EVADE;  break;
            case SPELL_MISS_IMMUNE:  procEx |= PROC_EX_IMMUNE; break;
            case SPELL_MISS_IMMUNE2: procEx |= PROC_EX_IMMUNE; break;
            case SPELL_MISS_DEFLECT: procEx |= PROC_EX_DEFLECT; break;
            case SPELL_MISS_ABSORB:  procEx |= PROC_EX_ABSORB; break;
            case SPELL_MISS_REFLECT: procEx |= PROC_EX_REFLECT; break;
            default:
                break;
        }
    else
    {
        // On block
        if (damageInfo->blocked)
            procEx |= PROC_EX_BLOCK;
        // On absorb
        if (damageInfo->absorb)
            procEx |= PROC_EX_ABSORB;
        // On crit
        if (damageInfo->HitInfo & SPELL_HIT_TYPE_CRIT)
            procEx |= PROC_EX_CRITICAL_HIT;
        else
            procEx |= PROC_EX_NORMAL_HIT;
    }
    return procEx;
}

void Unit::ProcDamageAndSpellFor(bool isVictim, Unit* pTarget, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, SpellEntry const* procSpell, uint32 damage)
{
    // For melee/ranged based attack need update skills and set some Aura states
    if (procFlag & MELEE_BASED_TRIGGER_MASK)
    {
        // Update skills here for players
        if (GetTypeId() == TYPEID_PLAYER)
        {
            // On melee based hit/miss/resist need update skill (for victim and attacker)
            if (procExtra & (PROC_EX_NORMAL_HIT | PROC_EX_CRITICAL_HIT | PROC_EX_MISS | PROC_EX_DODGE | PROC_EX_PARRY | PROC_EX_BLOCK))
            {
                if (pTarget->GetTypeId() != TYPEID_PLAYER && pTarget->GetCreatureType() != CREATURE_TYPE_CRITTER)
                    ((Player*)this)->UpdateCombatSkills(pTarget, attType, isVictim);
            }
            // Update defence if player is victim and parry/dodge/block
            if (isVictim && procExtra & (PROC_EX_DODGE | PROC_EX_PARRY | PROC_EX_BLOCK))
                ((Player*)this)->UpdateDefense();
        }
        // If exist crit/parry/dodge/block need update aura state (for victim and attacker)
        if (procExtra & (PROC_EX_CRITICAL_HIT | PROC_EX_PARRY | PROC_EX_DODGE | PROC_EX_BLOCK))
        {
            // for victim
            if (isVictim)
            {
                // if victim and dodge attack
                if (procExtra & PROC_EX_DODGE)
                {
                    // Update AURA_STATE on dodge
                    if (getClass() != CLASS_ROGUE) // skip Rogue Riposte
                    {
                        ModifyAuraState(AURA_STATE_DEFENSE, true);
                        StartReactiveTimer(REACTIVE_DEFENSE);
                    }
                }
                // if victim and parry attack
                if (procExtra & PROC_EX_PARRY)
                {
                    // For Hunters only Counterattack (skip Mongoose bite)
                    if (getClass() == CLASS_HUNTER)
                    {
                        ModifyAuraState(AURA_STATE_HUNTER_PARRY, true);
                        StartReactiveTimer(REACTIVE_HUNTER_PARRY);
                    }
                    else
                    {
                        ModifyAuraState(AURA_STATE_DEFENSE, true);
                        StartReactiveTimer(REACTIVE_DEFENSE);
                    }
                }
                // if and victim block attack
                if (procExtra & PROC_EX_BLOCK)
                {
                    ModifyAuraState(AURA_STATE_DEFENSE, true);
                    StartReactiveTimer(REACTIVE_DEFENSE);
                }
            }
            else // For attacker
            {
                // Overpower on victim dodge
                if (procExtra & PROC_EX_DODGE && GetTypeId() == TYPEID_PLAYER && getClass() == CLASS_WARRIOR)
                {
                    ((Player*)this)->AddComboPoints(pTarget, 1);
                    StartReactiveTimer(REACTIVE_OVERPOWER);
                }
            }
        }
    }

    RemoveSpellList removedSpells;
    ProcTriggeredList procTriggered;
    // Fill procTriggered list
    for (SpellAuraHolderMap::const_iterator itr = GetSpellAuraHolderMap().begin(); itr != GetSpellAuraHolderMap().end(); ++itr)
    {
        // skip deleted auras (possible at recursive triggered call
        if (itr->second->GetState() != SPELLAURAHOLDER_STATE_READY || itr->second->IsDeleted())
            continue;

        SpellProcEventEntry const* spellProcEvent = nullptr;
        if (!IsTriggeredAtSpellProcEvent(pTarget, itr->second, procSpell, procFlag, procExtra, attType, isVictim, spellProcEvent))
            continue;

        itr->second->SetInUse(true);                        // prevent holder deletion
        procTriggered.push_back(ProcTriggeredData(spellProcEvent, itr->second));
    }

    // Nothing found
    if (procTriggered.empty())
        return;

    // Handle effects proceed this time
    for (ProcTriggeredList::const_iterator itr = procTriggered.begin(); itr != procTriggered.end(); ++itr)
    {
        // Some auras can be deleted in function called in this loop (except first, ofc)
        SpellAuraHolder* triggeredByHolder = itr->triggeredByHolder;
        if (triggeredByHolder->IsDeleted())
            continue;

        SpellProcEventEntry const* spellProcEvent = itr->spellProcEvent;
        bool useCharges = triggeredByHolder->GetAuraCharges() > 0;
        bool procSuccess = true;
        bool anyAuraProc = false;

        // For players set spell cooldown if need
        uint32 cooldown = 0;
        if (GetTypeId() == TYPEID_PLAYER && spellProcEvent && spellProcEvent->cooldown)
            cooldown = spellProcEvent->cooldown;

        for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            Aura* triggeredByAura = triggeredByHolder->GetAuraByEffectIndex(SpellEffectIndex(i));
            if (!triggeredByAura)
                continue;

            Modifier* auraModifier = triggeredByAura->GetModifier();

            if (procSpell)
            {
                if (spellProcEvent)
                {
                    if (spellProcEvent->spellFamilyMask[i])
                    {
                        if (!procSpell->IsFitToFamilyMask(spellProcEvent->spellFamilyMask[i]))
                            continue;
                    }
                    // don't check dbc FamilyFlags if schoolMask exists
                    else if (!triggeredByAura->CanProcFrom(procSpell, spellProcEvent->procEx, procExtra, damage != 0, !spellProcEvent->schoolMask))
                        continue;
                }
                else if (!triggeredByAura->CanProcFrom(procSpell, PROC_EX_NONE, procExtra, damage != 0, true))
                    continue;
            }

            SpellAuraProcResult procResult = (*this.*AuraProcHandler[auraModifier->m_auraname])(pTarget, damage, triggeredByAura, procSpell, procFlag, procExtra, cooldown);
            switch (procResult)
            {
                case SPELL_AURA_PROC_CANT_TRIGGER:
                    continue;
                case SPELL_AURA_PROC_FAILED:
                    procSuccess = false;
                    break;
                case SPELL_AURA_PROC_OK:
                    break;
            }

            anyAuraProc = true;
        }

        // Remove charge (aura can be removed by triggers)
        if (useCharges && procSuccess && anyAuraProc && !triggeredByHolder->IsDeleted())
        {
            // If last charge dropped add spell to remove list
            if (triggeredByHolder->DropAuraCharge())
                removedSpells.push_back(triggeredByHolder->GetId());
        }

        triggeredByHolder->SetInUse(false);
    }

    if (!removedSpells.empty())
    {
        // Sort spells and remove duplicates
        removedSpells.sort();
        removedSpells.unique();
        // Remove auras from removedAuras
        for (RemoveSpellList::const_iterator i = removedSpells.begin(); i != removedSpells.end(); ++i)
            RemoveAurasDueToSpell(*i);
    }
}

SpellSchoolMask Unit::GetMeleeDamageSchoolMask() const
{
    return SPELL_SCHOOL_MASK_NORMAL;
}

Player* Unit::GetSpellModOwner() const
{
    if (GetTypeId() == TYPEID_PLAYER)
        return (Player*)this;
    if (((Creature*)this)->IsPet() || ((Creature*)this)->IsTotem())
    {
        Unit* owner = GetOwner();
        if (owner && owner->GetTypeId() == TYPEID_PLAYER)
            return (Player*)owner;
    }
    return nullptr;
}

///----------Pet responses methods-----------------
void Unit::SendPetCastFail(uint32 spellid, SpellCastResult msg) const
{
    if (msg == SPELL_CAST_OK)
        return;

    Unit* owner = GetCharmerOrOwner();
    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data(SMSG_PET_CAST_FAILED, 4 + 1);
    data << uint32(spellid);
    data << uint8(msg);
    ((Player*)owner)->GetSession()->SendPacket(data);
}

void Unit::SendPetActionFeedback(uint8 msg) const
{
    Unit* owner = GetOwner();
    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data(SMSG_PET_ACTION_FEEDBACK, 1);
    data << uint8(msg);
    ((Player*)owner)->GetSession()->SendPacket(data);
}

void Unit::SendPetTalk(uint32 pettalk) const
{
    Unit* owner = GetOwner();
    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data(SMSG_PET_ACTION_SOUND, 8 + 4);
    data << GetObjectGuid();
    data << uint32(pettalk);
    ((Player*)owner)->GetSession()->SendPacket(data);
}

void Unit::SendPetAIReaction() const
{
    Unit* owner = GetOwner();
    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data(SMSG_AI_REACTION, 8 + 4);
    data << GetObjectGuid();
    data << uint32(AI_REACTION_HOSTILE);
    ((Player*)owner)->GetSession()->SendPacket(data);
}

///----------End of Pet responses methods----------

void Unit::StopMoving(bool forceSendStop /*=false*/)
{
    if (IsStopped() && !forceSendStop)
        return;

    clearUnitState(UNIT_STAT_MOVING);

    // not need send any packets if not in world
    if (!IsInWorld())
        return;

    Movement::MoveSplineInit init(*this);
    init.Stop(forceSendStop);
}

void Unit::InterruptMoving(bool forceSendStop /*=false*/)
{
    bool isMoving = false;

    if (!movespline->Finalized())
    {
        Movement::Location loc = movespline->ComputePosition();
        movespline->_Interrupt();
        Relocate(loc.x, loc.y, loc.z, loc.orientation);
        isMoving = true;
    }

    StopMoving(forceSendStop || isMoving);
}

void Unit::SetImmobilizedState(bool apply, bool stun)
{
    const uint32 immobilized = (UNIT_STAT_ROOT | UNIT_STAT_STUNNED);
    const uint32 state = stun ? UNIT_STAT_STUNNED : UNIT_STAT_ROOT;
    const Unit* charmer = GetCharmer();
    const bool player = ((charmer ? charmer : this)->GetTypeId() == TYPEID_PLAYER);
    if (apply)
    {
        addUnitState(state);
        if (!player)
            StopMoving();
        else
        {
            // Clear unit movement flags
            m_movementInfo.SetMovementFlags(MOVEFLAG_NONE);
            if (stun)
                SetStandState(UNIT_STAND_STATE_STAND); // in 1.5 client
            SetRoot(true);
        }
    }
    else
    {
        clearUnitState(state);
        // Prevent giving ability to move if more immobilizers are active
        if (!hasUnitState(immobilized) && (player || m_movementInfo.HasMovementFlag(MOVEFLAG_ROOT)))
            SetRoot(false);
    }
}

void Unit::SetFeared(bool apply, ObjectGuid casterGuid, uint32 spellID, uint32 time)
{
    SetIncapacitatedState(apply, UNIT_FLAG_FLEEING, casterGuid, spellID, time);
}

void Unit::SetConfused(bool apply, ObjectGuid casterGuid, uint32 spellID)
{
    SetIncapacitatedState(apply, UNIT_FLAG_CONFUSED, casterGuid, spellID);
}

void Unit::SetStunned(bool apply, ObjectGuid casterGuid, uint32 spellID)
{
    SetIncapacitatedState(apply, UNIT_FLAG_STUNNED, casterGuid, spellID);
}

void Unit::SetIncapacitatedState(bool apply, uint32 state, ObjectGuid casterGuid, uint32 spellID, uint32 time)
{
    // We are interested only in a particular subset of flags:
    const uint32 filter = (UNIT_FLAG_STUNNED | UNIT_FLAG_CONFUSED | UNIT_FLAG_FLEEING);
    if (!state || !(state & filter) || (state & ~filter))
        return;

    Player* controller = GetCharmerOrOwnerPlayerOrPlayerItself();
    const bool control = controller ? controller->IsClientControl(this) : false;
    const bool movement = (state != UNIT_FLAG_STUNNED);
    const bool stun = !!(state & UNIT_FLAG_STUNNED);
    const bool fleeing = !!(state & UNIT_FLAG_FLEEING);

    if (apply)
    {
        if (fleeing && HasAuraType(SPELL_AURA_PREVENTS_FLEEING))
        {
            if (state == UNIT_FLAG_FLEEING)
                return;
            else
                state &= ~UNIT_FLAG_FLEEING;
        }
        SetFlag(UNIT_FIELD_FLAGS, state);
    }
    else
        RemoveFlag(UNIT_FIELD_FLAGS, state);

    if (movement)
        GetMotionMaster()->MovementExpired(false);
    if (apply)
        CastStop(GetObjectGuid() == casterGuid ? spellID : 0);

    if (GetTypeId() == TYPEID_UNIT)
    {
        if (HasFlag(UNIT_FIELD_FLAGS, filter))
        {
            if (!GetTargetGuid().IsEmpty()) // Incapacitated creature loses its target
                SetTargetGuid(ObjectGuid());
        }
        else if (isAlive())
        {
            if (Unit* victim = getVictim())
            {
                SetTargetGuid(victim->GetObjectGuid());  // Restore target
                if (movement)
                    GetMotionMaster()->MoveChase(victim); // Restore movement generator
            }
            else if (movement)
                GetMotionMaster()->Initialize(); // Reset movement generator

            if (!apply && fleeing)
            {
                // Attack the caster if can on fear expiration
                if (Unit* caster = IsInWorld() ? GetMap()->GetUnit(casterGuid) : nullptr)
                    ((Creature*)this)->AttackedBy(caster);
            }
        }
    }

    // Update stun if required:
    if (stun)
        SetImmobilizedState(apply, true);

    if (!movement)
        return;

    // Check if we should return or remove player control after change
    if (controller)
    {
        const bool remove = !controller->IsClientControl(this);
        if (control && remove)
            controller->SetClientControl(this, 0);
        else if (!control && !remove)
            controller->SetClientControl(this, 1);
    }

    // Update incapacitated movement if required:
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED))
    {
        StopMoving(true);
        GetMotionMaster()->MoveConfused();
    }
    else if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING))
    {
        StopMoving(true);
        GetMotionMaster()->MoveFleeing(IsInWorld() ?  GetMap()->GetUnit(casterGuid) : nullptr, time);
    }
}

void Unit::SetFeignDeath(bool apply, ObjectGuid casterGuid /*= ObjectGuid()*/)
{
    if (apply)
    {
        /*
        WorldPacket data(SMSG_FEIGN_DEATH_RESISTED, 9);
        data<<GetGUID();
        data<<uint8(0);
        SendMessageToSet(data,true);
        */

        if (GetTypeId() != TYPEID_PLAYER)
            StopMoving();
        else
            ((Player*)this)->m_movementInfo.SetMovementFlags(MOVEFLAG_NONE);


        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_29);
        // blizz like 2.0.x
        // SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);  [-ZERO] remove/replace ?

        SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);

        addUnitState(UNIT_STAT_DIED);
        CombatStop();
        RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);

        // prevent interrupt message
        if (casterGuid == GetObjectGuid())
            FinishSpell(CURRENT_GENERIC_SPELL, false);
        InterruptNonMeleeSpells(true);
        getHostileRefManager().deleteReferences();
    }
    else
    {
        /*
        WorldPacket data(SMSG_FEIGN_DEATH_RESISTED, 9);
        data<<GetGUID();
        data<<uint8(1);
        SendMessageToSet(data,true);
        */

        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_29);
        // blizz like 2.0.x
        // SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH); [-ZERO] remove/replace ?

        RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);

        clearUnitState(UNIT_STAT_DIED);

        if (GetTypeId() != TYPEID_PLAYER && isAlive())
        {
            // restore appropriate movement generator
            if (getVictim())
                GetMotionMaster()->MoveChase(getVictim());
            else
                GetMotionMaster()->Initialize();
        }
    }
}

bool Unit::IsSitState() const
{
    uint8 s = getStandState();
    return
        s == UNIT_STAND_STATE_SIT_CHAIR        || s == UNIT_STAND_STATE_SIT_LOW_CHAIR  ||
        s == UNIT_STAND_STATE_SIT_MEDIUM_CHAIR || s == UNIT_STAND_STATE_SIT_HIGH_CHAIR ||
        s == UNIT_STAND_STATE_SIT;
}

bool Unit::IsStandState() const
{
    uint8 s = getStandState();
    return !IsSitState() && s != UNIT_STAND_STATE_SLEEP && s != UNIT_STAND_STATE_KNEEL;
}

void Unit::SetStandState(uint8 state)
{
    SetByteValue(UNIT_FIELD_BYTES_1, 0, state);

    if (IsStandState())
        RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_NOT_SEATED);

    if (GetTypeId() == TYPEID_PLAYER)
    {
        WorldPacket data(SMSG_STANDSTATE_UPDATE, 1);
        data << (uint8)state;
        ((Player*)this)->GetSession()->SendPacket(data);
    }
}

bool Unit::IsPolymorphed() const
{
    return IsSpellMagePolymorph(getTransForm());
}

void Unit::SetDisplayId(uint32 modelId)
{
    SetUInt32Value(UNIT_FIELD_DISPLAYID, modelId);

    UpdateModelData();

    if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->IsPet())
    {
        Pet* pet = ((Pet*)this);
        if (!pet->isControlled())
            return;
        Unit* owner = GetOwner();
        if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && ((Player*)owner)->GetGroup())
            ((Player*)owner)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_MODEL_ID);
    }
}

void Unit::UpdateModelData()
{
    if (CreatureModelInfo const* modelInfo = sObjectMgr.GetCreatureModelInfo(GetDisplayId()))
    {
        // we expect values in database to be relative to scale = 1.0
        SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, GetObjectScale() * modelInfo->bounding_radius);

        // never actually update combat_reach for player, it's always the same. Below player case is for initialization
        if (GetTypeId() == TYPEID_PLAYER)
            SetFloatValue(UNIT_FIELD_COMBATREACH, 1.5f);
        else
            SetFloatValue(UNIT_FIELD_COMBATREACH, GetObjectScale() * modelInfo->combat_reach);
    }
}

float Unit::GetObjectScaleMod() const
{
    int32 modValue = 0;
    Unit::AuraList const& scaleAuraList = GetAurasByType(SPELL_AURA_MOD_SCALE);
    for (Unit::AuraList::const_iterator itr = scaleAuraList.begin(); itr != scaleAuraList.end(); ++itr)
        modValue += (*itr)->GetModifier()->m_amount;

    float result = (100 + modValue) / 100.0f;

    // TODO:: not sure we have to do this sanity check, less than /100 or more than *100 seem not reasonable
    if (result < 0.01f)
        result = 0.01f;
    else if (result > 100.0f)
        result = 100.0f;

    return result;
}

void Unit::ClearComboPointHolders()
{
    while (!m_ComboPointHolders.empty())
    {
        uint32 lowguid = *m_ComboPointHolders.begin();

        Player* plr = sObjectMgr.GetPlayer(ObjectGuid(HIGHGUID_PLAYER, lowguid));
        if (plr && plr->GetComboTargetGuid() == GetObjectGuid())// recheck for safe
            plr->ClearComboPoints();                        // remove also guid from m_ComboPointHolders;
        else
            m_ComboPointHolders.erase(lowguid);             // or remove manually
    }
}

void Unit::ClearAllReactives()
{
    for (int i = 0; i < MAX_REACTIVE; ++i)
        m_reactiveTimer[i] = 0;

    if (HasAuraState(AURA_STATE_DEFENSE))
        ModifyAuraState(AURA_STATE_DEFENSE, false);
    if (getClass() == CLASS_HUNTER && HasAuraState(AURA_STATE_HUNTER_PARRY))
        ModifyAuraState(AURA_STATE_HUNTER_PARRY, false);

    if (getClass() == CLASS_WARRIOR && GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->ClearComboPoints();
}

void Unit::UpdateReactives(uint32 p_time)
{
    for (int i = 0; i < MAX_REACTIVE; ++i)
    {
        ReactiveType reactive = ReactiveType(i);

        if (!m_reactiveTimer[reactive])
            continue;

        if (m_reactiveTimer[reactive] <= p_time)
        {
            m_reactiveTimer[reactive] = 0;

            switch (reactive)
            {
                case REACTIVE_DEFENSE:
                    if (HasAuraState(AURA_STATE_DEFENSE))
                        ModifyAuraState(AURA_STATE_DEFENSE, false);
                    break;
                case REACTIVE_HUNTER_PARRY:
                    if (getClass() == CLASS_HUNTER && HasAuraState(AURA_STATE_HUNTER_PARRY))
                        ModifyAuraState(AURA_STATE_HUNTER_PARRY, false);
                    break;
                case REACTIVE_OVERPOWER:
                    if (getClass() == CLASS_WARRIOR && GetTypeId() == TYPEID_PLAYER)
                        ((Player*)this)->ClearComboPoints();
                    break;
                default:
                    break;
            }
        }
        else
        {
            m_reactiveTimer[reactive] -= p_time;
        }
    }
}

Unit* Unit::SelectRandomUnfriendlyTarget(Unit* except /*= nullptr*/, float radius /*= ATTACK_DISTANCE*/) const
{
    std::list<Unit*> targets;

    MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck u_check(this, radius);
    MaNGOS::UnitListSearcher<MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck> searcher(targets, u_check);
    Cell::VisitAllObjects(this, searcher, radius);

    // remove current target
    if (except)
        targets.remove(except);

    // remove not LoS targets
    for (std::list<Unit*>::iterator tIter = targets.begin(); tIter != targets.end();)
    {
        if (!IsWithinLOSInMap(*tIter))
        {
            std::list<Unit*>::iterator tIter2 = tIter;
            ++tIter;
            targets.erase(tIter2);
        }
        else
            ++tIter;
    }

    // no appropriate targets
    if (targets.empty())
        return nullptr;

    // select random
    uint32 rIdx = urand(0, targets.size() - 1);
    std::list<Unit*>::const_iterator tcIter = targets.begin();
    for (uint32 i = 0; i < rIdx; ++i)
        ++tcIter;

    return *tcIter;
}

Unit* Unit::SelectRandomFriendlyTarget(Unit* except /*= nullptr*/, float radius /*= ATTACK_DISTANCE*/) const
{
    std::list<Unit*> targets;

    MaNGOS::AnyFriendlyUnitInObjectRangeCheck u_check(this, radius);
    MaNGOS::UnitListSearcher<MaNGOS::AnyFriendlyUnitInObjectRangeCheck> searcher(targets, u_check);

    Cell::VisitAllObjects(this, searcher, radius);

    // remove current target
    if (except)
        targets.remove(except);

    // remove not LoS targets
    for (std::list<Unit*>::iterator tIter = targets.begin(); tIter != targets.end();)
    {
        if (!IsWithinLOSInMap(*tIter))
        {
            std::list<Unit*>::iterator tIter2 = tIter;
            ++tIter;
            targets.erase(tIter2);
        }
        else
            ++tIter;
    }

    // no appropriate targets
    if (targets.empty())
        return nullptr;

    // select random
    uint32 rIdx = urand(0, targets.size() - 1);
    std::list<Unit*>::const_iterator tcIter = targets.begin();
    for (uint32 i = 0; i < rIdx; ++i)
        ++tcIter;

    return *tcIter;
}

bool Unit::hasNegativeAuraWithInterruptFlag(uint32 flag) const
{
    for (SpellAuraHolderMap::const_iterator iter = m_spellAuraHolders.begin(); iter != m_spellAuraHolders.end(); ++iter)
    {
        if (!iter->second->IsPositive() && iter->second->GetSpellProto()->AuraInterruptFlags & flag)
            return true;
    }
    return false;
}

void Unit::ApplyAttackTimePercentMod(WeaponAttackType att, float val, bool apply)
{
    if (val > 0)
    {
        ApplyPercentModFloatVar(m_modAttackSpeedPct[att], val, !apply);
        ApplyPercentModFloatValue(UNIT_FIELD_BASEATTACKTIME + att, val, !apply);
    }
    else
    {
        ApplyPercentModFloatVar(m_modAttackSpeedPct[att], -val, apply);
        ApplyPercentModFloatValue(UNIT_FIELD_BASEATTACKTIME + att, -val, apply);
    }
}

void Unit::ApplyCastTimePercentMod(float val, bool apply)
{
    if (val > 0)
        ApplyPercentModFloatValue(UNIT_MOD_CAST_SPEED, val, !apply);
    else
        ApplyPercentModFloatValue(UNIT_MOD_CAST_SPEED, -val, apply);
}

void Unit::UpdateAuraForGroup(uint8 slot)
{
    if (GetTypeId() == TYPEID_PLAYER)
    {
        Player* player = (Player*)this;
        if (player->GetGroup())
        {
            player->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_AURAS);
            player->SetAuraUpdateMask(slot);
        }
    }
    else if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->IsPet())
    {
        Pet* pet = ((Pet*)this);
        if (pet->isControlled())
        {
            Unit* owner = GetOwner();
            if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && ((Player*)owner)->GetGroup())
            {
                ((Player*)owner)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_AURAS);
                pet->SetAuraUpdateMask(slot);
            }
        }
    }
}

float Unit::GetAPMultiplier(WeaponAttackType attType, bool normalized)
{
    if (!normalized || GetTypeId() != TYPEID_PLAYER)
        return float(GetAttackTime(attType)) / 1000.0f;

    Item* Weapon = ((Player*)this)->GetWeaponForAttack(attType, true, false);
    if (!Weapon)
        return 2.4f;                                        // fist attack

    switch (Weapon->GetProto()->InventoryType)
    {
        case INVTYPE_2HWEAPON:
            return 3.3f;
        case INVTYPE_RANGED:
        case INVTYPE_RANGEDRIGHT:
        case INVTYPE_THROWN:
            return 2.8f;
        case INVTYPE_WEAPON:
        case INVTYPE_WEAPONMAINHAND:
        case INVTYPE_WEAPONOFFHAND:
        default:
            return Weapon->GetProto()->SubClass == ITEM_SUBCLASS_WEAPON_DAGGER ? 1.7f : 2.4f;
    }
}

Aura* Unit::GetDummyAura(uint32 spell_id) const
{
    Unit::AuraList const& mDummy = GetAurasByType(SPELL_AURA_DUMMY);
    for (Unit::AuraList::const_iterator itr = mDummy.begin(); itr != mDummy.end(); ++itr)
        if ((*itr)->GetId() == spell_id)
            return *itr;

    return nullptr;
}

void Unit::SetContestedPvP(Player* attackedPlayer)
{
    Player* player = GetCharmerOrOwnerPlayerOrPlayerItself();

    if (!player || (attackedPlayer && (attackedPlayer == player || player->IsInDuelWith(attackedPlayer))))
        return;

    player->SetContestedPvPTimer(30000);

    if (!player->hasUnitState(UNIT_STAT_ATTACK_PLAYER))
    {
        player->addUnitState(UNIT_STAT_ATTACK_PLAYER);
        player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP);
        // call MoveInLineOfSight for nearby contested guards
        UpdateVisibilityAndView();
    }

    if (!hasUnitState(UNIT_STAT_ATTACK_PLAYER))
    {
        addUnitState(UNIT_STAT_ATTACK_PLAYER);
        // call MoveInLineOfSight for nearby contested guards
        UpdateVisibilityAndView();
    }
}

void Unit::AddPetAura(PetAura const* petSpell)
{
    m_petAuras.insert(petSpell);
    if (Pet* pet = GetPet())
        pet->CastPetAura(petSpell);
}

void Unit::RemovePetAura(PetAura const* petSpell)
{
    m_petAuras.erase(petSpell);
    if (Pet* pet = GetPet())
        pet->RemoveAurasDueToSpell(petSpell->GetAura(pet->GetEntry()));
}

void Unit::RemoveAurasAtMechanicImmunity(uint32 mechMask, uint32 exceptSpellId, bool non_positive /*= false*/)
{
    Unit::SpellAuraHolderMap& auras = GetSpellAuraHolderMap();
    for (Unit::SpellAuraHolderMap::iterator iter = auras.begin(); iter != auras.end();)
    {
        SpellEntry const* spell = iter->second->GetSpellProto();
        if (spell->Id == exceptSpellId)
            ++iter;
        else if (non_positive && iter->second->IsPositive())
            ++iter;
        else if (spell->HasAttribute(SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY))
            ++iter;
        else if (iter->second->HasMechanicMask(mechMask))
        {
            RemoveAurasDueToSpell(spell->Id);

            if (auras.empty())
                break;
            else
                iter = auras.begin();
        }
        else
            ++iter;
    }
}

void Unit::NearTeleportTo(float x, float y, float z, float orientation, bool casting /*= false*/)
{
    DisableSpline();

    if (GetTypeId() == TYPEID_PLAYER)
        ((Player*)this)->TeleportTo(GetMapId(), x, y, z, orientation, TELE_TO_NOT_LEAVE_TRANSPORT | TELE_TO_NOT_LEAVE_COMBAT | TELE_TO_NOT_UNSUMMON_PET | (casting ? TELE_TO_SPELL : 0));
    else
    {
        Creature* c = (Creature*)this;
        // Creature relocation acts like instant movement generator, so current generator expects interrupt/reset calls to react properly
        if (!c->GetMotionMaster()->empty())
            if (MovementGenerator* movgen = c->GetMotionMaster()->top())
                movgen->Interrupt(*c);

        GetMap()->CreatureRelocation((Creature*)this, x, y, z, orientation);

        SendHeartBeat();

        // finished relocation, movegen can different from top before creature relocation,
        // but apply Reset expected to be safe in any case
        if (!c->GetMotionMaster()->empty())
            if (MovementGenerator* movgen = c->GetMotionMaster()->top())
                movgen->Reset(*c);
    }
}

void Unit::MonsterMoveWithSpeed(float x, float y, float z, float speed, bool generatePath, bool forceDestination)
{
    Movement::MoveSplineInit init(*this);
    init.MoveTo(x, y, z, generatePath, forceDestination);
    init.SetVelocity(speed);
    init.Launch();
}

struct SetPvPHelper
{
    explicit SetPvPHelper(bool _state) : state(_state) {}
    void operator()(Unit* unit) const { unit->SetPvP(state); }
    bool state;
};

void Unit::SetPvP(bool state)
{
    if (state)
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP);
    else
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP);

    if (GetTypeId() == TYPEID_PLAYER && ((Player*)this)->GetGroup())
        ((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_STATUS);

    CallForAllControlledUnits(SetPvPHelper(state), CONTROLLED_PET | CONTROLLED_TOTEMS | CONTROLLED_GUARDIANS | CONTROLLED_CHARM);
}

struct StopAttackFactionHelper
{
    explicit StopAttackFactionHelper(uint32 _faction_id) : faction_id(_faction_id) {}
    void operator()(Unit* unit) const { unit->StopAttackFaction(faction_id); }
    uint32 faction_id;
};

void Unit::StopAttackFaction(uint32 faction_id)
{
    if (Unit* victim = getVictim())
    {
        if (victim->getFactionTemplateEntry()->faction == faction_id)
        {
            AttackStop();
            if (IsNonMeleeSpellCasted(false))
                InterruptNonMeleeSpells(false);

            // melee and ranged forced attack cancel
            if (GetTypeId() == TYPEID_PLAYER)
                ((Player*)this)->SendAttackSwingCancelAttack();
        }
    }

    AttackerSet const& attackers = getAttackers();
    for (AttackerSet::const_iterator itr = attackers.begin(); itr != attackers.end();)
    {
        if ((*itr)->getFactionTemplateEntry()->faction == faction_id)
        {
            (*itr)->AttackStop();
            itr = attackers.begin();
        }
        else
            ++itr;
    }

    getHostileRefManager().deleteReferencesForFaction(faction_id);

    CallForAllControlledUnits(StopAttackFactionHelper(faction_id), CONTROLLED_PET | CONTROLLED_GUARDIANS | CONTROLLED_CHARM);
}

void Unit::CleanupDeletedAuras()
{
    for (SpellAuraHolderList::const_iterator iter = m_deletedHolders.begin(); iter != m_deletedHolders.end(); ++iter)
        delete *iter;
    m_deletedHolders.clear();

    // really delete auras "deleted" while processing its ApplyModify code
    for (AuraList::const_iterator itr = m_deletedAuras.begin(); itr != m_deletedAuras.end(); ++itr)
        delete *itr;
    m_deletedAuras.clear();
}

bool Unit::CheckAndIncreaseCastCounter()
{
    uint32 maxCasts = sWorld.getConfig(CONFIG_UINT32_MAX_SPELL_CASTS_IN_CHAIN);

    if (maxCasts && m_castCounter >= maxCasts)
        return false;

    ++m_castCounter;
    return true;
}

SpellAuraHolder* Unit::GetSpellAuraHolder(uint32 spellid) const
{
    SpellAuraHolderMap::const_iterator itr = m_spellAuraHolders.find(spellid);
    return itr != m_spellAuraHolders.end() ? itr->second : nullptr;
}

SpellAuraHolder* Unit::GetSpellAuraHolder(uint32 spellid, ObjectGuid casterGuid) const
{
    SpellAuraHolderConstBounds bounds = GetSpellAuraHolderBounds(spellid);
    for (SpellAuraHolderMap::const_iterator iter = bounds.first; iter != bounds.second; ++iter)
        if (iter->second->GetCasterGuid() == casterGuid)
            return iter->second;

    return nullptr;
}

class RelocationNotifyEvent : public BasicEvent
{
    public:
        RelocationNotifyEvent(Unit& owner) : BasicEvent(), m_owner(owner)
        {
            m_owner._SetAINotifyScheduled(true);
        }

        bool Execute(uint64 /*e_time*/, uint32 /*p_time*/)
        {
            float radius = MAX_CREATURE_ATTACK_RADIUS * sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_AGGRO);
            if (m_owner.GetTypeId() == TYPEID_PLAYER)
            {
                MaNGOS::PlayerRelocationNotifier notify((Player&)m_owner);
                Cell::VisitAllObjects(&m_owner, notify, radius);
            }
            else // if(m_owner.GetTypeId() == TYPEID_UNIT)
            {
                MaNGOS::CreatureRelocationNotifier notify((Creature&)m_owner);
                Cell::VisitAllObjects(&m_owner, notify, radius);
            }
            m_owner._SetAINotifyScheduled(false);
            return true;
        }

        void Abort(uint64)
        {
            m_owner._SetAINotifyScheduled(false);
        }

    private:
        Unit& m_owner;
};

void Unit::ScheduleAINotify(uint32 delay)
{
    if (!IsAINotifyScheduled())
        m_Events.AddEvent(new RelocationNotifyEvent(*this), m_Events.CalculateTime(delay));
}

void Unit::OnRelocated()
{
    // switch to use G3D::Vector3 is good idea, maybe
    float dx = m_last_notified_position.x - GetPositionX();
    float dy = m_last_notified_position.y - GetPositionY();
    float dz = m_last_notified_position.z - GetPositionZ();
    float distsq = dx * dx + dy * dy + dz * dz;
    if (distsq > World::GetRelocationLowerLimitSq())
    {
        m_last_notified_position.x = GetPositionX();
        m_last_notified_position.y = GetPositionY();
        m_last_notified_position.z = GetPositionZ();

        GetViewPoint().Call_UpdateVisibilityForOwner();
        UpdateObjectVisibility();
    }
    ScheduleAINotify(World::GetRelocationAINotifyDelay());
}

void Unit::UpdateSplineMovement(uint32 t_diff)
{
    enum
    {
        POSITION_UPDATE_DELAY = 400,
    };

    if (movespline->Finalized())
        return;

    movespline->updateState(t_diff);
    bool arrived = movespline->Finalized();

    if (arrived)
        DisableSpline();

    m_movesplineTimer.Update(t_diff);
    if (m_movesplineTimer.Passed() || arrived)
    {
        m_movesplineTimer.Reset(POSITION_UPDATE_DELAY);
        Movement::Location loc = movespline->ComputePosition();

        if (GetTypeId() == TYPEID_PLAYER)
            ((Player*)this)->SetPosition(loc.x, loc.y, loc.z, loc.orientation);
        else
            GetMap()->CreatureRelocation((Creature*)this, loc.x, loc.y, loc.z, loc.orientation);
    }
}

void Unit::DisableSpline()
{
    m_movementInfo.RemoveMovementFlag(MovementFlags(MOVEFLAG_SPLINE_ENABLED | MOVEFLAG_FORWARD));
    movespline->_Interrupt();
}

void Unit::ForceHealthAndPowerUpdate()
{
    uint32 powerType = GetPowerType();
    ForceValuesUpdateAtIndex(UNIT_FIELD_HEALTH);
    ForceValuesUpdateAtIndex(UNIT_FIELD_MAXHEALTH);
    ForceValuesUpdateAtIndex(UNIT_FIELD_POWER1 + powerType);
    ForceValuesUpdateAtIndex(UNIT_FIELD_MAXPOWER1 + powerType);
}

// This will create a new creature and set the current unit as the controller of that new creature
Unit* Unit::TakePossessOf(SpellEntry const* spellEntry, uint32 effIdx, float x, float y, float z, float ang)
{
    int32 const& creatureEntry = spellEntry->EffectMiscValue[effIdx];
    CreatureInfo const* cinfo = ObjectMgr::GetCreatureTemplate(creatureEntry);
    if (!cinfo)
    {
        sLog.outErrorDb("WorldObject::SummonCreature: Creature (Entry: %u) not existed for summoner: %s. ", creatureEntry, GetGuidStr().c_str());
        return nullptr;
    }

    if (GetCharm())
    {
        sLog.outError("Unit::TakePossessOf> There is already a charmed creature for %s its : %s. ", GetGuidStr().c_str(), GetCharm()->GetGuidStr().c_str());
        return nullptr;
    }

    TemporarySummon* pCreature = new TemporarySummon(GetObjectGuid());

    CreatureCreatePos pos(GetMap(), x, y, z, ang);

    if (x == 0.0f && y == 0.0f && z == 0.0f)
        pos = CreatureCreatePos(this, GetOrientation(), CONTACT_DISTANCE, ang);

    if (!pCreature->Create(GetMap()->GenerateLocalLowGuid(cinfo->GetHighGuid()), pos, cinfo))
    {
        delete pCreature;
        return nullptr;
    }

    Player* player = GetTypeId() == TYPEID_PLAYER ? static_cast<Player*>(this) : nullptr;

    pCreature->SetFactionTemporary(getFaction(), TEMPFACTION_NONE);     // set same faction than player
    pCreature->SetRespawnCoord(pos);                                    // set spawn coord
    pCreature->SetCharmerGuid(GetObjectGuid());                         // save guid of the charmer
    pCreature->SetUInt32Value(UNIT_CREATED_BY_SPELL, spellEntry->Id);   // set the spell id used to create this (may be used for removing corresponding aura
    pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_POSSESSED);          // set flag for client that mean this unit is controlled by a player
    pCreature->addUnitState(UNIT_STAT_CONTROLLED);                      // also set internal unit state flag
    pCreature->SelectLevel(getLevel());                                 // set level to same level than summoner TODO:: not sure its always the case...
    pCreature->SetLinkedToOwnerAura(TEMPSUMMON_LINKED_AURA_OWNER_CHECK | TEMPSUMMON_LINKED_AURA_REMOVE_OWNER); // set what to do if linked aura is removed or the creature is dead.
    pCreature->SetWalk(IsWalking(), true);                              // sync the walking state with the summoner

                                                                        // important before adding to the map!
    SetCharmGuid(pCreature->GetObjectGuid());                           // save guid of charmed creature

    pCreature->SetSummonProperties(TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000); // set 5s corpse decay
    GetMap()->Add(static_cast<Creature*>(pCreature));                   // create the creature in the client
    pCreature->AIM_Initialize();                                        // even if this will be replaced it need to be initialized to take care of spawn spells

                                                                        // Give the control to the player
    if (player)
    {
        player->GetCamera().SetView(pCreature);                         // modify camera view to the creature view
        player->SetClientControl(pCreature, 1);                         // transfer client control to the creature
        player->SetMover(pCreature);                                    // set mover so now we know that creature is "moved" by this unit
        player->SendForcedObjectUpdate();                               // we have to update client data here to avoid problem with the "release spirit" windows reappear.

        pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE); // this seem to be needed for controlled creature (else cannot attack neutral creature)

                                                                        // player pet is unsumoned while possessing
        player->UnsummonPetTemporaryIfAny();
    }

    // init CharmInfo class that will hold charm data
    CharmInfo* charmInfo = pCreature->InitCharmInfo(pCreature);

    // set temp possess ai (creature will not be able to react by itself)
    charmInfo->SetCharmState("PossessedAI");

    if (player)
    {
        // Initialize pet bar
        if (CharmInfo* charmInfo = pCreature->InitCharmInfo(pCreature))
            charmInfo->InitPossessCreateSpells();
        player->PossessSpellInitialize();
    }

    // Creature Linking, Initial load is handled like respawn
    if (pCreature->IsLinkingEventTrigger())
        GetMap()->GetCreatureLinkingHolder()->DoCreatureLinkingEvent(LINKING_EVENT_RESPAWN, pCreature);

    // return the creature therewith the summoner has access to it
    return pCreature;
}

bool Unit::TakePossessOf(Unit* possessed)
{
    Player* player = nullptr;
    if (GetTypeId() == TYPEID_PLAYER)
        player = static_cast<Player *>(this);

    // stop combat but keep threat list
    possessed->AttackStop(true, true);
    possessed->ClearInCombat();

    possessed->addUnitState(UNIT_STAT_CONTROLLED);
    possessed->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_POSSESSED);
    possessed->SetCharmerGuid(GetObjectGuid());

    SetCharm(possessed);

    Creature* possessedCreature = nullptr;
    Player* possessedPlayer = nullptr;
    CharmInfo* charmInfo = possessed->InitCharmInfo(possessed);

    // stop any generated movement TODO:: this may not be correct! what about possessing a feared creature?
    possessed->GetMotionMaster()->Clear();
    possessed->GetMotionMaster()->MoveIdle();
    possessed->StopMoving(true);

    if (possessed->GetTypeId() == TYPEID_UNIT)
    {
        possessedCreature = static_cast<Creature *>(possessed);
        possessedCreature->SetFactionTemporary(getFaction(), TEMPFACTION_NONE);
        possessedCreature->SetWalk(IsWalking(), true);
        charmInfo->SetCharmState("PossessedAI");
        getHostileRefManager().deleteReference(possessedCreature);

        // this seem to be needed for controlled creature (else cannot attack neutral creature)
        if (player)
            possessed->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
    }
    else if (possessed->GetTypeId() == TYPEID_PLAYER)
    {
        possessedPlayer = static_cast<Player *>(possessed);

        if (player && player->IsInDuelWith(possessedPlayer))
            possessedPlayer->SetUInt32Value(PLAYER_DUEL_TEAM, player->GetUInt32Value(PLAYER_DUEL_TEAM));
        else
            possessedPlayer->setFaction(getFaction());
    }

    if (player)
    {
        player->GetCamera().SetView(possessed);
        player->SetClientControl(possessed, player->IsClientControl(possessed));
        player->SetMover(possessed);
        player->SendForcedObjectUpdate();

        if (possessedCreature)
        {
            if (possessedCreature->IsPet() && possessedCreature->GetObjectGuid() == GetPetGuid())
            {
                // possessing own pet, pet bar already initialized
                return true;
            }
        }

        // player pet is unsmumoned while possessing
        player->UnsummonPetTemporaryIfAny();

        charmInfo->InitPossessCreateSpells();
        charmInfo->SetReactState(REACT_PASSIVE);
        charmInfo->SetCommandState(COMMAND_STAY);
        player->PossessSpellInitialize();
    }

    if (possessedPlayer)
        possessedPlayer->SetClientControl(possessed, 0);

    return true;
}

bool Unit::TakeCharmOf(Unit* charmed)
{

    Player* charmerPlayer = nullptr;
    if (GetTypeId() == TYPEID_PLAYER)
    {
        charmerPlayer = static_cast<Player *>(this);

        // player pet is unsmumoned while possessing
        charmerPlayer->UnsummonPetTemporaryIfAny();
    }

    // stop combat but keep threat list
    charmed->AttackStop(true, true);
    charmed->ClearInCombat();

    charmed->SetCharmerGuid(GetObjectGuid());
    SetCharm(charmed);

    CharmInfo* charmInfo = charmed->InitCharmInfo(charmed);

    if (charmed->GetTypeId() == TYPEID_PLAYER)
    {
        Player* charmedPlayer = static_cast<Player *>(charmed);
        if (charmerPlayer && charmerPlayer->IsInDuelWith(charmedPlayer))
            charmedPlayer->SetUInt32Value(PLAYER_DUEL_TEAM, charmerPlayer->GetUInt32Value(PLAYER_DUEL_TEAM));
        else
            charmedPlayer->setFaction(getFaction());

        charmInfo->SetCharmState("PetAI");
        charmed->SetByteValue(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_SUPPORTABLE | UNIT_BYTE2_FLAG_AURAS); // important have to be after charminfo initialization
        //charmedPlayer->SetWalk(IsWalking(), true);

        charmInfo->InitCharmCreateSpells();
        charmInfo->SetReactState(REACT_DEFENSIVE);
        charmInfo->SetCommandState(COMMAND_FOLLOW);
        charmInfo->SetIsRetreating(true);

        charmedPlayer->SetClientControl(charmedPlayer, 0);
        charmedPlayer->ForceHealAndPowerUpdateInZone();
    }
    else if (charmed->GetTypeId() == TYPEID_UNIT)
    {
        Creature* charmedCreature = static_cast<Creature *>(charmed);
        charmInfo->SetCharmState("PetAI");
        getHostileRefManager().deleteReference(charmedCreature);
        charmedCreature->SetFactionTemporary(getFaction(), TEMPFACTION_NONE);
        charmedCreature->SetWalk(IsWalking(), true);

        charmInfo->InitCharmCreateSpells();
        charmInfo->SetReactState(REACT_DEFENSIVE);
        charmInfo->SetCommandState(COMMAND_FOLLOW);
        charmInfo->SetIsRetreating(true);

        if (charmerPlayer)
        {
            // without this charmed unit will not be able to attack neutral target
            charmed->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);

            if (getClass() == CLASS_WARLOCK)
            {
                CreatureInfo const* cinfo = charmedCreature->GetCreatureInfo();
                if (cinfo && cinfo->CreatureType == CREATURE_TYPE_DEMON)
                {
                    // creature with pet number expected have class set
                    if (charmedCreature->GetByteValue(UNIT_FIELD_BYTES_0, 1) == 0)
                    {
                        if (cinfo->UnitClass == 0)
                            sLog.outErrorDb("Creature (Entry: %u) have UnitClass = 0 but used in charmed spell, that will be result client crash.", cinfo->Entry);
                        else
                            sLog.outError("Creature (Entry: %u) have UnitClass = %u but at charming have class 0!!! that will be result client crash.", cinfo->Entry, cinfo->UnitClass);

                        charmedCreature->SetByteValue(UNIT_FIELD_BYTES_0, 1, CLASS_MAGE);
                    }

                    // just to enable stat window
                    charmInfo->SetPetNumber(sObjectMgr.GeneratePetNumber(), true);
                    // if charmed two demons the same session, the 2nd gets the 1st one's name
                    charmedCreature->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, uint32(time(nullptr)));
                }
            }
        }
    }

    if (charmerPlayer)
        charmerPlayer->CharmSpellInitialize();

    return true;
}

void Unit::ResetControlState(bool attackCharmer /*= true*/)
{
    Player* player = nullptr;
    if (GetTypeId() == TYPEID_PLAYER)
        player = static_cast<Player *>(this);

    Unit* possessed = GetCharm();

    if (!possessed)
    {
        if (player)
        {
            player->GetCamera().ResetView();
            player->SetClientControl(player, player->IsClientControl(player));
            player->SetMover(nullptr);
            player->ForceHealAndPowerUpdateInZone();
        }
        return;
    }

    possessed->clearUnitState(UNIT_STAT_CONTROLLED);
    possessed->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_POSSESSED);
    possessed->SetCharmerGuid(ObjectGuid());

    // may be not correct we have to remove only some generator taking account of the current situation
    possessed->StopMoving(true);
    possessed->GetMotionMaster()->Clear();

    SetCharmGuid(ObjectGuid());
    Creature* possessedCreature = nullptr;
    CharmInfo* charmInfo = possessed->GetCharmInfo();

    if (possessed->GetTypeId() == TYPEID_UNIT)
    {
        // now we have to clean threat list to be able to restore normal creature behavior
        FactionTemplateEntry const* factionEntry = possessed->getFactionTemplateEntry();

        possessedCreature = static_cast<Creature *>(possessed);
        if (!possessedCreature->IsPet())
        {
            possessedCreature->ClearTemporaryFaction();

            // first find friendly target (stopping combat here is not recommended because m_attackers will be modified)
            AttackerSet friendlyTargets;
            for (Unit::AttackerSet::const_iterator itr = possessed->getAttackers().begin(); itr != possessed->getAttackers().end(); ++itr)
            {
                Unit* attacker = (*itr);
                if (attacker->GetTypeId() != TYPEID_UNIT)
                    continue;

                if (!factionEntry->IsHostileTo(*attacker->getFactionTemplateEntry()))
                    friendlyTargets.insert(attacker);
            }

            // now stop attackers combat and transfer threat generated from this to owner, also get the total generated threat
            for (Unit::AttackerSet::iterator itr = friendlyTargets.begin(); itr != friendlyTargets.end(); ++itr)
            {
                Unit* attacker = (*itr);
                attacker->AttackStop(true, true);
                attacker->m_Events.KillAllEvents(true);
                attacker->getThreatManager().modifyThreatPercent(possessed, -101);     // only remove the possessed creature from threat list because it can be filled by other players
                attacker->AddThreat(this);
            }

            possessed->AttackStop(true, true);
            possessed->m_Events.KillAllEvents(true);

            charmInfo->ResetCharmState();
            possessed->DeleteCharmInfo();

            // we have to restore initial MotionMaster
            while (possessed->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
                possessed->GetMotionMaster()->MovementExpired(true);

            if (possessed->isAlive())
            {
                possessedCreature->SetCombatStartPosition(GetPositionX(), GetPositionY(), GetPositionZ()); // needed for creature not yet entered in combat or SelectHostileTarget() will fail

                // TODO:: iam not sure we need that faction check
                if (!factionEntry->IsFriendlyTo(*getFactionTemplateEntry()))
                    possessed->getThreatManager().addThreat(this, GetMaxHealth());     // generating threat by max life amount best way i found to make it realistic
            }
            else
                possessed->GetCombatData()->threatManager.clearReferences();

            // we can remove that flag if its set, that is supposed to be only for creature controlled by player
            // however player's pet should keep it
            possessed->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
        }
        else
        {
            possessed->setFaction(possessedCreature->GetOwner()->getFaction());

            charmInfo->ResetCharmState();

            // as possessed is a pet we have to restore original charminfo so Pet::DeleteCharmInfo will take care of that
            possessed->DeleteCharmInfo();
        }
    }
    else if (possessed->GetTypeId() == TYPEID_PLAYER)
    {
        possessed->AttackStop(true, true);

        Player* possessedPlayer = static_cast<Player *>(possessed);

        if (player && player->IsInDuelWith(possessedPlayer))
            possessedPlayer->SetUInt32Value(PLAYER_DUEL_TEAM, player->GetUInt32Value(PLAYER_DUEL_TEAM) == 1 ? 2 : 1);
        else
            possessedPlayer->setFactionForRace(possessedPlayer->getRace());

        possessedPlayer->SetClientControl(possessedPlayer, possessedPlayer->IsClientControl(possessedPlayer));
        charmInfo->ResetCharmState();
        possessedPlayer->DeleteCharmInfo();

        possessedPlayer->ForceHealAndPowerUpdateInZone();

        while (possessedPlayer->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
            possessedPlayer->GetMotionMaster()->MovementExpired(true);
    }

    if (player)
    {
        player->SetClientControl(possessed, 0);
        player->SetMover(nullptr);
        player->GetCamera().ResetView();

        // player pet can be re summoned here
        player->ResummonPetTemporaryUnSummonedIfAny();

        // remove pet bar only if no pet
        if (!player->GetPet())
            player->RemovePetActionBar();
        else
            player->PetSpellInitialize();   // reset spell on pet bar

        player->ForceHealAndPowerUpdateInZone();
    }

    // be sure all those change will be made for clients
    SendForcedObjectUpdate();
}

float Unit::GetAttackDistance(Unit const* pl) const
{
    float aggroRate = sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_AGGRO);
    if (aggroRate == 0)
        return 0.0f;

    uint32 playerlevel = pl->GetLevelForTarget(this);
    uint32 creaturelevel = GetLevelForTarget(pl);

    int32 leveldif = int32(playerlevel) - int32(creaturelevel);

    // "The maximum Aggro Radius has a cap of 25 levels under. Example: A level 30 char has the same Aggro Radius of a level 5 char on a level 60 mob."
    if (leveldif < -25)
        leveldif = -25;

    // "The aggro radius of a mob having the same level as the player is roughly 20 yards"
    float RetDistance = 20;

    // "Aggro Radius varies with level difference at a rate of roughly 1 yard/level"
    // radius grow if playlevel < creaturelevel
    RetDistance -= (float)leveldif;

    if (creaturelevel + 5 <= sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
    {
        // detect range auras
        RetDistance += GetTotalAuraModifier(SPELL_AURA_MOD_DETECT_RANGE);

        // detected range auras
        RetDistance += pl->GetTotalAuraModifier(SPELL_AURA_MOD_DETECTED_RANGE);
    }

    // "Minimum Aggro Radius for a mob seems to be combat range (5 yards)"
    if (RetDistance < 5)
        RetDistance = 5;

    return (RetDistance * aggroRate);
}
