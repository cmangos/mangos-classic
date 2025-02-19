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
EndScriptData */

/* ContentData
event_spells_warlock_dreadsteed
npc_warlock_ritual_mob
DreadsteedQuestObjects
EndContentData */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "dire_maul.h"

/*######
## event_spells_warlock_dreadsteed
######*/

enum
{
    EVENT_ID_SUMMON_JEEVEE      = 8420,
    EVENT_ID_SUMMON_DREADSTEED  = 8428,
};

bool ProcessEventId_event_spells_warlock_dreadsteed(uint32 uiEventId, Object* pSource, Object* /*pTarget*/, bool bIsStart)
{
    if (bIsStart && pSource->GetTypeId() == TYPEID_PLAYER)
    {
        if (instance_dire_maul* pInstance = (instance_dire_maul*)((Player*)pSource)->GetInstanceData())
        {
            // summon J'eevee and start event
            if (uiEventId == EVENT_ID_SUMMON_JEEVEE)
            {
                if (pInstance->GetData(TYPE_DREADSTEED) == NOT_STARTED || pInstance->GetData(TYPE_DREADSTEED) == FAIL)
                {
                    // start event: summon the dummy infernal controller and set in progress
                    ((Player*)pSource)->SummonCreature(NPC_WARLOCK_DUMMY_INFERNAL, -37.9392f, 812.805f, -29.4525f, 4.81711f, TEMPSPAWN_DEAD_DESPAWN, 0);

                    // J'eevee starts on path 1 here. Path 0 is for Scholomance event
                    if (Creature* pImp = ((Player*)pSource)->SummonCreature(NPC_JEEVEE, -37.9392f, 812.805f, -29.4525f, 4.81711f, TEMPSPAWN_DEAD_DESPAWN, 0, true, true))
                        pImp->GetMotionMaster()->MoveWaypoint(1);

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
                    pInstance->SetData(TYPE_DREADSTEED, DONE);

                    // rest is done by DBscript
                    return false;
                }
            }
        }
    }
    return true;
}

/*######
## npc_warlock_ritual_mob
## TODO: find proper timings for spawns
######*/

enum
{
    SUMMON_WAVES_RELAY_SCRIPT_START_ID = 1450100,
};

static const float pedestalCords[3] = { -38.5f, 811.8f, -29.5f};

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
    uint32 m_uiDeactivateTimer;
    GuidList m_luiSummonedMobGUIDs;

    void Reset() override
    {
        m_uiPhase = 0;
        m_uiPhaseTimer = 48000;
        m_uiDeactivateTimer = 70000;
    }

    void DoSummonPack(uint8 uiIndex)
    {
        m_creature->GetMap()->ScriptsStart(SCRIPT_TYPE_RELAY, SUMMON_WAVES_RELAY_SCRIPT_START_ID + uiIndex, m_creature, m_creature);
        m_pInstance->DoRespawnGameObject(m_pInstance->GetRitualSymbolGuids().at(uiIndex - 1), 900);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        m_luiSummonedMobGUIDs.push_back(pSummoned->GetObjectGuid());
        // The spawning of the waves is handled in the DB relay scripts, the movement to the center I'm doing here because simply using a 
        // single point in the center of the room will result in the demons floating jankily through the air because the floor is uneven.
        // So move halfway to the pedestal with a certain Z height, then move to the atual pedestal.
        // Maybe it'd be better to handle the spawning of the waves here too, but whatever, this works.
        pSummoned->GetMotionMaster()->MovePoint(0, (pSummoned->GetPositionX() + pedestalCords[0]) / 2.0f, (pSummoned->GetPositionY() + pedestalCords[1]) / 2.0f, -32.0f);
        pSummoned->SetRespawnCoord(pedestalCords[0], pedestalCords[1], pedestalCords[2], 0.0f);
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 /*pMotionType*/, uint32 pData) override
    {
        switch (pData)
        {
            case 0:
                pSummoned->GetMotionMaster()->MovePoint(1, pedestalCords[0], pedestalCords[1], pedestalCords[2]);
                break;
            case 1:
                pSummoned->GetMotionMaster()->MoveRandomAroundPoint(pedestalCords[0], pedestalCords[1], pedestalCords[2], 15.0f);
                break;
        }
    }

    void SummonedJustReachedHome(Creature* pSummoned) override
    {
        pSummoned->GetMotionMaster()->MoveRandomAroundPoint(pedestalCords[0], pedestalCords[1], pedestalCords[2], 15.0f);
    }

    void UpdateAI(uint32 const diff) override
    {
        if (m_uiPhaseTimer <= diff) // Handle spawning waves and cleanup at the end
        {
            switch (m_uiPhase)
            {
                case 1:
                    m_pInstance->ProcessDreadsteedRitualStart();
                    m_uiPhaseTimer = 1000;
                    break;
                case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: // 9 waves of demons - each spawned by relay script in DB
                    m_uiPhaseTimer = 45000;
                    DoSummonPack(m_uiPhase - 1);
                    break;
                case 10: // Final wave, short, make players sweat just before ending
                    m_uiPhaseTimer = 17500;
                    DoSummonPack(m_uiPhase - 1);
                    break;
                case 11:
                    for (auto demon : m_luiSummonedMobGUIDs)
                    {
                        if (Creature* pSummoned = m_creature->GetMap()->GetCreature(demon))
                        {
                            pSummoned->CastSpell(pSummoned, SPELL_TELEPORT, TRIGGERED_OLD_TRIGGERED);
                            DoScriptText(SAY_UNSUMMON_DEMON, pSummoned);
                            pSummoned->ForcedDespawn(1000);
                        }
                    }
                    if (GameObject* pGo = m_pInstance->GetSingleGameObjectFromStorage(GO_WARLOCK_RITUAL_CIRCLE))
                        pGo->UseDoorOrButton();
                    m_pInstance->SetData(TYPE_DREADSTEED, SPECIAL);
                    m_creature->ForcedDespawn();
                    break;
            }
            ++m_uiPhase;
        }
        else
            m_uiPhaseTimer -= diff;

        if (m_uiDeactivateTimer <= diff) // Periodically deactivate the helping objects
        {
            uint32 rand = urand(0, 2); // Pick one of the 3 objects to deactivate
            GameObject* toBeDeactivated = nullptr;

            for (int i = 0; i < 3; ++i) // Use the rand variable as a starting point, but if needed, cylce through the other objects if the chosen one is already deactivated
            {
                switch ((rand + i) % 3)
                {
                    case 0:
                        toBeDeactivated = m_pInstance->GetSingleGameObjectFromStorage(GO_BELL_OF_DETHMOORA);
                        break;
                    case 1:
                        toBeDeactivated = m_pInstance->GetSingleGameObjectFromStorage(GO_WHEEL_OF_BLACK_MARCH);
                        break;
                    case 2:
                        toBeDeactivated = m_pInstance->GetSingleGameObjectFromStorage(GO_DOOMSDAY_CANDLE);
                        break;
                }

                // instance_dire_maul.cpp handles failing the event if all objects are deactivated
                if (toBeDeactivated != nullptr && toBeDeactivated->GetLootState() == GO_ACTIVATED)
                {
                    toBeDeactivated->ResetDoorOrButton();
                    break;
                }
            }

            m_uiDeactivateTimer = urand(30000, 45000);
        }
        else
            m_uiDeactivateTimer -= diff;
    }
};

