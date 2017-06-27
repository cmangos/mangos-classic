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
SDName: Areatrigger_Scripts
SD%Complete: 100
SDComment: Quest support: 4291, 6681, 7632, 273
SDCategory: Areatrigger
EndScriptData

*/

#include "AI/ScriptDevAI/PreCompiledHeader.h"/* ContentData
at_ravenholdt
at_childrens_week_spot          3546,3547,3548,3552,3549,3550
at_scent_larkorwi               1726,1727,1728,1729,1730,1731,1732,1733,1734,1735,1736,1737,1738,1739,1740
at_murkdeep                     1966
at_ancient_leaf                 3587
EndContentData */


#include "world_map_scripts.h"

static uint32 TriggerOrphanSpell[6][3] =
{
    {3546, 14305, 1479},                                    // The Bough of the Eternals
    {3547, 14444, 1800},                                    // Lordaeron Throne Room
    {3548, 14305, 1558},                                    // The Stonewrought Dam
    {3549, 14444, 911},                                     // Gateway to the Frontier
    {3550, 14444, 910},                                     // Down at the Docks
    {3552, 14305, 1687}                                     // Spooky Lighthouse
};

bool AreaTrigger_at_childrens_week_spot(Player* pPlayer, AreaTriggerEntry const* pAt)
{
    for (uint8 i = 0; i < 6; ++i)
    {
        if (pPlayer->IsCurrentQuest(TriggerOrphanSpell[i][2]))
        {
            if (pAt->id == TriggerOrphanSpell[i][0] &&
                pPlayer->GetMiniPet() && pPlayer->GetMiniPet()->GetEntry() == TriggerOrphanSpell[i][1])
            {
                pPlayer->SendQuestCompleteEvent(TriggerOrphanSpell[i][2]);
                pPlayer->CompleteQuest(TriggerOrphanSpell[i][2]);
                return true;
            }
        }
    }

    return false;
}

/*######
## at_ravenholdt
######*/

enum
{
    QUEST_MANOR_RAVENHOLDT  = 6681,
    NPC_RAVENHOLDT          = 13936
};

bool AreaTrigger_at_ravenholdt(Player* pPlayer, AreaTriggerEntry const* /*pAt*/)
{
    if (pPlayer->GetQuestStatus(QUEST_MANOR_RAVENHOLDT) == QUEST_STATUS_INCOMPLETE)
        pPlayer->KilledMonsterCredit(NPC_RAVENHOLDT);

    return false;
}

/*######
## at_scent_larkorwi
######*/

enum
{
    QUEST_SCENT_OF_LARKORWI     = 4291,
    NPC_LARKORWI_MATE           = 9683
};

bool AreaTrigger_at_scent_larkorwi(Player* pPlayer, AreaTriggerEntry const* pAt)
{
    if (pPlayer->isAlive() && !pPlayer->isGameMaster() && pPlayer->GetQuestStatus(QUEST_SCENT_OF_LARKORWI) == QUEST_STATUS_INCOMPLETE)
    {
        if (!GetClosestCreatureWithEntry(pPlayer, NPC_LARKORWI_MATE, 25.0f, false, false))
            pPlayer->SummonCreature(NPC_LARKORWI_MATE, pAt->x, pAt->y, pAt->z, 3.3f, TEMPSUMMON_TIMED_OOC_DESPAWN, 2 * MINUTE * IN_MILLISECONDS);
    }

    return false;
}

/*######
## at_murkdeep
######*/

