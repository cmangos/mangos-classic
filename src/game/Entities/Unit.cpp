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
#include "Entities/TemporarySpawn.h"
#include "Entities/Pet.h"
#include "Util.h"
#include "Entities/Totem.h"
#include "BattleGround/BattleGround.h"
#include "Maps/InstanceData.h"
#include "OutdoorPvP/OutdoorPvP.h"
#include "Maps/GridDefines.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "Maps/MapPersistentStateMgr.h"
#include "MotionGenerators/MovementGenerator.h"
#include "Movement/MoveSplineInit.h"
#include "Movement/MoveSpline.h"
#include "Entities/CreatureLinkingMgr.h"
#include "Tools/Formulas.h"
#include "Entities/Transports.h"

#ifdef BUILD_METRICS
 #include "Metric/Metric.h"
#endif

#include <math.h>
#include <limits>
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
    const SpellPartialResistDistribution precalculated =
    {
        {{10000, 0, 0, 0, 0}},
        {{9700, 200, 100, 0, 0}},
        {{9400, 400, 200, 0, 0}},
        {{9000, 800, 200, 0, 0}},
        {{8700, 1000, 300, 0, 0}},
        {{8400, 1200, 400, 0, 0}},
        {{8200, 1300, 400, 100, 0}},
        {{7900, 1500, 500, 100, 0}},
        {{7600, 1700, 600, 100, 0}},
        {{7300, 1900, 700, 100, 0}},
        {{6900, 2300, 700, 100, 0}},
        {{6600, 2500, 800, 100, 0}},
        {{6300, 2700, 900, 100, 0}},
        {{6000, 2900, 1000, 100, 0}},
        {{5800, 3000, 1000, 200, 0}},
        {{5400, 3300, 1100, 200, 0}},
        {{5100, 3600, 1100, 200, 0}},
        {{4800, 3800, 1200, 200, 0}},
        {{4400, 4200, 1200, 200, 0}},
        {{4100, 4400, 1300, 200, 0}},
        {{3700, 4800, 1300, 200, 0}},
        {{3400, 5000, 1400, 200, 0}},
        {{3100, 5200, 1500, 200, 0}},
        {{3000, 5200, 1500, 200, 100}},
        {{2800, 5300, 1500, 300, 100}},
        {{2500, 5500, 1600, 300, 100}},
        {{2400, 5400, 1700, 400, 100}},
        {{2300, 5300, 1800, 500, 100}},
        {{2200, 5100, 2100, 500, 100}},
        {{2100, 5000, 2200, 600, 100}},
        {{2000, 4900, 2400, 600, 100}},
        {{1900, 4700, 2600, 700, 100}},
        {{1800, 4600, 2700, 800, 100}},
        {{1700, 4400, 3000, 800, 100}},
        {{1600, 4300, 3100, 900, 100}},
        {{1500, 4200, 3200, 1000, 100}},
        {{1400, 4100, 3300, 1100, 100}},
        {{1300, 3900, 3600, 1100, 100}},
        {{1300, 3600, 3800, 1200, 100}},
        {{1200, 3500, 3900, 1300, 100}},
        {{1100, 3400, 4000, 1400, 100}},
        {{1000, 3300, 4100, 1500, 100}},
        {{900, 3100, 4400, 1500, 100}},
        {{800, 3000, 4500, 1600, 100}},
        {{800, 2700, 4700, 1700, 100}},
        {{700, 2600, 4800, 1800, 100}},
        {{600, 2500, 4900, 1900, 100}},
        {{600, 2300, 5000, 1900, 200}},
        {{500, 2200, 5100, 2000, 200}},
        {{300, 2200, 5300, 2000, 200}},
        {{200, 2100, 5400, 2100, 200}},
        {{200, 2000, 5300, 2200, 300}},
        {{200, 2000, 5100, 2200, 500}},
        {{200, 1900, 5000, 2300, 600}},
        {{100, 1900, 4900, 2500, 600}},
        {{100, 1800, 4800, 2600, 700}},
        {{100, 1700, 4700, 2700, 800}},
        {{100, 1600, 4500, 3000, 800}},
        {{100, 1500, 4400, 3100, 900}},
        {{100, 1500, 4100, 3300, 1000}},
        {{100, 1400, 4000, 3400, 1100}},
        {{100, 1300, 3900, 3500, 1200}},
        {{100, 1200, 3800, 3600, 1300}},
        {{100, 1100, 3600, 3900, 1300}},
        {{100, 1100, 3300, 4100, 1400}},
        {{100, 1000, 3200, 4200, 1500}},
        {{100, 900, 3100, 4300, 1600}},
        {{100, 800, 3000, 4400, 1700}},
        {{100, 800, 2700, 4600, 1800}},
        {{100, 700, 2600, 4700, 1900}},
        {{100, 600, 2400, 4900, 2000}},
        {{100, 600, 2200, 5000, 2100}},
        {{100, 500, 2100, 5100, 2200}},
        {{100, 500, 1800, 5300, 2300}},
        {{100, 400, 1700, 5400, 2400}},
        {{100, 300, 1600, 5500, 2500}},
        {{100, 300, 1500, 5300, 2800}},
        {{100, 200, 1500, 5200, 3000}},
        {{0, 200, 1500, 5200, 3100}},
        {{0, 200, 1400, 5000, 3400}},
        {{0, 200, 1300, 4800, 3700}},
        {{0, 200, 1300, 4400, 4100}},
        {{0, 200, 1200, 4200, 4400}},
        {{0, 200, 1200, 3800, 4800}},
        {{0, 200, 1100, 3600, 5100}},
        {{0, 200, 1100, 3300, 5400}},
        {{0, 200, 1000, 3000, 5800}},
        {{0, 100, 1000, 2900, 6000}},
        {{0, 100, 900, 2700, 6300}},
        {{0, 100, 800, 2500, 6600}},
        {{0, 100, 700, 2300, 6900}},
        {{0, 100, 700, 1900, 7300}},
        {{0, 100, 600, 1700, 7600}},
        {{0, 100, 500, 1500, 7900}},
        {{0, 100, 400, 1300, 8200}},
        {{0, 0, 400, 1200, 8400}},
        {{0, 0, 300, 1000, 8700}},
        {{0, 0, 200, 800, 9000}},
        {{0, 0, 200, 400, 9400}},
        {{0, 0, 100, 200, 9700}},
        {{0, 0, 0, 0, 10000}},
    };
    // Inflate up to two decimal places of chance %: add all intermediate values and 100% value (100*100 + 1)
    SpellPartialResistDistribution inflated(10001, {{}});
    for (size_t row = 0; row < precalculated.size(); ++row)
    {
        const size_t next = (row + 1);
        const size_t shift = (row * 100);
        const auto& source = precalculated.at(row);
        // Check if this is the last one first
        if (next == precalculated.size())
        {
            for (uint8 column = SPELL_PARTIAL_RESIST_NONE; column < NUM_SPELL_PARTIAL_RESISTS; ++column)
                inflated[shift][column] = source.at(column);
            break;
        }
        const auto& ahead = precalculated.at(next);
        for (size_t intermediate = 0; intermediate < 100; ++intermediate)
        {
            const size_t index = (shift + intermediate);
            auto& values = inflated[index];
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
    stime = sWorld.GetCurrentMSTime();
    data >> moveFlags;
    data >> ctime;
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
        data >> jump.cosAngle;
        data >> jump.sinAngle;
        data >> jump.xyspeed;
        if (!jump.startClientTime)
        {
            jump.startClientTime = ctime;
            jump.start = pos;
        }
    }
    else
        jump.startClientTime = 0;

    if (HasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
    {
        data >> u_unk1;                                     // unknown
    }
}

void MovementInfo::Write(ByteBuffer& data) const
{
    data << moveFlags;
    data << stime;
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
        data << jump.cosAngle;
        data << jump.sinAngle;
        data << jump.xyspeed;
    }

    if (HasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
    {
        data << u_unk1;                                     // unknown
    }
}

float MovementInfo::GetOrientationInMotion(MovementFlags flags, float orientation)
{
    float mod = ((flags & MOVEFLAG_BACKWARD) ? M_PI_F : 0);

    if (flags & (MOVEFLAG_STRAFE_LEFT | MOVEFLAG_STRAFE_RIGHT))
    {
        float flip = (M_PI_F * ((flags & MOVEFLAG_STRAFE_LEFT) ? 0.5f : -0.5f));
        flip = ((flags & MOVEFLAG_BACKWARD) ? -flip : flip);
        mod += (flip * ((flags & (MOVEFLAG_FORWARD | MOVEFLAG_BACKWARD)) ? 0.5f : 1));
    }
    return MapManager::NormalizeOrientation(orientation + mod);
}

////////////////////////////////////////////////////////////
// Methods of class Unit

Unit::Unit() :
    movespline(new Movement::MoveSpline()),
    m_charmInfo(nullptr),
    i_motionMaster(this),
    m_regenTimer(0),
    m_combatData(new CombatData(this)),
    m_guardianPetsIterator(m_guardianPets.end()),
    m_spellUpdateHappening(false),
    m_spellProcsHappening(false),
    m_auraUpdateMask(0),
    m_ignoreRangedTargets(false),
    m_combatManager(this)
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

    for (auto& m_currentSpell : m_currentSpells)
        m_currentSpell = nullptr;

    m_castCounter = 0;

    // m_Aura = nullptr;
    // m_AurasCheck = 2000;
    // m_removeAuraTimer = 4;
    m_spellAuraHoldersUpdateIterator = m_spellAuraHolders.end();
    m_AuraFlags = 0;

    m_Visibility = VISIBILITY_ON;
    m_AINotifyEvent = nullptr;

    m_transform = 0;
    m_canModifyStats = false;

    for (auto& i : m_spellImmune)
        i.clear();
    for (auto& i : m_auraModifiersGroup)
    {
        i[BASE_VALUE] = 0.0f;
        i[BASE_PCT] = 1.0f;
        i[TOTAL_VALUE] = 0.0f;
        i[TOTAL_PCT] = 1.0f;
    }

    // implement 50% base damage from offhand
    m_auraModifiersGroup[UNIT_MOD_DAMAGE_OFFHAND][TOTAL_PCT] = 0.5f;

    for (int i = 0; i < MAX_STATS; ++i)
        m_createStats[i] = 0.0f;

    for (auto& m_createResistance : m_createResistances)
        m_createResistance = 0;

    m_attacking = nullptr;
    m_modMeleeHitChance = 0.0f;
    m_modRangedHitChance = 0.0f;
    m_modSpellHitChance = 0.0f;
    for (float& i : m_modSpellCritChance)
        i = 0.0f;

    for (float& i : m_modCritChance)
        i = 0.0f;
    m_modDodgeChance = 0.0f;
    m_modParryChance = 0.0f;
    m_modBlockChance = 0.0f;

    m_canDodge = true;
    m_canParry = true;
    m_canBlock = true;

    m_lastManaUseTimer = 0;

    // m_victimThreat = 0.0f;
    for (float& i : m_threatModifier)
        i = 1.0f;
    m_isSorted = true;
    for (float& i : m_speed_rate)
        i = 1.0f;

    // remove aurastates allowing special moves
    for (unsigned int& i : m_reactiveTimer)
        i = 0;

    m_isCreatureLinkingTrigger = false;
    m_isSpawningLinked = false;
    m_dummyCombatState = false;

    m_canEnterCombat = true;

    m_noThreat = false;
    m_extraAttacksExecuting = false;
    m_debuggingMovement = false;

    m_baseSpeedWalk = 1.f;
    m_baseSpeedRun = 1.f;

    m_comboPoints = 0;
}

Unit::~Unit()
{
    // set current spells as deletable
    for (auto& m_currentSpell : m_currentSpells)
    {
        if (m_currentSpell)
        {
            m_currentSpell->SetReferencedFromCurrent(false);
            m_currentSpell = nullptr;
        }
    }

    CleanupDeletedAuras();

    delete m_combatData;
    delete m_charmInfo;
    delete movespline;

    // those should be already removed at "RemoveFromWorld()" call
    MANGOS_ASSERT(m_gameObj.empty());
    MANGOS_ASSERT(m_dynObjGUIDs.empty());
    MANGOS_ASSERT(m_deletedAuras.empty());
    MANGOS_ASSERT(m_deletedHolders.empty());
}

void Unit::Update(const uint32 diff)
{
    if (!IsInWorld())
        return;
#ifdef BUILD_METRICS
    metric::duration<std::chrono::microseconds> meas("unit.update", {
        { "entry", std::to_string(GetEntry()) },
        { "guid", std::to_string(GetGUIDLow()) },
        { "unit_type", std::to_string(GetGUIDHigh()) },
        { "map_id", std::to_string(GetMapId()) },
        { "instance_id", std::to_string(GetInstanceId()) }
    }, 1000);
#endif

    /*if(p_time > m_AurasCheck)
    {
    m_AurasCheck = 2000;
    _UpdateAura();
    }else
    m_AurasCheck -= p_time;*/

    // WARNING! Order of execution here is important, do not change.
    // Spells must be processed with event system BEFORE they go to _UpdateSpells.
    // Or else we may have some SPELL_STATE_FINISHED spells stalled in pointers, that is bad.
    m_spellUpdateHappening = true;

    UpdateCooldowns(GetMap()->GetCurrentClockTime());
    m_events.Update(diff);
    _UpdateSpells(diff);
    m_spellUpdateHappening = false;

    CleanupDeletedAuras();

    if (m_lastManaUseTimer)
    {
        if (diff >= m_lastManaUseTimer)
            m_lastManaUseTimer = 0;
        else
            m_lastManaUseTimer -= diff;
    }

    if (uint32 base_att = getAttackTimer(BASE_ATTACK))
        setAttackTimer(BASE_ATTACK, (diff >= base_att ? 0 : base_att - diff));

    if (uint32 base_att = getAttackTimer(OFF_ATTACK))
        setAttackTimer(OFF_ATTACK, (diff >= base_att ? 0 : base_att - diff));

    // update abilities available only for fraction of time
    UpdateReactives(diff);

    UpdateSplineMovement(diff);
    i_motionMaster.UpdateMotion(diff);

    if (AI() && IsAlive())
    {
#ifdef BUILD_METRICS
        metric::duration<std::chrono::microseconds> meas_ai("unit.update.ai", {
            { "entry", std::to_string(GetEntry()) },
            { "guid", std::to_string(GetGUIDLow()) },
            { "unit_type", std::to_string(GetGUIDHigh()) },
            { "map_id", std::to_string(GetMapId()) },
            { "instance_id", std::to_string(GetInstanceId()) }
        }, 1000);
#endif

        AI()->UpdateAI(diff);   // AI not react good at real update delays (while freeze in non-active part of map)
    }

    GetCombatManager().Update(diff);

    if (IsAlive())
        ModifyAuraState(AURA_STATE_HEALTHLESS_20_PERCENT, GetHealth() < GetMaxHealth() * 0.20f);
}

void Unit::TriggerAggroLinkingEvent(Unit* enemy)
{
    if (IsLinkingEventTrigger())
        GetMap()->GetCreatureLinkingHolder()->DoCreatureLinkingEvent(LINKING_EVENT_AGGRO, static_cast<Creature*>(this), enemy);
}

void Unit::TriggerEvadeEvents()
{
    static_cast<Creature*>(this)->SetLootRecipient(nullptr);

    if (InstanceData* mapInstance = GetInstanceData())
        mapInstance->OnCreatureEvade((Creature*)this);

    if (m_isCreatureLinkingTrigger)
        GetMap()->GetCreatureLinkingHolder()->DoCreatureLinkingEvent(LINKING_EVENT_EVADE, static_cast<Creature*>(this));

    CallForAllControlledUnits([](Unit* unit) { unit->HandleExitCombat(); }, CONTROLLED_PET | CONTROLLED_GUARDIANS | CONTROLLED_CHARM | CONTROLLED_TOTEMS);
}

void Unit::EvadeTimerExpired()
{
    // Can happen for any unit with an AI
    if (!AI())
        return;

    // When there is no target to switch to, leave combat
    if (getThreatManager().getThreatList().size() == 1)
    {
        AI()->EnterEvadeMode();
        return;
    }

    getThreatManager().SetTargetSuppressed(GetVictim());
}

enum SwingErrors
{
    SWING_ERROR_NOT_IN_RANGE,
    SWING_ERROR_TARGET_NOT_ALIVE,
    SWING_ERROR_BAD_FACING,
    SWING_ERROR_CANT_ATTACK_TARGET,
};

