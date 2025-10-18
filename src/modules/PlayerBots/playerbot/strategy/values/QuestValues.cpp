
#include "playerbot/playerbot.h"
#include "QuestValues.h"
#include "SharedValueContext.h"
#include "ItemUsageValue.h"
#include "playerbot/TravelMgr.h"
#include "playerbot/strategy/deathknight/DKActions.h"

using namespace ai;


//What kind of a relation does this entry have with this quest.
EntryQuestRelationMap EntryQuestRelationMapValue::Calculate()
{
	EntryQuestRelationMap rMap;

	//Quest givers takers
	QuestObjectMgr* questObjectMgr = (QuestObjectMgr*)&sObjectMgr;

	for (auto [entry, questId] : questObjectMgr->GetCreatureQuestRelationsMap())
		rMap[entry][questId] |= (uint8)TravelDestinationPurpose::QuestGiver;

	for (auto [entry, questId] : questObjectMgr->GetCreatureQuestInvolvedRelationsMap())
		rMap[entry][questId] |= (uint8)TravelDestinationPurpose::QuestTaker;

	for (auto [entry, questId] : questObjectMgr->GetGOQuestRelationsMap())
		rMap[-(int32)entry][questId] |= (uint8)TravelDestinationPurpose::QuestGiver;

	for (auto [entry, questId] : questObjectMgr->GetGOQuestInvolvedRelationsMap())
		rMap[-(int32)entry][questId] |= (uint8)TravelDestinationPurpose::QuestTaker;

	//Quest objectives
	ObjectMgr::QuestMap const& questMap = sObjectMgr.GetQuestTemplates();

	for (auto& [questId, questPtr] : questMap)
	{
		Quest* quest = questPtr.get();

		for (uint32 objective = 0; objective < QUEST_OBJECTIVES_COUNT; objective++)
		{
			uint32 relationFlag = 1 << (objective+1);

			//Kill objective
			if (quest->ReqCreatureOrGOId[objective])
				rMap[quest->ReqCreatureOrGOId[objective]][questId] |= relationFlag;

			//Loot objective
			if (quest->ReqItemId[objective])
			{
				uint32 itemId = quest->ReqItemId[objective];
				while (itemId)
				{
					ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);

					if (proto)
					{
						for (auto& entry : GAI_VALUE2(std::list<int32>, "item drop list", itemId))
						{
							std::string chanceQualifier = std::to_string(entry) + " " + std::to_string(itemId);
							if (proto->Class == ITEM_CLASS_QUEST || GAI_VALUE2(float, "loot chance", chanceQualifier) > 5.0f)
								rMap[entry][questId] |= relationFlag;
						}
					}

					itemId = ItemUsageValue::ItemCreatedFrom(itemId);
				}
			}

			//Buy from vendor objective
			if (quest->ReqItemId[objective])
			{
				for (auto& entry : GAI_VALUE2(std::list<int32>, "item vendor list", quest->ReqItemId[objective]))
					rMap[entry][questId] |= relationFlag;
			}
		}

		//Target entry of source item of quest. 
		if (quest->GetSrcItemId())
		{
			ItemRequiredTargetMapBounds bounds = sObjectMgr.GetItemRequiredTargetMapBounds(quest->GetSrcItemId());

			if (bounds.first != bounds.second) //Add target of source item to second quest objective.
				for (ItemRequiredTargetMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
					rMap[itr->second.m_uiTargetEntry][questId] |= (uint8)TravelDestinationPurpose::QuestObjective2;
		}
	}

	//Hard coded:
#ifdef MANGOSBOT_TWO
	rMap[28406][12733] |= (uint8)TravelDestinationPurpose::QuestObjective1; //[Death Knight Initiate][Death's Challenge]
	rMap[28605][12680] |= (uint8)TravelDestinationPurpose::QuestObjective1; //[Havenshire Stallion][Grand Theft Palomino]
	rMap[28606][12680] |= (uint8)TravelDestinationPurpose::QuestObjective1; //[Havenshire Mare][Grand Theft Palomino]
	rMap[28607][12680] |= (uint8)TravelDestinationPurpose::QuestObjective1; //[Havenshire Cotl][Grand Theft Palomino]
	rMap[28653][12680] |= (uint8)TravelDestinationPurpose::QuestObjective1; //[Salanar the Horseman][Grand Theft Palomino]	

	rMap[28909][12687] |= (uint8)TravelDestinationPurpose::QuestObjective1; //[Dark Rider of Acherus][Into the Realm of Shadows]	
	rMap[28782][12687] |= (uint8)TravelDestinationPurpose::QuestObjective1; //[Acherus Deathcharger][Into the Realm of Shadows]	
	rMap[29501][12687] |= (uint8)TravelDestinationPurpose::QuestObjective1; //[Scourge Gryphon][Into the Realm of Shadows]	
	
	rMap[28577][12678] |= (uint8)TravelDestinationPurpose::QuestObjective2; //[Citizen of Havenshire][If Chaos Drives, Let Suffering Hold The Reins]
	rMap[28576][12678] |= (uint8)TravelDestinationPurpose::QuestObjective2; //[Citizen of Havenshire][If Chaos Drives, Let Suffering Hold The Reins]

	rMap[28658][12698] |= (uint8)TravelDestinationPurpose::QuestObjective1; //[Gothic the Harvester][The Gift That Keeps On Giving]	
	rMap[28819][12698] |= (uint8)TravelDestinationPurpose::QuestObjective1; //[Scarlet Miner][The Gift That Keeps On Giving]	

	rMap[-190767][12701] |= (uint8)TravelDestinationPurpose::QuestObjective1; //[Inconspicuous mine car][Massacre At Light's Point]	

	rMap[28941][12722] |= (uint8)TravelDestinationPurpose::QuestObjective2; //[Citizen of New Avalon][Lambs To The Slaughter]
    rMap[28942][12722] |= (uint8)TravelDestinationPurpose::QuestObjective2; //[Citizen of Havenshire][Lambs To The Slaughter]

	rMap[28912][12727] |= (uint8)TravelDestinationPurpose::QuestObjective1; //[Koltira Deathweaver][Bloody Breakout]

	rMap[28936][12754] |= (uint8)TravelDestinationPurpose::QuestObjective1; //[Scarlet Commander][Ambush At The Overlook]
    rMap[29076][12754] |= (uint8)TravelDestinationPurpose::QuestObjective1; //[Scarlet Courier][Ambush At The Overlook]    


	rMap[29102][12779] |= (uint8)TravelDestinationPurpose::QuestObjective1; //[HearthglenCrusader][An End To All Things]    
	rMap[29103][12779] |= (uint8)TravelDestinationPurpose::QuestObjective1; //[Tirisfal Crusader][An End To All Things]    
	rMap[29104][12779] |= (uint8)TravelDestinationPurpose::QuestObjective2; //[Scarlet Ballista][An End To All Things]    

	rMap[29173][12801] |= (uint8)TravelDestinationPurpose::QuestObjective1; //[Highlord Darion Mograine][The Light of Dawn]    
	rMap[29175][12801] |= (uint8)TravelDestinationPurpose::QuestObjective1; //[Highlord Tirion Fordring][The Light of Dawn]    
	
#endif
	return rMap;
}


