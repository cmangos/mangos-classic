
#include "playerbot/playerbot.h"
#include "SpellIdValue.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#ifdef MANGOSBOT_TWO
#include "Entities/Vehicle.h"
#endif

using namespace ai;

SpellIdValue::SpellIdValue(PlayerbotAI* ai) : CalculatedValue<uint32>(ai, "spell id", 10), Qualified()
{
}

VehicleSpellIdValue::VehicleSpellIdValue(PlayerbotAI* ai) : CalculatedValue<uint32>(ai, "vehicle spell id"), Qualified()
{
}

uint32 SpellIdValue::Calculate()
{
    std::string namepart = qualifier;
    ItemIds itemIds = ChatHelper::parseItems(namepart);

    PlayerbotChatHandler handler(bot);
    uint32 extractedSpellId = handler.extractSpellId(namepart);
    if (extractedSpellId)
    {
        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(extractedSpellId);
        if (pSpellInfo) namepart = pSpellInfo->SpellName[0];
    }

    std::vector<uint32> ids = chat->SpellIds(namepart);

    char firstSymbol = 'x';
    int spellLength = 0;
    std::wstring wnamepart;

    if (ids.empty())
    {
        //sLog.outError("Please add spell %s to spell list", namepart.c_str());
        if (!Utf8toWStr(namepart, wnamepart))
            return 0;

        wstrToLower(wnamepart);
        firstSymbol = tolower(namepart[0]);
        spellLength = wnamepart.length();
    }

    int loc = bot->GetSession()->GetSessionDbcLocale();

    std::set<uint32> spellIds;
    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        uint32 spellId = itr->first;

        if (!ids.empty())
        {
            if (std::find(ids.begin(), ids.end(), spellId) == ids.end())
                continue;
        }

        if (itr->second.state == PLAYERSPELL_REMOVED || itr->second.disabled || IsPassiveSpell(spellId))
            continue;

        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
        if (!pSpellInfo)
            continue;

        if (pSpellInfo->Effect[0] == SPELL_EFFECT_LEARN_SPELL)
            continue;

        bool useByItem = false;
        for (int i = 0; i < 3; ++i)
        {
            if (pSpellInfo->Effect[i] == SPELL_EFFECT_CREATE_ITEM && itemIds.find(pSpellInfo->EffectItemType[i]) != itemIds.end())
            {
                useByItem = true;
                break;
            }
        }

        if (ids.empty())
        {
            char* spellName = pSpellInfo->SpellName[loc];
            if (!useByItem && (tolower(spellName[0]) != firstSymbol || strlen(spellName) != spellLength || !Utf8FitTo(spellName, wnamepart)))
                continue;
        }

        spellIds.insert(spellId);
    }

    Pet* pet = bot->GetPet();
    if (spellIds.empty() && pet)
    {
        for (PetSpellMap::const_iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end(); ++itr)
        {
            uint32 spellId = itr->first;
            if (!ids.empty())
            {
                if (std::find(ids.begin(), ids.end(), spellId) == ids.end())
                    continue;
            }

            if(itr->second.state == PETSPELL_REMOVED)
                continue;

            const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
            if (!pSpellInfo)
                continue;

            if (pSpellInfo->Effect[0] == SPELL_EFFECT_LEARN_SPELL)
                continue;

            if (ids.empty())
            {
                char* spellName = pSpellInfo->SpellName[loc];
                if (tolower(spellName[0]) != firstSymbol || strlen(spellName) != spellLength || !Utf8FitTo(spellName, wnamepart))
                    continue;
            }

            spellIds.insert(spellId);
        }
    }

    if (spellIds.empty()) return 0;

    int saveMana = (int) round(AI_VALUE(double, "mana save level"));
    int rank = 1;
    int highestRank = 0;
    int highestSpellId = 0;
    int lowestRank = 0;
    int lowestSpellId = 0;

    if (saveMana <= 1)
    {
        for (std::set<uint32>::reverse_iterator itr = spellIds.rbegin(); itr != spellIds.rend(); ++itr)
        {
            const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(*itr);
            if (!pSpellInfo)
                continue;

            std::string spellName = pSpellInfo->Rank[0];

            // For atoi, the input std::string has to start with a digit, so lets search for the first digit
            size_t i = 0;
            for (; i < spellName.length(); i++) { if (isdigit(spellName[i])) break; }

            // remove the first chars, which aren't digits
            spellName = spellName.substr(i, spellName.length() - i);

            // convert the remaining text to an integer
            int id = atoi(spellName.c_str());

            if (!id)
            {
                highestSpellId = *itr;
                continue;
            }

            if (!highestRank || id > highestRank)
            {
                highestRank = id;
                highestSpellId = *itr;
            }

            if (!lowestRank || (lowestRank && id < lowestRank))
            {
                lowestRank = id;
                lowestSpellId = *itr;
            }
        }
    }
    else
    {
        for (std::set<uint32>::reverse_iterator i = spellIds.rbegin(); i != spellIds.rend(); ++i)
        {
            if (!highestSpellId) highestSpellId = *i;
            if (sSpellMgr.IsSpellHigherRankOfSpell(*i, highestSpellId)) highestSpellId = *i;
            if (saveMana == rank) return *i;
            lowestSpellId = *i;
            rank++;
        }
    }

    return saveMana > 1 ? lowestSpellId : highestSpellId;
}

uint32 VehicleSpellIdValue::Calculate()
{
#ifdef MANGOSBOT_TWO
    TransportInfo* transportInfo = bot->GetTransportInfo();
    if (!transportInfo || !transportInfo->IsOnVehicle())
        return 0;

    Unit* vehicle = (Unit*)transportInfo->GetTransport();
    if (!vehicle || !vehicle->IsAlive())
        return 0;

    // do not allow if no spells
    VehicleSeatEntry const* seat = vehicle->GetVehicleInfo()->GetSeatEntry(transportInfo->GetTransportSeat());
    if (!seat || !seat->HasFlag(SEAT_FLAG_CAN_CAST))
        return 0;

    std::string namepart = qualifier;

    PlayerbotChatHandler handler(bot);
    uint32 extractedSpellId = handler.extractSpellId(namepart);
    if (extractedSpellId)
    {
        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(extractedSpellId);
        if (pSpellInfo) namepart = pSpellInfo->SpellName[0];
    }

    std::wstring wnamepart;

    if (!Utf8toWStr(namepart, wnamepart))
        return 0;

    wstrToLower(wnamepart);
    char firstSymbol = tolower(namepart[0]);
    int spellLength = wnamepart.length();

    int loc = bot->GetSession()->GetSessionDbcLocale();

    //Creature* creature = static_cast<Creature*>(vehicle);
    std::vector<uint32> spells = vehicle->GetCharmSpells();
    for (uint32 x = 0; x < CREATURE_MAX_SPELLS; ++x)
    {
        uint32 spellId = spells[x];

        if (spellId == 2)
            continue;
        if (IsPassiveSpell(spellId))
            continue;
        else
        {
            const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
            if (!pSpellInfo)
                continue;

            char* spellName = pSpellInfo->SpellName[loc];
            if (tolower(spellName[0]) != firstSymbol || strlen(spellName) != spellLength || !Utf8FitTo(spellName, wnamepart))
                continue;

            return spellId;
        }
    }
#endif

    return 0;
}
