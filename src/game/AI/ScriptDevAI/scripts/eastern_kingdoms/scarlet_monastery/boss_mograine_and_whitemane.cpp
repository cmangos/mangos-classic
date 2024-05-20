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
SDName: Boss_Mograine_And_Whitemane
SD%Complete: 95
SDComment:
SDCategory: Scarlet Monastery
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "Globals/SharedDefines.h"
#include "scarlet_monastery.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    // Mograine says
    SAY_MO_AGGRO                 = -1189005,
    SAY_MO_KILL                  = -1189006,
    SAY_MO_RESSURECTED           = -1189007,

    // Whitemane says
    SAY_WH_INTRO                 = -1189008,
    SAY_WH_KILL                  = -1189009,
    SAY_WH_RESSURECT             = -1189010,

    // Mograine Spells
    SPELL_CRUSADERSTRIKE         = 14518,
    SPELL_HAMMEROFJUSTICE        = 5589,
    SPELL_LAYONHANDS             = 9257,
    SPELL_RETRIBUTIONAURA        = 8990,
    SPELL_DIVINESHIELD           = 642,

    // Whitemanes Spells
    SPELL_DEEPSLEEP              = 9256,
    SPELL_SCARLETRESURRECTION    = 9232,
    SPELL_DOMINATEMIND           = 14515,
    SPELL_HOLYSMITE              = 9481,
    SPELL_HEAL                   = 12039,
    SPELL_POWERWORDSHIELD        = 22187,

    NPC_SCARLET_ABBOT               = 4303,
    NPC_SCARLET_MONK                = 4540,
    NPC_SCARLET_CHAMPION            = 4302,
    NPC_SCARLET_WIZARD              = 4300,
    NPC_SCARLET_CENTURION           = 4301,
    NPC_SCARLET_CHAPLAIN            = 4299,

    ASHBRINGER_RELAY_SCRIPT_ID   = 9001,
    SPELL_AB_EFFECT_000          = 28441,

    SOUND_MOGRAINE_FAKE_DEATH    = 1326,
};

enum MograineActions
{
    MOGRAINE_ACTION_CRUSADER_STRIKE,
    MOGRAINE_ACTION_HAMMER_OF_JUSTICE,
    MOGRAINE_ACTION_DIVINE_SHIELD,
    MOGRAINE_ACTION_MAX,
    MOGRAINE_ACTION_LAY_ON_HANDS,
    MOGRAINE_ACTION_REVIVED,
    MOGRAINE_ACTION_ROAR,
};

struct boss_scarlet_commander_mograineAI : public CombatAI
{
    boss_scarlet_commander_mograineAI(Creature* creature) : CombatAI(creature, MOGRAINE_ACTION_MAX), m_instance(static_cast<instance_scarlet_monastery*>(creature->GetInstanceData()))
    {
        AddCombatAction(MOGRAINE_ACTION_CRUSADER_STRIKE, 8400u);
        AddCombatAction(MOGRAINE_ACTION_HAMMER_OF_JUSTICE, 9600u);
        AddCombatAction(MOGRAINE_ACTION_DIVINE_SHIELD, 40000u);
        AddCustomAction(MOGRAINE_ACTION_LAY_ON_HANDS, false, [&]()
        {
            HandleLayOnHandsTimer(); 
        }, TIMER_COMBAT_COMBAT);
        AddCustomAction(MOGRAINE_ACTION_REVIVED, true, [&]()
        {
            HandleRevivedTimer();
        });
        AddCustomAction(MOGRAINE_ACTION_ROAR, true, [&]()
        {
            HandleRoar();
        });
    }

    instance_scarlet_monastery* m_instance;

    bool m_bHasDied;
    bool m_bHeal;

    void Reset() override
    {
        CombatAI::Reset();
        m_bHasDied = false;
        m_bHeal = false;

        // Incase wipe during phase that mograine fake death
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
        m_creature->SetStandState(UNIT_STAND_STATE_STAND);
        SetDeathPrevention(true);
        SetCombatScriptStatus(false);
    }

