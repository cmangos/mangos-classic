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
SDName: instance_dire_maul
SD%Complete: 30
SDComment: Basic Support - Most events and quest-related stuff missing
SDCategory: Dire Maul
EndScriptData */

#include "precompiled.h"
#include "dire_maul.h"

instance_dire_maul::instance_dire_maul(Map* pMap) : ScriptedInstance(pMap),
    m_bWallDestroyed(false),
    m_bDoNorthBeforeWest(false)
{
    Initialize();
}

void instance_dire_maul::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
}

void instance_dire_maul::Update(uint32 uiDiff)
{
    if (GetData(TYPE_RITUAL) == IN_PROGRESS)
    {
        if (instance->GetGameObject(m_mGoEntryGuidStore[GO_BELL_OF_DETHMOORA])->GetGoState() != GO_STATE_ACTIVE
            && instance->GetGameObject(m_mGoEntryGuidStore[GO_WHEEL_OF_BLACK_MARCH])->GetGoState() != GO_STATE_ACTIVE
            && instance->GetGameObject(m_mGoEntryGuidStore[GO_DOOMSDAY_CANDLE])->GetGoState() != GO_STATE_ACTIVE)
            SetData(TYPE_RITUAL, FAIL);
    }
}

void instance_dire_maul::OnPlayerEnter(Player* pPlayer)
{
    // figure where to enter to set library doors accordingly
    // Enter DM North first
    if (pPlayer->IsWithinDist2d(260.0f, -20.0f, 20.0f) && m_auiEncounter[TYPE_WARPWOOD] != DONE)
        m_bDoNorthBeforeWest = true;
    else
        m_bDoNorthBeforeWest = false;

    DoToggleGameObjectFlags(GO_WEST_LIBRARY_DOOR, GO_FLAG_NO_INTERACT, m_bDoNorthBeforeWest);
    DoToggleGameObjectFlags(GO_WEST_LIBRARY_DOOR, GO_FLAG_LOCKED, !m_bDoNorthBeforeWest);
}

void instance_dire_maul::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
            // East
        case NPC_OLD_IRONBARK:
            break;

            // West
        case NPC_PRINCE_TORTHELDRIN:
            if (m_auiEncounter[TYPE_IMMOLTHAR] == DONE)
                pCreature->SetFactionTemporary(FACTION_HOSTILE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_OOC_NOT_ATTACK);
            break;
        case NPC_ARCANE_ABERRATION:
        case NPC_MANA_REMNANT:
            m_lGeneratorGuardGUIDs.push_back(pCreature->GetObjectGuid());
            return;
        case NPC_IMMOLTHAR:
            break;
        case NPC_HIGHBORNE_SUMMONER:
            m_luiHighborneSummonerGUIDs.push_back(pCreature->GetObjectGuid());
            return;
        case NPC_RITUAL_DUMMY:
            break;

            // North
        case NPC_CHORUSH:
        case NPC_KING_GORDOK:
        case NPC_CAPTAIN_KROMCRUSH:
            break;

        default:
            return;
    }
    m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
}

