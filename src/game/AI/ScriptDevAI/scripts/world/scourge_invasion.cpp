/*
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

//#include "scriptPCH.h"
#include "scourge_invasion.h"
//#include "CreatureGroups.h"
#include "AI/ScriptDevAI/include/sc_common.h"
#include "AI/ScriptDevAI/base/CombatAI.h"
#include "Globals/ObjectMgr.h"
#include "World/WorldStateDefines.h"
#include "Grids/CellImpl.h"
#include "GameEvents/GameEventMgr.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"

inline uint32 GetCampType(Creature* unit) { return unit->HasAura(SPELL_CAMP_TYPE_GHOST_SKELETON) || unit->HasAura(SPELL_CAMP_TYPE_GHOST_GHOUL) || unit->HasAura(SPELL_CAMP_TYPE_GHOUL_SKELETON); };

inline bool IsGuardOrBoss(Unit* unit) {
    return unit->GetEntry() == NPC_ROYAL_DREADGUARD || unit->GetEntry() == NPC_STORMWIND_ROYAL_GUARD || unit->GetEntry() == NPC_UNDERCITY_ELITE_GUARDIAN || unit->GetEntry() == NPC_UNDERCITY_GUARDIAN || unit->GetEntry() == NPC_DEATHGUARD_ELITE ||
        unit->GetEntry() == NPC_STORMWIND_CITY_GUARD || unit->GetEntry() == NPC_HIGHLORD_BOLVAR_FORDRAGON || unit->GetEntry() == NPC_LADY_SYLVANAS_WINDRUNNER || unit->GetEntry() == NPC_VARIMATHRAS;
}

Unit* SelectRandomFlameshockerSpawnTarget(Creature* unit, Unit* except, float radius)
{
    std::list<Unit*> targets;

    MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck u_check(unit, radius);
    MaNGOS::UnitListSearcher<MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck> searcher(targets, u_check);
    Cell::VisitAllObjects(unit, searcher, radius);

    // remove current target
    if (except)
        targets.remove(except);

    for (std::list<Unit*>::iterator tIter = targets.begin(); tIter != targets.end();)
    {
        // || !(*tIter)->ToCreature()->CanSummonGuards() - CREATURE_FLAG_EXTRA_SUMMON_GUARD
        if (!(*tIter)->IsCreature() || !static_cast<Creature*>(*tIter)->IsCivilian() || (*tIter)->GetZoneId() != unit->GetZoneId() || !unit->CanAttack((*tIter)) || GetClosestCreatureWithEntry((*tIter), NPC_FLAMESHOCKER, VISIBILITY_DISTANCE_TINY))
        {
            std::list<Unit*>::iterator tIter2 = tIter;
            ++tIter;
            targets.erase(tIter2);
        }
        else
            ++tIter;
    }

    // no appropriate targets
    if (targets.empty())
        return nullptr;

    // select random
    uint32 rIdx = urand(0, targets.size() - 1);
    std::list<Unit*>::const_iterator tcIter = targets.begin();
    for (uint32 i = 0; i < rIdx; ++i)
        ++tcIter;

    return *tcIter;
}

void ChangeZoneEventStatus(Creature* mouth, bool on)
{
    if (!mouth)
        return;

    switch (mouth->GetZoneId())
    {
        case ZONEID_WINTERSPRING:
            if (on)
            {
                if (!sGameEventMgr.IsActiveEvent(GAME_EVENT_SCOURGE_INVASION_WINTERSPRING))
                    sGameEventMgr.StartEvent(GAME_EVENT_SCOURGE_INVASION_WINTERSPRING, true);
            }
            else
                sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION_WINTERSPRING, true);
            break;
        case ZONEID_TANARIS:
            if (on)
            {
                if (!sGameEventMgr.IsActiveEvent(GAME_EVENT_SCOURGE_INVASION_TANARIS))
                    sGameEventMgr.StartEvent(GAME_EVENT_SCOURGE_INVASION_TANARIS, true);
            }
            else
                sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION_TANARIS, true);
            break;
        case ZONEID_AZSHARA:
            if (on)
            {
                if (!sGameEventMgr.IsActiveEvent(GAME_EVENT_SCOURGE_INVASION_AZSHARA))
                    sGameEventMgr.StartEvent(GAME_EVENT_SCOURGE_INVASION_AZSHARA, true);
            }
            else
                sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION_AZSHARA, true);
            break;
        case ZONEID_BLASTED_LANDS:
            if (on)
            {
                if (!sGameEventMgr.IsActiveEvent(GAME_EVENT_SCOURGE_INVASION_BLASTED_LANDS))
                    sGameEventMgr.StartEvent(GAME_EVENT_SCOURGE_INVASION_BLASTED_LANDS, true);
            }
            else
                sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION_BLASTED_LANDS, true);
            break;
        case ZONEID_EASTERN_PLAGUELANDS:
            if (on)
            {
                if (!sGameEventMgr.IsActiveEvent(GAME_EVENT_SCOURGE_INVASION_EASTERN_PLAGUELANDS))
                    sGameEventMgr.StartEvent(GAME_EVENT_SCOURGE_INVASION_EASTERN_PLAGUELANDS, true);
            }
            else
                sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION_EASTERN_PLAGUELANDS, true);
            break;
        case ZONEID_BURNING_STEPPES:
            if (on)
            {
                if (!sGameEventMgr.IsActiveEvent(GAME_EVENT_SCOURGE_INVASION_BURNING_STEPPES))
                    sGameEventMgr.StartEvent(GAME_EVENT_SCOURGE_INVASION_BURNING_STEPPES, true);
            }
            else
                sGameEventMgr.StopEvent(GAME_EVENT_SCOURGE_INVASION_BURNING_STEPPES, true);
            break;
    }
}

void DespawnEventDoodads(Creature* shard)
{
    if (!shard)
        return;

    std::list<GameObject*> doodadList;
    GetGameObjectListWithEntryInGrid(doodadList, shard, { GOBJ_SUMMON_CIRCLE, GOBJ_UNDEAD_FIRE, GOBJ_UNDEAD_FIRE_AURA, GOBJ_SKULLPILE_01, GOBJ_SKULLPILE_02, GOBJ_SKULLPILE_03, GOBJ_SKULLPILE_04, GOBJ_SUMMONER_SHIELD }, 60.0f);
    for (const auto pDoodad : doodadList)
        pDoodad->ForcedDespawn();

    std::list<Creature*> finderList;
    GetCreatureListWithEntryInGrid(finderList, shard, NPC_SCOURGE_INVASION_MINION_FINDER, 60.0f);
    for (const auto pFinder : finderList)
        pFinder->ForcedDespawn();
}

void DespawnNecropolis(Unit* despawner)
{
    if (!despawner)
        return;

    std::list<GameObject*> necropolisList;
    GetGameObjectListWithEntryInGrid(necropolisList, despawner, { GOBJ_NECROPOLIS_TINY, GOBJ_NECROPOLIS_SMALL, GOBJ_NECROPOLIS_MEDIUM, GOBJ_NECROPOLIS_BIG, GOBJ_NECROPOLIS_HUGE }, ATTACK_DISTANCE);
    for (const auto pNecropolis : necropolisList)
        pNecropolis->ForcedDespawn();
}

void SummonCultists(Unit* shard)
{
    if (!shard)
        return;

    std::list<GameObject*> summonerShieldList;
    GetGameObjectListWithEntryInGrid(summonerShieldList, shard, GOBJ_SUMMONER_SHIELD, INSPECT_DISTANCE);
    for (const auto pSummonerShield : summonerShieldList)
        pSummonerShield->ForcedDespawn();

    // We don't have all positions sniffed from the Cultists, so why not using this code which placing them almost perfectly into the circle while blizzards positions are some times way off?
    if (GameObject* gameObject = GetClosestGameObjectWithEntry(shard, GOBJ_SUMMON_CIRCLE, CONTACT_DISTANCE))
    {
        for (int i = 0; i < 4; ++i)
        {
            float angle = (float(i) * (M_PI / 2)) + gameObject->GetOrientation();
            float x = gameObject->GetPositionX() + 6.95f * cos(angle);
            float y = gameObject->GetPositionY() + 6.75f * sin(angle);
            float z = gameObject->GetPositionZ() + 5.0f;
            shard->UpdateGroundPositionZ(x, y, z);
            if (Creature* cultist = shard->SummonCreature(NPC_CULTIST_ENGINEER, x, y, z, angle - M_PI, TEMPSPAWN_TIMED_OR_DEAD_DESPAWN, IN_MILLISECONDS * HOUR, true))
                cultist->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, shard, cultist, NPC_CULTIST_ENGINEER);
        }
    }
}

void DespawnCultists(Unit* despawner)
{
    if (!despawner)
        return;

    std::list<Creature*> cultistList;
    GetCreatureListWithEntryInGrid(cultistList, despawner, NPC_CULTIST_ENGINEER, INSPECT_DISTANCE);
    for (const auto pCultist : cultistList)
        if (pCultist)
            pCultist->ForcedDespawn();
}

void DespawnShadowsOfDoom(Unit* despawner)
{
    if (!despawner)
        return;

    std::list<Creature*> shadowList;
    GetCreatureListWithEntryInGrid(shadowList, despawner, NPC_SHADOW_OF_DOOM, 200.0f);
    for (const auto pShadow : shadowList)
        if (pShadow && pShadow->IsAlive() && !pShadow->IsInCombat())
            pShadow->ForcedDespawn();
}

uint32 HasMinion(Creature* summoner, float range)
{
    if (!summoner)
        return false;

    uint32 minionCounter = 0;
    std::list<Creature*> minionList;
    GetCreatureListWithEntryInGrid(minionList, summoner, { NPC_SKELETAL_SHOCKTROOPER, NPC_GHOUL_BERSERKER, NPC_SPECTRAL_SOLDIER, NPC_LUMBERING_HORROR, NPC_BONE_WITCH, NPC_SPIRIT_OF_THE_DAMNED }, ATTACK_DISTANCE);
    for (const auto pMinion : minionList)
        if (pMinion && pMinion->IsAlive())
            minionCounter++;

    return minionCounter;
}

bool UncommonMinionspawner(Creature* pSummoner) // Rare Minion Spawner.
{
    if (!pSummoner)
        return false;

    std::list<Creature*> uncommonMinionList;
    GetCreatureListWithEntryInGrid(uncommonMinionList, pSummoner, { NPC_LUMBERING_HORROR, NPC_BONE_WITCH, NPC_SPIRIT_OF_THE_DAMNED }, 100.0f);
    for (const auto pMinion : uncommonMinionList)
        if (pMinion)
            return false; // Already a rare found (dead or alive).

    /*
    The chance or timer for a Rare minion spawn is unknown and i don't see an exact pattern for a spawn sequence.
    Sniffed are: 19669 Minions and 90 Rares (Ratio: 217 to 1).
    */
    uint32 chance = urand(1, 217);
    if (chance > 1)
        return false; // Above 1 = Minion, else Rare.

    return true;
}

