
#include "playerbot/playerbot.h"
#include "ListSpellsAction.h"
#include "playerbot/strategy/ItemVisitors.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

std::map<uint32, SkillLineAbilityEntry const*> ListSpellsAction::skillSpells;
std::set<uint32> ListSpellsAction::vendorItems;

bool CompareSpells(std::pair<uint32, std::string>& s1, std::pair<uint32, std::string>& s2)
{
    const SpellEntry* const si1 = sServerFacade.LookupSpellInfo(s1.first);
    const SpellEntry* const si2 = sServerFacade.LookupSpellInfo(s2.first);
#ifdef MANGOSBOT_ZERO
    int p1 = si1->School * 20000;
    int p2 = si2->School * 20000;
#else
    int p1 = si1->SchoolMask * 20000;
    int p2 = si2->SchoolMask * 20000;
#endif

    uint32 skill1 = 0, skill2 = 0;
    uint32 skillValue1 = 0, skillValue2 = 0;
    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j);
        if (skillLine && skillLine->spellId == s1.first)
        {
            skill1 = skillLine->skillId;
            skillValue1 = skillLine->min_value;
        }
        if (skillLine && skillLine->spellId == s2.first)
        {
            skill2 = skillLine->skillId;
            skillValue2 = skillLine->min_value;
        }
        if (skill1 && skill2) break;
    }

    p1 += skill1 * 500;
    p2 += skill2 * 500;

    p1 += skillValue1;
    p2 += skillValue2;

    if (p1 == p2)
    {
        return strcmp(si1->SpellName[0], si1->SpellName[1]) > 0;
    }

    return p1 > p2;
}

