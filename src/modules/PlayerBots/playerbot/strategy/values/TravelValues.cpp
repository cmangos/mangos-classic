
#include "playerbot/playerbot.h"
#include "TravelValues.h"
#include "QuestValues.h"
#include "SharedValueContext.h"
#include "BudgetValues.h"

using namespace ai;

EntryGuidps EntryGuidpsValue::Calculate()
{
    EntryGuidps guidps;

    for (auto& creatureDataPair : WorldPosition().getCreaturesNear())
    {
        AsyncGuidPosition aGuidP(creatureDataPair);
        if (aGuidP.isValid() && !aGuidP.IsEventUnspawned())
        {
            aGuidP.FetchArea();
            guidps[aGuidP.GetEntry()].push_back(aGuidP);
        }
    }

    for (auto& goDataPair : WorldPosition().getGameObjectsNear())
    {
        AsyncGuidPosition aGuidP(goDataPair);
        if (aGuidP.isValid() && !aGuidP.IsEventUnspawned())
        {
            aGuidP.FetchArea();
            guidps[-((int32)aGuidP.GetEntry())].push_back(aGuidP);
        }
    }

    return guidps;
}

EntryTravelPurposeMap EntryTravelPurposeMapValue::Calculate()
{
    EntryQuestRelationMap relationMap = GAI_VALUE(EntryQuestRelationMap, "entry quest relation");
    EntryGuidps guidpMap = GAI_VALUE(EntryGuidps, "entry guidps");

    EntryTravelPurposeMap entryPurposeMap;

    std::vector<NPCFlags> allowedNpcFlags;

    allowedNpcFlags.push_back(UNIT_NPC_FLAG_INNKEEPER);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_GOSSIP);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_QUESTGIVER);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_FLIGHTMASTER);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_BANKER);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_AUCTIONEER);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_STABLEMASTER);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_PETITIONER);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_TABARDDESIGNER);

    allowedNpcFlags.push_back(UNIT_NPC_FLAG_TRAINER);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_REPAIR);

    std::unordered_map<NPCFlags, TravelDestinationPurpose> npcPurposeMap =
    {
        { UNIT_NPC_FLAG_REPAIR, TravelDestinationPurpose::Repair },
        { UNIT_NPC_FLAG_VENDOR, TravelDestinationPurpose::Vendor },
        { UNIT_NPC_FLAG_TRAINER, TravelDestinationPurpose::Trainer },
        { UNIT_NPC_FLAG_AUCTIONEER, TravelDestinationPurpose::AH }
    };

    for (uint32 entry = 0; entry < sCreatureStorage.GetMaxEntry(); ++entry)
    {
        CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(entry);

        if (!cInfo)
            continue;

        if (cInfo->ExtraFlags & CREATURE_EXTRA_FLAG_INVISIBLE)
            continue;

        DestinationPurose purpose = 0;

        if(relationMap.find(entry) != relationMap.end())
            for(auto& [questId, questFlag] : relationMap.at(entry))
                purpose |= questFlag;

        for (auto& flag : allowedNpcFlags)
        {
            if ((cInfo->NpcFlags & flag) != 0)
            {
                purpose |= (uint32)TravelDestinationPurpose::GenericRpg;
                break;
            }
        }

        for (auto& [flag, flagPurpose] : npcPurposeMap)
        {
            if ((cInfo->NpcFlags & flag) != 0)
            {
                purpose |= (uint32)flagPurpose;
            }
        }


        if (cInfo->MinLootGold > 0)
        {
            purpose |= (uint32)TravelDestinationPurpose::Grind;
        }
        else  //Added these specifically because dk start mobs drop no money but have sellable loot.
        {
            switch (entry)
            {
            case 29080: //Scarlet Champion
            case 29029: //Scarlet Inquisitor
            case 29000: //Scarlet Commander Rodrick
            case 28940: //Scarlet Crusader
            case 28939: //Scarlet Preacher
            case 28936: //Scarlet Commander
            case 28898: //Scarlet Captain
            case 28896: //Scarlet Infantryman
            case 28895: //Scarlet Medic
            case 28892: //Scarlet Peasant
            case 28610: //Scarlet Marksman
                purpose |= (uint32)TravelDestinationPurpose::Grind;
                break;
            }
        }

        if (cInfo->Rank == 3 || cInfo->Rank == 4 || cInfo->Rank == 1)
        {
            if (cInfo->Rank == 1)
            {
                if (guidpMap[entry].size() == 1)
                    if (WorldPosition(guidpMap[entry].front()).isOverworld())
                        purpose |= (uint32)TravelDestinationPurpose::Boss;
            }
            else
                purpose |= (uint32)TravelDestinationPurpose::Boss;
        }

        if (cInfo->SkinningLootId && cInfo->GetRequiredLootSkill() == SKILL_SKINNING)
        {
            purpose |= (uint32)TravelDestinationPurpose::GatherSkinning;
        }

        if (uint32 skillId = SkillIdToGatherEntry(entry))
        {
            if (skillId == SKILL_SKINNING)
                purpose |= (uint32)TravelDestinationPurpose::GatherSkinning;
            if (skillId == SKILL_MINING)
                purpose |= (uint32)TravelDestinationPurpose::GatherMining;
            if (skillId == SKILL_HERBALISM)
                purpose |= (uint32)TravelDestinationPurpose::GatherHerbalism;
        }

        if (purpose > 0)
            entryPurposeMap[entry] = purpose;
    }

    for (uint32 entry = 0; entry < sGOStorage.GetMaxEntry(); ++entry)
    {
        GameObjectInfo const* gInfo = ObjectMgr::GetGameObjectInfo(entry);

        if (!gInfo)
            continue;

        if (gInfo->ExtraFlags & CREATURE_EXTRA_FLAG_INVISIBLE)
            continue;

        uint32 purpose = 0;

        DestinationEntry goEntry = entry * -1;

        if (relationMap.find(goEntry) != relationMap.end())
            for (auto& [questId, questFlag] : relationMap.at(goEntry))
                purpose |= questFlag;

        std::vector<GameobjectTypes> allowedGoTypes;

        allowedGoTypes.push_back(GAMEOBJECT_TYPE_MAILBOX);

        for (auto type : allowedGoTypes)
        {
            if (gInfo->type == type)
            {
                purpose |= (uint32)TravelDestinationPurpose::Mail;
                break;
            }
        }

        if (uint32 skillId = SkillIdToGatherEntry(goEntry))
        {
            if (skillId == SKILL_SKINNING)
                purpose |= (uint32)TravelDestinationPurpose::GatherSkinning;
            if (skillId == SKILL_MINING)
                purpose |= (uint32)TravelDestinationPurpose::GatherMining;
            if (skillId == SKILL_HERBALISM)
                purpose |= (uint32)TravelDestinationPurpose::GatherHerbalism;
        }

        if (purpose > 0)
            entryPurposeMap[goEntry] = purpose;
    }

    return entryPurposeMap;
}