uint32 GetFindersAmount(Creature* shard)
{
    uint32 finderCounter = 0;
    std::list<Creature*> finderList;
    GetCreatureListWithEntryInGrid(finderList, shard, NPC_SCOURGE_INVASION_MINION_FINDER, 60.0f);
    for (const auto pFinder : finderList)
        if (pFinder)
            finderCounter++;

    return finderCounter;
}

/*
Circle
*/
class GoCircle : public GameObjectAI
{
    public:
        GoCircle(GameObject* gameobject) : GameObjectAI(gameobject)
        {
            //m_go->CastSpell(m_go, SPELL_CREATE_CRYSTAL, true);
            m_go->CastSpell(nullptr, nullptr, SPELL_CREATE_CRYSTAL, TRIGGERED_OLD_TRIGGERED);
        }
};

/*
Necropolis
*/
class GoNecropolis : public GameObjectAI
{
    public:
        GoNecropolis(GameObject* gameobject) : GameObjectAI(gameobject)
        {
            m_go->SetActiveObjectState(true);
            //m_go->SetVisibilityModifier(3000.0f);
        }
};

/*
Mouth of Kel'Thuzad
*/
struct MouthAI : public ScriptedAI
{
    MouthAI(Creature* creature) : ScriptedAI(creature)
    {
        AddCustomAction(EVENT_MOUTH_OF_KELTHUZAD_YELL, urand((IN_MILLISECONDS * 150), (IN_MILLISECONDS * HOUR)), [&]()
        {
            DoBroadcastText(PickRandomValue(BCT_MOUTH_OF_KELTHUZAD_RANDOM_1, BCT_MOUTH_OF_KELTHUZAD_RANDOM_2, BCT_MOUTH_OF_KELTHUZAD_RANDOM_3, BCT_MOUTH_OF_KELTHUZAD_RANDOM_4), m_creature, nullptr, CHAT_TYPE_ZONE_YELL);
            ResetTimer(EVENT_MOUTH_OF_KELTHUZAD_YELL, urand((IN_MILLISECONDS * 150), (IN_MILLISECONDS * HOUR)));
        });
        SetReactState(REACT_PASSIVE);
    }

