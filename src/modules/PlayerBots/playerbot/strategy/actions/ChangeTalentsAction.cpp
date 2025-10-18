
#include "playerbot/playerbot.h"
#include "playerbot/Talentspec.h"
#include "ChangeTalentsAction.h"

using namespace ai;

bool ChangeTalentsAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::ostringstream out;
    TalentSpec botSpec(bot);
    std::string param = event.getParam();

    if (!param.empty())
    {
        if (param.find("auto") != std::string::npos)
        {
            AutoSelectTalents(&out);
        }
        else  if (param.find("list ") != std::string::npos)
        {
            listPremadePaths(getPremadePaths(param.substr(5)), &out);
        }
        else  if (param.find("list") != std::string::npos)
        {
            listPremadePaths(getPremadePaths(""), &out);
        }
        else if (param.find("reset") != std::string::npos)
        {
            out << "Reset talents and spec";
            TalentSpec newSpec(bot, "0-0-0");
            newSpec.ApplyTalents(bot, &out);
            sRandomPlayerbotMgr.SetValue(bot->GetGUIDLow(), "specNo", 0);
            sRandomPlayerbotMgr.SetValue(bot->GetGUIDLow(), "specLink", 0);
        }
        else
        {
            bool crop = false;
            bool shift = false;
            if (param.find("do ") != std::string::npos)
            {
                crop = true;
                param = param.substr(3);
            }
            else if (param.find("shift ") != std::string::npos)
            {
                shift = true;
                param = param.substr(6);
            }

            out << "Apply talents [" << param << "] ";
            if (botSpec.CheckTalentLink(param, &out))
            {
                TalentSpec newSpec(bot, param);
                std::string specLink = newSpec.GetTalentLink();

                if (crop)
                {
                    newSpec.CropTalents(bot);
                    out << "becomes: " << newSpec.GetTalentLink();
                }

                if (shift)
                {
                    TalentSpec botSpec(bot);
                    newSpec.ShiftTalents(&botSpec, bot);
                    out << "becomes: " << newSpec.GetTalentLink();
                }

                if (newSpec.CheckTalents(bot, &out))
                {
                    newSpec.ApplyTalents(bot, &out);
                    sRandomPlayerbotMgr.SetValue(bot->GetGUIDLow(), "specNo", 0);
                    sRandomPlayerbotMgr.SetValue(bot->GetGUIDLow(), "specLink", 1, specLink);
                }

                ai->UpdateTalentSpec();
            }
            else
            {
                std::vector<TalentPath*> paths = getPremadePaths(param);
                if (paths.size() > 0)
                {
                    out.str("");
                    out.clear();

                    if (paths.size() > 1 && sPlayerbotAIConfig.autoPickTalents != "full")
                    {
                        out << "Found multiple specs: ";
                        listPremadePaths(paths, &out);
                    }
                    else
                    {
                        if (paths.size() > 1)
                            out << "Found " << paths.size() << " possible specs to choose from. ";
                        
                        TalentPath* path = PickPremadePath(paths, sRandomPlayerbotMgr.IsRandomBot(bot));
                        TalentSpec newSpec = *GetBestPremadeSpec(path->id);
                        std::string specLink = newSpec.GetTalentLink();
                        newSpec.CropTalents(bot);
                        newSpec.ApplyTalents(bot, &out);

                        if (newSpec.GetTalentPoints() > 0)
                        {
                            out << "Apply spec " << "|h|cffffffff" << path->name << " " << newSpec.formatSpec(bot);
                            sRandomPlayerbotMgr.SetValue(bot->GetGUIDLow(), "specNo", path->id + 1);
                            sRandomPlayerbotMgr.SetValue(bot->GetGUIDLow(), "specLink", 0);

                            ai->UpdateTalentSpec();
                        }
                    }
                }
            }
        }

        // learn available spells
        ai->DoSpecificAction("auto learn spell");
    }
    else
    {
        botSpec.ApplyTalents(bot, &out);
        out.str("");
        out.clear();

        uint32 specId = sRandomPlayerbotMgr.GetValue(bot->GetGUIDLow(), "specNo") - 1;
        std::string specName = "";
        TalentPath* specPath;
        if (specId)
        {
            specPath = getPremadePath(specId);

            if (!specPath)
            {
                ai->TellPlayer(requester, "Default talent spec for this class was not fount. Please check your config",PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
                return false;
            }


            if (specPath->id == specId)
                specName = specPath->name;
        }

        out << "My current talent spec is: " << "|h|cffffffff";

        if (specName != "")
            out << specName << " (" << botSpec.formatSpec(bot) << ")";
        else
            out << chat->formatClass(bot, botSpec.highestTree());

        out << " Link: ";
        out << botSpec.GetTalentLink();
    }

    ai->TellPlayer(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

    return true;
}

std::vector<TalentPath*> ChangeTalentsAction::getPremadePaths(std::string findName)
{
    std::vector<TalentPath*> ret;
    for (auto& path : sPlayerbotAIConfig.classSpecs[bot->getClass()].talentPath)
    {
        if (findName.empty() || path.name.find(findName) != std::string::npos)
        {
            ret.push_back(&path);
        }
    }

    return ret;
}

std::vector<TalentPath*> ChangeTalentsAction::getPremadePaths(TalentSpec *oldSpec)
{
    std::vector<TalentPath*> ret;
    
    for (auto& path : sPlayerbotAIConfig.classSpecs[bot->getClass()].talentPath)
    {
        TalentSpec newSpec = *GetBestPremadeSpec(path.id);
        newSpec.CropTalents(bot);        
        if (oldSpec->isEarlierVersionOf(newSpec))
        {
            ret.push_back(&path);
        }
    }

    return ret;
}

TalentPath* ChangeTalentsAction::getPremadePath(int id)
{
    for (auto& path : sPlayerbotAIConfig.classSpecs[bot->getClass()].talentPath)
    {
        if (id == path.id)
        {
            return &path;
        }
    }

    if (sPlayerbotAIConfig.classSpecs[bot->getClass()].talentPath.empty())
        return nullptr;

    return &sPlayerbotAIConfig.classSpecs[bot->getClass()].talentPath[0];
}

void ChangeTalentsAction::listPremadePaths(std::vector<TalentPath*> paths, std::ostringstream* out)
{
    if (paths.size() == 0)
    {
        *out << "No predefined talents found..";
    }

    *out << "|h|cffffffff";

    for (auto path : paths)
    {
        *out << path->name << " (" << path->talentSpec.back().formatSpec(bot) << "), ";
    }

    out->seekp(-2, out->cur);
    *out << ".";
}

TalentPath* ChangeTalentsAction::PickPremadePath(std::vector<TalentPath*> paths, bool useProbability)
{
    int totProbability = 0;
    int curProbability = 0;

    if(paths.size() == 1)
        return paths[0];

    for (auto path : paths)
    {
        totProbability += useProbability ? path->probability : 1;
    }

    totProbability = irand(0, totProbability);

    for (auto path : paths)
    {
        curProbability += (useProbability ? path->probability : 1);
        if (curProbability >= totProbability)
            return path;
    }

    return paths[0];
}

bool ChangeTalentsAction::AutoSelectTalents(std::ostringstream* out)
{
    //Does the bot have talentpoints?
    if (bot->GetLevel() < 10)
    {
        *out << "No free talent points.";
        return false;
    }

    uint32 specNo = sRandomPlayerbotMgr.GetValue(bot->GetGUIDLow(), "specNo");
    uint32 specId = specNo ? specNo - 1 : 0;
    std::string specLink = sRandomPlayerbotMgr.GetData(bot->GetGUIDLow(), "specLink");

    //Continue the current spec
    if (specNo > 0)
    {
        TalentSpec newSpec = *GetBestPremadeSpec(specId);
        newSpec.CropTalents(bot);
        newSpec.ApplyTalents(bot, out);
        ai->UpdateTalentSpec();
        if (newSpec.GetTalentPoints() > 0)
        {
            *out << "Upgrading spec " << "|h|cffffffff" << getPremadePath(specId)->name << " (" << newSpec.formatSpec(bot) << ")";
        }
    }
    else if (!specLink.empty())
    {
        TalentSpec newSpec(bot, specLink);
        newSpec.CropTalents(bot);
        newSpec.ApplyTalents(bot, out);
        ai->UpdateTalentSpec();
        if (newSpec.GetTalentPoints() > 0)
        {
            *out << "Upgrading saved spec " << "|h|cffffffff" << chat->formatClass(bot, newSpec.highestTree()) << " (" << newSpec.formatSpec(bot) << ")";
        }
    }

    //Spec was not found or not sufficient
    if (bot->GetFreeTalentPoints() > 0 || (!specNo && specLink.empty()))
    {
        TalentSpec oldSpec(bot);
        int currentTree = oldSpec.highestTree();
        std::vector<TalentPath*> paths = getPremadePaths(&oldSpec);

        if (paths.size() == 0) //No spec like the old one found. Pick any.
        {
            if (bot->CalculateTalentsPoints() > 0)
                *out << "No specs like the current spec found.";

            paths = getPremadePaths("");
        }   

        if(paths.size() > 0 && oldSpec.GetTalentPoints() > 0)
        {
            //Check if any spec has the same tree as the current spec.
            bool hasSameTree = false;
            for (auto it : paths)
            {
                if (it->talentSpec.back().highestTree() == currentTree)
                {
                    hasSameTree = true;
                    break;
                }
            }

            if (hasSameTree) //Remove specs that do not end up in the same tree.
            {
                auto it = paths.begin();
                while (it != paths.end()) 
                {
                    TalentPath* path = *it;
                    if (path->talentSpec.back().highestTree() != currentTree) 
                    {
                        it = paths.erase(it);
                    }
                    else 
                    {
                        ++it;
                    }
                }
            }
        }

        if (paths.size() == 0)
        {
            *out << "No predefined talents found for this class.";
            specId = -1;
        }
        else if (paths.size() > 1 && sPlayerbotAIConfig.autoPickTalents != "full" && !sRandomPlayerbotMgr.IsRandomBot(bot))
        {
            *out << "Found multiple specs: ";
            listPremadePaths(paths, out);
        }
        else
        {
            specId = PickPremadePath(paths, sRandomPlayerbotMgr.IsRandomBot(bot))->id;
            TalentSpec newSpec = *GetBestPremadeSpec(specId);
            specLink = newSpec.GetTalentLink();
            newSpec.CropTalents(bot);
            newSpec.ApplyTalents(bot, out);
            ai->UpdateTalentSpec();

            if (paths.size() > 1)
                *out << "Found " << paths.size() << " possible specs to choose from. ";

            *out << "Apply spec " << "|h|cffffffff" << getPremadePath(specId)->name << " " << newSpec.formatSpec(bot);
        }
    }

    sRandomPlayerbotMgr.SetValue(bot->GetGUIDLow(), "specNo", specId + 1);
    if (!specLink.empty() && specId == -1)
        sRandomPlayerbotMgr.SetValue(bot->GetGUIDLow(), "specLink", 1, specLink);
    else
        sRandomPlayerbotMgr.SetValue(bot->GetGUIDLow(), "specLink", 0);

    return (specNo == 0) ? false : true;
}

//Returns a pre-made talent spec that best suits the bots current talents. 
TalentSpec* ChangeTalentsAction::GetBestPremadeSpec(int specId)
{
    TalentPath* path = getPremadePath(specId);
    for (auto& spec : path->talentSpec)
    {
        if (spec.points >= bot->CalculateTalentsPoints())
            return &spec;
    }
    if (path->talentSpec.size())
        return &path->talentSpec.back();

    return &sPlayerbotAIConfig.classSpecs[bot->getClassMask()].baseSpec;
}

bool AutoSetTalentsAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    sPlayerbotAIConfig.logEvent(ai, "AutoSetTalentsAction", std::to_string(bot->m_Played_time[PLAYED_TIME_LEVEL]), std::to_string(bot->m_Played_time[PLAYED_TIME_TOTAL]));

    std::ostringstream out;

    if (sPlayerbotAIConfig.autoPickTalents == "no" && !sRandomPlayerbotMgr.IsRandomBot(bot))
    {
        return false;
    }

    if (bot->GetFreeTalentPoints() <= 0)
    {
        return false;
    }

    AutoSelectTalents(&out);

    ai->TellPlayer(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

    return true;
}


