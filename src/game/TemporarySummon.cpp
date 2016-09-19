/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
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

#include "TemporarySummon.h"
#include "Log.h"
#include "AI/CreatureAI.h"

TemporarySummon::TemporarySummon(ObjectGuid summoner) :
    Creature(CREATURE_SUBTYPE_TEMPORARY_SUMMON), m_type(TEMPSUMMON_TIMED_OOC_OR_CORPSE_DESPAWN), m_timer(0), m_lifetime(0), m_summoner(summoner), m_linkedToOwnerAura(0)
{
}

void TemporarySummon::Update(uint32 update_diff,  uint32 diff)
{
    switch (m_type)
    {
        case TEMPSUMMON_MANUAL_DESPAWN:
            break;
        case TEMPSUMMON_TIMED_DESPAWN:
        {
            if (m_timer <= update_diff)
            {
                UnSummon();
                return;
            }

            m_timer -= update_diff;
            break;
        }
        case TEMPSUMMON_TIMED_OOC_DESPAWN:
        {
            if (!isInCombat())
            {
                if (m_timer <= update_diff)
                {
                    UnSummon();
                    return;
                }

                m_timer -= update_diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;

            break;
        }

        case TEMPSUMMON_CORPSE_TIMED_DESPAWN:
        {
            if (IsCorpse())
            {
                if (m_timer <= update_diff)
                {
                    UnSummon();
                    return;
                }

                m_timer -= update_diff;
            }
            if (IsDespawned())
            {
                UnSummon();
                return;
            }
            break;
        }
        case TEMPSUMMON_CORPSE_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if (isDead())
            {
                UnSummon();
                return;
            }

            break;
        }
        case TEMPSUMMON_DEAD_DESPAWN:
        {
            if (IsDespawned())
            {
                UnSummon();
                return;
            }
            break;
        }
        case TEMPSUMMON_TIMED_OOC_OR_CORPSE_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if (isDead())
            {
                UnSummon();
                return;
            }

            if (!isInCombat())
            {
                if (m_timer <= update_diff)
                {
                    UnSummon();
                    return;
                }
                else
                    m_timer -= update_diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;
            break;
        }
        case TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if (IsDespawned())
            {
                UnSummon();
                return;
            }

            if (!isInCombat() && isAlive())
            {
                if (m_timer <= update_diff)
                {
                    UnSummon();
                    return;
                }
                else
                    m_timer -= update_diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;
            break;
        }
        case TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if (isDead())
            {
                UnSummon();
                return;
            }
            if (m_timer <= update_diff)
            {
                UnSummon();
                return;
            }
            m_timer -= update_diff;
            break;
        }
        case TEMPSUMMON_TIMED_OR_DEAD_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if (IsDespawned())
            {
                UnSummon();
                return;
            }
            if (m_timer <= update_diff)
            {
                UnSummon();
                return;
            }
            m_timer -= update_diff;
            break;
        }
        default:
            UnSummon();
            sLog.outError("Temporary summoned creature (entry: %u) have unknown type %u of ", GetEntry(), m_type);
            break;
    }

    switch (m_deathState)
    {
        case ALIVE:
            if (m_linkedToOwnerAura & TEMPSUMMON_LINKED_AURA_OWNER_CHECK)
            {
                // we have to check if owner still have the required aura
                Unit* owner = GetCharmerOrOwner();
                uint32 const& spellId = GetUInt32Value(UNIT_CREATED_BY_SPELL);
                if (!owner || !spellId || !owner->HasAura(spellId))
                    UnSummon();
            }
            break;

        case DEAD:
        case CORPSE:
            if (m_linkedToOwnerAura & TEMPSUMMON_LINKED_AURA_REMOVE_OWNER)
            {
                RemoveAuraFromOwner();
                m_linkedToOwnerAura = 0;                    // we dont need to recheck
            }

        default:
            break;
    }

    Creature::Update(update_diff, diff);
}

void TemporarySummon::SetSummonProperties(TempSummonType type, uint32 lifetime)
{
    m_type = type;
    m_timer = lifetime;
    m_lifetime = lifetime;
}

void TemporarySummon::Summon(TempSummonType type, uint32 lifetime)
{
    SetSummonProperties(type, lifetime);

    GetMap()->Add((Creature*)this);

    AIM_Initialize();
}

void TemporarySummon::UnSummon()
{
    CombatStop();

    if (m_linkedToOwnerAura & TEMPSUMMON_LINKED_AURA_REMOVE_OWNER)
        RemoveAuraFromOwner();

    if (GetSummonerGuid().IsCreature())
    {
        if (Creature* sum = GetMap()->GetCreature(GetSummonerGuid()))
            if (sum->AI())
                sum->AI()->SummonedCreatureDespawn(this);
    }
    else if (GetSummonerGuid().IsPlayer()) // if player that summoned this creature was MCing it, uncharm
        if (Player* player = GetMap()->GetPlayer(GetSummonerGuid()))
            if (player->GetMover() == this)
                player->Uncharm();

    if (AI())
        AI()->SummonedCreatureDespawn(this);

    AddObjectToRemoveList();
}

void TemporarySummon::RemoveAuraFromOwner()
{
    // creature is dead and we have to remove the charmer aura if exist
    uint32 const& spellId = GetUInt32Value(UNIT_CREATED_BY_SPELL);
    if (spellId)
    {

        if (Unit* charmer = GetCharmer())
        {
            charmer->RemoveAurasDueToSpell(spellId);
            charmer->ResetControlState(false);
        }
        else if (Unit* owner = GetOwner())
        {
            owner->RemoveAurasDueToSpell(spellId);
        }
    }
}

void TemporarySummon::SaveToDB()
{
}

TemporarySummonWaypoint::TemporarySummonWaypoint(ObjectGuid summoner, uint32 waypoint_id, int32 path_id, uint32 pathOrigin) :
    TemporarySummon(summoner),
    m_waypoint_id(waypoint_id),
    m_path_id(path_id),
    m_pathOrigin(pathOrigin)
{
}