    void Reset() override {}

    void ReceiveAIEvent(AIEventType eventType, Unit* /*sender*/, Unit* /*invoker*/, uint32 miscValue) override
    {
        if (eventType == AI_EVENT_CUSTOM_A)
        {
            switch (miscValue)
            {
                case EVENT_MOUTH_OF_KELTHUZAD_ZONE_START:
                {
                    ChangeZoneEventStatus(m_creature, true);
                    m_creature->GetMap()->SetWeather(m_creature->GetZoneId(), WEATHER_TYPE_STORM, 0.25f, true);
                    DoBroadcastText(PickRandomValue(BCT_MOUTH_OF_KELTHUZAD_ZONE_ATTACK_START_1, BCT_MOUTH_OF_KELTHUZAD_ZONE_ATTACK_START_2), m_creature, nullptr, CHAT_TYPE_ZONE_YELL);
                    break;
                }
                case EVENT_MOUTH_OF_KELTHUZAD_ZONE_STOP:
                {
                    DoBroadcastText(PickRandomValue(BCT_MOUTH_OF_KELTHUZAD_ZONE_ATTACK_ENDS_1, BCT_MOUTH_OF_KELTHUZAD_ZONE_ATTACK_ENDS_2, BCT_MOUTH_OF_KELTHUZAD_ZONE_ATTACK_ENDS_3), m_creature, nullptr, CHAT_TYPE_ZONE_YELL);
                    ChangeZoneEventStatus(m_creature, false);
                    m_creature->GetMap()->SetWeather(m_creature->GetZoneId(), WEATHER_TYPE_RAIN, 0.0f, false);
                    m_creature->ForcedDespawn();
                    break;
                }
            }
        }
    }
};

/*
Necropolis
*/
struct NecropolisAI : public ScriptedAI
{
    NecropolisAI(Creature* creature) : ScriptedAI(creature)
    {
        m_creature->SetActiveObjectState(true);
        //m_creature->SetVisibilityModifier(3000.0f);
    }

    void Reset() override {}

    void SpellHit(Unit* caster, SpellEntry const* spell) override
    {
        if (m_creature->HasAura(SPELL_COMMUNIQUE_TIMER_NECROPOLIS))
            return;

        if (spell->Id == SPELL_COMMUNIQUE_PROXY_TO_NECROPOLIS)
            m_creature->CastSpell(m_creature, SPELL_COMMUNIQUE_TIMER_NECROPOLIS, TRIGGERED_OLD_TRIGGERED); // m_creature->AddAura(SPELL_COMMUNIQUE_TIMER_NECROPOLIS);
    }

    void UpdateAI(uint32 const diff) override {}
};

/*
Necropolis Health
*/
struct NecropolisHealthAI : public ScriptedAI
{
    NecropolisHealthAI(Creature* creature) : ScriptedAI(creature)
    {
        //m_creature->SetVisibilityModifier(3000.0f);
    }

    int m_zapped = 0; // 3 = death.

    void Reset() override {}

    void SpellHit(Unit* caster, SpellEntry const* spell) override
    {
        if (spell->Id == SPELL_COMMUNIQUE_CAMP_TO_RELAY_DEATH)
            m_creature->CastSpell(m_creature, SPELL_ZAP_NECROPOLIS, TRIGGERED_OLD_TRIGGERED);

        // Just to make sure it finally dies!
        if (spell->Id == SPELL_ZAP_NECROPOLIS)
        {
            if (++m_zapped >= 3)
                m_creature->Suicide(); //m_creature->DoKillUnit(m_creature);
        }
    }

    void JustDied(Unit* killer) override
    {
        if (Creature* necropolis = GetClosestCreatureWithEntry(m_creature, NPC_NECROPOLIS, ATTACK_DISTANCE))
            m_creature->CastSpell(necropolis, SPELL_DESPAWNER_OTHER, TRIGGERED_OLD_TRIGGERED);

        SIRemaining remainingID;

        switch (m_creature->GetZoneId())
        {
            default:
            case ZONEID_TANARIS:
                remainingID = SI_REMAINING_TANARIS;
                break;
            case ZONEID_BLASTED_LANDS:
                remainingID = SI_REMAINING_BLASTED_LANDS;
                break;
            case ZONEID_EASTERN_PLAGUELANDS:
                remainingID = SI_REMAINING_EASTERN_PLAGUELANDS;
                break;
            case ZONEID_BURNING_STEPPES:
                remainingID = SI_REMAINING_BURNING_STEPPES;
                break;
            case ZONEID_WINTERSPRING:
                remainingID = SI_REMAINING_WINTERSPRING;
                break;
            case ZONEID_AZSHARA:
                remainingID = SI_REMAINING_AZSHARA;
                break;
        }

        uint32 remaining = sWorldState.GetSIRemaining(remainingID);
        if (remaining > 0)
            sWorldState.SetSIRemaining(remainingID, (remaining - 1));
    }

    void SpellHitTarget(Unit* target, SpellEntry const* spellInfo) override
    {
        // Make sure m_creature despawn after SPELL_DESPAWNER_OTHER triggered.
        if (spellInfo->Id == SPELL_DESPAWNER_OTHER && target->GetEntry() == NPC_NECROPOLIS)
        {
            DespawnNecropolis(target);
            static_cast<Creature*>(target)->ForcedDespawn();
            m_creature->ForcedDespawn();
        }
    }

    void UpdateAI(uint32 const diff) override {}
};

/*
Necropolis Proxy
*/
struct NecropolisProxyAI : public ScriptedAI
{
    NecropolisProxyAI(Creature* creature) : ScriptedAI(creature)
    {
        m_creature->SetActiveObjectState(true);
        //m_creature->SetVisibilityModifier(3000.0f);
        Reset();
    }

    void Reset() override {}

    void SpellHit(Unit* caster, SpellEntry const* spellInfo) override
    {
        switch (spellInfo->Id)
        {
            case SPELL_COMMUNIQUE_NECROPOLIS_TO_PROXIES:
                m_creature->CastSpell(m_creature, SPELL_COMMUNIQUE_PROXY_TO_RELAY, TRIGGERED_OLD_TRIGGERED);
                break;
            case SPELL_COMMUNIQUE_RELAY_TO_PROXY:
                m_creature->CastSpell(m_creature, SPELL_COMMUNIQUE_PROXY_TO_NECROPOLIS, TRIGGERED_OLD_TRIGGERED);
                break;
            case SPELL_COMMUNIQUE_CAMP_TO_RELAY_DEATH:
                if (Creature* health = GetClosestCreatureWithEntry(m_creature, NPC_NECROPOLIS_HEALTH, 200.0f))
                    m_creature->CastSpell(health, SPELL_COMMUNIQUE_CAMP_TO_RELAY_DEATH, TRIGGERED_OLD_TRIGGERED);
                break;
        }
    }