uint32 EntryTravelPurposeMapValue::SkillIdToGatherEntry(int32 entry)
{
    if (entry > 0)
    {
        CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(entry);

        if (!cInfo->SkinningLootId)
            return 0;

        return cInfo->GetRequiredLootSkill();
    }
    else
    {
        GameObjectInfo const* gInfo = ObjectMgr::GetGameObjectInfo(entry * -1);

        if (uint32 lockId = gInfo->GetLockId())
        {
            LockEntry const* lockInfo = sLockStore.LookupEntry(lockId);
            if (lockInfo)
            {
                uint32 skillId = SKILL_NONE;

                for (int i = 0; i < 8; ++i)
                {
                    if (lockInfo->Type[i] != LOCK_KEY_SKILL)
                        continue;

                    if (SkillByLockType(LockType(lockInfo->Index[i])) == 0)
                        continue;

                    return SkillByLockType(LockType(lockInfo->Index[i]));
                }
            }
        }
    }

    return 0;
}

bool NeedTravelPurposeValue::Calculate()
{
    TravelDestinationPurpose purpose = TravelDestinationPurpose(stoi(getQualifier()));

    const std::map<TravelDestinationPurpose, SkillType> gatheringSkills =
    { {TravelDestinationPurpose::GatherFishing, SKILL_FISHING}
        , {TravelDestinationPurpose::GatherSkinning, SKILL_SKINNING}
        , {TravelDestinationPurpose::GatherMining, SKILL_MINING}
        , {TravelDestinationPurpose::GatherHerbalism, SKILL_HERBALISM}
    };

    SkillType skill;

    switch (purpose)
    {
    case TravelDestinationPurpose::Repair:
        if (AI_VALUE2(bool, "group or", "should repair,can repair,following party"))
            return true;
        if (AI_VALUE2(bool, "has strategy", "free") && AI_VALUE(bool, "should repair") && AI_VALUE(bool, "can repair"))
            return true;
        break;
    case TravelDestinationPurpose::Vendor:
        if (AI_VALUE2(bool, "group or", "should sell,can sell,following party"))
            return true;
        if (AI_VALUE2(bool, "has strategy", "free") && AI_VALUE(bool, "should sell") && AI_VALUE(bool, "can sell"))
            return true;
        break;
    case TravelDestinationPurpose::AH:
        if (AI_VALUE2(bool, "group or", "should ah sell,can ah sell,following party"))
            return true;
        if (AI_VALUE2(bool, "has strategy", "free") && AI_VALUE(bool, "should ah sell") && AI_VALUE(bool, "can ah sell"))
            return true;
        break;
    case TravelDestinationPurpose::GatherFishing:
        if (!AI_VALUE2(bool, "has strategy", "tfish"))
            return false;
    case TravelDestinationPurpose::GatherSkinning:
    case TravelDestinationPurpose::GatherMining:
    case TravelDestinationPurpose::GatherHerbalism:
        skill = gatheringSkills.at(purpose);
        if (bot->GetSkillValue(skill) < std::min(bot->GetSkillMax(skill), bot->GetSkillMaxForLevel(bot)))
        {
            return true;
        }

        return false;
    case TravelDestinationPurpose::Boss:
        return AI_VALUE(bool, "can fight boss");
    case TravelDestinationPurpose::Mail:
        return AI_VALUE(bool, "can get mail") && AI_VALUE(bool, "should get mail");
    case TravelDestinationPurpose::Explore:
        return ai->HasStrategy("explore", BotState::BOT_STATE_NON_COMBAT);    
    case TravelDestinationPurpose::GenericRpg:
    {
        uint32 rpgPhase = ai->GetFixedBotNumber(BotTypeNumber::RPG_PHASE_NUMBER, 60, 1);

        if (rpgPhase < 15) //Only last 45 minutes of the hour allow generic rpg.
            return false;

        return !AI_VALUE2(bool, "manual bool", "is travel refresh");
    }
    case TravelDestinationPurpose::Grind:
    {
        uint32 rpgPhase = ai->GetFixedBotNumber(BotTypeNumber::RPG_PHASE_NUMBER, 60, 1);

        if (rpgPhase > 45) //Only first 45 minutes of the hour allow generic grind.
            return false;

        return !AI_VALUE2(bool, "manual bool", "is travel refresh");
    }
    default:
        return false;
    }

    return false;
}