bool Unit::UpdateMeleeAttackingState()
{
    Unit* victim = GetVictim();
    if (!victim || IsNonMeleeSpellCasted(false))
        return false;

    if (GetTypeId() != TYPEID_PLAYER && (!static_cast<Creature*>(this)->CanInitiateAttack()))
        return false;

    if (!isAttackReady(BASE_ATTACK) && !(isAttackReady(OFF_ATTACK) && hasOffhandWeaponForAttack()))
        return false;

    uint8 swingError = 0;
    if (!CanReachWithMeleeAttack(victim))
        swingError = SWING_ERROR_NOT_IN_RANGE;
    else if (!HasInArc(victim))
        swingError = SWING_ERROR_BAD_FACING;
    else if (!victim->IsAlive())
        swingError = SWING_ERROR_TARGET_NOT_ALIVE;
    else if (!CanAttack(victim))
        swingError = SWING_ERROR_CANT_ATTACK_TARGET;
    else
    {
        if (isAttackReady(BASE_ATTACK))
        {
            // prevent base and off attack in same time, delay attack at 0.2 sec
            if (hasOffhandWeaponForAttack())
            {
                if (getAttackTimer(OFF_ATTACK) < ATTACK_DISPLAY_DELAY)
                    setAttackTimer(OFF_ATTACK, ATTACK_DISPLAY_DELAY);
            }
            AttackerStateUpdate(victim, BASE_ATTACK);
            resetAttackTimer(BASE_ATTACK);
        }
        if (hasOffhandWeaponForAttack() && isAttackReady(OFF_ATTACK))
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

    if (swingError)
    {
        setAttackTimer(BASE_ATTACK, 100);
        setAttackTimer(OFF_ATTACK, 100);
    }

    Player* player = IsClientControlled() ? const_cast<Player*>(GetClientControlling()) : nullptr;
    if (player && swingError != player->LastSwingErrorMsg())
    {
        switch (swingError)
        {
            case SWING_ERROR_NOT_IN_RANGE:
                player->SendAttackSwingNotInRange();
                break;
            case SWING_ERROR_TARGET_NOT_ALIVE:
                player->SendAttackSwingDeadTarget();
                break;
            case SWING_ERROR_BAD_FACING:
                player->SendAttackSwingBadFacingAttack();
                break;
            case SWING_ERROR_CANT_ATTACK_TARGET:
                player->SendAttackSwingCantAttack();
                break;
        }
        player->SwingErrorMsg(swingError);
    }

    return swingError != 0;
}

void Unit::SendHeartBeat()
{
    WorldPacket data(MSG_MOVE_HEARTBEAT, 31);
    data << GetPackGUID();
    data << m_movementInfo;
    SendMessageToSet(data, true);
}

void Unit::SendMoveRoot(bool state, bool broadcastOnly)
{
    const Player* client = GetClientControlling();

    // Apply flags in-place when unit currently is not controlled by a player
    if (!client && !broadcastOnly)
    {
        if (state)
        {
            m_movementInfo.RemoveMovementFlag(movementFlagsMask);
            m_movementInfo.AddMovementFlag(MOVEFLAG_ROOT);
        }
        else
            m_movementInfo.RemoveMovementFlag(MOVEFLAG_ROOT);
    }

    const PackedGuid &guid = GetPackGUID();
    // Pre-Wrath spline root: when unit is currently not controlled by a player, or broadcasting to others
    if (!client || broadcastOnly)
    {
        WorldPacket data(state ? SMSG_SPLINE_MOVE_ROOT : SMSG_SPLINE_MOVE_UNROOT, guid.size());
        data << guid;
        SendMessageToSet(data, (!broadcastOnly));
    }
    // Pre-Wrath force root: send only to the controlling player
    else
    {
        WorldPacket data(state ? SMSG_FORCE_MOVE_ROOT : SMSG_FORCE_MOVE_UNROOT, guid.size() + 4);
        data << guid;
        data << uint32(0);
        client->GetSession()->SendPacket(data);
    }
}

void Unit::resetAttackTimer(WeaponAttackType type)
{
    m_attackTimer[type] = uint32(GetAttackTime(type) * m_modAttackSpeedPct[type]);
}

bool Unit::CanReachWithMeleeAttack(Unit const* pVictim, float flat_mod /*= 0.0f*/) const
{
    MANGOS_ASSERT(pVictim);

    float reach = GetCombinedCombatReach(pVictim, true, flat_mod);

    if (IsMoving() && !IsWalking() && pVictim->IsMoving() && !pVictim->IsWalking())
        reach += MELEE_LEEWAY;

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
        Aura* aura = (*iter);
        SpellAuraHolder* holder = aura->GetHolder();
        RemoveSpellAuraHolder(holder);
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

void Unit::RemoveSpellsCausingAura(AuraType auraType, SpellAuraHolder* except, bool onlyNegative)
{
    for (AuraList::const_iterator iter = m_modAuras[auraType].begin(); iter != m_modAuras[auraType].end();)
    {
        if ((*iter)->GetHolder() == except || (onlyNegative && ((*iter)->GetHolder()->IsPositive() || (*iter)->GetHolder()->GetSpellProto()->HasAttribute(SPELL_ATTR_AURA_IS_DEBUFF))))
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

void Unit::DealDamageMods(Unit* dealer, Unit* victim, uint32& damage, uint32* absorb, DamageEffectType damagetype, SpellEntry const* spellProto)
{
    if (!victim->IsAlive() || victim->IsTaxiFlying() || victim->GetCombatManager().IsInEvadeMode())
    {
        if (absorb)
            *absorb += damage;

        damage = 0;
        return;
    }

    uint32 originalDamage = damage;

    if (dealer) // dealer is optional
    {
        if (UnitAI* ai = dealer->AI()) // Script Event damage Deal
            ai->DamageDeal(victim, damage, damagetype, spellProto);
    }
    // Script Event damage taken
    if (victim->AI())
        victim->AI()->DamageTaken(dealer, damage, damagetype, spellProto);

    if (absorb && originalDamage > damage)
        *absorb += (originalDamage - damage);
}

void Unit::Suicide()
{
    DealDamage(this, this, this->GetHealth(), nullptr, INSTAKILL, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
}

uint32 Unit::DealDamage(Unit* dealer, Unit* victim, uint32 damage, CleanDamage const* cleanDamage, DamageEffectType damagetype, SpellSchoolMask damageSchoolMask, SpellEntry const* spellProto, bool durabilityLoss)
{
    // remove affects from attacker at any non-DoT damage (including 0 damage)
    if (damagetype != DOT && damagetype != INSTAKILL)
    {
        // Since patch 1.5.0 sitting characters always stand up on attack (even if stunned)
        if (!victim->IsStandState() && (victim->GetTypeId() == TYPEID_PLAYER || !victim->IsStunned()))
            victim->SetStandState(UNIT_STAND_STATE_STAND);
    }

    if (dealer)
    {
        // Rage from Damage made (only from direct weapon damage)
        if (cleanDamage && damagetype == DIRECT_DAMAGE && dealer != victim && dealer->GetTypeId() == TYPEID_PLAYER && dealer->GetPowerType() == POWER_RAGE && cleanDamage->attackType != RANGED_ATTACK)
            static_cast<Player*>(dealer)->RewardRage(damage, true);
    }

    if (!damage)
    {
        // Rage from physical damage received - extend to all units
        if (cleanDamage && cleanDamage->damage && (damageSchoolMask & SPELL_SCHOOL_MASK_NORMAL) && victim->GetTypeId() == TYPEID_PLAYER && (victim->GetPowerType() == POWER_RAGE))
            if (cleanDamage->hitOutCome != MELEE_HIT_DODGE && cleanDamage->hitOutCome != MELEE_HIT_PARRY)
                static_cast<Player*>(victim)->RewardRage(cleanDamage->damage, false);

        return 0;
    }

    DEBUG_FILTER_LOG(LOG_FILTER_DAMAGE, "DealDamageStart");

    uint32 health = victim->GetHealth();
    DEBUG_FILTER_LOG(LOG_FILTER_DAMAGE, "deal dmg:%d to health:%d ", damage, health);

    // duel ends when player has 1 or less hp
    bool duel_hasEnded = false;
    if (dealer)
    {
        if (dealer->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && victim->GetTypeId() == TYPEID_PLAYER && static_cast<Player*>(victim)->duel && damage >= (health - 1))
        {
            // prevent kill only if killed in duel and killed by opponent or opponent controlled creature
            Player* playerVictim = static_cast<Player*>(victim);
            if (playerVictim->duel->opponent == dealer || playerVictim->duel->opponent->GetObjectGuid() == dealer->GetControllingPlayer()->GetObjectGuid())
                damage = health - 1;

            duel_hasEnded = true;
        }

        if (victim->GetTypeId() == TYPEID_UNIT)
        {
            if (Creature* creatureVictim = static_cast<Creature*>(victim))
            {
                if (!creatureVictim->IsPet() && !creatureVictim->HasLootRecipient())
                    creatureVictim->SetLootRecipient(dealer);

                if (creatureVictim->HasLootRecipient())
                {
                    if (Player* player = creatureVictim->GetLootRecipient())
                    {
                        // Note: there is evidence that pre-mop this was split between player/nonplayer instead of group/nongroup
                        if (!dealer->IsInGroup(player))
                            victim->m_damageByOthers += damage;
                    }
                }
            }
        }
    }

    if (health <= damage)
        Kill(dealer, victim, damagetype, spellProto, durabilityLoss, duel_hasEnded);
    else                                                    // if (health <= damage)
        HandleDamageDealt(dealer, victim, damage, cleanDamage, damagetype, damageSchoolMask, spellProto, duel_hasEnded);

    DEBUG_FILTER_LOG(LOG_FILTER_DAMAGE, "DealDamageEnd returned %d damage", damage);

    return damage;
}

void Unit::Kill(Unit* killer, Unit* victim, DamageEffectType damagetype, SpellEntry const* spellProto, bool durabilityLoss, bool duel_hasEnded)
{
    DEBUG_FILTER_LOG(LOG_FILTER_DAMAGE, "DealDamage %s Killed %s", killer ? killer->GetGuidStr().c_str() : "", victim->GetGuidStr().c_str());

    /*
    *  Preparation: Who gets credit for killing whom, invoke SpiritOfRedemtion?
    */
    // for loot will be used only if group_tap == nullptr
    Player* responsiblePlayer = killer ? killer->GetBeneficiaryPlayer() : nullptr;
    Player* tapper = responsiblePlayer;
    Group* tapperGroup = nullptr;

    // in creature kill case group/player tap stored for creature
    if (victim->GetTypeId() == TYPEID_UNIT)
    {
        if (Player* recipient = ((Creature*)victim)->GetOriginalLootRecipient())
            tapper = recipient;

        tapperGroup = ((Creature*)victim)->GetGroupLootRecipient();
    }
    else
    {
        if (tapper)
            tapperGroup = tapper->GetGroup();
    }

    // Spirit of Redemtion Talent
    bool damageFromSpiritOfRedemtionTalent = spellProto && spellProto->Id == 27965;
    // if talent known but not triggered (check priest class for speedup check)
    Aura* spiritOfRedemtionTalentReady = nullptr;
    if (!damageFromSpiritOfRedemtionTalent &&           // not called from SPELL_AURA_SPIRIT_OF_REDEMPTION
        victim->GetTypeId() == TYPEID_PLAYER && victim->getClass() == CLASS_PRIEST)
    {
        AuraList const& vDummyAuras = victim->GetAurasByType(SPELL_AURA_DUMMY);
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
    if (damagetype != INSTAKILL)
    {
        if (responsiblePlayer && responsiblePlayer != victim && (responsiblePlayer == tapper || (responsiblePlayer->GetGroup() == tapperGroup && tapperGroup)))
        {
            WorldPacket data(SMSG_PARTYKILLLOG, (8 + 8));   // send event PARTY_KILL
            data << responsiblePlayer->GetObjectGuid();     // player with killing blow
            data << victim->GetObjectGuid();                // victim

            if (tapperGroup)
                tapperGroup->BroadcastPacket(data, false, tapperGroup->GetMemberGroup(responsiblePlayer->GetObjectGuid()), responsiblePlayer->GetObjectGuid());

            responsiblePlayer->SendDirectMessage(data);
        }
    }

    // proc only once for victim
    if (killer)
        if (Unit* owner = killer->GetOwner())
            ProcDamageAndSpell(ProcSystemArguments(owner, victim, PROC_FLAG_KILL, PROC_FLAG_NONE, PROC_EX_NONE, 0));

    ProcDamageAndSpell(ProcSystemArguments(killer, victim, PROC_FLAG_KILL, PROC_FLAG_KILLED, PROC_EX_NONE, 0));

    // tbc+ has on death proc

    // Reward player, his pets, and group/raid members
    if (tapper != victim)
    {
        if (tapperGroup)
            tapperGroup->RewardGroupAtKill(victim, tapper);
        else if (tapper)
            tapper->RewardSinglePlayerAtKill(victim);
    }

    /*
    *  Actions for the killer
    */
    if (spiritOfRedemtionTalentReady)
    {
        DEBUG_FILTER_LOG(LOG_FILTER_DAMAGE, "DealDamage: Spirit of Redemtion ready");

        // save value before aura remove
        uint32 ressSpellId = victim->GetUInt32Value(PLAYER_SELF_RES_SPELL);
        if (!ressSpellId)
            ressSpellId = ((Player*)victim)->GetResurrectionSpellId();

        // Remove all expected to remove at death auras (most important negative case like DoT or periodic triggers)
        victim->RemoveAllAurasOnDeath();

        // restore for use at real death
        victim->SetUInt32Value(PLAYER_SELF_RES_SPELL, ressSpellId);

        // FORM_SPIRITOFREDEMPTION and related auras
        victim->CastSpell(victim, 27827, TRIGGERED_OLD_TRIGGERED, nullptr, spiritOfRedemtionTalentReady);
    }
    else
        victim->SetHealth(0);

    if (killer)
    {
        // Call KilledUnit for creatures
        if (UnitAI* ai = killer->AI())
            ai->KilledUnit(victim);

        // Call AI OwnerKilledUnit (for any current summoned minipet/guardian/protector)
        killer->PetOwnerKilledUnit(victim);
    }

    /*
    *  Actions for the victim
    */
    if (victim->GetTypeId() == TYPEID_PLAYER)          // Killed player
    {
        Player* playerVictim = (Player*)victim;

        // remember victim PvP death for corpse type and corpse reclaim delay
        // at original death (not at SpiritOfRedemtionTalent timeout)
        if (!damageFromSpiritOfRedemtionTalent)
            playerVictim->SetPvPDeath(responsiblePlayer != nullptr);

        // 10% durability loss on death
        // only if not player and not controlled by player pet. And not at BG
        if (durabilityLoss && !responsiblePlayer && !playerVictim->InBattleGround())
        {
            DEBUG_LOG("DealDamage: Killed %s, looing 10 percents durability", victim->GetGuidStr().c_str());
            playerVictim->DurabilityLossAll(0.10f, false);
            // durability lost message
            WorldPacket data(SMSG_DURABILITY_DAMAGE_DEATH, 0);
            playerVictim->GetSession()->SendPacket(data);
        }

        if (!spiritOfRedemtionTalentReady)              // Before informing Battleground
        {
            DEBUG_FILTER_LOG(LOG_FILTER_DAMAGE, "SET JUST_DIED");
            victim->SetDeathState(JUST_DIED);
        }

        // playerVictim was in duel, duel must be interrupted
        // last damage from non duel opponent or non opponent controlled creature
        if (duel_hasEnded)
        {
            playerVictim->duel->opponent->CombatStopWithPets(true);
            playerVictim->CombatStopWithPets(true);

            playerVictim->DuelComplete(DUEL_INTERRUPTED);
        }

        if (responsiblePlayer)                         // PvP kill
        {
            if (BattleGround* bg = playerVictim->GetBattleGround())
            {
                bg->HandleKillPlayer(playerVictim, responsiblePlayer);
            }
            else if (victim != killer)
            {
                // selfkills are not handled in outdoor pvp scripts
                if (OutdoorPvP* outdoorPvP = sOutdoorPvPMgr.GetScript(playerVictim->GetCachedZoneId()))
                    outdoorPvP->HandlePlayerKill(responsiblePlayer, playerVictim);
            }
        }
    }
    else                                                // Killed creature
        JustKilledCreature(killer, static_cast<Creature*>(victim), responsiblePlayer);

    // stop combat
    DEBUG_FILTER_LOG(LOG_FILTER_DAMAGE, "DealDamageAttackStop");
    victim->CombatStop();
}

void Unit::HandleDamageDealt(Unit* dealer, Unit* victim, uint32& damage, CleanDamage const* cleanDamage, DamageEffectType damagetype, SpellSchoolMask damageSchoolMask, SpellEntry const* spellProto, bool duel_hasEnded)
{
    DEBUG_FILTER_LOG(LOG_FILTER_DAMAGE, "DealDamageAlive");

    victim->ModifyHealth(-(int32)damage);

    if (dealer)
    {
        if (victim->CanAttack(dealer) && (!spellProto || (!spellProto->HasAttribute(SPELL_ATTR_EX3_NO_INITIAL_AGGRO) &&
            !spellProto->HasAttribute(SPELL_ATTR_EX_NO_THREAT))) && dealer->CanEnterCombat() && victim->CanEnterCombat())
        {
            float threat = damage * sSpellMgr.GetSpellThreatMultiplier(spellProto);
            victim->AddThreat(dealer, threat, (cleanDamage && cleanDamage->hitOutCome == MELEE_HIT_CRIT), damageSchoolMask, spellProto);
            if (damagetype != DOT) // DOTs dont put in combat but still cause threat
            {
                dealer->SetInCombatWith(victim);
                victim->SetInCombatWith(dealer);
                dealer->GetCombatManager().TriggerCombatTimer(victim);
            }
        }

        if (dealer->GetTypeId() == TYPEID_PLAYER)
        {
            // random durability for items (HIT DONE)
            if (roll_chance_f(sWorld.getConfig(CONFIG_FLOAT_RATE_DURABILITY_LOSS_DAMAGE)))
            {
                EquipmentSlots slot = EquipmentSlots(urand(0, EQUIPMENT_SLOT_END - 1));
                static_cast<Player*>(dealer)->DurabilityPointLossForEquipSlot(slot);
            }
        }
    }

    if (victim->GetTypeId() == TYPEID_PLAYER)                               // victim is a player
    {
        // Rage from damage received
        if (dealer != victim && victim->GetPowerType() == POWER_RAGE) // player doesnt get rage when he parries
            if (cleanDamage && cleanDamage->damage && cleanDamage->hitOutCome != MELEE_HIT_DODGE && cleanDamage->hitOutCome != MELEE_HIT_PARRY)
                static_cast<Player*>(victim)->RewardRage(cleanDamage->damage, false);

        // random durability for items (HIT TAKEN)
        if (roll_chance_f(sWorld.getConfig(CONFIG_FLOAT_RATE_DURABILITY_LOSS_DAMAGE)))
        {
            EquipmentSlots slot = EquipmentSlots(urand(0, EQUIPMENT_SLOT_END - 1));
            static_cast<Player*>(victim)->DurabilityPointLossForEquipSlot(slot);
        }
    }

    if ((damagetype == DIRECT_DAMAGE || damagetype == SPELL_DIRECT_DAMAGE
    		|| damagetype == DOT || damagetype == SELF_DAMAGE )
        && !(spellProto && spellProto->HasAttribute(SPELL_ATTR_EX4_DAMAGE_DOESNT_BREAK_AURAS)))
    {
        int32 auraInterruptFlags = AURA_INTERRUPT_FLAG_DAMAGE;
        if (damagetype != DOT)
            auraInterruptFlags = (auraInterruptFlags | AURA_INTERRUPT_FLAG_DIRECT_DAMAGE);

        SpellAuraHolderMap& vInterrupts = victim->GetSpellAuraHolderMap();
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
            victim->RemoveAurasDueToSpell(aura);
    }

    if (dealer)
        dealer->InterruptOrDelaySpell(victim, damagetype);

    // last damage from duel opponent
    if (duel_hasEnded)
    {
        MANGOS_ASSERT(victim->GetTypeId() == TYPEID_PLAYER);
        Player* he = static_cast<Player*>(victim);

        MANGOS_ASSERT(he->duel);

        he->SetHealth(1);

        he->duel->opponent->CombatStopWithPets(true);
        he->CombatStopWithPets(true);

        he->CastSpell(he, 7267, TRIGGERED_OLD_TRIGGERED);                  // beg
        he->DuelComplete(DUEL_WON);
    }
}

void Unit::InterruptOrDelaySpell(Unit* pVictim, DamageEffectType damagetype)
{
    if (damagetype == NODAMAGE || damagetype == DOT)
        return;

    for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; ++i)
    {
        if (Spell* spell = pVictim->GetCurrentSpell(CurrentSpellTypes(i)))
        {
            if (spell->getState() == SPELL_STATE_CASTING)
            {
                if (spell->m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_ABORT_ON_DMG)
                    pVictim->InterruptSpell(CurrentSpellTypes(i));
                else
                    spell->Delayed();
            }

            if (CurrentSpellTypes(i) == CURRENT_CHANNELED_SPELL)
            {
                spell = pVictim->GetCurrentSpell(CURRENT_CHANNELED_SPELL); // fetch again because spell couldve been interrupted before
                if (spell)
                {
                    if (spell->CanBeInterrupted())
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

                        break;
                    }
                }
            }
        }
    }
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

void Unit::JustKilledCreature(Unit* killer, Creature* victim, Player* responsiblePlayer)
{
    victim->m_deathState = DEAD; // so that IsAlive, IsDead return expected results in the called hooks of JustKilledCreature
    // must be used only shortly before SetDeathState(JUST_DIED) and only for Creatures or Pets

    // some critters required for quests (need normal entry instead possible heroic in any cases)
    if (killer && victim->GetCreatureType() == CREATURE_TYPE_CRITTER && killer->GetTypeId() == TYPEID_PLAYER)
    {
        if (CreatureInfo const* normalInfo = ObjectMgr::GetCreatureTemplate(victim->GetEntry()))
            static_cast<Player*>(killer)->KilledMonster(normalInfo, victim->GetObjectGuid());
    }

    // Interrupt channeling spell when a Possessed Summoned is killed
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(victim->GetUInt32Value(UNIT_CREATED_BY_SPELL));
    if (spellInfo && spellInfo->HasAttribute(SPELL_ATTR_EX_FARSIGHT) && spellInfo->HasAttribute(SPELL_ATTR_EX_CHANNELED_1))
    {
        Unit* creator = victim->GetMap()->GetUnit(victim->GetCreatorGuid());
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
        victim->AI()->JustDied(killer);

    // Inform Owner
    Unit* pOwner = victim->GetMaster();
    if (victim->IsTemporarySummon())
    {
        if (victim->GetSpawnerGuid().IsCreature())
            if (Creature* pSummoner = victim->GetMap()->GetCreature(victim->GetSpawnerGuid()))
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
    if (OutdoorPvP* outdoorPvP = sOutdoorPvPMgr.GetScript(responsiblePlayer ? responsiblePlayer->GetCachedZoneId() : victim->GetZoneId()))
        outdoorPvP->HandleCreatureDeath(victim);

    // Start creature death script
    victim->GetMap()->ScriptsStart(sCreatureDeathScripts, victim->GetEntry(), victim, responsiblePlayer ? responsiblePlayer : killer);

    if (victim->IsLinkingEventTrigger())
        victim->GetMap()->GetCreatureLinkingHolder()->DoCreatureLinkingEvent(LINKING_EVENT_DIE, victim);

    // Dungeon specific stuff
    if (victim->GetInstanceId())
    {
        Map* map = victim->GetMap();
        if (map->IsDungeon())
        {
            Player* creditedPlayer = killer ? killer->GetBeneficiaryPlayer() : nullptr; // TODO: do instance binding anyway if the charmer/owner is offline
            if (map->IsRaid() && creditedPlayer)
            {
                if (victim->GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_INSTANCE_BIND)
                    static_cast<DungeonMap*>(map)->PermBindAllPlayers(creditedPlayer);
            }
            static_cast<DungeonMap*>(map)->GetPersistanceState()->UpdateEncounterState(ENCOUNTER_CREDIT_KILL_CREATURE, victim->GetEntry());
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
    CallForAllControlledUnits(PetOwnerKilledUnitHelper(pVictim), CONTROLLED_MINIPET | CONTROLLED_GUARDIANS | CONTROLLED_CHARM);
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

SpellCastResult Unit::CastSpell(SpellCastTargets& targets, SpellEntry const* spellInfo, uint32 triggeredFlags, Item* castItem, Aura* triggeredByAura, ObjectGuid originalCaster, SpellEntry const* triggeredBy)
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

    spell->m_CastItem = castItem;
    return spell->SpellStart(&targets, triggeredByAura);
}

SpellCastResult Unit::CastCustomSpell(SpellCastTargets& targets, SpellEntry const* spellInfo, int32 const* bp0, int32 const* bp1, int32 const* bp2, uint32 triggeredFlags, Item* castItem, Aura* triggeredByAura, ObjectGuid originalCaster, SpellEntry const* triggeredBy)
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

    if (bp0)
        spell->m_currentBasePoints[EFFECT_INDEX_0] = *bp0;

    if (bp1)
        spell->m_currentBasePoints[EFFECT_INDEX_1] = *bp1;

    if (bp2)
        spell->m_currentBasePoints[EFFECT_INDEX_2] = *bp2;

    spell->m_CastItem = castItem;
    return spell->SpellStart(&targets, triggeredByAura);
}

// Obsolete func need remove, here only for comotability vs another patches
uint32 Unit::SpellNonMeleeDamageLog(Unit* pVictim, uint32 spellID, uint32 damage)
{
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellID);
    SpellNonMeleeDamage spellDamageInfo(this, pVictim, spellInfo->Id, SpellSchools(spellInfo->School));
    CalculateSpellDamage(&spellDamageInfo, damage, spellInfo);
    spellDamageInfo.target->CalculateAbsorbResistBlock(this, &spellDamageInfo, spellInfo);
    Unit::DealDamageMods(this, spellDamageInfo.target, spellDamageInfo.damage, &spellDamageInfo.absorb, SPELL_DIRECT_DAMAGE);
    SendSpellNonMeleeDamageLog(&spellDamageInfo);
    DealSpellDamage(&spellDamageInfo, true);
    return spellDamageInfo.damage;
}

void Unit::CalculateSpellDamage(SpellNonMeleeDamage* spellDamageInfo, int32 damage, SpellEntry const* spellInfo, WeaponAttackType attackType)
{
    SpellSchoolMask damageSchoolMask = GetSchoolMask(spellDamageInfo->school);
    Unit* pVictim = spellDamageInfo->target;

    if (damage < 0)
        return;

    if (!pVictim)
        return;

    if (spellInfo->HasAttribute(SPELL_ATTR_EX3_NO_DONE_BONUS))
    {
        spellDamageInfo->damage = damage;
        return;
    }

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
            damage = MeleeDamageBonusDone(pVictim, damage, attackType, damageSchoolMask, spellInfo, SPELL_DIRECT_DAMAGE);
            damage = pVictim->MeleeDamageBonusTaken(this, damage, attackType, damageSchoolMask, spellInfo, SPELL_DIRECT_DAMAGE);
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
        spellDamageInfo->HitInfo |= SPELL_HIT_TYPE_CRIT;
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
    spellDamageInfo->damage = damage;
}

void Unit::DealSpellDamage(SpellNonMeleeDamage* spellDamageInfo, bool durabilityLoss)
{
    if (!spellDamageInfo)
        return;

    Unit* pVictim = spellDamageInfo->target;
    if (!pVictim)
        return;

    if (!pVictim->IsAlive() || pVictim->IsTaxiFlying() || pVictim->GetCombatManager().IsInEvadeMode())
        return;

    SpellEntry const* spellProto = sSpellTemplate.LookupEntry<SpellEntry>(spellDamageInfo->SpellID);
    if (spellProto == nullptr)
    {
        sLog.outError("Unit::DealSpellDamage have wrong damageInfo->SpellID: %u", spellDamageInfo->SpellID);
        return;
    }

    // Call default DealDamage (send critical in hit info for threat calculation)
    CleanDamage cleanDamage(spellDamageInfo->damage, BASE_ATTACK, spellDamageInfo->HitInfo & SPELL_HIT_TYPE_CRIT ? MELEE_HIT_CRIT : MELEE_HIT_NORMAL);
    DealDamage(this, pVictim, spellDamageInfo->damage, &cleanDamage, SPELL_DIRECT_DAMAGE, GetSchoolMask(spellDamageInfo->school), spellProto, durabilityLoss);
}

// TODO for melee need create structure as in
void Unit::CalculateMeleeDamage(Unit* pVictim, CalcDamageInfo* calcDamageInfo, WeaponAttackType attackType /*= BASE_ATTACK*/)
{
    calcDamageInfo->attacker         = this;
    calcDamageInfo->target           = pVictim;
    calcDamageInfo->attackType       = attackType;
    calcDamageInfo->totalDamage      = 0;
    calcDamageInfo->cleanDamage      = 0;
    calcDamageInfo->blocked_amount   = 0;

    calcDamageInfo->TargetState      = VICTIMSTATE_UNAFFECTED;
    calcDamageInfo->HitInfo          = HITINFO_NORMALSWING;
    calcDamageInfo->procAttacker     = PROC_FLAG_NONE;
    calcDamageInfo->procVictim       = PROC_FLAG_NONE;
    calcDamageInfo->procEx           = PROC_EX_NONE;
    calcDamageInfo->hitOutCome       = MELEE_HIT_EVADE;

    if (!pVictim)
        return;
    if (!this->IsAlive() || !pVictim->IsAlive())
        return;

    // Select HitInfo/procAttacker/procVictim flag based on attack type
    switch (attackType)
    {
        case BASE_ATTACK:
            calcDamageInfo->procAttacker = PROC_FLAG_SUCCESSFUL_MELEE_HIT;
            calcDamageInfo->procVictim   = PROC_FLAG_TAKEN_MELEE_HIT;
            calcDamageInfo->HitInfo      = HITINFO_NORMALSWING2;
            break;
        case OFF_ATTACK:
            calcDamageInfo->procAttacker = PROC_FLAG_SUCCESSFUL_MELEE_HIT | PROC_FLAG_SUCCESSFUL_OFFHAND_HIT;
            calcDamageInfo->procVictim   = PROC_FLAG_TAKEN_MELEE_HIT;//|PROC_FLAG_TAKEN_OFFHAND_HIT // not used
            calcDamageInfo->HitInfo = HITINFO_LEFTSWING;
            break;
        case RANGED_ATTACK:
            calcDamageInfo->procAttacker = PROC_FLAG_SUCCESSFUL_RANGED_HIT;
            calcDamageInfo->procVictim   = PROC_FLAG_TAKEN_RANGED_HIT;
            calcDamageInfo->HitInfo = HITINFO_UNK3;             // test (dev note: test what? HitInfo flag possibly not confirmed.)
            break;
        default:
            break;
    }

    bool immune = true;

    for (uint8 i = 0; i < m_weaponDamageInfo.weapon[calcDamageInfo->attackType].lines; i++)
    {
        SubDamageInfo* subDamage = &calcDamageInfo->subDamage[i];

        subDamage->damageSchoolMask = GetTypeId() == TYPEID_PLAYER
                                      ? GetSchoolMask(GetWeaponDamageSchool(calcDamageInfo->attackType, i))
                                      : GetMeleeDamageSchoolMask();

        if (calcDamageInfo->target->IsImmuneToDamage(subDamage->damageSchoolMask))
        {
            subDamage->damage = 0;
            continue;
        }
        else
            immune = false;

        subDamage->damage = CalculateDamage(calcDamageInfo->attackType, false, i);
        // Add melee damage bonus
        subDamage->damage = MeleeDamageBonusDone(calcDamageInfo->target, subDamage->damage, calcDamageInfo->attackType, subDamage->damageSchoolMask, nullptr, DIRECT_DAMAGE, 1, i == 0);
        subDamage->damage = calcDamageInfo->target->MeleeDamageBonusTaken(this, subDamage->damage, calcDamageInfo->attackType, subDamage->damageSchoolMask, nullptr, DIRECT_DAMAGE, 1, i == 0);

        // Calculate armor reduction
        if (subDamage->damageSchoolMask == SPELL_SCHOOL_MASK_NORMAL)
            subDamage->damage = CalcArmorReducedDamage(calcDamageInfo->target, subDamage->damage);

        calcDamageInfo->totalDamage += subDamage->damage;
    }

    // Physical Immune check
    if (immune)
    {
        calcDamageInfo->HitInfo |= HITINFO_NORMALSWING;
        calcDamageInfo->TargetState = VICTIMSTATE_IS_IMMUNE;

        calcDamageInfo->procEx |= PROC_EX_IMMUNE;
        calcDamageInfo->totalDamage = 0;
        calcDamageInfo->cleanDamage = 0;
        return;
    }

    calcDamageInfo->cleanDamage = calcDamageInfo->totalDamage;

    // FIXME: Fix individual school results later when appropriate API is ready
    uint32 mask = SPELL_SCHOOL_MASK_NORMAL;
    for (uint8 i = 0; i < m_weaponDamageInfo.weapon[calcDamageInfo->attackType].lines; ++i)
    {
        SubDamageInfo& subDamage = calcDamageInfo->subDamage[i];
        mask |= subDamage.damageSchoolMask;
    }
    calcDamageInfo->hitOutCome = RollMeleeOutcomeAgainst(calcDamageInfo->target, calcDamageInfo->attackType, SpellSchoolMask(mask));

    // Disable parry or dodge for ranged attack
    if (calcDamageInfo->attackType == RANGED_ATTACK)
    {
        if (calcDamageInfo->hitOutCome == MELEE_HIT_PARRY) calcDamageInfo->hitOutCome = MELEE_HIT_NORMAL;
        if (calcDamageInfo->hitOutCome == MELEE_HIT_DODGE) calcDamageInfo->hitOutCome = MELEE_HIT_MISS;
    }

    switch (calcDamageInfo->hitOutCome)
    {
        case MELEE_HIT_EVADE:
        {
            calcDamageInfo->HitInfo |= HITINFO_MISS | HITINFO_SWINGNOHITSOUND;
            calcDamageInfo->TargetState = VICTIMSTATE_EVADES;

            calcDamageInfo->procEx |= PROC_EX_EVADE;
            calcDamageInfo->totalDamage = 0;
            calcDamageInfo->cleanDamage = 0;

            for (uint8 i = 0; i < m_weaponDamageInfo.weapon[calcDamageInfo->attackType].lines; i++)
                calcDamageInfo->subDamage[i].damage = 0;

            return;
        }
        case MELEE_HIT_MISS:
        {
            calcDamageInfo->HitInfo |= HITINFO_MISS;
            calcDamageInfo->TargetState = VICTIMSTATE_UNAFFECTED;

            calcDamageInfo->procEx |= PROC_EX_MISS;
            calcDamageInfo->totalDamage = 0;
            calcDamageInfo->cleanDamage = 0;

            for (uint8 i = 0; i < m_weaponDamageInfo.weapon[calcDamageInfo->attackType].lines; i++)
                calcDamageInfo->subDamage[i].damage = 0;

            break;
        }
        case MELEE_HIT_NORMAL:
        {
            calcDamageInfo->TargetState = VICTIMSTATE_NORMAL;
            calcDamageInfo->procEx |= PROC_EX_NORMAL_HIT;
            break;
        }
        case MELEE_HIT_CRIT:
        {
            calcDamageInfo->HitInfo |= HITINFO_CRITICALHIT;
            calcDamageInfo->TargetState = VICTIMSTATE_NORMAL;
            calcDamageInfo->procEx |= PROC_EX_CRITICAL_HIT;
            calcDamageInfo->totalDamage = CalculateCritAmount(calcDamageInfo);
            break;
        }
        case MELEE_HIT_PARRY:
        {
            calcDamageInfo->TargetState = VICTIMSTATE_PARRY;
            calcDamageInfo->procEx |= PROC_EX_PARRY;
            calcDamageInfo->cleanDamage += calcDamageInfo->totalDamage;
            calcDamageInfo->totalDamage = 0;

            for (uint8 i = 0; i < m_weaponDamageInfo.weapon[calcDamageInfo->attackType].lines; i++)
                calcDamageInfo->subDamage[i].damage = 0;

            break;
        }
        case MELEE_HIT_DODGE:
        {
            calcDamageInfo->TargetState = VICTIMSTATE_DODGE;
            calcDamageInfo->procEx |= PROC_EX_DODGE;
            calcDamageInfo->cleanDamage += calcDamageInfo->totalDamage;
            calcDamageInfo->totalDamage = 0;

            for (uint8 i = 0; i < m_weaponDamageInfo.weapon[calcDamageInfo->attackType].lines; i++)
                calcDamageInfo->subDamage[i].damage = 0;

            break;
        }
        case MELEE_HIT_BLOCK:
        {
            calcDamageInfo->HitInfo |= HITINFO_BLOCK;
            calcDamageInfo->TargetState = VICTIMSTATE_NORMAL;
            calcDamageInfo->procEx |= PROC_EX_BLOCK;
            calcDamageInfo->blocked_amount = calcDamageInfo->target->GetShieldBlockValue();

            if (calcDamageInfo->blocked_amount >= calcDamageInfo->totalDamage)
            {
                // Full block
                calcDamageInfo->TargetState = VICTIMSTATE_BLOCKS;
                calcDamageInfo->blocked_amount = calcDamageInfo->totalDamage;

                for (uint8 i = 0; i < m_weaponDamageInfo.weapon[calcDamageInfo->attackType].lines; i++)
                    calcDamageInfo->subDamage[i].damage = 0;
            }
            else if (calcDamageInfo->blocked_amount)
            {
                // Partial block
                calcDamageInfo->procEx |= PROC_EX_NORMAL_HIT;

                auto amount = calcDamageInfo->blocked_amount;

                for (uint8 i = 0; i < m_weaponDamageInfo.weapon[calcDamageInfo->attackType].lines; i++)
                {
                    if (calcDamageInfo->subDamage[i].damage >= amount)
                    {
                        calcDamageInfo->subDamage[i].damage -= amount;
                        break;
                    }
                    else
                    {
                        amount -= calcDamageInfo->subDamage[i].damage;
                        calcDamageInfo->subDamage[i].damage = 0;
                    }
                }
            }

            calcDamageInfo->totalDamage -= calcDamageInfo->blocked_amount;
            calcDamageInfo->cleanDamage += calcDamageInfo->blocked_amount;

            break;
        }
        case MELEE_HIT_GLANCING:
        {
            calcDamageInfo->HitInfo |= HITINFO_GLANCING;
            calcDamageInfo->TargetState = VICTIMSTATE_NORMAL;
            calcDamageInfo->procEx |= PROC_EX_NORMAL_HIT;
            CalculateGlanceAmount(calcDamageInfo);
            break;
        }
        case MELEE_HIT_CRUSHING:
        {
            calcDamageInfo->HitInfo |= HITINFO_CRUSHING;
            calcDamageInfo->TargetState = VICTIMSTATE_NORMAL;
            calcDamageInfo->procEx |= PROC_EX_NORMAL_HIT;

            // 150% of normal damage
            calcDamageInfo->totalDamage += calcDamageInfo->totalDamage / 2;

            for (uint8 i = 0; i < m_weaponDamageInfo.weapon[calcDamageInfo->attackType].lines; i++)
                calcDamageInfo->subDamage[i].damage += calcDamageInfo->subDamage[i].damage / 2;

            break;
        }
        default:
            break;
    }

    // Calculate absorb resist
    if (int32(calcDamageInfo->totalDamage) > 0)
    {
        calcDamageInfo->procVictim |= PROC_FLAG_TAKEN_ANY_DAMAGE;

        // Calculate absorb & resists
        for (uint8 i = 0; i < m_weaponDamageInfo.weapon[calcDamageInfo->attackType].lines; i++)
        {
            SubDamageInfo* subDamage = &calcDamageInfo->subDamage[i];

            calcDamageInfo->target->CalculateDamageAbsorbAndResist(this, subDamage->damageSchoolMask, DIRECT_DAMAGE, subDamage->damage, &subDamage->absorb, &subDamage->resist, true);

            const uint32 bonus = (subDamage->resist < 0 ? uint32(std::abs(subDamage->resist)) : 0);
            subDamage->damage += bonus;
            calcDamageInfo->totalDamage += bonus;

            const uint32 malus = (subDamage->resist > 0 ? (subDamage->absorb + uint32(subDamage->resist)) : subDamage->absorb);

            if (subDamage->damage <= malus)
            {
                calcDamageInfo->totalDamage -= subDamage->damage;
                subDamage->damage = 0;
            }
            else
            {
                calcDamageInfo->totalDamage -= malus;
                subDamage->damage -= malus;
            }

            if (subDamage->absorb)
            {
                calcDamageInfo->cleanDamage = calcDamageInfo->cleanDamage <= subDamage->absorb ? 0 : calcDamageInfo->cleanDamage - subDamage->absorb;
                calcDamageInfo->HitInfo |= HITINFO_ABSORB;
                calcDamageInfo->procEx |= PROC_EX_ABSORB;
            }

            if (subDamage->resist)
            {
                calcDamageInfo->cleanDamage = int32(calcDamageInfo->cleanDamage) <= subDamage->resist ? 0 : calcDamageInfo->cleanDamage - subDamage->resist;
                calcDamageInfo->HitInfo |= HITINFO_RESIST;
            }
        }
    }
    else
        calcDamageInfo->totalDamage = 0;
}

void Unit::DealMeleeDamage(CalcDamageInfo* calcDamageInfo, bool durabilityLoss)
{
    if (!calcDamageInfo) return;
    Unit* victim = calcDamageInfo->target;
    if (!victim)
        return;

    if (!victim->IsAlive() || victim->IsTaxiFlying() || victim->GetCombatManager().IsInEvadeMode())
        return;

    // Hmmmm dont like this emotes client must by self do all animations
    if (calcDamageInfo->HitInfo & HITINFO_CRITICALHIT)
        victim->HandleEmoteCommand(EMOTE_ONESHOT_WOUNDCRITICAL);

    if (calcDamageInfo->TargetState == VICTIMSTATE_PARRY)
    {
        if (victim->GetTypeId() != TYPEID_UNIT ||
                !(((Creature*)victim)->GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_NO_PARRY_HASTEN))
        {
            // Get attack timers
            float offtime = float(victim->getAttackTimer(OFF_ATTACK));
            float basetime = float(victim->getAttackTimer(BASE_ATTACK));
            // Reduce attack time
            if (victim->hasOffhandWeaponForAttack() && offtime < basetime)
            {
                float percent20 = victim->GetAttackTime(OFF_ATTACK) * 0.20f;
                float percent60 = 3.0f * percent20;
                if (offtime > percent20 && offtime <= percent60)
                {
                    victim->setAttackTimer(OFF_ATTACK, uint32(percent20));
                }
                else if (offtime > percent60)
                {
                    offtime -= 2.0f * percent20;
                    victim->setAttackTimer(OFF_ATTACK, uint32(offtime));
                }
            }
            else
            {
                float percent20 = victim->GetAttackTime(BASE_ATTACK) * 0.20f;
                float percent60 = 3.0f * percent20;
                if (basetime > percent20 && basetime <= percent60)
                {
                    victim->setAttackTimer(BASE_ATTACK, uint32(percent20));
                }
                else if (basetime > percent60)
                {
                    basetime -= 2.0f * percent20;
                    victim->setAttackTimer(BASE_ATTACK, uint32(basetime));
                }
            }
        }
    }

    // Call default DealDamage
    CleanDamage cleanDamage(calcDamageInfo->cleanDamage, calcDamageInfo->attackType, calcDamageInfo->hitOutCome);
    DealDamage(this, victim, calcDamageInfo->totalDamage, &cleanDamage, DIRECT_DAMAGE, SpellSchoolMask(calcDamageInfo->subDamage[0].damageSchoolMask), nullptr, durabilityLoss);

    // If not immune
    if (calcDamageInfo->TargetState != VICTIMSTATE_IS_IMMUNE && calcDamageInfo->TargetState != VICTIMSTATE_EVADES)
    {
        if (CanEnterCombat() && victim->CanEnterCombat())
        {
            // if damage pVictim call AI reaction
            victim->AttackedBy(this);

            if (Unit* owner = GetOwner())
                if (owner->GetTypeId() == TYPEID_UNIT)
                    if (owner->CanJoinInAttacking(victim))
                        owner->EngageInCombatWithAggressor(victim);

            for (m_guardianPetsIterator = m_guardianPets.begin(); m_guardianPetsIterator != m_guardianPets.end();)
                if (Pet* guardian = GetMap()->GetPet(*(m_guardianPetsIterator++)))
                    if (guardian->CanJoinInAttacking(victim))
                        guardian->EngageInCombatWithAggressor(victim);
        }
    }

    if (calcDamageInfo->totalDamage)
    {
        // If this attack by an NPC dealt some damage from behind to a player, it has a chance to daze victim
        if (CanDazeInCombat(victim) && roll_chance_f(CalculateEffectiveDazeChance(victim, calcDamageInfo->attackType)))
            CastSpell(victim, 1604, TRIGGERED_OLD_TRIGGERED);

        // update at damage Judgement aura duration that applied by attacker at victim
        if (GetTypeId() == TYPEID_PLAYER)
        {
            SpellAuraHolderMap const& vAuras = victim->GetSpellAuraHolderMap();
            for (SpellAuraHolderMap::const_iterator itr = vAuras.begin(); itr != vAuras.end(); ++itr)
            {
                SpellEntry const* spellInfo = (*itr).second->GetSpellProto();
                if (spellInfo->HasAttribute(SPELL_ATTR_EX3_CANT_MISS) && spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN && ((*itr).second->GetCasterGuid() == GetObjectGuid()))
                    (*itr).second->RefreshHolder(); // TODO: Judgement - make this into a cast which should consume grounding totem
            }
        }

        // victim's damage shield
        std::set<Aura*> alreadyDone;
        AuraList const& vDamageShields = victim->GetAurasByType(SPELL_AURA_DAMAGE_SHIELD);
        for (AuraList::const_iterator i = vDamageShields.begin(); i != vDamageShields.end();)
        {
            if (alreadyDone.find(*i) == alreadyDone.end())
            {
                alreadyDone.insert(*i);

                SpellEntry const* spellProto = (*i)->GetSpellProto();

                // Damage shield can be resisted...
                if (SpellMissInfo missInfo = victim->SpellHitResult(this, spellProto, uint8(1 << (*i)->GetEffIndex()), false))
                {
                    victim->SendSpellMiss(this, spellProto->Id, missInfo);
                    continue;
                }

                // ...or immuned
                if (IsImmuneToDamage(GetSpellSchoolMask(spellProto)))
                {
                    Unit::SendSpellOrDamageImmune(victim->GetObjectGuid(), this, spellProto->Id);
                    continue;
                }

                uint32 damage = (*i)->GetModifier()->m_amount;

                if (victim->GetTypeId() == TYPEID_PLAYER)
                    ((Player*)victim)->ApplySpellMod(spellProto->Id, SPELLMOD_DAMAGE, damage);

                // Calculate absorb resist ??? no data in opcode for this possibly unable to absorb or resist?
                // uint32 absorb;
                // uint32 resist;
                // CalcAbsorbResist(pVictim, SpellSchools(spellProto->School), SPELL_DIRECT_DAMAGE, damage, &absorb, &resist);
                // damage-=absorb + resist;

                Unit::DealDamageMods(victim, this, damage, nullptr, SPELL_DIRECT_DAMAGE, spellProto);

                WorldPacket data(SMSG_SPELLDAMAGESHIELD, (8 + 8 + 4 + 4));
                data << victim->GetObjectGuid();
                data << GetObjectGuid();
                data << uint32(damage);                  // Damage
                data << uint32(spellProto->School);
                victim->SendMessageToSet(data, true);

                DealDamage(victim, this, damage, nullptr, SPELL_DIRECT_DAMAGE, GetSpellSchoolMask(spellProto), spellProto, true);

                i = vDamageShields.begin();
            }
            else
                ++i;
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

float Unit::CalcArmorReducedDamage(Unit* pVictim, const float damage)
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

    float newdamage = damage - (damage * tmpvalue);

    return (newdamage > 1) ? newdamage : 1.0f;
}

void Unit::CalculateDamageAbsorbAndResist(Unit* caster, SpellSchoolMask schoolMask, DamageEffectType damagetype, const uint32 damage, uint32* absorb, int32* resist, bool canReflect, bool canResist, bool binary)
{
    if (!IsAlive() || !damage)
        return;

    int32 RemainingDamage = int32(damage);

    // Magic damage, check for resists
    if (canResist && (schoolMask & SPELL_SCHOOL_MASK_MAGIC) && (!binary || damagetype == DOT))
    {
        const float multiplier = Unit::RollMagicResistanceMultiplierOutcomeAgainst(caster, this, schoolMask, damagetype, binary);
        *resist = int32(int64(damage) * multiplier);
        RemainingDamage -= *resist;
    }
    else
        *resist = 0;

    // Reflect damage spells (not cast any damage spell in aura lookup)
    uint32 reflectSpell = 0;
    int32  reflectDamage = 0;
    Aura*  reflectTriggeredBy = nullptr;                       // expected as not expired at reflect as in current cases
    // Death Prevention Aura
    Aura* preventDeathAura = nullptr;

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

        bool preventedDeath = false;
        (*i)->OnAbsorb(currentAbsorb, reflectSpell, reflectDamage, preventedDeath);
        if (preventedDeath)
            preventDeathAura = (*i);

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

    // Cast back reflect damage spell if caster exists
    if (canReflect && reflectSpell && caster)
    {
        CastCustomSpell(caster, reflectSpell, &reflectDamage, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED, nullptr, reflectTriggeredBy);
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

        (*i)->OnManaAbsorb(currentAbsorb);

        (*i)->GetModifier()->m_amount -= currentAbsorb;
        if ((*i)->GetModifier()->m_amount <= 0)
        {
            RemoveAurasDueToSpell((*i)->GetId());
            next = vManaShield.begin();
        }

        RemainingDamage -= currentAbsorb;
    }

    // only split damage if not damaging yourself
    if (caster && caster != this)
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
            if (!caster || caster == this || !caster->IsInWorld() || !caster->IsAlive())
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
            Unit::DealDamageMods(caster, caster, splitted, &splitted_absorb, DIRECT_DAMAGE, (*i)->GetSpellProto());

            caster->SendSpellNonMeleeDamageLog(caster, (*i)->GetSpellProto()->Id, splitted, schoolMask, splitted_absorb, 0, (damagetype == DOT), 0, false, true);

            CleanDamage cleanDamage = CleanDamage(splitted, BASE_ATTACK, MELEE_HIT_NORMAL);
            DealDamage(caster, caster, splitted, &cleanDamage, DIRECT_DAMAGE, schoolMask, (*i)->GetSpellProto(), false);
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
            if (!caster || caster == this || !caster->IsInWorld() || !caster->IsAlive())
                continue;

            uint32 splitted = uint32(RemainingDamage * (*i)->GetModifier()->m_amount / 100.0f);

            RemainingDamage -=  int32(splitted);

            if (caster->IsImmuneToDamage(schoolMask))
            {
                SendSpellMiss(caster, (*i)->GetSpellProto()->Id, SPELL_MISS_IMMUNE);
                return;
            }

            uint32 split_absorb = 0;
            Unit::DealDamageMods(caster, caster, splitted, &split_absorb, DIRECT_DAMAGE, (*i)->GetSpellProto());

            caster->SendSpellNonMeleeDamageLog(caster, (*i)->GetSpellProto()->Id, splitted, schoolMask, split_absorb, 0, (damagetype == DOT), 0, false, true);

            CleanDamage cleanDamage = CleanDamage(splitted, BASE_ATTACK, MELEE_HIT_NORMAL);
            Unit::DealDamage(caster, caster, splitted, &cleanDamage, DIRECT_DAMAGE, schoolMask, (*i)->GetSpellProto(), false);
        }
    }

    // Apply death prevention spells effects
    if (preventDeathAura && RemainingDamage >= (int32)GetHealth())
    {
        preventDeathAura->OnAuraDeathPrevention(RemainingDamage);
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

    CalculateDamageAbsorbAndResist(pCaster, GetSpellSchoolMask(spellProto), SPELL_DIRECT_DAMAGE, damageInfo->damage, &damageInfo->absorb, &damageInfo->resist, IsReflectableSpell(spellProto), IsResistableSpell(spellProto), IsBinarySpell(*spellProto));

    const uint32 bonus = (damageInfo->resist < 0 ? uint32(std::abs(damageInfo->resist)) : 0);
    damageInfo->damage += bonus;
    const uint32 malus = (damageInfo->resist > 0 ? (damageInfo->absorb + uint32(damageInfo->resist)) : damageInfo->absorb);
    damageInfo->damage = (damageInfo->damage <= malus ? 0 : (damageInfo->damage - malus));
}

void Unit::AttackerStateUpdate(Unit* pVictim, WeaponAttackType attType, bool extra)
{
    if (hasUnitState(UNIT_STAT_CAN_NOT_REACT) || HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED))
        return;

    if (!pVictim->IsAlive())
        return;

    if (!extra && IsNonMeleeSpellCasted(false))
        return;

    if (attType == RANGED_ATTACK)
        return;                                             // ignore ranged case

    // melee attack spell casted at main hand attack only - but only if its not already being executed
    if (attType == BASE_ATTACK && m_currentSpells[CURRENT_MELEE_SPELL] && !m_currentSpells[CURRENT_MELEE_SPELL]->IsExecutedCurrently())
    {
        m_currentSpells[CURRENT_MELEE_SPELL]->cast();
        return;
    }

    RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_MELEE_ATTACK);
    CalcDamageInfo meleeDamageInfo;
    CalculateMeleeDamage(pVictim, &meleeDamageInfo, attType);

    // Send log damage message to client
    for (uint8 i = 0; i < m_weaponDamageInfo.weapon[attType].lines; i++)
    {
        meleeDamageInfo.totalDamage -= meleeDamageInfo.subDamage[i].damage;
        Unit::DealDamageMods(this, pVictim, meleeDamageInfo.subDamage[i].damage, &meleeDamageInfo.subDamage[i].absorb, DIRECT_DAMAGE);
        meleeDamageInfo.totalDamage += meleeDamageInfo.subDamage[i].damage;
    }

    SendAttackStateUpdate(&meleeDamageInfo);
    DealMeleeDamage(&meleeDamageInfo, true);
    ProcDamageAndSpell(ProcSystemArguments(this, meleeDamageInfo.target, meleeDamageInfo.procAttacker, meleeDamageInfo.procVictim, meleeDamageInfo.procEx, meleeDamageInfo.totalDamage, meleeDamageInfo.attackType));

    uint32 totalAbsorb = 0;
    uint32 totalResist = 0;

    for (uint8 i = 0; i < m_weaponDamageInfo.weapon[attType].lines; i++)
    {
        totalAbsorb += meleeDamageInfo.subDamage[i].absorb;
        totalResist += meleeDamageInfo.subDamage[i].resist;
    }

    if (GetTypeId() == TYPEID_PLAYER)
        DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "AttackerStateUpdate: (Player) %u attacked %u (TypeId: %u) for %u dmg, absorbed %u, blocked %u, resisted %u.",
                         GetGUIDLow(), pVictim->GetGUIDLow(), pVictim->GetTypeId(), meleeDamageInfo.totalDamage, totalAbsorb, meleeDamageInfo.blocked_amount, totalResist);
    else
        DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "AttackerStateUpdate: (NPC)    %u attacked %u (TypeId: %u) for %u dmg, absorbed %u, blocked %u, resisted %u.",
                         GetGUIDLow(), pVictim->GetGUIDLow(), pVictim->GetTypeId(), meleeDamageInfo.totalDamage, totalAbsorb, meleeDamageInfo.blocked_amount, totalResist);
}

void Unit::DoExtraAttacks(Unit* pVictim)
{
    m_extraAttacksExecuting = true;
    while (m_extraAttacks)
    {
        AttackerStateUpdate(pVictim, BASE_ATTACK, true);
        if (m_extraAttacks > 0)
            --m_extraAttacks;
    }
    m_extraAttacksExecuting = false;
}

MeleeHitOutcome Unit::RollMeleeOutcomeAgainst(const Unit* pVictim, WeaponAttackType attType, SpellSchoolMask schoolMask) const
{
    if (pVictim->GetCombatManager().IsInEvadeMode())
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
    DETAIL_FILTER_LOG(LOG_FILTER_COMBAT, "WORLD: Sent SMSG_ATTACKSTART");
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
    DETAIL_FILTER_LOG(LOG_FILTER_COMBAT, "%s stopped attacking %s", GetGuidStr().c_str(), victim->GetGuidStr().c_str());

    /*if(victim->GetTypeId() == TYPEID_UNIT)
    ((Creature*)victim)->AI().EnterEvadeMode(this);*/
}

SpellMissInfo Unit::MeleeSpellHitResult(Unit* pVictim, SpellEntry const* spell, uint32* heartbeatResistChance/* = nullptr*/)
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

    // Memorize heartbeat resist chance if needed:
    if (heartbeatResistChance && spell->HasAttribute(SPELL_ATTR_HEARTBEAT_RESIST_CHECK))
        *heartbeatResistChance = (die.chance[UNIT_COMBAT_DIE_RESIST] + die.chance[UNIT_COMBAT_DIE_MISS]);

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

SpellMissInfo Unit::MagicSpellHitResult(Unit* pVictim, SpellEntry const* spell, SpellSchoolMask schoolMask, uint32* heartbeatResistChance/* = nullptr*/)
{
    Die<UnitCombatDieSide, UNIT_COMBAT_DIE_HIT, NUM_UNIT_COMBAT_DIE_SIDES> die;
    die.set(UNIT_COMBAT_DIE_RESIST, CalculateSpellResistChance(pVictim, schoolMask, spell));
    /* Deflect for spells is currently unused up until WotLK, commented out for performance
    if (pVictim->CanDeflectAbility(this, spell))
       die.set(UNIT_COMBAT_DIE_DEFLECT, pVictim->CalculateAbilityDeflectChance(this, spell));
    */
    DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "MagicSpellHitResult: New spell hit die: %u [RESIST:%u, DEFLECT:%u]", spell->Id,
                     die.chance[UNIT_COMBAT_DIE_RESIST], die.chance[UNIT_COMBAT_DIE_DEFLECT]);

    // Memorize heartbeat resist chance if needed:
    if (heartbeatResistChance && spell->HasAttribute(SPELL_ATTR_HEARTBEAT_RESIST_CHECK))
        *heartbeatResistChance = (die.chance[UNIT_COMBAT_DIE_RESIST] + die.chance[UNIT_COMBAT_DIE_MISS]);

    const uint32 random = urand(1, 10000);
    const UnitCombatDieSide side = die.roll(random);
    if (side != UNIT_COMBAT_DIE_HIT)
        DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "MagicSpellHitResult: Rolled %u, result: %s (chance %u)", random, UnitCombatDieSideText(side), die.chance[side]);
    switch (uint32(side))
    {
        case UNIT_COMBAT_DIE_RESIST:    return SPELL_MISS_RESIST;   // Pre-WotLK: magic resist and miss/hit are on the same die side
        case UNIT_COMBAT_DIE_DEFLECT:   return SPELL_MISS_DEFLECT;
    }
    DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "MagicSpellHitResult: Rolled %u, result: HIT", random);
    return SPELL_MISS_NONE;
}

SpellMissInfo Unit::SpellHitResult(Unit* pVictim, SpellEntry const* spell, uint8 effectMask, bool reflectable, bool reflected, uint32* heartbeatResistChance/* = nullptr*/)
{
    // Dead units can't be missed, can't resist, reflect, etc
    if (!pVictim->IsAlive())
        return SPELL_MISS_NONE;

    // Return evade for units in evade mode
    if (pVictim->GetCombatManager().IsInEvadeMode())
        return SPELL_MISS_EVADE;

    // All positive spells can`t miss
    // TODO: client not show miss log for this spells - so need find info for this in dbc and use it!
    if (IsPositiveEffectMask(spell, effectMask, this, pVictim))
    {
        if (pVictim->IsImmuneToSpell(spell, reflected ? false : (this == pVictim), effectMask))
            return SPELL_MISS_IMMUNE;

        return SPELL_MISS_NONE;
    }
    // !!!UNHACKME: Self-resists suppression hack for channeled spells until spell execution is modernized with effectmasks: Drain Soul, Blizzard, RoF
    if (pVictim == this && IsChanneledSpell(spell))
    {
        for (uint32 i = EFFECT_INDEX_0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (IsCasterSourceTarget(spell->EffectImplicitTargetA[i]) || IsCasterSourceTarget(spell->EffectImplicitTargetB[i]))
                return SPELL_MISS_NONE;
        }
    }
    // !!!
    SpellSchoolMask schoolMask = GetSpellSchoolMask(spell);
    // wand case
    bool wand = spell->Id == 5019;
    if (wand && !!(getClassMask() & CLASSMASK_WAND_USERS) && GetTypeId() == TYPEID_PLAYER)
        schoolMask = GetRangedDamageSchoolMask();

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
        if (!wand && pVictim->IsImmuneToSpell(spell, reflected ? false : (this == pVictim), effectMask))
            return SPELL_MISS_IMMUNE;
        // Check for immune to damage as hit result if spell hit composed entirely out of damage effects
        if (IsSpellEffectsDamage(*spell, effectMask) && pVictim->IsImmuneToDamage(schoolMask))
            return SPELL_MISS_IMMUNE;
    }
    switch (spell->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MELEE:
        case SPELL_DAMAGE_CLASS_RANGED:
            return MeleeSpellHitResult(pVictim, spell, heartbeatResistChance);
        case SPELL_DAMAGE_CLASS_MAGIC:
            return MagicSpellHitResult(pVictim, spell, schoolMask, heartbeatResistChance);
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
                       ? std::max(((Player*)this)->GetSkillMax(SKILL_DEFENSE), ((Player*)this)->GetSkillValue(SKILL_DEFENSE))
                       : ((Player*)this)->GetSkillValue(SKILL_DEFENSE);
        return value;
    }
    return GetUnitMeleeSkill(target);
}