bool AreaTrigger_at_murkdeep(Player* pPlayer, AreaTriggerEntry const* /*pAt*/)
{
    // Handle Murkdeep event start
    // The area trigger summons 3 Greymist Coastrunners; The rest of the event is handled by world map scripts
    if (pPlayer->isAlive() && !pPlayer->isGameMaster() && pPlayer->GetQuestStatus(QUEST_WANTED_MURKDEEP) == QUEST_STATUS_INCOMPLETE)
    {
        ScriptedMap* pScriptedMap = (ScriptedMap*)pPlayer->GetInstanceData();
        if (!pScriptedMap)
            return false;

        // If Murkdeep is already spawned, skip the rest
        if (pScriptedMap->GetSingleCreatureFromStorage(NPC_MURKDEEP, true))
            return true;

        // Check if there are already coastrunners (dead or alive) around the area
        if (GetClosestCreatureWithEntry(pPlayer, NPC_GREYMIST_COASTRUNNNER, 60.0f, false, false))
            return true;

        float fX, fY, fZ;
        for (uint8 i = 0; i < 3; ++i)
        {
            // Spawn locations are defined in World Maps Scripts.h
            pPlayer->GetRandomPoint(aSpawnLocations[POS_IDX_MURKDEEP_SPAWN][0], aSpawnLocations[POS_IDX_MURKDEEP_SPAWN][1], aSpawnLocations[POS_IDX_MURKDEEP_SPAWN][2], 5.0f, fX, fY, fZ);

            if (Creature* pTemp = pPlayer->SummonCreature(NPC_GREYMIST_COASTRUNNNER, fX, fY, fZ, aSpawnLocations[POS_IDX_MURKDEEP_SPAWN][3], TEMPSUMMON_DEAD_DESPAWN, 0))
            {
                pTemp->SetWalk(false);
                pTemp->GetRandomPoint(aSpawnLocations[POS_IDX_MURKDEEP_MOVE][0], aSpawnLocations[POS_IDX_MURKDEEP_MOVE][1], aSpawnLocations[POS_IDX_MURKDEEP_MOVE][2], 5.0f, fX, fY, fZ);
                pTemp->GetMotionMaster()->MovePoint(0, fX, fY, fZ);
            }
        }
    }

    return false;
}


/*######
## at_ancient_leaf
######*/

enum
{
    QUEST_ANCIENT_LEAF              = 7632,

    NPC_VARTRUS                     = 14524,
    NPC_STOMA                       = 14525,
    NPC_HASTAT                      = 14526,

    MAX_ANCIENTS                    = 3,
};

struct AncientSpawn
{
    uint32 uiEntry;
    float fX, fY, fZ, fO;
};

static const AncientSpawn afSpawnLocations[MAX_ANCIENTS] =
{
    { NPC_VARTRUS, 6204.051758f, -1172.575684f, 370.079224f, 0.86052f },    // Vartus the Ancient
    { NPC_STOMA,   6246.953613f, -1155.985718f, 366.182953f, 2.90269f },    // Stoma the Ancient
    { NPC_HASTAT,  6193.449219f, -1137.834106f, 366.260529f, 5.77332f },    // Hastat the Ancient
};

bool AreaTrigger_at_ancient_leaf(Player* pPlayer, AreaTriggerEntry const* pAt)
{
    if (pPlayer->isGameMaster() || !pPlayer->isAlive())
        return false;

    // Handle Call Ancients event start - The area trigger summons 3 ancients
    if (pPlayer->GetQuestStatus(QUEST_ANCIENT_LEAF) == QUEST_STATUS_COMPLETE)
    {
        // If ancients are already spawned, skip the rest
        if (GetClosestCreatureWithEntry(pPlayer, NPC_VARTRUS, 50.0f) || GetClosestCreatureWithEntry(pPlayer, NPC_STOMA, 50.0f) || GetClosestCreatureWithEntry(pPlayer, NPC_HASTAT, 50.0f))
            return true;

        for (uint8 i = 0; i < MAX_ANCIENTS; ++i)
            pPlayer->SummonCreature(afSpawnLocations[i].uiEntry, afSpawnLocations[i].fX, afSpawnLocations[i].fY, afSpawnLocations[i].fZ, afSpawnLocations[i].fO, TEMPSUMMON_TIMED_DESPAWN, 5 * MINUTE * IN_MILLISECONDS);
    }

    return false;
}

/*######
## Miran and Huldar are Ambushed when AT-171 is triggered
## when quest 273 is active
######*/

enum
{
    QUEST_RESUPPLYING_THE_EXCAVATION    = 273,

    NPC_SAEAN                           = 1380,
    NPC_MIRAN                           = 1379,
    NPC_HULDAR                          = 2057,
    NPC_DARK_IRON_AMBUSHER              = 1981,

    FACTION_HOSTILE                     = 14
};

struct Location
{
    float m_fX, m_fY, m_fZ, m_fO;
};

