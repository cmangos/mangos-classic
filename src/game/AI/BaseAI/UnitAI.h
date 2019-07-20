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

#ifndef MANGOS_UNITAI_H
#define MANGOS_UNITAI_H

#include "Platform/Define.h"
#include "Dynamic/FactoryHolder.h"
#include "Globals/SharedDefines.h"
#include "Dynamic/FactoryHolder.h"
#include "Entities/ObjectGuid.h"

class WorldObject;
class GameObject;
class Unit;
class Creature;
class Player;
struct SpellEntry;
class ChatHandler;
class Spell;

#define TIME_INTERVAL_LOOK   5000
#define VISIBILITY_RANGE    10000
#define DISTANCING_CONSTANT 1.f

enum CanCastResult
{
    CAST_OK                     = 0,
    CAST_FAIL_IS_CASTING        = 1,
    CAST_FAIL_OTHER             = 2,
    CAST_FAIL_TOO_FAR           = 3,
    CAST_FAIL_TOO_CLOSE         = 4,
    CAST_FAIL_POWER             = 5,
    CAST_FAIL_STATE             = 6,
    CAST_FAIL_TARGET_AURA       = 7,
    CAST_FAIL_NOT_IN_LOS        = 8,
    CAST_FAIL_COOLDOWN          = 9,
    CAST_FAIL_EVADE             = 10,
};

enum CastFlags
{
    CAST_INTERRUPT_PREVIOUS     = 0x01,                     // Interrupt any spell casting
    CAST_TRIGGERED              = 0x02,                     // Triggered (this makes spell cost zero mana and have no cast time)
    CAST_FORCE_CAST             = 0x04,                     // Forces cast even if creature is out of mana or out of range
    CAST_NO_MELEE_IF_OOM        = 0x08,                     // Prevents creature from entering melee if out of mana or out of range
    CAST_FORCE_TARGET_SELF      = 0x10,                     // Forces the target to cast this spell on itself
    CAST_AURA_NOT_PRESENT       = 0x20,                     // Only casts the spell if the target does not have an aura from the spell
    CAST_IGNORE_UNSELECTABLE_TARGET = 0x40,                 // Can target UNIT_FLAG_NOT_SELECTABLE - Needed in some scripts
    CAST_SWITCH_CASTER_TARGET   = 0x80,                     // Switches target and caster for spell cast
    CAST_MAIN_SPELL             = 0x100,                    // Marks main spell
    CAST_PLAYER_ONLY            = 0x200,                    // Selects only player targets - substitution for EAI not having more params
    CAST_DISTANCE_YOURSELF      = 0x400,                    // If spell with this cast flag hits main aggro target, caster distances himself - EAI only
};

enum AIEventType
{
    // Usable with Event AI
    AI_EVENT_JUST_DIED          = 0,                        // Sender = Killed Npc, Invoker = Killer
    AI_EVENT_CRITICAL_HEALTH    = 1,                        // Sender = Hurt Npc, Invoker = DamageDealer - Expected to be sent by 10% health
    AI_EVENT_LOST_HEALTH        = 2,                        // Sender = Hurt Npc, Invoker = DamageDealer - Expected to be sent by 50% health
    AI_EVENT_LOST_SOME_HEALTH   = 3,                        // Sender = Hurt Npc, Invoker = DamageDealer - Expected to be sent by 90% health
    AI_EVENT_GOT_FULL_HEALTH    = 4,                        // Sender = Healed Npc, Invoker = Healer
    AI_EVENT_CUSTOM_EVENTAI_A   = 5,                        // Sender = Npc that throws custom event, Invoker = TARGET_T_ACTION_INVOKER (if exists)
    AI_EVENT_CUSTOM_EVENTAI_B   = 6,                        // Sender = Npc that throws custom event, Invoker = TARGET_T_ACTION_INVOKER (if exists)
    AI_EVENT_GOT_CCED           = 7,                        // Sender = CCed Npc, Invoker = Caster that CCed
    AI_EVENT_CUSTOM_EVENTAI_C   = 8,                        // Sender = Npc that throws custom event, Invoker = TARGET_T_ACTION_INVOKER (if exists)
    AI_EVENT_CUSTOM_EVENTAI_D   = 9,                        // Sender = Npc that throws custom event, Invoker = TARGET_T_ACTION_INVOKER (if exists)
    AI_EVENT_CUSTOM_EVENTAI_E   = 10,                       // Sender = Npc that throws custom event, Invoker = TARGET_T_ACTION_INVOKER (if exists)
    AI_EVENT_CUSTOM_EVENTAI_F   = 11,                       // Sender = Npc that throws custom event, Invoker = TARGET_T_ACTION_INVOKER (if exists)
    MAXIMAL_AI_EVENT_EVENTAI    = 12,

