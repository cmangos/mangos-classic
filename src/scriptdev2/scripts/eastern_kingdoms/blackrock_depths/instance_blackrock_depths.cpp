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
SDName: Instance_Blackrock_Depths
SD%Complete: 80
SDComment:
SDCategory: Blackrock Depths
EndScriptData */

#include "precompiled.h"
#include "blackrock_depths.h"

instance_blackrock_depths::instance_blackrock_depths(Map* pMap) : ScriptedInstance(pMap),
    m_uiBarAleCount(0),
    m_uiBrokenKegs(0),
    m_uiCofferDoorsOpened(0),
    m_uiDwarfRound(0),
    m_uiDwarfFightTimer(0),
    m_uiPatronEmoteTimer(2000),
    m_uiPatrolTimer(0),
    m_uiStolenAles(0),
    m_uiDagranTimer(0),

    m_fArenaCenterX(0.0f),
    m_fArenaCenterY(0.0f),
    m_fArenaCenterZ(0.0f),
    m_bIsBarDoorOpen(false)
{
    Initialize();
}

void instance_blackrock_depths::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
}

void instance_blackrock_depths::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_PRINCESS:
            // replace the princess if required
            if (CanReplacePrincess())
                pCreature->UpdateEntry(NPC_PRIESTESS);
            // no break;
        case NPC_EMPEROR:
        case NPC_PHALANX:
        case NPC_PLUGGER_SPAZZRING:
        case NPC_HATEREL:
        case NPC_ANGERREL:
        case NPC_VILEREL:
        case NPC_GLOOMREL:
        case NPC_SEETHREL:
        case NPC_DOOMREL:
        case NPC_DOPEREL:
        case NPC_SHILL:
        case NPC_CREST:
        case NPC_JAZ:
        case NPC_TOBIAS:
        case NPC_DUGHAL:
        case NPC_LOREGRAIN:
        case NPC_RIBBLY_SCREWSPIGGOT:
            m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;
        case NPC_WARBRINGER_CONST:
            // Golems not in the Relict Vault?
            if (std::abs(pCreature->GetPositionZ() - aVaultPositions[2]) > 1.0f || !pCreature->IsWithinDist2d(aVaultPositions[0], aVaultPositions[1], 20.0f))
                break;
            // Golems in Relict Vault need to have a stoned aura, set manually to prevent reapply when reached home
            pCreature->CastSpell(pCreature, SPELL_STONED, true);
            // Store the Relict Vault Golems into m_sVaultNpcGuids
        case NPC_WATCHER_DOOMGRIP:
            m_sVaultNpcGuids.insert(pCreature->GetObjectGuid());
            break;
            // Arena crowd
        case NPC_ARENA_SPECTATOR:
        case NPC_SHADOWFORGE_PEASANT:
        case NPC_SHADOWFORGE_CITIZEN:
        case NPC_SHADOWFORGE_SENATOR:
        case NPC_ANVILRAGE_SOLDIER:
        case NPC_ANVILRAGE_MEDIC:
        case NPC_ANVILRAGE_OFFICER:
            if (pCreature->GetPositionZ() < aArenaCrowdVolume->m_fCenterZ || pCreature->GetPositionZ() > aArenaCrowdVolume->m_fCenterZ + aArenaCrowdVolume->m_uiHeight ||
                    !pCreature->IsWithinDist2d(aArenaCrowdVolume->m_fCenterX, aArenaCrowdVolume->m_fCenterY, aArenaCrowdVolume->m_uiRadius))
                break;
            m_sArenaCrowdNpcGuids.insert(pCreature->GetObjectGuid());
            if (m_auiEncounter[0] == DONE)
                pCreature->SetFactionTemporary(FACTION_ARENA_NEUTRAL, TEMPFACTION_RESTORE_RESPAWN);
            break;
            // Grim Guzzler bar crowd
        case NPC_GRIM_PATRON:
        case NPC_GUZZLING_PATRON:
        case NPC_HAMMERED_PATRON:
            m_sBarPatronNpcGuids.insert(pCreature->GetObjectGuid());
            if (m_auiEncounter[11] == DONE)
                pCreature->SetFactionTemporary(FACTION_DARK_IRON, TEMPFACTION_RESTORE_RESPAWN);
                pCreature->SetStandState(UNIT_STAND_STATE_STAND);
            break;
        case NPC_PRIVATE_ROCKNOT:
        case NPC_MISTRESS_NAGMARA:
            if (m_auiEncounter[11] == DONE)
                pCreature->ForcedDespawn();
            else
                m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;
    }
}

