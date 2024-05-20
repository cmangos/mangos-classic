/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
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

/* ScriptData
SDName: Boss_Skeram
SD%Complete: 100
SDComment:
SDCategory: Temple of Ahn'Qiraj
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "temple_of_ahnqiraj.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    SAY_AGGRO                   = -1531000,
    SAY_SLAY                    = 11446,
    SAY_DEATH                   = -1531002,

    SPELL_ARCANE_EXPLOSION      = 26192,
    SPELL_EARTH_SHOCK           = 26194,
    SPELL_TRUE_FULFILLMENT      = 785, // wotlk+ also casts 61286 on the target on cast end and also on aura end 65634
    SPELL_TRUE_FULFILLMENT_2    = 2313, // additional buff
    SPELL_TELEPORT_1            = 4801,
    SPELL_TELEPORT_2            = 8195,
    SPELL_TELEPORT_3            = 20449,
    SPELL_INITIALIZE_IMAGES     = 794,
    SPELL_INITIALIZE_IMAGE      = 3730,   // Triggers by spell 794 on each summoned image
    SPELL_SUMMON_IMAGES         = 747,
    SPELL_TELEPORT_IMAGE        = 26591,

    SPELL_BIRTH                 = 26262, // Different in tbc+ - unconfirmed but has same script
};

enum SkeramActions
{
    SKERAM_SPLIT,
    SKERAM_ARCANE_EXPLOSION,
    SKERAM_TRUE_FULFILMENT,
    SKERAM_BLINK,
    SKERAM_EARTH_SHOCK,
    SKERAM_ACTION_MAX,
    SKERAM_BLINK_DELAY,
    SKERAM_INIT_IMAGES,
};

struct boss_skeramAI : public CombatAI
{
    boss_skeramAI(Creature* creature) : CombatAI(creature, SKERAM_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData())), m_isImage(!m_creature->GetSpawnerGuid().IsEmpty())
    {
        if (!m_isImage)
        {
            AddTimerlessCombatAction(SKERAM_SPLIT, true);
            AddCombatAction(SKERAM_BLINK, 30000, 45000);
            AddCustomAction(SKERAM_INIT_IMAGES, true, [&]() { HandleInitImages(); });
        }
        AddCombatAction(SKERAM_ARCANE_EXPLOSION, 6000, 12000);
        AddCombatAction(SKERAM_TRUE_FULFILMENT, uint32(15) * IN_MILLISECONDS);
        AddCombatAction(SKERAM_EARTH_SHOCK, 1200u);
        AddCustomAction(SKERAM_BLINK_DELAY, true, [&]() { HandleBlinkDelay(); }, TIMER_COMBAT_COMBAT);
        AddOnKillText(SAY_SLAY);
    }

    ScriptedInstance* m_instance;

    std::vector<uint32> m_teleports;
    uint8 m_teleportCounter;

    uint8 m_maxMeleeAllowed;

    float m_hpCheck;

    bool m_isImage;

    int32 m_rangeCheckState;

    GuidVector m_images;

    void Reset() override
    {
        CombatAI::Reset();
        m_hpCheck               = 75.0f;

        m_teleports              = { SPELL_TELEPORT_1, SPELL_TELEPORT_2, SPELL_TELEPORT_3 };
        m_teleportCounter        = 0;

        m_maxMeleeAllowed        = 0;
    }

    void JustRespawned() override
    {
        CombatAI::JustRespawned();
        if (m_isImage)
            StopAttacking();
        DoCastSpellIfCan(nullptr, SPELL_BIRTH);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (!m_isImage)
        {
            DoScriptText(SAY_DEATH, m_creature);

            if (m_instance)
                m_instance->SetData(TYPE_SKERAM, DONE);

            DespawnGuids(m_images);
        }
        // Else despawn to avoid looting
        else
            m_creature->ForcedDespawn(1);
    }

    void Aggro(Unit* /*who*/) override
    {
        // Prophet Skeram will only cast Arcane Explosion if a given number of players are in melee range
        // Initial value was 4+ but it was changed in patch 1.12 to be less dependant on raid
        // We assume value is number of players / 10 (raid of 40 people in Classic -> value of 4)
        m_maxMeleeAllowed = m_instance->instance->GetPlayers().getSize() / 10 + 1;

        if (m_isImage)
            return;

        DoScriptText(SAY_AGGRO, m_creature);

        if (m_instance)
            m_instance->SetData(TYPE_SKERAM, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_SKERAM, FAIL);

        if (m_isImage)
            m_creature->ForcedDespawn();
    }

    void JustSummoned(Creature* summoned) override
    {
        if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            summoned->AI()->AttackStart(target);

        m_images.push_back(summoned->GetObjectGuid());
    }

    void StopAttacking()
    {
        SetCombatScriptStatus(true);
        SetMeleeEnabled(false);
        SetCombatMovement(false);
        m_creature->SetTarget(nullptr);
    }

    // Wrapper to handle the image teleport
    void DoTeleport(bool attacking)
    {
        if (!m_instance)
            return;

        uint32 teleportSpellID = 0;
        if (m_isImage)
        {
            // Get an available teleport location from original boss
            if (Creature* prophet = m_instance->GetSingleCreatureFromStorage(NPC_SKERAM))
            {
                if (boss_skeramAI* skeramAI = dynamic_cast<boss_skeramAI*>(prophet->AI()))
                    teleportSpellID = skeramAI->GetAvailableTeleport();
            }
        }
        else
            teleportSpellID = m_teleports[0];

        // Teleport, reset thread (and restore visibility if needed)
        DoCastSpellIfCan(nullptr, teleportSpellID);
        DoResetThreat();
        // ClearMark();     // Starting with patch 2.0.1 raid marks are cleared on teleport
        m_creature->RemoveAllAuras();

        if (attacking)
            StopAttacking();

        ResetTimer(SKERAM_BLINK_DELAY, 2000);
    }

    void HandleBlinkDelay()
    {
        SetCombatScriptStatus(false);
        SetMeleeEnabled(true);
        SetCombatMovement(true);
    }

    uint32 GetAvailableTeleport()
    {
        // Only original boss can give teleport location
        if (m_isImage)
            return 0;

        if (m_teleportCounter < m_teleports.size())
            return m_teleports[m_teleportCounter++];

        return 0;
    }

