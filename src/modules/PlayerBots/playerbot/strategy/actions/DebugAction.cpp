#include "playerbot/playerbot.h"
#include "DebugAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include <playerbot/TravelNode.h>
#include "ChooseTravelTargetAction.h"
#include "playerbot/strategy/values/SharedValueContext.h"
#include "playerbot/strategy/actions/RpgSubActions.h"
#include "playerbot/LootObjectStack.h"
#include "GameEvents/GameEventMgr.h"
#include "playerbot/TravelMgr.h"
#include "playerbot/PlayerbotHelpMgr.h"
#include "Entities/Transports.h"
#include "MotionGenerators/PathFinder.h"
#include "playerbot/PlayerbotLLMInterface.h"

#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

#include <iomanip>
#include "SayAction.h"

using namespace ai;
using namespace MaNGOS;

bool DebugAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    bool isMod = event.getSource() == ".bot" || (event.getOwner() && event.getOwner()->GetSession() && event.getOwner()->GetSession()->GetSecurity() >= SEC_MODERATOR);

    if (!requester)
    {
        requester = bot;
    }

    std::string text = event.getParam();
    if (text == "avoid scan" && isMod)
        return HandleAvoidScan(event, requester, text);
    else if (text.find("avoid add") == 0 && isMod)
        return HandleAvoidAdd(event, requester, text);
    else if (text.find("mount") == 0)
        return HandleMount(event, requester, text);
    else if (text.find("unmount") == 0)
        return HandleUnmount(event, requester, text);
    else if (text.find("area") == 0 && isMod)
        return HandleArea(event, requester, text);
    else if (text.find("llm ") == 0 && isMod)
        return HandleLLM(event, requester, text);
    else if (text.find("chatreplydo ") == 0 && isMod)
        return HandleChatReplyDo(event, requester, text);
    else if (text.find("monstertalk ") == 0 && isMod)
        return HandleMonsterTalk(event, requester, text);
    else if (text == "gy" && isMod)
        return HandleGY(event, requester, text);
    else if (text == "grid" && isMod)
        return HandleGrid(event, requester, text);
    else if (text.find("test") == 0 && isMod)
        return HandleTest(event, requester, text);
    else if (text.find("values") == 0)
        return HandleValues(event, requester, text);
    else if (text == "gps" && isMod)
        return HandleGPS(event, requester, text);
    else if (text.find("setvalueuin32 ") == 0)
        return HandleSetValueUInt32(event, requester, text);
    else if (text.find("do ") == 0)
        return HandleDo(event, requester, text);
    else if (text.find("trade ") == 0)
        return HandleTrade(event, requester, text);
    else if (text.find("trade") == 0)
        return HandleTrade(event, requester, text);
    else if (text.find("mail") == 0)
        return HandleMail(event, requester, text);
    else if (text.find("poi ") == 0)
        return HandlePOI(event, requester, text);
    else if (text.find("motion") == 0 && isMod)
        return HandleMotion(event, requester, text);
    else if (text.find("transport") == 0 && isMod)
        return HandleTransport(event, requester, text);
    else if (text.find("ontrans") == 0 && isMod)
        return HandleOnTrans(event, requester, text);
    else if (text.find("offtrans") == 0 && isMod)
        return HandleOffTrans(event, requester, text);
    else if (text.find("pathable") == 0 && isMod)
        return HandlePathable(event, requester, text);
    else if (text.find("randomspot") == 0 && isMod)
        return HandleRandomSpot(event, requester, text);
    else if (text.find("printmap") == 0 && isMod)
        return HandlePrintMap(event, requester, text);
    else if (text.find("corpse") == 0)
        return HandleCorpse(event, requester, text);
    else if (text.find("logouttime") == 0)
        return HandleLogoutTime(event, requester, text);
    else if (text.find("level") == 0)
        return HandleLevel(event, requester, text);
    else if (text.find("npc") == 0)
        return HandleNPC(event, requester, text);
    else if (text.find("go ") == 0)
        return HandleGO(event, requester, text);
    else if (text.find("item ") == 0)
        return HandleItem(event, requester, text);
    else if (text.find("find ") == 0)
        return HandleFind(event, requester, text);
    else if (text.find("rpg ") == 0)
        return HandleRPG(event, requester, text);
    else if (text.find("rpgtargets") == 0)
        return HandleRPGTargets(event, requester, text);    
    else if (text.find("travel ") == 0)
        return HandleTravel(event, requester, text);
    else if (text.find("print travel") == 0)
        return HandlePrintTravel(event, requester, text);
    else if (text.find("values ") == 0)
        return HandleValues(event, requester, text);
    else if (text.find("loot ") == 0)
        return HandleLoot(event, requester, text);
    else if (text.find("loot") == 0)
        return HandleLoot(event, requester, text);
    else if (text.find("drops ") == 0)
        return HandleDrops(event, requester, text);
    else if (text.find("taxi") == 0)
        return HandleTaxi(event, requester, text);
    else if (text.find("price ") == 0)
        return HandlePrice(event, requester, text);
    else if (text.find("nc") == 0)
        return HandleNC(event, requester, text);
    else if (text.find("add node") == 0 && isMod)
        return HandleAddNode(event, requester, text);
    else if (text.find("rem node") == 0 && isMod)
        return HandleRemNode(event, requester, text);
    else if (text.find("reset node") == 0 && isMod)
        return HandleResetNode(event, requester, text);
    else if (text.find("reset path") == 0 && isMod)
        return HandleResetPath(event, requester, text);
    else if (text.find("gen node") == 0 && isMod)
        return HandleGenNode(event, requester, text);
    else if (text.find("gen path") == 0 && isMod)
        return HandleGenPath(event, requester, text);
    else if (text.find("crop path") == 0 && isMod)
        return HandleCropPath(event, requester, text);
    else if (text.find("save node") == 0 && isMod)
        return HandleSaveNode(event, requester, text);
    else if (text.find("load node") == 0 && isMod)
        return HandleLoadNode(event, requester, text);
    else if (text.find("show node") == 0 && isMod)
        return HandleShowNode(event, requester, text);
    else if (text.find("dspell ") == 0 && isMod)
        return HandleDSpell(event, requester, text);
    else if (text.find("vspell ") == 0 && isMod)
        return HandleVSpell(event, requester, text);
    else if (text.find("aspell ") == 0 && isMod)
        return HandleASpell(event, requester, text);
    else if (text.find("cspell ") == 0 && isMod)
        return HandleCSpell(event, requester, text);
    else if (text.find("fspell ") == 0 && isMod)
        return HandleFSpell(event, requester, text);
    else if (text.find("spell ") == 0 && isMod)
        return HandleSpell(event, requester, text);
    else if (text.find("tspellmap") == 0 && isMod)
        return HandleTSpellMap(event, requester, text);
    else if (text.find("uspellmap") == 0 && isMod)
        return HandleUSpellMap(event, requester, text);
    else if (text.find("dspellmap") == 0 && isMod)
        return HandleDSpellMap(event, requester, text);
    else if (text.find("vspellmap") == 0 && isMod)
        return HandleVSpellMap(event, requester, text);
    else if (text.find("ispellmap") == 0 && isMod)
        return HandleISpellMap(event, requester, text);
    else if (text.find("cspellmap") == 0 && isMod)
        return HandleCSpellMap(event, requester, text);
    else if (text.find("aspellmap") == 0 && isMod)
        return HandleASpellMap(event, requester, text);
    else if (text.find("gspellmap") == 0 && isMod)
        return HandleGSpellMap(event, requester, text);
    else if (text.find("mspellmap") == 0 && isMod)
        return HandleMSpellMap(event, requester, text);
    else if (text.find("soundmap") == 0 && isMod)
        return HandleSoundMap(event, requester, text);
    else if (text.find("sounds") == 0 && isMod)
        return HandleSounds(event, requester, text);
    else if (text.find("dsound") == 0 && isMod)
        return HandleDSound(event, requester, text);
    else if (text.find("sound") == 0 && isMod)
        return HandleSound(event, requester, text);

    // Fallback/default behavior
    std::string response = ai->HandleRemoteCommand(text);
    ai->TellPlayer(requester, response);
    return true;
}

void DebugAction::FakeSpell(uint32 spellId, Unit* truecaster, Unit* caster, ObjectGuid target, std::list<ObjectGuid> otherTargets, std::list<ObjectGuid> missTargets, WorldPosition source, WorldPosition dest, bool forceDest)
{
    SpellEntry const* spellInfo = sServerFacade.LookupSpellInfo(spellId);
    {
        uint32 castFlags = CAST_FLAG_UNKNOWN2;

        if (spellInfo && spellInfo->HasAttribute(SPELL_ATTR_USES_RANGED_SLOT))
            castFlags |= CAST_FLAG_AMMO;                        // arrows/bullets visual

        WorldPacket data(SMSG_SPELL_START, (8 + 8 + 4 + 2 + 4));

        data << truecaster->GetPackGUID();                      //truecaster

        if (caster)
            data << caster->GetPackGUID();                          //m_caster->GetPackGUID();     
        else
            data << ObjectGuid();

        data << uint32(spellId);                                // spellId
        data << uint16(castFlags);                              // cast flags
        data << uint32(1000.0f);                                   // delay?

        SpellCastTargets m_targets;

        data << m_targets;

        //projectile info
        if (castFlags & CAST_FLAG_AMMO)
        {
            data << uint32(5996);
            data << uint32(INVTYPE_AMMO);
        }

        if(caster)
            caster->GetMap()->MessageBroadcast(caster, data);
        else
            truecaster->GetMap()->MessageBroadcast(truecaster, data);
    }
    {
        uint32 castFlags = CAST_FLAG_UNKNOWN9;



        if (spellInfo && spellInfo->HasAttribute(SPELL_ATTR_USES_RANGED_SLOT))
            castFlags |= CAST_FLAG_AMMO;                        // arrows/bullets visual
        if (spellInfo && HasPersistentAuraEffect(spellInfo))
            castFlags |= CAST_FLAG_PERSISTENT_AA;

        WorldPacket data(SMSG_SPELL_GO, 53);                    // guess size

        data << truecaster->GetPackGUID();                      //truecaster

        if (caster)
            data << caster->GetPackGUID();                          //m_caster->GetPackGUID();     
        else
            data << ObjectGuid();

        data << uint32(spellId);                                // spellId
        data << uint16(castFlags);                              // cast flags

        //WriteSpellGoTargets
        uint32 hits = otherTargets.size() + (target ? 1 : 0);

        data << uint8(hits);                                      // Hits                   

        if (target)
            data << target;

        //Hit targets here.                   
        for (auto otherTarget : otherTargets)
            data << otherTarget;

        data << (uint8)missTargets.size();                         //miss

        for (auto missTarget : missTargets)
        {
            data << missTarget;

            data << uint8(SPELL_MISS_RESIST);    //Miss condition
            data << uint8(SPELL_MISS_NONE);    //Miss condition
        }

        SpellCastTargets m_targets;

        if ((spellInfo && spellInfo->Targets & TARGET_FLAG_DEST_LOCATION) || forceDest)
            m_targets.setDestination(dest.getX(), dest.getY(), dest.getZ());
        if ((spellInfo && spellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION) || forceDest)
            m_targets.setSource(source.getX(), source.getY(), source.getZ());
        if (!forceDest && target)
            if(!spellInfo || !(spellInfo->Targets & TARGET_FLAG_DEST_LOCATION && spellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION))
                m_targets.setUnitTarget(ai->GetUnit(target));

        data << m_targets;

        //projectile info
        if (castFlags & CAST_FLAG_AMMO)
        {
            data << uint32(5996);
            data << uint32(INVTYPE_AMMO);
        }

        if (caster)
            caster->GetMap()->MessageBroadcast(caster, data);
        else
            truecaster->GetMap()->MessageBroadcast(truecaster, data);
    }
}