void instance_blackrock_depths::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
        case GO_ARENA_1:
        case GO_ARENA_2:
        case GO_ARENA_3:
        case GO_ARENA_4:
        case GO_SHADOW_LOCK:
        case GO_SHADOW_MECHANISM:
        case GO_SHADOW_GIANT_DOOR:
        case GO_SHADOW_DUMMY:
        case GO_BAR_KEG_SHOT:
        case GO_BAR_KEG_TRAP:
        case GO_TOMB_ENTER:
        case GO_TOMB_EXIT:
        case GO_LYCEUM:
        case GO_GOLEM_ROOM_N:
        case GO_GOLEM_ROOM_S:
        case GO_THRONE_ROOM:
        case GO_SPECTRAL_CHALICE:
        case GO_CHEST_SEVEN:
        case GO_ARENA_SPOILS:
        case GO_SECRET_DOOR:
        case GO_SECRET_SAFE:
        case GO_JAIL_DOOR_SUPPLY:
        case GO_JAIL_SUPPLY_CRATE:
        case GO_DWARFRUNE_A01:
        case GO_DWARFRUNE_B01:
        case GO_DWARFRUNE_C01:
        case GO_DWARFRUNE_D01:
        case GO_DWARFRUNE_E01:
        case GO_DWARFRUNE_F01:
        case GO_DWARFRUNE_G01:
            break;
        case GO_BAR_DOOR:
            if (GetData(TYPE_ROCKNOT) == DONE)
            {
                // Rocknot event done: set the Grim Guzzler door animation to "broken"
                // tell the instance script it is open to prevent some of the other events
                pGo->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);
                SetBarDoorIsOpen();
            }
            else if (m_auiEncounter[10] == DONE || m_auiEncounter[11] == DONE)
            {
                // bar or Plugger event done: open the Grim Guzzler door
                // tell the instance script it is open to prevent some of the other events
                DoUseDoorOrButton(GO_BAR_DOOR);
                SetBarDoorIsOpen();
            }
            break;

        default:
            return;
    }
    m_mGoEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
}

