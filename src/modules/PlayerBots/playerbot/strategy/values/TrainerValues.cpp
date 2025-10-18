
#include "playerbot/playerbot.h"
#include "TrainerValues.h"
#include "SharedValueContext.h"
#include "playerbot/PlayerbotHelpMgr.h"

using namespace ai;


trainableSpellMap* TrainableSpellMapValue::Calculate()
{
    trainableSpellMap* spellMap = new trainableSpellMap;

    //           template, trainer
    std::unordered_map <uint32, std::vector<CreatureInfo const*>> trainerTemplateIds;

    //Select all trainer lists and their trainers.
    for (uint32 id = 0; id < sCreatureStorage.GetMaxEntry(); ++id)
    {
        CreatureInfo const* creatureInfo = sCreatureStorage.LookupEntry<CreatureInfo>(id);
        if (!creatureInfo)
            continue;

        if (!creatureInfo->TrainerType && !creatureInfo->TrainerClass)
            continue;

        if(creatureInfo->TrainerTemplateId)
            trainerTemplateIds[creatureInfo->TrainerTemplateId].push_back(creatureInfo);
        else
            trainerTemplateIds[id].push_back(creatureInfo);
    }

    for (auto& [templateOrEntryId, trainers] : trainerTemplateIds)
    {
        TrainerSpellData const* trainer_spells = sObjectMgr.GetNpcTrainerTemplateSpells(templateOrEntryId);
        if (!trainer_spells)
            trainer_spells = sObjectMgr.GetNpcTrainerSpells(templateOrEntryId);

        if (!trainer_spells)
            continue;

        CreatureInfo const* firstTrainer = trainers.front();

        TrainerType trainerType = (TrainerType)firstTrainer->TrainerType;

        uint32 spellRequirement;
        if (trainerType == TRAINER_TYPE_CLASS || trainerType == TRAINER_TYPE_PETS)
            spellRequirement = firstTrainer->TrainerClass;
        else if (trainerType == TRAINER_TYPE_MOUNTS)
            spellRequirement = firstTrainer->TrainerRace;

        for (auto& [id, trainerSpell] : trainer_spells->spellList)
        {
            const TrainerSpell* sameTrainerSpell = &trainerSpell;
            for (auto& [otherTrainerSpell, trainers] : (*spellMap)[trainerType][spellRequirement])
            {
                if (otherTrainerSpell->spell != trainerSpell.spell)
                    continue;

                if (otherTrainerSpell->spellCost != trainerSpell.spellCost)
                    continue;

                if (otherTrainerSpell->reqSkill != trainerSpell.reqSkill)
                    continue;

                if (otherTrainerSpell->reqSkillValue != trainerSpell.reqSkillValue)
                    continue;

                if (otherTrainerSpell->reqLevel != trainerSpell.reqLevel)
                    continue;

#ifndef MANGOSBOT_TWO
                if (otherTrainerSpell->learnedSpell != trainerSpell.learnedSpell)
#else
                if (otherTrainerSpell->learnedSpell[0] != trainerSpell.learnedSpell[0])
#endif
                    continue;

                if (otherTrainerSpell->conditionId != trainerSpell.conditionId)
                    continue;

                sameTrainerSpell = otherTrainerSpell;
                break;
            }

            if (trainerType == TRAINER_TYPE_TRADESKILLS)
            {
                if (trainerSpell.reqSkill)
                    spellRequirement = trainerSpell.reqSkill;
                else
                {
                    // exist, already checked at loading
#ifndef MANGOSBOT_TWO
                    SpellEntry const* spell = sSpellTemplate.LookupEntry<SpellEntry>(trainerSpell.learnedSpell);
#else
                    SpellEntry const* spell = sSpellTemplate.LookupEntry<SpellEntry>(trainerSpell.learnedSpell[0]);
#endif

                    spellRequirement = spell->EffectMiscValue[1];
                }
            }

            for (auto& trainer : trainers)
                (*spellMap)[trainerType][spellRequirement][sameTrainerSpell].push_back(trainer->Entry);
        }
    }

    return spellMap;
}

