
#include "playerbot/playerbot.h"
#include "AutoLearnSpellAction.h"
#include "playerbot/ServerFacade.h"
#include "Entities/Item.h"
#include <Mails/Mail.h>

using namespace ai;

bool AutoLearnSpellAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string param = event.getParam();

    std::ostringstream out;

    LearnSpells(&out);

    if (!out.str().empty())
    {
        const std::string& temp = out.str();
        out.seekp(0);
        out << temp;
        out.seekp(-2, out.cur);
        out << ".";

        std::map<std::string, std::string> args;
        args["%spells"] = out.str();
        ai->TellPlayer(requester, BOT_TEXT2("auto_learn_spell", args), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
    }

    return true;
}

void AutoLearnSpellAction::LearnSpells(std::ostringstream* out)
{
    BroadcastHelper::BroadcastLevelup(ai, bot);

    if (sPlayerbotAIConfig.autoLearnQuestSpells)
        LearnQuestSpells(out);

    if (sPlayerbotAIConfig.autoLearnTrainerSpells)
        LearnTrainerSpells(out);

#ifdef MANGOSBOT_ZERO
    if (sPlayerbotAIConfig.autoLearnDroppedSpells)
        LearnDroppedSpells(out);
#endif

    if (!ai->HasActivePlayerMaster()) //Hunter spells for pets.
    {
        if (bot->getClass() == CLASS_HUNTER && bot->GetLevel() >= 10)
        {
#if !defined(MANGOSBOT_TWO) // Beast training not available in WotLK 
            bot->learnSpell(5149, false); //Beast training
#endif
            bot->learnSpell(883, false); //Call pet
            bot->learnSpell(982, false); //Revive pet
            bot->learnSpell(6991, false); //Feed pet
            bot->learnSpell(1515, false); //Tame beast
        }
    }
}

void AutoLearnSpellAction::LearnTrainerSpells(std::ostringstream* out)
{
    bot->learnDefaultSpells();

    for (uint32 id = 0; id < sCreatureStorage.GetMaxEntry(); ++id)
    {
        CreatureInfo const* co = sCreatureStorage.LookupEntry<CreatureInfo>(id);
        if (!co)
            continue;

        if (co->TrainerType != TRAINER_TYPE_CLASS && 
            co->TrainerType != TRAINER_TYPE_TRADESKILLS &&
            co->TrainerType != TRAINER_TYPE_PETS)
            continue;

        if ((co->TrainerType == TRAINER_TYPE_CLASS || co->TrainerType == TRAINER_TYPE_PETS) && co->TrainerClass != bot->getClass())
            continue;

        uint32 trainerId = co->TrainerTemplateId;
        if (!trainerId)
            trainerId = co->Entry;

        TrainerSpellData const* trainer_spells = sObjectMgr.GetNpcTrainerTemplateSpells(trainerId);
        if (!trainer_spells)
            trainer_spells = sObjectMgr.GetNpcTrainerSpells(trainerId);

        if (!trainer_spells)
            continue;

        for (TrainerSpellMap::const_iterator itr = trainer_spells->spellList.begin(); itr != trainer_spells->spellList.end(); ++itr)
        {
            TrainerSpell const* tSpell = &itr->second;

            if (!tSpell)
                continue;

            uint32 reqLevel = 0;

            reqLevel = tSpell->isProvidedReqLevel ? tSpell->reqLevel : std::max(reqLevel, tSpell->reqLevel);
            TrainerSpellState state = bot->GetTrainerSpellState(tSpell, reqLevel);
            if (state != TRAINER_SPELL_GREEN)
                continue;

            if (co->TrainerType == TRAINER_TYPE_TRADESKILLS)
            {
                SpellEntry const* spell = sServerFacade.LookupSpellInfo(tSpell->spell);
                if (spell)
                {
                    std::string SpellName = spell->SpellName[0];
#ifdef MANGOSBOT_ZERO
                    if (spell->Effect[EFFECT_INDEX_1] == SPELL_EFFECT_SKILL_STEP)
#elif defined(MANGOSBOT_ONE) || defined(MANGOSBOT_TWO) // TBC OR WOTLK
                        if (spell->Effect[EFFECT_INDEX_1] == SPELL_EFFECT_SKILL || spell->Effect[EFFECT_INDEX_1] == SPELL_EFFECT_SKILL_STEP)
#endif
                        {
                            uint32 skill = spell->EffectMiscValue[EFFECT_INDEX_1];

                            if (skill)
                            {
                                SkillLineEntry const* pSkill = sSkillLineStore.LookupEntry(skill);
                                if (pSkill)
                                {
#ifdef MANGOSBOT_ZERO
                                    if (SpellName.find("Apprentice") != std::string::npos && pSkill->categoryId == SKILL_CATEGORY_PROFESSION || pSkill->categoryId == SKILL_CATEGORY_SECONDARY)
                                        continue;
#elif defined(MANGOSBOT_ONE) || defined(MANGOSBOT_TWO) // TBC OR WOTLK
                                    std::string SpellRank = spell->Rank[0];
                                    if (SpellName.find("Apprentice") != std::string::npos && (pSkill->categoryId == SKILL_CATEGORY_PROFESSION || pSkill->categoryId == SKILL_CATEGORY_SECONDARY))
                                        continue;
                                    else if (SpellRank.find("Apprentice") != std::string::npos && (pSkill->categoryId == SKILL_CATEGORY_PROFESSION || pSkill->categoryId == SKILL_CATEGORY_SECONDARY))
                                        continue;
#endif
                                }
                            }
                        }
                }

            }
#ifdef MANGOSBOT_ZERO // Vanilla
            LearnSpellFromSpell(tSpell->spell, out);
#elif defined(MANGOSBOT_ONE) || defined(MANGOSBOT_TWO)
            if (IsTeachingSpellListedAsSpell(tSpell->spell))
            {
                LearnSpellFromSpell(tSpell->spell, out);
            }
            else
            {
                LearnSpell(tSpell->spell, out);
            }
#endif
        }
    }
}