void instance_dire_maul::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
            // East
        case GO_CONSERVATORY_DOOR:
            if (m_auiEncounter[TYPE_IRONBARK] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_CRUMBLE_WALL:
            if (m_bWallDestroyed || m_auiEncounter[TYPE_ALZZIN] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_CORRUPT_VINE:
            if (m_auiEncounter[TYPE_ALZZIN] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_FELVINE_SHARD:
            m_lFelvineShardGUIDs.push_back(pGo->GetObjectGuid());
            break;

            // West
        case GO_CRYSTAL_GENERATOR_1:
            m_aCrystalGeneratorGuid[0] = pGo->GetObjectGuid();
            if (m_auiEncounter[TYPE_PYLON_1] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            return;
        case GO_CRYSTAL_GENERATOR_2:
            m_aCrystalGeneratorGuid[1] = pGo->GetObjectGuid();
            if (m_auiEncounter[TYPE_PYLON_2] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            return;
        case GO_CRYSTAL_GENERATOR_3:
            m_aCrystalGeneratorGuid[2] = pGo->GetObjectGuid();
            if (m_auiEncounter[TYPE_PYLON_3] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            return;
        case GO_CRYSTAL_GENERATOR_4:
            m_aCrystalGeneratorGuid[3] = pGo->GetObjectGuid();
            if (m_auiEncounter[TYPE_PYLON_4] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            return;
        case GO_CRYSTAL_GENERATOR_5:
            m_aCrystalGeneratorGuid[4] = pGo->GetObjectGuid();
            if (m_auiEncounter[TYPE_PYLON_5] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            return;
        case GO_FORCEFIELD:
            if (CheckAllGeneratorsDestroyed())
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PRINCES_CHEST_AURA:
            break;
        case GO_WARPWOOD_DOOR:
            if (m_auiEncounter[TYPE_WARPWOOD] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_WEST_LIBRARY_DOOR:
            pGo->SetFlag(GAMEOBJECT_FLAGS, m_bDoNorthBeforeWest ? GO_FLAG_NO_INTERACT : GO_FLAG_LOCKED);
            pGo->RemoveFlag(GAMEOBJECT_FLAGS, m_bDoNorthBeforeWest ? GO_FLAG_LOCKED : GO_FLAG_NO_INTERACT);
            break;
        case GO_DREADSTEED_PORTAL:
            m_lDreadsteedPortalGUIDs.push_back(pGo->GetObjectGuid());
            break;
        case GO_BELL_OF_DETHMOORA:
        case GO_WHEEL_OF_BLACK_MARCH:
        case GO_DOOMSDAY_CANDLE:
        case GO_WARLOCK_RITUAL_CIRCLE:
            break;
        case GO_MOUNT_QUEST_SYMBOL1:
        case GO_MOUNT_QUEST_SYMBOL2:
        case GO_MOUNT_QUEST_SYMBOL3:
            m_lRitualSymbolGUIDs.push_back(pGo->GetObjectGuid());
            break;

            // North
        case GO_NORTH_LIBRARY_DOOR:
            break;

        default:
            return;
    }
    m_mGoEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
}

void instance_dire_maul::SetData(uint32 uiType, uint32 uiData)
{
    switch (uiType)
    {
            // East
        case TYPE_ZEVRIM:
            if (uiData == DONE)
            {
                // Update Old Ironbark so he can open the conservatory door
                if (Creature* pIronbark = GetSingleCreatureFromStorage(NPC_OLD_IRONBARK))
                {
                    DoScriptText(SAY_IRONBARK_REDEEM, pIronbark);
                    pIronbark->UpdateEntry(NPC_IRONBARK_REDEEMED);
                }
            }
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_IRONBARK:
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_ALZZIN:                                   // This Encounter is expected to be handled within Acid (reason handling at 50% hp)
            if (uiData == DONE)
            {
                if (!m_bWallDestroyed)
                {
                    DoUseDoorOrButton(GO_CRUMBLE_WALL);
                    m_bWallDestroyed = true;
                }

                DoUseDoorOrButton(GO_CORRUPT_VINE);

                if (!m_lFelvineShardGUIDs.empty())
                {
                    for (GuidList::const_iterator itr = m_lFelvineShardGUIDs.begin(); itr != m_lFelvineShardGUIDs.end(); ++itr)
                        DoRespawnGameObject(*itr);
                }
            }
            else if (uiData == SPECIAL && !m_bWallDestroyed)
            {
                DoUseDoorOrButton(GO_CRUMBLE_WALL);
                m_bWallDestroyed = true;
            }
            m_auiEncounter[uiType] = uiData;
            break;

            // West
        case TYPE_WARPWOOD:
            if (uiData == DONE)
                DoUseDoorOrButton(GO_WARPWOOD_DOOR);
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_IMMOLTHAR:
            if (uiData == DONE)
            {
                if (Creature* pPrince = GetSingleCreatureFromStorage(NPC_PRINCE_TORTHELDRIN))
                {
                    DoScriptText(SAY_FREE_IMMOLTHAR, pPrince);
                    pPrince->SetFactionTemporary(FACTION_HOSTILE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_OOC_NOT_ATTACK);
                    // Despawn Chest-Aura
                    if (GameObject* pChestAura = GetSingleGameObjectFromStorage(GO_PRINCES_CHEST_AURA))
                        pChestAura->Use(pPrince);
                }
            }
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_PRINCE:
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_PYLON_1:
        case TYPE_PYLON_2:
        case TYPE_PYLON_3:
        case TYPE_PYLON_4:
        case TYPE_PYLON_5:
            m_auiEncounter[uiType] = uiData;
            if (uiData == DONE)
            {
                DoUseDoorOrButton(m_aCrystalGeneratorGuid[uiType - TYPE_PYLON_1]);
                if (CheckAllGeneratorsDestroyed())
                    ProcessForceFieldOpening();
            }
            break;
        case TYPE_RITUAL:
            m_auiEncounter[uiType] = uiData;
            if (uiData == IN_PROGRESS)
            {
                if (GameObject *pGo = GetSingleGameObjectFromStorage(GO_WARLOCK_RITUAL_CIRCLE))
                {
                    pGo->SetRespawnTime(900);
                    pGo->Refresh();
                }

                m_lDreadsteedPortalGUIDs.sort();
                int count = 0; // Skip boss portal - this is spawned when event is completed (must be lowest GUID)
                for (auto portal : m_lDreadsteedPortalGUIDs)
                {
                    if (count != 0)
                        DoRespawnGameObject(portal, 360);

                    count++;
                }
            }
            else if (uiData == FAIL)
            {
                if (Creature* pRitual = GetSingleCreatureFromStorage(NPC_RITUAL_DUMMY))
                    pRitual->ForcedDespawn();

                for (auto portal : m_lDreadsteedPortalGUIDs)
                    instance->GetGameObject(portal)->SetLootState(GO_JUST_DEACTIVATED);

                for (auto symbol : m_lRitualSymbolGUIDs)
                    instance->GetGameObject(symbol)->SetLootState(GO_JUST_DEACTIVATED);

                if (GameObject *pGo = GetSingleGameObjectFromStorage(GO_BELL_OF_DETHMOORA))
                    pGo->SetLootState(GO_JUST_DEACTIVATED);

                if (GameObject *pGo = GetSingleGameObjectFromStorage(GO_DOOMSDAY_CANDLE))
                    pGo->SetLootState(GO_JUST_DEACTIVATED);

                if (GameObject *pGo = GetSingleGameObjectFromStorage(GO_WHEEL_OF_BLACK_MARCH))
                    pGo->SetLootState(GO_JUST_DEACTIVATED);

                if (GameObject *pGo = GetSingleGameObjectFromStorage(GO_WARLOCK_RITUAL_CIRCLE))
                    pGo->SetLootState(GO_JUST_DEACTIVATED);
            }
            break;

            // North
        case TYPE_KING_GORDOK:
            m_auiEncounter[uiType] = uiData;
            if (uiData == DONE)
            {
                // change faction to certian ogres
                if (Creature* pOgre = GetSingleCreatureFromStorage(NPC_CAPTAIN_KROMCRUSH))
                {
                    if (pOgre->isAlive())
                    {
                        pOgre->SetFactionTemporary(FACTION_FRIENDLY, TEMPFACTION_RESTORE_RESPAWN);

                        // only evade if required
                        if (pOgre->getVictim())
                            pOgre->AI()->EnterEvadeMode();
                    }
                }

                if (Creature* pOgre = GetSingleCreatureFromStorage(NPC_CHORUSH))
                {
                    // Chorush evades and yells on king death (if alive)
                    if (pOgre->isAlive())
                    {
                        DoScriptText(SAY_CHORUSH_KING_DEAD, pOgre);
                        pOgre->SetFactionTemporary(FACTION_FRIENDLY, TEMPFACTION_RESTORE_RESPAWN);
                        pOgre->AI()->EnterEvadeMode();
                    }

                    // start WP movement for Mizzle; event handled by movement and gossip dbscripts
                    if (Creature* pMizzle = pOgre->SummonCreature(NPC_MIZZLE_THE_CRAFTY, afMizzleSpawnLoc[0], afMizzleSpawnLoc[1], afMizzleSpawnLoc[2], afMizzleSpawnLoc[3], TEMPSUMMON_DEAD_DESPAWN, 0, true))
                    {
                        pMizzle->SetWalk(false);
                        pMizzle->GetMotionMaster()->MoveWaypoint();
                    }
                }
            }
            break;
        case TYPE_MOLDAR:
        case TYPE_FENGUS:
        case TYPE_SLIPKIK:
        case TYPE_KROMCRUSH:
            m_auiEncounter[uiType] = uiData;
            break;
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream    << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " "
                      << m_auiEncounter[3] << " " << m_auiEncounter[4] << " " << m_auiEncounter[5] << " "
                      << m_auiEncounter[6] << " " << m_auiEncounter[7] << " " << m_auiEncounter[8] << " "
                      << m_auiEncounter[9] << " " << m_auiEncounter[10] << " " << m_auiEncounter[11] << " "
                      << m_auiEncounter[12] << " " << m_auiEncounter[13] << " " << m_auiEncounter[14] << " "
                      << m_auiEncounter[15] << " " << m_auiEncounter[16];

        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

uint32 instance_dire_maul::GetData(uint32 uiType) const
{
    if (uiType < MAX_ENCOUNTER)
        return m_auiEncounter[uiType];

    return 0;
}

void instance_dire_maul::OnCreatureEnterCombat(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
            // West
            // - Handling of guards of generators
        case NPC_ARCANE_ABERRATION:
        case NPC_MANA_REMNANT:
            SortPylonGuards();
            break;
            // - Set InstData for ImmolThar
        case NPC_IMMOLTHAR:
            SetData(TYPE_IMMOLTHAR, IN_PROGRESS);
            break;
    }
}

void instance_dire_maul::OnCreatureDeath(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
            // East
            // - Handling Zevrim and Old Ironbark for the door event
        case NPC_ZEVRIM_THORNHOOF:
            SetData(TYPE_ZEVRIM, DONE);
            break;
        case NPC_IRONBARK_REDEEMED:
            SetData(TYPE_IRONBARK, DONE);
            break;

            // West
            // - Handling of guards of generators
        case NPC_ARCANE_ABERRATION:
        case NPC_MANA_REMNANT:
            PylonGuardJustDied(pCreature);
            break;
            // - InstData settings
        case NPC_TENDRIS_WARPWOOD:
            SetData(TYPE_WARPWOOD, DONE);
            break;
        case NPC_IMMOLTHAR:
            SetData(TYPE_IMMOLTHAR, DONE);
            break;

            // North
            // - Handling of Ogre Boss (Assume boss can be handled in Acid)
        case NPC_KING_GORDOK:
            SetData(TYPE_KING_GORDOK, DONE);
            break;
            // Handle Ogre guards for Tribute Run chest
        case NPC_GUARD_MOLDAR:
            SetData(TYPE_MOLDAR, DONE);
            break;
        case NPC_GUARD_FENGUS:
            SetData(TYPE_FENGUS, DONE);
            break;
        case NPC_GUARD_SLIPKIK:
            SetData(TYPE_SLIPKIK, DONE);
            break;
        case NPC_CAPTAIN_KROMCRUSH:
            SetData(TYPE_KROMCRUSH, DONE);
            break;
    }
}

void instance_dire_maul::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >>   m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >>
               m_auiEncounter[3] >> m_auiEncounter[4] >> m_auiEncounter[5] >>
               m_auiEncounter[6] >> m_auiEncounter[7] >> m_auiEncounter[8] >>
               m_auiEncounter[9] >> m_auiEncounter[10] >> m_auiEncounter[11] >>
               m_auiEncounter[12] >> m_auiEncounter[13] >> m_auiEncounter[14] >>
               m_auiEncounter[15] >> m_auiEncounter[16];

    if (m_auiEncounter[TYPE_ALZZIN] >= DONE)
        m_bWallDestroyed = true;

    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            m_auiEncounter[i] = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

bool instance_dire_maul::CheckConditionCriteriaMeet(Player const* pPlayer, uint32 uiInstanceConditionId, WorldObject const* pConditionSource, uint32 conditionSourceType) const
{
    switch (uiInstanceConditionId)
    {
        case INSTANCE_CONDITION_ID_NORMAL_MODE:             // No guards alive
        case INSTANCE_CONDITION_ID_HARD_MODE:               // One guard alive
        case INSTANCE_CONDITION_ID_HARD_MODE_2:             // Two guards alive
        case INSTANCE_CONDITION_ID_HARD_MODE_3:             // Three guards alive
        case INSTANCE_CONDITION_ID_HARD_MODE_4:             // All guards alive
        {
            uint8 uiTributeRunAliveBosses = (GetData(TYPE_MOLDAR) != DONE ? 1 : 0) + (GetData(TYPE_FENGUS) != DONE ? 1 : 0) + (GetData(TYPE_SLIPKIK) != DONE ? 1 : 0)
                                            + (GetData(TYPE_KROMCRUSH) != DONE ? 1 : 0);

            return uiInstanceConditionId == uiTributeRunAliveBosses;
        }
        case 5:                                             // Don't allow summon Dreadsteed boss if ritual isn't complete
            return GetData(TYPE_RITUAL) != DONE ? 1 : 0;
    }

    script_error_log("instance_dire_maul::CheckConditionCriteriaMeet called with unsupported Id %u. Called with param plr %s, src %s, condition source type %u",
                     uiInstanceConditionId, pPlayer ? pPlayer->GetGuidStr().c_str() : "nullptr", pConditionSource ? pConditionSource->GetGuidStr().c_str() : "nullptr", conditionSourceType);
    return false;
}

GuidVector instance_dire_maul::GetRitualSymbolGuids()
{
    return m_lRitualSymbolGUIDs;
}

bool instance_dire_maul::CheckAllGeneratorsDestroyed()
{
    if (m_auiEncounter[TYPE_PYLON_1] != DONE || m_auiEncounter[TYPE_PYLON_2] != DONE || m_auiEncounter[TYPE_PYLON_3] != DONE || m_auiEncounter[TYPE_PYLON_4] != DONE || m_auiEncounter[TYPE_PYLON_5] != DONE)
        return false;

    return true;
}

void instance_dire_maul::ProcessForceFieldOpening()
{
    // 'Open' the force field
    DoUseDoorOrButton(GO_FORCEFIELD);

    // Let the summoners attack Immol'Thar
    Creature* pImmolThar = GetSingleCreatureFromStorage(NPC_IMMOLTHAR);
    if (!pImmolThar || pImmolThar->isDead())
        return;

    bool bHasYelled = false;
    for (GuidList::const_iterator itr = m_luiHighborneSummonerGUIDs.begin(); itr != m_luiHighborneSummonerGUIDs.end(); ++itr)
    {
        Creature* pSummoner = instance->GetCreature(*itr);

        if (!bHasYelled && pSummoner)
        {
            DoScriptText(SAY_KILL_IMMOLTHAR, pSummoner);
            bHasYelled = true;
        }

        if (!pSummoner || pSummoner->isDead())
            continue;

        pSummoner->AI()->AttackStart(pImmolThar);
    }
    m_luiHighborneSummonerGUIDs.clear();
}

void instance_dire_maul::SortPylonGuards()
{
    if (!m_lGeneratorGuardGUIDs.empty())
    {
        for (uint8 i = 0; i < MAX_GENERATORS; ++i)
        {
            GameObject* pGenerator = instance->GetGameObject(m_aCrystalGeneratorGuid[i]);
            // Skip non-existing or finished generators
            if (!pGenerator || GetData(TYPE_PYLON_1 + i) == DONE)
                continue;

            // Sort all remaining (alive) NPCs to unfinished generators
            for (GuidList::iterator itr = m_lGeneratorGuardGUIDs.begin(); itr != m_lGeneratorGuardGUIDs.end();)
            {
                Creature* pGuard = instance->GetCreature(*itr);
                if (!pGuard || pGuard->isDead())    // Remove invalid guids and dead guards
                {
                    m_lGeneratorGuardGUIDs.erase(itr++);
                    continue;
                }

                if (pGuard->IsWithinDist2d(pGenerator->GetPositionX(), pGenerator->GetPositionY(), 20.0f))
                {
                    m_sSortedGeneratorGuards[i].insert(pGuard->GetGUIDLow());
                    m_lGeneratorGuardGUIDs.erase(itr++);
                }
                else
                    ++itr;
            }
        }
    }
}

void instance_dire_maul::PylonGuardJustDied(Creature* pCreature)
{
    for (uint8 i = 0; i < MAX_GENERATORS; ++i)
    {
        // Skip already activated generators
        if (GetData(TYPE_PYLON_1 + i) == DONE)
            continue;

        // Only process generator where the npc is sorted in
        if (m_sSortedGeneratorGuards[i].find(pCreature->GetGUIDLow()) != m_sSortedGeneratorGuards[i].end())
        {
            m_sSortedGeneratorGuards[i].erase(pCreature->GetGUIDLow());
            if (m_sSortedGeneratorGuards[i].empty())
                SetData(TYPE_PYLON_1 + i, DONE);

            break;
        }
    }
}

/*######
## npc_warlock_ritual_mob
######*/

enum
{
    MAX_SUMMON_POSITIONS = 201,
};

struct sSummonInformation
{
    uint32 uiPosition, uiEntry;
    float fX, fY, fZ, fO;
};

static const sSummonInformation asSummonInfo[MAX_SUMMON_POSITIONS] =
{
    { 1, NPC_DREAD_GUARD, -37.7177f, 743.281f, -27.0769f, 1.5708f },
    { 1, NPC_XOROTHIAN_IMP, 5.8428f, 865.497f, -27.2486f, 3.9968f },
    { 1, NPC_XOROTHIAN_IMP, -106.235f, 800.934f, -27.3196f, 0.174533f },
    { 1, NPC_XOROTHIAN_IMP, -82.9582f, 866.112f, -27.2077f, 5.41052f },
    { 1, NPC_XOROTHIAN_IMP, -98.1468f, 778.083f, -27.254f, 0.541052f },
    { 1, NPC_XOROTHIAN_IMP, -82.6964f, 759.662f, -27.1815f, 0.890118f },
    { 1, NPC_XOROTHIAN_IMP, -39.7076f, 882.205f, -27.1575f, 4.7473f },
    { 1, NPC_XOROTHIAN_IMP, -97.5622f, 848.091f, -27.3099f, 5.75959f },
    { 1, NPC_XOROTHIAN_IMP, 21.162f, 778.232f, -27.2357f, 2.60054f },
    { 1, NPC_XOROTHIAN_IMP, -61.2501f, 747.296f, -27.126f, 1.23918f },
    { 1, NPC_XOROTHIAN_IMP, -37.7177f, 743.281f, -27.0769f, 1.5708f },
    { 1, NPC_XOROTHIAN_IMP, -15.0206f, 878.409f, -27.0947f, 4.36332f },
    { 1, NPC_XOROTHIAN_IMP, 29.5943f, 801.049f, -27.1773f, 2.96706f },
    { 1, NPC_XOROTHIAN_IMP, 20.9339f, 848.326f, -27.1628f, 3.66519f },
    { 1, NPC_XOROTHIAN_IMP, 5.62079f, 759.756f, -27.1948f, 2.25147f },
    { 1, NPC_XOROTHIAN_IMP, -39.7076f, 882.205f, -27.1575f, 4.7473f },
    { 1, NPC_XOROTHIAN_IMP, 21.162f, 778.232f, -27.2357f, 2.60054f },
    { 1, NPC_XOROTHIAN_IMP, -82.6964f, 759.662f, -27.1815f, 0.890118f },
    { 1, NPC_XOROTHIAN_IMP, -82.9582f, 866.112f, -27.2077f, 5.41052f },
    { 1, NPC_XOROTHIAN_IMP, -14.4863f, 747.953f, -27.1958f, 1.91986f },
    { 2, NPC_DREAD_GUARD, 21.162f, 778.232f, -27.2357f, 2.60054f },
    { 2, NPC_XOROTHIAN_IMP, -98.1468f, 778.083f, -27.254f, 0.541052f },
    { 2, NPC_XOROTHIAN_IMP, 5.8428f, 865.497f, -27.2486f, 3.9968f },
    { 2, NPC_XOROTHIAN_IMP, -62.4677f, 878.058f, -27.1771f, 5.07891f },
    { 2, NPC_XOROTHIAN_IMP, -37.7177f, 743.281f, -27.0769f, 1.5708f },
    { 2, NPC_XOROTHIAN_IMP, 5.62079f, 759.756f, -27.1948f, 2.25147f },
    { 2, NPC_XOROTHIAN_IMP, 21.162f, 778.232f, -27.2357f, 2.60054f },
    { 2, NPC_XOROTHIAN_IMP, 20.9339f, 848.326f, -27.1628f, 3.66519f },
    { 2, NPC_XOROTHIAN_IMP, 29.2777f, 824.413f, -27.2753f, 3.29867f },
    { 2, NPC_XOROTHIAN_IMP, -39.7076f, 882.205f, -27.1575f, 4.7473f },
    { 2, NPC_XOROTHIAN_IMP, -82.9582f, 866.112f, -27.2077f, 5.41052f },
    { 2, NPC_XOROTHIAN_IMP, -97.5622f, 848.091f, -27.3099f, 5.75959f },
    { 2, NPC_XOROTHIAN_IMP, -105.782f, 826.821f, -27.2819f, 6.0912f },
    { 2, NPC_XOROTHIAN_IMP, -61.2501f, 747.296f, -27.126f, 1.23918f },
    { 2, NPC_XOROTHIAN_IMP, 29.5943f, 801.049f, -27.1773f, 2.96706f },
    { 2, NPC_XOROTHIAN_IMP, 5.62079f, 759.756f, -27.1948f, 2.25147f },
    { 2, NPC_XOROTHIAN_IMP, -39.7076f, 882.205f, -27.1575f, 4.7473f },
    { 2, NPC_XOROTHIAN_IMP, 5.8428f, 865.497f, -27.2486f, 3.9968f },
    { 2, NPC_XOROTHIAN_IMP, -37.7177f, 743.281f, -27.0769f, 1.5708f },
    { 2, NPC_XOROTHIAN_IMP, -105.782f, 826.821f, -27.2819f, 6.0912f },
    { 2, NPC_XOROTHIAN_IMP, -14.4863f, 747.953f, -27.1958f, 1.91986f },
    { 3, NPC_DREAD_GUARD, -82.6964f, 759.662f, -27.1815f, 0.890118f },
    { 3, NPC_DREAD_GUARD, 5.62079f, 759.756f, -27.1948f, 2.25147f },
    { 3, NPC_XOROTHIAN_IMP, -15.0206f, 878.409f, -27.0947f, 4.36332f },
    { 3, NPC_XOROTHIAN_IMP, 20.9339f, 848.326f, -27.1628f, 3.66519f },
    { 3, NPC_XOROTHIAN_IMP, 21.162f, 778.232f, -27.2357f, 2.60054f },
    { 3, NPC_XOROTHIAN_IMP, -106.235f, 800.934f, -27.3196f, 0.174533f },
    { 3, NPC_XOROTHIAN_IMP, -82.9582f, 866.112f, -27.2077f, 5.41052f },
    { 3, NPC_XOROTHIAN_IMP, -97.5622f, 848.091f, -27.3099f, 5.75959f },
    { 3, NPC_XOROTHIAN_IMP, -105.782f, 826.821f, -27.2819f, 6.0912f },
    { 3, NPC_XOROTHIAN_IMP, -37.7177f, 743.281f, -27.0769f, 1.5708f },
    { 3, NPC_XOROTHIAN_IMP, -61.2501f, 747.296f, -27.126f, 1.23918f },
    { 3, NPC_XOROTHIAN_IMP, -14.4863f, 747.953f, -27.1958f, 1.91986f },
    { 3, NPC_XOROTHIAN_IMP, -82.6964f, 759.662f, -27.1815f, 0.890118f },
    { 3, NPC_XOROTHIAN_IMP, -62.4677f, 878.058f, -27.1771f, 5.07891f },
    { 3, NPC_XOROTHIAN_IMP, 29.2777f, 824.413f, -27.2753f, 3.29867f },
    { 3, NPC_XOROTHIAN_IMP, 29.5943f, 801.049f, -27.1773f, 2.96706f },
    { 3, NPC_XOROTHIAN_IMP, -82.9582f, 866.112f, -27.2077f, 5.41052f },
    { 3, NPC_XOROTHIAN_IMP, -97.5622f, 848.091f, -27.3099f, 5.75959f },
    { 3, NPC_XOROTHIAN_IMP, -105.782f, 826.821f, -27.2819f, 6.0912f },
    { 3, NPC_XOROTHIAN_IMP, -61.2501f, 747.296f, -27.126f, 1.23918f },
    { 3, NPC_XOROTHIAN_IMP, -37.7177f, 743.281f, -27.0769f, 1.5708f },
    { 3, NPC_XOROTHIAN_IMP, -98.1468f, 778.083f, -27.254f, 0.541052f },
    { 4, NPC_DREAD_GUARD, 5.8428f, 865.497f, -27.2486f, 3.9968f },
    { 4, NPC_DREAD_GUARD, -15.0206f, 878.409f, -27.0947f, 4.36332f },
    { 4, NPC_XOROTHIAN_IMP, -39.7076f, 882.205f, -27.1575f, 4.7473f },
    { 4, NPC_XOROTHIAN_IMP, 5.62079f, 759.756f, -27.1948f, 2.25147f },
    { 4, NPC_XOROTHIAN_IMP, -14.4863f, 747.953f, -27.1958f, 1.91986f },
    { 4, NPC_XOROTHIAN_IMP, 20.9339f, 848.326f, -27.1628f, 3.66519f },
    { 4, NPC_XOROTHIAN_IMP, -106.235f, 800.934f, -27.3196f, 0.174533f },
    { 4, NPC_XOROTHIAN_IMP, -82.6964f, 759.662f, -27.1815f, 0.890118f },
    { 4, NPC_XOROTHIAN_IMP, -15.0206f, 878.409f, -27.0947f, 4.36332f },
    { 4, NPC_XOROTHIAN_IMP, -37.7177f, 743.281f, -27.0769f, 1.5708f },
    { 4, NPC_XOROTHIAN_IMP, -97.5622f, 848.091f, -27.3099f, 5.75959f },
    { 4, NPC_XOROTHIAN_IMP, 21.162f, 778.232f, -27.2357f, 2.60054f },
    { 4, NPC_XOROTHIAN_IMP, 29.2777f, 824.413f, -27.2753f, 3.29867f },
    { 4, NPC_XOROTHIAN_IMP, -82.9582f, 866.112f, -27.2077f, 5.41052f },
    { 4, NPC_XOROTHIAN_IMP, -105.782f, 826.821f, -27.2819f, 6.0912f },
    { 4, NPC_XOROTHIAN_IMP, 20.9339f, 848.326f, -27.1628f, 3.66519f },
    { 4, NPC_XOROTHIAN_IMP, -39.7076f, 882.205f, -27.1575f, 4.7473f },
    { 4, NPC_XOROTHIAN_IMP, -15.0206f, 878.409f, -27.0947f, 4.36332f },
    { 4, NPC_XOROTHIAN_IMP, 5.8428f, 865.497f, -27.2486f, 3.9968f },
    { 4, NPC_XOROTHIAN_IMP, -82.6964f, 759.662f, -27.1815f, 0.890118f },
    { 4, NPC_XOROTHIAN_IMP, 29.2777f, 824.413f, -27.2753f, 3.29867f },
    { 4, NPC_XOROTHIAN_IMP, -106.235f, 800.934f, -27.3196f, 0.174533f },
    { 5, NPC_DREAD_GUARD, -39.7076f, 882.205f, -27.1575f, 4.7473f },
    { 5, NPC_DREAD_GUARD, -37.7177f, 743.281f, -27.0769f, 1.5708f },
    { 5, NPC_XOROTHIAN_IMP, -97.5622f, 848.091f, -27.3099f, 5.75959f },
    { 5, NPC_XOROTHIAN_IMP, -98.1468f, 778.083f, -27.254f, 0.541052f },
    { 5, NPC_XOROTHIAN_IMP, -61.2501f, 747.296f, -27.126f, 1.23918f },
    { 5, NPC_XOROTHIAN_IMP, -39.7076f, 882.205f, -27.1575f, 4.7473f },
    { 5, NPC_XOROTHIAN_IMP, 5.62079f, 759.756f, -27.1948f, 2.25147f },
    { 5, NPC_XOROTHIAN_IMP, -82.6964f, 759.662f, -27.1815f, 0.890118f },
    { 5, NPC_XOROTHIAN_IMP, -105.782f, 826.821f, -27.2819f, 6.0912f },
    { 5, NPC_XOROTHIAN_IMP, 29.5943f, 801.049f, -27.1773f, 2.96706f },
    { 5, NPC_XOROTHIAN_IMP, 20.9339f, 848.326f, -27.1628f, 3.66519f },
    { 5, NPC_XOROTHIAN_IMP, -82.9582f, 866.112f, -27.2077f, 5.41052f },
    { 5, NPC_XOROTHIAN_IMP, -14.4863f, 747.953f, -27.1958f, 1.91986f },
    { 5, NPC_XOROTHIAN_IMP, 29.2777f, 824.413f, -27.2753f, 3.29867f },
    { 5, NPC_XOROTHIAN_IMP, -37.7177f, 743.281f, -27.0769f, 1.5708f },
    { 5, NPC_XOROTHIAN_IMP, -62.4677f, 878.058f, -27.1771f, 5.07891f },
    { 5, NPC_XOROTHIAN_IMP, -98.1468f, 778.083f, -27.254f, 0.541052f },
    { 5, NPC_XOROTHIAN_IMP, 5.62079f, 759.756f, -27.1948f, 2.25147f },
    { 5, NPC_XOROTHIAN_IMP, 21.162f, 778.232f, -27.2357f, 2.60054f },
    { 5, NPC_XOROTHIAN_IMP, -105.782f, 826.821f, -27.2819f, 6.0912f },
    { 5, NPC_XOROTHIAN_IMP, -97.5622f, 848.091f, -27.3099f, 5.75959f },
    { 5, NPC_XOROTHIAN_IMP, -82.6964f, 759.662f, -27.1815f, 0.890118f },
    { 6, NPC_DREAD_GUARD, -97.5622f, 848.091f, -27.3099f, 5.75959f },
    { 6, NPC_DREAD_GUARD, 29.5943f, 801.049f, -27.1773f, 2.96706f },
    { 6, NPC_DREAD_GUARD, -62.4677f, 878.058f, -27.1771f, 5.07891f },
    { 6, NPC_XOROTHIAN_IMP, -61.2501f, 747.296f, -27.126f, 1.23918f },
    { 6, NPC_XOROTHIAN_IMP, -14.4863f, 747.953f, -27.1958f, 1.91986f },
    { 6, NPC_XOROTHIAN_IMP, -39.7076f, 882.205f, -27.1575f, 4.7473f },
    { 6, NPC_XOROTHIAN_IMP, -82.9582f, 866.112f, -27.2077f, 5.41052f },
    { 6, NPC_XOROTHIAN_IMP, -106.235f, 800.934f, -27.3196f, 0.174533f },
    { 6, NPC_XOROTHIAN_IMP, -62.4677f, 878.058f, -27.1771f, 5.07891f },
    { 6, NPC_XOROTHIAN_IMP, -15.0206f, 878.409f, -27.0947f, 4.36332f },
    { 6, NPC_XOROTHIAN_IMP, 20.9339f, 848.326f, -27.1628f, 3.66519f },
    { 6, NPC_XOROTHIAN_IMP, 5.62079f, 759.756f, -27.1948f, 2.25147f },
    { 6, NPC_XOROTHIAN_IMP, -97.5622f, 848.091f, -27.3099f, 5.75959f },
    { 6, NPC_XOROTHIAN_IMP, 5.8428f, 865.497f, -27.2486f, 3.9968f },
    { 6, NPC_XOROTHIAN_IMP, 21.162f, 778.232f, -27.2357f, 2.60054f },
    { 6, NPC_XOROTHIAN_IMP, -82.9582f, 866.112f, -27.2077f, 5.41052f },
    { 6, NPC_XOROTHIAN_IMP, 29.5943f, 801.049f, -27.1773f, 2.96706f },
    { 6, NPC_XOROTHIAN_IMP, -61.2501f, 747.296f, -27.126f, 1.23918f },
    { 6, NPC_XOROTHIAN_IMP, -15.0206f, 878.409f, -27.0947f, 4.36332f },
    { 6, NPC_XOROTHIAN_IMP, -37.7177f, 743.281f, -27.0769f, 1.5708f },
    { 6, NPC_XOROTHIAN_IMP, -82.6964f, 759.662f, -27.1815f, 0.890118f },
    { 6, NPC_XOROTHIAN_IMP, -106.235f, 800.934f, -27.3196f, 0.174533f },
    { 6, NPC_XOROTHIAN_IMP, 20.9339f, 848.326f, -27.1628f, 3.66519f },
    { 7, NPC_DREAD_GUARD, 5.62079f, 759.756f, -27.1948f, 2.25147f },
    { 7, NPC_DREAD_GUARD, -106.235f, 800.934f, -27.3196f, 0.174533f },
    { 7, NPC_DREAD_GUARD, 29.5943f, 801.049f, -27.1773f, 2.96706f },
    { 7, NPC_DREAD_GUARD, -105.782f, 826.821f, -27.2819f, 6.0912f },
    { 7, NPC_XOROTHIAN_IMP, 29.2777f, 824.413f, -27.2753f, 3.29867f },
    { 7, NPC_XOROTHIAN_IMP, -39.7076f, 882.205f, -27.1575f, 4.7473f },
    { 7, NPC_XOROTHIAN_IMP, -105.782f, 826.821f, -27.2819f, 6.0912f },
    { 7, NPC_XOROTHIAN_IMP, 29.2777f, 824.413f, -27.2753f, 3.29867f },
    { 7, NPC_XOROTHIAN_IMP, -62.4677f, 878.058f, -27.1771f, 5.07891f },
    { 7, NPC_XOROTHIAN_IMP, -37.7177f, 743.281f, -27.0769f, 1.5708f },
    { 7, NPC_XOROTHIAN_IMP, 5.8428f, 865.497f, -27.2486f, 3.9968f },
    { 7, NPC_XOROTHIAN_IMP, 29.5943f, 801.049f, -27.1773f, 2.96706f },
    { 7, NPC_XOROTHIAN_IMP, 5.62079f, 759.756f, -27.1948f, 2.25147f },
    { 7, NPC_XOROTHIAN_IMP, -61.2501f, 747.296f, -27.126f, 1.23918f },
    { 7, NPC_XOROTHIAN_IMP, -15.0206f, 878.409f, -27.0947f, 4.36332f },
    { 7, NPC_XOROTHIAN_IMP, -14.4863f, 747.953f, -27.1958f, 1.91986f },
    { 7, NPC_XOROTHIAN_IMP, -39.7076f, 882.205f, -27.1575f, 4.7473f },
    { 7, NPC_XOROTHIAN_IMP, 5.62079f, 759.756f, -27.1948f, 2.25147f },
    { 7, NPC_XOROTHIAN_IMP, 5.8428f, 865.497f, -27.2486f, 3.9968f },
    { 7, NPC_XOROTHIAN_IMP, 21.162f, 778.232f, -27.2357f, 2.60054f },
    { 7, NPC_XOROTHIAN_IMP, -98.1468f, 778.083f, -27.254f, 0.541052f },
    { 7, NPC_XOROTHIAN_IMP, 20.9339f, 848.326f, -27.1628f, 3.66519f },
    { 7, NPC_XOROTHIAN_IMP, -62.4677f, 878.058f, -27.1771f, 5.07891f },
    { 7, NPC_XOROTHIAN_IMP, -61.2501f, 747.296f, -27.126f, 1.23918f },
    { 8, NPC_DREAD_GUARD, 20.9339f, 848.326f, -27.1628f, 3.66519f },
    { 8, NPC_DREAD_GUARD, 29.2777f, 824.413f, -27.2753f, 3.29867f },
    { 8, NPC_DREAD_GUARD, -14.4863f, 747.953f, -27.1958f, 1.91986f },
    { 8, NPC_DREAD_GUARD, -37.7177f, 743.281f, -27.0769f, 1.5708f },
    { 8, NPC_DREAD_GUARD, -106.235f, 800.934f, -27.3196f, 0.174533f },
    { 8, NPC_XOROTHIAN_IMP, -82.9582f, 866.112f, -27.2077f, 5.41052f },
    { 8, NPC_XOROTHIAN_IMP, -82.6964f, 759.662f, -27.1815f, 0.890118f },
    { 8, NPC_XOROTHIAN_IMP, 29.5943f, 801.049f, -27.1773f, 2.96706f },
    { 8, NPC_XOROTHIAN_IMP, -106.235f, 800.934f, -27.3196f, 0.174533f },
    { 8, NPC_XOROTHIAN_IMP, -97.5622f, 848.091f, -27.3099f, 5.75959f },
    { 8, NPC_XOROTHIAN_IMP, 21.162f, 778.232f, -27.2357f, 2.60054f },
    { 8, NPC_XOROTHIAN_IMP, 29.2777f, 824.413f, -27.2753f, 3.29867f },
    { 8, NPC_XOROTHIAN_IMP, -105.782f, 826.821f, -27.2819f, 6.0912f },
    { 8, NPC_XOROTHIAN_IMP, -98.1468f, 778.083f, -27.254f, 0.541052f },
    { 8, NPC_XOROTHIAN_IMP, 5.62079f, 759.756f, -27.1948f, 2.25147f },
    { 8, NPC_XOROTHIAN_IMP, 5.8428f, 865.497f, -27.2486f, 3.9968f },
    { 8, NPC_XOROTHIAN_IMP, 20.9339f, 848.326f, -27.1628f, 3.66519f },
    { 8, NPC_XOROTHIAN_IMP, -82.6964f, 759.662f, -27.1815f, 0.890118f },
    { 8, NPC_XOROTHIAN_IMP, -62.4677f, 878.058f, -27.1771f, 5.07891f },
    { 8, NPC_XOROTHIAN_IMP, -97.5622f, 848.091f, -27.3099f, 5.75959f },
    { 8, NPC_XOROTHIAN_IMP, -61.2501f, 747.296f, -27.126f, 1.23918f },
    { 8, NPC_XOROTHIAN_IMP, -106.235f, 800.934f, -27.3196f, 0.174533f },
    { 8, NPC_XOROTHIAN_IMP, 5.62079f, 759.756f, -27.1948f, 2.25147f },
    { 8, NPC_XOROTHIAN_IMP, 20.9339f, 848.326f, -27.1628f, 3.66519f },
    { 8, NPC_XOROTHIAN_IMP, -82.9582f, 866.112f, -27.2077f, 5.41052f },
    { 9, NPC_DREAD_GUARD, -61.2501f, 747.296f, -27.126f, 1.23918f },
    { 9, NPC_DREAD_GUARD, -62.4677f, 878.058f, -27.1771f, 5.07891f },
    { 9, NPC_DREAD_GUARD, 5.8428f, 865.497f, -27.2486f, 3.9968f },
    { 9, NPC_DREAD_GUARD, 20.9339f, 848.326f, -27.1628f, 3.66519f },
    { 9, NPC_DREAD_GUARD, -82.6964f, 759.662f, -27.1815f, 0.890118f },
    { 9, NPC_XOROTHIAN_IMP, -98.1468f, 778.083f, -27.254f, 0.541052f },
    { 9, NPC_XOROTHIAN_IMP, 29.2777f, 824.413f, -27.2753f, 3.29867f },
    { 9, NPC_XOROTHIAN_IMP, -62.4677f, 878.058f, -27.1771f, 5.07891f },
    { 9, NPC_XOROTHIAN_IMP, 29.5943f, 801.049f, -27.1773f, 2.96706f },
    { 9, NPC_XOROTHIAN_IMP, -106.235f, 800.934f, -27.3196f, 0.174533f },
    { 9, NPC_XOROTHIAN_IMP, -61.2501f, 747.296f, -27.126f, 1.23918f },
    { 9, NPC_XOROTHIAN_IMP, -105.782f, 826.821f, -27.2819f, 6.0912f },
    { 9, NPC_XOROTHIAN_IMP, 5.62079f, 759.756f, -27.1948f, 2.25147f },
    { 9, NPC_XOROTHIAN_IMP, 5.8428f, 865.497f, -27.2486f, 3.9968f },
    { 9, NPC_XOROTHIAN_IMP, 20.9339f, 848.326f, -27.1628f, 3.66519f },
    { 9, NPC_XOROTHIAN_IMP, -15.0206f, 878.409f, -27.0947f, 4.36332f },
    { 9, NPC_XOROTHIAN_IMP, -82.9582f, 866.112f, -27.2077f, 5.41052f },
    { 9, NPC_XOROTHIAN_IMP, -39.7076f, 882.205f, -27.1575f, 4.7473f },
    { 9, NPC_XOROTHIAN_IMP, -14.4863f, 747.953f, -27.1958f, 1.91986f },
    { 9, NPC_XOROTHIAN_IMP, -37.7177f, 743.281f, -27.0769f, 1.5708f },
    { 9, NPC_XOROTHIAN_IMP, 5.8428f, 865.497f, -27.2486f, 3.9968f },
    { 9, NPC_XOROTHIAN_IMP, -82.6964f, 759.662f, -27.1815f, 0.890118f },
};

struct npc_warlock_mount_ritualAI : public  Scripted_NoMovementAI
{
    npc_warlock_mount_ritualAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        m_pInstance = (instance_dire_maul*)pCreature->GetInstanceData();
        Reset();
    }

    instance_dire_maul* m_pInstance;
    uint8 m_uiPhase;
    uint32 m_uiPhaseTimer;
    GuidList m_luiSummonedMobGUIDs;

    void Reset() override
    {
        m_uiPhase = 0;
        m_uiPhaseTimer = 50000;
    }

    void DoSummonPack(uint8 uiIndex)
    {
        for (uint8 i = 0; i < MAX_SUMMON_POSITIONS; ++i)
        {
            if (asSummonInfo[i].uiPosition > uiIndex)
                break;
            if (asSummonInfo[i].uiPosition == uiIndex)
                m_creature->SummonCreature(asSummonInfo[i].uiEntry, asSummonInfo[i].fX, asSummonInfo[i].fY, asSummonInfo[i].fZ, asSummonInfo[i].fO, TEMPSUMMON_DEAD_DESPAWN, 0);
        }
        m_pInstance->DoRespawnGameObject(m_pInstance->GetRitualSymbolGuids().at(uiIndex - 1), 900);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        switch (pSummoned->GetEntry())
        {
        case NPC_XOROTHIAN_IMP:
        case NPC_DREAD_GUARD:
            m_luiSummonedMobGUIDs.push_back(pSummoned->GetObjectGuid());
            break;
        }
    }

    void UpdateAI(uint32 const uiDiff) override
    {
        if (m_uiPhaseTimer <= uiDiff)
        {
            switch (m_uiPhase)
            {
            case 1:
                m_pInstance->SetData(TYPE_RITUAL, IN_PROGRESS);
                m_uiPhaseTimer = 2000;
                break;
            case 2:
                m_uiPhaseTimer = 40000;
                DoSummonPack(1);
                break;
            case 3:
                m_uiPhaseTimer = 40000;
                DoSummonPack(2);
                break;
            case 4:
                m_uiPhaseTimer = 40000;
                DoSummonPack(3);
                break;
            case 5:
                m_uiPhaseTimer = 40000;
                DoSummonPack(4);
                break;
            case 6:
                m_uiPhaseTimer = 40000;
                DoSummonPack(5);
                break;
            case 7:
                m_uiPhaseTimer = 40000;
                DoSummonPack(6);
                break;
            case 8:
                m_uiPhaseTimer = 40000;
                DoSummonPack(7);
                break;
            case 9:
                m_uiPhaseTimer = 40000;
                DoSummonPack(8);
                break;
            case 10:
                m_uiPhaseTimer = 40000;
                DoSummonPack(9);
                break;
            case 11:
                for (auto demon : m_luiSummonedMobGUIDs)
                {
                    if (Creature* pSummoned = m_creature->GetMap()->GetCreature(demon))
                    {
                        pSummoned->CastSpell(pSummoned, SPELL_TELEPORT, true);
                        DoScriptText(SAY_UNSUMMON_DEMON, pSummoned);
                        pSummoned->ForcedDespawn(1000);
                    }
                }
                m_luiSummonedMobGUIDs.clear();
                if (GameObject* pGo = m_pInstance->GetSingleGameObjectFromStorage(GO_WARLOCK_RITUAL_CIRCLE))
                    pGo->UseDoorOrButton();
                m_pInstance->SetData(TYPE_RITUAL, DONE);
                m_creature->ForcedDespawn();
            }
            ++m_uiPhase;
        }
        else
            m_uiPhaseTimer -= uiDiff;
    }
};

CreatureAI* GetAI_npc_warlock_mount_ritual(Creature* pCreature)
{
    return new npc_warlock_mount_ritualAI(pCreature);
}

InstanceData* GetInstanceData_instance_dire_maul(Map* pMap)
{
    return new instance_dire_maul(pMap);
}

void AddSC_instance_dire_maul()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_dire_maul";
    pNewScript->GetInstanceData = &GetInstanceData_instance_dire_maul;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_warlock_mount_ritual";
    pNewScript->GetAI = &GetAI_npc_warlock_mount_ritual;
    pNewScript->RegisterSelf();
}
