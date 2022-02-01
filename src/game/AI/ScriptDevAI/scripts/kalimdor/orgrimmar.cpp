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
SDName: Orgrimmar
SD%Complete: 100
SDComment: Quest support: 2460
SDCategory: Orgrimmar
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"/* ContentData
npc_shenthul
EndContentData */



enum
{
    GO_HEAD_OF_NEFARIAN_OG = 179881,
    GO_HEAD_OF_ONYXIA_OG = 179556,

    GOSSIP_HEAD_OG = 6024,
};

bool GossipHello_npc_overlord_runthalak(Player* player, Creature* creature)
{
    uint32 gossipId = creature->GetCreatureInfo()->GossipMenuId;
    if (GameObject* go = GetClosestGameObjectWithEntry(creature, GO_HEAD_OF_NEFARIAN_OG, 100.f))
        if (go->IsSpawned())
            gossipId = GOSSIP_HEAD_OG;

    player->PrepareGossipMenu(creature, gossipId);
    player->SendPreparedGossip(creature);
    return true;
}

bool GossipHello_npc_high_overlord_saurfang(Player* player, Creature* creature)
{
    uint32 gossipId = creature->GetCreatureInfo()->GossipMenuId;
    if (GameObject* go = GetClosestGameObjectWithEntry(creature, GO_HEAD_OF_ONYXIA_OG, 100.f))
        if (go->IsSpawned())
            gossipId = GOSSIP_HEAD_OG;

    player->PrepareGossipMenu(creature, gossipId);
    player->SendPreparedGossip(creature);
    return true;
}

void AddSC_orgrimmar()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_overlord_runthalak";
    pNewScript->pGossipHello = &GossipHello_npc_overlord_runthalak;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_high_overlord_saurfang";
    pNewScript->pGossipHello = &GossipHello_npc_high_overlord_saurfang;
    pNewScript->RegisterSelf();
}
