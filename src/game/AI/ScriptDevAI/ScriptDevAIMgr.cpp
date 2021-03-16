/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
* This program is free software licensed under GPL version 2
* Please see the included DOCS/LICENSE.TXT for more information */

#include "AI/ScriptDevAI/include/sc_common.h"
#include "Policies/Singleton.h"
#include "Config/Config.h"
#include "Database/DatabaseEnv.h"
#include "Server/DBCStores.h"
#include "Globals/ObjectMgr.h"
#include "ProgressBar.h"
#include "system/system.h"
#include "ScriptDevAIMgr.h"
#include "include/sc_creature.h"
#include "Entities/Player.h"

#ifdef BUILD_SCRIPTDEV
#include "system/ScriptLoader.h"
#endif

INSTANTIATE_SINGLETON_1(ScriptDevAIMgr);

void FillSpellSummary();

void LoadDatabase()
{
    // Load content
    // pSystemMgr.LoadVersion(); // currently we are not checking for version; function to be completely removed in the future
    pSystemMgr.LoadScriptTexts();
    pSystemMgr.LoadScriptTextsCustom();
    pSystemMgr.LoadScriptGossipTexts();
    pSystemMgr.LoadScriptWaypoints();
}

//*********************************
//*** Functions used globally ***

/**
* Function that does script text
*
* @param iTextEntry Entry of the text, stored in SD2-database
* @param pSource Source of the text
* @param pTarget Can be nullptr (depending on CHAT_TYPE of iTextEntry). Possible target for the text
*/
void DoScriptText(int32 iTextEntry, WorldObject* pSource, Unit* pTarget)
{
    if (!pSource)
    {
        script_error_log("DoScriptText entry %i, invalid Source pointer.", iTextEntry);
        return;
    }

    if (iTextEntry >= 0)
    {
        script_error_log("DoScriptText with source entry %u (TypeId=%u, guid=%u) attempts to process text entry %i, but text entry must be negative.",
                         pSource->GetEntry(), pSource->GetTypeId(), pSource->GetGUIDLow(), iTextEntry);

        return;
    }

    DoDisplayText(pSource, iTextEntry, pTarget);
    // TODO - maybe add some call-stack like error output if above function returns false
}

/**
* Function that either simulates or does script text for a map
*
* @param iTextEntry Entry of the text, stored in SD2-database, only type CHAT_TYPE_ZONE_YELL supported
* @param uiCreatureEntry Id of the creature of whom saying will be simulated
* @param pMap Given Map on which the map-wide text is displayed
* @param pCreatureSource Can be nullptr. If pointer to Creature is given, then the creature does the map-wide text
* @param pTarget Can be nullptr. Possible target for the text
*/
void DoOrSimulateScriptTextForMap(int32 iTextEntry, uint32 uiCreatureEntry, Map* pMap, Creature* pCreatureSource /*=nullptr*/, Unit* pTarget /*=nullptr*/)
{
    if (!pMap)
    {
        script_error_log("DoOrSimulateScriptTextForMap entry %i, invalid Map pointer.", iTextEntry);
        return;
    }

    if (iTextEntry >= 0)
    {
        script_error_log("DoOrSimulateScriptTextForMap with source entry %u for map %u attempts to process text entry %i, but text entry must be negative.", uiCreatureEntry, pMap->GetId(), iTextEntry);
        return;
    }

    CreatureInfo const* pInfo = GetCreatureTemplateStore(uiCreatureEntry);
    if (!pInfo)
    {
        script_error_log("DoOrSimulateScriptTextForMap has invalid source entry %u for map %u.", uiCreatureEntry, pMap->GetId());
        return;
    }

    MangosStringLocale const* pData = GetMangosStringData(iTextEntry);
    if (!pData)
    {
        script_error_log("DoOrSimulateScriptTextForMap with source entry %u for map %u could not find text entry %i.", uiCreatureEntry, pMap->GetId(), iTextEntry);
        return;
    }

    debug_log("SD2: DoOrSimulateScriptTextForMap: text entry=%i, Sound=%u, Type=%u, Language=%u, Emote=%u",
              iTextEntry, pData->SoundId, pData->Type, pData->LanguageId, pData->Emote);

    if (pData->Type != CHAT_TYPE_ZONE_YELL && pData->Type != CHAT_TYPE_ZONE_EMOTE)
    {
        script_error_log("DoSimulateScriptTextForMap entry %i has not supported chat type %u.", iTextEntry, pData->Type);
        return;
    }

    if (pData->SoundId)
        pMap->PlayDirectSoundToMap(pData->SoundId);

    ChatMsg chatMsg = (pData->Type == CHAT_TYPE_ZONE_EMOTE ? CHAT_MSG_MONSTER_EMOTE : CHAT_MSG_MONSTER_YELL);

    if (pCreatureSource)                                // If provided pointer for sayer, use direct version
        pMap->MonsterYellToMap(pCreatureSource->GetObjectGuid(), iTextEntry, chatMsg, pData->LanguageId, pTarget);
    else                                                // Simulate yell
        pMap->MonsterYellToMap(pInfo, iTextEntry, chatMsg, pData->LanguageId, pTarget);
}