void DebugAction::addAura(uint32 spellId, Unit* target)
{
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo)
        return;

    if (!IsSpellAppliesAura(spellInfo, (1 << EFFECT_INDEX_0) | (1 << EFFECT_INDEX_1) | (1 << EFFECT_INDEX_2)) &&
        !IsSpellHaveEffect(spellInfo, SPELL_EFFECT_PERSISTENT_AREA_AURA))
    {
        return;
    }

    SpellAuraHolder* holder = CreateSpellAuraHolder(spellInfo, target, target);

    for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        uint8 eff = spellInfo->Effect[i];
        if (eff >= MAX_SPELL_EFFECTS)
            continue;
        if (IsAreaAuraEffect(eff) ||
            eff == SPELL_EFFECT_APPLY_AURA ||
            eff == SPELL_EFFECT_PERSISTENT_AREA_AURA)
        {
            int32 basePoints = spellInfo->CalculateSimpleValue(SpellEffectIndex(i));
            int32 damage = 0; // no damage cos caster doesnt exist
            Aura* aur = CreateAura(spellInfo, SpellEffectIndex(i), &damage, &basePoints, holder, target);
            holder->AddAura(aur, SpellEffectIndex(i));
        }
    }
    if (!target->AddSpellAuraHolder(holder))
        delete holder;

    return;
}

// Handler implementations

bool DebugAction::HandleAvoidScan(Event& event, Player* requester, const std::string& text)
{
    PathFinder path(requester);
    for (float x = -100.0f; x < 100.0f; x += 2.0f)
        for (float y = -100.0f; y < 100.0f; y += 2.0f)
        {
            WorldPosition p(requester);
            p.setX(p.getX() + x);
            p.setY(p.getY() + y);
            p.setZ(p.getHeight(bot->GetInstanceId()));
            Creature* wpCreature = requester->SummonCreature(2334, p.getX(), p.getY(), p.getZ(), 0.0, TEMPSPAWN_TIMED_DESPAWN, 20000.0f);
            if(path.getArea(p.getMapId(), p.getX(), p.getY(), p.getZ()) == 12)
                ai->AddAura(wpCreature, 246);
            if (path.getArea(p.getMapId(), p.getX(), p.getY(), p.getZ()) == 13)
                ai->AddAura(wpCreature, 1130);
        }
    return true;
}

bool DebugAction::HandleAvoidAdd(Event& event, Player* requester, const std::string& text)
{
    PathFinder pathfinder(bot);
    WorldPosition point(requester);
    uint32 area, radius;
    std::vector<std::string> args = { "12", "5" };
    if (text.length() > std::string("avoid add").size())
    {
        args = ChatHelper::splitString(text.substr(std::string("avoid add").size() + 1), " ");
        if (args.size() == 1)
            args.push_back("5");
    }
    area = stoi(args[0]);
    radius = stoi(args[1]);
    pathfinder.setArea(point.getMapId(), point.getX(), point.getY(), point.getZ(), area, radius);
    return true;
}