    void SpellHitTarget(Unit* target, SpellEntry const* spellInfo) override
    {
        // Make sure m_creature despawn after SPELL_COMMUNIQUE_CAMP_TO_RELAY_DEATH hits the target to avoid getting hit by Purple bolt again.
        if (spellInfo->Id == SPELL_COMMUNIQUE_CAMP_TO_RELAY_DEATH)
            m_creature->ForcedDespawn();
    }

    void UpdateAI(uint32 const diff) override {}
};

/*
Necropolis Relay
*/
struct NecropolisRelayAI : public ScriptedAI
{
    NecropolisRelayAI(Creature* creature) : ScriptedAI(creature)
    {
        m_creature->SetActiveObjectState(true);
        //m_creature->SetVisibilityModifier(3000.0f);
        Reset();
    }

    void Reset() override {}

    void SpellHit(Unit* caster, SpellEntry const* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_COMMUNIQUE_PROXY_TO_RELAY:
                m_creature->CastSpell(m_creature, SPELL_COMMUNIQUE_RELAY_TO_CAMP, TRIGGERED_OLD_TRIGGERED);
                break;
            case SPELL_COMMUNIQUE_CAMP_TO_RELAY:
                m_creature->CastSpell(m_creature, SPELL_COMMUNIQUE_RELAY_TO_PROXY, TRIGGERED_OLD_TRIGGERED);
                break;
            case SPELL_COMMUNIQUE_CAMP_TO_RELAY_DEATH:
                if (Creature* pProxy = GetClosestCreatureWithEntry(m_creature, NPC_NECROPOLIS_PROXY, 200.0f))
                    m_creature->CastSpell(pProxy, SPELL_COMMUNIQUE_CAMP_TO_RELAY_DEATH, TRIGGERED_OLD_TRIGGERED);
                break;
        }
    }

    void SpellHitTarget(Unit* target, SpellEntry const* spell) override
    {
        // Make sure m_creature despawn after SPELL_COMMUNIQUE_CAMP_TO_RELAY_DEATH hits the target to avoid getting hit by Purple bolt again.
        if (spell->Id == SPELL_COMMUNIQUE_CAMP_TO_RELAY_DEATH)
            m_creature->ForcedDespawn();
    }

    void UpdateAI(uint32 const diff) override {}
};

/*
Necrotic Shard
*/
struct NecroticShard : public ScriptedAI
{
    NecroticShard(Creature* creature) : ScriptedAI(creature)
    {
        m_creature->SetActiveObjectState(true);
        AddCustomAction(EVENT_SHARD_MINION_SPAWNER_SMALL, true, [&]() // Spawn Minions every 5 seconds.
        {
            HandleShardMinionSpawnerSmall();
        });
        //m_creature->SetVisibilityModifier(3000.0f);
        if (m_creature->GetEntry() == NPC_DAMAGED_NECROTIC_SHARD)
        {
            m_finders = GetFindersAmount(m_creature);                       // Count all finders to limit Minions spawns.
            ResetTimer(EVENT_SHARD_MINION_SPAWNER_SMALL, 5000); // Spawn Minions every 5 seconds.
            AddCustomAction(EVENT_SHARD_MINION_SPAWNER_BUTTRESS, 5000u, [&]() // Spawn Cultists every 60 minutes.
            {
                /*
                This is a placeholder for SPELL_MINION_SPAWNER_BUTTRESS [27888] which also activates unknown, not sniffable gamebjects
                and happens every hour if a Damaged Necrotic Shard is activ. The Cultists despawning after 1 hour,
                so this just resets everything and spawn them again and Refill the Health of the Shard.
                */
                m_creature->SetHealthPercent(100.f); // m_creature->SetFullHealth();
                // Despawn all remaining Shadows before respawning the Cultists?
                DespawnShadowsOfDoom(m_creature);
                // Respawn the Cultists.
                SummonCultists(m_creature);

                ResetTimer(EVENT_SHARD_MINION_SPAWNER_BUTTRESS, IN_MILLISECONDS * HOUR);
            });
        }
        else
        {
            // Just in case.
            std::list<Creature*> shardList;
            GetCreatureListWithEntryInGrid(shardList, m_creature, { NPC_NECROTIC_SHARD, NPC_DAMAGED_NECROTIC_SHARD }, CONTACT_DISTANCE);
            for (const auto shard : shardList)
                if (shard != m_creature)
                    shard->ForcedDespawn();
        }
        SetReactState(REACT_PASSIVE);
    }

    uint32 m_camptype = 0;
    uint32 m_finders = 0;