//*********************************
//*** Functions used internally ***

void Script::RegisterSelf(bool bReportError)
{
    if (uint32 id = sScriptDevAIMgr.GetScriptId(Name.c_str()))
        sScriptDevAIMgr.AddScript(id, this);
    else
    {
        if (bReportError)
            script_error_log("Script registering but ScriptName %s is not assigned in database. Script will not be used.", Name.c_str());

        delete this;
    }
}

//********************************
//*** Functions to be Exported ***

bool ScriptDevAIMgr::OnGossipHello(Player* pPlayer, Creature* pCreature)
{
    Script* pTempScript = GetScript(pCreature->GetScriptId());

    if (!pTempScript || !pTempScript->pGossipHello)
        return false;

    pPlayer->GetPlayerMenu()->ClearMenus();

    return pTempScript->pGossipHello(pPlayer, pCreature);
}

bool ScriptDevAIMgr::OnGossipHello(Player* pPlayer, GameObject* pGo)
{
    Script* pTempScript = GetScript(pGo->GetGOInfo()->ScriptId);

    if (!pTempScript || !pTempScript->pGossipHelloGO)
        return false;

    pPlayer->GetPlayerMenu()->ClearMenus();

    return pTempScript->pGossipHelloGO(pPlayer, pGo);
}

bool ScriptDevAIMgr::OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction, const char* code)
{
    debug_log("SD2: Gossip selection, sender: %u, action: %u", uiSender, uiAction);

    Script* pTempScript = GetScript(pCreature->GetScriptId());

    if (!pTempScript)
        return false;

    if (code)
    {
        if (!pTempScript->pGossipSelectWithCode)
            return false;

        pPlayer->GetPlayerMenu()->ClearMenus();
        return pTempScript->pGossipSelectWithCode(pPlayer, pCreature, uiSender, uiAction, code);
    }

    if (!pTempScript->pGossipSelect)
        return false;

    pPlayer->GetPlayerMenu()->ClearMenus();
    return pTempScript->pGossipSelect(pPlayer, pCreature, uiSender, uiAction);
}

bool ScriptDevAIMgr::OnGossipSelect(Player* pPlayer, GameObject* pGo, uint32 uiSender, uint32 uiAction, const char* code)
{
    debug_log("SD2: GO Gossip selection, sender: %u, action: %u", uiSender, uiAction);

    Script* pTempScript = GetScript(pGo->GetGOInfo()->ScriptId);

    if (!pTempScript)
        return false;

    if (code)
    {
        if (!pTempScript->pGossipSelectGOWithCode)
            return false;

        pPlayer->GetPlayerMenu()->ClearMenus();
        return pTempScript->pGossipSelectGOWithCode(pPlayer, pGo, uiSender, uiAction, code);
    }

    if (!pTempScript->pGossipSelectGO)
        return false;

    pPlayer->GetPlayerMenu()->ClearMenus();
    return pTempScript->pGossipSelectGO(pPlayer, pGo, uiSender, uiAction);
}