bool DebugAction::HandleMount(Event& event, Player* requester, const std::string& text)
{
    std::ostringstream out;
    out << (bot->IsMounted() ? "mounted" : "not mounted");
    out << (bot->IsTaxiFlying() ? ", taxi flying" : ", not taxi flying");
    out << ", mount speed: " << AI_VALUE2(uint32, "current mount speed", "self target");
    out << ", mount id: " << bot->GetMountID();        
    if(bot->GetMountInfo())
        out << ", mount info: " << bot->GetMountInfo()->Name;
    ai->TellPlayerNoFacing(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
    return true;
}

bool DebugAction::HandleUnmount(Event& event, Player* requester, const std::string& text)
{
    ai->Unmount();
    return true;
}

bool DebugAction::HandleArea(Event& event, Player* requester, const std::string& text)
{
    WorldPosition point(requester);
    AreaTableEntry const* area = point.GetArea();
    std::ostringstream out;
    out << point.getAreaName(true, false); 
    out << "," << area->team << " (" << (area->team != FACTION_GROUP_MASK_ALLIANCE ? (area->team != FACTION_GROUP_MASK_HORDE ? "neutral" : "horde") : "alliance") << ")";
    ai->TellPlayerNoFacing(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
    return true;
}

bool DebugAction::HandleLLM(Event& event, Player* requester, const std::string& text)
{
    Player* player = bot;
    std::map<std::string, std::string> jsonFill;
    jsonFill["<prompt>"] = text.substr(4);
    jsonFill["<context>"] = "";
    jsonFill["<pre prompt>"] = "";
    jsonFill["<post prompt>"] = "";
    std::string json = BOT_TEXT2(sPlayerbotAIConfig.llmApiJson, jsonFill);
    std::vector<std::string> debugLines = { json };
    std::string response = PlayerbotLLMInterface::Generate(json, sPlayerbotAIConfig.llmGenerationTimeout, sPlayerbotAIConfig.llmMaxSimultaniousGenerations, debugLines);
    for(auto line : debugLines)
        ai->TellPlayerNoFacing(requester, line, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
    ai->TellPlayerNoFacing(requester, response, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
    return true;
}

bool DebugAction::HandleChatReplyDo(Event& event, Player* requester, const std::string& text)
{
    ChatReplyAction::ChatReplyDo(bot, CHAT_MSG_WHISPER, requester->GetGUIDLow(), 0, text.substr(12), "", requester->GetName());
    return true;
}

bool DebugAction::HandleMonsterTalk(Event& event, Player* requester, const std::string& text)
{
    GuidPosition guidP = GuidPosition(requester->GetSelectionGuid(), requester);
    Player* player = requester;
    if (!guidP.IsUnit())
        return false;
    Action* action = context->GetAction("rpg ai chat");
    if (!action)
        return false;
    RpgAIChatAction* rpgAction = (RpgAIChatAction*)action;
    if (!rpgAction)
        return false;
    std::string line = text.substr(12);
    rpgAction->ManualChat(guidP, line);
    return true;
}

bool DebugAction::HandleGY(Event& event, Player* requester, const std::string& text)
{
    for (uint32 i = 0; i < sMapStore.GetNumRows(); ++i)
    {
        if (!sMapStore.LookupEntry(i))
            continue;

        uint32 mapId = sMapStore.LookupEntry(i)->MapID;

        Map* map = sMapMgr.FindMap(mapId);

        if (!map)
            continue;

        GraveyardManager* gy = &map->GetGraveyardManager();

        if (!gy)
            continue;

        for (uint32 x = 0; x < TOTAL_NUMBER_OF_CELLS_PER_MAP; x++)
        {
            for (uint32 y = 0; y < TOTAL_NUMBER_OF_CELLS_PER_MAP; y++)
            {
                CellPair c(x, y);

                WorldPosition pos(mapId, c);

                if (!pos.isValid())
                {
                    continue;
                }

                pos.setZ(pos.getHeight(bot->GetInstanceId()));

                const uint32 zoneId = sTerrainMgr.GetZoneId(mapId, pos.getX(), pos.getY(), pos.getZ());
                const uint32 areaId = sTerrainMgr.GetAreaId(mapId, pos.getX(), pos.getY(), pos.getZ());

                WorldSafeLocsEntry const* graveyard = nullptr;
                if (areaId != 0)
                {
                    WorldSafeLocsEntry const* ClosestGrave;
                    ClosestGrave = gy->GetClosestGraveYard(pos.getX(), pos.getY(), pos.getZ(), mapId, ALLIANCE);
                    ClosestGrave = gy->GetClosestGraveYard(pos.getX(), pos.getY(), pos.getZ(), mapId, HORDE);
                }
            }
        }
    }
    return true;
}

bool DebugAction::HandleGrid(Event& event, Player* requester, const std::string& text)
{
    WorldPosition botPos = bot;
    std::string loaded = botPos.getMap(bot->GetInstanceId())->IsLoaded(botPos.getX(), botPos.getY()) ? "loaded" : "unloaded";

    std::ostringstream out;

    out << "Map: " << botPos.getMapId() << " " << botPos.getAreaName() << " Grid: " << botPos.getGridPair().x_coord << "," << botPos.getGridPair().y_coord << " [" << loaded << "] Cell: " << botPos.getCellPair().x_coord << "," << botPos.getCellPair().y_coord;

    bot->Whisper(out.str().c_str(), LANG_UNIVERSAL, event.getOwner()->GetObjectGuid());

    return true;
}

bool DebugAction::HandleTest(Event& event, Player* requester, const std::string& text)
{
    std::string param = "";
    if (text.length() > 4)
    {
        param = text.substr(5);
    }

    ai->GetAiObjectContext()->ClearExpiredValues(param, 10);
    return true;
}

bool DebugAction::HandleGPS(Event& event, Player* requester, const std::string& text)
{
    std::ostringstream out;
    WorldPosition botPos(bot);
    out << bot->GetName() << ",";
    out << botPos.getAreaName() << ",";

    out << "\"node = sTravelNodeMap.addNode(WorldPosition(" << botPos.getMapId() << std::fixed << std::setprecision(2);
    out << ',' << botPos.coord_x << "f";
    out << ',' << botPos.coord_y << "f";
    out << ',' << botPos.coord_z << "f";
    out << "), \\\"c1" << botPos.getAreaName() << "\\\", true, false);\"";

    //node = sTravelNodeMap.addNode(WorldPosition(1, -3626.39f, 917.37f, 150.13f), "c1-Dire maul", true, false);

    if (!sPlayerbotAIConfig.isLogOpen("gps.csv"))
        sPlayerbotAIConfig.openLog("gps.csv", "a", true);
    sPlayerbotAIConfig.log("gps.csv", out.str().c_str());

    return true;
}

bool DebugAction::HandleSetValueUInt32(Event& event, Player* requester, const std::string& text)
{
    std::vector<std::string> args = Qualified::getMultiQualifiers(text.substr(14), ",");

    if (args.size() == 1)
    {
        RESET_AI_VALUE(uint32, args[0]);
        return true;
    }
    else if (args.size() == 2)
    {
        SET_AI_VALUE(uint32, args[0], stoi(args[1]));
        return true;
    }
    else if (args.size() == 3)
    {
        SET_AI_VALUE2(uint32, args[0], args[1], stoi(args[2]));
        return true;
    }

    return false;
}

bool DebugAction::HandleDo(Event& event, Player* requester, const std::string& text)
{
    return ai->DoSpecificAction(text.substr(3), Event(), true);
}

bool DebugAction::HandleTrade(Event& event, Player* requester, const std::string& text)
{
    std::string param = "";
    if (text.length() > 6)
    {
        param = text.substr(6);
    }

    if (param.find("stop") == 0)
    {
        if (bot->GetTradeData())
            bot->TradeCancel(true);
    }
    return true;
}

bool DebugAction::HandleMail(Event& event, Player* requester, const std::string& text)
{
    std::string param = "";
    if (text.length() > 5)
    {
        param = text.substr(6);
    }

    bool doAction = ai->DoSpecificAction("mail", Event("debug", param.empty() ? "?" : param), true);
    return doAction;
}

bool DebugAction::HandlePOI(Event& event, Player* requester, const std::string& text)
{        
    WorldPosition botPos = WorldPosition(bot);

    WorldPosition poiPoint = botPos;
    std::string name = "bot";

    std::vector<std::string> args = Qualified::getMultiQualifiers(text.substr(4), " ");

    DestinationList dests = ChooseTravelTargetAction::FindDestination(bot, args[0]);
    TravelDestination* dest = nullptr;

    if (!dests.empty())
    {
        WorldPosition botPos(bot);

        dest = *std::min_element(dests.begin(), dests.end(), [botPos](TravelDestination* i, TravelDestination* j) {return i->DistanceTo(botPos) < j->DistanceTo(botPos); });
    }

    if (dest)
    {
        std::list<uint8> chancesToGoFar = { 10,50,90 }; //Closest map, grid, cell.
        WorldPosition* point = dest->GetNextPoint(botPos, chancesToGoFar);
        if (point)
        {
            poiPoint = *point;
            name = dest->GetTitle();
        }
    }

    if (args.size() == 1)
    {
        args.push_back("99");
    }
    else if (args.size() == 2)
    {
        args.push_back("6");
    }
    else if (args.size() == 3)
    {
        args.push_back("0");
    }

    ai->Poi(poiPoint.coord_x, poiPoint.coord_y, name, nullptr, stoi(args[1]), stoi(args[2]), stoi(args[3]));

    return true;
}

bool DebugAction::HandleMotion(Event& event, Player* requester, const std::string& text)
{
    Unit* requesterTarget = nullptr;
    if (requester->GetSelectionGuid())
    {
        requesterTarget = ai->GetUnit(requester->GetSelectionGuid());
    }

    Unit* motionBot = bot;
    Unit* motionTarget = requesterTarget;

    if (text.find("motion") != 0)
    {
        if (requesterTarget)
        {
            motionBot = requesterTarget;
            motionTarget = requester;
        }
        else
        {
            motionBot = requester;
            motionTarget = bot;
        }
    }

    MotionMaster* mm = motionBot->GetMotionMaster();

    MovementGeneratorType type = mm->GetCurrentMovementGeneratorType();

    std::string sType = "TODO"; // GetMoveTypeStr(type);

    Unit* cTarget = sServerFacade.GetChaseTarget(motionBot);
    float cAngle = sServerFacade.GetChaseAngle(motionBot);
    float cOffset = sServerFacade.GetChaseOffset(motionBot);
    std::string cTargetName = cTarget ? cTarget->GetName() : "none";

    std::string motionName = motionBot->GetName();

    ai->TellPlayer(requester, motionName + " :" + sType + " (" + cTargetName + " a:" + std::to_string(cAngle) + " o:" + std::to_string(cOffset) + ")");

    if (!requesterTarget)
    {
        requesterTarget = requester;
    }

    if (text.size() > 7)
    {
        std::string cmd = text.substr(7);

        if (cmd == "clear")
            mm->Clear();
        else if (cmd == "reset")
            mm->Clear(true);
        else if (cmd == "clearall")
            mm->Clear(false, true);
        else if (cmd == "expire")
            mm->MovementExpired();
        else if (cmd == "flee")
            mm->MoveFleeing(motionTarget, 10);
        else if (cmd == "followmain")
            mm->MoveFollow(motionTarget, 5, 0, true, true);
        else if (cmd == "follow")
            mm->MoveFollow(motionTarget, 5, 0);
        else if (cmd == "dist")
            mm->DistanceYourself(10);
        else if (cmd == "update")
            mm->UpdateMotion(10);
        else if (cmd == "chase")
            mm->MoveChase(motionTarget, 5, 0);
        else if (cmd == "fall")
            mm->MoveFall();
        else if (cmd == "formation")
        {
            FormationSlotDataSPtr form = std::make_shared<FormationSlotData>(0, bot->GetObjectGuid(), nullptr, SpawnGroupFormationSlotType::SPAWN_GROUP_FORMATION_SLOT_TYPE_STATIC);
            mm->MoveInFormation(form);
        }

        std::string sType = "TODO"; // GetMoveTypeStr(type);
        ai->TellPlayer(requester, "new:" + sType);
    }
    return true;
}

bool DebugAction::HandleTransport(Event& event, Player* requester, const std::string& text)
{
    std::vector<GenericTransport*> transports;

    for (auto trans : WorldPosition(bot).getTransports())
        transports.push_back(trans);

    WorldPosition botPos(bot);

    //Closest transport last = below in chat.
    std::sort(transports.begin(), transports.end(), [botPos](GenericTransport* i, GenericTransport* j){ return botPos.distance(i) > botPos.distance(j); });

    for (auto trans : transports)
    {
        GameObjectInfo const* data = sGOStorage.LookupEntry<GameObjectInfo>(trans->GetEntry());
        std::ostringstream out;

        if (WorldPosition(bot).isOnTransport(trans))
        {
            out << "On transport ";
        }
        else
        {
            out << "Not on transport ";
        }

        std::string transportName = trans->GetName();
        if (transportName.empty())
            transportName = data->name;

        out << transportName << " (" << trans->GetEntry() << ")";

        out<< " dist:" << std::fixed << std::setprecision(2) << botPos.distance(trans) << " offset:" << (botPos -trans).print();

        ai->TellPlayer(requester, out);
    }

    return true;
}

bool DebugAction::HandleOnTrans(Event& event, Player* requester, const std::string& text)
{
    if (bot->GetTransport())
    {
        return false;
    }

    uint32 radius = 10;

    if (text.length() > std::string("ontrans").size())
    {
        radius = stoi(text.substr(std::string("ontrans").size() + 1));
    }

    WorldPosition botPos(bot);
    GenericTransport* transport = nullptr;
    for (auto trans : botPos.getTransports())
    {
        if (!transport || botPos.distance(trans) < botPos.distance(transport))
        {
            transport = trans;
        }
    }

    if (!transport)
    {
        return false;
    }

    GenericTransport* trans = transport;

    WorldPosition pos(trans);

    pos.setZ(botPos.getZ() + 3.0f);

    //if(trans->GetGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT)
    //    pos += WorldPosition(0, cos(pos.getAngleTo(botPos)) * 3.0f, sin(pos.getAngleTo(botPos)) * 3.0f);

    bot->SetTransport(trans);
    std::vector<WorldPosition> path = pos.getPathFrom(botPos, bot); //Use full pathstep to get proper paths on to transports.

    if (path.empty())
    {
        ai->TellPlayer(requester, "No path.");
        bot->SetTransport(nullptr);
        return false;
    }

    WorldPosition entryPos = path.back();

    if (!entryPos.isOnTransport(trans))
    {
        for (auto p : path)
        {
            bool onTrans = pos.isOnTransport(trans);

            if (onTrans) //Summon creature needs to be Summoned on offset coordinates.
            {
                p.CalculatePassengerOffset(trans);
                bot->SetTransport(trans);
            }
            else //Generate wp off transport so it doesn't spawn on transport.
            {
                bot->SetTransport(nullptr);
            }

            Creature* wpCreature = bot->SummonCreature(2334, p.getX(), p.getY(), p.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);
            ai->AddAura(wpCreature, 246);

            if (p.isOnTransport(transport))
            {
                ai->AddAura(wpCreature, 1130);
            }
        }

        ai->TellPlayer(requester, "No path on boat.");
        bot->SetTransport(nullptr);
        return false;
    }

    for (auto p : path)
    {
        bool onTrans = pos.isOnTransport(trans);

        if (onTrans) //Summon creature needs to be Summoned on offset coordinates.
        {
            p.CalculatePassengerOffset(trans);
            bot->SetTransport(trans);
        }
        else //Generate wp off transport so it doesn't spawn on transport.
        {
            bot->SetTransport(nullptr);
        }

        Creature* wpCreature = bot->SummonCreature(2334, p.getX(), p.getY(), p.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);
        ai->AddAura(wpCreature, 246);

        if (p.isOnTransport(transport))
        {
            ai->AddAura(wpCreature, 1130);
        }
    }

    transport->AddPassenger(bot);

    ai->StopMoving();
    bot->GetMotionMaster()->MovePoint(entryPos.getMapId(), entryPos.getX(), entryPos.getY(), botPos.getZ(), FORCED_MOVEMENT_RUN);

    return true;
}

bool DebugAction::HandleOffTrans(Event& event, Player* requester, const std::string& text)
{
    uint32 radius = 10;

    if (text.length() > std::string("offtrans").size())
    {
        radius = stoi(text.substr(std::string("offtrans").size() + 1));
    }

    if (!bot->GetTransport())
    {
        return false;
    }

    WorldPosition botPos(bot);
    GenericTransport* transport = bot->GetTransport();

    if (!transport)
    {
        return false;
    }

    WorldPosition destPos = botPos + WorldPosition(0,cos(bot->GetOrientation()) * radius, sin(bot->GetOrientation()) * radius);

    std::vector<WorldPosition> path = destPos.getPathFrom(botPos, nullptr);

    if (path.empty())
    {
        ai->TellPlayer(requester, "No path.");
        return false;
    }

    WorldPosition exitPos = path.back();

    if (exitPos.isOnTransport(transport))
    {
        ai->TellPlayer(requester, "Path still on boat.");
        return false;
    }
    
    for (auto& p : path)
    {
        p.CalculatePassengerOffset(bot->GetTransport());

        Creature* wpCreature = bot->SummonCreature(2334, p.getX(), p.getY(), p.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);
        ai->AddAura(wpCreature, 246);

        if (p.isOnTransport(transport))
            ai->AddAura(wpCreature, 1130);
    }

    ai->StopMoving();
    bot->GetMotionMaster()->MovePoint(exitPos.getMapId(), exitPos.getX(), exitPos.getY(), exitPos.getZ(),FORCED_MOVEMENT_RUN);

    return true;
}

bool DebugAction::HandlePathable(Event& event, Player* requester, const std::string& text)
{
    uint32 radius = 10;

    if (text.length() > std::string("pathable").size())
        radius = stoi(text.substr(std::string("pathable").size() + 1));

    GenericTransport* transport = nullptr;
    for (auto trans : WorldPosition(bot).getTransports(bot->GetInstanceId()))
        if (!transport || WorldPosition(bot).distance(trans) < WorldPosition(bot).distance(transport))
            transport = trans;

    for (float x = radius * -1.0f; x < radius; x += 1.0f)
    {
        for (float y = radius * -1.0f; y < radius; y += 1.0f)
        {
            if (x * x + y * y > radius * radius)
                continue;

            WorldPosition botPos(bot);
            WorldPosition pos = botPos + WorldPosition(0, x, y, 3.0f);

            Player* pathBot = bot;
            GenericTransport* botTrans = bot->GetTransport();
            GenericTransport* trans = botTrans ? botTrans : transport;

            if (!pos.isOnTransport(trans)) //When trying to calculate a position off the transport, act like the bot is off the transport.
                pathBot = nullptr;
            else 
                bot->SetTransport(trans);

            std::vector<WorldPosition> path = pos.getPathFrom(botPos, pathBot); //Use full pathstep to get proper paths on to transports.

            if (path.empty())
                continue;

            pos = path.back();

            bool onTrans = pos.isOnTransport(trans);

            if (onTrans) //Summon creature needs to be Summoned on offset coordinates.
            {
                pos.CalculatePassengerOffset(trans);
                bot->SetTransport(trans);
            }
            else //Generate wp off transport so it doesn't spawn on transport.
                bot->SetTransport(nullptr);

            Creature* wpCreature = bot->SummonCreature(2334, pos.getX(), pos.getY(), pos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);
            ai->AddAura(wpCreature, 246);

            if (onTrans)
                ai->AddAura(wpCreature, 1130);

            bot->SetTransport(botTrans);
        }
    }

    return false;
}

bool DebugAction::HandleRandomSpot(Event& event, Player* requester, const std::string& text)
{
    uint32 radius = 10;
    if(text.length() > std::string("randomspot").size())
        radius = stoi(text.substr(std::string("randomspot").size()+1));

    WorldPosition botPos(bot);

    PathFinder pathfinder(bot);

    if (bot->GetTransport())
        botPos.CalculatePassengerOffset(bot->GetTransport());

    pathfinder.ComputePathToRandomPoint(botPos.getVector3(), radius);
    PointsArray points = pathfinder.getPath();
    std::vector<WorldPosition> path = botPos.fromPointsArray(points);

    if (path.empty())
        return false;
   
    Creature* wpCreature = bot->SummonCreature(6, path.back().getX(), path.back().getY(), path.back().getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);
    wpCreature->SetObjectScale(0.5f);

    if (bot->GetTransport())
        bot->GetTransport()->AddPassenger(wpCreature,true);


    for (auto& p : path)
    {
        Creature* wpCreature = bot->SummonCreature(2334, p.getX(), p.getY(), p.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);
        ai->AddAura(wpCreature, 246);
        if (bot->GetTransport())
            bot->GetTransport()->AddPassenger(wpCreature,true);
    }
    return true;
}

bool DebugAction::HandlePrintMap(Event& event, Player* requester, const std::string& text)
{
    sTravelNodeMap.printMap();
    sTravelNodeMap.printNodeStore();
    return true;
}

bool DebugAction::HandleCorpse(Event& event, Player* requester, const std::string& text)
{

    Corpse* corpse = bot->GetCorpse();

    if (!corpse)
    {
        ai->TellPlayerNoFacing(requester, "no corpse");
        return true;
    }


    std::ostringstream out;

    if(corpse->GetType() == CORPSE_BONES)
        out << "CORPSE_BONES";

    if (corpse->GetType() == CORPSE_RESURRECTABLE_PVE)
        out << "CORPSE_RESURRECTABLE_PVE";

    if (corpse->GetType() == CORPSE_RESURRECTABLE_PVP)
        out << "CORPSE_RESURRECTABLE_PVP";

    out << ",p:" << WorldPosition(corpse).print();

    out << ",time: " << corpse->GetGhostTime();

    ai->TellPlayerNoFacing(requester, out);

    return true;
}

bool DebugAction::HandleLogoutTime(Event& event, Player* requester, const std::string& text)
{
    int32 time = sRandomPlayerbotMgr.GetValueValidTime(bot->GetGUIDLow(), "add");

    int32 min = 0, hr = 0;

    if (time > 3600)
    {
        hr = floor(time / 3600);
        time = time % 3600;
    }
    if (time > 60)
    {
        min = floor(time / 60);
        time = time % 60;
    }

    std::ostringstream out;

    out << "Logout in: " << hr << ":" << min << ":" << time;

    bot->Whisper(out.str().c_str(), LANG_UNIVERSAL, event.getOwner()->GetObjectGuid());

    return true;
}

bool DebugAction::HandleLevel(Event& event, Player* requester, const std::string& text)
{
    uint32 level = bot->GetLevel();
    uint32 nextLevelXp = bot->GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
    uint32 xp = bot->GetUInt32Value(PLAYER_XP);
    float flevel = ai->GetLevelFloat();

    std::ostringstream out;

    out << "Level: " << level << ", xp:" << xp << "/" << nextLevelXp << " :" || flevel;

    bot->Whisper(out.str().c_str(), LANG_UNIVERSAL, event.getOwner()->GetObjectGuid());

    return true;
}

bool DebugAction::HandleNPC(Event& event, Player* requester, const std::string& text)
{
    std::ostringstream out;

    GuidPosition guidP = GuidPosition(requester->GetSelectionGuid(), requester);

    if (text.size() > 4)
    {
        std::string link = text.substr(4);

        if (!link.empty())
        {
            std::list<int32> entries = chat->parseWorldEntries(link);
            if (!entries.empty())
            {

                int32 entry = entries.front();

                for (auto cre : WorldPosition(bot).getCreaturesNear(0.0f, entry))
                {
                    guidP = GuidPosition(cre);
                    break;
                }
            }
        }
    }

    if (!guidP)
        return false;

    if (guidP.GetWorldObject(bot->GetInstanceId()))
        out << chat->formatWorldobject(guidP.GetWorldObject(bot->GetInstanceId()));
    
    out << " (e:" << guidP.GetEntry();
    
    if (guidP.GetUnit(bot->GetInstanceId()))
        out << ",level:" << guidP.GetUnit(bot->GetInstanceId())->GetLevel();
        
    out << ") ";

    guidP.printWKT(out);

    out << "[a:" << guidP.GetArea()->area_name[0]; 

    if (guidP.GetArea() && guidP.getAreaLevel())
        out << " level: " << guidP.getAreaLevel();
    if (guidP.GetArea()->zone && GetAreaEntryByAreaID(guidP.GetArea()->zone))
    {
        out << " z:" << GetAreaEntryByAreaID(guidP.GetArea()->zone)->area_name[0];
        if (sTravelMgr.GetAreaLevel(guidP.GetArea()->zone))
            out << " level: " << sTravelMgr.GetAreaLevel(guidP.GetArea()->zone);
    }

    out << "] ";

    uint16 event_id = sGameEventMgr.GetGameEventId<Creature>(guidP.GetCounter());

    if (event_id)
        out << " event:" << event_id << (sGameEventMgr.IsActiveEvent(event_id) ? " active" : " inactive");

    uint16 topPoolId = sPoolMgr.IsPartOfTopPool<Creature>(guidP.GetCounter());

    if (topPoolId)
        out << " pool:" << topPoolId << (sGameEventMgr.GetGameEventId<Pool>(topPoolId) ? " event" : " nonevent");

    ai->TellPlayerNoFacing(requester, out);

    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_GOSSIP))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_GOSSIP");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_QUESTGIVER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_QUESTGIVER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_VENDOR))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_VENDOR");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_FLIGHTMASTER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_FLIGHTMASTER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_TRAINER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_TRAINER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_SPIRITHEALER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_SPIRITHEALER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_SPIRITGUIDE))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_SPIRITGUIDE");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_INNKEEPER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_INNKEEPER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_BANKER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_BANKER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_PETITIONER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_PETITIONER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_TABARDDESIGNER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_TABARDDESIGNER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_BATTLEMASTER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_BATTLEMASTER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_AUCTIONEER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_AUCTIONEER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_STABLEMASTER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_STABLEMASTER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_REPAIR))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_REPAIR");