    // Internal Use
    AI_EVENT_CALL_ASSISTANCE    = 13,                       // Sender = Attacked Npc, Invoker = Enemy

    // Predefined for SD2
    AI_EVENT_START_ESCORT       = 100,                      // Invoker = Escorting Player
    AI_EVENT_START_ESCORT_B     = 101,                      // Invoker = Escorting Player
    AI_EVENT_START_EVENT        = 102,                      // Invoker = EventStarter
    AI_EVENT_START_EVENT_A      = 103,                      // Invoker = EventStarter
    AI_EVENT_START_EVENT_B      = 104,                      // Invoker = EventStarter

    // Some IDs for special cases in SD2
    AI_EVENT_CUSTOM_A           = 1000,
    AI_EVENT_CUSTOM_B           = 1001,
    AI_EVENT_CUSTOM_C           = 1002,
    AI_EVENT_CUSTOM_D           = 1003,
    AI_EVENT_CUSTOM_E           = 1004,
    AI_EVENT_CUSTOM_F           = 1005,
};

enum ReactStates
{
    REACT_PASSIVE    = 0,
    REACT_DEFENSIVE  = 1,
    REACT_AGGRESSIVE = 2
};

enum AIOrders
{
    ORDER_NONE,
    ORDER_DISTANCING,
    ORDER_FLEEING,
    ORDER_EVADE,
    ORDER_CUSTOM,
};

class UnitAI
{
    public:
        explicit UnitAI(Unit* unit);

        virtual ~UnitAI();

        ///== Information about AI ========================
        /**
         * This funcion is used to display information about the AI.
         * It is called when the .npc aiinfo command is used.
         * Use this for on-the-fly debugging
         * @param reader is a ChatHandler to send messages to.
         */
        virtual void GetAIInformation(ChatHandler& /*reader*/) {}

        ///== Reactions At =================================

        /**
         * Called if IsVisible(Unit* pWho) is true at each (relative) override pWho move, reaction at visibility zone enter
         * Note: The Unit* pWho can be out of Line of Sight, usually this is only visibiliy (by state) and range dependendend
         * Note: This function is not called for creatures who are in evade mode
         * @param pWho Unit* who moved in the visibility range and is visisble
         */
        virtual void MoveInLineOfSight(Unit* /*who*/);

        /**
         * Called for reaction at enter to combat if not in combat yet
         * @param enemy Unit* of whom the Creature enters combat with, can be nullptr
         */
        virtual void EnterCombat(Unit* /*enemy*/) {}

        /**
         * Called for reaction at stopping attack at no attackers or targets
         * This is called usually in Unit::SelectHostileTarget, if no more target exists
         */
        virtual void EnterEvadeMode();

        /**
         * Called at reaching home after MoveTargetedHome
         */
        virtual void JustReachedHome() {}

        /**
        * Called at reaching home after MoveTargetedHome
        */
        virtual void SummonedJustReachedHome(Creature* /*summoned*/) {}

        /**
         * Called at any Heal received from any Unit
         * @param pHealer Unit* which deals the heal
         * @param uiHealedAmount Amount of healing received
         */
        virtual void HealedBy(Unit* /*healer*/, uint32& /*healedAmount*/) {}

        /**
         * Called at any Damage to any victim (before damage apply)
         * @param pDoneTo Unit* to whom Damage of amount uiDamage will be dealt
         * @param uiDamage Amount of Damage that will be dealt, can be changed here
         */
        virtual void DamageDeal(Unit* /*doneTo*/, uint32& /*damage*/, DamageEffectType /*damageType*/, SpellEntry const* /*spellInfo*/) {}