/*    void ClearMark()
    {
        if (m_instance)
        {
            if (Player* player = m_instance->GetPlayerInMap(true, false)) // we assume only one group in raid
            {
                if (Group* group = player->GetGroup())
                {
                    group->SetTargetIcon(7, m_creature->GetObjectGuid()); // set to none
                }
            }
        }
    }
*/
    void HandleInitImages()
    {
        std::shuffle(m_teleports.begin(), m_teleports.end(), *GetRandomGenerator()); // Shuffle the teleport spells to ensure that boss and images have a different location assigned randomly
        m_teleportCounter = 1;
        DoCastSpellIfCan(nullptr, SPELL_INITIALIZE_IMAGES, CAST_TRIGGERED);
        DoTeleport(false);
        ResetCombatAction(SKERAM_BLINK, urand(30000, 40000));
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case SKERAM_SPLIT:
            {
                // Summon images at 75%, 50% and 25%
                if (m_creature->GetHealthPercent() < m_hpCheck)
                {
                    DespawnGuids(m_images);
                    if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_IMAGES) == CAST_OK)
                    {
                        m_hpCheck -= 25.0f;
                        StopAttacking();
                        // Teleport shortly after the images are summoned and set invisible to clear the selection (Workaround alert!!!)
                        ResetTimer(SKERAM_INIT_IMAGES, 1500);
                    }
                }
                break;
            }
            case SKERAM_ARCANE_EXPLOSION:
            {
                // Arcane Explosion is done if more than a set number of people are in melee range
                PlayerList meleePlayerList;
                float meleeRange = m_creature->GetCombinedCombatReach(m_creature->GetVictim(), true);
                GetPlayerListWithEntryInWorld(meleePlayerList, m_creature, meleeRange);
                if (meleePlayerList.size() >= m_maxMeleeAllowed)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_ARCANE_EXPLOSION) == CAST_OK)
                        ResetCombatAction(action, urand(8, 18) * IN_MILLISECONDS);
                }
                else // Recheck in 1 second
                    ResetCombatAction(action, 1 * IN_MILLISECONDS);
                break;
            }
            case SKERAM_TRUE_FULFILMENT:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_NEAREST_BY, 1, SPELL_TRUE_FULFILLMENT, SELECT_FLAG_PLAYER))
                    if (DoCastSpellIfCan(target, SPELL_TRUE_FULFILLMENT) == CAST_OK)
                        ResetCombatAction(action, urand(20, 30) * IN_MILLISECONDS);
                break;
            }
            case SKERAM_BLINK:
            {
                std::shuffle(m_teleports.begin(), m_teleports.end(), *GetRandomGenerator()); // Shuffle the teleport spells to ensure that boss and images have a different location assigned randomly
                m_teleportCounter = 1;
                DoTeleport(true);
                for (auto guid : m_images)
                    if (Creature* image = m_creature->GetMap()->GetCreature(guid))
                        static_cast<boss_skeramAI*>(image->AI())->DoTeleport(true);
                ResetCombatAction(action, urand(10, 30) * IN_MILLISECONDS);
                break;
            }
            case SKERAM_EARTH_SHOCK:
            {
                uint32 timer = 500;
                // If victim exists we have a target in melee range
                if (m_creature->GetVictim() && m_creature->CanReachWithMeleeAttack(m_creature->GetVictim()))
                    m_rangeCheckState = -1;
                // Spam Waterbolt spell when not tanked
                else
                {
                    ++m_rangeCheckState;
                    if (m_rangeCheckState > 1)
                        if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_EARTH_SHOCK) == CAST_OK)
                            timer = 2500;
                }
                ResetCombatAction(action, timer);
                break;
            }
        }
    }
};