#ifdef MANGOSBOT_ZERO
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_OUTDOORPVP))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_OUTDOORPVP");
#endif


    std::ostringstream out2;
    FactionTemplateEntry const* requestFaction = sFactionTemplateStore.LookupEntry(requester->GetFaction());
    FactionTemplateEntry const* objectFaction = nullptr;
    if(guidP.GetCreatureTemplate() && guidP.GetCreatureTemplate()->Faction)
        objectFaction = sFactionTemplateStore.LookupEntry(guidP.GetCreatureTemplate()->Faction);
    FactionTemplateEntry const* humanFaction = sFactionTemplateStore.LookupEntry(1);
    FactionTemplateEntry const* orcFaction = sFactionTemplateStore.LookupEntry(2);

    std::map<ReputationRank, std::string> rep;

    rep[REP_HATED] = "hated";
    rep[REP_HOSTILE] = "hostile";
    rep[REP_UNFRIENDLY] = "unfriendly";
    rep[REP_NEUTRAL] = "neutral";
    rep[REP_FRIENDLY] = "friendly";
    rep[REP_HONORED] = "honored";
    rep[REP_REVERED] = "revered";
    rep[REP_EXALTED] = "exalted";

    if (objectFaction)
    {
        ReputationRank reactionRequest = PlayerbotAI::GetFactionReaction(requestFaction, objectFaction);
        ReputationRank reactionHum = PlayerbotAI::GetFactionReaction(humanFaction, objectFaction);
        ReputationRank reactionOrc = PlayerbotAI::GetFactionReaction(orcFaction, objectFaction);

        out2 << " faction:" << guidP.GetCreatureTemplate()->Faction << " reaction me: " << rep[reactionRequest] << ",alliance: " << rep[reactionHum] << " ,horde: " << rep[reactionOrc];
    }

    ai->TellPlayerNoFacing(requester, out2);

    return true;
}