bool Unit::CanCrush() const
{
    // Generally, only npcs and npc-controlled players/units are eligible to deal crushing blows
    if (GetTypeId() == TYPEID_PLAYER || HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        return GetCharmerGuid().IsCreature();
    return !GetMasterGuid().IsPlayer();
}

bool Unit::CanGlance() const
{
    // Generally, only players and player-controlled units are eligible to deal glancing blows
    if (GetTypeId() == TYPEID_PLAYER || HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        return !GetCharmerGuid().IsCreature();
    return false;
}

bool Unit::CanDaze() const
{
    // Generally, only npcs are able to daze targets in melee
    return (GetTypeId() == TYPEID_UNIT && !HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED));
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

bool Unit::CanDodgeInCombat(const Unit* attacker) const
{
    if (!attacker || !CanDodgeInCombat() || IsNonMeleeSpellCasted(false))
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

bool Unit::CanParryInCombat(const Unit* attacker) const
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

bool Unit::CanBlockInCombat(const Unit* attacker, SpellSchoolMask weaponSchoolMask) const
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

bool Unit::CanCrushInCombat(const Unit* victim) const
{
    return (victim && CanCrushInCombat() && victim->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED));
}

bool Unit::CanGlanceInCombat(const Unit* victim) const
{
    return (victim && CanGlanceInCombat() && !victim->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && victim->GetLevelForTarget(this) > 10);
}

bool Unit::CanDazeInCombat(const Unit* victim) const
{
    return (victim && CanDazeInCombat() && (victim->GetTypeId() == TYPEID_PLAYER) && IsFacingTargetsBack(victim));
}

bool Unit::CanReactOnAbility(const SpellEntry* ability) const
{
    return (ability && !ability->HasAttribute(SPELL_ATTR_IMPOSSIBLE_DODGE_PARRY_BLOCK));
}

bool Unit::CanDodgeAbility(const Unit* attacker, const SpellEntry* ability) const
{
    // Can't dodge unavoidable and ranged attacks
    if (!CanReactOnAbility(ability) || ability->DmgClass != SPELL_DAMAGE_CLASS_MELEE)
        return false;
    // Check if attacker can be dodged at the moment
    if (!CanDodgeInCombat(attacker))
        return false;
    return true;
}

bool Unit::CanParryAbility(const Unit* attacker, const SpellEntry* ability) const
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

bool Unit::CanBlockAbility(const Unit* attacker, const SpellEntry* ability, bool miss) const
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

bool Unit::CanDeflectAbility(const Unit* attacker, const SpellEntry* ability) const
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

float Unit::CalculateEffectiveDodgeChance(const Unit* attacker, WeaponAttackType attType, const SpellEntry* ability) const
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
    const bool isPlayerOrPet = HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    const uint32 skill = (weapon ? attacker->GetWeaponSkillValue(attType, this) : attacker->GetSkillMaxForLevel(this));
    int32 difference = int32(GetDefenseSkillValue(attacker) - skill);
    // Defense/weapon skill factor: for players and NPCs
    float factor = 0.04f;
    // NPCs gain additional bonus dodge chance based on positive skill difference
    if (!isPlayerOrPet && difference > 0)
        factor = 0.1f;
    chance += (difference * factor);
    return std::max(0.0f, std::min(chance, 100.0f));
}

float Unit::CalculateEffectiveParryChance(const Unit* attacker, WeaponAttackType attType, const SpellEntry* ability) const
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
    const bool isPlayerOrPet = HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    const uint32 skill = (weapon ? attacker->GetWeaponSkillValue(attType, this) : attacker->GetSkillMaxForLevel(this));
    int32 difference = int32(GetDefenseSkillValue(attacker) - skill);
    // Defense/weapon skill factor: for players and NPCs
    float factor = 0.04f;
    // NPCs gain additional bonus parry chance based on positive skill difference (same value as bonus miss rate)
    if (!isPlayerOrPet && difference > 0)
    {
        if (difference > 10)
            factor = 0.6f; // Pre-WotLK: 0.2 additional factor for each level above 2
        else
            factor = 0.1f;
    }
    chance += (difference * factor);
    return std::max(0.0f, std::min(chance, 100.0f));
}

float Unit::CalculateEffectiveBlockChance(const Unit* attacker, WeaponAttackType attType, const SpellEntry* ability) const
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
    const bool isPlayerOrPet = HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    const uint32 skill = (weapon ? attacker->GetWeaponSkillValue(attType, this) : attacker->GetSkillMaxForLevel(this));
    const int32 difference = int32(GetDefenseSkillValue(attacker) - skill);
    // Defense/weapon skill factor: for players and NPCs
    float factor = 0.04f;
    // NPCs cannot gain bonus block chance based on positive skill difference
    if (!isPlayerOrPet && difference > 0)
        factor = 0.0f;
    chance += (difference * factor);
    return std::max(0.0f, std::min(chance, 100.0f));
}

float Unit::CalculateEffectiveCrushChance(const Unit* victim, WeaponAttackType attType) const
{
    float chance = 0.0f;

    // Crushing blow chance is present when attacker's weapon skill is >= 15 over victim's own capped defense skill
    // The chance starts at 15% and increased by 2% per each additional skill point of defense deficit
    const uint32 cap = victim->GetSkillMaxForLevel();
    const uint32 defense = std::min(victim->GetDefenseSkillValue(this), cap);
    const int32 deficit = (int32(GetWeaponSkillValue(attType, victim)) - int32(defense));
    if (deficit >= 15)
        chance += ((2 * deficit) - 15);
    return std::max(0.0f, std::min(chance, 100.0f));
}

float Unit::CalculateEffectiveGlanceChance(const Unit* victim, WeaponAttackType attType) const
{
    float chance = 0.0f;

    // Glancing blow chance starts at 2% when victim's defense skill is > 10 lower than attacker's own capped weapon skill
    // The chance is increased for each point of skill diffrernce
    const uint32 cap = GetSkillMaxForLevel();
    const uint32 skill = std::min(GetWeaponSkillValue(attType, victim), cap);
    const int32 level = int32(GetLevelForTarget(victim));
    const uint32 defense = victim->GetDefenseSkillValue(this);
    // Penalty for caster classes in Classic
    if (GetTypeId() == TYPEID_PLAYER && (getClassMask() & CLASSMASK_WAND_USERS))
        chance += (level < 30) ? (level + (int32(defense - skill) * 2)) : (30 + (int32(defense - skill) * 2));
    else
        chance += (10 + (int32(defense - skill) * 2));
    return std::max(0.0f, std::min(chance, 100.0f));
}

float Unit::CalculateEffectiveDazeChance(const Unit* victim, WeaponAttackType attType) const
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

uint32 Unit::CalculateGlanceAmount(CalcDamageInfo* meleeInfo) const
{
    if (!meleeInfo)
        return 0;
    // Pre-2.0.1: uncapped skill value
    const uint32 skill = GetWeaponSkillValue(meleeInfo->attackType, meleeInfo->target);
    const uint32 defense = meleeInfo->target->GetDefenseSkillValue(this);
    const int32 difference = int32(defense - skill);
    // Attacker's skill beats victim's defense: no action required
    if (difference < 0)
        return meleeInfo->totalDamage;
    // calculate base values and mods
    const bool caster = (getClassMask() & CLASSMASK_WAND_USERS) != 0;
    const float baseHighEnd = (caster ? 0.9f : 1.2f);
    const float baseLowEnd = (caster ? 0.6f : 1.3f);
    const float maxLowEnd = (caster ? 0.6f : 0.91f);

    float highEnd = baseHighEnd - (0.03f * difference);
    float lowEnd = baseLowEnd - (0.05f * difference);
    // 0.2f <= highEnd <= 0.99f
    highEnd = std::min(std::max(highEnd, 0.2f), 0.99f);
    // 0.01f <= lowEnd <= maxLowEnd <= highEnd
    lowEnd = std::min(std::max(lowEnd, 0.01f), std::min(maxLowEnd, highEnd));
    // Roll for final glance damage multiplier and calculate amount of damage glancing blow will deal
    const float multiplier = (urand(uint32(lowEnd * 100), uint32(highEnd * 100)) * 0.01f);
    const uint32 result = uint32(meleeInfo->totalDamage * multiplier);
    meleeInfo->cleanDamage += (meleeInfo->totalDamage - result);
    meleeInfo->totalDamage = result;
    for (uint8 i = 0; i < m_weaponDamageInfo.weapon[meleeInfo->attackType].lines; i++)
        meleeInfo->subDamage[i].damage = uint32(meleeInfo->subDamage[i].damage * multiplier);
    return result;
}

float Unit::CalculateAbilityDeflectChance(const Unit* attacker, const SpellEntry* ability) const
{
    switch (ability->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MELEE:
        case SPELL_DAMAGE_CLASS_RANGED: return CalculateEffectiveParryChance(attacker, GetWeaponAttackType(ability), ability);
        case SPELL_DAMAGE_CLASS_MAGIC:  return CalculateEffectiveParryChance(attacker, RANGED_ATTACK, ability);
    }
    return 0.0f;
}

bool Unit::RollAbilityPartialBlockOutcome(const Unit* attacker, WeaponAttackType attType, const SpellEntry* ability) const
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

