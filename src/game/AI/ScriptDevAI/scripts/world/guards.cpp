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
SDName: Guards
SD%Complete: 100
SDComment: CombatAI should be organized better for future.
SDCategory: Guards
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"/* ContentData
guard_bluffwatcher
guard_contested
guard_darnassus
guard_dunmorogh
guard_durotar
guard_elwynnforest
guard_ironforge
guard_mulgore
guard_orgrimmar
guard_stormwind
guard_teldrassil
guard_tirisfal
guard_undercity
EndContentData */


#include "AI/ScriptDevAI/base/guard_ai.h"

UnitAI* GetAI_guard_bluffwatcher(Creature* pCreature)
{
    return new guardAI(pCreature);
}

UnitAI* GetAI_guard_contested(Creature* pCreature)
{
    return new guardAI(pCreature);
}

UnitAI* GetAI_guard_darnassus(Creature* pCreature)
{
    return new guardAI(pCreature);
}

UnitAI* GetAI_guard_dunmorogh(Creature* pCreature)
{
    return new guardAI(pCreature);
}

UnitAI* GetAI_guard_durotar(Creature* pCreature)
{
    return new guardAI(pCreature);
}

UnitAI* GetAI_guard_elwynnforest(Creature* pCreature)
{
    return new guardAI(pCreature);
}

UnitAI* GetAI_guard_ironforge(Creature* pCreature)
{
    return new guardAI(pCreature);
}

UnitAI* GetAI_guard_mulgore(Creature* pCreature)
{
    return new guardAI(pCreature);
}

UnitAI* GetAI_guard_orgrimmar(Creature* pCreature)
{
    return new guardAI_orgrimmar(pCreature);
}

enum{
    SPELL_WINDSOR_INSPIRATION_EFFECT = 20275,
    MAX_GUARD_SALUTES                = 7,
};

static const int32 aGuardSalute[MAX_GUARD_SALUTES] = { -1000842, -1000843, -1000844, -1000845, -1000846, -1000847, -1000848};

struct guardAI_stormwind : public guardAI
{
    guardAI_stormwind(Creature* creature) : guardAI(creature)
    {
        Reset();
    }

    uint32 m_saluteWaitTimer;

    void Reset() override
    {
        m_saluteWaitTimer = 0;
    }

    void SpellHit(Unit* /*caster*/, const SpellEntry* spell) override
    {
        if (spell->Id == SPELL_WINDSOR_INSPIRATION_EFFECT && !m_saluteWaitTimer)
        {
            DoScriptText(aGuardSalute[urand(0, MAX_GUARD_SALUTES - 1)], m_creature);
            m_saluteWaitTimer = 2 * MINUTE * IN_MILLISECONDS;
        }
    }

    void UpdateAI(const uint32 diff) override
    {
        if (m_saluteWaitTimer)
        {
            if (m_saluteWaitTimer < diff)
                m_saluteWaitTimer = 0;
            else
                m_saluteWaitTimer -= diff;
        }

        guardAI::UpdateAI(diff);
    }

    void  ReceiveEmote(Player* player, uint32 textEmote) override
    {
        if (player->GetTeam() == ALLIANCE)
            DoReplyToTextEmote(textEmote);
    }
};

UnitAI* GetAI_guard_stormwind(Creature* pCreature)
{
    return new guardAI_stormwind(pCreature);
}

UnitAI* GetAI_guard_teldrassil(Creature* pCreature)
{
    return new guardAI(pCreature);
}

UnitAI* GetAI_guard_tirisfal(Creature* pCreature)
{
    return new guardAI(pCreature);
}

UnitAI* GetAI_guard_undercity(Creature* pCreature)
{
    return new guardAI(pCreature);
}

void AddSC_guards()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "guard_bluffwatcher";
    pNewScript->GetAI = &GetAI_guard_bluffwatcher;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "guard_contested";
    pNewScript->GetAI = &GetAI_guard_contested;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "guard_darnassus";
    pNewScript->GetAI = &GetAI_guard_darnassus;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "guard_dunmorogh";
    pNewScript->GetAI = &GetAI_guard_dunmorogh;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "guard_durotar";
    pNewScript->GetAI = &GetAI_guard_durotar;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "guard_elwynnforest";
    pNewScript->GetAI = &GetAI_guard_elwynnforest;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "guard_ironforge";
    pNewScript->GetAI = &GetAI_guard_ironforge;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "guard_mulgore";
    pNewScript->GetAI = &GetAI_guard_mulgore;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "guard_orgrimmar";
    pNewScript->GetAI = &GetAI_guard_orgrimmar;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "guard_stormwind";
    pNewScript->GetAI = &GetAI_guard_stormwind;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "guard_teldrassil";
    pNewScript->GetAI = &GetAI_guard_teldrassil;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "guard_tirisfal";
    pNewScript->GetAI = &GetAI_guard_tirisfal;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "guard_undercity";
    pNewScript->GetAI = &GetAI_guard_undercity;
    pNewScript->RegisterSelf();
}