uint32 ScriptDevAIMgr::GetDialogStatus(const Player* pPlayer, const Creature* pCreature) const
{
    Script* pTempScript = GetScript(pCreature->GetScriptId());

    if (!pTempScript || !pTempScript->pDialogStatusNPC)
        return DIALOG_STATUS_UNDEFINED;

    pPlayer->GetPlayerMenu()->ClearMenus();

    return pTempScript->pDialogStatusNPC(pPlayer, pCreature);
}

uint32 ScriptDevAIMgr::GetDialogStatus(const Player* pPlayer, const GameObject* pGo) const
{
    Script* pTempScript = GetScript(pGo->GetGOInfo()->ScriptId);

    if (!pTempScript || !pTempScript->pDialogStatusGO)
        return DIALOG_STATUS_UNDEFINED;

    pPlayer->GetPlayerMenu()->ClearMenus();

    return pTempScript->pDialogStatusGO(pPlayer, pGo);
}

bool ScriptDevAIMgr::OnQuestAccept(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    Script* pTempScript = GetScript(pCreature->GetScriptId());

    if (!pTempScript || !pTempScript->pQuestAcceptNPC)
        return false;

    pPlayer->GetPlayerMenu()->ClearMenus();

    return pTempScript->pQuestAcceptNPC(pPlayer, pCreature, pQuest);
}

bool ScriptDevAIMgr::OnQuestAccept(Player* pPlayer, GameObject* pGo, const Quest* pQuest)
{
    Script* pTempScript = GetScript(pGo->GetGOInfo()->ScriptId);

    if (!pTempScript || !pTempScript->pQuestAcceptGO)
        return false;

    pPlayer->GetPlayerMenu()->ClearMenus();

    return pTempScript->pQuestAcceptGO(pPlayer, pGo, pQuest);
}

bool ScriptDevAIMgr::OnQuestAccept(Player* pPlayer, Item* pItem, Quest const* pQuest)
{
    Script* pTempScript = GetScript(pItem->GetProto()->ScriptId);

    if (!pTempScript || !pTempScript->pQuestAcceptItem)
        return false;

    pPlayer->GetPlayerMenu()->ClearMenus();

    return pTempScript->pQuestAcceptItem(pPlayer, pItem, pQuest);
}

bool ScriptDevAIMgr::OnGameObjectUse(Player* pPlayer, GameObject* pGo)
{
    Script* pTempScript = GetScript(pGo->GetGOInfo()->ScriptId);

    if (!pTempScript || !pTempScript->pGOUse)
        return false;

    return pTempScript->pGOUse(pPlayer, pGo);
}

std::function<bool(Unit*)>* ScriptDevAIMgr::OnTrapSearch(GameObject* go)
{
    Script* pTempScript = GetScript(go->GetGOInfo()->ScriptId);

    if (!pTempScript)
        return nullptr;

    return pTempScript->pTrapSearching;
}

bool ScriptDevAIMgr::OnQuestRewarded(Player* pPlayer, Creature* pCreature, Quest const* pQuest)
{
    Script* pTempScript = GetScript(pCreature->GetScriptId());

    if (!pTempScript || !pTempScript->pQuestRewardedNPC)
        return false;

    pPlayer->GetPlayerMenu()->ClearMenus();

    return pTempScript->pQuestRewardedNPC(pPlayer, pCreature, pQuest);
}

bool ScriptDevAIMgr::OnQuestRewarded(Player* pPlayer, GameObject* pGo, Quest const* pQuest)
{
    Script* pTempScript = GetScript(pGo->GetGOInfo()->ScriptId);

    if (!pTempScript || !pTempScript->pQuestRewardedGO)
        return false;

    pPlayer->GetPlayerMenu()->ClearMenus();

    return pTempScript->pQuestRewardedGO(pPlayer, pGo, pQuest);
}