bool Unit::CanCrit(const SpellEntry* entry, SpellSchoolMask schoolMask, WeaponAttackType attType) const
{
    // Only players, player pets and totems can crit with spells
    if (!HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        return false;
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

float Unit::GetCritChance(const SpellEntry* entry, SpellSchoolMask schoolMask) const
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

float Unit::GetCritMultiplier(SpellSchoolMask dmgSchoolMask, uint32 creatureTypeMask, const SpellEntry* spell, bool heal) const
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

uint32 Unit::CalculateCritAmount(const Unit* victim, uint32 amount, const SpellEntry* entry, bool heal) const
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

uint32 Unit::CalculateCritAmount(CalcDamageInfo* meleeInfo) const
{
    if (!meleeInfo)
        return 0;
    uint32 clean = 0;
    uint32 total = 0;
    const Unit* victim = meleeInfo->target;
    const uint32 creatureTypeMask = (victim ? victim->GetCreatureTypeMask() : 0);
    const SpellDmgClass dmgClass = ((meleeInfo->attackType == RANGED_ATTACK) ? SPELL_DAMAGE_CLASS_RANGED : SPELL_DAMAGE_CLASS_MELEE);
    const float ignoreReduction = 0.0f;
    for (uint8 i = 0; i < m_weaponDamageInfo.weapon[meleeInfo->attackType].lines; i++)
    {
        SubDamageInfo& subDamage = meleeInfo->subDamage[i];
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

float Unit::GetHitChance(const SpellEntry* entry, SpellSchoolMask schoolMask) const
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

float Unit::GetMissChance(const SpellEntry* entry, SpellSchoolMask schoolMask) const
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

float Unit::CalculateEffectiveCritChance(const Unit* victim, WeaponAttackType attType, const SpellEntry* ability) const
{
    float chance = 0.0f;
    chance += (ability ? GetCritChance(ability, SPELL_SCHOOL_MASK_NORMAL) : GetCritChance(attType));
    // Own chance appears to be zero / below zero / unmeaningful for some reason (debuffs?): skip calculation, unit is incapable
    if (chance < 0.005f)
        return 0.0f;
    // Skip victim calculation if positive ability
    if (ability && IsPositiveSpell(ability, this, victim))
        return std::max(0.0f, std::min(chance, 100.0f));
    const bool weapon = (!ability || ability->EquippedItemClass == ITEM_CLASS_WEAPON);
    // Skill difference can be both negative and positive.
    // a) Positive means that attacker's level is higher or additional weapon +skill bonuses
    // b) Negative means that victim's level is higher or additional +defense bonuses
    const bool vsPlayerOrPet = victim->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    const bool ranged = (attType == RANGED_ATTACK);
    // weapon skill does not benefit crit% vs NPCs
    const uint32 skill = (weapon && !vsPlayerOrPet ? GetWeaponSkillValue(attType, victim) : GetSkillMaxForLevel(victim));
    const int32 difference = int32(skill - victim->GetDefenseSkillValue(this));
    // Weapon skill factor: for players and NPCs
    float factor = 0.04f;
    // Crit suppression against NPCs with higher level
    if (!vsPlayerOrPet) // decrease by 1% of crit per level of target
        factor = 0.2f;
    chance += (difference * factor);
    // Victim's crit taken chance
    const SpellDmgClass dmgClass = (ranged ? SPELL_DAMAGE_CLASS_RANGED : SPELL_DAMAGE_CLASS_MELEE);
    chance += victim->GetCritTakenChance(SPELL_SCHOOL_MASK_NORMAL, dmgClass);
    return std::max(0.0f, std::min(chance, 100.0f));
}

float Unit::CalculateEffectiveMissChance(const Unit* victim, WeaponAttackType attType, const SpellEntry* ability) const
{
    float chance = 0.0f;

    chance += (ability ? victim->GetMissChance(ability, SPELL_SCHOOL_MASK_NORMAL) : victim->GetMissChance(attType));
    // Victim's own chance appears to be zero / below zero / unmeaningful for some reason (debuffs?): skip calculation, unit can't be missed
    if (chance < 0.005f)
        return 0.0f;
    const bool ranged = (attType == RANGED_ATTACK);
    const bool weapon = (!ability || ability->EquippedItemClass == ITEM_CLASS_WEAPON);
    // Check if dual wielding, add additional miss penalty
    if (!ability && !ranged && hasOffhandWeaponForAttack())
        chance += 19.0f;
    // Skill difference can be both negative and positive. Positive difference means that:
    // a) Victim's level is higher
    // b) Victim has additional defense skill bonuses
    const bool vsPlayerOrPet = victim->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    const uint32 skill = (weapon ? GetWeaponSkillValue(attType, victim) : GetSkillMaxForLevel(victim));
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
    chance -= victim->GetTotalAuraModifier(ranged ? SPELL_AURA_MOD_ATTACKER_RANGED_HIT_CHANCE : SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE);
    // For elemental melee auto-attacks: full resist outcome converted into miss chance (original research on combat logs)
    if (!ranged && !ability)
    {
        const float percent = victim->CalculateEffectiveMagicResistancePercent(this, GetMeleeDamageSchoolMask((attType == BASE_ATTACK), true));
        if (percent > 0)
        {
            if (const uint32 uindex = uint32(percent * 100))
            {
                const SpellPartialResistChanceEntry &chances = SPELL_PARTIAL_RESIST_DISTRIBUTION.at(uindex);
                chance += float(chances.at(SPELL_PARTIAL_RESIST_PCT_100) / 100);
            }
        }
    }
    // Finally, take hit chance
    chance -= (ability ? GetHitChance(ability, SPELL_SCHOOL_MASK_NORMAL) : GetHitChance(attType));
    float minimum = (ability && ability->DmgClass == SPELL_DAMAGE_CLASS_MAGIC) || difference > 10 ? 1.f : 0;
    return std::max(minimum, std::min(chance, 100.0f));
}

float Unit::CalculateSpellCritChance(const Unit* victim, SpellSchoolMask schoolMask, const SpellEntry* spell) const
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
    const AuraList& scripts = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    for (auto script : scripts)
    {
        if (!(script->isAffectedOnSpell(spell)))
            continue;
        switch (script->GetModifier()->m_miscvalue)
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

float Unit::CalculateSpellMissChance(const Unit* victim, SpellSchoolMask schoolMask, const SpellEntry* spell) const
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
    const bool vsPlayerOrPet = victim->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
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

bool Unit::RollSpellCritOutcome(const Unit* victim, SpellSchoolMask schoolMask, const SpellEntry* spell) const
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

float Unit::CalculateEffectiveMagicResistancePercent(const Unit* attacker, SpellSchoolMask schoolMask, bool binary) const
{
    // Non-magical or contains Physical as a component (such as Chaos) - ignores resistances completely
    if (!schoolMask || (schoolMask & SPELL_SCHOOL_MASK_NORMAL))
        return 0.0f;
    // Completely ignore holy resistance value (since beta stages)
    uint32 schools = uint32(schoolMask & ~uint32(SPELL_SCHOOL_MASK_HOLY));
    // Select a resistance value matching given school mask, prefer mininal for multischool spells
    int32 resistance = 0;
    for (uint32 school = 0; schools; (schools >>= 1), ++school)
    {
        if (schools & 1)
        {
            int32 amount = GetResistance(SpellSchools(school));
            int32 penetration = attacker ? attacker->GetResistancePenetration(SpellSchools(school)) : 0;

            // Modify by penetration, but can't go negative with it
            int32 result = (amount - penetration);

            if (result < 0)
                result = std::min(amount, 0);

            if (!resistance || result < resistance)
                resistance = result;
        }
    }

    float percent = 0;

    if (resistance >= 0) // Magic resistance calculation
    {
        // Attacker's level based skill, penalize when calculating for low levels (< 20):
        const float skill = std::max(attacker ? attacker->GetSkillMaxForLevel(this) : GetSkillMaxForLevel(), uint16(100));
        // Convert resistance value to resistance percentage through comparision with skill
        percent += (float(resistance) / skill) * 100;
        // Pre-3.0: multiplied by 0.75
        percent *= 0.75f;
        // Bonus resistance percent by positive level difference when calculating damage hit for NPCs only
        if (!binary && GetTypeId() == TYPEID_UNIT && !HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
            percent += (0.4f * std::max(int32(GetSkillMaxForLevel(attacker) - skill), 0));
        // Magic resistance percentage cap (same as armor cap)
        percent = std::min(percent, 75.0f);
    }
    else                    // Magic vulnerability calculation
    {
        // Victim's level based skill, penalize when calculating for low levels (< 20):
        const float skill = std::max(GetSkillMaxForLevel(attacker), uint16(100));
        // Convert resistance value to vulnerability percentage through comparision with skill
        percent += (float(resistance) / skill) * 100;
    }
    return percent;
}

float Unit::RollMagicResistanceMultiplierOutcomeAgainst(const Unit* attacker, const Unit *victim, SpellSchoolMask schoolMask, DamageEffectType dmgType, bool binary)
{
    float percentage = victim->CalculateEffectiveMagicResistancePercent(attacker, schoolMask);

    // Magic vulnerability instead of magic resistance:
    if (percentage < 0)
        return (percentage * 0.01f);

    // Pre-TBC: reduced mitigation (10% of normal amount) against DoTs originating from binary spells
    percentage *= (dmgType == DOT && binary ? 0.1f : 1.0f);

    const uint32 index = uint32(percentage * 100);

    if (!index)
        return 0.0f;

    const SpellPartialResistChanceEntry &chances = SPELL_PARTIAL_RESIST_DISTRIBUTION.at(index);
    Die<SpellPartialResist, SPELL_PARTIAL_RESIST_NONE, NUM_SPELL_PARTIAL_RESISTS> die;

    for (uint8 outcome = SPELL_PARTIAL_RESIST_NONE; outcome < NUM_SPELL_PARTIAL_RESISTS; ++outcome)
        die.chance[outcome] = chances.at(outcome);

    const uint32 random = urand(1, 10000);
    // We must exclude full resist chance from it, we already rolled for it as miss type in attack table (so n-1)
    uint8 portion = std::min(uint8(die.roll(random)), uint8(NUM_SPELL_PARTIAL_RESISTS - 1));

    // Pre-3.0: simulate old retail rouding error (full damage hit cut-off) for:
    if (portion == SPELL_PARTIAL_RESIST_NONE && percentage > 54.0f)
    {
        // NPC non-binary spells, Environmental damage (e.g. lava), Elemental attacks
        if (!binary && (dmgType == DIRECT_DAMAGE || (attacker && !attacker->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))))
            ++portion;
    }

    // To get resisted part ratio, we exclude zero outcome (it is n-1 anyway, so we reuse local var)
    return (float(portion) / float(NUM_SPELL_PARTIAL_RESISTS));
}

float Unit::CalculateSpellResistChance(const Unit* victim, SpellSchoolMask schoolMask, const SpellEntry* spell) const
{
    float chance = 0.0f;

    // Pre-WotLK: magic resist and miss/hit are on the same die side
    if (spell->DmgClass == SPELL_DAMAGE_CLASS_MAGIC || spell->DmgClass == SPELL_DAMAGE_CLASS_NONE)
        chance += CalculateSpellMissChance(victim, schoolMask, spell);

    // Chance to fully resist a spell by magic resistance
    // TODO: check if binaries are affected by SPELL_ATTR_EX3_CANT_MISS
    if (IsResistableSpell(spell) && spell->DmgClass == SPELL_DAMAGE_CLASS_MAGIC)
    {
        const bool binary = IsBinarySpell(*spell);
        const float percent = victim->CalculateEffectiveMagicResistancePercent(this, schoolMask, binary);
        if (percent > 0)
        {
            if (binary)
                chance += percent;
            // Only non-binary spells used by players and pets can be fully resisted by magic resistance, NPC spells can be cut-off
            else if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
                chance += float(SPELL_PARTIAL_RESIST_DISTRIBUTION.at(uint32(percent * 100)).at(SPELL_PARTIAL_RESIST_PCT_100)) * 0.01f;
        }
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
            return GetSkillMaxForLevel();              // always maximized SKILL_FERAL_COMBAT in fact

        // weapon skill or (unarmed for base attack)
        uint32 skill = item ? item->GetSkill() : uint32(SKILL_UNARMED);

        // in PvP always use full skill (and bonuses on top of it if present)
        value = (target && target->GetTypeId() == TYPEID_PLAYER)
                ? std::max(((Player*)this)->GetSkillMax(skill), ((Player*)this)->GetSkillValue(skill))
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
    for (auto& m_currentSpell : m_currentSpells)
    {
        if (m_currentSpell && m_currentSpell->getState() == SPELL_STATE_FINISHED)
        {
            m_currentSpell->SetReferencedFromCurrent(false);
            m_currentSpell = nullptr;                      // remove pointer
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
}

void Unit::_UpdateAutoRepeatSpell()
{
    // check "real time" interrupts
    if (IsMoving() || IsNonMeleeSpellCasted(false, false, true))
    {
        if (!IsNonMeleeSpellCasted(false, false, true, false, true)) // stricter check to see if we should introduce cooldown or just return
            return;
        // cancel wand shoot
        if ((m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo->ChannelInterruptFlags & CHANNEL_FLAG_MOVEMENT) != 0)
        {
            InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
            return;
        }
        // set 0.5 second wind-up time.
        m_AutoRepeatFirstCast = true;
        return;
    }

    // apply delay
    if (m_AutoRepeatFirstCast)
    {
        TimePoint expirationTime;
        bool isPermanent;
        if (GetExpireTime(*m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo, expirationTime, isPermanent))
        {
            if (GetMap()->GetCurrentClockTime() + std::chrono::milliseconds(500) > expirationTime)
            {
                RemoveSpellCooldown(*m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo, false);
                AddCooldown(*m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo, nullptr, false, 500);
            }
        }
        else
            AddCooldown(*m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo, nullptr, false, 500);
        m_AutoRepeatFirstCast = false;
        return;
    }

    // cast routine
    if (IsSpellReady(*m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo))
    {
        // be sure the unit is stand up
        if (getStandState() != UNIT_STAND_STATE_STAND)
            SetStandState(UNIT_STAND_STATE_STAND);

        Unit* currSpellTarget = GetMap()->GetUnit(m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_targets.getUnitTargetGuid());
        Unit* currTarget = nullptr;

        // Check i there is new target
        ObjectGuid const& currTargetGuid = GetTargetGuid();
        if (!currTargetGuid.IsEmpty() && currTargetGuid.IsUnit())
        {
            if (!currSpellTarget || currTargetGuid != currSpellTarget->GetObjectGuid())
                currTarget = GetMap()->GetUnit(currTargetGuid);
            else
                currTarget = currSpellTarget;
        }

        // some check about new target are necessary
        if (!currTarget || !CanAttack(currTarget))
        {
            // no valid target
            InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
            return;
        }

        SpellCastTargets targets;
        targets.setUnitTarget(currTarget);

        // we want to shoot
        Spell* spell = new Spell(this, m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo, TRIGGERED_AUTOREPEAT);

        // Check if able to cast
        if (spell->SpellStart(&targets) != SPELL_CAST_OK)
        {
            InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
            return;
        }
    }
}

void Unit::SetCurrentCastedSpell(Spell* newSpell)
{
    MANGOS_ASSERT(newSpell);                                  // nullptr may be never passed here, use InterruptSpell or InterruptNonMeleeSpells

    CurrentSpellTypes CSpellType = newSpell->GetCurrentContainer();

    if (newSpell == m_currentSpells[CSpellType]) return;      // avoid breaking self

    // break same type spell if it is not delayed
    InterruptSpell(CSpellType, false);

    // special breakage effects:
    switch (CSpellType)
    {
        case CURRENT_GENERIC_SPELL:
        {
            // generic spells always break channeled not delayed spells
            // Unless they have this attribute
            if (Spell const* channeledSpell = m_currentSpells[CURRENT_CHANNELED_SPELL])
            {
                if (!channeledSpell->m_spellInfo->HasAttribute(SPELL_ATTR_EX4_CAN_CAST_WHILE_CASTING) && !newSpell->m_spellInfo->HasAttribute(SPELL_ATTR_EX4_CAN_CAST_WHILE_CASTING))
                    InterruptSpell(CURRENT_CHANNELED_SPELL, false);
            }

            // autorepeat breaking
            if (m_currentSpells[CURRENT_AUTOREPEAT_SPELL])
            {
                // break autorepeat if not Auto Shot
                if (m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo->Category == 351) // TODO: figure out what channel interrupt flag corresponds to this
                    InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
            }
        } break;

        case CURRENT_CHANNELED_SPELL:
        {
            // channel spells always break generic non-delayed and any channeled spells
            InterruptSpell(CURRENT_CHANNELED_SPELL);

            // Channeled spell can only be one
            // Spells with this attribute can be cast in parallel with generic spells
            if (!newSpell->m_spellInfo->HasAttribute(SPELL_ATTR_EX4_CAN_CAST_WHILE_CASTING) && !newSpell->m_IsTriggeredSpell)
            {
                if (Spell const* genericSpell = m_currentSpells[CURRENT_AUTOREPEAT_SPELL])
                    if (!genericSpell->m_spellInfo->HasAttribute(SPELL_ATTR_EX4_CAN_CAST_WHILE_CASTING))
                        InterruptSpell(CURRENT_GENERIC_SPELL, false);

                // it also does break autorepeat if not Auto Shot
                if (m_currentSpells[CURRENT_AUTOREPEAT_SPELL] &&
                    m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo->Category == 351)
                    InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
            }
        } break;

        case CURRENT_AUTOREPEAT_SPELL:
        {
            // only Auto Shoot does not break anything
            if (newSpell->m_spellInfo->Category == 351)
            {
                // generic autorepeats break generic non-delayed and channeled non-delayed spells
                InterruptSpell(CURRENT_GENERIC_SPELL, false);
                if (m_currentSpells[CURRENT_CHANNELED_SPELL])
                {
                    if (!newSpell->m_spellInfo->HasAttribute(SPELL_ATTR_EX4_CAN_CAST_WHILE_CASTING))
                        InterruptSpell(CURRENT_CHANNELED_SPELL, false);
                }
            }
            // special action: set first cast flag
            ResetAutoRepeatSpells();
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
    m_currentSpells[CSpellType] = newSpell;
    newSpell->SetReferencedFromCurrent(true);
}

void Unit::InterruptSpell(CurrentSpellTypes spellType, bool withDelayed)
{
    if (m_currentSpells[spellType] && (withDelayed || m_currentSpells[spellType]->getState() != SPELL_STATE_TRAVELING))
    {
        // send autorepeat cancel message for autorepeat spells
        if (spellType == CURRENT_AUTOREPEAT_SPELL)
        {
            if (GetTypeId() == TYPEID_PLAYER)
                ((Player*)this)->SendAutoRepeatCancel();
        }

        if (m_currentSpells[spellType]->CanBeInterrupted())
        {
            SpellEntry const* spellInfo = m_currentSpells[spellType]->m_spellInfo;
            m_currentSpells[spellType]->cancel();

            if (AI())
                AI()->OnSpellInterrupt(spellInfo);

            // cancel can interrupt spell already (caster cancel ->target aura remove -> caster iterrupt)
            if (m_currentSpells[spellType])
            {
                m_currentSpells[spellType]->SetReferencedFromCurrent(false);
                m_currentSpells[spellType] = nullptr;
            }
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

bool Unit::IsNonMeleeSpellCasted(bool withDelayed, bool skipChanneled, bool skipAutorepeat, bool forMovement, bool forAutoIgnore) const
{
    // We don't do loop here to explicitly show that melee spell is excluded.
    // Maybe later some special spells will be excluded too.

    // generic spells are casted when they are not finished and not delayed
    if (Spell const* genericSpell = m_currentSpells[CURRENT_GENERIC_SPELL])
    {
        if (genericSpell->getState() != SPELL_STATE_FINISHED)
        {
            bool specialResult = true;
            if (forMovement) // mobs can move during spells without this flag
                specialResult = genericSpell->m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_MOVEMENT;
            bool isAutoNonInterrupting = forAutoIgnore && genericSpell->m_spellInfo->HasAttribute(SPELL_ATTR_EX2_NOT_RESET_AUTO_ACTIONS);
            if (!isAutoNonInterrupting && specialResult && (withDelayed || genericSpell->getState() != SPELL_STATE_TRAVELING))
                return true;
        }
    }

    // channeled spells may be delayed, but they are still considered casted
    if (!skipChanneled)
    {
        if (Spell const* channeledSpell = m_currentSpells[CURRENT_CHANNELED_SPELL])
        {
            bool attributeResult = false;
            if (!forMovement)
                attributeResult = channeledSpell->m_spellInfo->HasAttribute(SPELL_ATTR_EX4_CAN_CAST_WHILE_CASTING);

            bool isAutoNonInterrupting = forAutoIgnore && channeledSpell->m_spellInfo->HasAttribute(SPELL_ATTR_EX2_NOT_RESET_AUTO_ACTIONS);
            if (!isAutoNonInterrupting && !attributeResult && !channeledSpell->m_IsTriggeredSpell && (channeledSpell->getState() != SPELL_STATE_FINISHED))
                return true;
        }
    }

    // autorepeat spells may be finished or delayed, but they are still considered casted
    if (!skipAutorepeat && m_currentSpells[CURRENT_AUTOREPEAT_SPELL])
        return true;

    return forAutoIgnore;
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
    for (auto m_currentSpell : m_currentSpells)
        if (m_currentSpell && m_currentSpell->m_spellInfo->Id == spell_id)
            return m_currentSpell;
    return nullptr;
}

void Unit::SetInFront(Unit const* target)
{
    SetOrientation(GetAngle(target));
}

void Unit::SetFacingTo(float ori)
{
    Movement::MoveSplineInit init(*this);
    // supplied orientation is global space but we need local orientation
    if (GenericTransport* transport = GetTransport())
        transport->CalculatePassengerOrientation(ori);
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
    return unit->CanWalk() || unit->CanFly();
}

bool Unit::IsInWater() const
{
    return GetTerrain()->IsInWater(GetPositionX(), GetPositionY(), GetPositionZ());
}

bool Unit::IsInSwimmableWater() const
{
    return GetTerrain()->IsSwimmable(GetPositionX(), GetPositionY(), GetPositionZ());
}

bool Unit::IsUnderwater() const
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
    for (auto i : mTotalAuraList)
        modifier += i->GetModifier()->m_amount;

    return modifier;
}

float Unit::GetTotalAuraMultiplier(AuraType auratype) const
{
    float multiplier = 1.0f;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (auto i : mTotalAuraList)
        multiplier *= (100.0f + i->GetModifier()->m_amount) / 100.0f;

    return multiplier;
}

int32 Unit::GetMaxPositiveAuraModifier(AuraType auratype) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (auto i : mTotalAuraList)
        if (i->GetModifier()->m_amount > modifier)
            modifier = i->GetModifier()->m_amount;

    return modifier;
}

int32 Unit::GetMaxNegativeAuraModifier(AuraType auratype) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (auto i : mTotalAuraList)
        if (i->GetModifier()->m_amount < modifier)
            modifier = i->GetModifier()->m_amount;

    return modifier;
}

int32 Unit::GetTotalAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const
{
    if (!misc_mask)
        return 0;

    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (auto i : mTotalAuraList)
    {
        Modifier* mod = i->GetModifier();
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
    for (auto i : mTotalAuraList)
    {
        Modifier* mod = i->GetModifier();
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
    for (auto i : mTotalAuraList)
    {
        Modifier* mod = i->GetModifier();
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
    for (auto i : mTotalAuraList)
    {
        Modifier* mod = i->GetModifier();
        if (mod->m_miscvalue & misc_mask && mod->m_amount < modifier)
            modifier = mod->m_amount;
    }

    return modifier;
}

int32 Unit::GetTotalAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (auto i : mTotalAuraList)
    {
        Modifier* mod = i->GetModifier();
        if (mod->m_miscvalue == misc_value)
            modifier += mod->m_amount;
    }
    return modifier;
}

float Unit::GetTotalAuraMultiplierByMiscValue(AuraType auratype, int32 misc_value) const
{
    float multiplier = 1.0f;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (auto i : mTotalAuraList)
    {
        Modifier* mod = i->GetModifier();
        if (mod->m_miscvalue == misc_value)
            multiplier *= (100.0f + mod->m_amount) / 100.0f;
    }
    return multiplier;
}

int32 Unit::GetMaxPositiveAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (auto i : mTotalAuraList)
    {
        Modifier* mod = i->GetModifier();
        if (mod->m_miscvalue == misc_value && mod->m_amount > modifier)
            modifier = mod->m_amount;
    }

    return modifier;
}

int32 Unit::GetMaxNegativeAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    for (auto i : mTotalAuraList)
    {
        Modifier* mod = i->GetModifier();
        if (mod->m_miscvalue == misc_value && mod->m_amount < modifier)
            modifier = mod->m_amount;
    }

    return modifier;
}

bool Unit::AddSpellAuraHolder(SpellAuraHolder* holder)
{
    SpellEntry const* aurSpellInfo = holder->GetSpellProto();

    // ghost spell check, allow apply any auras at player loading in ghost mode (will be cleanup after load)
    if (!IsAlive() && !IsDeathPersistentSpell(aurSpellInfo) &&
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
                    foundHolder->ModStackAmount(holder->GetStackAmount(), holder->GetCaster());
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
                if (aurSpellInfo->StackAmount && !IsChanneledSpell(aurSpellInfo) && !aurSpellInfo->HasAttribute(SPELL_ATTR_EX3_STACK_FOR_DIFF_CASTERS))
                {
                    foundHolder->ModStackAmount(holder->GetStackAmount(), holder->GetCaster());
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
                        if (sSpellMgr.IsSingleTargetSpells(itr_spellEntry, aurSpellInfo))
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
    if (m_spellUpdateHappening)
        holder->SetCreationDelayFlag();
    m_spellAuraHolders.insert(SpellAuraHolderMap::value_type(holder->GetId(), holder));

    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (Aura* aur = holder->GetAuraByEffectIndex(SpellEffectIndex(i)))
            AddAuraToModList(aur);

    holder->ApplyAuraModifiers(true, true);                 // This is the place where auras are actually applied onto the target
    DETAIL_FILTER_LOG(LOG_FILTER_SPELL_CAST, "Holder of spell %u now is in use", holder->GetId());

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
    SpellAuraHolderMap::const_iterator next;
    for (SpellAuraHolderMap::const_iterator i = m_spellAuraHolders.begin(); i != m_spellAuraHolders.end(); i = next)
    {
        next = i;
        ++next;
        uint32 i_spellId = (*i).second->GetId();
        if ((*i).second && i_spellId && i_spellId != spellId)
        {
            if (sSpellMgr.IsSpellAnotherRankOfSpell(spellId, i_spellId))
            {
                RemoveAurasDueToSpell(i_spellId);

                if (m_spellAuraHolders.empty())
                    break;
                next =  m_spellAuraHolders.begin();
            }
        }
    }
}

void Unit::RemoveAllGroupBuffsFromCaster(ObjectGuid casterGuid)
{
    SpellAuraHolderMap::const_iterator next;
    for (SpellAuraHolderMap::const_iterator i = m_spellAuraHolders.begin(); i != m_spellAuraHolders.end(); i = next)
    {
        next = i;
        ++next;
        SpellAuraHolder* holder = (*i).second;
        if (holder->GetCasterGuid() == casterGuid && IsGroupBuff(holder->GetSpellProto()))
        {
            RemoveAurasDueToSpell((*i).first);

            if (m_spellAuraHolders.empty())
                break;
            next = m_spellAuraHolders.begin();
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
    auto drGroup = holder->getDiminishGroup();
    SpellEntry const* triggeredBy = holder->GetTriggeredBy();

    SpellAuraHolderMap::iterator next;
    for (SpellAuraHolderMap::iterator i = m_spellAuraHolders.begin(); i != m_spellAuraHolders.end(); i = next)
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
        auto existingDrGroup = existing->getDiminishGroup();
        const bool own = (holder->GetCasterGuid() == existing->GetCasterGuid());

        // early checks that spellId is passive non stackable spell
        // Experimental: passive abilities dont stack with itself
        if (IsPassiveSpell(existingSpellProto) && (spellId != existingSpellId || !spellProto->HasAttribute(SPELL_ATTR_ABILITY)))
        {
            // passive non-stackable spells not stackable only for same caster
            // Experimental: exclude party passive auras from this
            if (!own && !IsSpellHaveEffect(spellProto, SPELL_EFFECT_APPLY_AREA_AURA_PARTY))
                continue;

            // passive non-stackable spells not stackable only with another rank of same spell
            if (!sSpellMgr.IsSpellAnotherRankOfSpell(spellId, existingSpellId))
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

        bool diminished = false;
        // Remove any existing holders from the same diminishing returns group by treating as unique
        if (!unique && drGroup && drGroup != DIMINISHING_LIMITONLY)
        {
            diminished = (drGroup == existingDrGroup);
            unique = diminished;
        }

        bool stackable = (own ? sSpellMgr.IsSpellStackableWithSpell(spellProto, existingSpellProto) : sSpellMgr.IsSpellStackableWithSpellForDifferentCasters(spellProto, existingSpellProto));

        // Remove only own auras when multiranking
        if (!unique && own && stackable && sSpellMgr.IsSpellAnotherRankOfSpell(spellId, existingSpellId))
        {
            // Check for same item source (allow two weapon enchants)
            if (const ObjectGuid &itemGuid = holder->GetCastItemGuid())
            {
                if (itemGuid != existing->GetCastItemGuid())
                    continue;
            }

            unique = true;
            personal = true;
        }

        if (unique || !stackable)
        {
            // check if this spell can be triggered by any talent aura
            if (!unique && triggeredBy && sSpellMgr.IsSpellCanAffectSpell(triggeredBy, existingSpellProto))
                continue;

            if (!own && personal && stackable)
                continue;

            if (!IsSpellWithCasterSourceTargetsOnly(spellProto) && !IsSpellWithCasterSourceTargetsOnly(existingSpellProto))
            {
                // holder cannot remove higher/stronger rank if it isn't from the same caster
                if (IsSimilarExistingAuraStronger(holder, existing))
                    return false;

                if (!diminished && sSpellMgr.IsSpellAnotherRankOfSpell(spellId, existingSpellId) && sSpellMgr.IsSpellHigherRankOfSpell(existingSpellId, spellId))
                    return false;
            }

            if (personal && stackable)
                RemoveAurasByCasterSpell(existingSpellId, holder->GetCasterGuid());
            else
                RemoveAurasDueToSpell(existingSpellId);

            if (m_spellAuraHolders.empty())
                break;

            next = m_spellAuraHolders.begin();
        }
    }
    return true;
}

void Unit::RemoveAurasByCasterSpell(uint32 spellId, ObjectGuid casterGuid, AuraRemoveMode mode)
{
    SpellAuraHolderBounds spair = GetSpellAuraHolderBounds(spellId);
    for (SpellAuraHolderMap::iterator iter = spair.first; iter != spair.second;)
    {
        if (iter->second->GetCasterGuid() == casterGuid)
        {
            RemoveSpellAuraHolder(iter->second, mode);
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

void Unit::RemoveAuraHolderDueToSpellByDispel(uint32 spellId, uint32 dispellingSpellId, uint32 stackAmount, ObjectGuid casterGuid, Unit* dispeller)
{
    SpellAuraHolderBounds spair = GetSpellAuraHolderBounds(spellId);
    for (SpellAuraHolderMap::iterator iter = spair.first; iter != spair.second; ++iter)
    {
        SpellAuraHolder* holder = iter->second; // need to save due to iterator corruption
        if (!casterGuid || holder->GetCasterGuid() == casterGuid)
        {
            uint32 originalStacks = holder->GetStackAmount();
            if (holder->ModStackAmount(-int32(stackAmount), nullptr))
                RemoveSpellAuraHolder(holder, AURA_REMOVE_BY_DISPEL);
            holder->OnDispel(dispeller, dispellingSpellId, originalStacks);
            break;
        }
    }
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

void Unit::RemoveAuraStack(uint32 spellId)
{
    if (SpellAuraHolder* holder = GetSpellAuraHolder(spellId))
        if (holder->ModStackAmount(-1, nullptr)) // Remove aura on return true
            RemoveSpellAuraHolder(holder, AURA_REMOVE_BY_DEFAULT);
}

void Unit::RemoveAuraCharge(uint32 spellId)
{
    if (SpellAuraHolder* holder = GetSpellAuraHolder(spellId))
        if (holder->DropAuraCharge())
            RemoveAurasDueToSpell(spellId);
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
            if (iter->second->ModStackAmount(-int32(stackAmount), nullptr))
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
                for (auto aura : holder->m_auras)
                {
                    if (aura)
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
    MANGOS_ASSERT(!holder->IsDeleted());

    // Statue unsummoned at holder remove
    SpellEntry const* AurSpellInfo = holder->GetSpellProto();
    Totem* statue = nullptr;
    Unit* caster = holder->GetCaster();
    if (caster && IsChanneledSpell(AurSpellInfo))
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

    for (auto aura : holder->m_auras)
    {
        if (aura)
            RemoveAura(aura, mode);
    }

    holder->_RemoveSpellAuraHolder();

    if (mode != AURA_REMOVE_BY_DELETE)
        holder->HandleSpellSpecificBoosts(false);

    if (statue)
        statue->UnSummon();

    // If holder in use (removed from code that plan access to it data after return)
    // store it in holder list with delayed deletion
    holder->SetDeleted();
    m_deletedHolders.push_back(holder);

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
    m_deletedAuras.push_back(Aur);
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
    // Fly should not be removed on evade - neither should linked auras
    // Some cosmetic script auras should not be removed on evade either
    for (SpellAuraHolderMap::iterator iter = m_spellAuraHolders.begin(); iter != m_spellAuraHolders.end();)
    {
        SpellEntry const* proto = iter->second->GetSpellProto();
        if (IsSpellRemovedOnEvade(proto))
        {
            RemoveSpellAuraHolder(iter->second, AURA_REMOVE_BY_DEFAULT);
            iter = m_spellAuraHolders.begin();
        }
        else
            ++iter;
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

    for (auto aura : auras)
    {
        if (aura->isAffectedOnSpell(spellProto))
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
    for (auto aura : auras)
        if (aura->GetSpellProto()->IsFitToFamily(family, familyFlag) &&
                (!casterGuid || aura->GetCasterGuid() == casterGuid))
            return aura;
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

bool Unit::HasAuraTypeWithCaster(AuraType auratype, ObjectGuid caster) const
{
    AuraList const& auras = GetAurasByType(auratype);
    for (auto aura : auras)
        if (aura->GetCasterGuid() == caster)
            return true;
    return false;
}

uint32 Unit::GetAuraCount(uint32 spellId) const
{
    uint32 count = 0;
    SpellAuraHolderConstBounds spair = GetSpellAuraHolderBounds(spellId);

    for (auto itr = spair.first; itr != spair.second; ++itr)
    {
        if (itr->second->GetStackAmount() == 0)
            ++count;
        else
            count += (uint32)itr->second->GetStackAmount();
    }

    return count;
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
    for (auto i : m_gameObj)
        if (i->GetSpellId() == spellId)
            return i;

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

    if (gameObj->GetSpellId())
    {
        SpellEntry const* createBySpell = sSpellTemplate.LookupEntry<SpellEntry>(gameObj->GetSpellId());
        // Need disable spell use for owner
        if (createBySpell && createBySpell->HasAttribute(SPELL_ATTR_DISABLED_WHILE_ACTIVE))
            // note: item based cooldowns and cooldown spell mods with charges ignored (unknown existing cases)
            AddCooldown(*createBySpell);
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
                AddCooldown(*createBySpell);
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

    GameObjectList::iterator next;
    for (GameObjectList::iterator i = m_gameObj.begin(); i != m_gameObj.end(); i = next)
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
    // wild summoned GOs - only remove references, do not remove GOs
    m_gameObj.clear();
    m_wildGameObjs.clear();
}

void Unit::SendSpellNonMeleeDamageLog(SpellNonMeleeDamage* log) const
{
    WorldPacket data(SMSG_SPELLNONMELEEDAMAGELOG, (8 + 8 + 4 + 4 + 1 + 4 + 4 + 1 + 1 + 4 + 4 + 1));
    data << log->target->GetPackGUID();
    data << log->attacker->GetPackGUID();
    data << uint32(log->SpellID);
    data << uint32(log->damage);                            // damage amount
    data << uint8(log->school);                             // damage school
    data << uint32(log->absorb);                            // AbsorbedDamage
    data << int32(log->resist);                             // resist
    data << uint8(log->periodicLog);                        // if 1, then client show spell name (example: %s's ranged shot hit %s for %u school or %s suffers %u school damage from %s's spell_name
    data << uint8(log->unused);                             // unused
    data << uint32(log->blocked);                           // blocked
    data << uint32(log->HitInfo);

    // Debug mode bool switch (extended data):
    if (!(log->HitInfo & (SPELL_HIT_TYPE_CRIT_DEBUG | SPELL_HIT_TYPE_HIT_DEBUG | SPELL_HIT_TYPE_ATTACK_TABLE_DEBUG)))
        data << uint8(0);
    else
    {
        data << uint8(1);

        if (log->HitInfo & SPELL_HIT_TYPE_CRIT_DEBUG)
        {
            data << float(0); // roll
            data << float(0); // needed
        }

        if (log->HitInfo & SPELL_HIT_TYPE_HIT_DEBUG)
        {
            data << float(0); // roll
            data << float(0); // needed
        }

        if (log->HitInfo & SPELL_HIT_TYPE_ATTACK_TABLE_DEBUG)
        {
            data << float(0); // miss chance
            data << float(0); // dodge chance
            data << float(0); // parry chance
            data << float(0); // block chance
            data << float(0); // glance chance
            data << float(0); // crush chance
        }
    }

    SendMessageToSet(data, true);
}

void Unit::SendSpellNonMeleeDamageLog(Unit* target, uint32 spellID, uint32 damage, SpellSchoolMask damageSchoolMask, uint32 absorbedDamage, int32 resist, bool isPeriodic, uint32 blocked, bool criticalHit, bool split)
{
    SpellNonMeleeDamage log(this, target, spellID, GetFirstSchoolInMask(damageSchoolMask));
    log.damage = damage;
    log.damage += (resist < 0 ? uint32(std::abs(resist)) : 0);
    log.damage -= (absorbedDamage + (resist > 0 ? uint32(resist) : 0) + blocked);
    log.absorb = absorbedDamage;
    log.resist = resist;
    log.periodicLog = isPeriodic;
    log.blocked = blocked;
    log.HitInfo = 0;
    if (criticalHit)
        log.HitInfo |= SPELL_HIT_TYPE_CRIT;
    if (split)
        log.HitInfo |= SPELL_HIT_TYPE_SPLIT;
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
            data << int32(pInfo->resist);                   // resist
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

void Unit::SendSpellDamageResist(Unit* target, uint32 spellId) const
{
    WorldPacket data(SMSG_PROCRESIST, 8+8+4+1);
    data << GetObjectGuid();
    data << target->GetObjectGuid();
    data << uint32(spellId);
    data << uint8(0); // bool - log format: 0-default, 1-debug
    SendMessageToSet(data, true);
}

void Unit::SendSpellOrDamageImmune(ObjectGuid casterGuid, Unit* target, uint32 spellID)
{
    WorldPacket data(SMSG_SPELLORDAMAGE_IMMUNE, (8 + 8 + 4 + 1));
    data << casterGuid;
    data << target->GetObjectGuid();
    data << uint32(spellID);
    data << uint8(0);
    target->SendMessageToSet(data, true);
}

void Unit::SendEnchantmentLog(ObjectGuid targetGuid, uint32 itemEntry, uint32 enchantId) const
{
    WorldPacket data(SMSG_ENCHANTMENTLOG, (8 + 8 + 4 + 4));
    data << GetPackGUID();
    data << targetGuid.WriteAsPacked();
    data << uint32(itemEntry);
    data << uint32(enchantId);
    SendMessageToSet(data, true);
}

void Unit::CasterHitTargetWithSpell(Unit* realCaster, Unit* target, SpellEntry const* spellInfo, bool triggered, bool success/* = true*/)
{
    if (realCaster->CanAttack(target))
    {
        if (spellInfo->HasAttribute(SPELL_ATTR_EX3_NO_INITIAL_AGGRO) && !spellInfo->HasAttribute(SPELL_ATTR_EX3_OUT_OF_COMBAT_ATTACK))
            return;

        // we want to change the stand state of each character if possible/required
         // Since patch 1.5.0 sitting creature always stand up on attack (even if stunned)
        if (success && !target->IsStandState() && target->IsPlayer())
            target->SetStandState(UNIT_STAND_STATE_STAND);

        // Hostile spell hits count as attack made against target (if detected), stealth removed at Spell::cast if spell break it
        const bool attack = (!IsPositiveSpell(spellInfo->Id, realCaster, target) && IsVisibleForOrDetect(target, target, false) && CanEnterCombat() && target->CanEnterCombat());

        if (attack && !spellInfo->HasAttribute(SPELL_ATTR_EX3_NO_INITIAL_AGGRO) && !spellInfo->HasAttribute(SPELL_ATTR_EX_NO_THREAT))
        {
            if (success)
            {
                target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_HITBYSPELL);

                // caster can be detected but have stealth aura
                if (!spellInfo->HasAttribute(SPELL_ATTR_EX_NOT_BREAK_STEALTH))
                    RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);
            }

            target->AttackedBy(realCaster);

            target->AddThreat(realCaster);
            realCaster->AddThreat(target);
            target->SetInCombatWithAggressor(realCaster);
            realCaster->SetInCombatWithVictim(target);
            realCaster->GetCombatManager().TriggerCombatTimer(target);
        }

        if (attack && spellInfo->HasAttribute(SPELL_ATTR_EX3_OUT_OF_COMBAT_ATTACK))
        {
            target->SetOutOfCombatWithAggressor(realCaster);
            realCaster->SetOutOfCombatWithVictim(target);
        }
    }
    else if (realCaster->CanAssist(target) && target->IsInCombat())
    {
        // assisting case, healing and resurrection
        if (!spellInfo->HasAttribute(SPELL_ATTR_EX3_NO_INITIAL_AGGRO) && !spellInfo->HasAttribute(SPELL_ATTR_EX_NO_THREAT) && CanEnterCombat() && target->CanEnterCombat())
        {
            realCaster->SetInCombatWithAssisted(target);
            target->getHostileRefManager().threatAssist(realCaster, 0.0f, spellInfo, false, triggered);
        }

        if (spellInfo->HasAttribute(SPELL_ATTR_EX3_OUT_OF_COMBAT_ATTACK))
            realCaster->SetOutOfCombatWithAssisted(target);
    }
}

void Unit::SendAIReaction(AiReaction reactionType)
{
    WorldPacket data(SMSG_AI_REACTION, 12);

    data << GetObjectGuid();
    data << uint32(reactionType);

    SendMessageToSet(data, true);

    DEBUG_FILTER_LOG(LOG_FILTER_AI_AND_MOVEGENSS, "WORLD: Sent SMSG_AI_REACTION, type %u.", reactionType);
}

bool Unit::CanInitiateAttack() const
{
    if (hasUnitState(UNIT_STAT_CAN_NOT_REACT))
        return false;

    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE))
        if (GetTypeId() != TYPEID_UNIT || !((Creature*)this)->GetForceAttackingCapability())
            return false;

    if (GetTypeId() == TYPEID_UNIT && !((Creature*)this)->CanAggro())
        return false;

    return true;
}

void Unit::SendAttackStateUpdate(CalcDamageInfo* calcDamageInfo) const
{
    DEBUG_FILTER_LOG(LOG_FILTER_COMBAT, "WORLD: Sending SMSG_ATTACKERSTATEUPDATE");

    // Subdamage count:
    uint32 lines = m_weaponDamageInfo.weapon[calcDamageInfo->attackType].lines;

    WorldPacket data(SMSG_ATTACKERSTATEUPDATE, (4 + 8 + 8 + 4) + 1 + (lines * (4 + 4 + 4 + 4 + 4)) + (4 + 4 + 4 + 4));

    data << uint32(calcDamageInfo->HitInfo);
    data << calcDamageInfo->attacker->GetPackGUID();
    data << calcDamageInfo->target->GetPackGUID();
    data << uint32(calcDamageInfo->totalDamage);                // Total damage

    data << uint8(lines);

    // Subdamage information:
    for (uint8 i = 0; i < lines; ++i)
    {
        auto &line = calcDamageInfo->subDamage[i];

        data << uint32(GetFirstSchoolInMask(line.damageSchoolMask));
        data << float(line.damage) / float(calcDamageInfo->totalDamage);   // Float coefficient of subdamage
        data << uint32(line.damage);
        data << uint32(line.absorb);
        data << int32(line.resist);
    }

    data << uint32(calcDamageInfo->TargetState);
    data << uint32(0);                                      // unknown, usually seen with -1, 0 and 1000
    data << uint32(0);                                      // spell id, seen with heroic strike and disarm as examples.
    // HITINFO_NOACTION normally set if spell

    // Blocked amount:
    data << uint32(calcDamageInfo->blocked_amount);

    // Debug info
    if (calcDamageInfo->HitInfo & HITINFO_UNK0)
    {
        data << uint32(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        for (uint8 i = 0; i < 5; ++i)
        {
            data << float(0);
            data << float(0);
        }
        data << uint32(0);
    }

    SendMessageToSet(data, true);
}

void Unit::SendAttackStateUpdate(uint32 HitInfo, Unit* target, SpellSchoolMask damageSchoolMask, uint32 Damage,
                                 uint32 AbsorbDamage, int32 Resist, VictimState TargetState, uint32 BlockedAmount)
{
    CalcDamageInfo dmgInfo;
    dmgInfo.HitInfo = HitInfo;
    dmgInfo.attacker = this;
    dmgInfo.target = target;
    dmgInfo.attackType = BASE_ATTACK;
    dmgInfo.totalDamage = Damage;
    dmgInfo.totalDamage += (Resist < 0 ? uint32(std::abs(Resist)) : 0);
    dmgInfo.totalDamage -= (AbsorbDamage + (Resist > 0 ? uint32(Resist) : 0) + BlockedAmount);
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
    else if (HasCharmer())
    {
        Unit* charmer = GetCharmer();
        if (charmer && (charmer->GetTypeId() == TYPEID_PLAYER) && ((Player*)charmer)->GetGroup())
            ((Player*)charmer)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_POWER_TYPE);
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

FactionTemplateEntry const* Unit::GetFactionTemplateEntry() const
{
    FactionTemplateEntry const* entry = sFactionTemplateStore.LookupEntry(getFaction());
    if (!entry)
    {
        static ObjectGuid guid;                             // prevent repeating spam same faction problem

        if (GetObjectGuid() != guid)
        {
            guid = GetObjectGuid();

            if (guid.GetHigh() == HIGHGUID_PET)
                sLog.outError("%s (base creature entry %u) have invalid faction template id %u, owner %s",
                    GetGuidStr().c_str(), GetEntry(), getFaction(), ((Pet*)this)->GetOwnerGuid().GetString().c_str());
            else
                sLog.outError("%s have invalid faction template id %u", GetGuidStr().c_str(), getFaction());
        }
    }
    return entry;
}

bool Unit::IsNeutralToAll() const
{
    FactionTemplateEntry const* my_faction = GetFactionTemplateEntry();
    if (!my_faction || !my_faction->faction)
        return true;

    FactionEntry const* raw_faction = sFactionStore.LookupEntry(my_faction->faction);
    if (raw_faction && raw_faction->reputationListID >= 0)
        return false;

    return my_faction->IsNeutralToAll();
}

void Unit::AttackedBy(Unit* attacker)
{
    if (!IsAlive())
        return;

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

bool Unit::Attack(Unit* victim, bool meleeAttack)
{
    if (!victim || victim == this)
        return false;

    // dead units can neither attack nor be attacked
    if (!IsAlive() || !victim->IsInWorld() || !victim->IsAlive())
        return false;

    // player cannot attack in mount state
    if (GetTypeId() == TYPEID_PLAYER && IsMounted())
        return false;

    // nobody can attack GM in GM-mode
    if (victim->GetTypeId() == TYPEID_PLAYER)
    {
        if (((Player*) victim)->IsGameMaster())
            return false;
    }
    else
    {
        if (victim->GetCombatManager().IsInEvadeMode())
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
            bool attacked = false;
            // need to regenerate target guid after certain scenarios
            if (AI() && AI()->CanExecuteCombatAction())
            {
                if (GetTargetGuid().IsEmpty())
                    attacked = true;
                SetTargetGuid(victim->GetObjectGuid());
            }
            // switch to melee attack from ranged/magic
            if (meleeAttack)
            {
                MeleeAttackStart(m_attacking);
                return true;
            }
            return attacked;
        }

        // remove old target data
        AttackStop(true);
    }

    // Do not set new target, creatures automatically turn to target clientside if target is set, leading to desync
    if (AI() && AI()->CanExecuteCombatAction())
        SetTargetGuid(victim->GetObjectGuid());

    // If attacker was already added, it means we have m_attacking cleared due to some other
    if (!victim->_addAttacker(this))
        return false;

    m_attacking = victim;
    if (CanHaveThreatList())
        getThreatManager().setCurrentVictimByTarget(victim);

    // delay offhand weapon attack to next attack time
    if (hasOffhandWeaponForAttack())
        resetAttackTimer(OFF_ATTACK);

    if (meleeAttack)
        MeleeAttackStart(m_attacking);

    if (AI())
    {
        SendAIReaction(AI_REACTION_HOSTILE);
        if (GetTypeId() == TYPEID_UNIT)
            ((Creature*)this)->CallAssistance();
    }

    return true;
}

bool Unit::AttackStop(bool targetSwitch /*= false*/, bool includingCast /*= false*/, bool includingCombo /*= false*/, bool clientInitiated /*= false*/)
{
    if (includingCombo)
        ClearComboPoints();

    // interrupt cast only id includingCast == true and we have something to interrupt.
    if (includingCast && IsNonMeleeSpellCasted(false))
        InterruptNonMeleeSpells(false);

    // Clear our target only if targetSwitch == true
    if (targetSwitch && GetTypeId() != TYPEID_PLAYER)
        SetTargetGuid(ObjectGuid());

    if (!clientInitiated && IsPlayer())
        ((Player*)this)->SendAttackSwingCancelAttack();     // melee and ranged forced attack cancel
    MeleeAttackStop(m_attacking);

    if (m_attacking)
    {
        m_attacking->_removeAttacker(this);
        m_attacking = nullptr;
        return true;
    }
    return false;
}

void Unit::CombatStop(bool includingCast, bool includingCombo)
{
    AttackStop(true, includingCast, includingCombo);

    RemoveAllAttackers();
    DeleteThreatList();

    if (GetTypeId() != TYPEID_PLAYER)
    {
        ((Creature*)this)->SetNoCallAssistance(false);

        if (((Creature*)this)->GetTemporaryFactionFlags() & TEMPFACTION_RESTORE_COMBAT_STOP)
            ((Creature*)this)->ClearTemporaryFaction();
    }

    if (AI())
        AI()->CombatStop();

    GetCombatManager().StopCombatTimer();

    ClearInCombat();
}

void Unit::MeleeAttackStart(Unit* victim)
{
    if (!hasUnitState(UNIT_STAT_MELEE_ATTACKING))
    {
        addUnitState(UNIT_STAT_MELEE_ATTACKING);
        if (victim)
            SendMeleeAttackStart(victim);
    }
}

void Unit::MeleeAttackStop(Unit* victim)
{
    if (hasUnitState(UNIT_STAT_MELEE_ATTACKING))
    {
        clearUnitState(UNIT_STAT_MELEE_ATTACKING);
        InterruptSpell(CURRENT_MELEE_SPELL);
        if (victim)
            SendMeleeAttackStop(victim);
    }
}

struct CombatStopWithPetsHelper
{
    explicit CombatStopWithPetsHelper(bool _includingCast, bool _includingCombo) : includingCast(_includingCast), includingCombo(_includingCombo) {}
    void operator()(Unit* unit) const { unit->CombatStop(includingCast, includingCombo); }
    bool includingCast, includingCombo;
};

void Unit::CombatStopWithPets(bool includingCast, bool includingCombo)
{
    CombatStop(includingCast, includingCombo);
    CallForAllControlledUnits(CombatStopWithPetsHelper(includingCast, includingCombo), CONTROLLED_PET | CONTROLLED_GUARDIANS | CONTROLLED_CHARM);
}

void Unit::RemoveAllAttackers()
{
    while (!m_attackers.empty())
    {
        AttackerSet::iterator iter = m_attackers.begin();
        (*iter)->AttackStop();
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
                for (const auto& itr : sp_list)
                {
                    if (itr.second.state == PLAYERSPELL_REMOVED) continue;
                    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(itr.first);
                    if (!spellInfo || !IsPassiveSpell(spellInfo)) continue;
                    if (AuraState(spellInfo->CasterAuraState) == flag)
                        CastSpell(this, itr.first, TRIGGERED_OLD_TRIGGERED, nullptr);
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

Unit* Unit::GetBeneficiary() const
{
    Unit* master = GetMaster();
    return (master ? master : const_cast<Unit*>(this));
}

Player* Unit::GetBeneficiaryPlayer() const
{
    Unit const* beneficiary = GetBeneficiary();
    if (beneficiary)
        return (beneficiary->GetTypeId() == TYPEID_PLAYER ? const_cast<Player*>(static_cast<Player const*>(beneficiary)) : nullptr);
    return (GetTypeId() == TYPEID_PLAYER ? const_cast<Player*>(static_cast<Player const*>(this)) : nullptr);
}

bool Unit::IsClientControlled(Player const* exactClient /*= nullptr*/) const
{
    // Severvide method to check if unit is client controlled (optionally check for specific client in control)

    // Applies only to player controlled units
    if (!HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        return false;

    // These flags are meant to be used when server controls this unit, client control is taken away
    if (HasFlag(UNIT_FIELD_FLAGS, (UNIT_FLAG_CLIENT_CONTROL_LOST | UNIT_FLAG_CONFUSED | UNIT_FLAG_FLEEING)))
        return false;

    // If unit is possessed, it has lost original control...
    if (ObjectGuid const &guid = GetCharmerGuid())
    {
        // ... but if it is a possessing charm, then we have to check if some other player controls it
        if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_POSSESSED) && guid.IsPlayer())
            return (exactClient ? (exactClient->GetObjectGuid() == guid) : true);
        return false;
    }

    // By default: players have client control over themselves
    if (GetTypeId() == TYPEID_PLAYER)
        return (exactClient ? (exactClient == this) : true);
    return false;
}

Player const* Unit::GetClientControlling() const
{
    // Serverside reverse "mover" deduction logic at controlled unit

    // Applies only to player controlled units
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
    {
        // Charm always removes control from original client...
        if (HasCharmer())
        {
            // ... but if it is a possessing charm, some other client may have control
            if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_POSSESSED))
            {
                Unit const* charmer = GetCharmer();
                if (charmer && charmer->GetTypeId() == TYPEID_PLAYER)
                    return static_cast<Player const*>(charmer);
            }
        }
        else if (GetTypeId() == TYPEID_PLAYER)
        {
            // Check if anything prevents original client from controlling
            if (IsClientControlled(static_cast<Player const*>(this)))
                return static_cast<Player const*>(this);
        }
    }
    return nullptr;
}

Unit* Unit::GetCharm(WorldObject const* pov /*= nullptr*/) const
{
    if (ObjectGuid const& guid = GetCharmGuid())
    {
        WorldObject const* accessor = (pov ? pov : this);
        if (!accessor->IsInWorld())
        {
            // If this is a pet and not placed on any map yet (loading), we have to look for a player within world globally in all maps
            if (guid.IsPlayer() && accessor->GetTypeId() == TYPEID_UNIT)
            {
                if (static_cast<Creature const*>(accessor)->IsPet() && static_cast<Pet const*>(accessor)->isLoading())
                    return ObjectAccessor::FindPlayer(guid);
            }
            // Bugcheck
            sLog.outDebug("Unit::GetCharm: Guid field management continuity violation for %s, can't look up %s while accessor is outside of the world",
                          GetGuidStr().c_str(), guid.GetString().c_str());
            return nullptr;
        }
        // We need a unit in the same map only
        if (Unit* unit = accessor->GetMap()->GetUnit(guid))
            return unit;
        // Bugcheck
        sLog.outDebug("Unit::GetCharm: Guid field management continuity violation for %s in map '%s', %s does not exist in this instance",
                      GetGuidStr().c_str(), accessor->GetMap()->GetMapName(), guid.GetString().c_str());
        // const_cast<Unit*>(this)->SetCharm(nullptr);
    }
    return nullptr;
}

Unit* Unit::GetCharmer(WorldObject const* pov /*= nullptr*/) const
{
    if (ObjectGuid const& guid = GetCharmerGuid())
    {
        WorldObject const* accessor = (pov ? pov : this);
        if (!accessor->IsInWorld())
        {
            // If this is a pet and not placed on any map yet (loading), we have to look for a player within world globally in all maps
            if (guid.IsPlayer() && accessor->GetTypeId() == TYPEID_UNIT)
            {
                if (static_cast<Creature const*>(accessor)->IsPet() && static_cast<Pet const*>(accessor)->isLoading())
                    return ObjectAccessor::FindPlayer(guid);
            }
            // Bugcheck
            sLog.outDebug("Unit::GetCharmer: Guid field management continuity violation for %s, can't look up %s while accessor is outside of the world",
                          GetGuidStr().c_str(), guid.GetString().c_str());
            return nullptr;
        }
        // We need a unit in the same map only
        if (Unit* unit = accessor->GetMap()->GetUnit(guid))
            return unit;
        // Bugcheck
        sLog.outDebug("Unit::GetCharmer: Guid field management continuity violation for %s in map '%s', %s does not exist in this instance",
                      GetGuidStr().c_str(), accessor->GetMap()->GetMapName(), guid.GetString().c_str());
        // const_cast<Unit*>(this)->SetCharmer(nullptr);
    }
    return nullptr;
}

Unit* Unit::GetCreator(WorldObject const* pov /*= nullptr*/) const
{
    if (ObjectGuid const& guid = GetCreatorGuid())
    {
        WorldObject const* accessor = (pov ? pov : this);
        if (!accessor->IsInWorld())
        {
            // If this is a pet and not placed on any map yet (loading), we have to look for a player within world globally in all maps
            if (guid.IsPlayer() && accessor->GetTypeId() == TYPEID_UNIT)
            {
                if (static_cast<Creature const*>(accessor)->IsPet() && static_cast<Pet const*>(accessor)->isLoading())
                    return ObjectAccessor::FindPlayer(guid);
            }
            // Bugcheck
            sLog.outDebug("Unit::GetCreator: Guid field management continuity violation for %s, can't look up %s while accessor is outside of the world",
                          GetGuidStr().c_str(), guid.GetString().c_str());
            return nullptr;
        }
        // We need a unit in the same map only
        if (Unit* unit = accessor->GetMap()->GetUnit(guid))
            return unit;
        // Bugcheck
        sLog.outDebug("Unit::GetCreator: Guid field management continuity violation for %s in map '%s', %s does not exist in this instance",
                      GetGuidStr().c_str(), accessor->GetMap()->GetMapName(), guid.GetString().c_str());
        // const_cast<Unit*>(this)->SetCreator(nullptr);
    }
    return nullptr;
}

Unit* Unit::GetTarget(WorldObject const* pov /*= nullptr*/) const
{
    if (ObjectGuid const& guid = GetTargetGuid())
    {
        WorldObject const* accessor = (pov ? pov : this);
        if (!accessor->IsInWorld())
        {
            // If this is a pet and not placed on any map yet (loading), we have to look for a player within world globally in all maps
            if (guid.IsPlayer() && accessor->GetTypeId() == TYPEID_UNIT)
            {
                if (static_cast<Creature const*>(accessor)->IsPet() && static_cast<Pet const*>(accessor)->isLoading())
                    return ObjectAccessor::FindPlayer(guid);
            }
            // Bugcheck
            sLog.outDebug("Unit::GetTarget: Guid field management continuity violation for %s, can't look up %s while accessor is outside of the world",
                          GetGuidStr().c_str(), guid.GetString().c_str());
            return nullptr;
        }
        // We need a unit in the same map only
        if (Unit* unit = accessor->GetMap()->GetUnit(guid))
            return unit;
        // Bugcheck
        sLog.outDebug("Unit::GetTarget: Guid field management continuity violation for %s in map '%s', %s does not exist in this instance",
                      GetGuidStr().c_str(), accessor->GetMap()->GetMapName(), guid.GetString().c_str());
        // const_cast<Unit*>(this)->SetTarget(nullptr);
    }
    return nullptr;
}

Unit* Unit::GetChannelObject(WorldObject const* pov /*= nullptr*/) const
{
    if (ObjectGuid const& guid = GetChannelObjectGuid())
    {
        WorldObject const* accessor = (pov ? pov : this);
        if (!accessor->IsInWorld())
        {
            // If this is a pet and not placed on any map yet (loading), we have to look for a player within world globally in all maps
            if (guid.IsPlayer() && accessor->GetTypeId() == TYPEID_UNIT)
            {
                if (static_cast<Creature const*>(accessor)->IsPet() && static_cast<Pet const*>(accessor)->isLoading())
                    return ObjectAccessor::FindPlayer(guid);
            }
            // Bugcheck
            sLog.outDebug("Unit::GetChannelObject: Guid field management continuity violation for %s, can't look up %s while accessor is outside of the world",
                          GetGuidStr().c_str(), guid.GetString().c_str());
            return nullptr;
        }
        // We need a unit in the same map only
        if (Unit* unit = accessor->GetMap()->GetUnit(guid))
            return unit;
        // Bugcheck
        sLog.outDebug("Unit::GetChannelObject: Guid field management continuity violation for %s in map '%s', %s does not exist in this instance",
                      GetGuidStr().c_str(), accessor->GetMap()->GetMapName(), guid.GetString().c_str());
        // const_cast<Unit*>(this)->SetChannelObject(nullptr);
    }
    return nullptr;
}

Unit* Unit::GetOwner(WorldObject const* pov /*= nullptr*/, bool recursive /*= false*/) const
{
    // Default, creator field as owner is present in everything: totems, pets, guardians, etc
    Unit* owner = GetCreator(pov);
    // Query owner recursively (ascending)
    if (recursive)
    {
        while (Unit* grandowner = (owner ? owner->GetOwner(pov, recursive) : nullptr))
            owner = grandowner;
    }
    return owner;
}

bool Unit::HasMaster() const
{
    return HasCharmer() || GetOwnerGuid().IsUnit();
}

Unit* Unit::GetMaster(WorldObject const* pov /*= nullptr*/) const
{
    Unit* charmer = GetCharmer(pov);
    return (charmer ? charmer : GetOwner(pov));
}

Unit* Unit::GetSpawner(WorldObject const* pov /*= nullptr*/) const
{
    if (ObjectGuid const& guid = GetSpawnerGuid())
    {
        WorldObject const* accessor = (pov ? pov : this);
        if (!accessor->IsInWorld())
        {
            // If this is a pet and not placed on any map yet (loading), we have to look for a player within world globally in all maps
            if (guid.IsPlayer() && accessor->GetTypeId() == TYPEID_UNIT)
            {
                if (static_cast<Creature const*>(accessor)->IsPet() && static_cast<Pet const*>(accessor)->isLoading())
                    return ObjectAccessor::FindPlayer(guid);
            }
            // Bugcheck
            sLog.outDebug("Unit::GetSpawner: Guid field management continuity violation for %s, can't look up %s while accessor is outside of the world",
                          GetGuidStr().c_str(), guid.GetString().c_str());
            return nullptr;
        }
        // We need a unit in the same map only
        if (Unit* unit = accessor->GetMap()->GetUnit(guid))
            return unit;
        // Bugcheck
        sLog.outDebug("Unit::GetSpawner: Guid field management continuity violation for %s in map '%s', %s does not exist in this instance",
                      GetGuidStr().c_str(), accessor->GetMap()->GetMapName(), guid.GetString().c_str());
    }
    return nullptr;
}

Unit* Unit::_GetUnit(ObjectGuid guid) const
{
    if (Map* map = GetMap())
        return map->GetUnit(guid);
    return nullptr;
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

Pet* Unit::GetMiniPet() const
{
    if (!GetCritterGuid())
        return nullptr;

    return GetMap()->GetPet(GetCritterGuid());
}

void Unit::RemoveMiniPet()
{
    if (Pet* pet = GetMiniPet())
        pet->Unsummon(PET_SAVE_AS_DELETED, this);
    else
        SetCritterGuid(ObjectGuid());
}

void Unit::AddGuardian(Pet* pet)
{
    m_guardianPets.insert(pet->GetObjectGuid());
}

void Unit::RemoveGuardian(Pet* pet)
{
    if (m_guardianPetsIterator != m_guardianPets.end() && *m_guardianPetsIterator == pet->GetObjectGuid())
        ++m_guardianPetsIterator;
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
    for (auto m_guardianPet : m_guardianPets)
        if (Pet* guardian = GetMap()->GetPet(m_guardianPet))
            if (guardian->GetEntry() == entry)
                return guardian;

    return nullptr;
}

uint32 Unit::CountGuardiansWithEntry(uint32 entry)
{
    uint32 count = 0;

    for (auto m_guardianPet : m_guardianPets)
        if (Pet* guardian = GetMap()->GetPet(m_guardianPet))
            if (guardian->GetEntry() == entry)
                count++;

    return count;
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
    for (auto i : m_TotemSlot)
        if (!i)
            return false;
    return true;
}

void Unit::_AddTotem(TotemSlot slot, Totem* totem)
{
    m_TotemSlot[slot] = totem->GetObjectGuid();
}

void Unit::_RemoveTotem(Totem* totem)
{
    for (auto& i : m_TotemSlot)
    {
        if (i == totem->GetObjectGuid())
        {
            i.Clear();
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

Unit* Unit::SelectMagnetTarget(Unit* victim, Spell* spell)
{
    if (!victim)
        return nullptr;

    // Magic case
    if (spell && (spell->m_spellInfo->DmgClass == SPELL_DAMAGE_CLASS_NONE || spell->m_spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MAGIC))
    {
        if (spell->m_spellInfo->HasAttribute(SPELL_ATTR_ABILITY) || spell->m_spellInfo->HasAttribute(SPELL_ATTR_EX_CANT_BE_REDIRECTED) || spell->m_spellInfo->HasAttribute(SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY))
            return nullptr;

        Unit::AuraList const& magnetAuras = victim->GetAurasByType(SPELL_AURA_SPELL_MAGNET);
        for (auto magnetAura : magnetAuras)
        {
            if (Unit* magnet = magnetAura->GetCaster())
            {
                if (magnet->IsAlive() && magnet->IsWithinLOSInMap(this, true) && CanAttack(magnet))
                {
                    magnetAura->UseMagnet();
                    return magnet;
                }
            }
        }
    }

    return nullptr;
}

void Unit::SendHealSpellLog(Unit* pVictim, uint32 SpellID, uint32 Damage, bool critical)
{
    WorldPacket data(SMSG_SPELLHEALLOG, (8 + 8 + 4 + 4 + 1));
    data << pVictim->GetPackGUID();
    data << GetPackGUID();
    data << uint32(SpellID);
    data << uint32(Damage);
    data << uint8(critical ? 1 : 0);
    SendMessageToSet(data, true);
}

void Unit::SendEnergizeSpellLog(Unit* pVictim, uint32 SpellID, uint32 Damage, Powers powertype) const
{
    WorldPacket data(SMSG_SPELLENERGIZELOG, (8 + 8 + 4 + 4 + 4));
    data << pVictim->GetPackGUID();
    data << GetPackGUID();
    data << uint32(SpellID);
    data << uint32(powertype);
    data << uint32(Damage);
    SendMessageToSet(data, true);
}

void Unit::SendEnvironmentalDamageLog(uint8 type, uint32 damage, uint32 absorb, int32 resist) const
{
    WorldPacket data(SMSG_ENVIRONMENTALDAMAGELOG, (8 + 1 + 4 + 4 + 4));
    data << GetObjectGuid();
    data << uint8(type != DAMAGE_FALL_TO_VOID ? type : DAMAGE_FALL);
    data << uint32(damage);
    data << uint32(absorb);
    data << int32(resist);
    SendMessageToSet(data, true);
}

void Unit::EnergizeBySpell(Unit* victim, SpellEntry const* spellInfo, uint32 damage, Powers powerType)
{
    SendEnergizeSpellLog(victim, spellInfo->Id, damage, powerType);
    // needs to be called after sending spell log
    victim->ModifyPower(powerType, damage);
    victim->getHostileRefManager().threatAssist(this, float(damage) * 0.5f * sSpellMgr.GetSpellThreatMultiplier(spellInfo), spellInfo);
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
uint32 Unit::SpellDamageBonusDone(Unit* victim, SpellEntry const* spellProto, uint32 pdamage, DamageEffectType damagetype, uint32 stack)
{
    if (!spellProto || !victim || damagetype == DIRECT_DAMAGE)
        return pdamage;

    // Some spells don't benefit from done mods
    if (spellProto->HasAttribute(SPELL_ATTR_EX3_NO_DONE_BONUS))
        return pdamage;

    // For totems get damage bonus from owner (statue isn't totem in fact)
    if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->IsTotem() && ((Totem*)this)->GetTotemType() != TOTEM_STATUE)
    {
        if (Unit* owner = GetOwner())
            return owner->SpellDamageBonusDone(victim, spellProto, pdamage, damagetype);
    }

    uint32 creatureTypeMask = victim->GetCreatureTypeMask();
    float DoneTotalMod = 1.0f;
    int32 DoneTotal = 0;

    // Creature damage
    if (GetTypeId() == TYPEID_UNIT && !((Creature*)this)->IsPet())
        DoneTotalMod *= Creature::_GetSpellDamageMod(((Creature*)this)->GetCreatureInfo()->Rank);

    AuraList const& mModDamagePercentDone = GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
    for (auto i : mModDamagePercentDone)
    {
        if ((i->GetModifier()->m_miscvalue & GetSpellSchoolMask(spellProto)) &&
            i->GetSpellProto()->EquippedItemClass == -1 &&
                // -1 == any item class (not wand then)
            i->GetSpellProto()->EquippedItemInventoryTypeMask == 0)
            // 0 == any inventory type (not wand then)
        {
            DoneTotalMod *= (i->GetModifier()->m_amount + 100.0f) / 100.0f;
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

    // Done fixed damage bonus auras
    int32 DoneAdvertisedBenefit = SpellBaseDamageBonusDone(GetSpellSchoolMask(spellProto));

    for (auto i : GetScriptedLocationAuras(SCRIPT_LOCATION_SPELL_DAMAGE_DONE))
    {
        if (!i->isAffectedOnSpell(spellProto))
            continue;
        i->OnDamageCalculate(DoneAdvertisedBenefit, DoneTotalMod);
    }

    AuraList const& mOverrideClassScript = owner->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    for (auto i : mOverrideClassScript)
    {
        if (!i->isAffectedOnSpell(spellProto))
            continue;

        switch (i->GetModifier()->m_miscvalue)
        {
            case 4418: // Increased Shock Damage
            case 4554: // Increased Lightning Damage
            {
                DoneAdvertisedBenefit += i->GetModifier()->m_amount;
                break;
            }
            case 4555: // Improved Moonfire
            {
                DoneTotalMod *= (i->GetModifier()->m_amount + 100.0f) / 100.0f;
                break;
            }
        }
    }

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
uint32 Unit::SpellDamageBonusTaken(Unit* caster, SpellEntry const* spellProto, uint32 pdamage, DamageEffectType damagetype, uint32 stack)
{
    if (!spellProto || damagetype == DIRECT_DAMAGE)
        return pdamage;

    // Some spells don't benefit from taken mods
    if (spellProto->HasAttribute(SPELL_ATTR_EX3_NO_DONE_BONUS))
        return pdamage;

    uint32 schoolMask = GetSpellSchoolMask(spellProto);

    // Taken total percent damage auras
    float TakenTotalMod = 1.0f;
    int32 TakenTotal = 0;

    // ..taken
    TakenTotalMod *= GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, schoolMask);

    // Taken fixed damage bonus auras
    int32 TakenAdvertisedBenefit = SpellBaseDamageBonusTaken(GetSpellSchoolMask(spellProto));

    for (auto i : GetScriptedLocationAuras(SCRIPT_LOCATION_SPELL_DAMAGE_TAKEN))
    {
        if (!i->isAffectedOnSpell(spellProto))
            continue;
        i->OnDamageCalculate(TakenAdvertisedBenefit, TakenTotalMod);
    }

    // apply benefit affected by spell power implicit coeffs and spell level penalties
    if (caster)
        TakenTotal = caster->SpellBonusWithCoeffs(spellProto, TakenTotal, TakenAdvertisedBenefit, 0, damagetype, false);

    float tmpDamage = (int32(pdamage) + TakenTotal * int32(stack)) * TakenTotalMod;

    return tmpDamage > 0 ? uint32(tmpDamage) : 0;
}

int32 Unit::SpellBaseDamageBonusDone(SpellSchoolMask schoolMask)
{
    int32 DoneAdvertisedBenefit = 0;

    // ..done
    AuraList const& mDamageDone = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE);
    for (auto i : mDamageDone)
    {
        if ((i->GetModifier()->m_miscvalue & schoolMask) != 0 &&
            i->GetSpellProto()->EquippedItemClass == -1 &&                   // -1 == any item class (not wand then)
            i->GetSpellProto()->EquippedItemInventoryTypeMask == 0)          //  0 == any inventory type (not wand then)
            DoneAdvertisedBenefit += i->GetModifier()->m_amount;
    }

    if (GetTypeId() == TYPEID_PLAYER)
    {
        // Damage bonus from stats
        AuraList const& mDamageDoneOfStatPercent = GetAurasByType(SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT);
        for (auto i : mDamageDoneOfStatPercent)
        {
            if (i->GetModifier()->m_miscvalue & schoolMask)
            {
                // stat used stored in miscValueB for this aura
                Stats usedStat = STAT_SPIRIT;
                DoneAdvertisedBenefit += int32(GetStat(usedStat) * i->GetModifier()->m_amount / 100.0f);
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
    for (auto i : mDamageTaken)
    {
        if ((i->GetModifier()->m_miscvalue & schoolMask) != 0)
            TakenAdvertisedBenefit += i->GetModifier()->m_amount;
    }

    return TakenAdvertisedBenefit;
}

/**
 * Calculates caster part of healing spell bonuses,
 * also includes different bonuses dependent from target auras
 */
uint32 Unit::SpellHealingBonusDone(Unit* victim, SpellEntry const* spellProto, int32 healamount, DamageEffectType damagetype, uint32 stack)
{
    // Some spells don't benefit from done mods
    if (spellProto->HasAttribute(SPELL_ATTR_EX3_NO_DONE_BONUS))
        return healamount;

    // For totems get healing bonus from owner (statue isn't totem in fact)
    if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->IsTotem() && ((Totem*)this)->GetTotemType() != TOTEM_STATUE)
        if (Unit* owner = GetOwner())
            return owner->SpellHealingBonusDone(victim, spellProto, healamount, damagetype, stack);

    // No heal amount for this class spells
    if (spellProto->DmgClass == SPELL_DAMAGE_CLASS_NONE)
        return healamount < 0 ? 0 : healamount;

    // Healing Done
    // Done total percent damage auras
    float  DoneTotalMod = 1.0f;
    int32  DoneTotal = 0;

    // Healing done percent
    AuraList const& mHealingDonePct = GetAurasByType(SPELL_AURA_MOD_HEALING_DONE_PERCENT);
    for (auto i : mHealingDonePct)
        DoneTotalMod *= (100.0f + i->GetModifier()->m_amount) / 100.0f;

    // Done fixed damage bonus auras
    int32 DoneAdvertisedBenefit = SpellBaseHealingBonusDone(GetSpellSchoolMask(spellProto));

    // done scripted mod (take it from owner)
    Unit* owner = GetOwner();
    if (!owner) owner = this;

    AuraList const& mOverrideClassScript = owner->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    for (auto i : mOverrideClassScript)
    {
        if (!i->isAffectedOnSpell(spellProto))
            continue;
        switch (i->GetModifier()->m_miscvalue)
        {
            case 4415: // Increased Rejuvenation Healing
            case 3736: // Hateful Totem of the Third Wind / Increased Lesser Healing Wave / Savage Totem of the Third Wind
                DoneAdvertisedBenefit += i->GetModifier()->m_amount;
                break;
            default:
                break;
        }
    }

    AuraList const& auraDummy = owner->GetAurasByType(SPELL_AURA_DUMMY);
    for (auto i : auraDummy)
    {
        if (!i->isAffectedOnSpell(spellProto))
            continue;
        switch (i->GetSpellProto()->Id)
        {
            case 28851: // Flash of Light - need to create generic container for this and the above
            case 28853:
                DoneAdvertisedBenefit += i->GetModifier()->m_amount;
                break;
        }
    }

    for (auto i : GetScriptedLocationAuras(SCRIPT_LOCATION_SPELL_HEALING_DONE))
    {
        if (!i->isAffectedOnSpell(spellProto))
            continue;
        i->OnDamageCalculate(DoneAdvertisedBenefit, DoneTotalMod);
    }

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
        AuraList const& auraDummy = GetAurasByType(SPELL_AURA_DUMMY);
        for (auto i : auraDummy)
        {
            if (i->GetSpellProto()->SpellVisual == 300 && (i->GetSpellProto()->SpellFamilyFlags & uint64(0x0000000010000000)))
            {
                // Flash of Light
                if ((spellProto->SpellFamilyFlags & uint64(0x0000000000002000)) && i->GetEffIndex() == EFFECT_INDEX_1)
                    TakenTotal += i->GetModifier()->m_amount;
                // Holy Light
                else if ((spellProto->SpellFamilyFlags & uint64(0x0000000000004000)) && i->GetEffIndex() == EFFECT_INDEX_0)
                    TakenTotal += i->GetModifier()->m_amount;
            }
        }
    }

    for (auto i : GetScriptedLocationAuras(SCRIPT_LOCATION_SPELL_HEALING_TAKEN))
    {
        if (!i->isAffectedOnSpell(spellProto))
            continue;
        i->OnDamageCalculate(TakenAdvertisedBenefit, TakenTotalMod);
    }

    // apply benefit affected by spell power implicit coeffs and spell level penalties
    TakenTotal = pCaster->SpellBonusWithCoeffs(spellProto, TakenTotal, TakenAdvertisedBenefit, 0, damagetype, false);

    // Taken mods
    // Healing Wave cast
    if (spellProto->SpellFamilyName == SPELLFAMILY_SHAMAN && (spellProto->SpellFamilyFlags & uint64(0x0000000000000040)))
    {
        // Search for Healing Way on Victim
        AuraList const& auraDummy = GetAurasByType(SPELL_AURA_DUMMY);
        for (auto itr : auraDummy)
            if (itr->GetId() == 29203)
                TakenTotalMod *= (itr->GetModifier()->m_amount + 100.0f) / 100.0f;
    }


    // use float as more appropriate for negative values and percent applying
    float heal = (healamount + TakenTotal * int32(stack)) * TakenTotalMod;

    return heal < 0 ? 0 : uint32(heal);
}

int32 Unit::SpellBaseHealingBonusDone(SpellSchoolMask schoolMask)
{
    int32 AdvertisedBenefit = 0;

    AuraList const& mHealingDone = GetAurasByType(SPELL_AURA_MOD_HEALING_DONE);
    for (auto i : mHealingDone)
        if ((i->GetModifier()->m_miscvalue & schoolMask) != 0)
            AdvertisedBenefit += i->GetModifier()->m_amount;

    // Healing bonus of spirit, intellect and strength
    if (GetTypeId() == TYPEID_PLAYER)
    {
        // Healing bonus from stats
        AuraList const& mHealingDoneOfStatPercent = GetAurasByType(SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT);
        for (auto i : mHealingDoneOfStatPercent)
        {
            // 1.12.* have only 1 stat type support
            Stats usedStat = STAT_SPIRIT;
            AdvertisedBenefit += int32(GetStat(usedStat) * i->GetModifier()->m_amount / 100.0f);
        }
    }
    return AdvertisedBenefit;
}

int32 Unit::SpellBaseHealingBonusTaken(SpellSchoolMask schoolMask) const
{
    int32 AdvertisedBenefit = 0;
    AuraList const& mDamageTaken = GetAurasByType(SPELL_AURA_MOD_HEALING);
    for (auto i : mDamageTaken)
        if (i->GetModifier()->m_miscvalue & schoolMask)
            AdvertisedBenefit += i->GetModifier()->m_amount;

    return AdvertisedBenefit;
}

bool Unit::IsImmuneToDamage(SpellSchoolMask shoolMask)
{
    // If m_immuneToSchool type contain this school type, IMMUNE damage.
    SpellImmuneList const& schoolList = m_spellImmune[IMMUNITY_SCHOOL];
    for (auto itr : schoolList)
        if (itr.type & shoolMask)
            return true;

    // If m_immuneToDamage type contain magic, IMMUNE damage.
    SpellImmuneList const& damageList = m_spellImmune[IMMUNITY_DAMAGE];
    for (auto itr : damageList)
        if (itr.type & shoolMask)
            return true;

    return false;
}

bool Unit::IsImmuneToSpell(SpellEntry const* spellInfo, bool /*castOnSelf*/, uint8 effectMask)
{
    if (!spellInfo)
        return false;

    // TODO add spellEffect immunity checks!, player with flag in bg is immune to immunity buffs from other friendly players!
    // SpellImmuneList const& dispelList = m_spellImmune[IMMUNITY_EFFECT];

    SpellImmuneList const& dispelList = m_spellImmune[IMMUNITY_DISPEL];
    for (auto itr : dispelList)
        if (itr.type == spellInfo->Dispel)
            return true;

    {
        if (!spellInfo->HasAttribute(SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY) &&
                !spellInfo->HasAttribute(SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY))
        {
            bool isPositive = IsPositiveEffectMask(spellInfo, effectMask);
            SpellImmuneList const& schoolList = m_spellImmune[IMMUNITY_SCHOOL];
            for (auto itr : schoolList)
                if (!(itr.aura && IsPositiveEffectMask(itr.aura->GetSpellProto(), uint8(1 << (itr.aura->GetEffIndex() - 1))) && isPositive && !itr.aura->GetSpellProto()->HasAttribute(SPELL_ATTR_EX_UNAFFECTED_BY_SCHOOL_IMMUNE)) &&
                    (itr.type & GetSpellSchoolMask(spellInfo)))
                    return true;
        }
    }

    if (uint32 mechanic = spellInfo->Mechanic)
    {
        SpellImmuneList const& mechanicList = m_spellImmune[IMMUNITY_MECHANIC];
        for (auto itr : mechanicList)
            if (itr.type == mechanic)
                return true;

        AuraList const& immuneAuraApply = GetAurasByType(SPELL_AURA_MECHANIC_IMMUNITY_MASK);
        for (auto iter : immuneAuraApply)
            if (iter->GetModifier()->m_miscvalue & (1 << (mechanic - 1)))
                return true;
    }

    return false;
}

bool Unit::IsImmuneToSpellEffect(SpellEntry const* spellInfo, SpellEffectIndex index, bool /*castOnSelf*/) const
{
    if (spellInfo->HasAttribute(SPELL_ATTR_UNAFFECTED_BY_INVULNERABILITY))
        return false;

    // If m_immuneToEffect type contain this effect type, IMMUNE effect.
    uint32 effect = spellInfo->Effect[index];
    SpellImmuneList const& effectList = m_spellImmune[IMMUNITY_EFFECT];
    for (auto itr : effectList)
        if (itr.type == effect)
            return true;

    if (uint32 mechanic = spellInfo->EffectMechanic[index])
    {
        SpellImmuneList const& mechanicList = m_spellImmune[IMMUNITY_MECHANIC];
        for (auto itr : mechanicList)
            if (itr.type == mechanic)
                return true;

        AuraList const& immuneAuraApply = GetAurasByType(SPELL_AURA_MECHANIC_IMMUNITY_MASK);
        for (auto iter : immuneAuraApply)
            if (iter->GetModifier()->m_miscvalue & (1 << (mechanic - 1)))
                return true;
    }

    if (uint32 aura = spellInfo->EffectApplyAuraName[index])
    {
        SpellImmuneList const& list = m_spellImmune[IMMUNITY_STATE];
        for (auto itr : list)
            if (itr.type == aura)
                return true;
    }
    return false;
}

bool Unit::IsImmuneToSchool(SpellEntry const* spellInfo, uint8 effectMask) const
{
    if (!spellInfo->HasAttribute(SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY))           // can remove immune (by dispell or immune it)
    {
        SpellImmuneList const& schoolList = m_spellImmune[IMMUNITY_SCHOOL];
        for (auto itr : schoolList)
        {
            if (itr.aura && itr.aura->GetSpellProto() == spellInfo) // do not let itself immune out - fixes 39872 - Tidal Shield
                continue;

            if (!(itr.aura && IsPositiveEffect(itr.aura->GetSpellProto(), itr.aura->GetEffIndex()) && IsPositiveEffectMask(spellInfo, effectMask) && !itr.aura->GetSpellProto()->HasAttribute(SPELL_ATTR_EX_UNAFFECTED_BY_SCHOOL_IMMUNE)) &&
                (itr.type & GetSpellSchoolMask(spellInfo)))
                return true;
        }
    }

    return false;
}

/**
 * Calculates caster part of melee damage bonuses,
 * also includes different bonuses dependent from target auras
 */
uint32 Unit::MeleeDamageBonusDone(Unit* victim, uint32 pdamage, WeaponAttackType attType, SpellSchoolMask schoolMask, SpellEntry const* spellProto, DamageEffectType damagetype, uint32 stack, bool flat)
{
    if (!victim || pdamage == 0)
        return pdamage;

    // Some spells don't benefit from done mods
    if (spellProto && spellProto->HasAttribute(SPELL_ATTR_EX3_NO_DONE_BONUS))
        return pdamage;

    // differentiate for weapon damage based spells
    bool isWeaponDamageBasedSpell = !(spellProto && (damagetype == DOT || IsSpellHaveEffect(spellProto, SPELL_EFFECT_SCHOOL_DAMAGE)));
    Item*  pWeapon          = GetTypeId() == TYPEID_PLAYER ? ((Player*)this)->GetWeaponForAttack(attType, true, false) : nullptr;
    uint32 creatureTypeMask = victim->GetCreatureTypeMask();
    schoolMask              = ((spellProto && schoolMask != GetSpellSchoolMask(spellProto)) ? GetSpellSchoolMask(spellProto) : schoolMask);

    // FLAT damage bonus auras
    // =======================
    int32 DoneFlat  = 0;
    int32 APbonus   = 0;

    // ..done flat, already included in weapon damage based spells
    if (!isWeaponDamageBasedSpell || (spellProto && (schoolMask &~ SPELL_SCHOOL_MASK_NORMAL) != 0))
    {
        AuraList const& mModDamageDone = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE);
        for (auto i : mModDamageDone)
        {
            if (i->GetModifier()->m_miscvalue & schoolMask &&                              // schoolmask has to fit with the intrinsic spell school
                    ((i->GetSpellProto()->EquippedItemClass == -1) ||                      // general, weapon independent
                     (pWeapon && pWeapon->IsFitToSpellRequirements(i->GetSpellProto()))))  // OR used weapon fits aura requirements
            {
                DoneFlat += i->GetModifier()->m_amount;
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
        APbonus += victim->GetTotalAuraModifier(SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS);
        APbonus += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_RANGED_ATTACK_POWER_VERSUS, creatureTypeMask);
    }
    else
    {
        APbonus += victim->GetTotalAuraModifier(SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS);
        APbonus += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_MELEE_ATTACK_POWER_VERSUS, creatureTypeMask);
    }

    // PERCENT damage auras
    // ====================
    float DoneTotalMod   = 1.0f;

    // ..done pct, already included in weapon damage based spells
    if (!isWeaponDamageBasedSpell)
    {
        AuraList const& mModDamagePercentDone = GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
        for (auto i : mModDamagePercentDone)
        {
            if (i->GetModifier()->m_miscvalue & schoolMask &&                         // schoolmask has to fit with the intrinsic spell school
                i->GetModifier()->m_miscvalue & GetMeleeDamageSchoolMask(attType == BASE_ATTACK) &&         // AND schoolmask has to fit with weapon damage school (essential for non-physical spells)
                    ((i->GetSpellProto()->EquippedItemClass == -1) ||                     // general, weapon independent
                     (pWeapon && pWeapon->IsFitToSpellRequirements(i->GetSpellProto()))))  // OR used weapon fits aura requirements
            {
                DoneTotalMod *= (i->GetModifier()->m_amount + 100.0f) / 100.0f;
            }
        }

        if (attType == OFF_ATTACK)
            DoneTotalMod *= GetModifierValue(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_PCT);                    // no school check required
    }

    // ..done pct (by creature type mask)
    DoneTotalMod *= GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_DAMAGE_DONE_VERSUS, creatureTypeMask);

    // special dummys/class scripts and other effects
    // =============================================
    Unit* owner = GetOwner();
    if (!owner)
        owner = this;

    if (spellProto)
    {
        // dummies
        AuraList const& auraDummy = owner->GetAurasByType(SPELL_AURA_DUMMY);
        for (auto i : auraDummy)
        {
            if (!i->isAffectedOnSpell(spellProto))
                continue;
            switch (i->GetSpellProto()->Id)
            {
                case 20162: // Seal of the Crusader - all ranks
                case 20305:
                case 20306:
                case 20307:
                case 20308:
                case 21082:
                case 27158:
                {
                    DoneTotalMod *= 1.4f;
                    break;
                }
            }
        }

        // .. done (class scripts)
        AuraList const& mOverrideClassScript = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
        for (auto i : mOverrideClassScript)
        {
            if (!(i->isAffectedOnSpell(spellProto)))
                continue;
            switch (i->GetMiscValue())
            {
                // Dirty Deeds
                case 6427:
                case 6428:
                    if (victim->GetHealthPercent() < 35.f)
                    {
                        Aura* eff0 = i->GetHolder()->m_auras[EFFECT_INDEX_0];
                        if (!eff0)
                        {
                            sLog.outError("Spell structure of DD (%u) changed.", i->GetId());
                            continue;
                        }

                        // effect 0 have expected value but in negative state
                        DoneTotalMod *= (-eff0->GetModifier()->m_amount + 100.0f) / 100.0f;
                    }
                    break;
            }
        }
        for (auto i : GetScriptedLocationAuras(SCRIPT_LOCATION_MELEE_DAMAGE_DONE))
        {
            if (!i->isAffectedOnSpell(spellProto))
                continue;
            i->OnDamageCalculate(DoneFlat, DoneTotalMod);
        }
    }

    // final calculation
    // =================

    float DoneTotal = 0.0f;

    // scaling of non weapon based spells
    if (!isWeaponDamageBasedSpell || (spellProto && (schoolMask &~ SPELL_SCHOOL_MASK_NORMAL) !=0))
    {
        // apply ap bonus and benefit affected by spell power implicit coeffs and spell level penalties
        DoneTotal = SpellBonusWithCoeffs(spellProto, DoneTotal, DoneFlat, APbonus, damagetype, true);
    }
    // weapon damage based spells
    else if (isWeaponDamageBasedSpell && (APbonus || DoneFlat))
    {
        // have to add AP here to melee/spells because weapon damage is generic and this AP bonus is target specific
        bool normalized = spellProto ? IsSpellHaveEffect(spellProto, SPELL_EFFECT_NORMALIZED_WEAPON_DMG) : false;
        float baseValue = int32(APbonus / 14.0f * GetAPMultiplier(attType, normalized));

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
        baseValue *= GetModifierValue(unitMod, BASE_PCT);
        DoneTotal += DoneFlat + baseValue;
        DoneTotal *= GetModifierValue(unitMod, TOTAL_PCT);
    }

    if (!flat)
        DoneTotal = 0.0f;

    float tmpDamage = (int32(pdamage) + DoneTotal * int32(stack)) * DoneTotalMod;

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
uint32 Unit::MeleeDamageBonusTaken(Unit* caster, uint32 pdamage, WeaponAttackType attType, SpellSchoolMask schoolMask, SpellEntry const* spellProto, DamageEffectType damagetype, uint32 stack, bool flat)
{
    if (pdamage == 0)
        return pdamage;

    // Some spells don't benefit from taken mods
    if (spellProto && spellProto->HasAttribute(SPELL_ATTR_EX3_NO_DONE_BONUS))
        return pdamage;

    // differentiate for weapon damage based spells
    bool isWeaponDamageBasedSpell = !(spellProto && (damagetype == DOT || IsSpellHaveEffect(spellProto, SPELL_EFFECT_SCHOOL_DAMAGE)));
    schoolMask              = ((spellProto && schoolMask != GetSpellSchoolMask(spellProto)) ? GetSpellSchoolMask(spellProto) : schoolMask);

    // FLAT damage bonus auras
    // =======================
    int32 TakenFlat = 0;

    // ..taken flat (base at attack power for marked target and base at attack power for creature type)
    if (attType == RANGED_ATTACK)
        TakenFlat += GetTotalAuraModifier(SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN);
    else
        TakenFlat += GetTotalAuraModifier(SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN);

    // ..taken flat (by school mask)
    int32 TakenAdvertisedBenefit = GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_DAMAGE_TAKEN, schoolMask);

    // PERCENT damage auras
    // ====================
    float TakenTotalMod  = 1.0f;

    // ..taken pct (by school mask)
    TakenTotalMod *= GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, schoolMask);

    // ..taken pct (melee/ranged)
    if (attType == RANGED_ATTACK)
        TakenTotalMod *= GetTotalAuraMultiplier(SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT);
    else
        TakenTotalMod *= GetTotalAuraMultiplier(SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT);

    // special dummys/class scripts and other effects
    // =============================================

    if (spellProto)
    {
        for (auto i : GetScriptedLocationAuras(SCRIPT_LOCATION_MELEE_DAMAGE_TAKEN))
        {
            if (!i->isAffectedOnSpell(spellProto))
                continue;
            i->OnDamageCalculate(TakenAdvertisedBenefit, TakenTotalMod);
        }
    }

    // final calculation
    // =================

    // scaling of non weapon based spells
    if (!isWeaponDamageBasedSpell || (spellProto && (schoolMask &~ SPELL_SCHOOL_MASK_NORMAL) != 0))
    {
        // apply benefit affected by spell power implicit coeffs and spell level penalties
        if (caster)
            TakenAdvertisedBenefit = caster->SpellBonusWithCoeffs(spellProto, 0, TakenAdvertisedBenefit, 0, damagetype, false);
    }

    if (!flat)
        TakenFlat = 0.0f;

    float tmpDamage = (int32(pdamage) + (TakenFlat + TakenAdvertisedBenefit) * int32(stack)) * TakenTotalMod;

    // bonus result can be negative
    return tmpDamage > 0 ? uint32(tmpDamage) : 0;
}

void Unit::ApplySpellImmune(Aura const* aura, uint32 op, uint32 type, bool apply)
{
    if (apply)
    {
        for (SpellImmuneList::iterator itr = m_spellImmune[op].begin(), next; itr != m_spellImmune[op].end(); itr = next)
        {
            next = itr; ++next;
            if (itr->type == type && (!aura || itr->aura == aura))
            {
                m_spellImmune[op].erase(itr);
                next = m_spellImmune[op].begin();
            }
        }
        SpellImmune Immune;
        Immune.aura = aura;
        Immune.type = type;
        m_spellImmune[op].push_back(Immune);
    }
    else
    {
        for (SpellImmuneList::iterator itr = m_spellImmune[op].begin(); itr != m_spellImmune[op].end(); ++itr)
        {
            if (itr->aura == aura && (aura || itr->type == type))
            {
                m_spellImmune[op].erase(itr);
                break;
            }
        }
    }
}

void Unit::ApplySpellDispelImmunity(const Aura* aura, DispelType type, bool apply)
{
    ApplySpellImmune(aura, IMMUNITY_DISPEL, type, apply);

    if (apply && aura->GetSpellProto()->HasAttribute(SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY))
        RemoveAurasWithDispelType(type);
}

float Unit::GetWeaponProcChance() const
{
    // normalized proc chance for weapon attack speed
    // (odd formula...)
    if (isAttackReady(BASE_ATTACK))
        return (GetAttackTime(BASE_ATTACK) * 1.8f / 1000.0f);
    if (hasOffhandWeaponForAttack() && isAttackReady(OFF_ATTACK))
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

bool Unit::Mount(uint32 displayid, const Aura* aura/* = nullptr*/)
{
    // Custom mount (non-aura such as taxi or command) overwrites aura mounts
    if (!displayid || (IsMounted() && aura && uint32(aura->GetAmount()) != GetMountID()))
        return false;

    RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_MOUNTING);
    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, displayid);
    return true;
}

bool Unit::Unmount(const Aura* aura/* = nullptr*/)
{
    // Custom mount (non-aura such as taxi or command) overwrites aura mounts, do not dismount on aura removal
    if (!IsMounted() || (aura && uint32(aura->GetAmount()) != GetMountID()))
        return false;

    RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_NOT_MOUNTED);
    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);
    return true;
}

void Unit::SetInCombatWith(Unit* enemy)
{
    SetInCombatState(enemy && enemy->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED), enemy);
}

void Unit::SetInCombatWithAggressor(Unit* aggressor, bool touchOnly/* = false*/)
{
    // This is a wrapper for SetInCombatWith initially created to improve PvP timers responsiveness. Can be extended in the future for broader use.
    if (!aggressor)
        return;

    // PvP combat participation pulse: refresh pvp timers on pvp combat (we are the victim)
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
    {
        if (aggressor->IsPvP() && aggressor->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        {
            // Use beneficiary for detecting own player (pet's pets forced into combat is not counted as direct pvp action)
            if (Player* thisPlayer = GetBeneficiaryPlayer())
            {
                // Use controlling for detecting aggressor's player (forced into combat by pet's pets is counted as direct pvp action)
                if (Player const* aggressorPlayer = aggressor->GetControllingPlayer())
                {
                    if (thisPlayer != aggressorPlayer && !thisPlayer->IsInDuelWith(aggressorPlayer))
                    {
                        thisPlayer->pvpInfo.inPvPCombat = (thisPlayer->pvpInfo.inPvPCombat || !touchOnly);
                        thisPlayer->UpdatePvP(true);
                    }
                }
            }
        }
    }

    if (!touchOnly)
        SetInCombatWith(aggressor);
}

void Unit::SetInCombatWithAssisted(Unit* assisted, bool touchOnly/* = false*/)
{
    // This is a wrapper for SetInCombatWith initially created to improve PvP timers responsiveness. Can be extended in the future for broader use.

    if (!assisted)
        return;

    bool pvp = false;

    // PvP combat participation pulse: refresh pvp timers on pvp combat (we are the assister)
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
    {
        if (assisted->IsPvP() && assisted->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        {
            // Use beneficiary for detecting own player (pet's pets entering combat is not counted as assist)
            if (Player* thisPlayer = GetBeneficiaryPlayer())
            {
                // Use controlling for detecting asissted unit's player (entering combat with pet's pets is counted as assist)
                if (Player const* assistedPlayer = assisted->GetControllingPlayer())
                {
                    if (thisPlayer != assistedPlayer)
                    {
                        if (assistedPlayer->pvpInfo.inPvPCombat)
                            thisPlayer->pvpInfo.inPvPCombat = (thisPlayer->pvpInfo.inPvPCombat || !touchOnly);

                        thisPlayer->UpdatePvP(true);

                        if (assistedPlayer->IsPvPContested())
                            thisPlayer->UpdatePvPContested(true);

                        pvp = true;
                    }
                }
            }
        }
    }

    if (!touchOnly)
        SetInCombatState(pvp);
}

void Unit::SetInCombatWithVictim(Unit* victim, bool touchOnly/* = false*/)
{
    // This is a wrapper for SetInCombatWith initially created to improve PvP timers responsiveness. Can be extended in the future for broader use.
    if (!victim)
        return;

    // PvP combat participation pulse: refresh pvp timers on pvp combat (we are the aggressor)
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
    {
        if (victim->IsPvP() && victim->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        {
            // Use beneficiary for detecting own player (pet's pets entering combat is not an act of aggression)
            if (Player* thisPlayer = GetBeneficiaryPlayer())
            {
                // Use controlling for detecting victim's player (entering combat with pet's pets is an act of aggression)
                if (Player const* victimPlayer = victim->GetControllingPlayer())
                {
                    if (thisPlayer != victimPlayer && !thisPlayer->IsInDuelWith(victimPlayer))
                    {
                        thisPlayer->pvpInfo.inPvPCombat = (thisPlayer->pvpInfo.inPvPCombat || !touchOnly);
                        thisPlayer->UpdatePvP(true);
                        thisPlayer->UpdatePvPContested(true);
                    }
                }
            }
        }
    }

    if (!touchOnly)
        SetInCombatWith(victim);
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
    if (!IsAlive())
        return;

    bool notInCombat = !HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
    bool creatureNotInCombat = GetTypeId() == TYPEID_UNIT && notInCombat;

    // For player itself and his pet during pvp combat enable own combat timer
    if (PvP || creatureNotInCombat)
        GetCombatManager().TriggerCombatTimer(PvP);

    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

    if (HasCharmer() || !GetOwnerGuid().IsEmpty())
    {
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT);
        if (enemy && notInCombat)
        {
            // TODO: Unify this combat propagation with linking combat propagation in threat system
            Unit* controller = GetMaster();

            if (controller && controller->CanJoinInAttacking(enemy))
            {
                if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
                {
                    controller->SetInCombatWith(enemy); // player only enters combat
                    enemy->AddThreat(controller);
                }
                else if (controller->IsInCombat())
                {
                    controller->AddThreat(enemy);
                    enemy->AddThreat(controller);
                    enemy->SetInCombatWith(controller);
                    if (PvP || creatureNotInCombat)
                        enemy->GetCombatManager().TriggerCombatTimer(controller);
                }
                else
                    controller->AI()->AttackStart(enemy);
            }
        }
    }

    // interrupt all delayed non-combat casts
    for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; ++i)
        if (Spell* spell = GetCurrentSpell(CurrentSpellTypes(i)))
            if (IsNonCombatSpell(spell->m_spellInfo))
                InterruptSpell(CurrentSpellTypes(i), false);

    if (creatureNotInCombat)
    {
        Creature* creature = static_cast<Creature*>(this);

        m_damageByOthers = 0;

        // clear stand state if set in addon - else script has to do it on its own
        CreatureDataAddon const* cainfo = creature->GetCreatureAddon();
        if (cainfo)
        {
            if (getStandState() == cainfo->bytes1)
                SetStandState(UNIT_STAND_STATE_STAND);
            if (cainfo->emote && GetUInt32Value(UNIT_NPC_EMOTESTATE) == cainfo->emote)
                HandleEmoteState(0);
        }

        if (creature->GetCombatStartPosition().IsEmpty())
            creature->SetCombatStartPosition(GetPosition(GetTransport()));

        if (!creature->CanAggro()) // if creature aggroed during initial ignoration period, clear the state
        {
            creature->SetCanAggro(true);
            AbortAINotifyEvent();
        }

        if (creature->AI())
            creature->AI()->EnterCombat(enemy);

        // Some bosses are set into combat with zone
        if (GetMap()->IsDungeon() && (creature->GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_AGGRO_ZONE) && enemy && enemy->IsControlledByPlayer())
            creature->SetInCombatWithZone();

        if (InstanceData* mapInstance = GetInstanceData())
            mapInstance->OnCreatureEnterCombat(creature);

        TriggerAggroLinkingEvent(enemy);
    }
}

void Unit::EngageInCombatWith(Unit* enemy)
{
    MANGOS_ASSERT(enemy);
    AddThreat(enemy);
    SetInCombatWith(enemy);
    enemy->SetInCombatWith(this);
    GetCombatManager().TriggerCombatTimer(enemy);
}

void Unit::EngageInCombatWithAggressor(Unit* aggressor)
{
    MANGOS_ASSERT(aggressor);
    SetInCombatWithAggressor(aggressor);
    aggressor->SetInCombatWithAggressor(this);
    GetCombatManager().TriggerCombatTimer(aggressor);
}

void Unit::ClearInCombat()
{
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT);

    if (GetTypeId() == TYPEID_PLAYER)
        static_cast<Player*>(this)->pvpInfo.inPvPCombat = false;
}

void Unit::HandleExitCombat(bool pvpCombat)
{
    if (AI() && !GetClientControlling())
        AI()->EnterEvadeMode();
    else
        CombatStop(false, !pvpCombat);

    CallForAllControlledUnits([](Unit* unit) { unit->HandleExitCombat(); }, CONTROLLED_PET | CONTROLLED_GUARDIANS | CONTROLLED_CHARM | CONTROLLED_TOTEMS);
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

bool Unit::IsVisibleForOrDetect(Unit const* u, WorldObject const* viewPoint, bool detect, bool /*inVisibleList*/, bool is3dDistance, bool spell) const
{
    if (!u || !IsInMap(u))
        return false;

    // Always can see self
    if (u == this)
        return true;

    // player visible for other player if not logout and at same transport
    // including case when player is out of world
    bool at_same_transport = GetTransport() && GetTransport() == u->GetTransport();

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
        if (!u->IsAlive())
            detect = false;
    }
    else
    {
        // all dead creatures/players not visible for any creatures
        if (!u->IsAlive() || !IsAlive())
            return false;
    }

    // Any units far than max visible distance for viewer or not in our map are not visible too
    if (!at_same_transport) // distance for show player/pet/creature (no transport case)
    {
        if (!IsWithinDistInMap(viewPoint, u->GetVisibilityData().GetVisibilityDistanceFor((WorldObject *)this), is3dDistance))
            return false;
    }

    // always seen by owner
    if (GetMasterGuid() == u->GetObjectGuid())
        return true;

    // isInvisibleForAlive() those units can only be seen by dead or if other
    // unit is also invisible for alive.. if an isinvisibleforalive unit dies we
    // should be able to see it too
    if (u->IsAlive() && IsAlive() && isInvisibleForAlive() != u->isInvisibleForAlive())
        if (u->GetTypeId() != TYPEID_PLAYER || !((Player*) u)->IsGameMaster())
            return false;

    // Visible units, always are visible for all units
    // Prior to TBC, invisible units could see visible units without restriction
    if (m_Visibility == VISIBILITY_ON)
        return true;

    // GMs see any players, not higher GMs and all units
    if (u->GetTypeId() == TYPEID_PLAYER && ((Player*) u)->IsGameMaster())
    {
        if (GetTypeId() == TYPEID_PLAYER)
            return ((Player*)this)->GetSession()->GetSecurity() <= ((Player*)u)->GetSession()->GetSecurity();
        return true;
    }

    // non faction visibility non-breakable for non-GMs
    if (m_Visibility == VISIBILITY_OFF)
        return false;

    // Spell check for travelling spells - stealth and invisibility are ignored - when creature was able to target it, it should go through
    if (spell)
        return true;

    // check group settings
    if (IsFogOfWarVisibleStealth(u))
        return true;

    // raw invisibility
    bool invisible = (GetVisibilityData().GetInvisibilityMask() != 0 || u->GetVisibilityData().GetInvisibilityMask() != 0);

    if (u->GetTypeId() == TYPEID_PLAYER) // if object is player with mover, use its visibility masks, so that an invisible player MCing a creature can see stuff
        if (Unit* mover = ((Player*)u)->GetMover())
            invisible = (GetVisibilityData().GetInvisibilityMask() != 0 || mover->GetVisibilityData().GetInvisibilityMask() != 0);

    // detectable invisibility case
    if (invisible && (
                // Invisible units, always are visible for unit that can detect this invisibility (have appropriate level for detect)
                u->GetVisibilityData().CanDetectInvisibilityOf(this))
                // Prior to TBC units with invisibility detection and units with same invisibility type are not always visible for units that can be detected
                )
    {
        invisible = false;
    }

    // special cases for always overwrite invisibility/stealth
    if (invisible || (m_Visibility == VISIBILITY_GROUP_STEALTH || m_Visibility == VISIBILITY_GROUP_NO_DETECT))
    {
        if (u->CanAttack(this)) // Hunter mark functionality
            if (HasAuraTypeWithCaster(SPELL_AURA_MOD_STALKED, u->GetObjectGuid()))
                return true;

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

    // If a mob or player is stunned he will not be able to detect stealth
    if (u->IsStunned() && (u != this))
        return false;

    // set max distance
    float visibleDistance = (u->GetTypeId() == TYPEID_PLAYER) ? MAX_PLAYER_STEALTH_DETECT_RANGE : ((Creature const*)u)->GetAttackDistance(this);

    // Always invisible from back (when stealth detection is on), also filter max distance cases
    bool isInFront = viewPoint->isInFrontInMap(this, visibleDistance);
    if (!isInFront)
        return false;

    // In TBC+ this is safeguarded by 228 aura
    visibleDistance = GetVisibilityData().GetStealthVisibilityDistance(u, HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED));

    // recheck new distance
    if (visibleDistance <= 0 || GetDistance(viewPoint, true, DIST_CALC_NONE) > visibleDistance* visibleDistance)
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

            if (!owner || !IsVisibleForOrDetect(owner, this, false))
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

SpellSchoolMask Unit::GetMainAttackSchoolMask()
{
    if (UnitAI* ai = AI())
        return ai->GetMainAttackSchoolMask();
    else
        return GetMeleeDamageSchoolMask();
}

void Unit::SetVisibility(UnitVisibility x)
{
    m_Visibility = x;

    if (IsInWorld())
        UpdateVisibilityAndView();
}

void Unit::UpdateSpeed(UnitMoveType mtype, bool forced, float ratio)
{
    int32 main_speed_mod  = 0;
    float stack_bonus     = 1.0f;
    float non_stack_bonus = 1.0f;

    switch (mtype)
    {
        case MOVE_WALK:
            break;
        case MOVE_RUN:
        {
            if (IsMounted() && !IsTaxiFlying()) // Use on mount auras
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
            break;
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
            if (GetTypeId() == TYPEID_PLAYER)
            {
                if (int32 normalization = GetMaxPositiveAuraModifier(SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED))
                {
                    // Use speed from aura
                    float max_speed = normalization / baseMoveSpeed[mtype];
                    if (speed > max_speed)
                        speed = max_speed;
                }
            }
            break;
        }
        default:
            break;
    }

    // for player case, we look for some custom rates
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (GetDeathState() == CORPSE)
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
                speed *= m_baseSpeedRun;
                break;
            case MOVE_WALK:
                speed *= m_baseSpeedWalk;
                break;
            default:
                break;
        }
    }

    SetSpeedRate(mtype, speed * ratio, forced);
}

float Unit::GetSpeedInMotion() const
{
    return (movespline->Finalized() ? GetSpeed(m_movementInfo.GetSpeedType()) : movespline->Speed());
}

float Unit::GetSpeed(UnitMoveType mtype) const
{
    return m_speed_rate[mtype] * baseMoveSpeed[mtype];
}

float Unit::GetSpeedRateInMotion() const
{
    const UnitMoveType type = m_movementInfo.GetSpeedType();
    return (movespline->Finalized() ? GetSpeedRate(type) : (movespline->Speed() / GetSpeed(type)));
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
        BreakCharmOutgoing();
        BreakCharmIncoming();
        RemoveMiniPet();
        UnsummonAllTotems();

        StopMoving(true);
        i_motionMaster.Clear(false, true);
        if (!CanFly() || !i_motionMaster.MoveFall())
            i_motionMaster.MoveIdle();

        GetCombatManager().StopEvade();

        ModifyAuraState(AURA_STATE_HEALTHLESS_20_PERCENT, false);
        // remove aurastates allowing special moves
        ClearAllReactives();
        ClearDiminishings();
    }
    else if (s == JUST_ALIVED)
    {
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);  // clear skinnable for creature and player (at battleground)
        ScheduleAINotify(GetTypeId() == TYPEID_UNIT ? sWorld.getConfig(CONFIG_UINT32_CREATURE_RESPAWN_AGGRO_DELAY) : 0, true);
    }

    if (s != ALIVE && s != JUST_ALIVED)
    {
        CombatStop();
        ClearComboPointHolders();                           // any combo points pointed to unit lost at it death

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
        if (target.IsEnemy(charmer))
            return true;
    }

    return false;
}

bool Unit::CanHaveThreatList(bool ignoreAliveState/*=false*/) const
{
    // only alive units can have threat list
    if (!IsAlive() && !ignoreAliveState)
        return false;

    if (GetTypeId() == TYPEID_PLAYER) // only charmed players have a threat list
        return !HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

    Creature const* creature = static_cast<Creature const*>(this);

    // totems can not have threat list
    if (creature->IsTotem() && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        return false;

    // pets can not have a threat list, unless they are controlled by a creature
    if (creature->IsPet() && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
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
    getHostileRefManager().deleteReferences();
}

//======================================================================

void Unit::TauntUpdate()
{
    if (!CanHaveThreatList())
        return;

    getThreatManager().TauntUpdate();
}

void Unit::FixateTarget(Unit* taunter)
{
    if (!CanHaveThreatList())
        return;

    getThreatManager().FixateTarget(taunter);
}

//======================================================================

bool Unit::SelectHostileTarget()
{
    // function provides main threat functionality
    // next-victim-selection algorithm and evade mode are called
    // threat list sorting etc.

    if (!this->IsAlive())
        return false;

    // This function only useful once AI has been initialized
    if (!AI())
        return false;

    if (!AI()->CanExecuteCombatAction())
    {
        if (Unit* target = GetMap()->GetUnit(GetTargetGuid()))
            if (target != this)
                SetInFront(target);

        return !((AI()->GetCombatScriptStatus() || IsStunned()) && getThreatManager().isThreatListEmpty());
    }

    Unit* target = nullptr;
    Unit* oldTarget = GetVictim();

    if (!target && !getThreatManager().isThreatListEmpty())
        target = getThreatManager().getHostileTarget();

    if (!target && oldTarget)
    {
        if (IsIgnoringRangedTargets())
        {
            if (CanReachWithMeleeAttack(oldTarget))
                target = oldTarget;
            else
            {
                m_attacking = nullptr;
                SetTargetGuid(ObjectGuid());
                oldTarget->_removeAttacker(this);
            }
        }
    }

    if (target)
    {
        if (!AI()->IsTargetingRestricted())
        {
            // needs a much better check, seems to cause quite a bit of trouble
            SetInFront(target);

            if (oldTarget != target || GetTarget() != target)
                AI()->AttackStart(target);
        }

        // check if currently selected target is reachable
        // NOTE: path alrteady generated from AttackStart()
        if (AI()->IsCombatMovement())
        {
            if (!GetMotionMaster()->GetCurrent()->IsReachable())
            {
                if (!GetCombatManager().IsInEvadeMode())
                    GetCombatManager().StartEvadeTimer();
            }
            else if(GetCombatManager().IsInEvadeMode())
                GetCombatManager().StopEvade();
        }

        return true;
    }
    if (IsIgnoringRangedTargets() && !getThreatManager().isThreatListEmpty())
        return true;
    // return true but no target

    // no target but something prevent go to evade mode
    if (!IsInCombat())
        return false;

    // last case when creature don't must go to evade mode:
    // it in combat but attacker not make any damage and not enter to aggro radius to have record in threat list
    // for example at owner command to pet attack some far away creature
    // Note: creature not have targeted movement generator but have attacker in this case
    if (GetMotionMaster()->GetCurrentMovementGeneratorType() != CHASE_MOTION_TYPE)
    {
        for (auto m_attacker : m_attackers)
        {
            if (m_attacker->IsInMap(this) && CanAttack(m_attacker) && m_attacker->isInAccessablePlaceFor(this))
                return false;
        }
    }

    // enter in evade mode in other case
    FixateTarget(nullptr);
    AI()->EnterEvadeMode();

    return false;
}

//======================================================================
//======================================================================
//======================================================================

DiminishingLevels Unit::GetDiminishing(DiminishingGroup group)
{
    for (auto& i : m_Diminishing)
    {
        if (i.DRGroup != group)
            continue;

        if (!i.hitCount)
            return DIMINISHING_LEVEL_1;

        if (!i.hitTime)
            return DIMINISHING_LEVEL_1;

        const bool pvp = (GetTypeId() == TYPEID_PLAYER);
        const bool diminished = IsDiminishingReturnsGroupDurationDiminished(group, pvp);

        // If enough time has passed sinc the last spell from this group was casted - reset the count
        if (diminished && i.stack == 0 && WorldTimer::getMSTimeDiff(i.hitTime, WorldTimer::getMSTime()) > (15 * IN_MILLISECONDS))
        {
            i.hitCount = DIMINISHING_LEVEL_1;
            return DIMINISHING_LEVEL_1;
        }
        // or else increase the count.
        return DiminishingLevels(i.hitCount);
    }
    return DIMINISHING_LEVEL_1;
}

void Unit::IncrDiminishing(DiminishingGroup group, uint32 duration, bool pvp)
{
    const bool diminished = IsDiminishingReturnsGroupDurationDiminished(group, pvp);

    // Checking for existing in the table
    for (auto& i : m_Diminishing)
    {
        if (i.DRGroup != group)
            continue;

        if (!diminished)
            i.hitCount += (i.hitCount < std::numeric_limits<uint32>::max() ? 1 : 0);
        else if (i.hitCount < DIMINISHING_LEVEL_IMMUNE)
            i.hitCount += 1;
        else
            i.hitCount = DIMINISHING_LEVEL_IMMUNE;

        return;
    }
    m_Diminishing.push_back(DiminishingReturn(group, WorldTimer::getMSTime(), DIMINISHING_LEVEL_2, duration));
}

void Unit::ApplyDiminishingToDuration(DiminishingGroup group, int32& duration, Unit* caster, DiminishingLevels Level, bool isReflected)
{
    if (duration == -1 || group == DIMINISHING_NONE || (!isReflected && caster->CanAssist(this)))
        return;

    float mod = 1.0f;

    const bool pvp = (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED));

    // Some diminishings applies to mobs too (for example, Stun)
    if ((GetDiminishingReturnsGroupType(group) == DRTYPE_PLAYER && pvp) || GetDiminishingReturnsGroupType(group) == DRTYPE_ALL)
    {
        if (IsDiminishingReturnsGroupDurationDiminished(group, pvp))
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
    }

    duration = int32(duration * mod);
}

void Unit::ApplyDiminishingAura(DiminishingGroup group, bool apply)
{
    // Checking for existing in the table
    for (auto& i : m_Diminishing)
    {
        if (i.DRGroup != group)
            continue;

        if (apply)
            i.stack += 1;
        else if (i.stack)
        {
            i.stack -= 1;
            // Remember time after last aura from group removed
            if (i.stack == 0)
                i.hitTime = WorldTimer::getMSTime();
        }
        break;
    }
}

bool Unit::isVisibleForInState(Player const* u, WorldObject const* viewPoint, bool inVisibleList) const
{
    return IsVisibleForOrDetect(u, viewPoint, false, inVisibleList, false);
}

/// returns true if creature can't be seen by alive units
bool Unit::isInvisibleForAlive() const
{
    if (m_AuraFlags & UNIT_AURAFLAG_ALIVE_INVISIBLE)
        return true;
    // TODO: maybe spiritservices also have just an aura
    return isSpiritService();
}

bool Unit::IsSuppressedTarget(Unit* target) const
{
    MANGOS_ASSERT(target);

    if (target->IsImmuneToDamage(GetMeleeDamageSchoolMask()))
        return true;

    if (target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED))
        return true;

    if (target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED) && target->HasDamageInterruptibleStunAura())
        return true;

    if (target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING) && target->HasAuraType(SPELL_AURA_MOD_FEAR))
        return true;

    return false;
}

bool Unit::IsOfflineTarget(Unit* victim) const
{
    if (IsFeigningDeathSuccessfully())
        return true;

    if (!CanAttack(victim))
        return true;

    return false;
}

uint32 Unit::GetCreatureType() const
{
    if (GetTypeId() == TYPEID_PLAYER)
    {
        SpellShapeshiftFormEntry const* ssEntry = sSpellShapeshiftFormStore.LookupEntry(GetShapeshiftForm());
        if (ssEntry && ssEntry->creatureType > 0)
            return ssEntry->creatureType;
        return CREATURE_TYPE_HUMANOID;
    }
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

int32 Unit::GetTotalResistanceValue(SpellSchools school) const
{
    UnitMods unitMod = UnitMods(UNIT_MOD_RESISTANCE_START + school);

    if (m_auraModifiersGroup[unitMod][TOTAL_PCT] <= 0.0f)
        return 0.0f;

    // value = ((base_value * base_pct) + total_value) * total_pct
    float value = GetCreateResistance(school);

    const bool vulnerability = (value < 0);

    value += m_auraModifiersGroup[unitMod][BASE_VALUE];
    value *= m_auraModifiersGroup[unitMod][BASE_PCT];
    value += m_auraModifiersGroup[unitMod][TOTAL_VALUE];
    value *= m_auraModifiersGroup[unitMod][TOTAL_PCT];

    // Auras can't cause resistances to dip below 0 since early vanilla
    // PS: Actually, they can, but only visually advertised in the fields, calculations ignore it, we limit both
    if (value < 0 && !vulnerability)
        value = 0;

    return int32(value);
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
    int32 ap = GetInt32Value(UNIT_FIELD_ATTACK_POWER) + GetInt32Value(UNIT_FIELD_ATTACK_POWER_MODS);
    if (ap < 0)
        return 0.0f;
    return ap * (1.0f + GetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER));
}

float Unit::GetBaseWeaponDamage(WeaponAttackType attType, WeaponDamageRange damageRange, uint8 index) const
{
    if (attType == OFF_ATTACK && !hasOffhandWeaponForAttack())
        return 0.0f;

    return (m_weaponDamageInfo.weapon[attType].damage[index].value[damageRange] * m_modAttackBaseDPSPct[attType]);
}

SpellSchoolMask Unit::GetAttackDamageSchoolMask(WeaponAttackType attType, bool first /*= false*/) const
{
    if (first)
        return SpellSchoolMask(1 << (m_weaponDamageInfo.weapon[attType].damage[0].school));

    uint32 mask = SPELL_SCHOOL_MASK_NONE;
    for (uint8 i = 0; i < m_weaponDamageInfo.weapon[attType].lines; i++)
        mask |= (1 << m_weaponDamageInfo.weapon[attType].damage[i].school);
    return SpellSchoolMask(mask);
}

void Unit::SetAttackDamageSchool(WeaponAttackType attType, SpellSchools school)
{
    for (uint8 i = 0; i < m_weaponDamageInfo.weapon[attType].lines; i++)
        m_weaponDamageInfo.weapon[attType].damage[i].school = school;
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
    else if (HasCharmer())
    {
        Unit* charmer = GetCharmer();
        if (charmer && (charmer->GetTypeId() == TYPEID_PLAYER) && ((Player*)charmer)->GetGroup())
            ((Player*)charmer)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_CUR_HP);
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
    else if (HasCharmer())
    {
        Unit* charmer = GetCharmer();
        if (charmer && (charmer->GetTypeId() == TYPEID_PLAYER) && ((Player*)charmer)->GetGroup())
            ((Player*)charmer)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_MAX_HP);
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
    else if (HasCharmer())
    {
        Unit* charmer = GetCharmer();
        if (charmer && (charmer->GetTypeId() == TYPEID_PLAYER) && ((Player*)charmer)->GetGroup())
            ((Player*)charmer)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_CUR_POWER);
    }

    if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->IsPet())
    {
        Pet* pet = (Pet*)this;
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
    else if (HasCharmer())
    {
        Unit* charmer = GetCharmer();
        if (charmer && (charmer->GetTypeId() == TYPEID_PLAYER) && ((Player*)charmer)->GetGroup())
            ((Player*)charmer)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_MAX_POWER);
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
    else if (HasCharmer())
    {
        Unit* charmer = GetCharmer();
        if (charmer && (charmer->GetTypeId() == TYPEID_PLAYER) && ((Player*)charmer)->GetGroup())
            ((Player*)charmer)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_CUR_POWER);
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
    else if (HasCharmer())
    {
        Unit* charmer = GetCharmer();
        if (charmer && (charmer->GetTypeId() == TYPEID_PLAYER) && ((Player*)charmer)->GetGroup())
            ((Player*)charmer)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_MAX_POWER);
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
    WorldObject::AddToWorld();
    ScheduleAINotify(GetTypeId() == TYPEID_UNIT && !HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) ? sWorld.getConfig(CONFIG_UINT32_CREATURE_RESPAWN_AGGRO_DELAY) : 0);
}

void Unit::RemoveFromWorld()
{
    // cleanup
    if (IsInWorld())
    {
        CombatStop();
        RemoveNotOwnTrackedTargetAuras();
        BreakCharmOutgoing();
        BreakCharmIncoming();
        RemoveGuardians();
        RemoveMiniPet();
        RemoveAllGameObjects();
        RemoveAllDynObjects();
        GetViewPoint().Event_RemovedFromWorld();

        if (!IsPlayer())
            if (GenericTransport* transport = GetTransport())
                transport->RemovePassenger(this);

        m_FollowingRefManager.clearReferences();
    }

    Object::RemoveFromWorld();
}

void Unit::CleanupsBeforeDelete()
{
    if (m_uint32Values)                                     // only for fully created object
    {
        InterruptNonMeleeSpells(true);
        m_events.KillAllEvents(false);                      // non-delatable (currently casted spells) will not deleted now but it will deleted at call in Map::RemoveAllObjectsInRemoveList
        ClearComboPointHolders();
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
    m_CommandState(COMMAND_FOLLOW),
    m_petnumber(0), m_opener(0), m_openerMinRange(0),
    m_openerMaxRange(0), m_unitFieldFlags(0), m_unitFieldBytes2_1(0), m_retreating(false), m_stayPosSet(false),
    m_stayPosX(0), m_stayPosY(0), m_stayPosZ(0),
    m_stayPosO(0), m_walk(true), m_deleted(false)
{
    for (auto& m_charmspell : m_charmspells)
        m_charmspell.SetActionAndType(0, ACT_DISABLED);
}

CharmInfo::~CharmInfo()
{
    delete m_combatData;
    delete m_ai;
    m_deleted = true;
}

void CharmInfo::SetCharmState(std::string const& ainame, bool withNewThreatList /*= true*/)
{
    SetCharmState(((!ainame.empty()) ? FactorySelector::GetSpecificAI(m_unit, ainame) : nullptr), withNewThreatList);
}

void CharmInfo::SetCharmState(UnitAI* ai, bool withNewThreatList /*= true*/)
{
    m_ai = ai;

    if (withNewThreatList)
        m_combatData = new CombatData(m_unit);

    // Save specific pre-charm flags
    // Save entire bytes2_1
    m_unitFieldBytes2_1 = m_unit->GetByteValue(UNIT_FIELD_BYTES_2, 1);
    // Save unit flags
    m_unitFieldFlags = m_unit->GetUInt32Value(UNIT_FIELD_FLAGS);
    SetWalk(m_unit->IsWalking());
}

void CharmInfo::ResetCharmState()
{
    // Restore specific pre-charm flags
    // Restore entire bytes2_1
    m_unit->SetByteValue(UNIT_FIELD_BYTES_2, 1, m_unitFieldBytes2_1);
    // Restore unit flags
    uint32 savedUnitFlags = m_unitFieldFlags;
    if (savedUnitFlags & UNIT_FLAG_PLAYER_CONTROLLED)
        m_unit->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    else
        m_unit->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

    if (savedUnitFlags & UNIT_FLAG_IMMUNE_TO_PLAYER)
        m_unit->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
    else
        m_unit->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);

    if (savedUnitFlags & UNIT_FLAG_IMMUNE_TO_NPC)
        m_unit->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
    else
        m_unit->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);

    if (savedUnitFlags & UNIT_FLAG_PVP)
        m_unit->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP);
    else
        m_unit->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP);

    m_unit->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT);

    if (m_unit->IsCreature())
        static_cast<Creature*>(m_unit)->SetWalk(GetWalk(), true);
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

void CharmInfo::ProcessUnattackableTargets()
{
    // if after faction change and combat init cant attack target, remove it
    if (!m_unit->getThreatManager().getThreatList().empty()) // threat list case
    {
        Unit::AttackerSet friendlyTargets;
        for (auto itr = m_unit->getThreatManager().getThreatList().begin(); itr != m_unit->getThreatManager().getThreatList().end(); ++itr)
        {
            Unit* attacker = (*itr)->getTarget();
            if (attacker->GetTypeId() != TYPEID_UNIT)
                continue;

            if (!m_unit->CanAttack(attacker))
                friendlyTargets.insert(attacker);
        }

        for (auto attacker : friendlyTargets)
        {
            attacker->AttackStop(true, true);
            attacker->getThreatManager().modifyThreatPercent(m_unit, -101);
        }
    }
    else // attacker set case
    {
        Unit::AttackerSet friendlyTargets;
        for (Unit::AttackerSet::const_iterator itr = m_unit->getAttackers().begin(); itr != m_unit->getAttackers().end(); ++itr)
        {
            Unit* attacker = (*itr);
            if (attacker->GetTypeId() != TYPEID_UNIT)
                continue;

            if (!m_unit->CanAttack(attacker))
                friendlyTargets.insert(attacker);
        }

        for (auto attacker : friendlyTargets)
        {
            attacker->AttackStop(true, true);
            attacker->getThreatManager().modifyThreatPercent(m_unit, -101);
        }
    }
}

void CharmInfo::InitPossessCreateSpells()
{
    InitEmptyActionBar();                                   // charm action bar

    if (m_unit->GetTypeId() == TYPEID_PLAYER)               // possessed players don't have spells, keep the action bar empty
        return;

    Creature* creature = static_cast<Creature*>(m_unit);

    for (uint32 x = 0; x < CREATURE_MAX_SPELLS; ++x)
    {
        if (creature->m_spells[x] == 2)
            SetActionBar(x, COMMAND_ATTACK, ACT_COMMAND);
        if (IsPassiveSpell(creature->m_spells[x]))
            m_unit->CastSpell(creature, creature->m_spells[x], TRIGGERED_OLD_TRIGGERED);
        else
            AddSpellToActionBar(creature->m_spells[x], ACT_PASSIVE, x);
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

        if (spellId == 2) // hardcoded spell id 2 attack - only for possesss
            continue;

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
        if (IsPassiveSpell(spellInfo))
        {
            m_unit->CastSpell(m_unit, spellId, TRIGGERED_OLD_TRIGGERED);
            m_charmspells[x].SetActionAndType(spellId, ACT_PASSIVE);
        }
        else
        {
            m_charmspells[x].SetActionAndType(spellId, ACT_DISABLED);

            ActiveStates newstate;
            bool onlyselfcast = true;

            for (uint32 i = 0; i < 3 && onlyselfcast; ++i)  // nonexistent spell will not make any problems as onlyselfcast would be false -> break right away
            {
                if (spellInfo->EffectImplicitTargetA[i] != TARGET_UNIT_CASTER && spellInfo->EffectImplicitTargetA[i] != 0)
                    onlyselfcast = false;
            }

            if (IsAutocastable(spellInfo))
                newstate = ACT_ENABLED;
            else
                newstate = ACT_PASSIVE;

            AddSpellToActionBar(spellId, newstate);
        }
    }
}

bool CharmInfo::AddSpellToActionBar(uint32 spellId, ActiveStates newstate, uint8 forceSlot)
{
    if (forceSlot != 255)
    {
        if (!PetActionBar[forceSlot].GetAction() && PetActionBar[forceSlot].IsActionBarForSpell())
        {
            SetActionBar(forceSlot, spellId, newstate == ACT_DECIDE ? IsAutocastable(spellId) ? ACT_DISABLED : ACT_PASSIVE : newstate);
            return true;
        }
        return false;
    }

    uint32 first_id = sSpellMgr.GetFirstSpellInChain(spellId);

    // new spell rank can be already listed
    for (auto& i : PetActionBar)
    {
        if (uint32 action = i.GetAction())
        {
            if (i.IsActionBarForSpell() && sSpellMgr.GetFirstSpellInChain(action) == first_id)
            {
                i.SetAction(spellId);
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

bool CharmInfo::RemoveSpellFromActionBar(uint32 spellId)
{
    uint32 first_id = sSpellMgr.GetFirstSpellInChain(spellId);

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

    for (auto& m_charmspell : m_charmspells)
        if (spellid == m_charmspell.GetAction())
            m_charmspell.SetType(apply ? ACT_ENABLED : ACT_DISABLED);
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
            ResetStayPosition();
            SetSpellOpener();
            break;

        case COMMAND_FOLLOW:
            ResetStayPosition();
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
        uint8 type  = (uint8)std::stoul(*iter);
        ++iter;
        uint32 action = std::stoul(*iter);

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
    for (auto i : PetActionBar)
        data << uint32(i.packedData);
}

void CharmInfo::SetSpellAutocast(uint32 spell_id, bool state)
{
    for (auto& i : PetActionBar)
    {
        if (spell_id == i.GetAction() && i.IsActionBarForSpell())
        {
            i.SetType(state ? ACT_ENABLED : ACT_DISABLED);
            break;
        }
    }
}

void CharmInfo::ResetStayPosition()
{
    m_stayPosX = 0;
    m_stayPosY = 0;
    m_stayPosZ = 0;
    m_stayPosO = 0;
    m_stayPosSet = false;
}

void CharmInfo::SetStayPosition()
{
    if (!m_unit->movespline->Finalized())
    {
        Movement::Location loc = m_unit->movespline->ComputePosition();
        if (GenericTransport* transport = m_unit->GetTransport())
            transport->CalculatePassengerPosition(loc.x, loc.y, loc.z, &loc.orientation);
        m_stayPosX = loc.x;
        m_stayPosY = loc.y;
        m_stayPosZ = loc.z;
        m_stayPosO = loc.orientation;
    }
    else
    {
        m_stayPosX = m_unit->GetPositionX();
        m_stayPosY = m_unit->GetPositionY();
        m_stayPosZ = m_unit->GetPositionZ();
        m_stayPosO = m_unit->GetOrientation();
    }
    m_stayPosSet = true;
}

bool CharmInfo::UpdateStayPosition()
{
    if (m_stayPosSet)
        return false;

    SetStayPosition();
    return true;
}

bool Unit::isFrozen() const
{
    return HasAuraState(AURA_STATE_FROZEN);
}

///----------Pet responses methods-----------------
void Unit::SendPetCastFail(uint32 spellid, SpellCastResult msg) const
{
    if (msg == SPELL_CAST_OK)
        return;

    Unit* owner = GetMaster();
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

void Unit::SendPetDismiss(uint32 soundId) const
{
    WorldPacket data(SMSG_PET_DISMISS_SOUND, 4 + 4 + 4 + 4);
    data << soundId;
    data << GetPositionX() << GetPositionY() << GetPositionZ();
    SendMessageToSet(data, true);
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
        UpdateSplinePosition(true);

        movespline->_Interrupt();
        isMoving = true;
    }

    StopMoving(forceSendStop || isMoving);
}

bool Unit::SetConfused(bool apply, ObjectGuid casterGuid, uint32 spellID)
{
    if (apply != HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED))
    {
        if (apply)
            CastStop(GetObjectGuid() == casterGuid ? spellID : 0);
        else
            RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);

        // Update crowd controlled movement if required:
        // TODO: requires motionmster upgrade for proper handling past this line
        // We are effectively rebuilding motion master contents: confused > fleeing > panic
        {
            const bool panic = IsInPanic();

            GetMotionMaster()->MovementExpired();

            if (apply)
                GetMotionMaster()->MoveConfused();
            else if (IsFleeing() && !panic)
            {
                AuraList const& fears = GetAurasByType(SPELL_AURA_MOD_FEAR);
                Unit* source = (fears.empty() ? nullptr : fears.back()->GetCaster());
                GetMotionMaster()->MoveFleeing(source ? source : this);
            }
        }

        if (apply)
            SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);

        return true;
    }
    return false;
}

bool Unit::SetFleeing(bool apply, ObjectGuid casterGuid/* = ObjectGuid()*/, uint32 spellID/* = 0*/, uint32 duration/* = 0*/)
{
    // Normal flee always takes prio over timed flee (panic)
    if (apply != HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING) || (apply && IsInPanic()))
    {
        if (apply)
        {
            // Fleeing prevention aura taken into account first
            if (HasAuraType(SPELL_AURA_PREVENTS_FLEEING))
                return false;

            // Do not panic if already confused
            if (duration && IsConfused())
                return false;

            CastStop(GetObjectGuid() == casterGuid ? spellID : 0);
        }
        else
            RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING);

        // Update crowd controlled movement if required:
        // TODO: requires motionmster upgrade for proper handling past this line
        // We are effectively rebuilding motion master contents: confused > fleeing > panic
        {
            GetMotionMaster()->MovementExpired();

            if (IsConfused())
                GetMotionMaster()->MoveConfused();
            else if (apply)
            {
                Unit* source = (IsInWorld() ? GetMap()->GetUnit(casterGuid) : nullptr);
                GetMotionMaster()->MoveFleeing((source ? source : this), duration);
            }
        }

        if (apply)
            SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING);

        return true;
    }
    return false;
}

bool Unit::SetStunned(bool apply, ObjectGuid casterGuid, uint32 spellID, bool logout/* = false*/)
{
    const bool initial = (apply && !IsStunned());
    const bool update = (apply != hasUnitState(logout ? UNIT_STAT_LOGOUT_TIMER : UNIT_STAT_STUNNED));

    if (initial || update || (logout && !apply))
    {
        if (initial)
            CastStop(GetObjectGuid() == casterGuid ? spellID : 0);

        SetImmobilizedState(apply, true, logout);

        if (initial)
        {
            // Non-client controlled unit with an AI should drop target
            if (const bool requireTargetChange = (!IsControlledByPlayer() && AI()))
            {
                if (!GetTargetGuid().IsEmpty())
                    SetTargetGuid(ObjectGuid());

                // Broadcast orientation change on stun start for creatures
                // FIXME: TODO
                SetFacingTo(GetOrientation());
            }
        }

        ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED, hasUnitState(UNIT_STAT_STUNNED | UNIT_STAT_LOGOUT_TIMER));

        if (!logout)
            ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_29, (IsStunned() || IsFeigningDeath()));

        return true;
    }
    return false;
}

bool Unit::SetStunnedByLogout(bool apply)
{
    if (SetStunned(apply, ObjectGuid(), 0, true))
    {
        // Sit down when eligible:
        if (apply)
        {
            if (IsStandState())
            {
                if (!m_movementInfo.HasMovementFlag(MovementFlags(movementFlagsMask | MOVEFLAG_SWIMMING | MOVEFLAG_SPLINE_ENABLED)))
                    SetStandState(UNIT_STAND_STATE_SIT);
            }
        }
        // Stand up on cancel
        else if (getStandState() == UNIT_STAND_STATE_SIT)
            SetStandState(UNIT_STAND_STATE_STAND);

        return true;
    }
    return false;
}

void Unit::SetImmobilizedState(bool apply, bool stun, bool logout)
{
    const uint32 state = (stun ? (logout ? UNIT_STAT_LOGOUT_TIMER : UNIT_STAT_STUNNED) : UNIT_STAT_ROOT);

    if (apply)
    {
        addUnitState(state);

        if (!IsClientControlled())
            StopMoving();

        SendMoveRoot(true);
    }
    else
    {
        clearUnitState(state);

        // Prevent giving ability to move if more immobilizers are active
        if (!IsImmobilizedState())
            SendMoveRoot(false);

        SetIgnoreRangedTargets(false);
    }
}

void Unit::SetFeignDeath(bool apply, ObjectGuid casterGuid /*= ObjectGuid()*/, uint32 spellID /*= 0*/, bool dynamic /*= true*/, bool success /*= true*/)
{
    if (apply)
    {
        RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_IMMUNE_OR_LOST_SELECTION);

        if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        {
            if (success)
            {
                // Successful FD: set state, stop attack (+clear target for player-controlled npcs) and clear combat if applicable
                addUnitState(UNIT_STAT_FEIGN_DEATH);

                InstanceData* instance = GetInstanceData();
                if (instance && sWorld.getConfig(CONFIG_BOOL_INSTANCE_STRICT_COMBAT_LOCKDOWN) && instance->IsEncounterInProgress())
                {
                    // This rule was introduced in 2.3.0+: do not clear combat state if zone is in combat lockdown by encounter
                    AttackStop(true, false, true);
                    getHostileRefManager().addThreatPercent(-100);
                    getHostileRefManager().updateOnlineOfflineState(false);
                }
                else
                    CombatStop();
            }
            else
            {
                // Unsuccessful FD: do not set UNIT_STAT_FEIGN_DEATH, send resist message and stop attack (+clear target for player-controlled npcs)
                if (GetTypeId() == TYPEID_PLAYER)
                    static_cast<Player*>(this)->SendFeignDeathResisted();
                AttackStop(true, false, true);
            }
        }
        else
        {
            // NPC FD is always successful, but never observed to disengage from combat
            addUnitState(UNIT_STAT_FEIGN_DEATH);
            AttackStop(true, false, true);
            getHostileRefManager().addThreatPercent(-100);
            getHostileRefManager().updateOnlineOfflineState(false);
        }

        if (dynamic)
            SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);

        // prevent interrupt message
        if (spellID && casterGuid == GetObjectGuid())
        {
            if (m_currentSpells[CURRENT_GENERIC_SPELL] && m_currentSpells[CURRENT_GENERIC_SPELL]->m_spellInfo->Id == spellID)
                FinishSpell(CURRENT_GENERIC_SPELL, false);
        }
        InterruptNonMeleeSpells(true);
    }
    else if (IsFeigningDeath())
    {
        clearUnitState(UNIT_STAT_FEIGN_DEATH);

        RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);

        getHostileRefManager().updateOnlineOfflineState(true);
    }

    ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNK_29, (IsStunned() || IsFeigningDeath()));
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

bool Unit::IsSeatedState() const
{
    uint8 standState = getStandState();
    return standState != UNIT_STAND_STATE_SLEEP && standState != UNIT_STAND_STATE_STAND;
}

void Unit::SetStandState(uint8 state, bool acknowledge/* = false*/)
{
    if (getStandState() == state)
        return;

    SetByteValue(UNIT_FIELD_BYTES_1, 0, state);

    if (!IsSeatedState())
        RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_NOT_SEATED);

    if (!acknowledge && GetTypeId() == TYPEID_PLAYER)
    {
        WorldPacket data(SMSG_STANDSTATE_UPDATE, 1);
        data << uint8(state);
        static_cast<Player*>(this)->GetSession()->SendPacket(data);
    }
}

bool Unit::IsPolymorphed() const
{
    return HasAura(12939); // Polymorph heal effect common for all polys
}

void Unit::SetDisplayId(uint32 modelId)
{
    SetUInt32Value(UNIT_FIELD_DISPLAYID, modelId);

    UpdateModelData();

    if (HasCharmer())
    {
        Unit* charmer = GetCharmer();
        if (charmer && (charmer->GetTypeId() == TYPEID_PLAYER) && ((Player*)charmer)->GetGroup())
            ((Player*)charmer)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_MODEL_ID);
    }
}

void Unit::RestoreDisplayId()
{
    Aura const* handledAura = nullptr;
    // try to receive model from transform auras
    AuraList const& transforms = GetAurasByType(SPELL_AURA_TRANSFORM);
    if (!transforms.empty())
    {
        // iterate over already applied transform auras - from newest to oldest
        for (auto i = transforms.rbegin(); i != transforms.rend(); ++i)
        {
            if (!handledAura)
                handledAura = (*i);
            // prefer negative auras
            if (!(*i)->IsPositive())
            {
                handledAura = (*i);
                break;
            }
        }
    }

    // transform aura was found
    if (handledAura)
    {
        SetDisplayId(handledAura->GetModifier()->m_amount);
        return;
    }

    AuraList const& shapeshiftAura = GetAurasByType(SPELL_AURA_MOD_SHAPESHIFT);
    if (!shapeshiftAura.empty()) // we've found shapeshift
    {
        if (uint32 displayId = shapeshiftAura.front()->GetModifier()->m_amount) // can be zero
        {
            SetDisplayId(displayId);
            return;
        }
    }

    // no auras found - set modelid to default
    SetDisplayId(GetNativeDisplayId());
}

void Unit::UpdateModelData()
{
    if (CreatureModelInfo const* modelInfo = sObjectMgr.GetCreatureModelInfo(GetDisplayId()))
    {
        // we expect values in database to be relative to scale = 1.0
        SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, GetObjectScale() * modelInfo->bounding_radius);

        // never actually update combat_reach for player, it's always the same. Below player case is for initialization
        SetFloatValue(UNIT_FIELD_COMBATREACH, GetObjectScale() * modelInfo->combat_reach);

        SetBaseWalkSpeed(modelInfo->SpeedWalk);
        SetBaseRunSpeed(modelInfo->SpeedRun);
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
    for (unsigned int& i : m_reactiveTimer)
        i = 0;

    if (HasAuraState(AURA_STATE_DEFENSE))
        ModifyAuraState(AURA_STATE_DEFENSE, false);
    if (getClass() == CLASS_HUNTER && HasAuraState(AURA_STATE_HUNTER_PARRY))
        ModifyAuraState(AURA_STATE_HUNTER_PARRY, false);
    if (getClass() == CLASS_WARRIOR)
        ClearComboPoints();
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
                    if (getClass() == CLASS_WARRIOR)
                        ClearComboPoints();
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
    UnitList targets;

    MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck u_check(this, radius);
    MaNGOS::UnitListSearcher<MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck> searcher(targets, u_check);
    Cell::VisitAllObjects(this, searcher, radius);

    // remove current target
    if (except)
        targets.remove(except);

    // remove not LoS targets
    for (UnitList::iterator tIter = targets.begin(); tIter != targets.end();)
    {
        if (!IsWithinLOSInMap(*tIter, true))
        {
            UnitList::iterator tIter2 = tIter;
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
    UnitList::const_iterator tcIter = targets.begin();
    for (uint32 i = 0; i < rIdx; ++i)
        ++tcIter;

    return *tcIter;
}

Unit* Unit::SelectRandomFriendlyTarget(Unit* except /*= nullptr*/, float radius /*= ATTACK_DISTANCE*/) const
{
    UnitList targets;

    MaNGOS::AnyFriendlyUnitInObjectRangeCheck u_check(this, nullptr, radius);
    MaNGOS::UnitListSearcher<MaNGOS::AnyFriendlyUnitInObjectRangeCheck> searcher(targets, u_check);

    Cell::VisitAllObjects(this, searcher, radius);

    // remove current target
    if (except)
        targets.remove(except);

    // remove not LoS targets
    for (UnitList::iterator tIter = targets.begin(); tIter != targets.end();)
    {
        if (!IsWithinLOSInMap(*tIter, true))
        {
            UnitList::iterator tIter2 = tIter;
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
    UnitList::const_iterator tcIter = targets.begin();
    for (uint32 i = 0; i < rIdx; ++i)
        ++tcIter;

    return *tcIter;
}

bool Unit::HasDamageInterruptibleStunAura() const
{
    for (Aura* aura : GetAurasByType(SPELL_AURA_MOD_STUN))
        if (aura->GetSpellProto()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_DAMAGE)
            return true;

    return false;
}

void Unit::ApplyAttackTimePercentMod(WeaponAttackType att, float val, bool apply)
{
    float oldVal = GetFloatValue(UNIT_FIELD_BASEATTACKTIME + att);
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
    float newVal = GetFloatValue(UNIT_FIELD_BASEATTACKTIME + att);
    uint32 attackTimer = getAttackTimer(att);
    int32 diff = newVal - oldVal;
    setAttackTimer(att, diff < -int32(attackTimer) ? 0 : attackTimer + diff);
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
    else if (HasCharmer())
    {
        Unit* charmer = GetCharmer();
        if (charmer && (charmer->GetTypeId() == TYPEID_PLAYER) && ((Player*)charmer)->GetGroup())
        {
            ((Player*)charmer)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_AURAS);
            SetAuraUpdateMask(slot);
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
    for (auto itr : mDummy)
        if (itr->GetId() == spell_id)
            return itr;

    return nullptr;
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
            iter = auras.begin();
        }
        else
            ++iter;
    }
}

void Unit::NearTeleportTo(float x, float y, float z, float orientation, bool casting /*= false*/, bool transportLeave /*= false*/)
{
    if (IsPlayer())
    {
        uint32 options = (transportLeave ? 0 : TELE_TO_NOT_LEAVE_TRANSPORT) | (casting ? TELE_TO_SPELL : 0);
        if (GetDistance(x, y, z, DIST_CALC_NONE) < 100.f * 100.f)
            options |= TELE_TO_NOT_LEAVE_COMBAT;
        static_cast<Player*>(this)->TeleportTo(GetMapId(), x, y, z, orientation, options);
    }
    else
    {
        DisableSpline();

        Creature* c = static_cast<Creature*>(this);
        // Creature relocation acts like instant movement generator, so current generator expects interrupt/reset calls to react properly
        if (!c->GetMotionMaster()->empty())
            if (MovementGenerator* movgen = c->GetMotionMaster()->top())
                movgen->Interrupt(*c);

        GetMap()->CreatureRelocation(c, x, y, z, orientation);

        SendHeartBeat();

        // finished relocation, movegen can different from top before creature relocation,
        // but apply Reset expected to be safe in any case
        if (!c->GetMotionMaster()->empty())
            if (MovementGenerator* movgen = c->GetMotionMaster()->top())
                movgen->Reset(*c);
    }
}

void Unit::SendTeleportPacket(float x, float y, float z, float ori, GenericTransport* transport)
{
    MovementInfo teleportMovementInfo = m_movementInfo;
    if (transport)
    {
        teleportMovementInfo.SetTransportData(transport->GetObjectGuid(), x, y, z, ori, transport->GetPathProgress());
        transport->CalculatePassengerPosition(x, y, z, &ori); // recalculate to real coords
    }
    teleportMovementInfo.ChangePosition(x, y, z, ori);

    Player* player = nullptr;

    if (GetTypeId() == TYPEID_PLAYER)
    {
        player = static_cast<Player*>(this);
        WorldPacket data;
        data.Initialize(MSG_MOVE_TELEPORT_ACK, 41);
        data << GetPackGUID();
        data << uint32(0); // this value increments every time
        data << teleportMovementInfo;
        player->GetSession()->SendPacket(data);
    }

    WorldPacket moveUpdateTeleport(MSG_MOVE_TELEPORT, 38);
    moveUpdateTeleport << GetPackGUID();
    teleportMovementInfo.Write(moveUpdateTeleport);
    SendMessageToSetExcept(moveUpdateTeleport, player);
}

void Unit::MonsterMoveWithSpeed(float x, float y, float z, float speed, bool generatePath, bool forceDestination)
{
    Movement::MoveSplineInit init(*this);
    init.MoveTo(x, y, z, generatePath, forceDestination);
    init.SetVelocity(speed);
    init.Launch();
}

struct SetImmuneToNPCHelper
{
    explicit SetImmuneToNPCHelper(bool _state) : state(_state) {}
    void operator()(Unit* unit) const { unit->SetImmuneToNPC(state); }
    bool state;
};

void Unit::SetImmuneToNPC(bool state)
{
    if (state)
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
    else
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);

    // Do not propagate during charm
    if (!HasCharmer())
        CallForAllControlledUnits(SetImmuneToNPCHelper(state), CONTROLLED_PET | CONTROLLED_TOTEMS | CONTROLLED_GUARDIANS | CONTROLLED_CHARM);
}

struct SetImmuneToPlayerHelper
{
    explicit SetImmuneToPlayerHelper(bool _state) : state(_state) {}
    void operator()(Unit* unit) const { unit->SetImmuneToPlayer(state); }
    bool state;
};

void Unit::SetImmuneToPlayer(bool state)
{
    if (state)
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
    else
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);

    // Do not propagate during charm
    if (!HasCharmer())
        CallForAllControlledUnits(SetImmuneToPlayerHelper(state), CONTROLLED_PET | CONTROLLED_TOTEMS | CONTROLLED_GUARDIANS | CONTROLLED_CHARM);
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

    // Do not propagate during charm
    if (!HasCharmer())
    {
        if (GetTypeId() == TYPEID_PLAYER && static_cast<Player*>(this)->GetGroup())
            static_cast<Player*>(this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_STATUS);

        CallForAllControlledUnits(SetPvPHelper(state), CONTROLLED_PET | CONTROLLED_TOTEMS | CONTROLLED_GUARDIANS | CONTROLLED_CHARM);
    }
}

bool Unit::IsPvPFreeForAll() const
{
    // Pre-WotLK free for all check (query player in charge)
    if (const Player* thisPlayer = GetControllingPlayer())
        return thisPlayer->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP);
    return false;
}

void Unit::SetPvPFreeForAll(bool state)
{
    // Pre-WotLK free for all set (for player only)
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (state)
            SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP);
        else
            RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP);

        Player* thisPlayer = static_cast<Player*>(this);
        thisPlayer->ForceHealAndPowerUpdateInZone();
        thisPlayer->ForceHealthAndPowerUpdate();
        if (thisPlayer->GetGroup())
            thisPlayer->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_STATUS);
    }
}