std::list<std::pair<uint32, std::string> > ListSpellsAction::GetSpellList(std::string filter)
{    
    if (skillSpells.empty())
    {
        for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
        {
            SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j);
            if (skillLine)
                skillSpells[skillLine->spellId] = skillLine;
        }
    }

    if (vendorItems.empty())
    {
        auto results = WorldDatabase.PQuery("SELECT item FROM npc_vendor where maxcount = 0");
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                vendorItems.insert(fields[0].GetUInt32());
            } while (results->NextRow());
        }
    }

    std::ostringstream posOut;
    std::ostringstream negOut;

    uint32 skill = 0;

    std::vector<std::string> ss = split(filter, ' ');
    if (!ss.empty())
    {
        skill = chat->parseSkillName(ss[0]);
        if (skill != SKILL_NONE)
        {
            filter = ss.size() > 1 ? filter = ss[1] : "";
        }

        if (ss[0] == "first" && ss[1] == "aid")
        {
            skill = SKILL_FIRST_AID;
            filter = ss.size() > 2 ? filter = ss[2] : "";
        }
    }


    const std::string ignoreList = ",Opening,Closing,Stuck,Remove Insignia,Opening - No Text,Grovel,Duel,Honorless Target,";
    std::string alreadySeenList = ",";

    int minLevel = 0, maxLevel = 0;
    if (filter.find("-") != std::string::npos)
    {
        std::vector<std::string> ff = split(filter, '-');
        minLevel = atoi(ff[0].c_str());
        maxLevel = atoi(ff[1].c_str());
        filter = "";
    }

    bool craftableOnly = false;
    if (filter.find("+") != std::string::npos)
    {
        craftableOnly = true;
        filter.erase(remove(filter.begin(), filter.end(), '+'), filter.end());
    }

    uint32 slot = chat->parseSlot(filter);
    if (slot != EQUIPMENT_SLOT_END)
        filter = "";

    std::list<std::pair<uint32, std::string> > spells;
    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr) {
        const uint32 spellId = itr->first;

        if (itr->second.state == PLAYERSPELL_REMOVED || itr->second.disabled || IsPassiveSpell(spellId))
            continue;

        const SpellEntry* const pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
        if (!pSpellInfo)
            continue;

        SkillLineAbilityEntry const* skillLine = skillSpells[spellId];
        if (skill != SKILL_NONE && (!skillLine || skillLine->skillId != skill))
            continue;

        std::string comp = pSpellInfo->SpellName[0];
        if (!(ignoreList.find(comp) == std::string::npos && alreadySeenList.find(comp) == std::string::npos))
            continue;

        if (!filter.empty() && !strstri(pSpellInfo->SpellName[0], filter.c_str()))
            continue;

        bool first = true;
        int craftCount = -1;
        std::ostringstream materials;
        for (uint32 x = 0; x < MAX_SPELL_REAGENTS; ++x)
        {
            if (pSpellInfo->Reagent[x] <= 0)
            {
                continue;
            }

            uint32 itemid = pSpellInfo->Reagent[x];
            uint32 reagentsRequired = pSpellInfo->ReagentCount[x];
            if (itemid)
            {
                ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemid);
                if (proto)
                {
                    if (first) { materials << ": "; first = false; }
                    else materials << ", ";
                    materials << chat->formatItem(proto, reagentsRequired);

                    FindItemByIdVisitor visitor(itemid);
                    uint32 reagentsInInventory = ai->InventoryGetItemCount(&visitor, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
                    bool buyable = (vendorItems.find(itemid) != vendorItems.end());
                    if (!buyable)
                    {
                        int32 craftable = reagentsInInventory / reagentsRequired;
                        if (craftCount < 0 || craftCount > craftable)
                            craftCount = craftable;
                    }

                    if (reagentsInInventory)
                        materials << "|cffffff00(x" << reagentsInInventory << ")|r ";
                    else if (buyable)
                        materials << "|cffffff00(buy)|r ";
                }
            }
        }

        if (craftCount < 0) craftCount = 0;

        std::ostringstream out;
        bool filtered = false;
        if (skillLine)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (pSpellInfo->Effect[i] == SPELL_EFFECT_CREATE_ITEM)
                {
                    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(pSpellInfo->EffectItemType[i]);
                    if (proto)
                    {
                        if (craftCount)
                            out << "|cffffff00(x" << craftCount << ")|r ";
                        out << chat->formatItem(proto);

                        if ((minLevel || maxLevel) && (!proto->RequiredLevel || (int)proto->RequiredLevel < minLevel || (int)proto->RequiredLevel > maxLevel))
                        {
                            filtered = true;
                            break;
                        }

                        if (slot != EQUIPMENT_SLOT_END && bot->FindEquipSlot(proto, slot, true) != slot)
                        {
                            filtered = true;
                            break;
                        }
                    }
                }
            }
        }

        if (out.str().empty())
            out << chat->formatSpell(pSpellInfo);

        if (filtered)
            continue;

        if (craftableOnly && !craftCount)
            continue;

        out << materials.str();

        if (skillLine && skillLine->skillId)
        {
            int GrayLevel = (int)skillLine->max_value,
                GreenLevel = (int)(skillLine->max_value + skillLine->min_value) / 2,
                YellowLevel = (int)skillLine->min_value,
                SkillValue = (int)bot->GetSkillValue(skillLine->skillId);

            out << " - ";
            if (SkillValue >= GrayLevel)
                out << " |cff808080gray";
            else if (SkillValue >= GreenLevel)
                out << " |cff80be80green";
            else if (SkillValue >= YellowLevel)
                out << " |cffffff00yellow";
            else
                out << " |cffff8040orange";
            out << "|r";
        }

        if (out.str().empty())
            continue;

        spells.push_back(std::pair<uint32, std::string>(spellId, out.str()));
        alreadySeenList += pSpellInfo->SpellName[0];
        alreadySeenList += ",";
    }

    return spells;
}

bool ListSpellsAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester)
        return false;

    std::string filter = event.getParam();

    std::list<std::pair<uint32, std::string> > spells = GetSpellList(filter);

    ai->TellPlayer(requester, "=== Spells ===");
    spells.sort(CompareSpells);

    int count = 0;
    for (std::list<std::pair<uint32, std::string> >::iterator i = spells.begin(); i != spells.end(); ++i)
    {
        ai->TellPlayerNoFacing(requester, i->second);
        if (++count >= 50)
        {
            std::ostringstream msg; msg << (spells.size() - 50) << " more...";
            ai->TellPlayerNoFacing(requester, msg.str());
            break;
        }
    }

    return true;
}

