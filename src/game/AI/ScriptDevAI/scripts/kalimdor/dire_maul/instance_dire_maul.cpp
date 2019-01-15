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
SD%Complete: 70
SDComment: Ogre costume suit missing for Tribute Run, Cho'Rush spells randomisation is not handled properly, Warpwood pods are not implemented, the Maul event is not handled
SDCategory: Dire Maul
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "dire_maul.h"

instance_dire_maul::instance_dire_maul(Map* pMap) : ScriptedInstance(pMap),
    m_bWallDestroyed(false),
    m_uiDreadsteedEventTimer(0),
    m_bDoNorthBeforeWest(false)
{
    Initialize();
}

void instance_dire_maul::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
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
                pCreature->SetFactionTemporary(FACTION_HOSTILE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_IMMUNE_TO_PLAYER);
            break;
        case NPC_ARCANE_ABERRATION:
        case NPC_MANA_REMNANT:
            m_lGeneratorGuardGUIDs.push_back(pCreature->GetObjectGuid());
            return;
        case NPC_IMMOLTHAR:
        case NPC_WARLOCK_DUMMY_INFERNAL:
            break;
        case NPC_HIGHBORNE_SUMMONER:
            m_luiHighborneSummonerGUIDs.push_back(pCreature->GetObjectGuid());
            return;

        // North
        case NPC_CHORUSH:
        case NPC_KING_GORDOK:
        case NPC_CAPTAIN_KROMCRUSH:
        case NPC_GUARD_SLIPKIK:
            break;

        default:
            return;
    }
    m_npcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
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
            // exclude the central portal
            if (pGo->GetPositionZ() > -29.0f)
                m_lDreadsteedPortalsGUIDs.push_back(pGo->GetObjectGuid());
            return;
        case GO_WARLOCK_RITUAL_CIRCLE:
            break;

        // North
        case GO_NORTH_LIBRARY_DOOR:
            break;

        default:
            return;
    }
    m_goEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
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
                    pPrince->SetFactionTemporary(FACTION_HOSTILE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_TOGGLE_IMMUNE_TO_PLAYER);
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
        case TYPE_DREADSTEED:
            // start timer
            if (uiData == IN_PROGRESS)
                m_uiDreadsteedEventTimer = 390000;
            else if (uiData == SPECIAL)
            {
                // set animation for next stage
                if (GameObject* pCircle = GetSingleGameObjectFromStorage(GO_WARLOCK_RITUAL_CIRCLE))
                    pCircle->SetGoState(GO_STATE_ACTIVE);

                // despawn the controller; Inform the attackers to teleport
                if (Creature* pDummy = GetSingleCreatureFromStorage(NPC_WARLOCK_DUMMY_INFERNAL))
                {
                    pDummy->AI()->SendAIEventAround(AI_EVENT_CUSTOM_EVENTAI_A, pDummy, 4000, 100.0f);
                    pDummy->ForcedDespawn(5000);
                }

                // despawn side portals
                for (GuidList::const_iterator itr = m_lDreadsteedPortalsGUIDs.begin(); itr != m_lDreadsteedPortalsGUIDs.end(); ++itr)
                {
                    if (GameObject* pGo = instance->GetGameObject(*itr))
                        pGo->SetLootState(GO_JUST_DEACTIVATED);
                }
            }
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
                    if (Creature* pMizzle = pOgre->SummonCreature(NPC_MIZZLE_THE_CRAFTY, afMizzleSpawnLoc[0], afMizzleSpawnLoc[1], afMizzleSpawnLoc[2], afMizzleSpawnLoc[3], TEMPSPAWN_DEAD_DESPAWN, 0, true))
                    {
                        pMizzle->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
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
        case TYPE_CHORUSH:
        case TYPE_STOMPER_KREEG:
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
                      << m_auiEncounter[15] << " " << m_auiEncounter[16] << " " << m_auiEncounter[17] << " "
                      << m_auiEncounter[18];

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
        case NPC_CHORUSH:
            SetData(TYPE_CHORUSH, DONE);
            break;
        case NPC_STOMPER_KREEG:
            SetData(TYPE_STOMPER_KREEG, DONE);
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
               m_auiEncounter[15] >> m_auiEncounter[16] >> m_auiEncounter[17] >>
               m_auiEncounter[18];

    if (m_auiEncounter[TYPE_ALZZIN] >= DONE)
        m_bWallDestroyed = true;

    for (uint32& i : m_auiEncounter)
    {
        if (i == IN_PROGRESS)
            i = NOT_STARTED;
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
        case INSTANCE_CONDITION_ID_HARD_MODE_4:             // Four guards alive
        case INSTANCE_CONDITION_ID_HARD_MODE_5:             // Five guards alive
        case INSTANCE_CONDITION_ID_HARD_MODE_6:             // Six guards alive
        {
            uint8 uiTributeRunAliveBosses = (GetData(TYPE_MOLDAR) != DONE ? 1 : 0) + (GetData(TYPE_FENGUS) != DONE ? 1 : 0) + (GetData(TYPE_SLIPKIK) != DONE ? 1 : 0)
                                            + (GetData(TYPE_KROMCRUSH) != DONE ? 1 : 0) + (GetData(TYPE_CHORUSH) != DONE ? 1 : 0) + (GetData(TYPE_STOMPER_KREEG) != DONE ? 1 : 0);

            return uiInstanceConditionId == uiTributeRunAliveBosses;
        }
    }

    script_error_log("instance_dire_maul::CheckConditionCriteriaMeet called with unsupported Id %u. Called with param plr %s, src %s, condition source type %u",
                     uiInstanceConditionId, pPlayer ? pPlayer->GetGuidStr().c_str() : "nullptr", pConditionSource ? pConditionSource->GetGuidStr().c_str() : "nullptr", conditionSourceType);
    return false;
}