bool ScriptDevAIMgr::OnAreaTrigger(Player* pPlayer, AreaTriggerEntry const* atEntry)
{
    Script* pTempScript = GetScript(GetAreaTriggerScriptId(atEntry->id));

    if (!pTempScript || !pTempScript->pAreaTrigger)
        return false;

    return pTempScript->pAreaTrigger(pPlayer, atEntry);
}

bool ScriptDevAIMgr::OnProcessEvent(uint32 uiEventId, Object* pSource, Object* pTarget, bool bIsStart)
{
    Script* pTempScript = GetScript(GetEventIdScriptId(uiEventId));

    if (!pTempScript || !pTempScript->pProcessEventId)
        return false;

    // bIsStart may be false, when event is from taxi node events (arrival=false, departure=true)
    return pTempScript->pProcessEventId(uiEventId, pSource, pTarget, bIsStart);
}

UnitAI* ScriptDevAIMgr::GetCreatureAI(Creature* pCreature) const
{
    Script* pTempScript = GetScript(pCreature->GetScriptId());

    if (!pTempScript || !pTempScript->GetAI)
        return nullptr;

    return pTempScript->GetAI(pCreature);
}

GameObjectAI* ScriptDevAIMgr::GetGameObjectAI(GameObject* gameobject) const
{
    Script* pTempScript = GetScript(gameobject->GetScriptId());

    if (!pTempScript || !pTempScript->GetGameObjectAI)
        return nullptr;

    return pTempScript->GetGameObjectAI(gameobject);
}

bool ScriptDevAIMgr::OnItemUse(Player* pPlayer, Item* pItem, SpellCastTargets const& targets)
{
    Script* pTempScript = GetScript(pItem->GetProto()->ScriptId);

    if (!pTempScript || !pTempScript->pItemUse)
        return false;

    return pTempScript->pItemUse(pPlayer, pItem, targets);
}

bool ScriptDevAIMgr::OnItemLoot(Player* pPlayer, Item* pItem, bool apply)
{
    Script* pTempScript = GetScript(pItem->GetProto()->ScriptId);

    if (!pTempScript || !pTempScript->pItemLoot)
        return false;

    return pTempScript->pItemLoot(pPlayer, pItem, apply);
}

bool ScriptDevAIMgr::OnEffectDummy(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, Creature* pTarget, ObjectGuid originalCasterGuid)
{
    Script* pTempScript = GetScript(pTarget->GetScriptId());

    if (!pTempScript || !pTempScript->pEffectDummyNPC)
        return false;

    return pTempScript->pEffectDummyNPC(pCaster, spellId, effIndex, pTarget, originalCasterGuid);
}

bool ScriptDevAIMgr::OnEffectDummy(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, GameObject* pTarget, ObjectGuid originalCasterGuid)
{
    Script* pTempScript = GetScript(pTarget->GetGOInfo()->ScriptId);

    if (!pTempScript || !pTempScript->pEffectDummyGO)
        return false;

    return pTempScript->pEffectDummyGO(pCaster, spellId, effIndex, pTarget, originalCasterGuid);
}

bool ScriptDevAIMgr::OnEffectDummy(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, Item* pTarget, ObjectGuid originalCasterGuid)
{
    Script* pTempScript = GetScript(pTarget->GetProto()->ScriptId);

    if (!pTempScript || !pTempScript->pEffectDummyItem)
        return false;

    return pTempScript->pEffectDummyItem(pCaster, spellId, effIndex, pTarget, originalCasterGuid);
}

bool ScriptDevAIMgr::OnEffectScriptEffect(Unit* pCaster, uint32 spellId, SpellEffectIndex effIndex, Creature* pTarget, ObjectGuid originalCasterGuid)
{
    Script* pTempScript = GetScript(pTarget->GetScriptId());

    if (!pTempScript || !pTempScript->pEffectScriptEffectNPC)
        return false;

    return pTempScript->pEffectScriptEffectNPC(pCaster, spellId, effIndex, pTarget, originalCasterGuid);
}