    void Reset() override
    {
        DoCastSpellIfCan(nullptr, SPELL_COMMUNIQUE_TIMER_CAMP, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void SpellHit(Unit* caster, SpellEntry const* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_ZAP_CRYSTAL_CORPSE:
            {
                m_creature->DealDamage(m_creature, m_creature, (m_creature->GetMaxHealth() / 4), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
                break;
            }
            case SPELL_COMMUNIQUE_RELAY_TO_CAMP:
            {
                m_creature->CastSpell(nullptr, SPELL_CAMP_RECEIVES_COMMUNIQUE, TRIGGERED_OLD_TRIGGERED);
                break;
            }
            case SPELL_CHOOSE_CAMP_TYPE:
            {
                m_camptype = PickRandomValue(SPELL_CAMP_TYPE_GHOUL_SKELETON, SPELL_CAMP_TYPE_GHOST_GHOUL, SPELL_CAMP_TYPE_GHOST_SKELETON);
                m_creature->CastSpell(m_creature, m_camptype, TRIGGERED_OLD_TRIGGERED);
                break;
            }
            case SPELL_CAMP_RECEIVES_COMMUNIQUE:
            {
                if (!GetCampType(m_creature) && m_creature->GetEntry() == NPC_NECROTIC_SHARD)
                {
                    m_finders = GetFindersAmount(m_creature);
                    m_creature->CastSpell(m_creature, SPELL_CHOOSE_CAMP_TYPE, TRIGGERED_OLD_TRIGGERED);
                    ResetTimer(EVENT_SHARD_MINION_SPAWNER_SMALL, 0); // Spawn Minions every 5 seconds.
                }
                break;
            }
            case SPELL_FIND_CAMP_TYPE:
            {
                // Don't spawn more Minions than finders.
                if (m_finders < HasMinion(m_creature, 60.0f))
                    return;

                // Lets the finder spawn the associated spawner.
                if (m_creature->HasAura(SPELL_CAMP_TYPE_GHOST_SKELETON))
                    caster->CastSpell(caster, SPELL_PH_SUMMON_MINION_TRAP_GHOST_SKELETON, TRIGGERED_OLD_TRIGGERED);
                else if (m_creature->HasAura(SPELL_CAMP_TYPE_GHOST_GHOUL))
                    caster->CastSpell(caster, SPELL_PH_SUMMON_MINION_TRAP_GHOST_GHOUL, TRIGGERED_OLD_TRIGGERED);
                else if (m_creature->HasAura(SPELL_CAMP_TYPE_GHOUL_SKELETON))
                    caster->CastSpell(caster, SPELL_PH_SUMMON_MINION_TRAP_GHOUL_SKELETON, TRIGGERED_OLD_TRIGGERED);
                break;
            }
        }
    }

    void SpellHitTarget(Unit* target, SpellEntry const* spellInfo) override
    {
        if (m_creature->GetEntry() != NPC_DAMAGED_NECROTIC_SHARD)
            return;

        if (spellInfo->Id == SPELL_COMMUNIQUE_CAMP_TO_RELAY_DEATH)
            m_creature->ForcedDespawn();
    }

    void DamageTaken(Unit* dealer, uint32& damage, DamageEffectType /*damagetype*/, SpellEntry const* /*spellInfo*/) override
    {
        // Only Minions and the shard itself can deal damage.
        if (dealer->GetFactionTemplateEntry() != m_creature->GetFactionTemplateEntry())
            damage = 0;
    }

    // No healing possible.
    void HealedBy(Unit* healer, uint32& uiHealedAmount) override
    {
        uiHealedAmount = 0;
    }

    void JustDied(Unit* killer) override
    {
        switch (m_creature->GetEntry())
        {
            case NPC_NECROTIC_SHARD:
                if (Creature* pShard = m_creature->SummonCreature(NPC_DAMAGED_NECROTIC_SHARD, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), m_creature->GetOrientation(), TEMPSPAWN_MANUAL_DESPAWN, 0))
                {
                    // Get the camp type from the Necrotic Shard.
                    if (m_camptype)
                        pShard->CastSpell(pShard, m_camptype, TRIGGERED_OLD_TRIGGERED);
                    else
                        pShard->CastSpell(pShard, SPELL_CHOOSE_CAMP_TYPE, TRIGGERED_OLD_TRIGGERED);

                    m_creature->ForcedDespawn();
                }
                break;
            case NPC_DAMAGED_NECROTIC_SHARD:
                // Buff Players.
                m_creature->CastSpell(m_creature, SPELL_SOUL_REVIVAL, TRIGGERED_OLD_TRIGGERED);
                // Sending the Death Bolt.
                if (Creature* pRelay = GetClosestCreatureWithEntry(m_creature, NPC_NECROPOLIS_RELAY, 200.0f))
                    m_creature->CastSpell(pRelay, SPELL_COMMUNIQUE_CAMP_TO_RELAY_DEATH, TRIGGERED_OLD_TRIGGERED);
                // Despawn remaining Cultists (should never happen).
                DespawnCultists(m_creature);
                // Remove Objects from the event around the Shard (Yes this is Blizzlike).
                DespawnEventDoodads(m_creature);
                break;
        }
    }

    void HandleShardMinionSpawnerSmall()
    {
        /*
        This is a placeholder for SPELL_MINION_SPAWNER_SMALL [27887] which also activates unknown, not sniffable objects, which possibly checks whether a minion is in his range
        and happens every 15 seconds for both, Necrotic Shard and Damaged Necrotic Shard.
        */

        int finderCounter = 0;
        int finderAmount = urand(1, 3); // Up to 3 spawns.

        std::list<Creature*> finderList;
        GetCreatureListWithEntryInGrid(finderList, m_creature, NPC_SCOURGE_INVASION_MINION_FINDER, 60.0f);
        if (finderList.empty())
            return;

        // On a fresh camp, first the minions are spawned close to the shard and then further and further out.
        finderList.sort(ObjectDistanceOrder(m_creature));

        for (const auto& pFinder : finderList)
        {
            // Stop summoning Minions if we reached the max spawn amount.
            if (finderAmount == finderCounter)
                break;

            // Skip dead finders.
            if (!pFinder->IsAlive())
                continue;

            // Don't take finders with Minions.
            if (HasMinion(pFinder, ATTACK_DISTANCE))
                continue;

            /*
            A finder disappears after summoning the spawner NPC (which summons the minion).
            after 160-185 seconds a finder respawns on the same position as before.
            */
            if (pFinder->AI()->DoCastSpellIfCan(m_creature, SPELL_FIND_CAMP_TYPE, CAST_TRIGGERED) == CAST_OK)
            {
                pFinder->SetRespawnDelay(urand(150, 200)); // Values are from Sniffs (rounded). Shortest and Longest respawn time from a finder on the same spot.
                pFinder->ForcedDespawn();
                finderCounter++;
            }
        }
        ResetTimer(EVENT_SHARD_MINION_SPAWNER_SMALL, 5000);
    }
};

/*
Minion Spawner
*/
struct MinionspawnerAI : public ScriptedAI
{
    MinionspawnerAI(Creature* creature) : ScriptedAI(creature)
    {
        AddCustomAction(EVENT_SPAWNER_SUMMON_MINION, 2000, 5000, [&]() // Spawn Minions every 5 seconds.
        {
            uint32 Entry = NPC_GHOUL_BERSERKER; // just in case.

            switch (m_creature->GetEntry())
            {
                case NPC_SCOURGE_INVASION_MINION_SPAWNER_GHOST_GHOUL:
                    Entry = UncommonMinionspawner(m_creature) ? PickRandomValue(NPC_SPIRIT_OF_THE_DAMNED, NPC_LUMBERING_HORROR) : PickRandomValue(NPC_SPECTRAL_SOLDIER, NPC_GHOUL_BERSERKER);
                    break;
                case NPC_SCOURGE_INVASION_MINION_SPAWNER_GHOST_SKELETON:
                    Entry = UncommonMinionspawner(m_creature) ? PickRandomValue(NPC_SPIRIT_OF_THE_DAMNED, NPC_BONE_WITCH) : PickRandomValue(NPC_SPECTRAL_SOLDIER, NPC_SKELETAL_SHOCKTROOPER);
                    break;
                case NPC_SCOURGE_INVASION_MINION_SPAWNER_GHOUL_SKELETON:
                    Entry = UncommonMinionspawner(m_creature) ? PickRandomValue(NPC_LUMBERING_HORROR, NPC_BONE_WITCH) : PickRandomValue(NPC_GHOUL_BERSERKER, NPC_SKELETAL_SHOCKTROOPER);
                    break;
            }
            if (Creature* pMinion = m_creature->SummonCreature(Entry, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), m_creature->GetOrientation(), TEMPSPAWN_TIMED_OR_DEAD_DESPAWN, IN_MILLISECONDS * HOUR, true))
            {
                pMinion->GetMotionMaster()->MoveRandomAroundPoint(pMinion->GetPositionX(), pMinion->GetPositionY(), pMinion->GetPositionZ(), 1.0f); // pMinion->SetWanderDistance(1.0f); // Seems to be very low.
                m_creature->CastSpell(nullptr, SPELL_MINION_SPAWN_IN, TRIGGERED_NONE);
            }
        });
        SetReactState(REACT_PASSIVE);
    }