bool instance_dire_maul::CheckAllGeneratorsDestroyed()
{
    return !(m_auiEncounter[TYPE_PYLON_1] != DONE || m_auiEncounter[TYPE_PYLON_2] != DONE || m_auiEncounter[TYPE_PYLON_3] != DONE || m_auiEncounter[TYPE_PYLON_4] != DONE || m_auiEncounter[TYPE_PYLON_5] != DONE);
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

void instance_dire_maul::Update(uint32 uiDiff)
{
    if (m_uiDreadsteedEventTimer)
    {
        if (m_uiDreadsteedEventTimer <= uiDiff)
        {
            // set encounter to special to allow the next event to proceed
            SetData(TYPE_DREADSTEED, SPECIAL);
            m_uiDreadsteedEventTimer = 0;
        }
        else
            m_uiDreadsteedEventTimer -= uiDiff;
    }
}

InstanceData* GetInstanceData_instance_dire_maul(Map* pMap)
{
    return new instance_dire_maul(pMap);
}

/*###############
## go_fixed_trap
################*/

struct go_ai_fixed_trap : public GameObjectAI
{
    go_ai_fixed_trap(GameObject* go) : GameObjectAI(go) {}

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_go->IsSpawned())
        {
            // Sniffs show that trap should only react to Guard Slip'kik
            // He then enters evade mode and becomes an invalid target to hostile actions from players
            // Faction is changed to 14 and restored by spell 22799 (King of the Gordok)
            // Additionnaly, Guard Slip'kik should get UnitFlags 16 but purpose is unknown so we skip it for now
            if (Creature* slipkik = GetClosestCreatureWithEntry(m_go, NPC_GUARD_SLIPKIK, 0.5f))
            {
                m_go->Use(slipkik);
                DoScriptText(SAY_SLIPKIK_TRAP, slipkik);
                slipkik->AI()->EnterEvadeMode();
                slipkik->SetImmuneToPlayer(true);
                slipkik->SetFactionTemporary(FACTION_HOSTILE, TEMPFACTION_RESTORE_RESPAWN);
                m_go->SetLootState(GO_JUST_DEACTIVATED);    // Despawn the trap
            }
        }
    }
};

GameObjectAI* GetAI_go_fixed_trap(GameObject* go)
{
    return new go_ai_fixed_trap(go);
}

/*####################################
## Guard Slip'kik Trigger dummy effect
####################################*/

bool EffectDummyCreature_spell_guard_slip_kik(Unit* pCaster, uint32 uiSpellId, SpellEffectIndex /* uiEffIndex */, Creature* /* pCreatureTarget */, ObjectGuid /*originalCasterGuid*/)
{
    if (uiSpellId == SPELL_GUARD_SLIPKIK_TRIGGER)
    {
        instance_dire_maul* pInstance = (instance_dire_maul*)pCaster->GetInstanceData();
        if (pInstance)
        {
            if (Creature* slipkik = pInstance->GetSingleCreatureFromStorage(NPC_GUARD_SLIPKIK))
                slipkik->setFaction(FACTION_OGRE);
            return true;
        }
    }
    return false;
}

void AddSC_instance_dire_maul()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "instance_dire_maul";
    pNewScript->GetInstanceData = &GetInstanceData_instance_dire_maul;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_fixed_trap";
    pNewScript->GetGameObjectAI = &GetAI_go_fixed_trap;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_mizzle_crafty";
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_spell_guard_slip_kik;
    pNewScript->RegisterSelf();
}