/*######
## DreadsteedQuestObjects
######*/

struct DreadsteedQuestObjects : public GameObjectAI
{
    DreadsteedQuestObjects(GameObject* go) : GameObjectAI(go) {}

    uint32 m_uiPulseTimer = 0;
    ObjectGuid m_lastUserGuid;

    void JustSpawned() override
    {
        if (instance_dire_maul* instance = (instance_dire_maul*)m_go->GetInstanceData()) // Without this check, the 3 objects are spawning when first loaded in. Couldn't find another way to stop them from doing this
            if (instance->GetData(TYPE_DREADSTEED) != IN_PROGRESS)
            {
                m_go->ForcedDespawn();
                m_go->SetLootState(GO_JUST_DEACTIVATED);
                return;
            }
        m_go->SetRespawnDelay(0);
    }

    void UpdateAI(const uint32 diff) override
    {
        if (m_uiPulseTimer)
        {
            if (m_uiPulseTimer <= diff)
            {
                uint32 spellId = 0;
                switch (m_go->GetEntry())
                {
                    case GO_BELL_OF_DETHMOORA:
                        spellId = SPELL_RITUAL_BELL_AURA;
                        break;
                    case GO_DOOMSDAY_CANDLE:
                        spellId = SPELL_RITUAL_CANDLE_AURA;
                        break;
                    case GO_WHEEL_OF_BLACK_MARCH:
                        spellId = SPELL_BLACK_MARCH_BLESSING;
                        break;
                }

                if (instance_dire_maul* instance = (instance_dire_maul*)m_go->GetInstanceData())
                    if (GameObject* candleAura = instance->GetSingleGameObjectFromStorage(GO_RITUAL_CANDLE_AURA))
                        if (Unit* target = m_go->GetMap()->GetUnit(m_lastUserGuid))
                        {
                            if (spellId == SPELL_RITUAL_CANDLE_AURA)
                                candleAura->CastSpell(target, nullptr, spellId, TRIGGERED_OLD_TRIGGERED);
                            else
                                m_go->CastSpell(target, nullptr, spellId, TRIGGERED_OLD_TRIGGERED);
                        }

                m_uiPulseTimer = 6000;
            }
            else
                m_uiPulseTimer -= diff;
        }
    }

    void OnLootStateChange(Unit* /*user*/) override
    {
        if (m_go->GetLootState() == GO_JUST_DEACTIVATED)
            m_uiPulseTimer = 0;
    }

    void OnUse(Unit* user, SpellEntry const* /*spellInfo*/) override
    {
        if (user->IsPlayer())
            m_lastUserGuid = user->GetObjectGuid();
        else // The candle won't work until a player reactivates it unless we check for this because the npc imp is the first "user" of it
            m_lastUserGuid = user->GetSpawnerGuid();
        m_uiPulseTimer = 1;
    }
};

struct RitualCandleAura : public SpellScript
{
    bool OnCheckTarget(const Spell* /*spell*/, Unit* target, SpellEffectIndex /*eff*/) const override
    {
        if (target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
            return false;
        return true;
    }
};

void AddSC_dire_maul()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "event_spells_warlock_dreadsteed";
    pNewScript->pProcessEventId = &ProcessEventId_event_spells_warlock_dreadsteed;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_warlock_mount_ritual";
    pNewScript->GetAI = &GetNewAIInstance<npc_warlock_mount_ritualAI>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_dreadsteed_quest_objects";
    pNewScript->GetGameObjectAI = &GetNewAIInstance<DreadsteedQuestObjects>;
    pNewScript->RegisterSelf();

    RegisterSpellScript<RitualCandleAura>("spell_ritual_candle_aura");
}