bool Unit::IsPvPContested() const
{
    if (const Player* thisPlayer = GetControllingPlayer())
        return thisPlayer->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP);
    return false;
}

void Unit::SetPvPContested(bool state)
{
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (state)
            SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP);
        else
            RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP);
    }
}

void Unit::KnockBackFrom(Unit* target, float horizontalSpeed, float verticalSpeed)
{
    float angle = this == target ? GetOrientation() + M_PI_F : target->GetAngle(this);
    KnockBackWithAngle(angle, horizontalSpeed, verticalSpeed);
}

void Unit::KnockBackWithAngle(float angle, float horizontalSpeed, float verticalSpeed)
{
    if (IsClientControlled())
    {
        if (Player const* player = GetControllingPlayer())
        {
            player->GetSession()->SendKnockBack(this, angle, horizontalSpeed, verticalSpeed);
            return;
        }
    }

    // no non player controlled pre-wotlk
    return;
}

struct StopAttackFactionHelper
{
    explicit StopAttackFactionHelper(uint32 _faction_id) : faction_id(_faction_id) {}
    void operator()(Unit* unit) const { unit->StopAttackFaction(faction_id); }
    uint32 faction_id;
};

void Unit::StopAttackFaction(uint32 faction_id)
{
    if (Unit* victim = GetVictim())
    {
        if (victim->GetFactionTemplateEntry()->faction == faction_id)
        {
            AttackStop();
            if (IsNonMeleeSpellCasted(false))
                InterruptNonMeleeSpells(false);
        }
    }

    AttackerSet const& attackers = getAttackers();
    for (AttackerSet::const_iterator itr = attackers.begin(); itr != attackers.end();)
    {
        if ((*itr)->GetFactionTemplateEntry()->faction == faction_id)
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

bool Unit::IsShapeShifted() const
{
    // Mirroring clientside gameplay logic
    if (ShapeshiftForm form = GetShapeshiftForm())
    {
        if (SpellShapeshiftFormEntry const* entry = sSpellShapeshiftFormStore.LookupEntry(form))
            return !(entry->flags1 & SHAPESHIFT_FORM_FLAG_ALLOW_ACTIVITY);
    }
    return false;
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

class UnitVisitObjectsInRangeNotifyEvent : public BasicEvent
{
    public:
        UnitVisitObjectsInRangeNotifyEvent(Unit& owner) : BasicEvent(), m_owner(owner) {}

        bool Execute(uint64 /*e_time*/, uint32 /*p_time*/) override
        {
            float radius = MAX_CREATURE_ATTACK_RADIUS * sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_AGGRO);
            if (m_owner.GetTypeId() == TYPEID_PLAYER)
            {
                MaNGOS::PlayerVisitObjectsNotifier notify(static_cast<Player&>(m_owner));
                Cell::VisitAllObjects(&m_owner, notify, radius);
            }
            else // if(m_owner.GetTypeId() == TYPEID_UNIT)
            {
                Creature& creature = static_cast<Creature&>(m_owner);
                //since visitor was called we override can aggro with true if creature is alive
                creature.SetCanAggro(creature.IsAlive());
                MaNGOS::CreatureVisitObjectsNotifier notify(creature);
                Cell::VisitAllObjects(&m_owner, notify, radius);
            }
            m_owner.FinalizeAINotifyEvent();
            return true;
        }

        void Abort(uint64) override
        {
            m_owner.FinalizeAINotifyEvent();
        }

    private:
        Unit & m_owner;
};

void Unit::ScheduleAINotify(uint32 delay, bool forced)
{
    if (!IsAINotifyScheduled())
    {
        m_AINotifyEvent = new UnitVisitObjectsInRangeNotifyEvent(*this);
        m_events.AddEvent(m_AINotifyEvent, m_events.CalculateTime(delay));
    }
    else if (forced)
    {
        m_events.KillEvent(m_AINotifyEvent);
        m_AINotifyEvent = new UnitVisitObjectsInRangeNotifyEvent(*this);
        m_events.AddEvent(m_AINotifyEvent, m_events.CalculateTime(delay));
    }
}

void Unit::AbortAINotifyEvent()
{
    if (m_AINotifyEvent)
    {
        m_events.KillEvent(m_AINotifyEvent);
        m_AINotifyEvent = nullptr;
    }
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
#ifdef BUILD_METRICS
    metric::duration<std::chrono::microseconds> meas("unit.updatesplinemovement", {
        { "entry", std::to_string(GetEntry()) },
        { "guid", std::to_string(GetGUIDLow()) },
        { "unit_type", std::to_string(GetGUIDHigh()) },
        { "map_id", std::to_string(GetMapId()) },
        { "instance_id", std::to_string(GetInstanceId()) }
    }, 1000);
#endif
    movespline->updateState(t_diff);
    bool arrived = movespline->Finalized();

    if (arrived)
        DisableSpline();

    m_movesplineTimer.Update(t_diff);
    if (m_movesplineTimer.Passed() || arrived)
    {
        m_movesplineTimer.Reset(POSITION_UPDATE_DELAY);
        UpdateSplinePosition();
    }
}

void Unit::UpdateSplinePosition(bool relocateOnly)
{
    Movement::Location computedLoc = movespline->ComputePosition();
    Position pos(computedLoc.x, computedLoc.y, computedLoc.z, computedLoc.orientation);
    if (GenericTransport* transport = GetTransport())
    {
        m_movementInfo.UpdateTransportData(pos);
        transport->CalculatePassengerPosition(pos.x, pos.y, pos.z, &pos.o);
    }

    bool faced = false;
    if (movespline->isFacing())
    {
        if (movespline->isFacingTarget())
        {
            if (Unit const* target = ObjectAccessor::GetUnit(*this, ObjectGuid(movespline->GetFacing().target)))
            {
                pos.o = GetAngle(target);
                faced = true;
            }
        }
        else if (movespline->isFacingPoint())
        {
            auto& facing = movespline->GetFacing();
            pos.o = GetAngle(facing.f.x, facing.f.y);
            faced = true;
        }
        else if (movespline->isFacingAngle())
        {
            pos.o = movespline->GetFacing().angle;
            faced = true;
        }
    }

    if (!faced)
    {
        if (pos.y == GetPositionY() && pos.x == GetPositionX())
            pos.o = GetOrientation();
        else
        {
            float angle = atan2((pos.y - GetPositionY()), (pos.x - GetPositionX()));
            pos.o = (angle >= 0 ? angle : ((2 * M_PI_F) + angle));
        }
    }

    if (relocateOnly)
    {
        Relocate(pos.x, pos.y, pos.z, pos.o);
        return;
    }

    if (IsPlayer())
        static_cast<Player*>(this)->SetPosition(pos.x, pos.y, pos.z, pos.o);
    else
        GetMap()->CreatureRelocation((Creature*)this, pos.x, pos.y, pos.z, pos.o);
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
    // Possess is a unique advertised charm, another advertised charm already exists: we should get rid of it first
    if (HasCharm())
        return nullptr;

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

    TemporarySpawn* possessed = new TemporarySpawn(GetObjectGuid());

    CreatureCreatePos pos(GetMap(), x, y, z, ang);

    if (x == 0.0f && y == 0.0f && z == 0.0f)
        pos = CreatureCreatePos(this, GetOrientation(), CONTACT_DISTANCE, ang);

    if (!possessed->Create(GetMap()->GenerateLocalLowGuid(cinfo->GetHighGuid()), pos, cinfo))
    {
        delete possessed;
        return nullptr;
    }

    Player* player = GetTypeId() == TYPEID_PLAYER ? static_cast<Player*>(this) : nullptr;

    possessed->SetFactionTemporary(getFaction(), TEMPFACTION_NONE);     // set same faction than player
    possessed->SetRespawnCoord(pos);                                    // set spawn coord
    possessed->SetCharmerGuid(GetObjectGuid());                         // save guid of the charmer
    possessed->SetCreatorGuid(GetObjectGuid());                         // save guid of the creator
    possessed->SetUInt32Value(UNIT_CREATED_BY_SPELL, spellEntry->Id);   // set the spell id used to create this (may be used for removing corresponding aura
    possessed->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_POSSESSED);          // set flag for client that mean this unit is controlled by a player
    possessed->addUnitState(UNIT_STAT_POSSESSED);                       // also set internal unit state flag
    possessed->SelectLevel(getLevel());                                 // set level to same level than summoner TODO:: not sure its always the case...
    possessed->SetLinkedToOwnerAura(TEMPSPAWN_LINKED_AURA_OWNER_CHECK | TEMPSPAWN_LINKED_AURA_REMOVE_OWNER); // set what to do if linked aura is removed or the creature is dead.

    // important before adding to the map!
    SetCharmGuid(possessed->GetObjectGuid());                           // save guid of charmed creature

    possessed->SetSummonProperties(TEMPSPAWN_CORPSE_TIMED_DESPAWN, 5000); // set 5s corpse decay
    GetMap()->Add(static_cast<Creature*>(possessed));                   // create the creature in the client
    possessed->AIM_Initialize();                                        // even if this will be replaced it need to be initialized to take care of spawn spells

    // Give the control to the player
    if (player)
    {
        player->UnsummonPetTemporaryIfAny();

        player->GetCamera().SetView(possessed);                         // modify camera view to the creature view
        // Force client control (required to function propely)
        player->UpdateClientControl(possessed, true, true);             // transfer client control to the creature after altering flags
        player->SetMover(possessed);                                    // set mover so now we know that creature is "moved" by this unit
        player->SendForcedObjectUpdate();                               // we have to update client data here to avoid problem with the "release spirit" windows reappear.
    }

    // init CharmInfo class that will hold charm data
    CharmInfo* charmInfo = possessed->InitCharmInfo(possessed);

    // set temp possess ai (creature will not be able to react by itself)
    charmInfo->SetCharmState("PossessedAI");
    possessed->SetWalk(IsWalking(), true);                              // sync the walking state with the summoner - after SetCharmState

    // New flags for the duration of charm need to be set after SetCharmState, gets reset in ResetCharmState
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        possessed->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

    const bool immunePC = IsImmuneToPlayer();
    if (possessed->IsImmuneToPlayer() != immunePC)
        possessed->SetImmuneToPlayer(immunePC);

    const bool immuneNPC = IsImmuneToNPC();
    if (possessed->IsImmuneToNPC() != immuneNPC)
        possessed->SetImmuneToNPC(immuneNPC);

    charmInfo->ProcessUnattackableTargets();

    if (player)
    {
        // Initialize pet bar
        charmInfo->InitPossessCreateSpells();
        player->PossessSpellInitialize();

        // Take away client control immediately if we are not supposed to have control at the moment
        if (!possessed->IsClientControlled(player))
            player->UpdateClientControl(possessed, false);
    }

    // Creature Linking, Initial load is handled like respawn
    if (possessed->IsLinkingEventTrigger())
        GetMap()->GetCreatureLinkingHolder()->DoCreatureLinkingEvent(LINKING_EVENT_RESPAWN, possessed);

    // return the creature therewith the summoner has access to it
    return possessed;
}

bool Unit::TakePossessOf(Unit* possessed)
{
    // Possess is a unique advertised charm, another advertised charm already exists: we should get rid of it first
    if (HasCharm())
        return false;

    Player* player = (GetTypeId() == TYPEID_PLAYER ? static_cast<Player*>(this) : nullptr);

    if (player && possessed->GetObjectGuid() != GetPetGuid())
        player->UnsummonPetTemporaryIfAny();

    // Update possessed's client control status before altering flags
    if (const Player* controllingClientPlayer = possessed->GetClientControlling())
        controllingClientPlayer->UpdateClientControl(possessed, false);

    // stop attacking
    possessed->AttackStop(true, true);

    Creature* possessedCreature = nullptr;
    Player* possessedPlayer = nullptr;
    CharmInfo* charmInfo = possessed->InitCharmInfo(possessed);
    possessed->SetCharmerGuid(GetObjectGuid());
    SetCharm(possessed);

    // stop any generated movement TODO:: this may not be correct! what about possessing a feared creature?
    possessed->GetMotionMaster()->Clear();
    possessed->GetMotionMaster()->MoveIdle();
    possessed->StopMoving(true);

    Position combatStartPosition;

    if (possessed->GetTypeId() == TYPEID_UNIT)
    {
        possessedCreature = static_cast<Creature*>(possessed);
        possessedCreature->GetCombatStartPosition(combatStartPosition);
        possessedCreature->SetFactionTemporary(getFaction(), TEMPFACTION_NONE);

        charmInfo->SetCharmState("PossessedAI");
        possessedCreature->SetWalk(IsWalking(), true);
        getHostileRefManager().deleteReference(possessedCreature);
    }
    else if (possessed->GetTypeId() == TYPEID_PLAYER)
    {
        possessedPlayer = static_cast<Player*>(possessed);

        if (player && player->IsInDuelWith(possessedPlayer))
            possessedPlayer->SetUInt32Value(PLAYER_DUEL_TEAM, player->GetUInt32Value(PLAYER_DUEL_TEAM));
        else
            possessedPlayer->setFaction(getFaction());

        charmInfo->SetCharmState("PossessedAI");
    }

    // Manipulate unit flags after SetCharmState
    possessed->addUnitState(UNIT_STAT_POSSESSED);
    possessed->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_POSSESSED);

    // New flags for the duration of charm need to be set after SetCharmState, gets reset in ResetCharmState
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        possessed->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    else
        possessed->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

    const bool immunePC = IsImmuneToPlayer();
    if (possessed->IsImmuneToPlayer() != immunePC) // Do not propagate immunity flag to pets
        possessed->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);

    const bool immuneNPC = IsImmuneToNPC();
    if (possessed->IsImmuneToNPC() != immuneNPC)
        possessed->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);

    charmInfo->SetCharmStartPosition(combatStartPosition.IsEmpty() ? possessed->GetPosition(possessed->GetTransport()) : combatStartPosition);

    charmInfo->ProcessUnattackableTargets();

    if (!IsInCombat())
    {
        possessed->GetCombatManager().StopCombatTimer();
        possessed->ClearInCombat();
    }

    if (player)
    {
        player->GetCamera().SetView(possessed);
        // Force client control (required to function propely)
        player->UpdateClientControl(possessed, true, true);
        player->SetMover(possessed);
        player->SendForcedObjectUpdate();

        // Unsummon existing pet and initialize pet bar only when not possessing own pet
        if (possessedCreature && !(possessedCreature->IsPet() && possessedCreature->GetObjectGuid() == GetPetGuid()))
        {
            // player pet is unsmumoned while possessing
            player->UnsummonPetTemporaryIfAny();

            charmInfo->InitPossessCreateSpells();
            // may not always have AI, when posessing a player for example
            if (possessed->AI())
                possessed->AI()->SetReactState(REACT_PASSIVE);
            charmInfo->SetCommandState(COMMAND_STAY);
            player->PossessSpellInitialize();
        }

        // Take away client control immediately if we are not supposed to have control at the moment
        if (!possessed->IsClientControlled(player))
            player->UpdateClientControl(possessed, false);

        if (player->GetGroup())
            player->SetGroupUpdateFlag(GROUP_UPDATE_PET);
    }

    return true;
}