    uint32 GetSubsequentActionTimer(uint32 id)
    {
        switch (id)
        {
            case MOGRAINE_ACTION_CRUSADER_STRIKE: return urand(6000, 15000);
            case MOGRAINE_ACTION_HAMMER_OF_JUSTICE: return urand(7000, 18500);
            default: return 0; // never occurs but for compiler
        }
    }

    void EnterEvadeMode() override
    {
        if (m_instance)
        {
            if (m_instance->GetData(TYPE_MOGRAINE_AND_WHITE_EVENT) == NOT_STARTED)
            {
                CombatAI::EnterEvadeMode();
                return;
            }
            if (m_instance->GetData(TYPE_MOGRAINE_AND_WHITE_EVENT) != NOT_STARTED && m_instance->GetData(TYPE_MOGRAINE_AND_WHITE_EVENT) != FAIL)
                m_instance->SetData(TYPE_MOGRAINE_AND_WHITE_EVENT, FAIL);
        }
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(SAY_MO_AGGRO, m_creature);
        DoCastSpellIfCan(m_creature, SPELL_RETRIBUTIONAURA);

        MograineCallForHelp();
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        DoScriptText(SAY_MO_KILL, m_creature);
    }

    void JustPreventedDeath(Unit* /*attacker*/) override
    {
        if (!m_instance)
            return;

        if (Creature* pWhitemane = m_instance->GetSingleCreatureFromStorage(NPC_WHITEMANE))
        {
            m_instance->SetData(TYPE_MOGRAINE_AND_WHITE_EVENT, IN_PROGRESS);

            pWhitemane->GetMotionMaster()->MovePoint(1, 1163.113370f, 1398.856812f, 32.527786f, FORCED_MOVEMENT_RUN);
            pWhitemane->AI()->SetCombatScriptStatus(true);

            DoScriptText(SAY_WH_INTRO, pWhitemane);

            m_creature->GetMotionMaster()->MovementExpired();
            m_creature->GetMotionMaster()->MoveIdle();

            if (m_creature->IsNonMeleeSpellCasted(false))
                m_creature->InterruptNonMeleeSpells(false);

            m_creature->ClearComboPointHolders();
            m_creature->RemoveAllAurasOnDeath();
            m_creature->ClearAllReactives();
            m_creature->AttackStop(true, true, true);

            SetCombatScriptStatus(true);
            m_creature->SetTarget(nullptr);

            m_creature->PlayDistanceSound(SOUND_MOGRAINE_FAKE_DEATH);

            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
            m_creature->SetStandState(UNIT_STAND_STATE_DEAD);

            m_bHasDied = true;
        }
    }

    void SpellHit(Unit* pWho, const SpellEntry* pSpell) override
    {
        // When hit with ressurection say text
        if (pSpell->Id == SPELL_SCARLETRESURRECTION)
        {
            if (m_instance)
                m_instance->SetData(TYPE_MOGRAINE_AND_WHITE_EVENT, SPECIAL);
            ResetTimer(MOGRAINE_ACTION_LAY_ON_HANDS, 3000u);
        }
        // casted by player every 5 seconds while wielding Corrupted Ashbringer
        else if (pSpell->Id == SPELL_AB_EFFECT_000)
        {
            if (m_instance->GetData(TYPE_ASHBRINGER_EVENT) == IN_PROGRESS)
            {
                pWho->GetMap()->ScriptsStart(SCRIPT_TYPE_RELAY, ASHBRINGER_RELAY_SCRIPT_ID, m_creature, pWho);
                m_instance->SetData(TYPE_ASHBRINGER_EVENT, DONE);
            }
        }
    }