        /**
         * Called at any Damage from any attacker (before damage apply)
         * Note:    Use for recalculation damage or special reaction at damage
         *          for attack reaction use AttackedBy called for not DOT damage in Unit::DealDamage also
         * @param pDealer Unit* who will deal Damage to the creature
         * @param uiDamage Amount of Damage that will be dealt, can be changed here
         */
        virtual void DamageTaken(Unit* /*dealer*/, uint32& /*damage*/, DamageEffectType /*damageType*/, SpellEntry const* /*spellInfo*/) {}

        /**
         * Called when the creature is killed
         * @param pKiller Unit* who killed the creature
         */
        virtual void JustDied(Unit* /*killer*/) {}

        /**
         * Called when the corpse of this creature gets removed
         * @param uiRespawnDelay Delay (in seconds). If != 0, then this is the time after which the creature will respawn, if = 0 the default respawn-delay will be used
         */
        virtual void CorpseRemoved(uint32& /*respawnDelay*/) {}

        /**
         * Called when a summoned creature is killed
         * @param pSummoned Summoned Creature* that got killed
         */
        virtual void SummonedCreatureJustDied(Creature* /*summoned*/) {}

        /**
         * Called when the creature kills a unit
         * @param pVictim Victim that got killed
         */
        virtual void KilledUnit(Unit* /*victim*/) {}

        /**
         * Called when owner of m_creature (if m_creature is PROTECTOR_PET) kills a unit
         * @param pVictim Victim that got killed (by owner of creature)
         */
        virtual void OwnerKilledUnit(Unit* /*victim*/) {}

        /**
         * Called when the creature summon successfully other creature
         * @param pSummoned Creature that got summoned
         */
        virtual void JustSummoned(Creature* /*summoned*/) {}

        /**
         * Called when the creature summon successfully a gameobject
         * @param pGo GameObject that was summoned
         */
        virtual void JustSummoned(GameObject* /*go*/) {}

        /**
         * Called when a summoned creature gets TemporarySummon::UnSummon ed
         * @param pSummoned Summoned creature that despawned
         */
        virtual void SummonedCreatureDespawn(Creature* /*summoned*/) {}

        /**
         * Called when hit by a spell
         * @param pCaster Caster who casted the spell
         * @param pSpell The spell that hit the creature
         */
        virtual void SpellHit(Unit* /*caster*/, const SpellEntry* /*spellInfo*/) {}

        /**
         * Called when spell hits creature's target
         * @param pTarget Target that we hit with the spell
         * @param pSpell Spell with which we hit pTarget
         */
        virtual void SpellHitTarget(Unit* target, const SpellEntry* spellInfo, SpellMissInfo /*missInfo*/) { SpellHitTarget(target, spellInfo); }
        virtual void SpellHitTarget(Unit* /*target*/, const SpellEntry* /*spellInfo*/) {}

        /**
         * Called when the creature is target of hostile action: swing, hostile spell landed, fear/etc)
         * @param pAttacker Unit* who attacked the creature
         */
        virtual void AttackedBy(Unit* attacker);

        /**
         * Called when creature is respawned (for reseting variables)
         */
        virtual void JustRespawned() {}

        /**
         * Called at waypoint reached or point movement finished
         * @param uiMovementType Type of the movement (enum MovementGeneratorType)
         * @param uiData Data related to the finished movement (ie point-id)
         */
        virtual void MovementInform(uint32 /*movementType*/, uint32 /*data*/) {}

        /**
         * Called if a temporary summoned of m_creature reach a move point
         * @param pSummoned Summoned Creature that finished some movement
         * @param uiMotionType Type of the movement (enum MovementGeneratorType)
         * @param uiData Data related to the finished movement (ie point-id)
         */
        virtual void SummonedMovementInform(Creature* /*summoned*/, uint32 /*motionType*/, uint32 /*data*/) {}

        /**
         * Called at text emote receive from player
         * @param pPlayer Player* who sent the emote
         * @param uiEmote ID of the emote the player used with the creature as target
         */
        virtual void ReceiveEmote(Player* /*player*/, uint32 /*emote*/) {}

        ///== Data Fetch bind ==============================

        virtual uint32 GetScriptData() { return 0; }

        ///== Triggered Actions Requested ==================

        /**
         * Called when creature attack expected (if creature can and no have current victim)
         * Note: for reaction at hostile action must be called AttackedBy function.
         * Note: Usually called by MoveInLineOfSight, in Unit::SelectHostileTarget or when the AI is forced to attack an enemy
         * @param pWho Unit* who is possible target
         */
        virtual void AttackStart(Unit* /*who*/);