void instance_blackrock_depths::SetData(uint32 uiType, uint32 uiData)
{
    switch (uiType)
    {
        case TYPE_RING_OF_LAW:
            // If finished the arena event after theldren fight
            if (uiData == DONE && m_auiEncounter[0] == SPECIAL)
                DoRespawnGameObject(GO_ARENA_SPOILS, HOUR);
            else if (uiData == DONE)
            {
                for (GuidSet::const_iterator itr = m_sArenaCrowdNpcGuids.begin(); itr != m_sArenaCrowdNpcGuids.end(); ++itr)
                {
                    if (Creature* pSpectator = instance->GetCreature(*itr))
                        pSpectator->SetFactionTemporary(FACTION_ARENA_NEUTRAL, TEMPFACTION_RESTORE_RESPAWN);
                }
            }
            m_auiEncounter[0] = uiData;
            break;
        case TYPE_VAULT:
            if (uiData == SPECIAL)
            {
                ++m_uiCofferDoorsOpened;

                if (m_uiCofferDoorsOpened == MAX_RELIC_DOORS)
                {
                    SetData(TYPE_VAULT, IN_PROGRESS);

                    Creature* pConstruct = nullptr;

                    // Activate vault constructs
                    for (GuidSet::const_iterator itr = m_sVaultNpcGuids.begin(); itr != m_sVaultNpcGuids.end(); ++itr)
                    {
                        pConstruct = instance->GetCreature(*itr);
                        if (pConstruct)
                            pConstruct->RemoveAurasDueToSpell(SPELL_STONED);
                    }

                    if (!pConstruct)
                        return;

                    // Summon doomgrip
                    pConstruct->SummonCreature(NPC_WATCHER_DOOMGRIP, aVaultPositions[0], aVaultPositions[1], aVaultPositions[2], aVaultPositions[3], TEMPSUMMON_DEAD_DESPAWN, 0);
                }
                // No need to store in this case
                return;
            }
            if (uiData == DONE)
            {
                DoUseDoorOrButton(GO_SECRET_DOOR);
                DoToggleGameObjectFlags(GO_SECRET_SAFE, GO_FLAG_NO_INTERACT, false);
            }
            m_auiEncounter[1] = uiData;
            break;
        case TYPE_ROCKNOT:
            if (uiData == SPECIAL)
                ++m_uiBarAleCount;
            else
            {
                if (uiData == DONE)
                {
                    HandleBarPatrons(PATRON_PISSED);
                    SetBarDoorIsOpen();
                }
                m_auiEncounter[2] = uiData;
            }
            break;
        case TYPE_TOMB_OF_SEVEN:
            // Don't set the same data twice
            if (uiData == m_auiEncounter[3])
                break;
            // Combat door
            DoUseDoorOrButton(GO_TOMB_ENTER);
            // Start the event
            if (uiData == IN_PROGRESS)
                DoCallNextDwarf();
            if (uiData == FAIL)
            {
                // Reset dwarfes
                for (uint8 i = 0; i < MAX_DWARFS; ++i)
                {
                    if (Creature* pDwarf = GetSingleCreatureFromStorage(aTombDwarfes[i]))
                    {
                        if (!pDwarf->isAlive())
                            pDwarf->Respawn();
                    }
                }

                m_uiDwarfRound = 0;
                m_uiDwarfFightTimer = 0;
            }
            if (uiData == DONE)
            {
                DoRespawnGameObject(GO_CHEST_SEVEN, HOUR);
                DoUseDoorOrButton(GO_TOMB_EXIT);
            }
            m_auiEncounter[3] = uiData;
            break;
        case TYPE_LYCEUM:
            if (uiData == DONE)
            {
                DoUseDoorOrButton(GO_GOLEM_ROOM_N);
                DoUseDoorOrButton(GO_GOLEM_ROOM_S);
            }
            m_auiEncounter[4] = uiData;
            break;
        case TYPE_IRON_HALL:
            switch (uiData)
            {
                case IN_PROGRESS:
                    DoUseDoorOrButton(GO_GOLEM_ROOM_N);
                    DoUseDoorOrButton(GO_GOLEM_ROOM_S);
                    break;
                case FAIL:
                    DoUseDoorOrButton(GO_GOLEM_ROOM_N);
                    DoUseDoorOrButton(GO_GOLEM_ROOM_S);
                    break;
                case DONE:
                    DoUseDoorOrButton(GO_GOLEM_ROOM_N);
                    DoUseDoorOrButton(GO_GOLEM_ROOM_S);
                    DoUseDoorOrButton(GO_THRONE_ROOM);
                    break;
            }
            m_auiEncounter[5] = uiData;
            break;
        case TYPE_QUEST_JAIL_BREAK:
            m_auiEncounter[6] = uiData;
            return;
        case TYPE_FLAMELASH:
            for (int i = 0; i < MAX_DWARF_RUNES; ++i)
                DoUseDoorOrButton(GO_DWARFRUNE_A01 + i);
            return;
        case TYPE_HURLEY:
            if (uiData == SPECIAL)
            {
                ++m_uiBrokenKegs;

                if (m_uiBrokenKegs == 3)
                {
                    if (Creature* pPlugger = GetSingleCreatureFromStorage(NPC_PLUGGER_SPAZZRING))
                    {
                        // Summon Hurley Blackbreath
                        Creature* pHurley = pPlugger->SummonCreature(NPC_HURLEY_BLACKBREATH, aHurleyPositions[0], aHurleyPositions[1], aHurleyPositions[2], aHurleyPositions[3], TEMPSUMMON_DEAD_DESPAWN, 0);

                        if (!pHurley)
                            return;

                        // Summon cronies around Hurley
                        for (uint8 i = 0; i < MAX_CRONIES; ++i)
                        {
                            float fX, fY, fZ;
                            pPlugger->GetRandomPoint(aHurleyPositions[0], aHurleyPositions[1], aHurleyPositions[2], 2.0f, fX, fY, fZ);
                            if (Creature* pSummoned = pPlugger->SummonCreature(NPC_BLACKBREATH_CRONY, fX, fY, fZ, aHurleyPositions[3], TEMPSUMMON_DEAD_DESPAWN, 0))
                            {
                                pSummoned->SetWalk(false);
                                // The cronies should not engage anyone until their boss does so
                                // the linking is done by DB
                                pSummoned->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
                                // The movement toward the kegs is handled by Hurley EscortAI
                                // and we want the cronies to follow him there
                                pSummoned->GetMotionMaster()->MoveFollow(pHurley, 1.0f, 0);
                            }
                        }
                        SetData(TYPE_HURLEY, IN_PROGRESS);
                    }
                }
            }
            else
                m_auiEncounter[8] = uiData;
            break;
        case TYPE_BRIDGE:
            m_auiEncounter[9] = uiData;
            return;
        case TYPE_BAR:
            if (uiData == IN_PROGRESS)
                HandleBarPatrol(0);
            m_auiEncounter[10] = uiData;
            break;
        case TYPE_PLUGGER:
            if (uiData == SPECIAL)
            {
                if (Creature* pPlugger = GetSingleCreatureFromStorage(NPC_PLUGGER_SPAZZRING))
                {
                    ++m_uiStolenAles;
                    if (m_uiStolenAles == 3)
                        uiData = IN_PROGRESS;
                }
            }
            m_auiEncounter[11] = uiData;
            break;
        case TYPE_NAGMARA:
            m_auiEncounter[12] = uiData;
            break;
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " "
                   << m_auiEncounter[3] << " " << m_auiEncounter[4] << " " << m_auiEncounter[5] << " "
                   << m_auiEncounter[6] << " " << m_auiEncounter[7] << " " << m_auiEncounter[8] << " "
                   << m_auiEncounter[9] << " " << m_auiEncounter[10] << " " << m_auiEncounter[11] << " "
                   << m_auiEncounter[12];

        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

uint32 instance_blackrock_depths::GetData(uint32 uiType) const
{
    switch (uiType)
    {
        case TYPE_RING_OF_LAW:
            return m_auiEncounter[0];
        case TYPE_VAULT:
            return m_auiEncounter[1];
        case TYPE_ROCKNOT:
            if (m_auiEncounter[2] == IN_PROGRESS && m_uiBarAleCount == 3)
                return SPECIAL;
            else
                return m_auiEncounter[2];
        case TYPE_TOMB_OF_SEVEN:
            return m_auiEncounter[3];
        case TYPE_LYCEUM:
            return m_auiEncounter[4];
        case TYPE_IRON_HALL:
            return m_auiEncounter[5];
        case TYPE_QUEST_JAIL_BREAK:
            return m_auiEncounter[6];
        case TYPE_FLAMELASH:
            return m_auiEncounter[7];
        case TYPE_HURLEY:
            return m_auiEncounter[8];
        case TYPE_BRIDGE:
            return m_auiEncounter[9];
        case TYPE_BAR:
            return m_auiEncounter[10];
        case TYPE_PLUGGER:
            return m_auiEncounter[11];
        case TYPE_NAGMARA:
            return m_auiEncounter[12];
        default:
            return 0;
    }
}

void instance_blackrock_depths::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3]
               >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6] >> m_auiEncounter[7]
               >> m_auiEncounter[8] >> m_auiEncounter[9] >> m_auiEncounter[10] >> m_auiEncounter[11]
               >> m_auiEncounter[12];

    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
        if (m_auiEncounter[i] == IN_PROGRESS && i != TYPE_IRON_HALL) // specific check for Iron Hall event: once started, it never stops, the Ironhall Guardians switch to flamethrower mode and never stop even after event completion, i.e. the event remains started if Magmus resets
            m_auiEncounter[i] = NOT_STARTED;

    OUT_LOAD_INST_DATA_COMPLETE;
}