    void HandleLayOnHandsTimer()
    {
        if (m_bHasDied && !m_bHeal && m_instance && m_instance->GetData(TYPE_MOGRAINE_AND_WHITE_EVENT) == SPECIAL)
        {
            if (Creature* pWhitemane = m_instance->GetSingleCreatureFromStorage(NPC_WHITEMANE))
            {
                // On resurrection, stop fake death and heal whitemane and resume fight
                m_creature->SetStandState(UNIT_STAND_STATE_STAND);
                DoScriptText(SAY_MO_RESSURECTED, m_creature);
                SetDeathPrevention(false);

                // spell has script target on Whitemane but sometimes wouldn't work
                m_creature->CastSpell(nullptr, SPELL_LAYONHANDS, TRIGGERED_OLD_TRIGGERED);

                m_creature->GetMotionMaster()->Clear();
                ResetTimer(MOGRAINE_ACTION_ROAR, 500u);

                ResetTimer(MOGRAINE_ACTION_REVIVED, 2000u);

                m_bHeal = true;
            }
        }
    }

    void MograineCallForHelp()
    {
        std::vector<uint32> entries = {NPC_SCARLET_ABBOT, NPC_SCARLET_CENTURION, NPC_SCARLET_CHAMPION, NPC_SCARLET_CHAPLAIN, NPC_SCARLET_MONK, NPC_SCARLET_WIZARD};
        for (uint32 entry : entries)
        {
            CreatureList scarletTrash;
            GetCreatureListWithEntryInGrid(scarletTrash, m_creature, entry, 80.f);
            for (Creature* creature : scarletTrash)
            {
                if (creature && creature->IsAlive())
                {
                    creature->SetInCombatWithZone();
                    creature->AI()->AttackClosestEnemy();
                }
            }
        }
    }

    void HandleRoar()
    {
        m_creature->HandleEmote(EMOTE_ONESHOT_ROAR);
    }

    void HandleRevivedTimer()
    {
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);

        SetActionReadyStatus(MOGRAINE_ACTION_DIVINE_SHIELD, true);

        ResetCombatAction(MOGRAINE_ACTION_CRUSADER_STRIKE, GetSubsequentActionTimer(MOGRAINE_ACTION_CRUSADER_STRIKE));
        ResetCombatAction(MOGRAINE_ACTION_HAMMER_OF_JUSTICE, GetSubsequentActionTimer(MOGRAINE_ACTION_HAMMER_OF_JUSTICE));

        DoCastSpellIfCan(m_creature, SPELL_RETRIBUTIONAURA);

        SetCombatScriptStatus(false);

        if (m_creature->GetVictim())
            m_creature->GetMotionMaster()->MoveChase(m_creature->GetVictim());
    }

    void ExecuteAction(uint32 action) override
    {
        if (!m_instance)
            return;

        switch (action)
        {
            case MOGRAINE_ACTION_CRUSADER_STRIKE:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_CRUSADERSTRIKE) == CAST_OK)
                    ResetCombatAction(action, GetSubsequentActionTimer(action));
                return;
            }
            case MOGRAINE_ACTION_HAMMER_OF_JUSTICE:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_HAMMEROFJUSTICE) == CAST_OK)
                    ResetCombatAction(action, GetSubsequentActionTimer(action));
                return;
            }
            case MOGRAINE_ACTION_DIVINE_SHIELD:
            {
                if (m_creature->GetHealthPercent() <= 50.f)
                    if (DoCastSpellIfCan(nullptr, SPELL_DIVINESHIELD) == CAST_OK)
                        DisableCombatAction(action);
                return;
            }
        }
    }
};

enum WhitemaneActions
{
    WHITEMANE_ACTION_DEEP_SLEEP,
    WHITEMANE_ACTION_SCARLET_RESURRECTION,
    WHITEMANE_ACTION_DOMINATE_MIND,
    WHITEMANE_ACTION_HOLY_SMITE,
    WHITEMANE_ACTION_HEAL,
    WHITEMANE_ACTION_POWERWORD_SHIELD,
    WHITEMANE_ACTION_MAX,
    WHITEMANE_ACTION_SALUTE,
    WHITEMANE_ACTION_SCARLET_RESURRECTION_ENTER_COMBAT,
};