bool Unit::TakeCharmOf(Unit* charmed, uint32 spellId, bool advertised /*= true*/)
{
    Player* charmerPlayer = (GetTypeId() == TYPEID_PLAYER ? static_cast<Player*>(this) : nullptr);

    if (charmerPlayer && advertised)
        charmerPlayer->UnsummonPetTemporaryIfAny();

    // Update charmed's client control status before altering flags
    if (const Player* controllingClientPlayer = charmed->GetClientControlling())
        controllingClientPlayer->UpdateClientControl(charmed, false);

    // stop attacking
    charmed->AttackStop(true, true);

    charmed->SetCharmerGuid(GetObjectGuid());
    if (advertised)
    {
        // If an advertised charm already exists: offload and overwrite
        if (const ObjectGuid &charmGuid = GetCharmGuid())
            m_charmedUnitsPrivate.insert(charmGuid);
        SetCharmGuid(charmed->GetObjectGuid());
    }
    else
        m_charmedUnitsPrivate.insert(charmed->GetObjectGuid());

    CharmInfo* charmInfo = charmed->InitCharmInfo(charmed);

    bool isPossessCharm = IsPossessCharmType(spellId);

    Position combatStartPosition;

    if (charmed->GetTypeId() == TYPEID_PLAYER)
    {
        Player* charmedPlayer = static_cast<Player*>(charmed);
        if (charmerPlayer && charmerPlayer->IsInDuelWith(charmedPlayer))
            charmedPlayer->SetUInt32Value(PLAYER_DUEL_TEAM, charmerPlayer->GetUInt32Value(PLAYER_DUEL_TEAM));
        else
            charmedPlayer->setFaction(getFaction());

        charmInfo->SetCharmState("PetAI");

        if (isPossessCharm)
            charmInfo->InitPossessCreateSpells();
        else
        {
            charmInfo->InitCharmCreateSpells();
            charmed->AI()->SetReactState(REACT_DEFENSIVE);
            charmInfo->SetCommandState(COMMAND_FOLLOW);
            charmInfo->SetIsRetreating(true);
        }

        // vanilla core only code - what is this for?
        charmedPlayer->ForceHealAndPowerUpdateInZone();
        
        charmedPlayer->SendForcedObjectUpdate();
    }
    else if (charmed->GetTypeId() == TYPEID_UNIT)
    {
        Creature* charmedCreature = static_cast<Creature*>(charmed);

        charmedCreature->GetCombatStartPosition(combatStartPosition);

        if (charmed->AI() && charmed->AI()->CanHandleCharm())
            charmInfo->SetCharmState("", false);
        else
            charmInfo->SetCharmState("PetAI");

        charmedCreature->SetWalk(IsWalking(), true);

        getHostileRefManager().deleteReference(charmedCreature);

        charmedCreature->SetFactionTemporary(getFaction(), TEMPFACTION_NONE);

        if (isPossessCharm)
            charmInfo->InitPossessCreateSpells();
        else
        {
            charmInfo->InitCharmCreateSpells();
            charmed->AI()->SetReactState(REACT_DEFENSIVE);
            charmInfo->SetCommandState(COMMAND_FOLLOW);
            charmInfo->SetIsRetreating(true);
        }
    }

    if (charmInfo->GetUnit() != charmed)
        sLog.outCustomLog("Unit didnt equal in Unit::TakeCharmOf after base changes.");

    // New flags for the duration of charm need to be set after SetCharmState, gets reset in ResetCharmState
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        charmed->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    else
        charmed->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

    const bool immunePC = IsImmuneToPlayer();
    if (charmed->IsImmuneToPlayer() != immunePC) // Do not propagate immunity flag to pets
        charmed->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);

    const bool immuneNPC = IsImmuneToNPC();
    if (charmed->IsImmuneToNPC() != immuneNPC)
        charmed->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);

    charmInfo->SetCharmStartPosition(combatStartPosition.IsEmpty() ? charmed->GetPosition(charmed->GetTransport()) : combatStartPosition);

    if (charmerPlayer && advertised)
    {
        charmerPlayer->CharmSpellInitialize();
        if (charmerPlayer->GetGroup())
            charmerPlayer->SetGroupUpdateFlag(GROUP_UPDATE_PET);
    }

    if (charmInfo->GetUnit() != charmed)
        sLog.outCustomLog("Unit didnt equal in Unit::TakeCharmOf after flag changes.");

    charmInfo->ProcessUnattackableTargets();

    if (charmInfo->GetUnit() != charmed)
        sLog.outCustomLog("Unit didnt equal in Unit::TakeCharmOf after attackability changes.");

    // put charmed in combat with all charmers enemies - must be done after flags
    ThreatList const& list = getThreatManager().getThreatList();
    for (auto& data : list)
    {
        Unit* enemy = data->getTarget();
        if (charmed->CanAttack(enemy))
            charmed->AddThreat(enemy, 0.f);
    }

    if (!IsInCombat())
    {
        charmed->GetCombatManager().StopCombatTimer();
        charmed->ClearInCombat();
    }

    if (UnitAI* ai = charmed->AI())
        ai->JustGotCharmed(this);

    return true;
}