    void Reset() {}
};

/*
npc_cultist_engineer
*/
struct npc_cultist_engineer : public ScriptedAI
{
    npc_cultist_engineer(Creature* creature) : ScriptedAI(creature)
    {
        AddCustomAction(EVENT_CULTIST_CHANNELING, false, [&]()
        {
            m_creature->CastSpell(nullptr, SPELL_BUTTRESS_CHANNEL, TRIGGERED_OLD_TRIGGERED);
        });
        SetReactState(REACT_PASSIVE);
    }

    void Reset() override {}

    void JustDied(Unit*) override
    {
        if (Creature* shard = GetClosestCreatureWithEntry(m_creature, NPC_DAMAGED_NECROTIC_SHARD, 15.0f))
        {
            shard->CastSpell(shard, SPELL_DAMAGE_CRYSTAL, TRIGGERED_OLD_TRIGGERED);
        }
        if (GameObject* gameObject = GetClosestGameObjectWithEntry(m_creature, GOBJ_SUMMONER_SHIELD, CONTACT_DISTANCE))
            gameObject->Delete();
    }

    void ReceiveAIEvent(uint32 eventType, Unit* /*sender*/, Unit* invoker, uint32 miscValue)
    {
        if (eventType == 7166 && miscValue == 0)
        {
            if (Player* player = dynamic_cast<Player*>(invoker))
            {
                // Player summons a Shadow of Doom for 1 hour.
                player->CastSpell(nullptr, SPELL_SUMMON_BOSS, TRIGGERED_OLD_TRIGGERED);
                m_creature->CastSpell(nullptr, SPELL_QUIET_SUICIDE, TRIGGERED_OLD_TRIGGERED);
            }
        }
        if (eventType == AI_EVENT_CUSTOM_A && miscValue == NPC_CULTIST_ENGINEER)
        {
            m_creature->SetCorpseDelay(10); // Corpse despawns 10 seconds after a Shadow of Doom spawns.
            m_creature->CastSpell(m_creature, SPELL_CREATE_SUMMONER_SHIELD, TRIGGERED_OLD_TRIGGERED);
            m_creature->CastSpell(m_creature, SPELL_MINION_SPAWN_IN, TRIGGERED_OLD_TRIGGERED);
            ResetTimer(EVENT_CULTIST_CHANNELING, 1000);
        }
    }
};

struct SummonBoss : public SpellScript
{
    virtual void OnSummon(Spell* spell, Creature* summon) const override
    {
        Unit* caster = spell->GetCaster();
        summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
        summon->SetFacingToObject(caster);
        summon->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, caster, summon, NPC_SHADOW_OF_DOOM);
        if (caster->IsPlayer())
            static_cast<Player*>(caster)->DestroyItemCount(ITEM_NECROTIC_RUNE, 8, true);
    }
};

/*
npc_minion
Notes: Shard Minions, Rares and Shadow of Doom.
*/
struct ScourgeMinion : public CombatAI
{
    ScourgeMinion(Creature* creature) : CombatAI(creature, 999)
    {
        switch (m_creature->GetEntry())
        {
            case NPC_SHADOW_OF_DOOM:
                AddCombatAction(EVENT_DOOM_MINDFLAY, 2000u);
                AddCombatAction(EVENT_DOOM_FEAR, 2000u);
                break;
            case NPC_FLAMESHOCKER:
                AddCombatAction(EVENT_MINION_FLAMESHOCKERS_TOUCH, 2000u);
                AddCustomAction(EVENT_MINION_FLAMESHOCKERS_DESPAWN, true, [&]()
                {
                    if (!m_creature->IsInCombat())
                        m_creature->CastSpell(m_creature, SPELL_DESPAWNER_SELF, TRIGGERED_OLD_TRIGGERED);
                    else
                        ResetTimer(EVENT_MINION_FLAMESHOCKERS_DESPAWN, 60000);
                });
                break;
        }
    }

    ObjectGuid m_summonerGuid;


    void ReceiveAIEvent(AIEventType eventType, Unit* /*sender*/, Unit* invoker, uint32 miscValue) override
    {
        if (!invoker)
            return;

        if (eventType == AI_EVENT_CUSTOM_A)
        {
            if (miscValue == NPC_SHADOW_OF_DOOM)
            {
                ResetTimer(EVENT_DOOM_START_ATTACK, 5000); // Remove Flag (immune to Players) after 5 seconds.
                // Pickup random emote like here: https://youtu.be/evOs9aJa2Jw?t=229
                DoBroadcastText(PickRandomValue(BCT_SHADOW_OF_DOOM_TEXT_0, BCT_SHADOW_OF_DOOM_TEXT_1, BCT_SHADOW_OF_DOOM_TEXT_2, BCT_SHADOW_OF_DOOM_TEXT_3), m_creature, invoker);
                m_creature->CastSpell(m_creature, SPELL_SPAWN_SMOKE, TRIGGERED_OLD_TRIGGERED);
            }
            if (miscValue == NPC_FLAMESHOCKER)
                ResetTimer(EVENT_MINION_FLAMESHOCKERS_DESPAWN, 60000);
        }
    }

    void JustDied(Unit* pKiller) override
    {
        switch (m_creature->GetEntry())
        {
            case NPC_SHADOW_OF_DOOM:
                m_creature->CastSpell(m_creature, SPELL_ZAP_CRYSTAL_CORPSE, TRIGGERED_OLD_TRIGGERED);
                break;
            case NPC_FLAMESHOCKER:
                m_creature->CastSpell(m_creature, SPELL_FLAMESHOCKERS_REVENGE, TRIGGERED_OLD_TRIGGERED);
                break;
        }
    }