struct boss_high_inquisitor_whitemaneAI : public CombatAI
{
    boss_high_inquisitor_whitemaneAI(Creature* creature) : CombatAI(creature, WHITEMANE_ACTION_MAX), m_instance(static_cast<instance_scarlet_monastery*>(creature->GetInstanceData()))
    {
        AddTimerlessCombatAction(WHITEMANE_ACTION_DEEP_SLEEP, true);
        AddCombatAction(WHITEMANE_ACTION_HEAL, true);
        AddCombatAction(WHITEMANE_ACTION_POWERWORD_SHIELD, true);
        AddCombatAction(WHITEMANE_ACTION_HOLY_SMITE, true);
        AddCustomAction(WHITEMANE_ACTION_SCARLET_RESURRECTION, true, [&]()
        {
            HandleResurrection();
        });
        AddCustomAction(WHITEMANE_ACTION_SCARLET_RESURRECTION_ENTER_COMBAT, true, [&]()
        {
            HandleResurrectionCombat();
        });
        AddCustomAction(WHITEMANE_ACTION_SALUTE, true, [&]()
        {
            HandleSalute();
        });
        AddCombatAction(WHITEMANE_ACTION_DOMINATE_MIND, true);
        SetMeleeEnabled(false);
        SetReactState(REACT_PASSIVE);
        AddMainSpell(SPELL_HOLYSMITE);
        SetRangedMode(true, 25.f, TYPE_PROXIMITY);
    }

    uint32 GetSubsequentActionTimer(uint32 id)
    {
        switch (id)
        {
            case WHITEMANE_ACTION_HEAL: return 13000u;
            case WHITEMANE_ACTION_POWERWORD_SHIELD: return urand(22000, 45000);
            case WHITEMANE_ACTION_HOLY_SMITE: return GetCurrentRangedMode() ? urand(2000, 3000) : urand(3500, 8000);
            case WHITEMANE_ACTION_DOMINATE_MIND: return urand(5000, 10000);
            default: return 0; // never occurs but for compiler
        }
    }
    instance_scarlet_monastery* m_instance;

    void JustRespawned() override
    {
        CombatAI::JustRespawned();
        SetDeathPrevention(true);
    }

    void EnterEvadeMode() override
    {
        SetCombatScriptStatus(false);
        SetCombatMovement(true);
        SetMeleeEnabled(true);
        if (m_instance)
        {
            if (m_instance->GetData(TYPE_MOGRAINE_AND_WHITE_EVENT) == NOT_STARTED)
                CombatAI::EnterEvadeMode();
            if (m_instance->GetData(TYPE_MOGRAINE_AND_WHITE_EVENT) != NOT_STARTED && m_instance->GetData(TYPE_MOGRAINE_AND_WHITE_EVENT) != FAIL)
                m_instance->SetData(TYPE_MOGRAINE_AND_WHITE_EVENT, FAIL);
        }
    }

    void EnterCombat(Unit* /*enemy*/) override
    {
        if (m_instance && m_instance->GetData(TYPE_MOGRAINE_AND_WHITE_EVENT) == IN_PROGRESS)
        {
            SetCombatScriptStatus(false);
            ResetCombatAction(WHITEMANE_ACTION_HEAL, 10000u);
            ResetCombatAction(WHITEMANE_ACTION_POWERWORD_SHIELD, 15000u);
            ResetCombatAction(WHITEMANE_ACTION_HOLY_SMITE, 100u);
            SetMeleeEnabled(true);
            SetReactState(REACT_AGGRESSIVE);
        }
    }

    void MovementInform(uint32 uiMotionType, uint32 uiPointId) override
    {
        if (uiMotionType == POINT_MOTION_TYPE && uiPointId == 2)
        {
            if (m_instance)
            {
                if (Creature* pMograine = m_instance->GetSingleCreatureFromStorage(NPC_MOGRAINE))
                {
                    m_creature->SetFacingToObject(pMograine);
                    ResetTimer(WHITEMANE_ACTION_SCARLET_RESURRECTION, 3000u);
                }
            }
        }
        if (uiMotionType == POINT_MOTION_TYPE && uiPointId == 1)
        {
            m_creature->SetInCombatWithZone();
            AttackClosestEnemy();
        }
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        DoScriptText(SAY_WH_KILL, m_creature);
    }