bool ScriptDevAIMgr::OnAuraDummy(Aura const* pAura, bool bApply)
{
    Script* pTempScript = GetScript(((Creature*)pAura->GetTarget())->GetScriptId());

    if (!pTempScript || !pTempScript->pEffectAuraDummy)
        return false;

    return pTempScript->pEffectAuraDummy(pAura, bApply);
}

InstanceData* ScriptDevAIMgr::CreateInstanceData(Map* pMap)
{
    Script* pTempScript = GetScript(pMap->GetScriptId());

    if (!pTempScript || !pTempScript->GetInstanceData)
        return nullptr;

    return pTempScript->GetInstanceData(pMap);
}

ScriptDevAIMgr::~ScriptDevAIMgr()
{
    // Free resources before library unload
    for (SDScriptVec::const_iterator itr = m_scripts.begin(); itr != m_scripts.end(); ++itr)
        delete *itr;

    m_scripts.clear();

    num_sc_scripts = 0;

    setScriptLibraryErrorFile(nullptr, nullptr);
}

void ScriptDevAIMgr::AddScript(uint32 id, Script* script)
{
    if (!id)
        return;

    m_scripts[id] = script;
    ++num_sc_scripts;
}

Script* ScriptDevAIMgr::GetScript(uint32 id) const
{
#ifdef BUILD_SCRIPTDEV
    if (!id || id < m_scripts.size())
        return m_scripts[id];
#endif
    return nullptr;
}

void ScriptDevAIMgr::Initialize()
{
#ifdef BUILD_SCRIPTDEV
    // ScriptDev startup
    outstring_log("  _____           _       _   _____ ");
    outstring_log(" / ____|         (_)     | | |  __ \\");
    outstring_log("| (___   ___ _ __ _ _ __ | |_| |  | | _____   __");
    outstring_log(" \\___ \\ / __| '__| | '_ \\| __| |  | |/ _ \\ \\ / /");
    outstring_log(" ____) | (__| |  | | |_) | |_| |__| |  __/\\ V / ");
    outstring_log("|_____/ \\___|_|  |_| .__/ \\__|_____/ \\___| \\_/ ");
    outstring_log("                   | |");
    outstring_log("                   |_| http://www.cmangos.net");



    // Load database (must be called after SD2Config.SetSource).
    LoadDatabase();

    outstring_log("SD2: Loading C++ scripts\n");
    BarGoLink bar(1);
    bar.step();

    // Resize script ids to needed amount of assigned ScriptNames (from core)
    m_scripts.resize(GetScriptIdsCount(), nullptr);

    FillSpellSummary();

    AddScripts();

    // Check existence scripts for all registered by core script names
    for (uint32 i = 1; i < GetScriptIdsCount(); ++i)
    {
        if (!m_scripts[i])
            script_error_log("No script found for ScriptName '%s'.", GetScriptName(i));
    }

    outstring_log(">> Loaded %i C++ Scripts.", num_sc_scripts);
#else
    outstring_log(">> ScriptDev is disabled!\n");
#endif
}

void ScriptDevAIMgr::LoadScriptNames()
{
    m_scriptNames.push_back("");
    QueryResult* result = WorldDatabase.Query(
                              "SELECT DISTINCT(ScriptName) FROM creature_template WHERE ScriptName <> '' "
                              "UNION "
                              "SELECT DISTINCT(ScriptName) FROM gameobject_template WHERE ScriptName <> '' "
                              "UNION "
                              "SELECT DISTINCT(ScriptName) FROM item_template WHERE ScriptName <> '' "
                              "UNION "
                              "SELECT DISTINCT(ScriptName) FROM scripted_areatrigger WHERE ScriptName <> '' "
                              "UNION "
                              "SELECT DISTINCT(ScriptName) FROM scripted_event_id WHERE ScriptName <> '' "
                              "UNION "
                              "SELECT DISTINCT(ScriptName) FROM instance_template WHERE ScriptName <> '' "
                              "UNION "
                              "SELECT DISTINCT(ScriptName) FROM world_template WHERE ScriptName <> ''");

    if (!result)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outErrorDb(">> Loaded empty set of Script Names!");
        sLog.outString();
        return;
    }

    BarGoLink bar(result->GetRowCount());
    uint32 count = 0;

    do
    {
        bar.step();
        m_scriptNames.push_back((*result)[0].GetString());
        ++count;
    }
    while (result->NextRow());
    delete result;

    std::sort(m_scriptNames.begin(), m_scriptNames.end());

    sLog.outString(">> Loaded %d Script Names", count);
    sLog.outString();
}