//Get all the objective entries for a specific quest.
void FindQuestObjectData::GetObjectiveEntries()
{
	relationMap = GAI_VALUE(EntryQuestRelationMap, "entry quest relation");
}

//Data worker. Checks for a specific creature what quest they are needed for and puts them in the proper place in the quest map.
bool FindQuestObjectData::operator()(CreatureDataPair const& dataPair)
{
	uint32 entry = dataPair.second.id;

	for (auto& [questId, flag] : relationMap[entry])
	{
		data[questId][flag][entry].push_back(GuidPosition(&dataPair));
	}

	return false;
}

//GameObject data worker. Checks for a specific gameObject what quest they are needed for and puts them in the proper place in the quest map.
bool FindQuestObjectData::operator()(GameObjectDataPair const& dataPair)
{
	int32 entry = dataPair.second.id * -1;

	for (auto& [questId,flag] : relationMap[entry])
	{
		data[questId][flag][entry].push_back(GuidPosition(&dataPair));
	}

	return false;
}

//Goes past all creatures and gameobjects and creatures the full quest guid map.
questGuidpMap QuestGuidpMapValue::Calculate()
{
	FindQuestObjectData worker;
	sObjectMgr.DoCreatureData(worker);
	sObjectMgr.DoGOData(worker);
	return worker.GetResult();
}