void AutoLearnSpellAction::LearnQuestSpells(std::ostringstream* out)
{
    ObjectMgr::QuestMap const& questTemplates = sObjectMgr.GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
    {
        uint32 questId = i->first;
        Quest const* quest = i->second.get();

        if (!quest->GetRequiredClasses() || quest->IsRepeatable())
            continue;

        if (!bot->SatisfyQuestClass(quest, false) ||
            quest->GetMinLevel() > bot->GetLevel() ||
            !bot->SatisfyQuestRace(quest, false))
            continue;

        if (quest->GetRewSpellCast() > 0 &&
            quest->GetRewSpellCast() != 12510) // Prevents mages from learning the Teleport to Azushara Tower spell.
        {
            if (LearnSpellFromSpell(quest->GetRewSpellCast(), out))
            {
                GetClassQuestItem(quest, out);
            }
            // Shaman Call of Air Quest casts Swift Wind on player and rewards Air Totem, Swift Wind is not to be learned however it is a one time cast.
            else if (quest->GetRewSpellCast() == 8385)
            {
                bool hasAirTotem = false;
                hasAirTotem = bot->HasItemCount(5178, 1, true);
                if (!hasAirTotem)
                {
                    GetClassQuestItem(quest, out);
                }
            }
        }
        else if (quest->GetRewSpell() > 0)
        {
            if (IsTeachingSpellListedAsSpell(quest->GetRewSpell()))
            {
                if (LearnSpellFromSpell(quest->GetRewSpell(), out))
                {
                    GetClassQuestItem(quest, out);
                }
            }
            else
            {
                if (LearnSpell(quest->GetRewSpell(), out))
                {
                    GetClassQuestItem(quest, out);
                }
            }
        }
    }
}