bool DebugAction::HandleGO(Event& event, Player* requester, const std::string& text)
{
    std::ostringstream out;

    if (text.size() < 4)
        return false;

    GuidPosition guidP;

    std::string link = text.substr(3);

    if (!link.empty())
    {
        std::list<ObjectGuid> gos = chat->parseGameobjects(link);
        if (!gos.empty())
        {
            for (auto go : gos)
            {
                guidP = GuidPosition(go, bot);
                break;
            }
        }
    }

    if (!guidP)
        return false;

    if (!guidP.IsGameObject())
        return false;

    if (guidP.GetWorldObject(bot->GetInstanceId()))
        out << chat->formatWorldobject(guidP.GetWorldObject(bot->GetInstanceId()));

    out << " (e:" << guidP.GetEntry();

    if (guidP.GetUnit(bot->GetInstanceId()))
        out << ",level:" << guidP.GetUnit(bot->GetInstanceId())->GetLevel();

    out << ") ";

    guidP.printWKT(out);


    ai->TellPlayerNoFacing(requester, out);

    std::ostringstream out2;
    
    FactionTemplateEntry const* requestFaction = sFactionTemplateStore.LookupEntry(requester->GetFaction());
    FactionTemplateEntry const* objectFaction = sFactionTemplateStore.LookupEntry(guidP.GetGameObjectInfo()->faction);
    FactionTemplateEntry const* humanFaction = sFactionTemplateStore.LookupEntry(1);
    FactionTemplateEntry const* orcFaction = sFactionTemplateStore.LookupEntry(2);

    std::map<ReputationRank, std::string> rep;

    rep[REP_HATED] = "hated";
    rep[REP_HOSTILE] = "hostile";
    rep[REP_UNFRIENDLY] = "unfriendly";
    rep[REP_NEUTRAL] = "neutral";
    rep[REP_FRIENDLY] = "friendly";
    rep[REP_HONORED] = "honored";
    rep[REP_REVERED] = "revered";
    rep[REP_EXALTED] = "exalted";

    if (objectFaction)
    {
        ReputationRank reactionRequest = PlayerbotAI::GetFactionReaction(requestFaction, objectFaction);
        ReputationRank reactionHum = PlayerbotAI::GetFactionReaction(humanFaction, objectFaction);
        ReputationRank reactionOrc = PlayerbotAI::GetFactionReaction(orcFaction, objectFaction);

        out2 << " faction:" << guidP.GetGameObjectInfo()->faction << " reaction me: " << rep[reactionRequest] << ",alliance: " << rep[reactionHum] << " ,horde: " << rep[reactionOrc];
    }

    ai->TellPlayerNoFacing(requester, out2);

    std::unordered_map<uint32, std::string>  types;
    types[GAMEOBJECT_TYPE_DOOR] = "GAMEOBJECT_TYPE_DOOR";
    types[GAMEOBJECT_TYPE_BUTTON] = "GAMEOBJECT_TYPE_BUTTON";
    types[GAMEOBJECT_TYPE_QUESTGIVER] = "GAMEOBJECT_TYPE_QUESTGIVER";
    types[GAMEOBJECT_TYPE_CHEST] = "GAMEOBJECT_TYPE_CHEST";
    types[GAMEOBJECT_TYPE_BINDER] = "GAMEOBJECT_TYPE_BINDER";
    types[GAMEOBJECT_TYPE_GENERIC] = "GAMEOBJECT_TYPE_GENERIC";
    types[GAMEOBJECT_TYPE_TRAP] = "GAMEOBJECT_TYPE_TRAP";
    types[GAMEOBJECT_TYPE_CHAIR] = "GAMEOBJECT_TYPE_CHAIR";
    types[GAMEOBJECT_TYPE_SPELL_FOCUS] = "GAMEOBJECT_TYPE_SPELL_FOCUS";
    types[GAMEOBJECT_TYPE_TEXT] = "GAMEOBJECT_TYPE_TEXT";
    types[GAMEOBJECT_TYPE_GOOBER] = "GAMEOBJECT_TYPE_GOOBER";
    types[GAMEOBJECT_TYPE_TRANSPORT] = "GAMEOBJECT_TYPE_TRANSPORT";
    types[GAMEOBJECT_TYPE_AREADAMAGE] = "GAMEOBJECT_TYPE_AREADAMAGE";
    types[GAMEOBJECT_TYPE_CAMERA] = "GAMEOBJECT_TYPE_CAMERA";
    types[GAMEOBJECT_TYPE_MAP_OBJECT] = "GAMEOBJECT_TYPE_MAP_OBJECT";
    types[GAMEOBJECT_TYPE_MO_TRANSPORT] = "GAMEOBJECT_TYPE_MO_TRANSPORT";
    types[GAMEOBJECT_TYPE_DUEL_ARBITER] = "GAMEOBJECT_TYPE_DUEL_ARBITER";
    types[GAMEOBJECT_TYPE_FISHINGNODE] = "GAMEOBJECT_TYPE_FISHINGNODE";
    types[GAMEOBJECT_TYPE_SUMMONING_RITUAL] = "GAMEOBJECT_TYPE_SUMMONING_RITUAL";
    types[GAMEOBJECT_TYPE_MAILBOX] = "GAMEOBJECT_TYPE_MAILBOX";
#ifndef MANGOSBOT_TWO
    types[GAMEOBJECT_TYPE_AUCTIONHOUSE] = "GAMEOBJECT_TYPE_AUCTIONHOUSE";
#endif
    types[GAMEOBJECT_TYPE_GUARDPOST] = "GAMEOBJECT_TYPE_GUARDPOST";
    types[GAMEOBJECT_TYPE_SPELLCASTER] = "GAMEOBJECT_TYPE_SPELLCASTER";
    types[GAMEOBJECT_TYPE_MEETINGSTONE] = "GAMEOBJECT_TYPE_MEETINGSTONE";
    types[GAMEOBJECT_TYPE_FLAGSTAND] = "GAMEOBJECT_TYPE_FLAGSTAND";
    types[GAMEOBJECT_TYPE_FISHINGHOLE] = "GAMEOBJECT_TYPE_FISHINGHOLE";
    types[GAMEOBJECT_TYPE_FLAGDROP] = "GAMEOBJECT_TYPE_FLAGDROP";
    types[GAMEOBJECT_TYPE_MINI_GAME] = "GAMEOBJECT_TYPE_MINI_GAME";
#ifndef MANGOSBOT_TWO
    types[GAMEOBJECT_TYPE_LOTTERY_KIOSK] = "GAMEOBJECT_TYPE_LOTTERY_KIOSK";
#endif
    types[GAMEOBJECT_TYPE_CAPTURE_POINT] = "GAMEOBJECT_TYPE_CAPTURE_POINT";
    types[GAMEOBJECT_TYPE_AURA_GENERATOR] = "GAMEOBJECT_TYPE_AURA_GENERATOR";
#ifdef MANGOSBOT_TWO
    types[GAMEOBJECT_TYPE_DUNGEON_DIFFICULTY] = "GAMEOBJECT_TYPE_DUNGEON_DIFFICULTY";
    types[GAMEOBJECT_TYPE_BARBER_CHAIR] = "GAMEOBJECT_TYPE_BARBER_CHAIR";
    types[GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING] = "GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING";
    types[GAMEOBJECT_TYPE_GUILD_BANK] = "GAMEOBJECT_TYPE_GUILD_BANK";
    types[GAMEOBJECT_TYPE_TRAPDOOR] = "GAMEOBJECT_TYPE_TRAPDOOR";
#endif

    ai->TellPlayerNoFacing(requester, types[guidP.GetGameObjectInfo()->type]);

    if (guidP.GetGameObject(bot->GetInstanceId()))
    {
        GameObject* object = guidP.GetGameObject(bot->GetInstanceId());

        GOState state = object->GetGoState();

        std::ostringstream out;

        out << "state:";

        if (state == GO_STATE_ACTIVE)
            out << "GO_STATE_ACTIVE";
        if (state == GO_STATE_READY)
            out << "GO_STATE_READY";
        if (state == GO_STATE_ACTIVE_ALTERNATIVE)
            out << "GO_STATE_ACTIVE_ALTERNATIVE";

        out << (object->IsInUse() ? ", in use" : ", not in use");

        LootState lootState = object->GetLootState();

        out << " lootState:";

        if (lootState == GO_NOT_READY)
            out << "GO_NOT_READY";
        if (lootState == GO_READY)
            out << "GO_READY";
        if (lootState == GO_ACTIVATED)
            out << "GO_ACTIVATED";
        if (lootState == GO_JUST_DEACTIVATED)
            out << "GO_JUST_DEACTIVATED";

        ai->TellPlayerNoFacing(requester, out);
    }

    return true;
}

bool DebugAction::HandleFind(Event& event, Player* requester, const std::string& text)
{
    std::ostringstream out;

    if (text.size() <= 5)
        return false;

    std::string link = text.substr(5);

    if (link.empty())
        return false;

    if (!Qualified::isValidNumberString(link))
        return false;

    uint32 entry = stoi(link);

    Creature* creature = nullptr;
    MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check(*bot, entry, true, false, 1000.0f, true);
    MaNGOS::CreatureLastSearcher<MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(creature, creature_check);
    Cell::VisitGridObjects(bot, searcher, 1000.0f);

    if (!creature)
        return false;

    out << "Found: ";
    out << chat->formatWorldobject(creature);
    out << " at distance ";
    out << bot->GetDistance(creature);

    ai->TellPlayerNoFacing(requester, out);

    return true;
}

bool DebugAction::HandleItem(Event& event, Player* requester, const std::string& text)
{
    std::ostringstream out;

    if (text.size() < 4)
        return false;

    GuidPosition guidP;

    std::string link = text.substr(3);

    if (link.empty())
        return false;

        ItemIds items = chat->parseItems(link);
        if (items.empty())
            return false;

    uint32 itemId = *items.begin();

    Item* item = bot->GetItemByEntry(itemId);

    ItemQualifier qualifier;
    ItemPrototype const* proto;
    if (item)
    {
        qualifier = ItemQualifier(item);
        proto = item->GetProto();
    }
    else
    {
        qualifier = ItemQualifier(itemId);
        proto = qualifier.GetProto();
    }


    if (item)
        out << chat->formatItem(item, item->GetCount());
    else
        out << chat->formatItem(qualifier);

    out << " (id:" << itemId;

    out << ",reqlev:" << proto->RequiredLevel << " ilev:" << proto->ItemLevel;

    out << ") ";
    

    ai->TellPlayerNoFacing(requester, out);

    std::ostringstream out2;

    // Print class and subclass string equivalents
    static const std::unordered_map<uint32, std::string> itemClassNames = {
        { ITEM_CLASS_CONSUMABLE, "Consumable" },
        { ITEM_CLASS_CONTAINER, "Container" },
        { ITEM_CLASS_WEAPON, "Weapon" },
        { ITEM_CLASS_GEM, "Gem" },
        { ITEM_CLASS_ARMOR, "Armor" },
        { ITEM_CLASS_REAGENT, "Reagent" },
        { ITEM_CLASS_PROJECTILE, "Projectile" },
        { ITEM_CLASS_TRADE_GOODS, "Trade Goods" },
        { ITEM_CLASS_GENERIC, "Generic" },
        { ITEM_CLASS_RECIPE, "Recipe" },
        { ITEM_CLASS_MONEY, "Money" },
        { ITEM_CLASS_QUIVER, "Quiver" },
        { ITEM_CLASS_QUEST, "Quest" },
        { ITEM_CLASS_KEY, "Key" },
        { ITEM_CLASS_PERMANENT, "Permanent" },
        { ITEM_CLASS_MISC, "Miscellaneous" },
#ifdef MANGOSBOT_TWO
        { ITEM_CLASS_GLYPH, "Glyph" }
#endif
    };

    static const std::unordered_map<uint32, std::unordered_map<uint32, std::string>> itemSubclassNames = {
        { ITEM_CLASS_CONSUMABLE, {
            { 0, "Consumable" }, { 1, "Potion" }, { 2, "Elixir" }, { 3, "Flask" }, { 4, "Scroll" }, { 5, "Food" },
            { 6, "Item Enhancement" }, { 7, "Bandage" }, { 8, "Other" }
        }},
        { ITEM_CLASS_CONTAINER, {
            { 0, "Container" }, { 1, "Soul Container" }, { 2, "Herb Container" }, { 3, "Enchanting Container" },
            { 4, "Engineering Container" }, { 5, "Gem Container" }, { 6, "Mining Container" },
            { 7, "Leatherworking Container" }, { 8, "Inscription Container" }
        }},
        { ITEM_CLASS_WEAPON, {
            { 0, "Axe (1H)" }, { 1, "Axe (2H)" }, { 2, "Bow" }, { 3, "Gun" }, { 4, "Mace (1H)" }, { 5, "Mace (2H)" },
            { 6, "Polearm" }, { 7, "Sword (1H)" }, { 8, "Sword (2H)" }, { 9, "Obsolete" }, { 10, "Staff" },
            { 11, "Exotic (1H)" }, { 12, "Exotic (2H)" }, { 13, "Fist Weapon" }, { 14, "Miscellaneous" },
            { 15, "Dagger" }, { 16, "Thrown" }, { 17, "Spear" }, { 18, "Crossbow" }, { 19, "Wand" }, { 20, "Fishing Pole" }
        }},
        { ITEM_CLASS_GEM, {
            { 0, "Red" }, { 1, "Blue" }, { 2, "Yellow" }, { 3, "Purple" }, { 4, "Green" }, { 5, "Orange" },
            { 6, "Meta" }, { 7, "Simple" }, { 8, "Prismatic" }
        }},
        { ITEM_CLASS_ARMOR, {
            { 0, "Miscellaneous" }, { 1, "Cloth" }, { 2, "Leather" }, { 3, "Mail" }, { 4, "Plate" }, { 5, "Buckler" },
            { 6, "Shield" }, { 7, "Libram" }, { 8, "Idol" }, { 9, "Totem" }, { 10, "Sigil" }
        }},
        { ITEM_CLASS_REAGENT, {
            { 0, "Reagent" }
        }},
        { ITEM_CLASS_PROJECTILE, {
            { 0, "Wand" }, { 1, "Bolt" }, { 2, "Arrow" }, { 3, "Bullet" }, { 4, "Thrown" }
        }},
        { ITEM_CLASS_TRADE_GOODS, {
            { 0, "Trade Goods" }, { 1, "Parts" }, { 2, "Explosives" }, { 3, "Devices" }, { 4, "Jewelcrafting" },
            { 5, "Cloth" }, { 6, "Leather" }, { 7, "Metal & Stone" }, { 8, "Meat" }, { 9, "Herb" },
            { 10, "Elemental" }, { 11, "Other" }, { 12, "Enchanting" }, { 13, "Material" },
            { 14, "Armor Enchantment" }, { 15, "Weapon Enchantment" }
        }},
        { ITEM_CLASS_GENERIC, {
            { 0, "Generic" }
        }},
        { ITEM_CLASS_RECIPE, {
            { 0, "Book" }, { 1, "Leatherworking Pattern" }, { 2, "Tailoring Pattern" }, { 3, "Engineering Schematic" },
            { 4, "Blacksmithing" }, { 5, "Cooking Recipe" }, { 6, "Alchemy Recipe" }, { 7, "First Aid Manual" },
            { 8, "Enchanting Formula" }, { 9, "Fishing Manual" }, { 10, "Jewelcrafting Recipe" }
        }},
        { ITEM_CLASS_MONEY, {
            { 0, "Money" }
        }},
        { ITEM_CLASS_QUIVER, {
            { 0, "Quiver0" }, { 1, "Quiver1" }, { 2, "Quiver" }, { 3, "Ammo Pouch" }
        }},
        { ITEM_CLASS_QUEST, {
            { 0, "Quest" }
        }},
        { ITEM_CLASS_KEY, {
            { 0, "Key" }, { 1, "Lockpick" }
        }},
        { ITEM_CLASS_PERMANENT, {
            { 0, "Permanent" }
        }},
        { ITEM_CLASS_MISC, {
            { 0, "Junk" }, { 1, "Reagent" }, { 2, "Pet" }, { 3, "Holiday" }, { 4, "Other" }, { 5, "Mount" }
        }},
#ifdef MANGOSBOT_TWO
        { ITEM_CLASS_GLYPH, {
            { 1, "Glyph Warrior" }, { 2, "Glyph Paladin" }, { 3, "Glyph Hunter" }, { 4, "Glyph Rogue" },
            { 5, "Glyph Priest" }, { 6, "Glyph Death Knight" }, { 7, "Glyph Shaman" }, { 8, "Glyph Mage" },
            { 9, "Glyph Warlock" }, { 11, "Glyph Druid" }
        }}
#endif
    };



    out2 << "class: ";
    auto classIt = itemClassNames.find(proto->Class);
    if (classIt != itemClassNames.end())
        out2 << classIt->second;
    else
        out2 << "Unknown(" << proto->Class << ")";

    out2 << ", subclass: ";
    auto subclassMapIt = itemSubclassNames.find(proto->Class);
    if (subclassMapIt != itemSubclassNames.end())
    {
        auto subIt = subclassMapIt->second.find(proto->SubClass);
        if (subIt != subclassMapIt->second.end())
            out2 << subIt->second;
        else
            out2 << "Unknown(" << proto->SubClass << ")";
    }
    else
    {
        out2 << "Unknown(" << proto->SubClass << ")";
    }

    ai->TellPlayerNoFacing(requester, out2);

    std::ostringstream out3;

    //Print the items contained inside item.


    ai->TellPlayerNoFacing(requester, out3);
    
    return true;
}