struct TrueFulfillment : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const override
    {
        if (apply)
            aura->GetTarget()->CastSpell(aura->GetTarget(), SPELL_TRUE_FULFILLMENT_2, TRIGGERED_OLD_TRIGGERED);
        // wotlk add stuff on break
    }
};

struct InitializeImages : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (Unit* unitTarget = spell->GetUnitTarget())
        {
            // Cast Initialize Image for each image target with orignal boss HP percent as basepoints
            int32 healthPercent = int32(spell->GetCaster()->GetHealthPercent());
            unitTarget->CastCustomSpell(nullptr, SPELL_INITIALIZE_IMAGE, &healthPercent, nullptr, nullptr, TRIGGERED_OLD_TRIGGERED, nullptr);
            unitTarget->CastSpell(nullptr, SPELL_TELEPORT_IMAGE, TRIGGERED_OLD_TRIGGERED);   // Teleport Image
        }
    }
};

struct InitializeImage : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (Unit* unitTarget = spell->GetUnitTarget())
        {
            float healthPct = spell->m_currentBasePoints[EFFECT_INDEX_0];
            float maxHealthPct = 0.0f;

            // The max health depends on the split phase. It's percent * original boss health
            if (healthPct < 25.0f)
                maxHealthPct = 0.50f;
            else if (healthPct < 50.0f)
                maxHealthPct = 0.20f;
            else
                maxHealthPct = 0.10f;

            // Set the same health percent as the original boss
            spell->GetCaster()->SetMaxHealth(unitTarget->GetMaxHealth() * maxHealthPct);
            spell->GetCaster()->SetHealthPercent(healthPct);
        }
    }

    bool OnCheckTarget(const Spell* spell, Unit* target, SpellEffectIndex eff) const override
    {
        if (target->GetSpawnerGuid()) // only original spawn can be hit by this
            return false;

        return true;
    }
};

struct TeleportImage : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (Unit* target = spell->GetUnitTarget())
            if (boss_skeramAI* skeramAI = dynamic_cast<boss_skeramAI*>(target->AI()))
                skeramAI->DoTeleport(false);
    }
};

void AddSC_boss_skeram()
{
    Script* newScript = new Script;
    newScript->Name = "boss_skeram";
    newScript->GetAI = &GetNewAIInstance<boss_skeramAI>;
    newScript->RegisterSelf();

    RegisterSpellScript<TrueFulfillment>("spell_true_fulfillment");
    RegisterSpellScript<InitializeImages>("spell_initialize_images");
    RegisterSpellScript<TeleportImage>("spell_teleport_image");
    RegisterSpellScript<InitializeImage>("spell_initialize_image");
}
