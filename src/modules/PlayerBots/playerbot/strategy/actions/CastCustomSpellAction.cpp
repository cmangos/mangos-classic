
#include "playerbot/playerbot.h"
#include "CastCustomSpellAction.h"

#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "CheckMountStateAction.h"

using namespace ai;

int FindLastSeparator(std::string text, std::string sep)
{
    int pos = text.rfind(sep);
    if (pos == std::string::npos) return pos;

    int lastLinkBegin = text.rfind("|H");
    int lastLinkEnd = text.find("|h|r", lastLinkBegin + 1);
    if (pos >= lastLinkBegin && pos <= lastLinkEnd)
        pos = text.find_last_of(sep, lastLinkBegin);

    return pos;
}

static inline void ltrim(std::string& s) 
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) 
    {
        return !std::isspace(ch);
    }));
}

bool CastCustomSpellAction::Execute(Event& event)
{
    // only allow proper vehicle seats
    if (ai->IsInVehicle() && !ai->IsInVehicle(false, false, true))
        return false;

    Unit* target = nullptr;
    std::string text = getQualifier();

    if(text.empty() || text == "travel")
        text = event.getParam();

    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    // Process summon request
    if (CastSummonPlayer(requester, text))
    {
        return true;
    }

    // Grab the first game object or unit from the parameters as target.
    GameObject* gameObjectTarget = nullptr;
    std::list<ObjectGuid> gos = chat->parseGameobjects(text);
    if (!gos.empty())
    {
        for (auto go : gos)
        {
            if (go.IsGameObject())
            {
                gameObjectTarget = ai->GetGameObject(go);

                if (gameObjectTarget)
                {
                    chat->eraseAllSubStr(text, chat->formatWorldobject(gameObjectTarget));
                    break;
                }
            }
            else if (go.IsUnit())
            {
                target = ai->GetUnit(go);

                if (target)
                {
                    chat->eraseAllSubStr(text, chat->formatWorldobject(target));
                    break;
                }
            }
        }

        ltrim(text);
    }
    
    if (!target)
    {
        if (requester && requester->GetSelectionGuid())
        {
            target = ai->GetUnit(requester->GetSelectionGuid());
        }
    }

    if (!target)
        target = GetTarget();

    if (!target)
        target = bot;

    if (!requester) //Use self as requester for permissions.
        requester = bot;

    Item* itemTarget = nullptr;
    int pos = FindLastSeparator(text, " ");
    int castCount = 1;
    if (pos != std::string::npos)
    {
        std::string param = text.substr(pos + 1);
        std::list<Item*> items = ai->InventoryParseItems(param, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
        if (!items.empty()) itemTarget = *items.begin();
        else
        {
            castCount = atoi(param.c_str());
            if (castCount > 0)
                text = text.substr(0, pos);
        }
    }

    uint32 spell = AI_VALUE2(uint32, "spell id", text);

    if (!spell)
    {
        std::map<std::string, std::string> args;
        args["%spell"] = text;
        ai->TellPlayerNoFacing(requester, BOT_TEXT2("cast_spell_command_error_unknown_spell", args));
        return false;
    }

    SpellEntry const* pSpellInfo = sServerFacade.LookupSpellInfo(spell);
    if (!pSpellInfo)
    {
        std::map<std::string, std::string> args;
        args["%spell"] = text;
        ai->TellPlayerNoFacing(requester, BOT_TEXT2("cast_spell_command_error_unknown_spell", args));
        return false;
    }

    // Don't use totem items for totem spells (except enchanting bars)
    if (pSpellInfo->Totem[0] > 0 && 
        pSpellInfo->Totem[0] != 5060 &&
        pSpellInfo->Totem[0] != 6218 &&
        pSpellInfo->Totem[0] != 6339 &&
        pSpellInfo->Totem[0] != 11130 &&
        pSpellInfo->Totem[0] != 11145 &&
        pSpellInfo->Totem[0] != 16207 &&
        pSpellInfo->Totem[0] != 22461 &&
        pSpellInfo->Totem[0] != 22462 &&
        pSpellInfo->Totem[0] != 22463 &&
        pSpellInfo->Totem[0] != 44452)
    {
        itemTarget = nullptr;
    }

    if ((pSpellInfo->Targets & TARGET_FLAG_ITEM) || (pSpellInfo->Targets & TARGET_FLAG_SELF))
        target = bot;

    WorldObject* woTarget = nullptr;
    if (gameObjectTarget)
        woTarget = gameObjectTarget;
    else
        woTarget = target;

    if (woTarget != bot && !sServerFacade.IsInFront(bot, woTarget, sPlayerbotAIConfig.sightDistance, CAST_ANGLE_IN_FRONT))
    {
        sServerFacade.SetFacingTo(bot, woTarget);
        SetDuration(sPlayerbotAIConfig.globalCoolDown);
        std::ostringstream msg;
        msg << "cast " << text;
        ai->HandleCommand(CHAT_MSG_WHISPER, msg.str(), event.getOwner() ? *event.getOwner() : *bot);
        return true;
    }

    if (bot->IsMoving())
    {
        ai->StopMoving();
    }

    if (AI_VALUE2(uint32, "current mount speed", "self target"))
    {
        if (bot->IsFlying() && WorldPosition(bot).currentHeight() > 10.0f)
            return false;

        ai->Unmount();
    }

    ai->RemoveShapeshift();

    std::ostringstream replyStr;
    std::map<std::string, std::string> replyArgs;
    if (!pSpellInfo->EffectItemType[0])
    {
        replyStr << BOT_TEXT("cast_spell_command_spell");

        replyArgs["%spell"] = ChatHelper::formatSpell(pSpellInfo);
    }
    else
    {
        replyStr << BOT_TEXT("cast_spell_command_craft");

        uint32 newItemId = pSpellInfo->EffectItemType[0];

        if (!newItemId)
            replyArgs["%spell"] = ChatHelper::formatSpell(pSpellInfo);
        else
        {

            ItemPrototype const* proto = ObjectMgr::GetItemPrototype(newItemId);
            replyArgs["%spell"] = ChatHelper::formatItem(proto);
        }
    }    

    if (bot->GetTrader())
    {
        replyStr << " " << BOT_TEXT("command_target_trade");
    }
    else if (itemTarget)
    {
        replyStr << " " << BOT_TEXT("command_target_item");
        replyArgs["%item"] = chat->formatItem(itemTarget);
    }
    else if (gameObjectTarget)
    {
        replyStr << " " << BOT_TEXT("command_target_item");
        replyArgs["%item"] = chat->formatGameobject(gameObjectTarget);
    }
    else if (pSpellInfo->EffectItemType[0])
    {
        replyStr << "";
    }
    else if (target == bot)
    {
        replyStr << " " << BOT_TEXT("command_target_self");
    }
    else
    {
        replyStr << " " << BOT_TEXT("command_target_unit");
        replyArgs["%unit"] = target->GetName();
    }

    SpellCastResult checkResult;
    const bool canCast = gameObjectTarget ? ai->CanCastSpell(spell, gameObjectTarget, 0, true, false, false, false, &checkResult) : ai->CanCastSpell(spell, target, 0, true, itemTarget, false, false, false, &checkResult);
    if (!bot->GetTrader() && !canCast)
    {
        std::map<std::string, std::string> args;
        args["%spell"] = replyArgs["%spell"];
        args["%fail_reason"] = BOT_TEXT2(GetSpellCastResultString(checkResult), args);
        ai->TellPlayerNoFacing(requester, BOT_TEXT2("cast_spell_command_error", args));
        return false;
    }

    MotionMaster& mm = *bot->GetMotionMaster();
    uint32 spellDuration = sPlayerbotAIConfig.globalCoolDown;

    bool result = gameObjectTarget ? ai->CastSpell(spell, gameObjectTarget, itemTarget, true, &spellDuration) : ai->CastSpell(spell, target, itemTarget, true, &spellDuration);
    if (result)
    {
        SetDuration(spellDuration);

        if (castCount > 1)
        {
            std::ostringstream cmd;
            cmd << castString(target) << " " << text << " " << (castCount - 1);
            ai->HandleCommand(CHAT_MSG_WHISPER, cmd.str(), *requester);

            replyStr << " " << BOT_TEXT("cast_spell_command_amount");
            replyArgs["%amount"] = std::to_string(castCount - 1);
        }

        ai->TellPlayerNoFacing(requester, BOT_TEXT2(replyStr.str(), replyArgs), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
    }
    else
    {
        std::map<std::string, std::string> args;
        args["%spell"] = replyArgs["%spell"];
        ai->TellPlayerNoFacing(requester, BOT_TEXT2("cast_spell_command_error_failed", args));
    }

    return result;
}

bool CastCustomSpellAction::CastSummonPlayer(Player* requester, std::string command)
{
    if (bot->getClass() == CLASS_WARLOCK)
    {
        if (command.find("summon") != std::string::npos)
        {
            // Don't summon player when trying to summon warlock pet
            if (command.find("imp") != std::string::npos || 
                command.find("voidwalker") != std::string::npos || 
                command.find("succubus") != std::string::npos || 
                command.find("felhunter") != std::string::npos ||
                command.find("felguard") != std::string::npos ||
                command.find("felsteed") != std::string::npos ||
                command.find("dreadsteed") != std::string::npos)
            {
                return false;
            }

            if (!ai->IsStateActive(BotState::BOT_STATE_COMBAT))
            {
                // Get target from command parameters
                uint8 membersAroundSummoner = 0;
                Player* target = nullptr;
                const std::string summonString = "summon ";
                const int pos = command.find(summonString);
                if (pos != std::string::npos)
                {
                    // Get player name
                    std::string playerName = command.substr(summonString.size());

                    const Group* group = bot->GetGroup();
                    if (group && !playerName.empty())
                    {
                        const Group::MemberSlotList& groupSlot = group->GetMemberSlots();
                        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
                        {
                            Player* member = sObjectMgr.GetPlayer(itr->guid);
                            if (member)
                            {
                                if (member->GetName() == playerName)
                                {
                                    target = member;
                                }

                                if (member->GetDistance(bot) <= sPlayerbotAIConfig.reactDistance)
                                {
                                    membersAroundSummoner++;
                                }
                            }
                        }
                    }
                }
                else
                {
                    // Get target from requester target
                    if (requester && requester->GetSelectionGuid())
                    {
                        const ObjectGuid& targetGuid = requester->GetSelectionGuid();
                        if (targetGuid.IsPlayer())
                        {
                            const Group* group = bot->GetGroup();
                            if (group)
                            {
                                const Group::MemberSlotList& groupSlot = group->GetMemberSlots();
                                for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
                                {
                                    Player* member = sObjectMgr.GetPlayer(itr->guid);
                                    if (member)
                                    {
                                        if (itr->guid == targetGuid)
                                        {
                                            target = member;
                                        }

                                        if (ai->IsSafe(member) && member->GetDistance(bot) <= sPlayerbotAIConfig.reactDistance)
                                        {
                                            membersAroundSummoner++;
                                        }
                                    }
                                }
                            }
                        }
                    }   
                }

                if (target)
                {
                    if (membersAroundSummoner >= 3)
                    {
                        if (target->isRealPlayer())
                        {
                            float x, y, z;
                            bot->GetPosition(x, y, z);
                            target->SetSummonPoint(bot->GetMapId(), x, y, z, bot->GetObjectGuid());

                            WorldPacket data(SMSG_SUMMON_REQUEST, 8 + 4 + 4);
                            data << bot->GetObjectGuid();
                            data << uint32(bot->GetZoneId());
                            data << uint32(MAX_PLAYER_SUMMON_DELAY * IN_MILLISECONDS);
                            target->GetSession()->SendPacket(data);
                        }
                        else
                        {
                            target->TeleportTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetOrientation());
                            if (target->isRealPlayer())
                                target->SendHeartBeat();
                        }

                        std::ostringstream msg;
                        msg << "Summoning " << target->GetName();

                        std::map<std::string, std::string> args;
                        args["%target"] = target->GetName();
                        ai->TellPlayerNoFacing(requester, BOT_TEXT2("cast_spell_command_summon", args), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
                        SetDuration(sPlayerbotAIConfig.globalCoolDown);
                        return true;
                    }
                    else
                    {
                        ai->TellPlayerNoFacing(requester, BOT_TEXT("cast_spell_command_summon_error_members"));
                    }
                }
                else
                {
                    ai->TellPlayerNoFacing(requester, BOT_TEXT("cast_spell_command_summon_error_target"));
                }
            }
            else
            {
                ai->TellPlayerNoFacing(requester, BOT_TEXT("cast_spell_command_summon_error_combat"));
            }
        }
    }

    return false;
}

bool CastRandomSpellAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::list<std::pair<uint32, std::string>> spellMap = GetSpellList();
    
    Unit* target = nullptr;
    GameObject* got = nullptr;

    std::string name = event.getParam();
    if (name.empty())
    {
        name = getName();
    }

    std::list<ObjectGuid> wos = chat->parseGameobjects(name);

    for (auto wo : wos)
    {
        target = ai->GetUnit(wo);
        got = ai->GetGameObject(wo);

        if (got || target)
        {
            break;
        }
    }    

    if (!got && !target && bot->GetSelectionGuid())
    {
        target = ai->GetUnit(bot->GetSelectionGuid());
        got = ai->GetGameObject(bot->GetSelectionGuid());
    }

    if (!got && !target)
    {
        if (requester && requester->GetSelectionGuid())
        {
            target = ai->GetUnit(requester->GetSelectionGuid());
        }
    }

    if (!got && !target)
    {
        target = bot;
    }

    std::vector<std::pair<uint32, std::pair<uint32, WorldObject*>>> spellList;
    for (auto & spell : spellMap)
    {
        uint32 spellId = spell.first;

        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
        if (!pSpellInfo)
        {
            continue;
        }

        if (!AcceptSpell(pSpellInfo))
        {
            continue;
        }

        if (bot->HasSpell(spellId))
        {
            uint32 spellPriority = GetSpellPriority(pSpellInfo);

            if (!spellPriority)
            {
                continue;
            }

            if (target && ai->CanCastSpell(spellId, target, 0))
            {
                spellList.push_back(std::make_pair(spellId, std::make_pair(spellPriority, target)));
            }

            if (target && ai->CanCastSpell(spellId, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0))
            {
                spellList.push_back(std::make_pair(spellId, std::make_pair(spellPriority, target)));
            }

            if (got && ai->CanCastSpell(spellId, got->GetPositionX(), got->GetPositionY(), got->GetPositionZ(), 0))
            {
                spellList.push_back(std::make_pair(spellId, std::make_pair(spellPriority, got)));
            }

            Item* item = nullptr;

            if (pSpellInfo->Targets & TARGET_FLAG_ITEM)
                item = AI_VALUE2(Item*, "item for spell", pSpellInfo->Id);

            if (ai->CanCastSpell(spellId, bot, 0, true, item))
            {
                spellList.push_back(std::make_pair(spellId, std::make_pair(spellPriority, bot)));
            }
        }
    }

    if (spellList.empty())
    {
        return false;
    }

    bool isCast = false;

    bool allTheSame = true;

    for(auto& spell : spellList)
    { 
        if (spell.first != spellList[0].first)
        {
            allTheSame = false;
            break;
        }
    }

    if (!allTheSame)
        std::sort(spellList.begin(), spellList.end(), [](std::pair<uint32, std::pair<uint32, WorldObject*>> i, std::pair<uint32, std::pair<uint32, WorldObject*>> j) {return i.second.first > j.second.first; });

    uint32 rndBound = spellList.size() - 1;

    if(!allTheSame)
        rndBound = std::min(rndBound/4, (uint32)10);

    for (uint32 i = 0; i < 5; i++)
    {
        uint32 rnd = urand(0, rndBound);

        auto spell = spellList[rnd];

        uint32 spellId = spell.first;
        WorldObject* wo = spell.second.second;

        return castSpell(spellId, wo, requester);
    }

    return false;
}

bool CastRandomSpellAction::castSpell(uint32 spellId, WorldObject* wo, Player* requester)
{
    bool executed = false;
    uint32 spellDuration = sPlayerbotAIConfig.globalCoolDown;

    SpellEntry const* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);

    Item* spellItem = AI_VALUE2(Item*, "item for spell", spellId);

    if (spellItem)
    {
        if (ai->CastSpell(spellId, bot, spellItem, false, &spellDuration))
        {
            ai->TellPlayer(requester, "Casting " + ChatHelper::formatSpell(pSpellInfo) + " on " + ChatHelper::formatItem(spellItem), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
            executed = true;
        }
    }
    
    if (!executed && wo)
    {
        if (pSpellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
        {
            if (ai->CastSpell(spellId, wo->GetPositionX(), wo->GetPositionY(), wo->GetPositionZ(), nullptr, false, &spellDuration))
            {
                ai->TellPlayer(requester, "Casting " + ChatHelper::formatSpell(pSpellInfo) + " near " + ChatHelper::formatWorldobject(wo), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
                executed = true;
            }
        }
        else if (wo->GetObjectGuid().IsUnit())
        {
            if (ai->CastSpell(spellId, (Unit*)wo, nullptr, false, &spellDuration))
            {
                if(wo != bot)
                    ai->TellPlayer(requester, "Casting " + ChatHelper::formatSpell(pSpellInfo) + " on " + ChatHelper::formatWorldobject(wo), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
                else
                    ai->TellPlayer(requester, "Casting " + ChatHelper::formatSpell(pSpellInfo), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
                executed = true;
            }
        }
        else if (wo->GetObjectGuid().IsGameObject())
        {
            if (ai->CastSpell(spellId, (GameObject*)wo, nullptr, false, &spellDuration))
            {
                ai->TellPlayer(requester, "Casting " + ChatHelper::formatSpell(pSpellInfo) + " on " + ChatHelper::formatWorldobject(wo), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
                executed = true;
            }
        }
        else
        {
            if (ai->CastSpell(spellId, wo->GetPositionX(), wo->GetPositionY(), wo->GetPositionZ(), nullptr, false, &spellDuration))
            {
                ai->TellPlayer(requester, "Casting " + ChatHelper::formatSpell(pSpellInfo) + " near " + ChatHelper::formatWorldobject(wo), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
                executed = true;
            }
        }
    }

    if (!executed)
    {
        if (ai->CastSpell(spellId, bot, nullptr, false, &spellDuration))
        {
            ai->TellPlayer(requester, "Casting " + ChatHelper::formatSpell(pSpellInfo), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
            executed = true;
        }
    }

    if (executed)
    {
        SetDuration(spellDuration);
    }    
    return executed;
}

bool CraftRandomItemAction::Execute(Event& event)
{
    std::vector<uint32> spellIds = AI_VALUE(std::vector<uint32>, "craft spells");
    std::shuffle(spellIds.begin(), spellIds.end(),*GetRandomGenerator());

    std::list<ObjectGuid> wos = chat->parseGameobjects(event.getParam());
    WorldObject* wot = nullptr;

    for (auto wo : wos)
    {
        wot = ai->GetGameObject(wo);

        if (wot)
            break;
    }

    if (!wot)
        wot = bot;

    for (uint32 spellId : spellIds)
    {
        if (!AI_VALUE2(bool, "can craft spell", spellId))
            continue;

        if (!AI_VALUE2(bool, "should craft spell", spellId))
            continue;

        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);

        if (pSpellInfo->RequiresSpellFocus)
        {
            if (!GuidPosition(wot).IsGameObject())
                continue;

            if (GuidPosition(wot).GetGameObjectInfo()->type != GAMEOBJECT_TYPE_SPELL_FOCUS)
                continue;

            if (GuidPosition(wot).GetGameObjectInfo()->spellFocus.focusId != pSpellInfo->RequiresSpellFocus)
                continue;
        }
        else if(wot != bot)
        {
            wot = nullptr;
        }

        uint32 castCount = AI_VALUE2(uint32, "has reagents for", spellId);

        if (spellId == 61288) //Crafting random glyph
        {
            castCount = 1;
        }
        else
        {
            uint32 newItemId = pSpellInfo->EffectItemType[0];

            if (!newItemId)
                continue;

            ItemPrototype const* proto = ObjectMgr::GetItemPrototype(newItemId);

            if (!proto)
                continue;

            if (castCount > proto->GetMaxStackSize())
                castCount = proto->GetMaxStackSize();
        }

        std::ostringstream cmd;
        cmd << "castnc ";

        if (((wot && sServerFacade.IsInFront(bot, wot, sPlayerbotAIConfig.sightDistance, CAST_ANGLE_IN_FRONT))))
        {
            cmd << chat->formatWorldobject(wot) << " ";
        }

        cmd << spellId << " " << castCount;

        ai->HandleCommand(CHAT_MSG_WHISPER, cmd.str(), *bot);
        SetDuration(1.0f); //Spel was not cast yet so no delay is needed.
        return true;

    }

    return false;
}

bool DisenchantRandomItemAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::list<uint32> items = AI_VALUE2(std::list<uint32>, "inventory item ids", "usage " + std::to_string((uint8)ItemUsage::ITEM_USAGE_DISENCHANT));

    items.reverse();

    if (bot->IsMoving())
    {
        ai->StopMoving();
    }

    if (bot->IsMounted())
    {
        return false;
    }

    for (auto& item: items)
    {
        ItemPrototype const* proto = ObjectMgr::GetItemPrototype(item);

        if (!proto)
            continue;

        if (!proto->DisenchantID)
            continue;

#ifndef MANGOSBOT_ZERO
        // 2.0.x addon: Check player enchanting level against the item disenchanting requirements
        int32 item_disenchantskilllevel = proto->RequiredDisenchantSkill;
        if (item_disenchantskilllevel > int32(bot->GetSkillValue(SKILL_ENCHANTING)))
            continue;
#endif

        // don't touch rare+ items if with real player/guild
        if ((ai->HasRealPlayerMaster() || ai->IsInRealGuild()) && proto->Quality > ITEM_QUALITY_UNCOMMON)
        {
            continue;
        }

        ItemQualifier itemQualifier(item);

        Event disenchantEvent = Event("disenchant random item", "13262 " + chat->formatQItem(item));
        bool didCast = CastCustomSpellAction::Execute(disenchantEvent);

        if(didCast)
        {
            ai->TellPlayer(requester, "Disenchanting " + chat->formatItem(itemQualifier), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        }

        return didCast;
    }

    return false;
};