void AutoLearnSpellAction::LearnDroppedSpells(std::ostringstream* out)
{   //       Class       Level      // Spells
    std::map<uint8, std::map<uint8, std::vector<uint32>>> spellList;
    spellList[CLASS_WARRIOR][60] = { 25289, 25288, 25286 };
    spellList[CLASS_PALADIN][60] = { 25291, 25290, 25292 };
    spellList[CLASS_HUNTER][60] = { 19801, 25296, 25294, 25295 };
    spellList[CLASS_ROGUE][60] = { 25300, 25347, 25302, 31016 };
    spellList[CLASS_PRIEST][48] = { 21562 };
    // Spell 27683 Prayer of Shadow Protection https://www.wowhead.com/classic/spell=27683/prayer-of-shadow-protection book requires level 60, spell requires 56. 
    // Went with the book requirement.
    spellList[CLASS_PRIEST][60] = { 25314, 25315, 25316,21564,27683 };
    spellList[CLASS_SHAMAN][60] = { 29228, 25359, 25357, 25361 };
    spellList[CLASS_MAGE][56] = { 23028 };
    // Spell 25345 Arcane Missiles https://www.wowhead.com/classic/spell=25345/arcane-missiles book requires level 60, spell requires 56.
    // Went with the book requirement.
    spellList[CLASS_MAGE][60] = { 28612, 28609, 25345, 25306, 25304, 28271 };
    spellList[CLASS_WARLOCK][50] = { 1122 };
    spellList[CLASS_WARLOCK][60] = { 18540, 25311, 25309, 25307, 28610 };
    spellList[CLASS_DRUID][50] = { 21849 };
    spellList[CLASS_DRUID][60] = { 31018, 25297, 25299, 25298, 21850 };
    
    for (const auto& levelSpells : spellList[bot->getClass()])
    {
        if (bot->GetLevel() >= levelSpells.first)
        {
            for (uint32 spellId : levelSpells.second)
            {
                if (!bot->HasSpell(spellId))
                {
                    bot->learnSpell(spellId, false);
                }
            }
        }
    }
}

/**
* Attempts to add the quest item
* If the bot's bag is full report to player.
*/
void AutoLearnSpellAction::GetClassQuestItem(Quest const* quest, std::ostringstream* out)
{
    if (quest->GetRewItemsCount() > 0)
    {
        for (uint32 i = 0; i < quest->GetRewItemsCount(); i++)
        {
            ItemPosCountVec itemVec;
            ItemPrototype const* itemP = sObjectMgr.GetItemPrototype(quest->RewItemId[i]);
            InventoryResult result = bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, itemVec, itemP->ItemId, quest->RewItemCount[i]);
            if (result == EQUIP_ERR_OK)
            {
                if (quest->RewItemId[i] != 8432 && // Stops Rogues from getting a quest reward item that is a quest item itself.
                    quest->RewItemId[i] != 8095)   // Stops Rogues from getting a quest reward item that is a quest item itself.
                {
                    ItemPosCountVec itemVec;
                    ItemPrototype const* itemP = sObjectMgr.GetItemPrototype(quest->RewItemId[i]);
                    InventoryResult result = bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, itemVec, itemP->ItemId, quest->RewItemCount[i]);
                    if (result == EQUIP_ERR_OK)
                    {
                        bot->StoreNewItemInInventorySlot(itemP->ItemId, quest->RewItemCount[i]);
                        *out << "Got " << chat->formatItem(itemP, 1, 1) << " from " << quest->GetTitle();
                    }
                    else if (result == EQUIP_ERR_INVENTORY_FULL)
                    {
                        MailDraft draft("Item(s) from quest reward", quest->GetTitle());
                        Item* item = item->CreateItem(itemP->ItemId, quest->RewItemCount[i]);
                        draft.AddItem(item);
                        draft.SendMailTo(MailReceiver(bot), MailSender(bot));
                        *out << "Could not add item " << chat->formatItem(itemP) << " from " << quest->GetTitle() << ". " << bot->GetName() << "'s inventory is full.";
                    }
                }
            }
        }
    }
}

std::string formatSpell(SpellEntry const* sInfo)
{
    std::ostringstream out;
    std::string rank = sInfo->Rank[0];

    if (rank.empty())
        out << "|cffffffff|Hspell:" << sInfo->Id << "|h[" << sInfo->SpellName[LOCALE_enUS] << "]|h|r" << " (" << sInfo->Id << ")";
    else
        out << "|cffffffff|Hspell:" << sInfo->Id << "|h[" << sInfo->SpellName[LOCALE_enUS] << " " << rank << "]|h|r" << " (" << sInfo->Id << ")";
    return out.str();
}

bool AutoLearnSpellAction::LearnSpell(uint32 spellId, std::ostringstream* out)
{
    bool learned = false;
    if (IsValidSpell(spellId))
    {
        SpellEntry const* proto = sServerFacade.LookupSpellInfo(spellId);

        if (!proto)
            return false;
        if (!learned && !bot->HasSpell(spellId)) {
            bot->learnSpell(spellId, false);
            *out << formatSpell(proto) << ", ";

            learned = bot->HasSpell(spellId);
        }
    }
    return learned;
}