    void SpellHit(Unit* unit, SpellEntry const* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_SPIRIT_SPAWN_OUT:
                m_creature->ForcedDespawn(3000);
                break;
        }
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        if (m_creature->GetEntry() == NPC_FLAMESHOCKER)
            if (pWho->IsCreature() && m_creature->IsWithinDistInMap(pWho, VISIBILITY_DISTANCE_TINY) && m_creature->IsWithinLOSInMap(pWho) && !pWho->GetVictim())
                if (IsGuardOrBoss(pWho) && pWho->AI())
                    pWho->AI()->AttackStart(m_creature);

        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case EVENT_DOOM_START_ATTACK:
            {
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
                // Shadow of Doom seems to attack the Summoner here.
                if (Player* player = m_creature->GetMap()->GetPlayer(m_summonerGuid))
                {
                    if (player->IsWithinLOSInMap(m_creature))
                    {
                        m_creature->SetInCombatWith(player);
                        m_creature->SetDetectionRange(2.0f);
                    }
                }
                break;
            }
            case EVENT_DOOM_MINDFLAY:
            {
                DoCastSpellIfCan(m_creature->GetVictim(), SPELL_MINDFLAY);
                ResetTimer(EVENT_DOOM_MINDFLAY, urand(6500, 13000));
                break;
            }
            case EVENT_DOOM_FEAR:
            {
                DoCastSpellIfCan(m_creature->GetVictim(), SPELL_FEAR);
                ResetTimer(EVENT_DOOM_FEAR, 14500);
                break;
            }
            case EVENT_MINION_FLAMESHOCKERS_TOUCH:
            {
                DoCastSpellIfCan(m_creature->GetVictim(), PickRandomValue(SPELL_FLAMESHOCKERS_TOUCH, SPELL_FLAMESHOCKERS_TOUCH2), CAST_TRIGGERED);
                ResetTimer(EVENT_MINION_FLAMESHOCKERS_TOUCH, urand(30000, 45000));
                break;
            }
        }
    }

    void UpdateAI(uint32 const diff) override
    {
        UpdateTimers(diff, m_creature->SelectHostileTarget());

        // Instakill every mob nearby, except Players, Pets or NPCs with the same faction.
        // m_creature->IsValidAttackTarget(m_creature->GetVictim(), true)
        if (m_creature->GetEntry() != NPC_FLAMESHOCKER && m_creature->IsWithinDistInMap(m_creature->GetVictim(), 30.0f) && !m_creature->GetVictim()->IsControlledByPlayer() && m_creature->CanAttack(m_creature->GetVictim()))
            DoCastSpellIfCan(m_creature->GetVictim(), SPELL_SCOURGE_STRIKE, CAST_TRIGGERED);

        DoMeleeAttackIfReady();
    }
};

struct PallidHorrorAI : public CombatAI
{
    std::set<ObjectGuid> m_flameshockers;

    PallidHorrorAI(Creature* creature) : CombatAI(creature, 999)
    {
        uint32 amount = urand(5, 9); // sniffed are group sizes of 5-9 shockers on spawn.

        if (m_creature->GetHealthPercent() == 100.0f)
        {
            for (uint32 i = 0; i < amount; ++i)
            {
                if (Creature* pFlameshocker = m_creature->SummonCreature(NPC_FLAMESHOCKER, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0.0f, TEMPSPAWN_TIMED_OR_DEAD_DESPAWN, HOUR * IN_MILLISECONDS, true))
                {
                    float angle = (float(i) * (M_PI / (amount / static_cast<float>(2)))) + m_creature->GetOrientation();
                    //pFlameshocker->JoinCreatureGroup(m_creature, 5.0f, angle - M_PI, OPTION_FORMATION_MOVE); // Perfect Circle around the Pallid.
                    pFlameshocker->CastSpell(pFlameshocker, SPELL_MINION_SPAWN_IN, TRIGGERED_OLD_TRIGGERED);
                    m_flameshockers.insert(pFlameshocker->GetObjectGuid());
                }
            }   
        }
        m_creature->SetCorpseDelay(10); // Corpse despawns 10 seconds after a crystal spawns.
        AddCombatAction(EVENT_PALLID_RANDOM_YELL, 5000u);
        AddCombatAction(EVENT_PALLID_SPELL_DAMAGE_VS_GUARDS, 5000u);
        AddCombatAction(EVENT_PALLID_SUMMON_FLAMESHOCKER, 5000u);
    }

    void Reset() override
    {
        CombatAI::Reset();
        //m_creature->AddAura(SPELL_AURA_OF_FEAR);
        m_creature->CastSpell(m_creature, SPELL_AURA_OF_FEAR, TRIGGERED_OLD_TRIGGERED);
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        if (pWho->IsCreature() && m_creature->IsWithinDistInMap(pWho, VISIBILITY_DISTANCE_TINY) && m_creature->IsWithinLOSInMap(pWho) && !pWho->GetVictim())
            if (IsGuardOrBoss(pWho) && pWho->AI())
                pWho->AI()->AttackStart(m_creature);

        ScriptedAI::MoveInLineOfSight(pWho);
    }

    void JustDied(Unit* unit) override
    {
        if (Creature* creature = GetClosestCreatureWithEntry(m_creature, NPC_HIGHLORD_BOLVAR_FORDRAGON, VISIBILITY_DISTANCE_NORMAL))
            DoBroadcastText(BCT_STORMWIND_BOLVAR_2, creature, m_creature, CHAT_TYPE_ZONE_YELL);

        if (Creature* creature = GetClosestCreatureWithEntry(m_creature, NPC_LADY_SYLVANAS_WINDRUNNER, VISIBILITY_DISTANCE_NORMAL))
            DoBroadcastText(BCT_UNDERCITY_SYLVANAS_1, creature, m_creature, CHAT_TYPE_ZONE_YELL);

        // Remove all custom summoned Flameshockers.
        auto flameshockers = m_flameshockers;
        for (const auto& guid : flameshockers)
            if (Creature* pFlameshocker = m_creature->GetMap()->GetCreature(guid))
                pFlameshocker->Suicide(); //pFlameshocker->DoKillUnit(pFlameshocker);

        m_creature->CastSpell(m_creature, (m_creature->GetZoneId() == ZONEID_UNDERCITY_A ? SPELL_SUMMON_FAINT_NECROTIC_CRYSTAL : SPELL_SUMMON_CRACKED_NECROTIC_CRYSTAL), TRIGGERED_OLD_TRIGGERED);
        m_creature->RemoveAurasDueToSpell(SPELL_AURA_OF_FEAR);

        TimePoint now = m_creature->GetMap()->GetCurrentClockTime();
        uint32 cityAttackTimer = urand(CITY_ATTACK_TIMER_MIN, CITY_ATTACK_TIMER_MAX);
        TimePoint nextAttack = now + std::chrono::milliseconds(cityAttackTimer);
        uint64 timeToNextAttack = (nextAttack - now).count();
        SITimers index = m_creature->GetZoneId() == ZONEID_UNDERCITY_A ? SI_TIMER_UNDERCITY : SI_TIMER_STORMWIND;
        sWorldState.SetSITimer(index, nextAttack);
        sWorldState.SetPallidGuid(index, ObjectGuid());
        sLog.outBasic("[Scourge Invasion Event] The Scourge has been defeated in %s, next attack starting in %d minutes", m_creature->GetZoneId() == ZONEID_UNDERCITY_A ? "Undercity" : "Stormwind", uint32(timeToNextAttack / 60));
    }

