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
SDName: dire_maul
SD%Complete: 100%
SDComment: Quest support: 7631.
SDCategory: Dire Maul
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"/* ContentData
event_spell_release_jeevee
EndContentData */


#include "dire_maul.h"

/*######
## event_spells_warlock_dreadsteed
######*/

enum
{
    NPC_JEEVEES                 = 14500,

    EVENT_ID_SUMMON_JEEVEES     = 8420,
    EVENT_ID_SUMMON_DREADSTEED  = 8428,
};

bool ProcessEventId_event_spells_warlock_dreadsteed(uint32 uiEventId, Object* pSource, Object* /*pTarget*/, bool bIsStart)
{
    if (bIsStart && pSource->GetTypeId() == TYPEID_PLAYER)
    {
        if (instance_dire_maul* pInstance = (instance_dire_maul*)((Player*)pSource)->GetInstanceData())
        {
            // summon J'eevees and start event
            if (uiEventId == EVENT_ID_SUMMON_JEEVEES)
            {
                if (pInstance->GetData(TYPE_DREADSTEED) == NOT_STARTED)
                {
                    // start event: summon the dummy infernal controller and set in progress
                    // The dummy infernal is used to check for event == FAIL and stop the script on the DB side
                    ((Player*)pSource)->SummonCreature(NPC_WARLOCK_DUMMY_INFERNAL, -37.9392f, 812.805f, -29.4525f, 4.81711f, TEMPSPAWN_DEAD_DESPAWN, 0);

                    // start from point 13. Others are for Scholomance event
                    if (Creature* pImp = ((Player*)pSource)->SummonCreature(NPC_JEEVEES, -37.9392f, 812.805f, -29.4525f, 4.81711f, TEMPSPAWN_DEAD_DESPAWN, 0))
                    {
                        pImp->GetMotionMaster()->MoveWaypoint();
                        pImp->GetMotionMaster()->SetNextWaypoint(13);
                    }

                    pInstance->SetData(TYPE_DREADSTEED, IN_PROGRESS);

                    // allow the rest to be handled by dbscript
                    return false;
                }
            }
            // summon Dreadsteed
            else if (uiEventId == EVENT_ID_SUMMON_DREADSTEED)
            {
                if (pInstance->GetData(TYPE_DREADSTEED) == SPECIAL)
                {
                    // despawn the circle
                    if (GameObject* pCircle = pInstance->GetSingleGameObjectFromStorage(GO_WARLOCK_RITUAL_CIRCLE))
                        pCircle->SetLootState(GO_JUST_DEACTIVATED);

                    pInstance->SetData(TYPE_DREADSTEED, DONE);

                    // rest is done by DBscript
                    return false;
                }
            }
        }
    }
    return true;
}

void AddSC_dire_maul()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "event_spells_warlock_dreadsteed";
    pNewScript->pProcessEventId = &ProcessEventId_event_spells_warlock_dreadsteed;
    pNewScript->RegisterSelf();
}