std::vector<TrainerSpell const*> TrainableSpellsValue::Calculate()
{
    std::vector<TrainerSpell const*> trainableSpells;

    int8 qualifierType = getQualifier().empty() ? -1 : stoi(getQualifier());

    trainableSpellMap* spellMap = GAI_VALUE(trainableSpellMap*, "trainable spell map");

    for (auto& [trainerType, spellReqList] : *spellMap)
    {
        if (trainerType >= 0 && trainerType != qualifierType)
            continue;

        for (auto& [requirement, trainerSpellList] : spellReqList)
        {
            if (trainerType == TRAINER_TYPE_CLASS && requirement != bot->getClass())
                continue;
            if (trainerType == TRAINER_TYPE_MOUNTS && requirement != bot->getRace())
                continue;

            for (auto& [trainerSpell, trainers] : trainerSpellList)
            {
                uint32 reqLevel = 0;

                reqLevel = trainerSpell->isProvidedReqLevel ? trainerSpell->reqLevel : std::max(reqLevel, trainerSpell->reqLevel);
                TrainerSpellState state = bot->GetTrainerSpellState(trainerSpell, reqLevel);
                if (state != TRAINER_SPELL_GREEN)
                    continue;

                //Skip initial profession training.
#ifndef MANGOSBOT_TWO
                if (bot->GetLevel() < 10 && sSpellMgr.IsProfessionSpell(trainerSpell->learnedSpell) && sSpellMgr.GetSpellRank(trainerSpell->learnedSpell) == 1)
#else
                if (bot->GetLevel() < 10 && sSpellMgr.IsProfessionSpell(trainerSpell->learnedSpell[0]) && sSpellMgr.GetSpellRank(trainerSpell->learnedSpell[0]) == 1)
#endif
                    continue;

                trainableSpells.push_back(trainerSpell);
            }
        }
    }   

    return trainableSpells;
}

std::string TrainableSpellsValue::Format()
{
    std::vector<std::string> vec;  
    for (auto t : value) {
        SpellEntry const* spell = sServerFacade.LookupSpellInfo(t->spell);
        if (!spell)
            continue;
        vec.push_back(chat->formatSpell(spell));
    } 
    
    return sPlayerbotHelpMgr.makeList(vec, "[<part>]");
}

std::vector<int32> AvailableTrainersValue::Calculate()
{
    std::vector<TrainerSpell const*> trainableSpells = AI_VALUE2(std::vector<TrainerSpell const*>, "trainable spells", getQualifier());;
    std::vector<int32> retTrainers;

    int8 qualifierType = getQualifier().empty() ? -1 : stoi(getQualifier());

    trainableSpellMap* spellMap = GAI_VALUE(trainableSpellMap*, "trainable spell map");

    for (auto& [trainerType, spellReqList] : *spellMap)
    {
        if (trainerType >= 0 && trainerType != qualifierType)
            continue;

        for (auto& [requirement, trainerSpellList] : spellReqList)
        {
            if (trainerType == TRAINER_TYPE_CLASS && requirement != bot->getClass())
                continue;
            if (trainerType == TRAINER_TYPE_MOUNTS && requirement != bot->getRace())
                continue;

            for (auto& [trainerSpell, trainers] : trainerSpellList)
            {
                if (std::find(trainableSpells.begin(), trainableSpells.end(), trainerSpell) == trainableSpells.end())
                    continue;

                for (auto& trainer : trainers)
                {
                    if(std::find(retTrainers.begin(), retTrainers.end(), trainer) == retTrainers.end())
                        retTrainers.push_back(trainer);
                }
            }
        }
    }

    return retTrainers;
}

uint32 TrainCostValue::Calculate()
{
    uint32 TotalCost = 0;

    for (auto& spells : AI_VALUE2(std::vector<TrainerSpell const*>, "trainable spells", getQualifier()))
        TotalCost += spells->spellCost;
   
    return TotalCost;
}