void instance_blackrock_depths::OnCreatureEnterCombat(Creature* pCreature)
{
    if (pCreature->GetEntry() == NPC_MAGMUS)
        SetData(TYPE_IRON_HALL, IN_PROGRESS);
}

void instance_blackrock_depths::OnCreatureEvade(Creature* pCreature)
{
    if (GetData(TYPE_RING_OF_LAW) == IN_PROGRESS || GetData(TYPE_RING_OF_LAW) == SPECIAL)
    {
        for (uint8 i = 0; i < countof(aArenaNPCs); ++i)
        {
            if (pCreature->GetEntry() == aArenaNPCs[i])
            {
                SetData(TYPE_RING_OF_LAW, FAIL);
                return;
            }
        }
    }

    switch (pCreature->GetEntry())
    {
            // Handle Tomb of the Seven reset in case of wipe
        case NPC_HATEREL:
        case NPC_ANGERREL:
        case NPC_VILEREL:
        case NPC_GLOOMREL:
        case NPC_SEETHREL:
        case NPC_DOPEREL:
        case NPC_DOOMREL:
            SetData(TYPE_TOMB_OF_SEVEN, FAIL);
            break;
        case NPC_MAGMUS:
            SetData(TYPE_IRON_HALL, FAIL);
            break;
    }
}