void Unit::BreakCharmOutgoing(Unit* charmed)
{
    // Cache our own guid
    const ObjectGuid &guid = GetObjectGuid();

    // Verify charmed with self as a charmer
    if (charmed && charmed->HasCharmer(guid))
    {
        // Break any aura-based charm spells on the charmed unit placed by self
        charmed->RemoveSpellsCausingAura(SPELL_AURA_MOD_POSSESS, guid);
        charmed->RemoveSpellsCausingAura(SPELL_AURA_MOD_CHARM, guid);
        charmed->RemoveSpellsCausingAura(SPELL_AURA_MOD_POSSESS_PET, guid);
        charmed->RemoveSpellsCausingAura(SPELL_AURA_AOE_CHARM, guid);

        // Re-check: if still charmed after aura removal attempt - likely non-aura charm (summon charmed for example)
        if (charmed->HasCharmer(guid))
            Uncharm(charmed);

        if (charmed->GetSpawnerGuid() == guid)
        {
            // Looks like a charmed temporary summon, possibly summoned by a linked specific aura, try removing
            if (uint32 spellid = charmed->GetUInt32Value(UNIT_CREATED_BY_SPELL))
                RemoveAurasDueToSpell(spellid);
        }
    }
}

void Unit::BreakCharmOutgoing(bool advertisedOnly /*= false*/)
{
    // Always break charm on the unit advertised in own charm field
    BreakCharmOutgoing(GetCharm());

    // If set, do not break non-advertised charms (such as aoe charms)
    if (advertisedOnly)
        return;

    // Proceed breaking the rest of charms
    if (Map* map = GetMap())
    {
        while (!m_charmedUnitsPrivate.empty())
        {
            const ObjectGuid &guid = (*m_charmedUnitsPrivate.begin());

            if (Unit* unit = map->GetUnit(guid))
                // Erase is performed further down the line
                BreakCharmOutgoing(unit);
            else
                // Erase it from the container if somehow does not exist / is inaccesible anymore
                m_charmedUnitsPrivate.erase(guid);
        }
    }
}

