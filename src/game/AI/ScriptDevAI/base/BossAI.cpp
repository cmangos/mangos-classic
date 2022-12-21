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

#include "AI/ScriptDevAI/include/sc_creature.h"
#include "AI/ScriptDevAI/include/sc_instance.h"
#include "Entities/Creature.h"
#include "AI/ScriptDevAI/base/BossAI.h"
#include "Spells/Spell.h"
#include "Spells/SpellMgr.h"
#include "AI/ScriptDevAI/ScriptDevAIMgr.h"

void BossAI::AddOnDeathText(uint32 text)
{
    m_onKilledTexts.push_back(text);
}

void BossAI::AddOnAggroText(uint32 text)
{
    m_onAggroTexts.push_back(text);
}

void BossAI::Reset()
{
    CombatAI::Reset();
    m_creature->SetSpellList(m_creature->GetCreatureInfo()->SpellList);
}

void BossAI::JustDied(Unit* killer)
{
    CombatAI::JustDied(killer);
    if (!m_onKilledTexts.empty())
        DoBroadcastText(m_onKilledTexts[urand(0, m_onKilledTexts.size() - 1)], m_creature, killer);
    for (QueuedCast& cast : m_castOnDeath)
    {
        Unit* target = m_creature->GetMap()->GetUnit(cast.target);
        m_creature->CastSpell(target, cast.spellId, cast.flags);
    }
    if (m_instanceDataType == -1)
        return;
    if (ScriptedInstance* instance = static_cast<ScriptedInstance*>(m_creature->GetInstanceData()))
        instance->SetData(m_instanceDataType, DONE);
    OpenEntrances();
    OpenExits();
}

void BossAI::JustReachedHome()
{
    if (m_instanceDataType == -1)
        return;
    if (ScriptedInstance* instance = static_cast<ScriptedInstance*>(m_creature->GetInstanceData()))
        instance->SetData(m_instanceDataType, FAIL);
    OpenEntrances();
    Reset();
}

void BossAI::Aggro(Unit* who)
{
    m_combatStartTimestamp = std::chrono::steady_clock::now();

    if (!m_onAggroTexts.empty())
        DoBroadcastText(m_onAggroTexts[urand(0, m_onAggroTexts.size() - 1)], m_creature, who);
    if (m_instanceDataType == -1)
        return;
    ScriptedInstance* instance = static_cast<ScriptedInstance*>(m_creature->GetInstanceData());
    if (!instance)
        return;
    instance->SetData(m_instanceDataType, IN_PROGRESS);
    ResetTimer(INSTANCE_CLOSE_ENTRANCE_DOOR, m_gateDelay);
}

void BossAI::AddEntranceObject(uint32 value)
{
    m_entranceObjects.push_back(value);
}

void BossAI::AddExitObject(uint32 value)
{
    m_exitObjects.push_back(value);
}

void BossAI::EnterEvadeMode()
{
    if (m_instanceDataType == -1)
        return;
    if (ScriptedInstance* instance = static_cast<ScriptedInstance*>(m_creature->GetInstanceData()))
        instance->SetData(m_instanceDataType, FAIL);
    OpenEntrances();
    if (m_respawnDelay == -1)
    {
        CombatAI::EnterEvadeMode();
        return;
    }
    m_creature->SetRespawnDelay(m_respawnDelay, true);
    m_creature->ForcedDespawn();
}

void BossAI::AddCastOnDeath(QueuedCast cast)
{
    m_castOnDeath.push_back(cast);
}

void BossAI::AddRespawnOnEvade(std::chrono::seconds delay)
{
    m_respawnDelay = delay.count();
}