void instance_blackrock_depths::OnCreatureDeath(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_WARBRINGER_CONST:
        case NPC_WATCHER_DOOMGRIP:
            if (GetData(TYPE_VAULT) == IN_PROGRESS)
            {
                m_sVaultNpcGuids.erase(pCreature->GetObjectGuid());

                // If all event npcs dead then set event to done
                if (m_sVaultNpcGuids.empty())
                    SetData(TYPE_VAULT, DONE);
            }
            break;
        case NPC_OGRABISI:
        case NPC_SHILL:
        case NPC_CREST:
        case NPC_JAZ:
            if (GetData(TYPE_QUEST_JAIL_BREAK) == IN_PROGRESS)
                SetData(TYPE_QUEST_JAIL_BREAK, SPECIAL);
            break;
            // Handle Tomb of the Seven dwarf death event
        case NPC_HATEREL:
        case NPC_ANGERREL:
        case NPC_VILEREL:
        case NPC_GLOOMREL:
        case NPC_SEETHREL:
        case NPC_DOPEREL:
            // Only handle the event when event is in progress
            if (GetData(TYPE_TOMB_OF_SEVEN) != IN_PROGRESS)
                return;
            // Call the next dwarf only if it's the last one which joined the fight
            if (pCreature->GetEntry() == aTombDwarfes[m_uiDwarfRound - 1])
                DoCallNextDwarf();
            break;
        case NPC_DOOMREL:
            SetData(TYPE_TOMB_OF_SEVEN, DONE);
            break;
        case NPC_MAGMUS:
            SetData(TYPE_IRON_HALL, DONE);
            break;
        case NPC_HURLEY_BLACKBREATH:
            SetData(TYPE_HURLEY, DONE);
            break;
        case NPC_RIBBLY_SCREWSPIGGOT:
            // Do nothing if the patrol was already spawned or is about to:
            // Plugger has made the bar hostile
            if (GetData(TYPE_BAR) == IN_PROGRESS || GetData(TYPE_PLUGGER) == IN_PROGRESS || GetData(TYPE_BAR) == DONE || GetData(TYPE_PLUGGER) == DONE)
                return;
            else
                SetData(TYPE_BAR, IN_PROGRESS);
            break;
        case NPC_SHADOWFORGE_SENATOR:
            // Emperor Dagran Thaurissan performs a random yell upon the death
            // of Shadowforge Senators in the Throne Room
            if (Creature* pDagran = GetSingleCreatureFromStorage(NPC_EMPEROR))
            {
                uint32 uiTextId;

                if (!pDagran->isAlive())
                    return;

                if (m_uiDagranTimer > 0)
                    return;

                switch (urand(0, 3))
                {
                    case 0: uiTextId = YELL_SENATOR_1; break;
                    case 1: uiTextId = YELL_SENATOR_2; break;
                    case 2: uiTextId = YELL_SENATOR_3; break;
                    case 3: uiTextId = YELL_SENATOR_4; break;
                }
                DoScriptText(uiTextId, pDagran);
                m_uiDagranTimer = 30000;    // set a timer of 30 sec to avoid Emperor Thaurissan to spam yells in case many senators are killed in a short amount of time
            }
            break;
    }
}

void instance_blackrock_depths::DoCallNextDwarf()
{
    if (Creature* pDwarf = GetSingleCreatureFromStorage(aTombDwarfes[m_uiDwarfRound]))
    {
        if (Player* pPlayer = GetPlayerInMap())
        {
            pDwarf->SetFactionTemporary(FACTION_DWARF_HOSTILE, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_RESTORE_REACH_HOME);
            pDwarf->AI()->AttackStart(pPlayer);
        }
    }
    m_uiDwarfFightTimer = 30000;
    ++m_uiDwarfRound;
}