bool DebugAction::HandleRPG(Event& event, Player* requester, const std::string& text)
{
    std::ostringstream out;

    if (text.size() < 4)
        return false;

    GuidPosition guidP;

    std::string link = text.substr(3);

    if (!link.empty())
    {
        std::list<ObjectGuid> gos = chat->parseGameobjects(link);
        if (!gos.empty())
        {
            for (auto go : gos)
            {
                guidP = GuidPosition(go,bot);
                guidP.updatePosition(bot->GetInstanceId());

                if(guidP)
                    break;
            }
        }
    }

    if (!guidP)
        return false;

    SET_AI_VALUE(GuidPosition, "rpg target", guidP);

    if (guidP.GetWorldObject(bot->GetInstanceId()))
        ai->TellPlayerNoFacing(requester, "Setting Rpg target to " + ChatHelper::formatWorldobject(guidP.GetWorldObject(bot->GetInstanceId())));

    return true;
}

bool DebugAction::HandleRPGTargets(Event& event, Player* requester, const std::string& text)
{
    Action* action = ai->GetAiObjectContext()->GetAction("choose rpg target");

    if (!action)
        return false;

    ChooseRpgTargetAction* targetAction = static_cast<ChooseRpgTargetAction*>(action);

    if (!targetAction)
        return false;

    targetAction->GetTargets(requester, true);

    return true;
}

bool DebugAction::HandleTravel(Event& event, Player* requester, const std::string& text)
{
    WorldPosition botPos = WorldPosition(bot);

    std::string destination = text.substr(7);

    DestinationList dests = ChooseTravelTargetAction::FindDestination(bot, destination);
    TravelDestination* dest = nullptr;

    if (!dests.empty())
    {
        WorldPosition botPos(bot);

        dest = *std::min_element(dests.begin(), dests.end(), [botPos](TravelDestination* i, TravelDestination* j) {return i->DistanceTo(botPos) < j->DistanceTo(botPos); });
    }

    if (dest)
    {
        std::list<uint8> chancesToGoFar = { 10,50,90 }; //Closest map, grid, cell.
        WorldPosition* point = dest->GetNextPoint(botPos, chancesToGoFar);

        if (!point)
            return false;

        std::vector<WorldPosition> beginPath, endPath;
        TravelNodeRoute route = sTravelNodeMap.getRoute(botPos, *point, beginPath, bot);

        std::ostringstream out; out << "Traveling to " << dest->GetTitle() << ": ";

        for (auto node : route.getNodes())
        {
            out << node->getName() << ", ";
        }

        ai->TellPlayerNoFacing(requester, out.str());

        return true;
    }
    else
    {
        ai->TellPlayerNoFacing(requester, "Destination " + destination + " not found.");
        return true;
    }
}

bool DebugAction::HandlePrintTravel(Event& event, Player* requester, const std::string& text)
{
    if (!sPlayerbotAIConfig.isLogOpen("travel.csv"))
        sPlayerbotAIConfig.openLog("travel.csv", "w", true);

    if (!sPlayerbotAIConfig.isLogOpen("travelActive.csv"))
        sPlayerbotAIConfig.openLog("travelActive.csv", "w", true);

    PlayerTravelInfo info(bot);

    std::vector<std::type_index> types = { typeid(QuestTravelDestination), typeid(RpgTravelDestination), typeid(ExploreTravelDestination), typeid(GrindTravelDestination), typeid(BossTravelDestination), typeid(GatherTravelDestination) };

    for (auto& type : types)
    {
        for (auto& dest : sTravelMgr.GetDestinations(info, (uint32)TravelDestinationPurpose::None, {}, true, 0))
        {
            bool isPossible = dest->IsPossible(info);
            bool isActive = dest->IsActive(bot, info);
            for (auto& point : dest->GetPoints())
            {
                GuidPosition* guidP = dynamic_cast<GuidPosition*>(point);

                if (guidP && guidP->IsEventUnspawned()) //Skip points that are not spawned due to events.
                {
                    continue;
                }

                std::ostringstream out;

                out << type.name() << ",";
                out << (uint32)((EntryTravelDestination*)dest)->GetPurpose() << ",";
                out << "\"";
                if (type == typeid(QuestTravelDestination))
                {
                    uint32 questId = ((QuestTravelDestination*)dest)->GetQuestId();
                    Quest const* quest = sObjectMgr.GetQuestTemplate(questId);
                    out << quest->GetTitle();
                }
                else if (type != typeid(ExploreTravelDestination))
                {
                    if (((EntryTravelDestination*)dest)->GetCreatureInfo())
                        out << ((EntryTravelDestination*)dest)->GetCreatureInfo()->Name;
                    else if (((EntryTravelDestination*)dest)->GetGoInfo())
                        out << ((EntryTravelDestination*)dest)->GetGoInfo()->name;
                    else
                        out << "";
                }
                else
                {
                    out << dest->GetTitle();
                }
                out << "\",";
                out << std::to_string(dest->GetEntry()) << ",";
                out << "\"" << dest->GetTitle() << "\",";
                out << isPossible << ",";
                out << isActive << ",";
                point->printWKT(out);
                out << point->getAreaLevel();

                //sPlayerbotAIConfig.log("travel.csv", out.str().c_str());
                if(isActive)
                    sPlayerbotAIConfig.log("travelActive.csv", out.str().c_str());
            }

            if (dest->GetPoints().empty())
                sLog.outError("Destination %s has no points!", dest->GetTitle().c_str());
        }
    }
    return true;
}

