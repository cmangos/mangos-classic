#include "playerbot/playerbot.h"
#include "Talentspec.h"
#include "playerbot/ServerFacade.h"
#include "Server/DBCStructure.h"
#include "Guilds/GuildMgr.h"

using namespace std::placeholders;

//Checks a talent link on basic validity.
bool TalentSpec::CheckTalentLink(std::string link, std::ostringstream* out) {

    std::string validChar = "-";
    std::string validNums = "012345";
    int nums = 0;

    for (char& c : link) {
        if (validChar.find(c) == std::string::npos && validNums.find(c) == std::string::npos)
        {
            *out << "talent link is invalid. Must be in format 0-0-0 (see end of wowhead talent calculator url) or a part of a predefined spec as shown with command 'talents list'";
            return false;
        }
        if (validNums.find(c) != std::string::npos)
            nums++;
    }

    if (nums == 0) {
        *out << "talents are invalid. Needs atleast one number.";
        return false;
    }

    return true;
}

uint32 TalentSpec::LeveltoPoints(uint32 level)
{
    uint32 talentPointsForLevel = level < 10 ? 0 : level - 9;
    return uint32(talentPointsForLevel * sWorld.getConfig(CONFIG_FLOAT_RATE_TALENT));
}

uint32 TalentSpec::PointstoLevel(int points) const
{
    return uint32(ceil(points / sWorld.getConfig(CONFIG_FLOAT_RATE_TALENT)))+9;
}

//Check the talentspec for errors.
bool TalentSpec::CheckTalents(uint32 freeTalentPoints, std::ostringstream* out)
{
    for (auto& entry : talents)
    {
        if (entry.rank > entry.maxRank)
        {
            SpellEntry const* spellInfo = sServerFacade.LookupSpellInfo(entry.talentInfo->RankID[0]);
            *out << "spec is not for this class. " << spellInfo->SpellName[0] << " has " << (entry.rank - entry.maxRank) << " points above max rank.";
            return false;
        }

        if (entry.rank > 0 && entry.talentInfo->DependsOn)
        {
            TalentEntry const* talentInfo = sTalentStore.LookupEntry(entry.talentInfo->DependsOn);
            if (!talentInfo)
                continue;

            bool found = false;
            SpellEntry const* spellInfodep;

            for (auto& dep : talents)
                if (dep.talentInfo->TalentID == entry.talentInfo->DependsOn)
                {
                    spellInfodep = sServerFacade.LookupSpellInfo(dep.talentInfo->RankID[0]);
                    if (dep.rank >= (int)entry.talentInfo->DependsOnRank)
                        found = true;
                }
            if (!found)
            {
                SpellEntry const* spellInfo = sServerFacade.LookupSpellInfo(entry.talentInfo->RankID[0]);
                *out << "spec is is invalid. Talent:" << spellInfo->SpellName[0] << " needs: " << spellInfodep->SpellName[0] << " at rank: " << entry.talentInfo->DependsOnRank;
                return false;
            }
        }
    }

    for (int i = 0; i < 3; i++)
    {
        std::vector<TalentListEntry> talentTree = GetTalentTree(i);
        int points = 0;

        for (auto& entry : talentTree)
        {
            if (entry.rank > 0 && (int)(entry.talentInfo->Row * 5) > points)
            {
                SpellEntry const* spellInfo = sServerFacade.LookupSpellInfo(entry.talentInfo->RankID[0]);
                *out << "spec is is invalid. Talent " << spellInfo->SpellName[0] << " is selected with only " << points << " in row below it.";
                return false;
            }
            points += entry.rank;
        }
    }

    if (points > freeTalentPoints)
    {
        *out << "spec is for a higher level. (" << PointstoLevel(points) << ")";
        return false;
    }

    return true;
}

//Set the talents for the bots to the current spec.
void TalentSpec::ApplyTalents(Player* bot, std::ostringstream* out)
{
    for (auto& entry : talents)
        for (int rank = 0; rank < MAX_TALENT_RANK; ++rank)
        {
            uint32 spellId = entry.talentInfo->RankID[rank];

            if (!spellId)
                continue;

            if (spellId == 0)
                continue;

            if (bot->HasSpell(spellId) && entry.rank - 1 != rank)
            {
                bot->removeSpell(spellId, false, false);
            }
            else if (!bot->HasSpell(spellId) && entry.rank - 1 == rank)
            {
                bot->learnSpell(spellId, false);
            }
        }

    SetPublicNote(bot);
}

void TalentSpec::SetPublicNote(Player* bot)
{
    TalentSpec spec(bot);
    if (sPlayerbotAIConfig.talentsInPublicNote && bot->GetGuildId())
    {
        Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
        MemberSlot* member = guild->GetMemberSlot(bot->GetObjectGuid());
        if (guild->HasRankRight(member->RankId, GR_RIGHT_EPNOTE))
            member->SetPNOTE(ChatHelper::specName(bot) + " (" + std::to_string(spec.GetTalentPoints(0)) + "/" + std::to_string(spec.GetTalentPoints(1)) + "/" + std::to_string(spec.GetTalentPoints(2)) + ")");
    }
}