// function that replaces the princess if requirements are met
bool instance_blackrock_depths::CanReplacePrincess()
{
    Map::PlayerList const& players = instance->GetPlayers();
    if (players.isEmpty())
        return false;

    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
    {
        if (Player* pPlayer = itr->getSource())
        {
            // if at least one player didn't complete the quest, return false
            if ((pPlayer->GetTeam() == ALLIANCE && !pPlayer->GetQuestRewardStatus(QUEST_FATE_KINGDOM))
                    || (pPlayer->GetTeam() == HORDE && !pPlayer->GetQuestRewardStatus(QUEST_ROYAL_RESCUE)))
                return false;
        }
    }

    return true;
}

void instance_blackrock_depths::HandleBarPatrons(uint8 uiEventType)
{
    switch (uiEventType)
    {
        // case for periodical handle of random emotes
        case PATRON_EMOTE:
            if (GetData(TYPE_PLUGGER) == DONE)
                return;

            for (GuidSet::const_iterator itr = m_sBarPatronNpcGuids.begin(); itr != m_sBarPatronNpcGuids.end(); ++itr)
            {
                 // About 5% of patrons do emote at a given time
                // So avoid executing follow up code for the 95% others
                if (urand(0, 100) < 4)
                {
                    // Only three emotes are seen in data: laugh, cheer and exclamation
                    // the last one appearing the least and the first one appearing the most
                    // emotes are stored in a table and frequency is handled there
                    if (Creature* pPatron = instance->GetCreature(*itr))
                       pPatron->HandleEmote(aPatronsEmotes[urand(0, 5)]);
                }
            }
            return;
        // case for Rocknot event when breaking the barrel
        case PATRON_PISSED:
            // Three texts are said, one less often than the two others
            // Only by patrons near the broken barrel react to Rocknot's rampage
            if (GameObject* pGo = GetSingleGameObjectFromStorage(GO_BAR_KEG_SHOT))
            {
                for (GuidSet::const_iterator itr = m_sBarPatronNpcGuids.begin(); itr != m_sBarPatronNpcGuids.end(); ++itr)
                {
                    if (Creature* pPatron = instance->GetCreature(*itr))
                    {
                        if (pPatron->GetPositionZ() > pGo->GetPositionZ() - 1 && pPatron->IsWithinDist2d(pGo->GetPositionX(), pGo->GetPositionY(), 18.0f))
                        {
                            uint32 uiTextId = 0;
                            switch (urand(0, 4))
                            {
                                case 0: uiTextId = SAY_PISSED_PATRON_3; break;
                                case 1:  // case is double to give this text twice the chance of the previous one do be displayed
                                case 2: uiTextId = SAY_PISSED_PATRON_2; break;
                                // covers the two remaining cases
                                default: uiTextId = SAY_PISSED_PATRON_1; break;
                            }
                            DoScriptText(uiTextId, pPatron);
                        }
                    }
                }
            }
            return;
        // case when Plugger is killed
        case PATRON_HOSTILE:
            for (GuidSet::const_iterator itr = m_sBarPatronNpcGuids.begin(); itr != m_sBarPatronNpcGuids.end(); ++itr)
            {
                if (Creature* pPatron = instance->GetCreature(*itr))
                {
                    pPatron->SetFactionTemporary(FACTION_DARK_IRON, TEMPFACTION_RESTORE_RESPAWN);
                    pPatron->SetStandState(UNIT_STAND_STATE_STAND);
                    // Patron should also start random movement around their current position (= home)
                }
            }
            // Mistress Nagmara and Private Rocknot despawn if the bar turns hostile
            if (Creature* pRocknot = GetSingleCreatureFromStorage(NPC_PRIVATE_ROCKNOT))
            {
                DoScriptText(SAY_ROCKNOT_DESPAWN, pRocknot);
                pRocknot->ForcedDespawn();
            }
            if (Creature* pNagmara = GetSingleCreatureFromStorage(NPC_MISTRESS_NAGMARA))
            {
                pNagmara->CastSpell(pNagmara, SPELL_NAGMARA_VANISH, true);
                pNagmara->ForcedDespawn();
            }
            return;
    }
}

