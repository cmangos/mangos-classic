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
SDName: Elwynn_Forest
SD%Complete: 0
SDComment: Placeholder
SDCategory: Elwynn Forest
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"/* ContentData
EndContentData */



enum
{
    NPC_FORLORN_SPIRIT = 2044,
};

// Marshal Haggard's Chest ID: 1562
struct go_marshal_haggards_chestAI : public GameObjectAI
{
    go_marshal_haggards_chestAI(GameObject* go) : GameObjectAI(go) {}

    void OnLootStateChange(Unit* user)
    {
        if (m_go->GetLootState() == GO_JUST_DEACTIVATED)
        {
            Creature* forlornSpirit = GetClosestCreatureWithEntry(m_go, NPC_FORLORN_SPIRIT, 20.0f);
            if (!forlornSpirit || !forlornSpirit->IsAlive())
            {
                if (Creature* spirit = m_go->SummonCreature(NPC_FORLORN_SPIRIT, -9553.5127f, -1430.542f, 62.377f, 4.6077f, TEMPSPAWN_TIMED_OOC_DESPAWN, 300000, true, false))
                {
                    spirit->AI()->AttackStart(user);
                }
            }
            
        }
    }
};

void AddSC_elwynn_forest()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "go_marshal_haggards_chest";
    pNewScript->GetGameObjectAI = &GetNewAIInstance<go_marshal_haggards_chestAI>;
    pNewScript->RegisterSelf();
}
