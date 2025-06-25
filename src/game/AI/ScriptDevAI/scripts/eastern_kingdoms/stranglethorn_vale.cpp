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
SDName: Stranglethorn_Vale
SD%Complete: 100
SDComment: Quest support: 592
SDCategory: Stranglethorn Vale
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

/* ContentData
mob_yenniku
mob_colonel_kurzen
EndContentData */

/*######
## mob_yenniku
######*/

enum
{
    SPELL_YENNIKUS_RELEASE      = 3607,

    QUEST_ID_SAVING_YENNIKU     = 592,

    FACTION_ID_HORDE_GENERIC    = 83,                       // Note: faction may not be correct!
};

struct mob_yennikuAI : public ScriptedAI
{
    mob_yennikuAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiResetTimer;

    void Reset() override { m_uiResetTimer = 0; }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell) override
    {
        if (pSpell->Id == SPELL_YENNIKUS_RELEASE && pCaster->GetTypeId() == TYPEID_PLAYER)
        {
            if (!m_uiResetTimer && ((Player*)pCaster)->GetQuestStatus(QUEST_ID_SAVING_YENNIKU) == QUEST_STATUS_INCOMPLETE)
            {
                m_uiResetTimer = 60000;
                EnterEvadeMode();
            }
        }
    }

    void EnterEvadeMode() override
    {
        if (m_uiResetTimer)
        {
            m_creature->RemoveAllAurasOnEvade();
            m_creature->CombatStop(true);
            m_creature->LoadCreatureAddon(true);

            m_creature->SetLootRecipient(nullptr);

            m_creature->HandleEmote(EMOTE_STATE_STUN);
            m_creature->SetFactionTemporary(FACTION_ID_HORDE_GENERIC, TEMPFACTION_RESTORE_REACH_HOME);
        }
        else
            ScriptedAI::EnterEvadeMode();
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiResetTimer)
        {
            if (m_uiResetTimer <= uiDiff)
            {
                m_creature->HandleEmote(EMOTE_STATE_NONE);
                m_uiResetTimer = 0;
                EnterEvadeMode();
            }
            else
                m_uiResetTimer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_mob_yenniku(Creature* _Creature)
{
    return new mob_yennikuAI(_Creature);
}

enum
{
    SPELL_SMOKE_BOMB = 8817,
    SPELL_GARROTE = 8818,
    SPELL_STEALTH = 8822 // spell that get applied to npc after using smoke bomb
};

enum KurzenActions
{
    KURZEN_ACTION_VANISH,
    KURZEN_ACTION_GAROTTE,
    KURZEN_ACTION_MAX,
};

struct mob_colonel_kurzenAI : public CombatAI
{
    mob_colonel_kurzenAI(Creature* creature) : CombatAI(creature, KURZEN_ACTION_MAX)
    {
        AddCombatAction(KURZEN_ACTION_VANISH, urand(6000, 12000));
        AddCombatAction(KURZEN_ACTION_GAROTTE, true);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case KURZEN_ACTION_VANISH:
            {
                m_creature->SetCombatOnlyStealth(true); // must be before smoke bomb
                DoCastSpellIfCan(nullptr, SPELL_SMOKE_BOMB);
                ResetCombatAction(KURZEN_ACTION_GAROTTE, urand(2000, 6000));
                DisableCombatAction(action);
                break;
            }
            case KURZEN_ACTION_GAROTTE:
            {
                // Safety check if we still have Stealth Aura
                if (m_creature->HasAura(SPELL_STEALTH))
                    DoCastSpellIfCan(m_creature->GetVictim(), SPELL_GARROTE);

                m_creature->SetCombatOnlyStealth(false);
                ResetCombatAction(KURZEN_ACTION_VANISH, urand(6000, 12000));
                DisableCombatAction(action);
                break;
            }
        }
    }
};

/*######
##
######*/

void AddSC_stranglethorn_vale()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "mob_yenniku";
    pNewScript->GetAI = &GetAI_mob_yenniku;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_colonel_kurzen";
    pNewScript->GetAI = &GetNewAIInstance<mob_colonel_kurzenAI>;
    pNewScript->RegisterSelf();
}
