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

#include "SpellScript.h"
#include "Entities/DynamicObject.h"

std::map<uint32, SpellScript*> SpellScriptMgr::m_spellScriptMap {};
std::map<uint32, AuraScript*> SpellScriptMgr::m_auraScriptMap {};

std::map<std::string, std::unique_ptr<SpellScript>> SpellScriptMgr::m_spellScriptStringMap {};
std::map<std::string, std::unique_ptr<AuraScript>> SpellScriptMgr::m_auraScriptStringMap {};

SpellScript* SpellScriptMgr::GetSpellScript(uint32 spellId)
{
    auto itr = m_spellScriptMap.find(spellId);
    if (itr != m_spellScriptMap.end())
        return (*itr).second;
    return nullptr;
}

AuraScript* SpellScriptMgr::GetAuraScript(uint32 spellId)
{
    auto itr = m_auraScriptMap.find(spellId);
    if (itr != m_auraScriptMap.end())
        return (*itr).second;
    return nullptr;
}

void SpellScriptMgr::SetSpellScript(std::string scriptName, SpellScript* script)
{
    m_spellScriptStringMap.emplace(scriptName, script);
}

void SpellScriptMgr::SetAuraScript(std::string scriptName, AuraScript* script)
{
    m_auraScriptStringMap.emplace(scriptName, script);
}

extern void LoadDruidScripts();
extern void LoadHunterScripts();
extern void LoadMageScripts();
extern void LoadPaladinScripts();
extern void LoadPriestScripts();
extern void LoadRogueScripts();
extern void LoadShamanScripts();
extern void LoadWarlockScripts();
extern void LoadWarriorScripts();

void SpellScriptMgr::LoadScripts()
{
    // load all scripts with names
    LoadDruidScripts();
    LoadHunterScripts();
    LoadMageScripts();
    LoadPaladinScripts();
    LoadPriestScripts();
    LoadRogueScripts();
    LoadShamanScripts();
    LoadWarlockScripts();
    LoadWarriorScripts();

    // load names from DB and pair names to Ids
    std::unique_ptr<QueryResult> result(WorldDatabase.Query("SELECT Id, ScriptName FROM spell_scripts"));
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 spellId = fields[0].GetUInt32();
            std::string scriptName = fields[1].GetCppString();

            if (!sSpellTemplate.LookupEntry<SpellEntry>(spellId))
            {
                sLog.outErrorDb("Spell %u has script %s in spell_scripts but spell does not exist. Skipping.", spellId, scriptName.data());
                continue;
            }

            bool found = false;
            if (SpellScript* script = GetSpellScript(scriptName))
                SetSpellScript(spellId, script), found = true;
            if (AuraScript* script = GetAuraScript(scriptName))
                SetAuraScript(spellId, script), found = true;

            if (!found)
            {
                sLog.outErrorDb("Spell %u has script %s in spell_scripts but script does not exist. Skipping.", spellId, scriptName.data());
                continue;
            }
        } while (result->NextRow());
    }
}

SpellScript* SpellScriptMgr::GetSpellScript(std::string scriptName)
{
    auto itr = m_spellScriptStringMap.find(scriptName);
    if (itr != m_spellScriptStringMap.end())
        return (*itr).second.get();
    return nullptr;
}

AuraScript* SpellScriptMgr::GetAuraScript(std::string scriptName)
{
    auto itr = m_auraScriptStringMap.find(scriptName);
    if (itr != m_auraScriptStringMap.end())
        return (*itr).second.get();
    return nullptr;
}

void SpellScriptMgr::SetSpellScript(uint32 spellId, SpellScript* script)
{
    m_spellScriptMap.emplace(spellId, script);
}

void SpellScriptMgr::SetAuraScript(uint32 spellId, AuraScript* script)
{
    m_auraScriptMap.emplace(spellId, script);
}