void instance_blackrock_depths::HandleBarPatrol(uint8 uiStep)
{
    if (GetData(TYPE_BAR) == DONE)
        return;

    switch (uiStep)
    {
        case 0:
            if (Creature* pPlugger = GetSingleCreatureFromStorage(NPC_PLUGGER_SPAZZRING))
            {
                // if relevant, open the bar door and tell the instance it is open
                if (!m_bIsBarDoorOpen)
                {
                    DoUseDoorOrButton(GO_BAR_DOOR);
                    SetBarDoorIsOpen();
                }

                // One Fireguard Destroyer and two Anvilrage Officers are spawned
                for (uint8 i = 0; i < 3; ++i)
                {
                    float fX, fY, fZ;
                    // spawn them behind the bar door
                    pPlugger->GetRandomPoint(aBarPatrolPositions[0][0], aBarPatrolPositions[0][1], aBarPatrolPositions[0][2], 2.0f, fX, fY, fZ);
                    if (Creature* pSummoned = pPlugger->SummonCreature(aBarPatrolId[i], fX, fY, fZ, aBarPatrolPositions[0][3], TEMPSUMMON_DEAD_DESPAWN, 0))
                    {
                        m_sBarPatrolGuids.insert(pSummoned->GetObjectGuid());
                        // move them to the Grim Guzzler
                        pPlugger->GetRandomPoint(aBarPatrolPositions[1][0], aBarPatrolPositions[1][1], aBarPatrolPositions[1][2], 2.0f, fX, fY, fZ);
                        pSummoned->GetMotionMaster()->MoveIdle();
                        pSummoned->GetMotionMaster()->MovePoint(0,fX, fY, fZ);
                    }
                }
                // start timer to handle the yells
                m_uiPatrolTimer = 4000;
                break;
            }
        case 1:
            for (GuidSet::const_iterator itr = m_sBarPatrolGuids.begin(); itr != m_sBarPatrolGuids.end(); ++itr)
            {
                if (Creature* pTmp = instance->GetCreature(*itr))
                {
                    if (pTmp->GetEntry() == NPC_FIREGUARD_DESTROYER)
                    {
                        DoScriptText(YELL_PATROL_1, pTmp);
                        SetData(TYPE_BAR, SPECIAL); // temporary set the status to special before the next yell: event will then be complete
                        m_uiPatrolTimer = 2000;
                        break;
                    }
                }
            }
            break;
        case 2:
            for (GuidSet::const_iterator itr = m_sBarPatrolGuids.begin(); itr != m_sBarPatrolGuids.end(); ++itr)
            {
                if (Creature* pTmp = instance->GetCreature(*itr))
                {
                    if (pTmp->GetEntry() == NPC_FIREGUARD_DESTROYER)
                    {
                        DoScriptText(YELL_PATROL_2, pTmp);
                        SetData(TYPE_BAR, DONE);
                        m_uiPatrolTimer = 0;
                        break;
                    }
                }
            }
            break;
    }
}

void instance_blackrock_depths::Update(uint32 uiDiff)
{
    if (m_uiDwarfFightTimer)
    {
        if (m_uiDwarfFightTimer <= uiDiff)
        {
            if (m_uiDwarfRound < MAX_DWARFS)
            {
                DoCallNextDwarf();
                m_uiDwarfFightTimer = 30000;
            }
            else
                m_uiDwarfFightTimer = 0;
        }
        else
            m_uiDwarfFightTimer -= uiDiff;
    }

    if (m_uiDagranTimer)
    {
        if (m_uiDagranTimer <= uiDiff)
            m_uiDagranTimer = 0;
        else
            m_uiDagranTimer -= uiDiff;
    }

    // Every second some of the patrons will do one random emote if they are not hostile (i.e. Plugger event is not done/in progress)
    if (m_uiPatronEmoteTimer)
    {
        if (m_uiPatronEmoteTimer <= uiDiff)
        {
            HandleBarPatrons(PATRON_EMOTE);
            m_uiPatronEmoteTimer = 1000;
        }
        else
            m_uiPatronEmoteTimer -= uiDiff;
    }

    if (m_uiPatrolTimer)
    {
        if (m_uiPatrolTimer <= uiDiff)
        {
            switch(GetData(TYPE_BAR))
            {
                case IN_PROGRESS:
                    HandleBarPatrol(1);
                    break;
                case SPECIAL:
                    HandleBarPatrol(2);
                    break;
                default:
                    break;
            }
        }
        else
            m_uiPatrolTimer -= uiDiff;
    }
}

InstanceData* GetInstanceData_instance_blackrock_depths(Map* pMap)
{
    return new instance_blackrock_depths(pMap);
}

void AddSC_instance_blackrock_depths()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_blackrock_depths";
    pNewScript->GetInstanceData = &GetInstanceData_instance_blackrock_depths;
    pNewScript->RegisterSelf();
}