//Selects all questgivers for a specific level (range).
questGiverMap QuestGiversValue::Calculate()
{
	uint32 level = 0;
	std::string q = getQualifier();
	bool hasQualifier = !q.empty();

	if (hasQualifier)
		level = stoi(q);

	questGuidpMap questMap = GAI_VALUE(questGuidpMap, "quest guidp map");

	questGiverMap guidps;

	for (auto& [questId, questRelationGuidps]: questMap)
	{
		for (auto& entry : questRelationGuidps[(uint8)TravelDestinationPurpose::QuestGiver])
		{
			for (auto& guidp : entry.second)
			{
				if (hasQualifier)
				{
					Quest const* quest = sObjectMgr.GetQuestTemplate(questId);

					if (quest && (level < quest->GetMinLevel() || (int32)level > quest->GetQuestLevel() + 10))
						continue;
				}

				guidps[questId].push_back(guidp);
			}
		}
	}

	return guidps;
}

std::list<GuidPosition> ActiveQuestGiversValue::Calculate()
{
	questGiverMap qGivers = GAI_VALUE2(questGiverMap, "quest givers", bot->GetLevel());

	std::list<GuidPosition> retQuestGivers;

	for (auto& [questId, guidPs] :qGivers)
	{
		Quest const* quest = sObjectMgr.GetQuestTemplate(questId);

		if (!quest || !quest->IsActive())
		{
			continue;
		}

		if (!bot->CanTakeQuest(quest, false))
			continue;

		QuestStatus status = bot->GetQuestStatus(questId);

		if (status != QUEST_STATUS_NONE)
			continue;

		for (auto& guidp : guidPs)
		{
			CreatureInfo const* creatureInfo = guidp.GetCreatureTemplate();

			if (creatureInfo)
			{
				if (!ai->IsFriendlyTo(creatureInfo->Faction))
					continue;
			}

			if (guidp.isDead(bot->GetInstanceId()))
				continue;

			retQuestGivers.push_back(guidp);
		}
	}

	return retQuestGivers;
}

std::list<GuidPosition> ActiveQuestTakersValue::Calculate()
{
	questGuidpMap questMap = GAI_VALUE(questGuidpMap, "quest guidp map");

	std::list<GuidPosition> retQuestTakers;

	QuestStatusMap& questStatusMap = bot->getQuestStatusMap();

	for (auto& [questId, questStatusData]: questStatusMap)
	{
		Quest const* quest = sObjectMgr.GetQuestTemplate(questId);

		if (!quest || !quest->IsActive())
		{
			continue;
		}

		QuestStatus status = questStatusData.m_status;

		if ((status != QUEST_STATUS_COMPLETE || bot->GetQuestRewardStatus(questId)) && (!quest->IsAutoComplete() || !bot->CanTakeQuest(quest, false)))
			continue;

		auto q = questMap.find(questId);

		if (q == questMap.end())
			continue;

		auto qt = q->second.find((uint8)TravelDestinationPurpose::QuestTaker);

		if (qt == q->second.end())
			continue;		

		for (auto& [entry, guidps] : qt->second)
		{
			if (entry > 0)
			{
				CreatureInfo const* info = sObjectMgr.GetCreatureTemplate(entry);

				if (info)
				{
					if (!ai->IsFriendlyTo(info->Faction))
						continue;
				}
			}

			for (auto& guidp : guidps)
			{
				if (guidp.isDead(bot->GetInstanceId()))
					continue;

				retQuestTakers.push_back(guidp);
			}
		}
	}

	return retQuestTakers;
}

std::list<GuidPosition> ActiveQuestObjectivesValue::Calculate()
{
	questGuidpMap questMap = GAI_VALUE(questGuidpMap, "quest guidp map");

	std::list<GuidPosition> retQuestObjectives;

	QuestStatusMap& questStatusMap = bot->getQuestStatusMap();

	for (auto& [questId, questStatusData] : questStatusMap)
	{		
		Quest const* quest = sObjectMgr.GetQuestTemplate(questId);

		if (!quest || !quest->IsActive())
		{
			continue;
		}

		if (questStatusData.m_status != QUEST_STATUS_INCOMPLETE)
			continue;

		for (uint32 objective = 0; objective < QUEST_OBJECTIVES_COUNT; objective++)
		{
			if (quest->ReqItemCount[objective])
			{
				uint32  reqCount = quest->ReqItemCount[objective];
				uint32  hasCount = questStatusData.m_itemcount[objective];

				if (!reqCount || hasCount >= reqCount)
					continue;
			}

			if (quest->ReqCreatureOrGOCount[objective])
			{
				uint32 reqCount = quest->ReqCreatureOrGOCount[objective];
				uint32 hasCount = questStatusData.m_creatureOrGOcount[objective];

				if (!reqCount || hasCount >= reqCount)
					continue;
			}

			auto q = questMap.find(questId);

			if (q == questMap.end())
				continue;

			auto qt = q->second.find((uint8)TravelDestinationPurpose(1<<(objective+1)));

			if (qt == q->second.end())
				continue;

			for (auto& entry : qt->second)
			{
				for (auto& guidp : entry.second)
				{
					if (guidp.isDead(bot->GetInstanceId()))
						continue;

					retQuestObjectives.push_back(guidp);
				}
			}
		}
	}

	return retQuestObjectives;
}