bool ShouldTravelNamedValue::Calculate()
{
    std::string name = getQualifier();

    WorldPosition botPos(bot);

    if (name == "city")
    {
        if (bot->GetLevel() <= 5)
            return false;

        if (!botPos.isOverworld())
            return false;

        uint32 rpgPhase = ai->GetFixedBotNumber(BotTypeNumber::RPG_PHASE_NUMBER, 60, 1);

        if (rpgPhase > 20) //Only first 20 minutes of the hour allow generic city pvp without reason.
            return false;

        if (AI_VALUE2(bool, "manual bool", "is travel refresh"))
            return false;

        return true;
    }
    else if (name == "pvp")
    {
        if (bot->GetLevel() <= 50)
            return false;

        int32 rpgStyle = AI_VALUE2(int32, "manual saved int", "rpg style override");

        if (rpgStyle < 0)
            rpgStyle = ai->GetFixedBotNumber(BotTypeNumber::RPG_STYLE_NUMBER, 100);

        if (rpgStyle > 10) //Only 10% of the bots like to go to world-pvp.
            return false;

        uint32 rpgPhase = ai->GetFixedBotNumber(BotTypeNumber::RPG_PHASE_NUMBER, 60, 1);

        if (rpgPhase > 15) //Only first 15 minutes of the hour allow world pvp.
            return false;

        if (!botPos.isOverworld())
            return false;

        return true;
    }
    else if (name == "mount")
    {
        if (AI_VALUE(bool, "can buy mount"))
            return true;

        return false;
    }
    else if (name.find("trainer") == 0)
    {
        TrainerType trainerType = TRAINER_TYPE_CLASS;
        NeedMoneyFor budgetType = NeedMoneyFor::spells;

        if (name == "trainer mount")
        {
            trainerType = TRAINER_TYPE_MOUNTS;
            budgetType = NeedMoneyFor::mount; //Only train mounts when you can actually buy mount
        }
        if (name == "trainer trade")
        {
            trainerType = TRAINER_TYPE_TRADESKILLS;
            budgetType = NeedMoneyFor::skilltraining;
        }
        if (name == "trainer pet")
        {
            trainerType = TRAINER_TYPE_PETS;
            budgetType = NeedMoneyFor::anything;
        }

        if (AI_VALUE2(uint32, "train cost", trainerType) == 0) //Has nothing to train
            return false;

        if (!AI_VALUE2(bool, "has all money for", (uint32)budgetType))
            return false;

        return true;
    }

    return false;
}