static const Location m_miranAmbushSpawns[] =
{
    { -5760.73f, -3437.71f, 305.54f, 2.41f },   // Saean 
    { -5759.85f, -3441.29f, 305.57f, 2.24f },   // Dark Iron Ambusher 1
    { -5757.75f, -3437.61f, 304.32f, 2.56f },   // Dark Iron Ambusher 2
};

bool AreaTrigger_at_huldar_miran(Player* pPlayer, AreaTriggerEntry const* /*pAt*/)
{
    // Player is deaed, a GM, quest complete or no quest, do nothing
    if (!pPlayer->isAlive() || pPlayer->isGameMaster() ||
        pPlayer->GetQuestStatus(QUEST_RESUPPLYING_THE_EXCAVATION) == QUEST_STATUS_COMPLETE ||
        pPlayer->GetQuestStatus(QUEST_RESUPPLYING_THE_EXCAVATION) == QUEST_STATUS_NONE)
        return false;

    ScriptedMap* pScriptedMap = (ScriptedMap*)pPlayer->GetInstanceData();
    if (!pScriptedMap)
        return false;

    Creature* m_miran = GetClosestCreatureWithEntry(pPlayer, NPC_MIRAN, 60.0f, true);
    Creature* m_huldar = GetClosestCreatureWithEntry(pPlayer, NPC_HULDAR, 60.0f, true);
    Creature* m_saean = GetClosestCreatureWithEntry(pPlayer, NPC_SAEAN, 60.0f, true);

    // Quest NPCs not availble, do noting
    if (!m_miran || !m_huldar)
        return false;

    // complete quest
    pPlayer->CompleteQuest(QUEST_RESUPPLYING_THE_EXCAVATION);
    pPlayer->SendQuestCompleteEvent(QUEST_RESUPPLYING_THE_EXCAVATION);

    // Quest NPCs in combat, skip the rest, prevent double spawns
    if (m_miran->isInCombat() || m_huldar->isInCombat()) 
        return true;

    // Check if Saean is spawned and set his faction to hostile - summon him if not spawned
    if (m_saean)
        m_saean->SetFactionTemporary(FACTION_HOSTILE, TEMPFACTION_RESTORE_RESPAWN);
    else 
    {
        m_huldar
            ->SummonCreature(NPC_SAEAN, m_miranAmbushSpawns[0].m_fX, m_miranAmbushSpawns[0].m_fY, m_miranAmbushSpawns[0].m_fZ, m_miranAmbushSpawns[0].m_fO, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 25000)
            ->SetFactionTemporary(FACTION_HOSTILE, TEMPFACTION_RESTORE_RESPAWN);
        m_saean = GetClosestCreatureWithEntry(pPlayer, NPC_SAEAN, 60.0f, true);
    }
   
    // Check if any Dark Iron Ambusher are already spawned or dead, if so, do nothing
    if (!GetClosestCreatureWithEntry(pPlayer, NPC_DARK_IRON_AMBUSHER, 60.0f, false, false))
    {
        m_saean->SummonCreature(NPC_DARK_IRON_AMBUSHER, m_miranAmbushSpawns[1].m_fX, m_miranAmbushSpawns[1].m_fY, m_miranAmbushSpawns[1].m_fZ, m_miranAmbushSpawns[1].m_fO, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 25000);
        m_saean->SummonCreature(NPC_DARK_IRON_AMBUSHER, m_miranAmbushSpawns[2].m_fX, m_miranAmbushSpawns[2].m_fY, m_miranAmbushSpawns[2].m_fZ, m_miranAmbushSpawns[2].m_fO, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 25000);
    }

    return true;
}

void AddSC_areatrigger_scripts()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "at_childrens_week_spot";
    pNewScript->pAreaTrigger = &AreaTrigger_at_childrens_week_spot;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "at_ravenholdt";
    pNewScript->pAreaTrigger = &AreaTrigger_at_ravenholdt;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "at_scent_larkorwi";
    pNewScript->pAreaTrigger = &AreaTrigger_at_scent_larkorwi;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "at_murkdeep";
    pNewScript->pAreaTrigger = &AreaTrigger_at_murkdeep;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "at_ancient_leaf";
    pNewScript->pAreaTrigger = &AreaTrigger_at_ancient_leaf;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "at_huldar_miran";
    pNewScript->pAreaTrigger = &AreaTrigger_at_huldar_miran;
    pNewScript->RegisterSelf();
}