bool NeedForQuestValue::Calculate()
{
	if (!Qualified::isValidNumberString(getQualifier()))
		return false;

	int32 entry = stoi(getQualifier());

	const PlayerTravelInfo& info(bot);

	std::list<GuidPosition> retQuestObjectives;

	QuestStatusMap& questStatusMap = bot->getQuestStatusMap();

	for (auto& questStatus : questStatusMap)
	{
		uint32 questId = questStatus.first;

		Quest const* quest = sObjectMgr.GetQuestTemplate(questId);

		if (!quest || !quest->IsActive())
		{
			continue;
		}

		QuestStatusData statusData = questStatus.second;

		if (statusData.m_status != QUEST_STATUS_INCOMPLETE)
			continue;

		for (uint32 objective = 0; objective < QUEST_OBJECTIVES_COUNT; objective++)
		{
			std::vector<std::string> qualifier = { std::to_string(questId), std::to_string(objective) };

			if (!AI_VALUE2(bool, "need quest objective", Qualified::MultiQualify(qualifier, ",")))
				continue;

			DestinationList destinations = sTravelMgr.GetDestinations(info, uint32(TravelDestinationPurpose::QuestAllObjective), {int32(questId)}, false, 0);
			
			for (auto destination : destinations)
			{
                if (entry == destination->GetEntry())
					return true;

			}
		}
	}

	return false;
}

uint8 FreeQuestLogSlotValue::Calculate()
{
	uint8 numQuest = 0;
	for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
	{
		uint32 questId = bot->GetQuestSlotQuestId(slot);

		if (!questId)
			continue;

		Quest const* quest = sObjectMgr.GetQuestTemplate(questId);
		if (!quest)
			continue;

		numQuest++;
	}

	return MAX_QUEST_LOG_SIZE - numQuest;
}