//Returns a base talentlist for a class.
void TalentSpec::GetTalents(uint32 classMask) {
    TalentListEntry entry;

    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo)
            continue;

        if ((classMask & talentTabInfo->ClassMask) == 0)
            continue;

        entry.entry = i;
        entry.rank = 0;
        entry.talentInfo = talentInfo;
        entry.talentTabInfo = talentTabInfo;

        for (int rank = 0; rank < MAX_TALENT_RANK; ++rank)
        {
            uint32 spellId = talentInfo->RankID[rank];
            if (!spellId)
                continue;

            if (spellId == 0)
                continue;

            entry.maxRank = rank + 1;
        }
        talents.push_back(entry);
    }
    SortTalents(talents, SORT_BY_DEFAULT);
}

//Sorts a talent list by page, row, column.
bool sortTalentMap(TalentSpec::TalentListEntry i, TalentSpec::TalentListEntry j, int* tabSort) {
    uint32 itab = i.tabPage();
    uint32 jtab = j.tabPage();
    if (tabSort[itab] < tabSort[jtab])
        return true;
    if (tabSort[itab] > tabSort[jtab])
        return false;
    if (i.talentInfo->Row < j.talentInfo->Row)
        return true;
    if (i.talentInfo->Row > j.talentInfo->Row)
        return false;
    if (i.talentInfo->Col < j.talentInfo->Col)
        return true;

    return false;
}

//Sort the talents.
void TalentSpec::SortTalents(std::vector<TalentListEntry>& talents, int sortBy)
{
    switch (sortBy)
    {
    case SORT_BY_DEFAULT:
    {
        int tabSort[] = { 0,1,2 };
        sort(talents.begin(), talents.end(), [&tabSort](TalentSpec::TalentListEntry i, TalentSpec::TalentListEntry j) {return sortTalentMap(i, j, tabSort); });
        break;
    }
    case SORT_BY_POINTS_TREE:
    {
        int tabSort[] = { GetTalentPoints(talents, 0) * -100 - irand(0, 99),GetTalentPoints(talents, 1) * -100 - irand(0, 99),GetTalentPoints(talents, 2) * -100 - irand(0, 99) };
        sort(talents.begin(), talents.end(), [&tabSort](TalentSpec::TalentListEntry i, TalentSpec::TalentListEntry j) {return sortTalentMap(i, j, tabSort); });
        break;
    }
    }
}

//Set the talent ranks to the current rank of the player.
void TalentSpec::ReadTalents(Player* bot) {
    for (auto& entry : talents)
        for (int rank = 0; rank < MAX_TALENT_RANK; ++rank)
        {
            uint32 spellId = entry.talentInfo->RankID[rank];

            if (!spellId)
                continue;

            if (spellId == 0)
                continue;

            if (bot->HasSpell(spellId))
            {
                entry.rank = rank + 1;
                points += 1;
            }
        }
}

//Set the talent ranks to the ranks of the link.
void TalentSpec::ReadTalents(std::string link) {
    int rank = 0;
    int pos = 0;
    int tab = 0;
    std::string chr;

    if (link.substr(pos, 1) == "-") {
        pos++;
        tab++;
    }

    if (link.substr(pos, 1) == "-") {
        pos++;
        tab++;
    }

    for (auto& entry : talents)
    {
        if (entry.tabPage() == tab)
        {
            chr = link.substr(pos, 1);

            if (chr == " " || chr == "#")
                break;

            entry.rank = stoi(chr);
            points += entry.rank;

            pos++;
            if (pos <= link.size())
                if (link.substr(pos, 1) == "-")
                {
                    pos++;
                    tab++;
                }
            if (pos <= link.size())
                if (link.substr(pos, 1) == "-")
                {
                    pos++;
                    tab++;
                }
        }
        if (pos > link.size() - 1)
            break;
    };
}

//Returns only a specific tree from a talent list.
std::vector<TalentSpec::TalentListEntry> TalentSpec::GetTalentTree(int tabpage)
{
    std::vector<TalentListEntry> retList;

    for (auto& entry : talents)
        if (entry.tabPage() == tabpage)
            retList.push_back(entry);

    return retList;
}

//Counts the point in a talent list.
int TalentSpec::GetTalentPoints(std::vector<TalentListEntry>& talents, int tabpage)
{
    if (tabpage == -1)
        return points;

    int tPoints = 0;

    for (auto& entry : talents)
        if (entry.tabPage() == tabpage)
            tPoints = tPoints + entry.rank;

    return tPoints;
}