bool AutoLearnSpellAction::LearnSpellFromSpell(uint32 spellId, std::ostringstream* out)
{
    SpellEntry const* proto = sServerFacade.LookupSpellInfo(spellId);

    if (!proto)
        return false;

    bool learned = false;
    for (int j = 0; j < 3; ++j)
    {
        if (proto->Effect[j] == SPELL_EFFECT_LEARN_SPELL)
        {
            uint32 learnedSpell = proto->EffectTriggerSpell[j];

            if (IsValidSpell(learnedSpell))
            {
                if (!bot->HasSpell(learnedSpell))
                {
                    bot->learnSpell(learnedSpell, false);
                    SpellEntry const* spellInfo = sServerFacade.LookupSpellInfo(learnedSpell);
                    *out << formatSpell(spellInfo) << ", ";
                    learned = true;
                }
            }
        }
    }
    return learned;
}
/**
* Some spells are being learned when they shouldn't be, either they are left over spells from previous patches or from mobs, or just unused blizz spells or in some cases the spell passed is also the spell that is used to teach the actual spell.
*/
bool AutoLearnSpellAction::IsValidSpell(uint32 spellId)
{
    bool isSpellValid = true;
#ifdef MANGOSBOT_ZERO
    isSpellValid =
        // All Classes (Classic)
        spellId != 6463 && // Incorrect lock pick skill that was taught to all classes (Rogues still learn correct lock pick skill) (DB Error)
        spellId != 6461 &&  // Incorrect lock pick skill that was taught to all classes (Rogues still learn correct lock pick skill) (DB Error)
        // Warrior (Classic)
        spellId != 877 && // Prevent Warriors from learning Elemental Fury (DB Error)
        // Shaman (Classic)
        spellId != 8385 && // Prevents Shaman from learning Swift Wind spell which is cast onto player as a reward, the spell is not supposed to be learned.
        // Hunter
        spellId != 542  && // Prevents hunter from learning pet skill zzOLDLearn Nature Resistance.
        spellId != 6284 && // Prevents hunter from learning pet skill Pet Hardiness Rank 1
        spellId != 6287 && // Prevents hunter from learning pet skill Pet Hardiness Rank 2
        spellId != 6288 && // Prevents hunter from learning pet skill Pet Hardiness Rank 3
        spellId != 6289 && // Prevents hunter from learning pet skill Pet Hardiness Rank 4
        spellId != 6290 && // Prevents hunter from learning pet skill Pet Hardiness Rank 5
        spellId != 6312 && // Prevents hunter from learning pet skill Pet Aggression Rank 1
        spellId != 6318 && // Prevents hunter from learning pet skill Pet Aggression Rank 2
        spellId != 6319 && // Prevents hunter from learning pet skill Pet Aggression Rank 3
        spellId != 6320 && // Prevents hunter from learning pet skill Pet Aggression Rank 4
        spellId != 6321 && // Prevents hunter from learning pet skill Pet Aggression Rank 5
        spellId != 6329 && // Prevents hunter from learning pet skill Pet Recovery Rank 1
        spellId != 6335 && // Prevents hunter from learning pet skill Pet Recovery Rank 2
        spellId != 6336 && // Prevents hunter from learning pet skill Pet Recovery Rank 3
        spellId != 6337 && // Prevents hunter from learning pet skill Pet Recovery Rank 4
        spellId != 6338 && // Prevents hunter from learning pet skill Pet Recovery Rank 5
        spellId != 6448 && // Prevents hunter from learning pet skill Pet Resistance Rank 1
        spellId != 6450 && // Prevents hunter from learning pet skill Pet Resistance Rank 2
        spellId != 6451 && // Prevents hunter from learning pet skill Pet Resistance Rank 3
        spellId != 6452 && // Prevents hunter from learning pet skill Pet Resistance Rank 4
        spellId != 6453 && // Prevents hunter from learning pet skill Pet Resistance Rank 5
        // Paladin
        spellId != 1973; // Prevents Paladins from learning zzOldHip Shot III.
#elif MANGOSBOT_ONE
    isSpellValid =
        // Paladin
        spellId != 10321 &&       // Prevents Paladin from learning judgment training spell.   
        // Hunter
        spellId != 530;     // Prevents hunter from learning Charm (Possess) (Spell is attached to a pet trainer)
#elif MANGOSBOT_TWO
    isSpellValid =
        // Rogue
        spellId != 1785 && // Rogue Stealth no longer has ranks so remove learning ranks 2-4
        spellId != 1786 && // Rogue Stealth no longer has ranks so remove learning ranks 2-4
        spellId != 1787 && // Rogue Stealth no longer has ranks so remove learning ranks 2-4
        // Druid
        spellId != 6783 && // Druid Prowl no longer has ranks so remove learning ranks 2-3
        spellId != 9913 && // Druid Prowl no longer has ranks so remove learning ranks 2-3
        // DK
        spellId != 51426 && // DK Pestilence doesn't have ranks
        spellId != 51427 && // DK Pestilence doesn't have ranks
        spellId != 51428 && // DK Pestilence doesn't have ranks
        spellId != 51429 && // DK Pestilence doesn't have ranks
        spellId != 49913 && // DK Strangulate doesn't have ranks
        spellId != 49914 && // DK Strangulate doesn't have ranks
        spellId != 49915 && // DK Strangulate doesn't have ranks
        spellId != 49916 && // DK Strangulate doesn't have ranks
        // Mage
        spellId != 526 &&   // Prevents mage from learning shaman Cure Toxins
        spellId != 52127 && // Prevents mage from learning shaman Water Shield Rank 1
        spellId != 52129 && // Prevents mage from learning shaman Water Shield Rank 2
        spellId != 52131 && // Prevents mage from learning shaman Water Shield Rank 3
        spellId != 52134 && // Prevents mage from learning shaman Water Shield Rank 4
        spellId != 52136 && // Prevents mage from learning shaman Water Shield Rank 5
        spellId != 52138 && // Prevents mage from learning shaman Water Shield Rank 6
        spellId != 51730 && // Prevents mage from learning shaman Earthliving Weapon 1
        spellId != 51988 && // Prevents mage from learning shaman Earthliving Weapon 2
        spellId != 51991 && // Prevents mage from learning shaman Earthliving Weapon 3
        spellId != 51992 && // Prevents mage from learning shaman Earthliving Weapon 4
        spellId != 51993 && // Prevents mage from learning shaman Earthliving Weapon 5
        spellId != 51994 && // Prevents mage from learning shaman Earthliving Weapon 6
        spellId != 66842 && // Prevents mage from learning shaman Call of the Elements
        spellId != 66843 && // Prevents mage from learning shaman Call of the Ancestors
        spellId != 66844 && // Prevents mage from learning shaman Call of the Spirits
        spellId != 42748 && // Prevents mage from learning shaman Shadow Axe
        spellId != 2894 &&  // Prevents mage from learning shaman Fire Elemental Totem
        spellId != 51505 && // Prevents mage from learning shaman Lave Burst Rank 1
        spellId != 51514;   // Prevents mage from learning shaman Hex
#endif
    return isSpellValid;
}

bool AutoLearnSpellAction::IsTeachingSpellListedAsSpell(uint32 spellId)
{
    bool isTeachingSpellListedAsSpell = false;
#ifdef MANGOSBOT_ZERO
    isTeachingSpellListedAsSpell =
        spellId == 19318 ||    // Touch of weakness Teaching Spell listed as actual spell
        spellId == 2946  ||    // Devouring Plague Teaching Spell listed as actual spell
        spellId == 19325 ||    // Hex Of Weakness Teaching Spell listed as actual spell
        spellId == 19331 ||    // Shadowguard Teaching Spell listed as actual spell
        spellId == 19338 ||    // Desperate Prayer Teaching Spell listed as actual spell
        spellId == 19345 ||    // Feed Back Teaching Spell listed as actual spell
        spellId == 19337 ||    // Fear Ward Teaching Spell listed as actual spell
        spellId == 19357 ||    // Elune's Grace Teaching Spell listed as actual spell
        spellId == 19350;      // Starshards Teaching Spell listed as actual spell
#endif
        return isTeachingSpellListedAsSpell;
}