        /**
        * Called when creature stop attack expected
        * Note: have to be called to reinitialize some states or movement
        * Note: Not needed for creature, they use EnterEvadeMode
        */
        virtual void CombatStop() {}

        /**
         * Called at World update tick, by default every 100ms
         * This setting is dependend on CONFIG_UINT32_INTERVAL_MAPUPDATE
         * Note: Use this function to handle Timers, Threat-Management and MeleeAttacking
         * @param uiDiff Passed time since last call
         */
        virtual void UpdateAI(const uint32 /*diff*/) {}

        ///== State checks =================================

        /**
         * Check if unit is visible for MoveInLineOfSight
         * Note: This check is by default only the state-depending (visibility, range), NOT LineOfSight
         * @param pWho Unit* who is checked if it is visisble for the creature
         */
        virtual bool IsVisible(Unit* /*who*/) const;

        /// Check if this AI can be replaced in possess case
        // virtual bool IsControllable() const { return false; }

        ///== Helper functions =============================

        /// This function is used to do the actual melee damage (if possible)
        bool DoMeleeAttackIfReady() const;

        /// Internal helper function, to check if a spell can be cast
        CanCastResult CanCastSpell(Unit* target, const SpellEntry* spellInfo, bool isTriggered) const;

        /**
         * Function to cast a spell if possible
         * @param pTarget Unit* onto whom the spell should be cast
         * @param uiSpell ID of the spell that the creature will try to cast
         * @param uiCastFlags Some flags to define how to cast, see enum CastFlags
         * @param OriginalCasterGuid the original caster of the spell if required, empty by default
         */
        CanCastResult DoCastSpellIfCan(Unit* target, uint32 spellId, uint32 castFlags = 0, ObjectGuid originalCasterGUID = ObjectGuid()) const;

        /// Set combat movement (on/off), also sets UNIT_STAT_NO_COMBAT_MOVEMENT
        void SetCombatMovement(bool enable, bool stopOrStartMovement = false);
        bool IsCombatMovement() const;

        ///== Event Handling ===============================

        /**
         * Send an AI Event to nearby Creatures around
         * @param uiType number to specify the event, default cases listed in enum AIEventType
         * @param pInvoker Unit that triggered this event (like an attacker)
         * @param uiDelay  delay time until the Event will be triggered
         * @param fRadius  range in which for receiver is searched
         */
        void SendAIEventAround(AIEventType eventType, Unit* invoker, uint32 delay, float radius, uint32 miscValue = 0) const;

        /**
         * Send an AI Event to a Creature
         * @param eventType to specify the event, default cases listed in enum AIEventType
         * @param pInvoker Unit that triggered this event (like an attacker)
         * @param pReceiver Creature to receive this event
         */
        void SendAIEvent(AIEventType eventType, Unit* invoker, Unit* receiver, uint32 miscValue = 0) const;

        /**
         * Called when an AI Event is received
         * @param eventType to specify the event, default cases listed in enum AIEventType
         * @param pSender Creature that sent this event
         * @param pInvoker Unit that triggered this event (like an attacker)
         */
        virtual void ReceiveAIEvent(AIEventType /*eventType*/, Unit* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/) {}

        /**
        * Called when a Game Event starts or ends
        * @param eventId to specify id of event from database
        * @param activate to specify if it started or stopped
        * @param resume to specify whether it launched normally or was resumed after a restart
        */
        virtual void OnEventHappened(uint16 /*eventId*/, bool /*activate*/, bool /*resume*/) {}

        /*
         * Evaluates conditions and returns true if it is going to assist player
         */
        virtual bool AssistPlayerInCombat(Unit* /*who*/) { return false; }

        /*
         * Called when a spell is interrupted
         * @param spellInfo to specify which spell was interrupted
         */
        virtual void OnSpellInterrupt(SpellEntry const* /*spellInfo*/) {}

        /*
         * Notifies AI on cast state change
         */
        virtual void OnSpellCastStateChange(Spell const* spell, bool state, WorldObject* target = nullptr);

        /*
         * Notifies AI on channel state update
         */
        virtual void OnChannelStateChange(Spell const* spell, bool state, WorldObject* target = nullptr);

