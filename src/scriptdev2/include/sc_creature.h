/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_CREATURE_H
#define SC_CREATURE_H

#include "Chat.h"
#include "DBCStores.h"                                      // Mostly only used the Lookup acces, but a few cases really do use the DBC-Stores

// Spell targets used by SelectSpell
enum SelectTarget
{
    SELECT_TARGET_DONTCARE = 0,                             // All target types allowed

    SELECT_TARGET_SELF,                                     // Only Self casting

    SELECT_TARGET_SINGLE_ENEMY,                             // Only Single Enemy
    SELECT_TARGET_AOE_ENEMY,                                // Only AoE Enemy
    SELECT_TARGET_ANY_ENEMY,                                // AoE or Single Enemy

    SELECT_TARGET_SINGLE_FRIEND,                            // Only Single Friend
    SELECT_TARGET_AOE_FRIEND,                               // Only AoE Friend
    SELECT_TARGET_ANY_FRIEND,                               // AoE or Single Friend
};

// Spell Effects used by SelectSpell
enum SelectEffect
{
    SELECT_EFFECT_DONTCARE = 0,                             // All spell effects allowed
    SELECT_EFFECT_DAMAGE,                                   // Spell does damage
    SELECT_EFFECT_HEALING,                                  // Spell does healing
    SELECT_EFFECT_AURA,                                     // Spell applies an aura
};

enum SCEquip
{
    EQUIP_NO_CHANGE = -1,
    EQUIP_UNEQUIP   = 0
};

/// Documentation of CreatureAI functions can be found in MaNGOS source
// Only list them here again to ensure that the interface between SD2 and the core is not changed unnoticed
struct ScriptedAI : public CreatureAI
{
    public:
        explicit ScriptedAI(Creature* pCreature);
        ~ScriptedAI() {}

        // *************
        // CreatureAI Functions
        // *************

        // == Information about AI ========================
        // Get information about the AI
        void GetAIInformation(ChatHandler& reader) override;

        // == Reactions At =================================

        // Called if IsVisible(Unit* pWho) is true at each relative pWho move
        void MoveInLineOfSight(Unit* pWho) override;

        // Called for reaction at enter to combat if not in combat yet (enemy can be NULL)
        void EnterCombat(Unit* pEnemy) override;

        // Called at stoping attack by any attacker
        void EnterEvadeMode() override;

        // Called when reached home after MoveTargetHome (in evade)
        void JustReachedHome() override {}

        // Called at any Heal received
        void HealedBy(Unit* /*pHealer*/, uint32& /*uiHealedAmount*/) override {}

        // Called at any Damage to any victim (before damage apply)
        void DamageDeal(Unit* /*pDoneTo*/, uint32& /*uiDamage*/) override {}

        // Called at any Damage from any attacker (before damage apply)
        void DamageTaken(Unit* /*pDealer*/, uint32& /*uiDamage*/) override {}

        // Called at creature death
        void JustDied(Unit* /*pKiller*/) override {}

        // Called when the corpse of this creature gets removed
        void CorpseRemoved(uint32& /*uiRespawnDelay*/) override {}

        // Called when a summoned creature is killed
        void SummonedCreatureJustDied(Creature* /*pSummoned*/) override {}

        // Called at creature killing another unit
        void KilledUnit(Unit* /*pVictim*/) override {}

        // Called when owner of m_creature (if m_creature is PROTECTOR_PET) kills a unit
        void OwnerKilledUnit(Unit* /*pVictim*/) override {}

        // Called when the creature successfully summons a creature
        void JustSummoned(Creature* /*pSummoned*/) override {}

        // Called when the creature successfully summons a gameobject
        void JustSummoned(GameObject* /*pGo*/) override {}

        // Called when a summoned creature gets TemporarySummon::UnSummon ed
        void SummonedCreatureDespawn(Creature* /*pSummoned*/) override {}

        // Called when hit by a spell
        void SpellHit(Unit* /*pCaster*/, const SpellEntry* /*pSpell*/) override {}

        // Called when spell hits creature's target
        void SpellHitTarget(Unit* /*pTarget*/, const SpellEntry* /*pSpell*/) override {}

        // Called when the creature is target of hostile action: swing, hostile spell landed, fear/etc)
        /// This will by default result in reattacking, if the creature has no victim
        void AttackedBy(Unit* pAttacker) override { CreatureAI::AttackedBy(pAttacker); }

        // Called when creature is respawned (for reseting variables)
        void JustRespawned() override;

        // Called at waypoint reached or point movement finished
        void MovementInform(uint32 /*uiMovementType*/, uint32 /*uiData*/) override {}

