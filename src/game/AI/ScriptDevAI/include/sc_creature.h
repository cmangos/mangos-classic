/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_CREATURE_H
#define SC_CREATURE_H

#include "Chat/Chat.h"
#include "Server/DBCStores.h"                               // Mostly only used the Lookup acces, but a few cases really do use the DBC-Stores
#include "AI/BaseAI/CreatureAI.h"
#include "Entities/Creature.h"

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

/// Documentation of UnitAI functions can be found in MaNGOS source
// Only list them here again to ensure that the interface between SD2 and the core is not changed unnoticed
struct ScriptedAI : public CreatureAI
{
    public:
        explicit ScriptedAI(Creature* creature, uint32 combatActions);
        explicit ScriptedAI(Creature* creature) : ScriptedAI(creature, 0) {}
        ~ScriptedAI() {}

        // *************
        // UnitAI Functions
        // *************

        // == Information about AI ========================
        // Get information about the AI
        void GetAIInformation(ChatHandler& reader) override;

        // == Reactions At =================================

        // Called if IsVisible(Unit* pWho) is true at each relative pWho move
        // void MoveInLineOfSight(Unit* who) override;

        // Called for reaction at enter to combat if not in combat yet (enemy can be nullptr)
        void EnterCombat(Unit* enemy) override;

        // Called at stoping attack by any attacker
        // void EnterEvadeMode() override;

        // Called when reached home after MoveTargetHome (in evade)
        // void JustReachedHome() override {}

        // Called at any Heal received
        // void HealedBy(Unit* /*healer*/, uint32& /*healedAmount*/) override {}

        // Called at any Damage to any victim (before damage apply)
        // void DamageDeal(Unit* /*doneTo*/, uint32& /*damage*/, DamageEffectType damagetype) override {}

        // Called at any Damage from any attacker (before damage apply) - dealer can be nullptr during DOT tick
        // void DamageTaken(Unit* /*dealer*/, uint32& /*damage*/, DamageEffectType damagetype) override {}

        // Called at creature death
        // void JustDied(Unit* /*killer*/) override {}

        // Called when the corpse of this creature gets removed
        // void CorpseRemoved(uint32& /*respawnDelay*/) override {}

        // Called when a summoned creature is killed
        // void SummonedCreatureJustDied(Creature* /*summoned*/) override {}

        // Called at creature killing another unit
        // void KilledUnit(Unit* /*victim*/) override {}

        // Called when owner of m_creature (if m_creature is PROTECTOR_PET) kills a unit
        // void OwnerKilledUnit(Unit* /*victim*/) override {}

        // Called when the creature successfully summons a creature
        // void JustSummoned(Creature* /*summoned*/) override {}

        // Called when the creature successfully summons a gameobject
        // void JustSummoned(GameObject* /*go*/) override {}

        // Called when a summoned creature gets TemporarySummon::UnSummon ed
        // void SummonedCreatureDespawn(Creature* /*summoned*/) override {}

        // Called when hit by a spell
        // void SpellHit(Unit* /*caster*/, const SpellEntry* /*spell*/) override {}

        // Called when spell hits creature's target
        // void SpellHitTarget(Unit* /*target*/, const SpellEntry* /*spell*/) override {}

        // Called when the creature is target of hostile action: swing, hostile spell landed, fear/etc)
        /// This will by default result in reattacking, if the creature has no victim
        // void AttackedBy(Unit* pAttacker) override { UnitAI::AttackedBy(pAttacker); }

        // Called when creature is respawned (for reseting variables)
        // void JustRespawned() override;

        // Called at waypoint reached or point movement finished
        // void MovementInform(uint32 /*movementType*/, uint32 /*data*/) override {}

        // Called if a temporary summoned of m_creature reach a move point
        // void SummonedMovementInform(Creature* /*summoned*/, uint32 /*motionType*/, uint32 /*data*/) override {}

        // Called at text emote receive from player
        // void ReceiveEmote(Player* /*player*/, uint32 /*emote*/) override {}

        // Called at each attack of m_creature by any victim
        // void AttackStart(Unit* who) override;

        // Called at World update tick
        // void UpdateAI(const uint32 diff) override;

        // Called when an AI Event is received
        void ReceiveAIEvent(AIEventType /*eventType*/, Unit* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/) override {}

        // == State checks =================================

        // Check if unit is visible for MoveInLineOfSight
        // bool IsVisible(Unit* who) const override;

        // Called when victim entered water and creature can not enter water
        // bool canReachByRangeAttack(Unit* who) override { return UnitAI::canReachByRangeAttack(pWho); }

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
        // virtual void Reset() = 0;

        /// Called at creature EnterCombat with an enemy
        /**
         * This is a SD2 internal function
         * Called by default on creature EnterCombat with an enemy
         */
        virtual void Aggro(Unit* /*who*/) {}

        // *************
        // AI Helper Functions
        // *************

        // Start movement toward victim
        void DoStartMovement(Unit* victim);

        // Start no movement on victim
        void DoStartNoMovement(Unit* victim);

        // Stop attack of current victim
        void DoStopAttack();

        // Plays a sound to all nearby players
        void DoPlaySoundToSet(WorldObject* source, uint32 soundId);

        // Teleports a player without dropping threat (only teleports to same map)
        void DoTeleportPlayer(Unit* unit, float x, float y, float z, float ori);

        // Returns a list of all friendly units missing a specific buff within range
        CreatureList DoFindFriendlyMissingBuff(float range, uint32 spellId, bool inCombat);

        // Return a player with at least minimumRange from m_creature
        Player* GetPlayerAtMinimumRange(float minimumRange) const;

        // Returns spells that meet the specified criteria from the creatures spell list
        SpellEntry const* SelectSpell(Unit* target, int32 school, int32 mechanic, SelectTarget selectTargets, uint32 powerCostMin, uint32 powerCostMax, float rangeMin, float rangeMax, SelectEffect selectEffects);

        // Checks if you can cast the specified spell
        bool CanCast(Unit* target, SpellEntry const* spellInfo, bool triggered = false);

        void SetEquipmentSlots(bool loadDefault, int32 mainHand = EQUIP_NO_CHANGE, int32 offHand = EQUIP_NO_CHANGE, int32 ranged = EQUIP_NO_CHANGE);

    protected:
        std::string GetAIName() override { return m_creature->GetAIName(); }

    private:
        uint32 m_uiEvadeCheckCooldown;

};

struct Scripted_NoMovementAI : public ScriptedAI
{
    Scripted_NoMovementAI(Creature* creature) : ScriptedAI(creature)
    {
        SetCombatMovement(false);
    }

    void GetAIInformation(ChatHandler& reader) override;
};

#endif