    void HandleResurrection()
    {
        // spell has script target on Mograine
        DoCastSpellIfCan(nullptr, SPELL_SCARLETRESURRECTION);
        DoScriptText(SAY_WH_RESSURECT, m_creature);
        ResetTimer(WHITEMANE_ACTION_SALUTE, 3400u);
        m_creature->GetMotionMaster()->Clear();
        ResetTimer(WHITEMANE_ACTION_SCARLET_RESURRECTION_ENTER_COMBAT, 5700u);
    }

    void HandleResurrectionCombat()
    {
        ResetCombatAction(WHITEMANE_ACTION_HEAL, GetSubsequentActionTimer(WHITEMANE_ACTION_HEAL));
        ResetCombatAction(WHITEMANE_ACTION_POWERWORD_SHIELD, GetSubsequentActionTimer(WHITEMANE_ACTION_POWERWORD_SHIELD));
        ResetCombatAction(WHITEMANE_ACTION_HOLY_SMITE, GetSubsequentActionTimer(WHITEMANE_ACTION_HOLY_SMITE));
        ResetCombatAction(WHITEMANE_ACTION_DOMINATE_MIND, GetSubsequentActionTimer(WHITEMANE_ACTION_DOMINATE_MIND));
        SetCombatScriptStatus(false);
        SetCombatMovement(true, true);
        SetMeleeEnabled(true);
        SetDeathPrevention(false);
    }

    void HandleSalute()
    {
        m_creature->HandleEmote(EMOTE_ONESHOT_SALUTE);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case WHITEMANE_ACTION_HEAL:
            {
                if (Unit* target = DoSelectLowestHpFriendly(50.0f))
                {
                    if (DoCastSpellIfCan(target, SPELL_HEAL) == CAST_OK)
                        ResetCombatAction(action, GetSubsequentActionTimer(action));
                }
                return;
            }
            case WHITEMANE_ACTION_HOLY_SMITE:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_HOLYSMITE) == CAST_OK)
                    ResetCombatAction(action, GetSubsequentActionTimer(action));
                return;
            }
            case WHITEMANE_ACTION_POWERWORD_SHIELD:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_POWERWORDSHIELD) == CAST_OK)
                    ResetCombatAction(action, GetSubsequentActionTimer(action));
                return;
            }
            case WHITEMANE_ACTION_DEEP_SLEEP:
            {
                if (m_creature->GetHealthPercent() <= 50.0f)
                {
                    DoCastSpellIfCan(nullptr, SPELL_DEEPSLEEP, CAST_INTERRUPT_PREVIOUS);
                    SetCombatMovement(false);
                    SetMeleeEnabled(false);
                    SetCombatScriptStatus(true);
                    m_creature->SetTarget(nullptr);
                    if (Creature* mograine = m_instance->GetSingleCreatureFromStorage(NPC_MOGRAINE))
                    {
                        SetCombatMovement(false);
                        m_creature->AttackStop(true);
                        float fX, fY, fZ;
                        mograine->GetContactPoint(m_creature, fX, fY, fZ, INTERACTION_DISTANCE / 2.f);
                        m_creature->GetMotionMaster()->Clear();
                        m_creature->GetMotionMaster()->MovePoint(2, fX, fY, fZ, FORCED_MOVEMENT_RUN);
                    }
                    DisableCombatAction(action);
                }
                return;
            }
            case WHITEMANE_ACTION_DOMINATE_MIND:
            {
                if (!urand(0, 50))
                    if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_DOMINATEMIND, SELECT_FLAG_PLAYER))
                        if (DoCastSpellIfCan(target, SPELL_DOMINATEMIND))
                            ResetCombatAction(action, urand(20000, 30000));
                return;
            }
        }
    }
};

void AddSC_boss_mograine_and_whitemane()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_scarlet_commander_mograine";
    pNewScript->GetAI = &GetNewAIInstance<boss_scarlet_commander_mograineAI>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_high_inquisitor_whitemane";
    pNewScript->GetAI = &GetNewAIInstance<boss_high_inquisitor_whitemaneAI>;
    pNewScript->RegisterSelf();
}