void Unit::BreakCharmIncoming()
{
    if (Unit* charmer = GetCharmer())
        charmer->BreakCharmOutgoing(this);
}

void Unit::Uncharm(Unit* charmed, uint32 spellId)
{
    Player* player = (GetTypeId() == TYPEID_PLAYER ? static_cast<Player*>(this) : nullptr);

    // Charmed unit should exist and be charmed by us
    if (!charmed || !charmed->HasCharmer(GetObjectGuid()))
        return;

    // Cache the guid of the charmed unit
    const ObjectGuid charmedGuid = charmed->GetObjectGuid();

    // Detect if the charm is the possessing charm
    const bool possess = charmed->hasUnitState(UNIT_STAT_POSSESSED);

    // Detect if this charm is advertised in own charm field (public) or a private one
    const bool advertised = HasCharm(charmedGuid);

    if (possess)
    {
        charmed->clearUnitState(UNIT_STAT_POSSESSED);
        charmed->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_POSSESSED);
            player->ForceHealAndPowerUpdateInZone();
    }
    charmed->SetCharmerGuid(ObjectGuid());
    if (advertised)
        SetCharmGuid(ObjectGuid());
    else
        m_charmedUnitsPrivate.erase(charmedGuid);

    // Update movement of the victim
    // Update crowd controlled movement if required:
    // TODO: requires motionmster upgrade for proper handling past this line
    // We are effectively rebuilding motion master contents: confused > fleeing > panic
    if (!IsPossessCharmType(spellId))
    {
        const bool panic = charmed->IsInPanic(), fleeing = charmed->IsFleeing(), confused = charmed->IsConfused();

        // stop any generated movement: current solution
        charmed->StopMoving(true);
        charmed->GetMotionMaster()->Initialize();

        if (confused)
            charmed->GetMotionMaster()->MoveConfused();
        else if (fleeing && !panic)
        {
            AuraList const& fears = charmed->GetAurasByType(SPELL_AURA_MOD_FEAR);
            Unit* source = (fears.empty() ? nullptr : fears.back()->GetCaster());
            charmed->GetMotionMaster()->MoveFleeing(source ? source : this);
        }
    }

    Creature* charmedCreature = nullptr;
    CharmInfo* charmInfo = charmed->GetCharmInfo();

    // if charm expires mid evade clear evade since movement is also cleared
    // TODO: maybe should be done on HomeMovementGenerator::MovementExpires
    charmed->GetCombatManager().SetEvadeState(EVADE_NONE);

    if (charmed->GetTypeId() == TYPEID_UNIT)
    {
        // now we have to clean threat list to be able to restore normal creature behavior
        charmedCreature = static_cast<Creature*>(charmed);
        if (!charmedCreature->IsPet())
        {
            charmedCreature->ClearTemporaryFaction();

            charmed->AttackStop(true, true);
            charmed->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
            charmed->InterruptSpell(CURRENT_MELEE_SPELL);

            charmInfo->ResetCharmState();
            charmed->DeleteCharmInfo();

            // first find friendly target (stopping combat here is not recommended because m_attackers will be modified)
            AttackerSet friendlyTargets;
            for (auto itr = charmed->getAttackers().begin(); itr != charmed->getAttackers().end(); ++itr)
            {
                Unit* attacker = (*itr);
                if (attacker->GetTypeId() != TYPEID_UNIT)
                    continue;

                if (charmed->CanAttack(attacker))
                    friendlyTargets.insert(attacker);
            }

            for (auto itr = charmed->getThreatManager().getThreatList().begin(); itr != charmed->getThreatManager().getThreatList().end(); ++itr)
            {
                Unit* attacker = (*itr)->getTarget();
                if (attacker->GetTypeId() != TYPEID_UNIT)
                    continue;

                if (!charmed->CanAttack(attacker))
                    friendlyTargets.insert(attacker);
            }

            // now stop attackers combat and transfer threat generated from this to owner, also get the total generated threat
            for (auto attacker : friendlyTargets)
            {
                attacker->AttackStop(true, true);
                attacker->getThreatManager().modifyThreatPercent(charmed, -101);     // only remove the possessed creature from threat list because it can be filled by other players
                attacker->AddThreat(this);
            }

            // we have to restore initial MotionMaster
            charmed->GetMotionMaster()->UnMarkFollowMovegens();
        }
        else
        {
            // safeguard against nullptr on totem elemental despawn
            if (Unit* owner = charmedCreature->GetOwner())
                charmed->setFaction(owner->getFaction());

            charmInfo->ResetCharmState();

            // as possessed is a pet we have to restore original charminfo so Pet::DeleteCharmInfo will take care of that
            charmed->DeleteCharmInfo();
        }

        charmed->SetTarget(charmed->GetVictim());
    }
    else if (charmed->GetTypeId() == TYPEID_PLAYER)
    {
        charmed->AttackStop(true, true);

        Player* charmedPlayer = static_cast<Player*>(charmed);

        if (player && player->IsInDuelWith(charmedPlayer))
            charmedPlayer->SetUInt32Value(PLAYER_DUEL_TEAM, player->GetUInt32Value(PLAYER_DUEL_TEAM) == 1 ? 2 : 1);
        else
            charmedPlayer->setFactionForRace(charmedPlayer->getRace());

        charmInfo->ResetCharmState();
        charmedPlayer->DeleteCharmInfo();

        charmedPlayer->ForceHealAndPowerUpdateInZone();

        charmed->GetMotionMaster()->Clear(false, true);
        charmed->GetMotionMaster()->MoveIdle();

        charmed->DeleteThreatList();

        charmed->SetTarget(nullptr);

        charmed->GetMotionMaster()->UnMarkFollowMovegens();
    }

    // Update possessed's client control status after altering flags
    if (const Player* controllingClientPlayer = charmed->GetClientControlling())
        controllingClientPlayer->UpdateClientControl(charmed, true);

    if (charmed->IsAlive()) // must be done after flag update
    {
        if (charmed->CanAttack(this))
        {
            if (!charmed->IsInCombat())
                EngageInCombatWithAggressor(charmed);

            if (charmed->GetTypeId() == TYPEID_UNIT)
                charmed->AddThreat(this, charmed->GetMaxHealth()); // Simulates being charmed
            this->AddThreat(charmed);

            if (charmed->GetTypeId() == TYPEID_UNIT)
            {
                Position const& pos = charmInfo->GetCharmStartPosition();
                if (!pos.IsEmpty())
                    static_cast<Creature*>(charmed)->SetCombatStartPosition(pos);
            }
        }
    }
    else
        charmed->GetCombatData()->threatManager.clearReferences();

    if (player)
    {
        if (possess)
        {
            player->GetCamera().ResetView();
            player->UpdateClientControl(charmed, false);
            player->SetMover(player);
        }
    }

    // be sure all those change will be made for clients
    SendForcedObjectUpdate();

    if (player)
    {
        // Player pet can be re-summoned here
        if (advertised)
        {
            player->ResummonPetTemporaryUnSummonedIfAny();

            // remove pet bar only if no pet
            if (!player->GetPet())
                player->RemovePetActionBar();
            else
                player->PetSpellInitialize();   // reset spell on pet bar

            if (player->GetGroup())
                player->SetGroupUpdateFlag(GROUP_UPDATE_PET);
        }
        player->ForceHealAndPowerUpdateInZone();
    }
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
    float RetDistance = GetDetectionRange();
    if (RetDistance == 0.f)
        return 0.0f;

    // "Aggro Radius varies with level difference at a rate of roughly 1 yard/level"
    // radius grow if playlevel < creaturelevel
    RetDistance -= (float)leveldif;

    // detect range auras
    RetDistance += GetTotalAuraModifier(SPELL_AURA_MOD_DETECT_RANGE);

    // detected range auras
    RetDistance += pl->GetTotalAuraModifier(SPELL_AURA_MOD_DETECTED_RANGE);

    // "Minimum Aggro Radius for a mob seems to be combat range (5 yards)"
    if (RetDistance < 5)
        RetDistance = 5;

    return (RetDistance * aggroRate);
}

void Unit::InterruptSpellsCastedOnMe(bool killDelayed)
{
    UnitList targets;
    // Maximum spell range=100m ?
    MaNGOS::AnyUnitInObjectRangeCheck u_check(this, 100.0f);
    MaNGOS::UnitListSearcher<MaNGOS::AnyUnitInObjectRangeCheck> searcher(targets, u_check);
    Cell::VisitAllObjects(this, searcher, GetMap()->GetVisibilityDistance());
    for (auto& target : targets)
    {
        if (!CanAttack(target))
            continue;
        for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
            if (Spell *spell = target->GetCurrentSpell(CurrentSpellTypes(i)))
                if (spell->m_targets.getUnitTargetGuid() == GetObjectGuid())
                    if (killDelayed || (spell->getState() == SPELL_STATE_CASTING && spell->GetCastedTime()) || i == CURRENT_CHANNELED_SPELL)
                        target->InterruptSpell(CurrentSpellTypes(i), true);
        if (!killDelayed)
            continue;
        // 2/ Interrupt spells that are not referenced but that still have an event (like delayed spell)
        for (auto i_events = target->m_events.GetEvents().begin(); i_events != target->m_events.GetEvents().end(); ++i_events)
            if (SpellEvent* event = dynamic_cast<SpellEvent*>(i_events->second))
                if (event && event->GetSpell()->m_targets.getUnitTargetGuid() == GetObjectGuid())
                    if (event->GetSpell()->getState() != SPELL_STATE_FINISHED)
                        event->GetSpell()->cancel();
    }
}

void Unit::UpdateAllowedPositionZ(float x, float y, float& z, Map* atMap /*=nullptr*/) const
{
    if (!atMap)
        atMap = GetMap();

    // non fly unit don't must be in air
    // non swim unit must be at ground (mostly speedup, because it don't must be in water and water level check less fast
    if (!CanFly())
    {
        bool canSwim = CanSwim();
        float groundZ = GetMap()->GetHeight(x, y, z, canSwim), maxZ;
        if (canSwim)
            maxZ = atMap->GetTerrain()->GetWaterOrGroundLevel(x, y, z, groundZ, !HasAuraType(SPELL_AURA_WATER_WALK), GetCollisionHeight());
        else
            maxZ = groundZ;
        if (maxZ > INVALID_HEIGHT)
        {
            if (z > maxZ)
                z = maxZ;
            else if (z < groundZ)
                z = groundZ;
        }
    }
    else
    {
        float groundZ = atMap->GetHeight(x, y, z);
        if (z < groundZ)
            z = groundZ;
    }
}

uint32 Unit::GetSpellRank(SpellEntry const* spellInfo)
{
    uint32 spellRank = getLevel();
    if (spellInfo->maxLevel > 0 && spellRank >= spellInfo->maxLevel * 5)
        spellRank = spellInfo->maxLevel * 5;
    return spellRank;
}

float Unit::OCTRegenHPPerSpirit() const
{
    float regen = 0.0f;

    float Spirit = GetStat(STAT_SPIRIT);
    uint8 Class = getClass();

    switch (Class)
    {
        case CLASS_DRUID:   regen = (Spirit * 0.11 + 1);    break;
        case CLASS_HUNTER:  regen = (Spirit * 0.43 - 5.5);  break;
        case CLASS_MAGE:    regen = (Spirit * 0.11 + 1);    break;
        case CLASS_PALADIN: regen = (Spirit * 0.25);        break;
        case CLASS_PRIEST:  regen = (Spirit * 0.15 + 1.4);  break;
        case CLASS_ROGUE:   regen = (Spirit * 0.84 - 13);   break;
        case CLASS_SHAMAN:  regen = (Spirit * 0.28 - 3.6);  break;
        case CLASS_WARLOCK: regen = (Spirit * 0.12 + 1.5);  break;
        case CLASS_WARRIOR: regen = (Spirit * 1.26 - 22.6); break;
    }

    return regen;
}

float Unit::OCTRegenMPPerSpirit() const
{
    float addvalue = 0.0;

    float Spirit = GetStat(STAT_SPIRIT);
    uint8 Class = getClass();

    switch (Class)
    {
        case CLASS_DRUID:   addvalue = (Spirit / 5 + 15);   break;
        case CLASS_HUNTER:  addvalue = (Spirit / 5 + 15);   break;
        case CLASS_MAGE:    addvalue = (Spirit / 4 + 12.5); break;
        case CLASS_PALADIN: addvalue = (Spirit / 5 + 15);   break;
        case CLASS_PRIEST:  addvalue = (Spirit / 4 + 12.5); break;
        case CLASS_SHAMAN:  addvalue = (Spirit / 5 + 17);   break;
        case CLASS_WARLOCK: addvalue = (Spirit / 5 + 15);   break;
    }

    addvalue /= 2.0f;   // the above addvalue are given per tick which occurs every 2 seconds, hence this divide by 2

    return addvalue;
}

float Unit::GetCollisionHeight() const
{
    float scaleMod = GetObjectScale(); // 99% sure about this

    //if (IsMounted()) - backport mounted from tbc if needed in future
    //{
    //    if (CreatureDisplayInfoEntry const* mountDisplayInfo = sCreatureDisplayInfoStore.LookupEntry(GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID)))
    //    {
    //        if (CreatureModelDataEntry const* mountModelData = sCreatureModelDataStore.LookupEntry(mountDisplayInfo->ModelId))
    //        {
    //            CreatureDisplayInfoEntry const* displayInfo = sCreatureDisplayInfoStore.LookupEntry(GetNativeDisplayId());
    //            MANGOS_ASSERT(displayInfo);
    //            CreatureModelDataEntry const* modelData = sCreatureModelDataStore.LookupEntry(displayInfo->ModelId);
    //            MANGOS_ASSERT(modelData);
    //            float const collisionHeight = scaleMod * (mountModelData->MountHeight + modelData->CollisionHeight * modelData->Scale * displayInfo->scale * 0.5f);
    //            return collisionHeight == 0.0f ? DEFAULT_COLLISION_HEIGHT : collisionHeight;
    //        }
    //    }
    //}

    //! Dismounting case - use basic default model data
    CreatureDisplayInfoEntry const* displayInfo = sCreatureDisplayInfoStore.LookupEntry(GetNativeDisplayId());
    MANGOS_ASSERT(displayInfo);
    CreatureModelDataEntry const* modelData = sCreatureModelDataStore.LookupEntry(displayInfo->ModelId);
    MANGOS_ASSERT(modelData);

    float const collisionHeight = scaleMod * modelData->CollisionHeight * modelData->Scale * displayInfo->scale;
    return collisionHeight == 0.0f ? DEFAULT_COLLISION_HEIGHT : collisionHeight;
}

bool Unit::HasOverrideScript(uint32 id) const
{
    return m_classScripts.find(id) != m_classScripts.end();
}

Aura* Unit::GetOverrideScript(uint32 id) const
{
    auto itr = m_classScripts.find(id);
    return itr == m_classScripts.end() ? nullptr : (*itr).second;
}

void Unit::RegisterOverrideScriptAura(Aura* aura, uint32 id, bool apply)
{
    if (apply)
        m_classScripts.insert({ id, aura });
    else
        m_classScripts.erase(id);
}

void Unit::RegisterScriptedLocationAura(Aura* aura, AuraScriptLocation location, bool apply)
{
    if (apply)
        m_scriptedLocations[location].push_back(aura);
    else
        m_scriptedLocations[location].erase(std::remove(m_scriptedLocations[location].begin(), m_scriptedLocations[location].end(), aura), m_scriptedLocations[location].end());
}

void Unit::AddComboPoints(Unit* target, int8 count)
{
    if (!count)
        return;

    // without combo points lost (duration checked in aura)
    RemoveSpellsCausingAura(SPELL_AURA_RETAIN_COMBO_POINTS);

    if (target->GetObjectGuid() == m_comboTargetGuid)
    {
        m_comboPoints += count;
    }
    else
    {
        if (m_comboTargetGuid)
            if (Unit* target2 = ObjectAccessor::GetUnit(*this, m_comboTargetGuid))
                target2->RemoveComboPointHolder(GetGUIDLow());

        m_comboTargetGuid = target->GetObjectGuid();
        m_comboPoints = count;

        target->AddComboPointHolder(GetGUIDLow());
    }

    if (m_comboPoints > 5) m_comboPoints = 5;
    if (m_comboPoints < 0) m_comboPoints = 0;

    if (IsPlayer())
        static_cast<Player*>(this)->SendComboPoints();
}

void Unit::ClearComboPoints()
{
    if (!m_comboTargetGuid)
        return;

    // without combopoints lost (duration checked in aura)
    RemoveSpellsCausingAura(SPELL_AURA_RETAIN_COMBO_POINTS);

    m_comboPoints = 0;

    if (Unit* target = ObjectAccessor::GetUnit(*this, m_comboTargetGuid))
        target->RemoveComboPointHolder(GetGUIDLow());

    if (IsPlayer())
        static_cast<Player*>(this)->SendComboPoints();

    m_comboTargetGuid.Clear();
}

uint32 Unit::GetModifierXpBasedOnDamageReceived(uint32 xp)
{
    if (xp && IsCreature() && GetDamageDoneByOthers())
    {
        uint32 health = GetMaxHealth();
        float percentageHp = float(GetDamageDoneByOthers()) / health;
        if (percentageHp >= 1.f)
            xp = 0;
        else if (percentageHp > 0.f)
            xp *= (1.f - percentageHp);
    }
    return xp;
}