uint32 DialogStatusValue::getDialogStatus(Player* bot, int32 questgiver, uint32 questId)
{
	uint32 dialogStatus = DIALOG_STATUS_NONE;

	QuestRelationsMapBounds rbounds;                        // QuestRelations (quest-giver)
	QuestRelationsMapBounds irbounds;                       // InvolvedRelations (quest-finisher)

	if (questgiver > 0)
	{
		rbounds = sObjectMgr.GetCreatureQuestRelationsMapBounds(questgiver);
		irbounds = sObjectMgr.GetCreatureQuestInvolvedRelationsMapBounds(questgiver);
	}
	else
	{
		rbounds = sObjectMgr.GetGOQuestRelationsMapBounds(questgiver * -1);
		irbounds = sObjectMgr.GetGOQuestInvolvedRelationsMapBounds(questgiver * -1);
	}

	// Check markings for quest-finisher
	for (QuestRelationsMap::const_iterator itr = irbounds.first; itr != irbounds.second; ++itr)
	{
		if (questId && itr->second != questId)
			continue;

		Quest const* pQuest = sObjectMgr.GetQuestTemplate(itr->second);

		uint32 dialogStatusNew = DIALOG_STATUS_NONE;

		if (!pQuest || !pQuest->IsActive())
		{
			continue;
		}

		QuestStatus status = bot->GetQuestStatus(itr->second);

		if ((status == QUEST_STATUS_COMPLETE && !bot->GetQuestRewardStatus(itr->second)) ||
			(pQuest->IsAutoComplete() && bot->CanTakeQuest(pQuest, false)))
		{
			if (pQuest->IsAutoComplete() && pQuest->IsRepeatable())
			{
				dialogStatusNew = DIALOG_STATUS_REWARD_REP;
			}
			else
			{
				dialogStatusNew = DIALOG_STATUS_REWARD2;
			}
		}
		else if (status == QUEST_STATUS_INCOMPLETE)
		{
			dialogStatusNew = DIALOG_STATUS_INCOMPLETE;
		}

		if (dialogStatusNew > dialogStatus)
		{
			dialogStatus = dialogStatusNew;
		}
	}

	// check markings for quest-giver
	for (QuestRelationsMap::const_iterator itr = rbounds.first; itr != rbounds.second; ++itr)
	{
		if (questId && itr->second != questId)
			continue;

		Quest const* pQuest = sObjectMgr.GetQuestTemplate(itr->second);

		uint32 dialogStatusNew = DIALOG_STATUS_NONE;

		if (!pQuest || !pQuest->IsActive())
		{
			continue;
		}

		QuestStatus status = bot->GetQuestStatus(itr->second);

		if (status == QUEST_STATUS_NONE)                    // For all other cases the mark is handled either at some place else, or with involved-relations already
		{
			if (bot->CanSeeStartQuest(pQuest))
			{
				if (bot->SatisfyQuestLevel(pQuest, false))
				{
					int32 lowLevelDiff = sWorld.getConfig(CONFIG_INT32_QUEST_LOW_LEVEL_HIDE_DIFF);
					if (pQuest->IsAutoComplete() || (pQuest->IsRepeatable() && bot->getQuestStatusMap()[itr->second].m_rewarded))
					{
						dialogStatusNew = DIALOG_STATUS_REWARD_REP;
					}
                    else if (lowLevelDiff < 0 || bot->GetLevel() <= bot->GetQuestLevelForPlayer(pQuest) + uint32(lowLevelDiff) || pQuest->GetRequiredClasses())
					{
						dialogStatusNew = DIALOG_STATUS_AVAILABLE;
					}
					else
					{
#ifndef MANGOSBOT_TWO
						dialogStatusNew = DIALOG_STATUS_CHAT;
#else
						dialogStatusNew = DIALOG_STATUS_LOW_LEVEL_AVAILABLE;
#endif
					}
				}
				else
				{
					dialogStatusNew = DIALOG_STATUS_UNAVAILABLE;
				}
			}
		}

		if (dialogStatusNew > dialogStatus)
		{
			dialogStatus = dialogStatusNew;
		}
	}

	return dialogStatus;
}

bool NeedQuestRewardValue::Calculate()
{
	uint32 questId = stoi(getQualifier());
	Quest const* pQuest = sObjectMgr.GetQuestTemplate(questId);

	if (pQuest->GetRequiredClasses()) //Always try to do class quests.
        return true;

	for (uint8 i = 0; i < pQuest->GetRewChoiceItemsCount(); ++i)
	{
		ItemUsage usage = AI_VALUE2_LAZY(ItemUsage, "item usage", pQuest->RewChoiceItemId[i]);
		if (usage == ItemUsage::ITEM_USAGE_EQUIP || usage == ItemUsage::ITEM_USAGE_BAD_EQUIP)
			return true;
	}

	return false;
}

bool NeedQuestObjectiveValue::CanGetItemSomewhere(const uint32 itemId, const uint32 reqCount, Player* bot)
{
    if (!GAI_VALUE2(std::list<int32>, "item drop list", itemId).empty()) //Can get it from drop.
        return true;

    if (GAI_VALUE2(std::list<int32>, "item vendor list", itemId).empty()) //Can not get it from vendor.
        return true;

    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);

    if (!proto)
        return true;

    if (bot->GetMoney() > proto->BuyPrice * reqCount) //Bot has money to buy item from vendor.
        return true;

	return false;
}

bool NeedQuestObjectiveValue::Calculate()
{
	uint32 questId = getMultiQualifierInt(getQualifier(),0,",");
	if (!bot->IsActiveQuest(questId))
		return false;

	QuestStatusData& questStatus = bot->getQuestStatusMap().at(questId);

	if (questStatus.m_status != QUEST_STATUS_INCOMPLETE)
		return false;

	if (getQualifier().find(",") == std::string::npos) //Status of entire quest.
		return true;

#ifdef MANGOSBOT_TWO        
		switch (questId) {
		case 12687: //Into the Realm of Shadows
			return true; //No objectives for this quest.
		}
#endif

	Quest const* pQuest = sObjectMgr.GetQuestTemplate(questId);

	uint32 objective = getMultiQualifierInt(getQualifier(), 1, ",");

	uint32  reqCount = pQuest->ReqItemCount[objective];
	uint32  hasCount = questStatus.m_itemcount[objective];

	bool needItems = false;
    bool needCreatures = false;

	if (reqCount && hasCount < reqCount)
	{
		uint32 itemId = pQuest->ReqItemId[objective];

        if (CanGetItemSomewhere(itemId, reqCount, bot))
            needItems = true;
	}

	reqCount = pQuest->ReqCreatureOrGOCount[objective];
	hasCount = questStatus.m_creatureOrGOcount[objective];

	if (reqCount && hasCount < reqCount)
        needCreatures = true;

	return needItems || needCreatures;
}