bool TravelTargetActiveValue::Calculate() 
{
    return AI_VALUE(TravelTarget*, "travel target")->IsActive(); 
};

bool TravelTargetTravelingValue::Calculate()
{
    return AI_VALUE(TravelTarget*, "leader travel target")->GetStatus() == TravelStatus::TRAVEL_STATUS_TRAVEL || AI_VALUE(TravelTarget*, "leader travel target")->GetStatus() == TravelStatus::TRAVEL_STATUS_READY;
};

bool TravelTargetWorkingValue::Calculate()
{
    return AI_VALUE(TravelTarget*, "leader travel target")->GetStatus() == TravelStatus::TRAVEL_STATUS_WORK;
};

bool QuestStageActiveValue::Calculate()
{
    uint32 questId = getMultiQualifierInt(getQualifier(), 0, ",");
    TravelDestinationPurpose purpose = (TravelDestinationPurpose)getMultiQualifierInt(getQualifier(), 1, ",");

    uint32 objective = 0;

    switch (purpose)
    {
    case TravelDestinationPurpose::QuestGiver:
        if (bot->HasQuest(questId))
            return false;
        break;
    case TravelDestinationPurpose::QuestTaker:
        if (!bot->CanCompleteQuest(questId))
            return false;
        break;
    case TravelDestinationPurpose::QuestObjective1:
        objective = 1;
        break;
    case TravelDestinationPurpose::QuestObjective2:
        objective = 2;
        break;
    case TravelDestinationPurpose::QuestObjective3:
        objective = 3;
        break;
    case TravelDestinationPurpose::QuestObjective4:
        objective = 4;
        break;
    }

    if(objective)
        if (!AI_VALUE2(bool, "need quest objective", "{" + std::to_string(questId) + "," + std::to_string(objective - 1) + "}"))
            return false;

    return true;
}