    void SummonedCreatureJustDied(Creature* unit) override
    {
        // Remove dead Flameshockers here to respawn them if needed.
        if (m_flameshockers.find(unit->GetObjectGuid()) != m_flameshockers.end())
            m_flameshockers.erase(unit->GetObjectGuid());
    }

    void SummonedCreatureDespawn(Creature* unit) override
    {
        // Remove despawned Flameshockers here to respawn them if needed.
        if (m_flameshockers.find(unit->GetObjectGuid()) != m_flameshockers.end())
            m_flameshockers.erase(unit->GetObjectGuid());
    }

    void OnRemoveFromWorld() override
    {
        // Remove all custom summoned Flameshockers.
        auto flameshockers = m_flameshockers;
        for (const auto& guid : flameshockers)
            if (Creature* pFlameshocker = m_creature->GetMap()->GetCreature(guid))
                pFlameshocker->AddObjectToRemoveList();
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case EVENT_PALLID_RANDOM_YELL:
            {
                DoBroadcastText(PickRandomValue(BCT_PALLID_HORROR_YELL1, BCT_PALLID_HORROR_YELL2, BCT_PALLID_HORROR_YELL3, BCT_PALLID_HORROR_YELL4,
                    BCT_PALLID_HORROR_YELL5, BCT_PALLID_HORROR_YELL6, BCT_PALLID_HORROR_YELL7, BCT_PALLID_HORROR_YELL8), m_creature, nullptr, CHAT_TYPE_ZONE_YELL);
                ResetTimer(EVENT_PALLID_RANDOM_YELL, urand(IN_MILLISECONDS * 65, IN_MILLISECONDS * 300));
                break;
            }
            case EVENT_PALLID_SPELL_DAMAGE_VS_GUARDS:
            {
                DoCastSpellIfCan(m_creature->GetVictim(), SPELL_DAMAGE_VS_GUARDS, CAST_TRIGGERED);
                ResetTimer(EVENT_PALLID_SPELL_DAMAGE_VS_GUARDS, urand(11000, 81000));
                break;
            }
            case EVENT_PALLID_SUMMON_FLAMESHOCKER:
            {
                if (m_flameshockers.size() < 30)
                {
                    if (Unit* target = SelectRandomFlameshockerSpawnTarget(m_creature, (Unit*) nullptr, DEFAULT_VISIBILITY_BG))
                    {
                        float x, y, z;
                        target->GetNearPoint(target, x, y, z, 5.0f, 5.0f, 0.0f);
                        if (Creature* pFlameshocker = m_creature->SummonCreature(NPC_FLAMESHOCKER, x, y, z, target->GetOrientation(), TEMPSPAWN_TIMED_OR_DEAD_DESPAWN, IN_MILLISECONDS * HOUR, true))
                        {
                            m_flameshockers.insert(pFlameshocker->GetObjectGuid());
                            pFlameshocker->CastSpell(pFlameshocker, SPELL_MINION_SPAWN_IN, TRIGGERED_OLD_TRIGGERED);
                            pFlameshocker->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, pFlameshocker, pFlameshocker, NPC_FLAMESHOCKER);
                        }
                    }
                }
                ResetTimer(EVENT_PALLID_SUMMON_FLAMESHOCKER, 2000);
                break;
            }
        }
    }
};

struct DespawnerSelf : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        Unit* caster = spell->GetCaster();
        if (!caster->IsInCombat())
            caster->CastSpell(nullptr, SPELL_SPIRIT_SPAWN_OUT, TRIGGERED_OLD_TRIGGERED);
    }
};

struct CommuniqueTrigger : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /*effIdx*/) const override
    {
        if (Unit* target = spell->GetUnitTarget())
            target->CastSpell(nullptr, SPELL_COMMUNIQUE_CAMP_TO_RELAY, TRIGGERED_OLD_TRIGGERED);
    }
};

void AddSC_scourge_invasion()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "scourge_invasion_necropolis";
    newscript->GetAI = &GetNewAIInstance<NecropolisAI>;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "scourge_invasion_mouth";
    newscript->GetAI = &GetNewAIInstance<MouthAI>;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "scourge_invasion_necropolis_health";
    newscript->GetAI = &GetNewAIInstance<NecropolisHealthAI>;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "scourge_invasion_necropolis_relay";
    newscript->GetAI = &GetNewAIInstance<NecropolisRelayAI>;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "scourge_invasion_necropolis_proxy";
    newscript->GetAI = &GetNewAIInstance<NecropolisProxyAI>;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "scourge_invasion_necrotic_shard";
    newscript->GetAI = &GetNewAIInstance<NecroticShard>;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "scourge_invasion_minion_spawner";
    newscript->GetAI = &GetNewAIInstance<MinionspawnerAI>;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "scourge_invasion_cultist_engineer";
    newscript->GetAI = &GetNewAIInstance<npc_cultist_engineer>;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "scourge_invasion_minion";
    newscript->GetAI = &GetNewAIInstance<ScourgeMinion>;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "scourge_invasion_go_circle";
    newscript->GetGameObjectAI = &GetNewAIInstance<GoCircle>;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "scourge_invasion_go_necropolis";
    newscript->GetGameObjectAI = &GetNewAIInstance<GoNecropolis>;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_pallid_horror";
    newscript->GetAI = &GetNewAIInstance<PallidHorrorAI>;
    newscript->RegisterSelf();

    RegisterSpellScript<SummonBoss>("spell_summon_boss");
    RegisterSpellScript<DespawnerSelf>("spell_despawner_self");
    RegisterSpellScript<CommuniqueTrigger>("spell_communique_trigger");
}