bool DebugAction::HandleValues(Event& event, Player* requester, const std::string& text)
{
    //    ai->TellPlayerNoFacing(requester, ai->GetAiObjectContext()->FormatValues(text.substr(7)));

    std::string param = "";
    if (text.length() > 6)
    {
        param = text.substr(7);
    }

    std::set<std::string> names = context->GetValues();
    std::vector<std::string> values;
    for (auto name : names)
    {
        UntypedValue* value = context->GetUntypedValue(name);
        if (!value)
        {
            continue;
        }

        if (!param.empty() && name.find(param) == std::string::npos)
        {
            continue;
        }

        std::string text = value->Format();
        if (text == "?")
        {
            continue;
        }

        values.push_back(name + "=" + text);
    }

    std::string valuestring = sPlayerbotHelpMgr.makeList(values, "[<part>]");

    std::vector<std::string> lines = Qualified::getMultiQualifiers(valuestring, "\n");
    for (auto& line : lines)
    {
        ai->TellPlayerNoFacing(requester, line, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
    }

    return true;
}

bool DebugAction::HandleLoot(Event& event, Player* requester, const std::string& text)
{
    std::string param = "";
    if (text.length() > 6)
        param = text.substr(5);

    bool doAction = ai->DoSpecificAction("add all loot", Event("debug", param), true);

    if (doAction)
        ai->TellPlayerNoFacing(requester, "Added new loot");
    else
        ai->TellPlayerNoFacing(requester, "No new loot");

    LootObjectStack* loots = AI_VALUE(LootObjectStack*, "available loot");

    for (int i = 0; i < 2000; i++)
    {
        LootObject loot = loots->GetLoot();

        if (loot.IsEmpty())
            break;

        WorldObject* wo = ai->GetWorldObject(loot.guid);

        if (wo)
            ai->TellPlayerNoFacing(requester, chat->formatWorldobject(wo) + " " + (loot.IsLootPossible(bot) ? "can loot" : "can not loot"));
        else
            ai->TellPlayerNoFacing(requester,
                                   std::to_string(loot.guid) + " " + (loot.IsLootPossible(bot) ? "can loot" : "can not loot") + " " + std::to_string(loot.guid.GetEntry()));

        if (loot.guid.IsGameObject())
        {
            GameObject* go = ai->GetGameObject(loot.guid);

            if (go->ActivateToQuest(bot))
                ai->TellPlayerNoFacing(requester, std::to_string(go->GetGoType()) + " for quest");
            else
                ai->TellPlayerNoFacing(requester, std::to_string(go->GetGoType()));
        }

        loots->Remove(loot.guid);
    }

    ai->DoSpecificAction("add all loot", Event(), true);

    return true;
}

bool DebugAction::HandleDrops(Event& event, Player* requester, const std::string& text)
{
    std::string textSubstr;
    std::ostringstream out;
    for (auto entry : chat->parseWorldEntries(textSubstr = text.substr(6)))
    {
        std::list<uint32> itemIds = GAI_VALUE2(std::list<uint32>, "entry loot list", entry);

        if (itemIds.empty())
            out << chat->formatWorldEntry(entry) << " no drops found.";
        else
            out << chat->formatWorldEntry(entry) << " " << std::to_string(itemIds.size()) << " drops found:";

        ai->TellPlayerNoFacing(requester, out);
        out.str("");
        out.clear();

        std::vector<std::pair<uint32, float>> chances;

        for (auto itemId : itemIds)
        {
            std::vector<std::string> qualifiers = { std::to_string(entry) , std::to_string(itemId) };
            std::string qualifier = Qualified::MultiQualify(qualifiers, " ");
            float chance = GAI_VALUE2(float, "loot chance", qualifier);
            if (chance > 0 && sObjectMgr.GetItemPrototype(itemId))
            {
                chances.push_back(std::make_pair(itemId, chance));
            }
        }

        std::sort(chances.begin(), chances.end(), [](std::pair<uint32, float> i, std::pair<int32, float> j) {return i.second > j.second; });

        chances.resize(std::min(20, (int)chances.size()));

        for (auto chance : chances)
        {
            out << chat->formatItem(sObjectMgr.GetItemPrototype(chance.first), 0, 0) << ": " << chance.second << "%";
            ai->TellPlayerNoFacing(requester, out);
            out.str("");
            out.clear();
        }
    }
    return true;
}

bool DebugAction::HandleTaxi(Event& event, Player* requester, const std::string& text)
{
    for (uint32 i = 1; i < sTaxiNodesStore.GetNumRows(); ++i)
    {
        if (!bot->m_taxi.IsTaximaskNodeKnown(i))
            continue;

        TaxiNodesEntry const* taxiNode = sTaxiNodesStore.LookupEntry(i);

        std::ostringstream out;

        out << taxiNode->name[0];

        ai->TellPlayerNoFacing(requester, out);
    }
    return true;
}

bool DebugAction::HandlePrice(Event& event, Player* requester, const std::string& text)
{
    std::ostringstream out;

    if (text.size() < 7)
        return false;

    std::string link = text.substr(6);
    ItemIds ids = ChatHelper::parseItems(link);

    for (auto id : ids)
    {
        std::ostringstream out;

        const ItemPrototype* proto = sObjectMgr.GetItemPrototype(id);
        if (!proto)
            continue;

        out << ChatHelper::formatItem(proto);
        out << " Buy from vendor: " << ChatHelper::formatMoney(proto->BuyPrice);
        out << " Sell to vendor: " << ChatHelper::formatMoney(proto->SellPrice);
        out << " Median buyout from AH (price per item): " << ChatHelper::formatMoney(ItemUsageValue::GetAHMedianBuyoutPricePerItem(proto));
        out << " Lowest AH listing buyout (price per item): " << ChatHelper::formatMoney(ItemUsageValue::GetAHListingLowestBuyoutPricePerItem(proto));
        out << " Sell to AH: " << ChatHelper::formatMoney(ItemUsageValue::GetBotAHSellMinPrice(proto)) << " to " << ChatHelper::formatMoney(ItemUsageValue::GetBotAHSellMaxPrice(proto));

        ai->TellPlayerNoFacing(requester, out);
    }

    return true;
}

bool DebugAction::HandleNC(Event& event, Player* requester, const std::string& text)
{
    std::ostringstream out;

    std::string name;
    if (text.size() > 3)
        name = text.substr(3);

    std::list<TriggerNode*> triggerNodes;

    for (auto& strategyName : context->GetSupportedStrategies())
    {
        if (!name.empty() && strategyName.find(name) == std::string::npos)
            continue;

        Strategy* strategy = context->GetStrategy(strategyName);

        ai->TellPlayerNoFacing(requester, "s:" + strategyName + (ai->HasStrategy(strategyName, BotState::BOT_STATE_NON_COMBAT) ? " [active]" : ""), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false, false);

        strategy->InitTriggers(triggerNodes, BotState::BOT_STATE_NON_COMBAT);

        //Loop over all triggers of this strategy.
        for (auto& triggerNode : triggerNodes)
        {
            Trigger* trigger = context->GetTrigger(triggerNode->getName());

            if (trigger)
            {
                ai->TellPlayerNoFacing(requester, "t: " + triggerNode->getName() + (trigger->IsActive() ? " [active]" : ""), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false, false);
                triggerNode->setTrigger(trigger);

                Trigger* trigger = triggerNode->getTrigger();

                NextAction** nextActions = triggerNode->getHandlers();

                bool isRpg = false;

                //Loop over all actions triggered by this trigger and check if any is an 'rpg action'.
                for (int32 i = 0; i < NextAction::size(nextActions); i++)
                {
                    NextAction* nextAction = nextActions[i];

                    Action* action = ai->GetAiObjectContext()->GetAction(nextAction->getName());

                    if (action)
                    {
                        ai->TellPlayerNoFacing(requester, "a:  " + nextAction->getName() + triggerNode->getName() + (action->isUseful() ? " [usefull]" : "") + (action->isPossible() ? " [possible]" : "") + "(" + std::to_string(nextAction->getRelevance()) + ")"
                        , PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false, false);
                    }
                    else
                    {
                        ai->TellPlayerNoFacing(requester,
                                               "a:  [unknown]" + nextAction->getName() + triggerNode->getName() + " (" + std::to_string(nextAction->getRelevance()) + ")",
                                               PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false, false);
                    }
                    
                }
                NextAction::destroy(nextActions);
            }
        }

        for (std::list<TriggerNode*>::iterator i = triggerNodes.begin(); i != triggerNodes.end(); i++)
        {
            TriggerNode* trigger = *i;
            delete trigger;
        }

        triggerNodes.clear();
    }        

    return true;
}

bool DebugAction::HandleAddNode(Event& event, Player* requester, const std::string& text)
{
    WorldPosition pos(bot);

    std::string name = "USER:" + text.substr(9);

    TravelNode* startNode = sTravelNodeMap.addNode(pos, name, false, false);

    for (auto& endNode : sTravelNodeMap.getNodes(pos, 2000))
    {
        endNode->setLinked(false);
    }

    ai->TellPlayerNoFacing(requester, "Node " + name + " created.");
    
    sTravelNodeMap.setHasToGen();

    return true;
}

bool DebugAction::HandleRemNode(Event& event, Player* requester, const std::string& text)
{
    WorldPosition pos(bot);

    TravelNode* startNode = sTravelNodeMap.getNode(pos, nullptr, 50);

    if (!startNode)
        return false;

    if (startNode->isImportant())
    {
        ai->TellPlayerNoFacing(requester, "Node can not be removed.");
    }
    sTravelNodeMap.m_nMapMtx.lock();
    sTravelNodeMap.removeNode(startNode);
    ai->TellPlayerNoFacing(requester, "Node removed.");
    sTravelNodeMap.m_nMapMtx.unlock();

    sTravelNodeMap.setHasToGen();

    return true;
}

bool DebugAction::HandleResetNode(Event& event, Player* requester, const std::string& text)
{
    for (auto& node : sTravelNodeMap.getNodes())
        node->setLinked(false);
    return true;
}

bool DebugAction::HandleResetPath(Event& event, Player* requester, const std::string& text)
{
    for (auto& node : sTravelNodeMap.getNodes())
        for (auto& path : *node->getLinks())
            node->removeLinkTo(path.first, true);
    return true;
}

bool DebugAction::HandleGenNode(Event& event, Player* requester, const std::string& text)
{
    //Pathfinder
    sTravelNodeMap.generateNodes();
    return true;
}

bool DebugAction::HandleGenPath(Event& event, Player* requester, const std::string& text)
{
    sTravelNodeMap.generatePaths(false);
    return true;
}

bool DebugAction::HandleCropPath(Event& event, Player* requester, const std::string& text)
{
    sTravelNodeMap.removeUselessPaths();
    return true;
}

bool DebugAction::HandleSaveNode(Event& event, Player* requester, const std::string& text)
{
    sTravelNodeMap.printNodeStore();
    sTravelNodeMap.saveNodeStore(true);
    return true;
}

bool DebugAction::HandleLoadNode(Event& event, Player* requester, const std::string& text)
{
    std::thread t([] {if (sTravelNodeMap.removeNodes())
        sTravelNodeMap.loadNodeStore(); });

    t.detach();

    return true;
}

bool DebugAction::HandleShowNode(Event& event, Player* requester, const std::string& text)
{
    WorldPosition pos(bot);

    std::vector<TravelNode*> nodes = sTravelNodeMap.getNodes(pos, 500);

    for (auto& node : nodes)
    {
        for (auto& l : *node->getLinks())
        {
            Unit* start = nullptr;
            std::list<ObjectGuid> units;

            uint32 time = 60 * IN_MILLISECONDS;

            std::vector<WorldPosition> ppath = l.second->getPath();

            for (auto p : ppath)
            {
                Creature* wpCreature = bot->SummonCreature(2334, p.getX(), p.getY(), p.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 20000.0f);
                //addAura(246, wpCreature);
                units.push_back(wpCreature->GetObjectGuid());

                if(!start)
                    ai->AddAura(wpCreature, 1130);
                else
                    ai->AddAura(wpCreature, 246);
                    

                if (!start)
                    start = wpCreature;
            }
        }
    }
    return true;
}

bool DebugAction::HandleDSpell(Event& event, Player* requester, const std::string& text)
{
    uint32 spellEffect = stoi(text.substr(7));

    Unit* prev = bot;

    for (float i = 0; i < 60; i++)
    {
        float ang = i / 60 * M_PI_F * 4;
        float dist = i / 60 * 30;

        WorldPosition botPos(bot);
        WorldPosition botPos1 = botPos;

        botPos.setX(botPos.getX() + cos(ang) * dist);
        botPos.setY(botPos.getY() + sin(ang) * dist);
        botPos.setZ(botPos.getHeight(bot->GetInstanceId()) + 2);

        Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

        FakeSpell(spellEffect, wpCreature, wpCreature, prev->GetObjectGuid(), {}, {}, botPos, botPos);

        prev = wpCreature;
    }
    return true;
}

bool DebugAction::HandleVSpell(Event& event, Player* requester, const std::string& text)
{
    uint32 spellEffect = stoi(text.substr(7));

    Unit* prev = bot;

    for (float i = 0; i < 60; i++)
    {
        float ang = i / 60 * M_PI_F * 4;
        float dist = i / 60 * 30;

        WorldPosition botPos(bot);
        WorldPosition botPos1 = botPos;

        botPos.setX(botPos.getX() + cos(ang) * dist);
        botPos.setY(botPos.getY() + sin(ang) * dist);
        botPos.setZ(botPos.getHeight(bot->GetInstanceId()) + 2);

        Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);            

        if (wpCreature)
        {
            WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 8 + 4);        // visual effect on guid
            data << wpCreature->GetObjectGuid();
            data << uint32(spellEffect);                               // index from SpellVisualKit.dbc
            wpCreature->SendMessageToSet(data, true);                
        }
    }
    return true;
}

bool DebugAction::HandleASpell(Event& event, Player* requester, const std::string& text)
{
    uint32 spellEffect = stoi(text.substr(7));

    Unit* prev = bot;

    for (float i = 0; i < 60; i++)
    {
        float ang = i / 60 * M_PI_F * 4;
        float dist = i / 60 * 30;

        WorldPosition botPos(bot);
        WorldPosition botPos1 = botPos;

        botPos.setX(botPos.getX() + cos(ang) * dist);
        botPos.setY(botPos.getY() + sin(ang) * dist);
        botPos.setZ(botPos.getHeight(bot->GetInstanceId()) + 2);

        Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 5000.0f + i * 100.0f);
        wpCreature->SetObjectScale(0.5f);

        if (wpCreature)
        {
            addAura(spellEffect, wpCreature);
        }
    }
    return true;
}