        /*
         * Notifies AI on successfull spell execution
         */
        virtual void OnSpellCooldownAdded(SpellEntry const* /*spellInfo*/) {}

        void CheckForHelp(Unit* /*who*/, Unit* /*me*/, float /*dist*/);
        void DetectOrAttack(Unit* who);
        bool CanTriggerStealthAlert(Unit* who, float attackRadius) const;

        virtual void HandleMovementOnAttackStart(Unit* victim) const;


        // TODO: Implement proper casterAI in EAI and remove this from Leotheras script
        void SetMoveChaseParams(float dist, float angle, bool moveFurther) { m_attackDistance = dist; m_attackAngle = angle; m_moveFurther = moveFurther; }

        // Returns friendly unit with the most amount of hp missing from max hp - ignoreSelf - some spells cant target self
        Unit* DoSelectLowestHpFriendly(float range, float minMissing = 1.f, bool percent = false, bool ignoreSelf = false);

        // Start movement toward victim
        void DoStartMovement(Unit* victim);

        void SetReactState(ReactStates st) { m_reactState = st; }
        ReactStates GetReactState() const { return m_reactState; }
        bool HasReactState(ReactStates state) const { return (m_reactState == state); }

        virtual bool CanExecuteCombatAction();
        void SetCombatScriptStatus(bool state) { m_combatScriptHappening = state; };
        bool GetCombatScriptStatus() const { return m_combatScriptHappening; }

        void SetAIOrder(AIOrders order) { m_currentAIOrder = order; }
        AIOrders GetAIOrder() const { return m_currentAIOrder; }

        void DoFlee();
        void DoDistance(); // TODO
        void DoCallForHelp(); // TODO
        void DoSeekAssistance(); // TODO

        void SetMeleeEnabled(bool state);

        // Rough prototype, we might not need such fidelity
        virtual void JustRootedTarget(SpellEntry const* spellInfo, Unit* victim) { JustStoppedMovementOfTarget(spellInfo, victim); }
        virtual void JustStunnedTarget(SpellEntry const* spellInfo, Unit* victim) { JustStoppedMovementOfTarget(spellInfo, victim); }
        virtual void JustStoppedMovementOfTarget(SpellEntry const* /*spellInfo*/, Unit* /*victim*/) {}

        // AI selection - works in connection with IsPossessCharmType
        virtual bool CanHandleCharm() { return false; }

        // Movement generator responses
        virtual void TimedFleeingEnded();

        virtual void DistancingStarted();
        virtual void DistancingEnded();

    protected:
        virtual std::string GetAIName() { return "UnitAI"; }

        ///== Fields =======================================

        /// Pointer to the Creature controlled by this AI
        Unit* m_unit;

        /// How should an enemy be chased
        float m_attackDistance;
        float m_attackAngle;
        bool m_moveFurther;
        bool m_chaseRun;

        // How far a creature can detect in MoveInLineOfSight
        float m_visibilityDistance;

        bool m_combatMovementStarted;

        bool m_dismountOnAggro;

        bool m_meleeEnabled;                              // If we allow melee auto attack

        ReactStates m_reactState;

        bool m_combatScriptHappening;                    // disables normal combat functions without leaving combat
        AIOrders m_currentAIOrder;
};

struct SelectableAI : public FactoryHolder<UnitAI>, public Permissible<Creature>
{
    SelectableAI(const char* id) : FactoryHolder<UnitAI>(id) {}
};

template<class REAL_AI>
struct CreatureAIFactory : public SelectableAI
{
    CreatureAIFactory(const char* name) : SelectableAI(name) {}

    UnitAI* Create(void*) const override;

    int Permit(const Creature* c) const { return REAL_AI::Permissible(c); }
};

enum Permitions
{
    PERMIT_BASE_NO                 = -1,
    PERMIT_BASE_IDLE               = 1,
    PERMIT_BASE_REACTIVE           = 100,
    PERMIT_BASE_PROACTIVE          = 200,
    PERMIT_BASE_FACTION_SPECIFIC   = 400,
    PERMIT_BASE_SPECIAL            = 800
};

typedef FactoryHolder<UnitAI> CreatureAICreator;
typedef FactoryHolder<UnitAI>::FactoryHolderRegistry CreatureAIRegistry;
typedef FactoryHolder<UnitAI>::FactoryHolderRepository CreatureAIRepository;

#endif
