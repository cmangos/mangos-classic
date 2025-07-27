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
SDName: Boss_Anubrekhan
SD%Complete: 99
SDComment: Corpse Scarabs summon from dead Crypt Guard may need improvement: timer and spell logic
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/base/BossAI.h"
#include "naxxramas.h"

enum
{
    SAY_GREET1                  = 13004,
    SAY_GREET2                  = 13006,
    SAY_GREET3                  = 13007,
    SAY_GREET4                  = 13008,
    SAY_GREET5                  = 13009,
    SAY_AGGRO1                  = 13000,
    SAY_AGGRO2                  = 13002,
    SAY_AGGRO3                  = 13003,
    SAY_SLAY                    = 13005,

    SPELL_DOUBLE_ATTACK         = 18943,
    SPELL_IMPALE                = 28783,
    SPELL_LOCUSTSWARM           = 28785,                    // This is a self buff that triggers the dmg debuff
    SPELL_ANUB_AURA             = 29103,                    // Periodically apply aura 29104 onto players to handle the corpse scarabs summon when they die (spell 29105)

    SPELL_SUMMON_GUARD          = 29508,                    // Summons 1 Crypt Guard at targeted location
    SPELL_DESPAWN_GUARDS        = 29379,                    // Remove all Crypt Guards and Corpse Scarabs
    SPELL_SPAWN_CORPSE_SCARABS  = 28961,                    // Trigger 28864 Summon 10 Corpse Scarabs from dead Crypt Guard
//  SPELL_DELAY_SUMMON_SCARABS1 = 28992,                    // Probably used to prevent two dead Crypt Guards to explode in a short time
//  SPELL_DELAY_SUMMON_SCARABS2 = 28994,                    // Probably used to prevent two dead Crypt Guards to explode in a short time by triggering same spell cooldown than 28961
};

static const DialogueEntry introDialogue[] =
{
    {SAY_GREET1,  NPC_ANUB_REKHAN,  7000},
    {SAY_GREET2,  NPC_ANUB_REKHAN,  13000},
    {SAY_GREET3,  NPC_ANUB_REKHAN,  11000},
    {SAY_GREET4,  NPC_ANUB_REKHAN,  10000},
    {SAY_GREET5,  NPC_ANUB_REKHAN,  0},
};

enum AnubRekhanActions
{
    ANUBREKHAN_ACTIONS_MAX,
    ANUBREKHAN_SUMMON,
};

struct boss_anubrekhanAI : public BossAI
{
    boss_anubrekhanAI(Creature* creature) : BossAI(creature, ANUBREKHAN_ACTIONS_MAX),
        m_introDialogue(introDialogue), m_instance(static_cast<instance_naxxramas*>(creature->GetInstanceData())), m_hasDoneIntro(false)
    {
        SetDataType(TYPE_ANUB_REKHAN);
        AddOnAggroText(SAY_AGGRO1, SAY_AGGRO2, SAY_AGGRO3);
        AddOnKillText(SAY_SLAY);
        m_introDialogue.InitializeDialogueHelper(m_instance);
        AddCustomAction(ANUBREKHAN_SUMMON, true, [&]()
        {
            DoCastSpellIfCan(nullptr, SPELL_SUMMON_GUARD);
        });
    }

    instance_naxxramas* m_instance;
    DialogueHelper m_introDialogue;

    bool m_hasDoneIntro;

    void Reset() override
    {
        BossAI::Reset();

        DoCastSpellIfCan(nullptr, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void Aggro(Unit* /*who*/) override
    {
        BossAI::Aggro();
        DoCastSpellIfCan(m_creature, SPELL_ANUB_AURA, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void StartIntro()
    {
        if (!m_hasDoneIntro)
        {
            m_introDialogue.StartNextDialogueText(SAY_GREET1);
            m_hasDoneIntro = true;
        }
    }

    void JustSummoned(Creature* summoned) override
    {
        summoned->SetInCombatWithZone();
    }

    void EnterEvadeMode() override
    {
        // We despawn the guardians before entering evade mode to prevent despawning also the static adds that are linked to respawn on evade
        DoCastSpellIfCan(nullptr, SPELL_DESPAWN_GUARDS, CAST_TRIGGERED);

        BossAI::EnterEvadeMode();
    }

    void OnSpellCast(SpellEntry const* spellInfo, Unit* /*target*/) override
    {
        if (spellInfo->Id == SPELL_LOCUSTSWARM)
            ResetTimer(ANUBREKHAN_SUMMON, 3000);
    }

    void UpdateAI(const uint32 diff) override
    {
        m_introDialogue.DialogueUpdate(diff);
        BossAI::UpdateAI(diff);
    }
};

bool GOUse_go_anub_door(Player* /*player*/, GameObject* go)
{
    if (ScriptedInstance* instance = static_cast<ScriptedInstance*>(go->GetInstanceData()))
    {
        if (instance->GetData(TYPE_ANUB_REKHAN) == IN_PROGRESS || instance->GetData(TYPE_ANUB_REKHAN) == DONE) // GOs always open once used (or handled by script), so this check is only there for safety
            return false;
        else
        {
            if (Creature* anub = instance->GetSingleCreatureFromStorage(NPC_ANUB_REKHAN))
            {
                if (boss_anubrekhanAI* anubAI = dynamic_cast<boss_anubrekhanAI*>(anub->AI()))
                    anubAI->StartIntro();
            }
        }
    }
    return false;
}

// 28961 - Summon Corpse Scarabs
struct SummonCorpseScarabs : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        Unit* unitTarget = spell->GetUnitTarget();
        if (unitTarget->IsAlive())
            return;

        unitTarget->CastSpell(nullptr, 28864, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, spell->GetTrueCaster()->GetObjectGuid());  // Actual summoning spell
        if (unitTarget->IsCreature())
            static_cast<Creature*>(unitTarget)->ForcedDespawn(2000);
    }
};

// 29379 - Despawn Crypt Guards
struct DespawnCryptGuards : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        Unit* target = spell->GetUnitTarget();
        if (target && target->IsCreature())
            static_cast<Creature*>(target)->ForcedDespawn();
    }
};

void AddSC_boss_anubrekhan()
{
    Script* newScript = new Script;
    newScript->Name = "boss_anubrekhan";
    newScript->GetAI = &GetNewAIInstance<boss_anubrekhanAI>;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "go_anub_door";
    newScript->pGOUse = &GOUse_go_anub_door;
    newScript->RegisterSelf();

    RegisterSpellScript<SummonCorpseScarabs>("spell_summon_corpse_scarabs");
    RegisterSpellScript<DespawnCryptGuards>("spell_despawn_crypt_guards");
}