bool CanUseItemOn::Calculate()
{
	uint32 itemId = getMultiQualifierInt(getQualifier(), 0, ",");
	std::string guidPString = getMultiQualifierStr(getQualifier(), 1, ",");
	GuidPosition guidP(guidPString);

	Unit* unit = nullptr;
	GameObject* gameObject = nullptr;

	if (!guidP)
		return false;

	switch (itemId)
    {
        case 17117:                                                                                                                 //Rat Catcher's Flute
            return guidP.IsCreature() && guidP.GetEntry() == 13016;                                                                 //Deeprun Rat
        case 52566:                                                                                                                 //Motivate-a-Tron (currently broken?)
            return guidP.IsCreature() && guidP.GetEntry() == 39623;                                                                 //Gnome Citizen
        case 38607:                                                                                                                 //Battle-worn Sword
            return guidP.IsGameObject() && (std::find(RUNEFORGES.begin(), RUNEFORGES.end(), guidP.GetEntry()) != RUNEFORGES.end()); //Runeforge
        case 39253:                                                                                                                 //Gift of the Harester
            return guidP.IsCreature() && guidP.GetEntry() == 28819;                                                                 //Scarlet Miner
        case 39645:                                                                                                                 //Makeshift Cover
            return guidP.IsCreature() && guidP.GetEntry() == 28936;                                                                 //Scarlet Commander
        case 39700:                                                                                                                 //Horn of the Frostbrood
            /*
			switch (AI_VALUE(TravelTarget*, "travel target")->GetEntry())
            case 29102: //HearthglenCrusader
            case 29103: //Tirisfal Crusader
            case 29104: //Scarlet Ballista
			*/
            return !AI_VALUE2(bool, "trigger active", "in vehicle") && AI_VALUE2(bool, "need quest objective", "12779"); //Mount up!
    }

	if (guidP.IsUnit())
		unit = guidP.GetUnit(bot->GetInstanceId());
	else if (guidP.IsGameObject())
		gameObject = guidP.GetGameObject(bot->GetInstanceId());

	ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);

	if (!proto)
		return false;

	bool tryRandomCast = !urand(0, 20);

	uint32 spellId = proto->Spells[0].SpellId;
	if (spellId)
	{
		SpellEntry const* spellInfo = sServerFacade.LookupSpellInfo(spellId);

		if (unit)
		{
			if (spellInfo->Effect[0] == SPELL_EFFECT_DUMMY && spellInfo->Id == 19938)  // Awaken Lazy Peon (hardcoded in core!)
			{
				// 17743 = Lazy Peon Sleep | 10556 = Lazy Peon
				if (!unit->HasAura(17743) || unit->GetEntry() != 10556)
					return false;

				return true;
			}

			if (ai->CanCastSpell(spellId, unit, 0, false, nullptr, true))
				return tryRandomCast;
		}
		else if (gameObject)
		{
			if (ai->CanCastSpell(spellId, gameObject, 0, false, true))
				return tryRandomCast;
		}
	}

	return false;
};

bool HasNearbyQuestTakerValue::Calculate()
{
	std::list<ObjectGuid> possibleTargets = AI_VALUE(std::list<ObjectGuid>, "possible rpg targets");

	int32 travelEntry = AI_VALUE(TravelTarget*, "travel target")->GetEntry();
	
	for (auto& target : possibleTargets)
	{
		if (target.GetEntry() == travelEntry)
			continue;

		if(AI_VALUE2(bool, "can turn in quest npc", target.GetEntry()))
			return true;
	}

	std::list<ObjectGuid> possibleObjects = bot->GetMap()->IsDungeon() ? AI_VALUE(std::list<ObjectGuid>, "nearest game objects") : AI_VALUE(std::list<ObjectGuid>, "nearest game objects no los");

	for (auto& target : possibleObjects)
	{
		if (target.GetEntry() == (travelEntry * -1))
			continue;

		if (AI_VALUE2(bool, "can turn in quest npc", target.GetEntry()))
			return true;
	}

	return false;
};