//Generates a wow-head link from a talent list.
std::string TalentSpec::GetTalentLink()
{
    std::string link = "";
    std::string treeLink[3];
    int points[3];
    int curPoints = 0;

    for (int i = 0; i < 3; i++) {
        points[i] = GetTalentPoints(i);
        for (auto& entry : GetTalentTree(i))
        {
            curPoints += entry.rank;
            treeLink[i] += std::to_string(entry.rank);
            if (curPoints >= points[i])
            {
                curPoints = 0;
                break;
            }
        }
    }

    link = treeLink[0];
    if (treeLink[1] != "0" || treeLink[2] != "0")
        link = link + "-" + treeLink[1];
    if (treeLink[2] != "0")
        link = link + "-" + treeLink[2];

    return link;
}


int TalentSpec::highestTree()
{
    int p1 = GetTalentPoints(0);
    int p2 = GetTalentPoints(1);
    int p3 = GetTalentPoints(2);

    if (p1 > p2 && p1 > p3)
        return 0;
    if (p2 > p1 && p2 > p3)
        return 1;
    if (p3 > p1 && p3 > p2)
        return 2;

    if (p1 > p2 || p1 > p3)
        return 0;
    if (p2 > p3 || p2 > p1)
        return 1;

    return 0;
}

std::string TalentSpec::formatSpec(Player* bot)
{
    uint8 cls = bot->getClass();

    std::ostringstream out;
    //out << chathelper:: specs[cls][highestTree()] << " (";

    int c0 = GetTalentPoints(0);
    int c1 = GetTalentPoints(1);
    int c2 = GetTalentPoints(2);

    out << (c0 ? "|h|cff00ff00" : "") << c0 << "|h|cffffffff/";
    out << (c1 ? "|h|cff00ff00" : "") << c1 << "|h|cffffffff/";
    out << (c2 ? "|h|cff00ff00" : "") << c2 << "|h|cffffffff";

    return out.str();
}

//Removes talentpoints to match the level
void TalentSpec::CropTalents(Player* bot)
{
    if (points <= bot->GetFreeTalentPoints())
        return;

    SortTalents(talents, SORT_BY_POINTS_TREE);

    int points = 0;

    for (auto& entry : talents)
    {
        if (points + entry.rank > (int)bot->GetFreeTalentPoints())
            entry.rank = std::max(0, (int)(bot->GetFreeTalentPoints() - points));
        points += entry.rank;
    }

    SortTalents(talents, SORT_BY_DEFAULT);
}

//Substracts ranks. Follows the sorting of the newList.
std::vector<TalentSpec::TalentListEntry> TalentSpec::SubTalentList(std::vector<TalentListEntry>& oldList, std::vector<TalentListEntry>& newList, int reverse = SUBSTRACT_OLD_NEW) {
    std::vector<TalentSpec::TalentListEntry> deltaList = newList;
    for (auto& newentry : deltaList)
        for (auto& oldentry : oldList)
            if (oldentry.entry == newentry.entry)
            {
                if (reverse == ABSOLUTE_DIST)
                    newentry.rank = abs(newentry.rank - oldentry.rank);
                else if (reverse == ADDED_POINTS || reverse == REMOVED_POINTS)
                    newentry.rank = std::max(0, (newentry.rank - oldentry.rank) * (reverse / 2));
                else
                    newentry.rank = (newentry.rank - oldentry.rank) * reverse;
            }

    return deltaList;
}

bool TalentSpec::isEarlierVersionOf(TalentSpec& newSpec)
{
    for (auto& newentry : newSpec.talents)
        for (auto& oldentry : talents)
            if (oldentry.entry == newentry.entry)
                if (oldentry.rank > newentry.rank)
                    return false;
    return true;
}


//Modifies current talents towards new talents up to a maximum of points.
void TalentSpec::ShiftTalents(TalentSpec* currentSpec, Player* bot)
{    

    if (points >= bot->GetFreeTalentPoints()) //We have no more points to spend. Better reset and crop
    {
        CropTalents(bot);
        return;
    }

    SortTalents(SORT_BY_POINTS_TREE); //Apply points first to the largest new tree.

    std::vector<TalentSpec::TalentListEntry> deltaList = SubTalentList(currentSpec->talents, talents);

    for (auto& entry : deltaList)
    {
        if (entry.rank < 0) //We have to remove talents. Might as well reset and crop the new list.
        {
            CropTalents(bot);
            return;
        }
    }

    //Start from the current spec.
    talents = currentSpec->talents;

    for (auto& entry : deltaList)
    {
        if (entry.rank + points > bot->GetFreeTalentPoints()) //Running out of points. Only apply what we have left.
            entry.rank = std::max(0, int(bot->GetFreeTalentPoints() - points));

        for (auto& subentry : talents)
            if (entry.entry == subentry.entry)
                subentry.rank = subentry.rank + entry.rank;

        points = points + entry.rank;
    }
}