        // Called if a temporary summoned of m_creature reach a move point
        void SummonedMovementInform(Creature* /*pSummoned*/, uint32 /*uiMotionType*/, uint32 /*uiData*/) override {}

        // Called at text emote receive from player
        void ReceiveEmote(Player* /*pPlayer*/, uint32 /*uiEmote*/) override {}

        // Called at each attack of m_creature by any victim
        void AttackStart(Unit* pWho) override;

        // Called at World update tick
        void UpdateAI(const uint32) override;

        // Called when an AI Event is received
        void ReceiveAIEvent(AIEventType /*eventType*/, Creature* /*pSender*/, Unit* /*pInvoker*/, uint32 /*uiMiscValue*/) override {}

        // == State checks =================================

        // Check if unit is visible for MoveInLineOfSight
        bool IsVisible(Unit* pWho) const override;

        // Called when victim entered water and creature can not enter water
        bool canReachByRangeAttack(Unit* pWho) override { return CreatureAI::canReachByRangeAttack(pWho); }

        // *************
        // Variables
        // *************

        // *************
        // Pure virtual functions
        // *************

        /**
         * This is a SD2 internal function, that every AI must implement
         * Usally used to reset combat variables
         * Called by default on creature evade and respawn
         * In most scripts also called in the constructor of the AI
         */
        virtual void Reset() = 0;

        /// Called at creature EnterCombat with an enemy
        /**
         * This is a SD2 internal function
         * Called by default on creature EnterCombat with an enemy
         */
        virtual void Aggro(Unit* /*pWho*/) {}

        // *************
        // AI Helper Functions
        // *************

        // Start movement toward victim
        void DoStartMovement(Unit* pVictim, float fDistance = 0, float fAngle = 0);

        // Start no movement on victim
        void DoStartNoMovement(Unit* pVictim);

        // Stop attack of current victim
        void DoStopAttack();

        // Cast spell by Id
        void DoCast(Unit* pTarget, uint32 uiSpellId, bool bTriggered = false);

        // Cast spell by spell info
        void DoCastSpell(Unit* pTarget, SpellEntry const* pSpellInfo, bool bTriggered = false);

        // Plays a sound to all nearby players
        void DoPlaySoundToSet(WorldObject* pSource, uint32 uiSoundId);

        // Drops all threat to 0%. Does not remove enemies from the threat list
        void DoResetThreat();

        // Teleports a player without dropping threat (only teleports to same map)
        void DoTeleportPlayer(Unit* pUnit, float fX, float fY, float fZ, float fO);

        // Returns friendly unit with the most amount of hp missing from max hp
        Unit* DoSelectLowestHpFriendly(float fRange, uint32 uiMinHPDiff = 1);

        // Returns a list of friendly CC'd units within range
        std::list<Creature*> DoFindFriendlyCC(float fRange);

        // Returns a list of all friendly units missing a specific buff within range
        std::list<Creature*> DoFindFriendlyMissingBuff(float fRange, uint32 uiSpellId);

        // Return a player with at least minimumRange from m_creature
        Player* GetPlayerAtMinimumRange(float fMinimumRange);

        // Spawns a creature relative to m_creature
        Creature* DoSpawnCreature(uint32 uiId, float fX, float fY, float fZ, float fAngle, uint32 uiType, uint32 uiDespawntime);

        // Returns spells that meet the specified criteria from the creatures spell list
        SpellEntry const* SelectSpell(Unit* pTarget, int32 uiSchool, int32 iMechanic, SelectTarget selectTargets, uint32 uiPowerCostMin, uint32 uiPowerCostMax, float fRangeMin, float fRangeMax, SelectEffect selectEffect);

        // Checks if you can cast the specified spell
        bool CanCast(Unit* pTarget, SpellEntry const* pSpell, bool bTriggered = false);

        void SetEquipmentSlots(bool bLoadDefault, int32 iMainHand = EQUIP_NO_CHANGE, int32 iOffHand = EQUIP_NO_CHANGE, int32 iRanged = EQUIP_NO_CHANGE);

        bool EnterEvadeIfOutOfCombatArea(const uint32 uiDiff);

    private:
        uint32 m_uiEvadeCheckCooldown;
};

struct Scripted_NoMovementAI : public ScriptedAI
{
    Scripted_NoMovementAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        SetCombatMovement(false);
    }

    void GetAIInformation(ChatHandler& reader) override;

    // Called at each attack of m_creature by any victim
    void AttackStart(Unit* pWho) override;
};

#endif