uint32 ScriptDevAIMgr::GetScriptId(const char* name) const
{
    // use binary search to find the script name in the sorted vector
    // assume "" is the first element
    if (!name)
        return 0;

    ScriptNameMap::const_iterator itr = std::lower_bound(m_scriptNames.begin(), m_scriptNames.end(), name);

    if (itr == m_scriptNames.end() || *itr != name)
        return 0;

    return uint32(itr - m_scriptNames.begin());
}

void ScriptDevAIMgr::LoadAreaTriggerScripts()
{
    m_AreaTriggerScripts.clear();                           // need for reload case
    QueryResult* result = WorldDatabase.Query("SELECT entry, ScriptName FROM scripted_areatrigger");

    uint32 count = 0;

    if (!result)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outString(">> Loaded %u scripted areatrigger", count);
        sLog.outString();
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        ++count;
        bar.step();

        Field* fields = result->Fetch();

        uint32 triggerId = fields[0].GetUInt32();
        const char* scriptName = fields[1].GetString();

        if (!sAreaTriggerStore.LookupEntry(triggerId))
        {
            sLog.outErrorDb("Table `scripted_areatrigger` has area trigger (ID: %u) not listed in `AreaTrigger.dbc`.", triggerId);
            continue;
        }

        m_AreaTriggerScripts[triggerId] = GetScriptId(scriptName);
    }
    while (result->NextRow());

    delete result;

    sLog.outString(">> Loaded %u areatrigger scripts", count);
    sLog.outString();

}

void ScriptDevAIMgr::LoadEventIdScripts()
{
    m_EventIdScripts.clear();                           // need for reload case
    QueryResult* result = WorldDatabase.Query("SELECT id, ScriptName FROM scripted_event_id");

    uint32 count = 0;

    if (!result)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outString(">> Loaded %u scripted event id", count);
        sLog.outString();
        return;
    }

    BarGoLink bar(result->GetRowCount());

    std::set<uint32> eventIds;                              // Store possible event ids
    ScriptMgr::CollectPossibleEventIds(eventIds);

    do
    {
        ++count;
        bar.step();

        Field* fields = result->Fetch();

        uint32 eventId = fields[0].GetUInt32();
        const char* scriptName = fields[1].GetString();

        std::set<uint32>::const_iterator itr = eventIds.find(eventId);
        if (itr == eventIds.end())
            sLog.outErrorDb("Table `scripted_event_id` has id %u not referring to any gameobject_template type 10 data2 field, type 3 data6 field, type 13 data 2 field, type 29 or any spell effect %u or path taxi node data",
                            eventId, SPELL_EFFECT_SEND_EVENT);

        m_EventIdScripts[eventId] = GetScriptId(scriptName);
    }
    while (result->NextRow());

    delete result;

    sLog.outString(">> Loaded %u scripted event id", count);
    sLog.outString();
}

uint32 ScriptDevAIMgr::GetAreaTriggerScriptId(uint32 triggerId) const
{
    AreaTriggerScriptMap::const_iterator itr = m_AreaTriggerScripts.find(triggerId);
    if (itr != m_AreaTriggerScripts.end())
        return itr->second;

    return 0;
}

uint32 ScriptDevAIMgr::GetEventIdScriptId(uint32 eventId) const
{
    EventIdScriptMap::const_iterator itr = m_EventIdScripts.find(eventId);
    if (itr != m_EventIdScripts.end())
        return itr->second;

    return 0;
}