bool DebugAction::HandleCSpell(Event& event, Player* requester, const std::string& text)
{
    uint32 spellEffect = stoi(text.substr(7));

    std::list<ObjectGuid> units;

    for (float i = 0; i < 60; i++)
    {
        float ang = i / 60 * M_PI_F * 4;
        float dist = i / 60 * 30;

        WorldPosition botPos(bot);

        botPos.setX(botPos.getX() + cos(ang) * dist);
        botPos.setY(botPos.getY() + sin(ang) * dist);
        botPos.setZ(botPos.getHeight(bot->GetInstanceId()) + 2);

        Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);
        units.push_back(wpCreature->GetObjectGuid());
    }

    WorldPosition botPos(bot);

    if(urand(0,1))
        FakeSpell(spellEffect, bot, bot, ObjectGuid(), units, {}, botPos, botPos);
    else
        FakeSpell(spellEffect, bot, bot, units.front(), units, {}, botPos, botPos);

    return true;
}

bool DebugAction::HandleFSpell(Event& event, Player* requester, const std::string& text)
{
    uint32 spellEffect = stoi(text.substr(7));
    {
        WorldPacket data(SMSG_SPELL_START, (8 + 8 + 4 + 2 + 4));

        data.Initialize(SMSG_SPELL_START);
        data << bot->GetPackGUID();
        data << bot->GetPackGUID();
        data << uint32(spellEffect);
        data << uint16(0);
        data << uint32(0);
        data << uint16(2);
        data << requester->GetObjectGuid();
        bot->SendMessageToSet(data, true);
    }

    {
        WorldPacket data(SMSG_SPELL_GO, 53);                    // guess size
        data << bot->GetPackGUID();
        data << bot->GetPackGUID();
        data << uint32(spellEffect);  // spellID
        data << uint8(0) << uint8(1);   // flags
        data << uint8(1);			   // amount of targets
        data << requester->GetObjectGuid();
        data << uint8(0);
        data << uint16(2);
        data << requester->GetObjectGuid();
        bot->SendMessageToSet(data, true);
    }

    return true;
}

bool DebugAction::HandleSpell(Event& event, Player* requester, const std::string& text)
{
    uint32 spellEffect = stoi(text.substr(6));
    requester->GetSession()->SendPlaySpellVisual(bot->GetObjectGuid(), spellEffect);
    return true;
}

bool DebugAction::HandleTSpellMap(Event& event, Player* requester, const std::string& text)
{
    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            uint32 spellEffect = stoi(text.substr(10));
            uint32 effect = dx + dy * 10 + spellEffect * 100;
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            Creature* wpCreature = bot->SummonCreature(6, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

            if (wpCreature)
            {
                std::ostringstream out;
                out << "effect ";
                out << effect;

                const std::string& Cname = out.str();

                wpCreature->MonsterSay(Cname.c_str(), 0, requester);
            }
        }
    }
    return true;
}

bool DebugAction::HandleUSpellMap(Event& event, Player* requester, const std::string& text)
{
    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            uint32 spellEffect = stoi(text.substr(10));
            uint32 effect = dx + dy * 10 + spellEffect * 100;
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            Creature* wpCreature = bot->SummonCreature(effect, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);
        }
    }
    return true;
}

bool DebugAction::HandleDSpellMap(Event& event, Player* requester, const std::string& text)
{
    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            uint32 spellEffect = stoi(text.substr(10));
            uint32 effect = dx + dy * 10 + spellEffect * 100;
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            FakeSpell(effect, bot, nullptr, ObjectGuid(), {}, {}, botPos, botPos, true);
        }
    }
    return true;
}

bool DebugAction::HandleVSpellMap(Event& event, Player* requester, const std::string& text)
{
    std::vector<WorldPacket> datMap;
    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            uint32 spellEffect = stoi(text.substr(10));
            uint32 effect = dx + dy * 10 + spellEffect * 100;
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);
                                   
            if (wpCreature)
            {
                WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 8 + 4);        // visual effect on guid
                data << wpCreature->GetObjectGuid();
                data << uint32(effect); ;                               // index from SpellVisualKit.dbc
                //wpCreature->SendMessageToSet(data, true);
                datMap.push_back(data);

                //wpCreature->MonsterMoveWithSpeed(botPos.getX(), botPos.getY()+80, botPos.getZ(), 8.0f,true,true);
            }
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (auto dat : datMap)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        bot->SendMessageToSet(dat, true);
    }
    for (auto dat : datMap)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        bot->SendMessageToSet(dat, true);
    }

    return true;
}

bool DebugAction::HandleISpellMap(Event& event, Player* requester, const std::string& text)
{
    std::vector<WorldPacket> datMap;
    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            uint32 spellEffect = stoi(text.substr(10));
            uint32 effect = dx + dy * 10 + spellEffect * 100;
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            Creature* wpCreature = bot->SummonCreature(6, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

            if (wpCreature)
            {
                WorldPacket data(SMSG_PLAY_SPELL_IMPACT, 8 + 4);        // visual effect on player
                data << wpCreature->GetObjectGuid();
                data << uint32(effect);                                 // index from SpellVisualKit.dbc
                //wpCreature->SendMessageToSet(data, true);
                datMap.push_back(data);
            }
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (auto dat : datMap)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        bot->SendMessageToSet(dat, true);
    }
    for (auto dat : datMap)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        bot->SendMessageToSet(dat, true);
    }

    return true;
}

bool DebugAction::HandleCSpellMap(Event& event, Player* requester, const std::string& text)
{
    Creature* wpCreature = nullptr;
    Creature* lCreature = nullptr;
    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            uint32 spellEffect = stoi(text.substr(10));
            uint32 effect = dx + dy * 10 + spellEffect * 100;
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            wpCreature = bot->SummonCreature(6, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

            if (wpCreature && lCreature)
            {
                wpCreature->CastSpell(lCreature, effect, TRIGGERED_OLD_TRIGGERED);
            }

            lCreature = wpCreature;
        }
    }
    return true;
}

bool DebugAction::HandleASpellMap(Event& event, Player* requester, const std::string& text)
{
    Creature* wpCreature = nullptr;

    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            uint32 spellEffect = stoi(text.substr(10));
            uint32 effect = dx + dy * 10 + spellEffect * 100;
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

            if (wpCreature)
            {
                addAura(effect, wpCreature);
            }
        }
    }
    return true;
}

bool DebugAction::HandleGSpellMap(Event& event, Player* requester, const std::string& text)
{
    std::vector<ObjectGuid> all_targets;// = { bot->GetObjectGuid(), master->GetObjectGuid() };
    //vector<ObjectGuid> all_dummies = { bot->GetObjectGuid(), master->GetObjectGuid() };

    /*list<ObjectGuid> a_targets = *context->GetValue<std::list<ObjectGuid> >("all targets");
    for (auto t : a_targets)
    {
        all_targets.push_back(t);
    }
    */


    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

            all_targets.push_back(wpCreature->GetObjectGuid());
        }
    }

    all_targets.push_back(requester->GetObjectGuid());
    all_targets.push_back(bot->GetObjectGuid());

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (uint32 i = 0; i < 2; i++)
        for (int32 dx = 0; dx < 10; dx++)
        {
            for (int32 dy = 0; dy < 10; dy++)
            {
                uint32 spellEffect = stoi(text.substr(10));
                uint32 effect = dx + dy * 10 + spellEffect * 100;

                uint32 i = dx + dy * 10;
                std::list<ObjectGuid> hits, miss;

                SpellEntry const* spellInfo = sServerFacade.LookupSpellInfo(effect);

                if (spellInfo && spellInfo->speed > 0)
                    for (auto tar : all_targets)
                    {
                        //hits.push_back(tar);

                        switch (urand(0, 10))
                        {
                        case 0:
                            hits.push_back(tar);
                            break;
                        case 1:
                            miss.push_back(tar);
                            break;
                        case 2:
                        case 3:
                            break;
                        }
                    }

                Unit* realCaster = ai->GetUnit(all_targets[i]);//ai->GetUnit(all_targets[urand(0, all_targets.size() - 1)]);
                Unit* caster = ai->GetUnit(all_targets[i]);//ai->GetUnit(all_targets[urand(0, all_targets.size() - 1)]);
                Unit* target = ai->GetUnit(all_targets[i + 1]);

                if (!realCaster)
                    realCaster = bot;

                if (!caster)
                    caster = bot;

                if (!target)
                    target = requester;

                FakeSpell(effect, realCaster, caster, target->GetObjectGuid(), hits, miss, WorldPosition(caster), WorldPosition(target));

                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        }
    return true;
}

bool DebugAction::HandleMSpellMap(Event& event, Player* requester, const std::string& text)
{
    std::vector<ObjectGuid> all_targets;

    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

            all_targets.push_back(wpCreature->GetObjectGuid());
        }
    }

    all_targets.push_back(requester->GetObjectGuid());
    all_targets.push_back(bot->GetObjectGuid());

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (uint32 i = 0; i < 2; i++)
        for (int32 dx = 0; dx < 10; dx++)
        {
            for (int32 dy = 0; dy < 10; dy++)
            {
                uint32 spellEffect = stoi(text.substr(10));
                uint32 effect = dx + dy * 10 + spellEffect * 100;

                uint32 i = dx + dy * 10;
                std::list<ObjectGuid> hits, miss;

                SpellEntry const* spellInfo = sServerFacade.LookupSpellInfo(effect);

                    for (auto tar : all_targets)
                    {
                        //hits.push_back(tar);

                        switch (urand(0, 10))
                        {
                        case 0:
                            hits.push_back(tar);
                            break;
                        case 1:
                            miss.push_back(tar);
                            break;
                        case 2:
                        case 3:
                            break;
                        }
                    }

                Unit* realCaster = ai->GetUnit(all_targets[i]);//ai->GetUnit(all_targets[urand(0, all_targets.size() - 1)]);
                Unit* caster = ai->GetUnit(all_targets[i]);//ai->GetUnit(all_targets[urand(0, all_targets.size() - 1)]);
                Unit* target = ai->GetUnit(all_targets[i + 1]);

                if (!realCaster)
                    realCaster = bot;

                if (!caster)
                    caster = bot;

                if (!target)
                    target = requester;

                requester->GetSession()->SendPlaySpellVisual(caster->GetObjectGuid(), 5036);
                FakeSpell(effect, realCaster, caster, target->GetObjectGuid(), hits, miss, WorldPosition(caster), WorldPosition(target));

                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        }
    return true;
}

bool DebugAction::HandleSoundMap(Event& event, Player* requester, const std::string& text)
{
    uint32 soundEffects = stoi(text.substr(9));
    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            uint32 effect = dx + dy * 10 + soundEffects * 100;
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            Creature* wpCreature = bot->SummonCreature(6, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

            wpCreature->PlayDistanceSound(effect);
        }
    }

    return true;
}

bool DebugAction::HandleSounds(Event& event, Player* requester, const std::string& text)
{
    uint32 soundEffects = stoi(text.substr(6));

    for (uint32 i = 0; i < 100; i++)
    {
        bot->PlayDistanceSound(i + soundEffects * 100);
        bot->Say(std::to_string(i + soundEffects * 100), 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return true;
}

bool DebugAction::HandleDSound(Event& event, Player* requester, const std::string& text)
{
    uint32 soundEffect = stoi(text.substr(7));
    bot->PlayDirectSound(soundEffect);
    return true;
}

bool DebugAction::HandleSound(Event& event, Player* requester, const std::string& text)
{
    uint32 soundEffect = stoi(text.substr(6));
    bot->PlayDistanceSound(soundEffect);
    return true;
}