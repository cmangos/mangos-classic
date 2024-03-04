/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Server/DBCStores.h"
#include "Policies/Singleton.h"
#include "Log/Log.h"
#include "Util/ProgressBar.h"
#include "Util/Util.h"
#include "Globals/Locales.h"
#include "Globals/SharedDefines.h"
#include "Server/SQLStorages.h"

#include "DBCfmt.h"

#include <map>

typedef std::map<uint32, uint32> AreaIDByAreaFlag;
typedef std::map<uint32, uint32> AreaFlagByMapID;

struct WMOAreaTableTripple
{
    WMOAreaTableTripple(int32 r, int32 a, int32 g) : groupId(g), rootId(r), adtId(a)
    {
    }

    bool operator <(const WMOAreaTableTripple& b) const
    {
        return memcmp(this, &b, sizeof(WMOAreaTableTripple)) < 0;
    }

    // ordered by entropy; that way memcmp will have a minimal medium runtime
    int32 groupId;
    int32 rootId;
    int32 adtId;
};

typedef std::map<WMOAreaTableTripple, std::vector<WMOAreaTableEntry const*>> WMOAreaInfoByTripple;

DBCStorage <AreaTableEntry> sAreaStore(AreaTableEntryfmt);
static AreaIDByAreaFlag sAreaIDByAreaFlag;
static AreaFlagByMapID  sAreaFlagByMapID;                   // for instances without generated *.map files

static WMOAreaInfoByTripple sWMOAreaInfoByTripple;

DBCStorage <AreaTriggerEntry> sAreaTriggerStore(AreaTriggerEntryfmt);
DBCStorage <AuctionHouseEntry> sAuctionHouseStore(AuctionHouseEntryfmt);
DBCStorage <BankBagSlotPricesEntry> sBankBagSlotPricesStore(BankBagSlotPricesEntryfmt);
DBCStorage <CharStartOutfitEntry> sCharStartOutfitStore(CharStartOutfitEntryfmt);
DBCStorage <ChatChannelsEntry> sChatChannelsStore(ChatChannelsEntryfmt);
DBCStorage <CharacterFacialHairStylesEntry> sCharacterFacialHairStylesStore(CharacterFacialHairStylesfmt);
std::unordered_map<uint32, CharacterFacialHairStylesEntry const*> sCharFacialHairMap;
DBCStorage <CharSectionsEntry> sCharSectionsStore(CharSectionsEntryfmt);
std::multimap<uint32, CharSectionsEntry const*> sCharSectionMap;
DBCStorage <ChrClassesEntry> sChrClassesStore(ChrClassesEntryfmt);
DBCStorage <ChrRacesEntry> sChrRacesStore(ChrRacesEntryfmt);
DBCStorage <CinematicCameraEntry> sCinematicCameraStore(CinematicCameraEntryfmt);
DBCStorage <CinematicSequencesEntry> sCinematicSequencesStore(CinematicSequencesEntryfmt);
DBCStorage <CreatureDisplayInfoEntry> sCreatureDisplayInfoStore(CreatureDisplayInfofmt);
DBCStorage <CreatureDisplayInfoExtraEntry> sCreatureDisplayInfoExtraStore(CreatureDisplayInfoExtrafmt);
DBCStorage <CreatureFamilyEntry> sCreatureFamilyStore(CreatureFamilyfmt);
DBCStorage <CreatureModelDataEntry> sCreatureModelDataStore(CreatureModelDatafmt);
DBCStorage <CreatureSpellDataEntry> sCreatureSpellDataStore(CreatureSpellDatafmt);
DBCStorage <CreatureTypeEntry> sCreatureTypeStore(CreatureTypefmt);

DBCStorage <DurabilityQualityEntry> sDurabilityQualityStore(DurabilityQualityfmt);
DBCStorage <DurabilityCostsEntry> sDurabilityCostsStore(DurabilityCostsfmt);

DBCStorage <EmotesEntry> sEmotesStore(EmotesEntryfmt);
DBCStorage <EmotesTextEntry> sEmotesTextStore(EmotesTextEntryfmt);

#ifdef ENABLE_PLAYERBOTS
typedef std::tuple<uint32, uint32, uint32> EmotesTextSoundKey;
static std::map<EmotesTextSoundKey, EmotesTextSoundEntry const*> sEmotesTextSoundMap;
DBCStorage <EmotesTextSoundEntry> sEmotesTextSoundStore(EmotesTextSoundEntryfmt);
#endif

typedef std::map<uint32, SimpleFactionsList> FactionTeamMap;
static FactionTeamMap sFactionTeamMap;
DBCStorage <FactionEntry> sFactionStore(FactionEntryfmt);
DBCStorage <FactionTemplateEntry> sFactionTemplateStore(FactionTemplateEntryfmt);

DBCStorage <GameObjectArtKitEntry> sGameObjectArtKitStore(GameObjectArtKitfmt);
DBCStorage <GameObjectDisplayInfoEntry> sGameObjectDisplayInfoStore(GameObjectDisplayInfofmt);

DBCStorage <GMSurveyCurrentSurveyEntry> sGMSurveyCurrentSurveyStore(GMSurveyCurrentSurveyfmt);
DBCStorage <GMSurveyQuestionsEntry> sGMSurveyQuestionsStore(GMSurveyQuestionsfmt);
DBCStorage <GMSurveyEntry> sGMSurveySurveysStore(GMSurveySurveysfmt);
DBCStorage <GMTicketCategoryEntry> sGMTicketCategoryStore(GMTicketCategoryfmt);

DBCStorage <ItemBagFamilyEntry>           sItemBagFamilyStore(ItemBagFamilyfmt);
DBCStorage <ItemClassEntry>               sItemClassStore(ItemClassfmt);
// DBCStorage <ItemDisplayInfoEntry> sItemDisplayInfoStore(ItemDisplayTemplateEntryfmt); -- not used currently
DBCStorage <ItemRandomPropertiesEntry> sItemRandomPropertiesStore(ItemRandomPropertiesfmt);
DBCStorage <ItemSetEntry> sItemSetStore(ItemSetEntryfmt);
DBCStorage <LiquidTypeEntry> sLiquidTypeStore(LiquidTypefmt);
DBCStorage <LockEntry> sLockStore(LockEntryfmt);

DBCStorage <MailTemplateEntry> sMailTemplateStore(MailTemplateEntryfmt);
DBCStorage <MapEntry> sMapStore(MapEntryfmt);

DBCStorage <QuestSortEntry> sQuestSortStore(QuestSortEntryfmt);

DBCStorage <SkillLineEntry> sSkillLineStore(SkillLinefmt);
DBCStorage <SkillLineAbilityEntry> sSkillLineAbilityStore(SkillLineAbilityfmt);
DBCStorage <SkillRaceClassInfoEntry> sSkillRaceClassInfoStore(SkillRaceClassInfofmt);
DBCStorage <SkillTiersEntry> sSkillTiersStore(SkillTiersfmt);

DBCStorage <SoundEntriesEntry> sSoundEntriesStore(SoundEntriesfmt);

DBCStorage <SpellItemEnchantmentEntry> sSpellItemEnchantmentStore(SpellItemEnchantmentfmt);
SpellCategoryStore sSpellCategoryStore;
ItemSpellCategoryStore sItemSpellCategoryStore;
PetFamilySpellsStore sPetFamilySpellsStore;

DBCStorage <SpellCastTimesEntry> sSpellCastTimesStore(SpellCastTimefmt);
DBCStorage <SpellDurationEntry> sSpellDurationStore(SpellDurationfmt);
DBCStorage <SpellFocusObjectEntry> sSpellFocusObjectStore(SpellFocusObjectfmt);
DBCStorage <SpellRadiusEntry> sSpellRadiusStore(SpellRadiusfmt);
DBCStorage <SpellRangeEntry> sSpellRangeStore(SpellRangefmt);
DBCStorage <SpellShapeshiftFormEntry> sSpellShapeshiftFormStore(SpellShapeshiftfmt);
DBCStorage <StableSlotPricesEntry> sStableSlotPricesStore(StableSlotPricesfmt);
// DBCStorage <SummonPropertiesEntry> sSummonPropertiesStore(SummonPropertiesfmt);
DBCStorage <TalentEntry> sTalentStore(TalentEntryfmt);
TalentSpellPosMap sTalentSpellPosMap;
DBCStorage <TalentTabEntry> sTalentTabStore(TalentTabEntryfmt);

// store absolute bit position for first rank for talent inspect
typedef std::map<uint32, uint32> TalentInspectMap;
static TalentInspectMap sTalentPosInInspect;
static TalentInspectMap sTalentTabSizeInInspect;
static uint32 sTalentTabPages[12/*MAX_CLASSES*/][3];

DBCStorage <TaxiNodesEntry> sTaxiNodesStore(TaxiNodesEntryfmt);
TaxiMask sTaxiNodesMask;

// DBC used only for initialization sTaxiPathSetBySource at startup.
TaxiPathSetBySource sTaxiPathSetBySource;
DBCStorage <TaxiPathEntry> sTaxiPathStore(TaxiPathEntryfmt);

// DBC store data but sTaxiPathNodesByPath used for fast access to entries (it's not owner pointed data).
TaxiPathNodesByPath sTaxiPathNodesByPath;
static DBCStorage <TaxiPathNodeEntry> sTaxiPathNodeStore(TaxiPathNodeEntryfmt);

DBCStorage <TransportAnimationEntry> sTransportAnimationStore(TransportAnimationfmt);

DBCStorage <WMOAreaTableEntry>  sWMOAreaTableStore(WMOAreaTableEntryfmt);
DBCStorage <WorldMapAreaEntry>  sWorldMapAreaStore(WorldMapAreaEntryfmt);
// DBCStorage <WorldMapOverlayEntry> sWorldMapOverlayStore(WorldMapOverlayEntryfmt);

typedef std::list<std::string> StoreProblemList;

bool IsAcceptableClientBuild(uint32 build)
{
    int accepted_versions[] = EXPECTED_MANGOSD_CLIENT_BUILD;
    for (int i = 0; accepted_versions[i]; ++i)
        if (int(build) == accepted_versions[i])
            return true;

    return false;
}

std::string AcceptableClientBuildsListStr()
{
    std::ostringstream data;
    int accepted_versions[] = EXPECTED_MANGOSD_CLIENT_BUILD;
    for (int i = 0; accepted_versions[i]; ++i)
        data << accepted_versions[i] << " ";
    return data.str();
}

static bool LoadDBC_assert_print(uint32 fsize, uint32 rsize, const std::string& filename)
{
    sLog.outError("Size of '%s' setted by format string (%u) not equal size of C++ structure (%u).", filename.c_str(), fsize, rsize);

    // ASSERT must fail after function call
    return false;
}

template<class T>
inline void LoadDBC(uint32& availableDbcLocales, BarGoLink& bar, StoreProblemList& errlist, DBCStorage<T>& storage, const std::string& dbc_path, const std::string& filename)
{
    // compatibility format and C++ structure sizes
    MANGOS_ASSERT(DBCFileLoader::GetFormatRecordSize(storage.GetFormat()) == sizeof(T) || LoadDBC_assert_print(DBCFileLoader::GetFormatRecordSize(storage.GetFormat()), sizeof(T), filename));

    std::string dbc_filename = dbc_path + filename;
    if (storage.Load(dbc_filename.c_str()))
    {
        bar.step();
        for (uint8 i = 0; fullLocaleNameList[i].name; ++i)
        {
            if (!(availableDbcLocales & (1 << i)))
                continue;

            std::string dbc_filename_loc = dbc_path + fullLocaleNameList[i].name + "/" + filename;
            if (!storage.LoadStringsFrom(dbc_filename_loc.c_str()))
                availableDbcLocales &= ~(1 << i);           // mark as not available for speedup next checks
        }
    }
    else
    {
        // sort problematic dbc to (1) non compatible and (2) nonexistent
        FILE* f = fopen(dbc_filename.c_str(), "rb");
        if (f)
        {
            char buf[100];
            snprintf(buf, 100, " (exist, but have %u fields instead " SIZEFMTD ") Wrong client version DBC file?", storage.GetFieldCount(), strlen(storage.GetFormat()));
            errlist.push_back(dbc_filename + buf);
            fclose(f);
        }
        else
            errlist.push_back(dbc_filename);
    }
}

void LoadDBCStores(const std::string& dataPath)
{
    std::string dbcPath = dataPath + "dbc/";

    if (!MaNGOS::Filesystem::exists(dbcPath))
    {
        sLog.outError("DBC directory does not exist: %s", dataPath.c_str());
        Log::WaitBeforeContinueIfNeed();
        exit(1);
    }

    const uint32 DBCFilesCount = 52;

    BarGoLink bar(DBCFilesCount);

    StoreProblemList bad_dbc_files;

    // bitmask for index of fullLocaleNameList
    uint32 availableDbcLocales = 0xFFFFFFFF;

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sAreaStore,                dbcPath, "AreaTable.dbc");

    // must be after sAreaStore loading
    for (uint32 i = 1; i <= sAreaStore.GetNumRows(); ++i)   // areaid numbered from 1
    {
        if (AreaTableEntry const* area = sAreaStore.LookupEntry(i))
        {
            // fill AreaId->DBC records
            sAreaIDByAreaFlag.insert(AreaIDByAreaFlag::value_type(uint16(area->exploreFlag), area->ID));

            // fill MapId->DBC records ( skip sub zones and continents )
            if (area->zone == 0 && area->mapid != 0 && area->mapid != 1)
                sAreaFlagByMapID.insert(AreaFlagByMapID::value_type(area->mapid, area->exploreFlag));
        }
    }

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sAreaTriggerStore,         dbcPath, "AreaTrigger.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sAuctionHouseStore,        dbcPath, "AuctionHouse.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sBankBagSlotPricesStore,   dbcPath, "BankBagSlotPrices.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sCharStartOutfitStore,     dbcPath, "CharStartOutfit.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sChatChannelsStore,        dbcPath, "ChatChannels.dbc");

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sCharacterFacialHairStylesStore, dbcPath, "CharacterFacialHairStyles.dbc");
    for (uint32 i = 0; i < sCharacterFacialHairStylesStore.GetNumRows(); ++i)
        if (CharacterFacialHairStylesEntry const* entry = sCharacterFacialHairStylesStore.LookupEntry(i))
            if (entry->RaceID && ((1 << (entry->RaceID - 1)) & RACEMASK_ALL_PLAYABLE) != 0) // ignore nonplayable races
                sCharFacialHairMap.insert({ entry->RaceID | (entry->SexID << 8) | (entry->VariationID << 16), entry });

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sCharSectionsStore, dbcPath, "CharSections.dbc");
    for (uint32 i = 0; i < sCharSectionsStore.GetNumRows(); ++i)
        if (CharSectionsEntry const* entry = sCharSectionsStore.LookupEntry(i))
            if (entry->Race && ((1 << (entry->Race - 1)) & RACEMASK_ALL_PLAYABLE) != 0) //ignore Nonplayable races
                sCharSectionMap.emplace(uint8(entry->BaseSection) | (uint8(entry->Gender) << 8) | (uint8(entry->Race) << 16), entry);

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sChrClassesStore,          dbcPath, "ChrClasses.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sChrRacesStore,            dbcPath, "ChrRaces.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sCinematicCameraStore,     dbcPath, "CinematicCamera.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sCinematicSequencesStore,  dbcPath, "CinematicSequences.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sCreatureDisplayInfoStore, dbcPath, "CreatureDisplayInfo.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sCreatureDisplayInfoExtraStore, dbcPath, "CreatureDisplayInfoExtra.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sCreatureFamilyStore,      dbcPath, "CreatureFamily.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sCreatureModelDataStore,   dbcPath, "CreatureModelData.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sCreatureSpellDataStore,   dbcPath, "CreatureSpellData.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sCreatureTypeStore,        dbcPath, "CreatureType.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sDurabilityCostsStore,     dbcPath, "DurabilityCosts.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sDurabilityQualityStore,   dbcPath, "DurabilityQuality.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sEmotesStore,              dbcPath, "Emotes.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sEmotesTextStore,          dbcPath, "EmotesText.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sFactionStore,             dbcPath, "Faction.dbc");
    for (uint32 i = 0; i < sFactionStore.GetNumRows(); ++i)
    {
        FactionEntry const* faction = sFactionStore.LookupEntry(i);
        if (faction && faction->team)
        {
            SimpleFactionsList& flist = sFactionTeamMap[faction->team];
            flist.push_back(i);
        }
    }

#ifdef ENABLE_PLAYERBOTS
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sEmotesTextSoundStore, dbcPath, "EmotesTextSound.dbc");
    for (uint32 i = 0; i < sEmotesTextSoundStore.GetNumRows(); ++i)
    {
        if (EmotesTextSoundEntry const* entry = sEmotesTextSoundStore.LookupEntry(i))
        {
            sEmotesTextSoundMap[EmotesTextSoundKey(entry->EmotesTextId, entry->RaceId, entry->SexId)] = entry;
        }
    }
#endif

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sFactionTemplateStore,     dbcPath, "FactionTemplate.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sGameObjectArtKitStore,    dbcPath, "GameObjectArtKit.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sGameObjectDisplayInfoStore, dbcPath, "GameObjectDisplayInfo.dbc");

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sGMSurveyCurrentSurveyStore,  dbcPath, "GMSurveyCurrentSurvey.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sGMSurveyQuestionsStore,  dbcPath, "GMSurveyQuestions.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sGMSurveySurveysStore,  dbcPath, "GMSurveySurveys.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sGMTicketCategoryStore, dbcPath, "GMTicketCategory.dbc");

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sItemBagFamilyStore,       dbcPath, "ItemBagFamily.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sItemClassStore,           dbcPath, "ItemClass.dbc");
    // LoadDBC(availableDbcLocales,bar,bad_dbc_files,sItemDisplayInfoStore,     dbcPath,"ItemDisplayInfo.dbc");     -- not used currently
    // LoadDBC(availableDbcLocales,bar,bad_dbc_files,sItemCondExtCostsStore,    dbcPath,"ItemCondExtCosts.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sItemRandomPropertiesStore, dbcPath, "ItemRandomProperties.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sItemSetStore,             dbcPath, "ItemSet.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sLiquidTypeStore,          dbcPath, "LiquidType.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sLockStore,                dbcPath, "Lock.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sMailTemplateStore,        dbcPath, "MailTemplate.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sMapStore,                 dbcPath, "Map.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sQuestSortStore,           dbcPath, "QuestSort.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSkillLineStore,           dbcPath, "SkillLine.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSkillLineAbilityStore,    dbcPath, "SkillLineAbility.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSkillRaceClassInfoStore,  dbcPath, "SkillRaceClassInfo.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSkillTiersStore,          dbcPath, "SkillTiers.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSoundEntriesStore,        dbcPath, "SoundEntries.dbc");

    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j);

        if (!skillLine)
            continue;

        std::set<uint32> wrongIds = { 4073, 12749, 19804, 13258, 13166 };
        if (wrongIds.find(skillLine->spellId) != wrongIds.end())
        {
            // repairs entry for netherstorm - should be moved to SQL
            auto fixedEntry = new SkillLineAbilityEntry(*skillLine);
            fixedEntry->learnOnGetSkill = 0;
            sSkillLineAbilityStore.EraseEntry(j);
            sSkillLineAbilityStore.InsertEntry(fixedEntry, j);
            skillLine = fixedEntry;
        }

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(skillLine->spellId);
        if (spellInfo && (spellInfo->Attributes & (SPELL_ATTR_IS_ABILITY | SPELL_ATTR_PASSIVE | SPELL_ATTR_DO_NOT_DISPLAY | SPELL_ATTR_DO_NOT_LOG)) == (SPELL_ATTR_IS_ABILITY | SPELL_ATTR_PASSIVE | SPELL_ATTR_DO_NOT_DISPLAY | SPELL_ATTR_DO_NOT_LOG))
        {
            for (unsigned int i = 1; i < sCreatureFamilyStore.GetNumRows(); ++i)
            {
                CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(i);
                if (!cFamily)
                    continue;

                if (skillLine->skillId != cFamily->skillLine[0] && skillLine->skillId != cFamily->skillLine[1])
                    continue;

                sPetFamilySpellsStore[i].insert(spellInfo->Id);
            }
        }
    }

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSpellCastTimesStore,      dbcPath, "SpellCastTimes.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSpellDurationStore,       dbcPath, "SpellDuration.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSpellFocusObjectStore,    dbcPath, "SpellFocusObject.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSpellItemEnchantmentStore, dbcPath, "SpellItemEnchantment.dbc");
    //for (uint32 i = 0; i < sSpellItemEnchantmentStore.GetNumRows(); ++i)
    //{
    //    SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(i);
    //    if (!enchantEntry)
    //        continue;
    //    for (uint32 k = 0; k < 3; ++k)
    //    {
    //        if (enchantEntry->spellid[k])
    //            if (!sSpellTemplate.LookupEntry<SpellEntry>(enchantEntry->spellid[k]))
    //                sLog.outErrorDb("Spell ID %u found in spell item enchant %u does not exist.", enchantEntry->spellid[k], i);
    //    }
    //}
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSpellRadiusStore,         dbcPath, "SpellRadius.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSpellRangeStore,          dbcPath, "SpellRange.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSpellShapeshiftFormStore, dbcPath, "SpellShapeshiftForm.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sStableSlotPricesStore,    dbcPath, "StableSlotPrices.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sTalentStore,              dbcPath, "Talent.dbc");

    // create talent spells set
    for (unsigned int i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo) continue;
        for (int j = 0; j < 5; ++j)
            if (talentInfo->RankID[j])
                sTalentSpellPosMap[talentInfo->RankID[j]] = TalentSpellPos(i, j);
    }

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sTalentTabStore,           dbcPath, "TalentTab.dbc");

    // prepare fast data access to bit pos of talent ranks for use at inspecting
    {
        // fill table by amount of talent ranks and fill sTalentTabBitSizeInInspect
        // store in with (row,col,talent)->size key for correct sorting by (row,col)
        typedef std::map<uint32, uint32> TalentBitSize;
        TalentBitSize sTalentBitSize;
        for (uint32 i = 1; i < sTalentStore.GetNumRows(); ++i)
        {
            TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
            if (!talentInfo) continue;

            TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
            if (!talentTabInfo)
                continue;

            // find talent rank
            uint32 curtalent_maxrank = 0;
            for (uint32 k = 5; k > 0; --k)
            {
                if (talentInfo->RankID[k - 1])
                {
                    curtalent_maxrank = k;
                    break;
                }
            }

            sTalentBitSize[(talentInfo->Row << 24) + (talentInfo->Col << 16) + talentInfo->TalentID] = curtalent_maxrank;
            sTalentTabSizeInInspect[talentInfo->TalentTab] += curtalent_maxrank;
        }

        // now have all max ranks (and then bit amount used for store talent ranks in inspect)
        for (uint32 talentTabId = 1; talentTabId < sTalentTabStore.GetNumRows(); ++talentTabId)
        {
            TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentTabId);
            if (!talentTabInfo)
                continue;

            // prevent memory corruption; otherwise cls will become 12 below
            if ((talentTabInfo->ClassMask & CLASSMASK_ALL_PLAYABLE) == 0)
                continue;

            // store class talent tab pages
            uint32 cls = 1;
            for (uint32 m = 1; !(m & talentTabInfo->ClassMask) && cls < MAX_CLASSES; m <<= 1, ++cls) {}

            sTalentTabPages[cls][talentTabInfo->tabpage] = talentTabId;

            // add total amount bits for first rank starting from talent tab first talent rank pos.
            uint32 pos = 0;
            for (TalentBitSize::iterator itr = sTalentBitSize.begin(); itr != sTalentBitSize.end(); ++itr)
            {
                uint32 talentId = itr->first & 0xFFFF;
                TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentId);
                if (!talentInfo)
                    continue;

                if (talentInfo->TalentTab != talentTabId)
                    continue;

                sTalentPosInInspect[talentId] = pos;
                pos += itr->second;
            }
        }
    }

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sTaxiNodesStore,           dbcPath, "TaxiNodes.dbc");

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sTaxiPathStore,            dbcPath, "TaxiPath.dbc");
    for (uint32 i = 1; i < sTaxiPathStore.GetNumRows(); ++i)
        if (TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i))
            sTaxiPathSetBySource[entry->from][entry->to] = TaxiPathBySourceAndDestination(entry->ID, entry->price);
    uint32 pathCount = sTaxiPathStore.GetNumRows();

    //## TaxiPathNode.dbc ## Loaded only for initialization different structures
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sTaxiPathNodeStore,        dbcPath, "TaxiPathNode.dbc");
    // Calculate path nodes count
    std::vector<uint32> pathLength;
    pathLength.resize(pathCount);                           // 0 and some other indexes not used
    for (uint32 i = 1; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        if (TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
        {
            if (pathLength[entry->path] < entry->index + 1)
                pathLength[entry->path] = entry->index + 1;
        }
    // Set path length
    sTaxiPathNodesByPath.resize(pathCount);                 // 0 and some other indexes not used
    for (uint32 i = 1; i < sTaxiPathNodesByPath.size(); ++i)
        sTaxiPathNodesByPath[i].resize(pathLength[i]);
    // fill data (pointers to sTaxiPathNodeStore elements
    for (uint32 i = 1; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        if (TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
            sTaxiPathNodesByPath[entry->path].set(entry->index, entry);

    // Initialize global taxinodes mask
    // include existing nodes that have at least single not spell base (scripted) path
    {
        std::set<uint32> spellPaths;
        for (uint32 i = 1; i < sSpellTemplate.GetMaxEntry(); ++i)
            if (SpellEntry const* sInfo = sSpellTemplate.LookupEntry<SpellEntry>(i))
                for (int j = 0; j < MAX_EFFECT_INDEX; ++j)
                    if (sInfo->Effect[j] == 123 /*SPELL_EFFECT_SEND_TAXI*/)
                        spellPaths.insert(sInfo->EffectMiscValue[j]);

        memset(sTaxiNodesMask, 0, sizeof(sTaxiNodesMask));
        for (uint32 i = 1; i < sTaxiNodesStore.GetNumRows(); ++i)
        {
            TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(i);
            if (!node)
                continue;

            TaxiPathSetBySource::const_iterator src_i = sTaxiPathSetBySource.find(i);
            if (src_i != sTaxiPathSetBySource.end() && !src_i->second.empty())
            {
                bool ok = false;
                for (const auto& dest_i : src_i->second)
                {
                    // not spell path
                    if (spellPaths.find(dest_i.second.ID) == spellPaths.end())
                    {
                        ok = true;
                        break;
                    }
                }

                if (!ok)
                    continue;
            }

            // valid taxi network node
            uint8  field   = (uint8)((i - 1) / 32);
            uint32 submask = 1 << ((i - 1) % 32);
            sTaxiNodesMask[field] |= submask;
        }
    }

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sTransportAnimationStore,  dbcPath, "TransportAnimation.dbc");

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sWorldMapAreaStore,        dbcPath, "WorldMapArea.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sWMOAreaTableStore,        dbcPath, "WMOAreaTable.dbc");
    for (uint32 i = 0; i < sWMOAreaTableStore.GetNumRows(); ++i)
    {
        if (WMOAreaTableEntry const* entry = sWMOAreaTableStore.LookupEntry(i))
        {
            sWMOAreaInfoByTripple[WMOAreaTableTripple(entry->rootId, entry->adtId, entry->groupId)].push_back(entry);
        }
    }
    // LoadDBC(availableDbcLocales,bar,bad_dbc_files,sWorldMapOverlayStore,     dbcPath,"WorldMapOverlay.dbc");
    // LoadDBC(availableDbcLocales, bar, bad_dbc_files, sWorldSafeLocsStore,       dbcPath, "WorldSafeLocs.dbc");

    // error checks
    if (bad_dbc_files.size() >= DBCFilesCount)
    {
        sLog.outError("\nIncorrect DataDir value in mangosd.conf or ALL required *.dbc files (%d) not found by path: %sdbc", DBCFilesCount, dataPath.c_str());
        Log::WaitBeforeContinueIfNeed();
        exit(1);
    }
    if (!bad_dbc_files.empty())
    {
        std::string str;
        for (auto& bad_dbc_file : bad_dbc_files)
            str += bad_dbc_file + "\n";

        sLog.outError("\nSome required *.dbc files (%u from %d) not found or not compatible:\n%s", (uint32)bad_dbc_files.size(), DBCFilesCount, str.c_str());
        Log::WaitBeforeContinueIfNeed();
        exit(1);
    }

    // Check loaded DBC files proper version
    if (!sSkillLineAbilityStore.LookupEntry(15030) ||
            !sMapStore.LookupEntry(533)                ||
            !sAreaStore.LookupEntry(3486))
    {
        sLog.outError("\nYou have _outdated_ DBC files. Please re-extract DBC files for one from client build: %s", AcceptableClientBuildsListStr().c_str());
        Log::WaitBeforeContinueIfNeed();
        exit(1);
    }

    sLog.outString(">> Initialized %d data stores", DBCFilesCount);
    sLog.outString();
}

SimpleFactionsList const* GetFactionTeamList(uint32 faction)
{
    FactionTeamMap::const_iterator itr = sFactionTeamMap.find(faction);
    if (itr == sFactionTeamMap.end())
        return nullptr;
    return &itr->second;
}

char const* GetPetName(uint32 petfamily, uint32 dbclang)
{
    if (!petfamily)
        return nullptr;
    CreatureFamilyEntry const* pet_family = sCreatureFamilyStore.LookupEntry(petfamily);
    if (!pet_family)
        return nullptr;
    return pet_family->Name[dbclang] ? pet_family->Name[dbclang] : nullptr;
}

TalentSpellPos const* GetTalentSpellPos(uint32 spellId)
{
    TalentSpellPosMap::const_iterator itr = sTalentSpellPosMap.find(spellId);
    if (itr == sTalentSpellPosMap.end())
        return nullptr;

    return &itr->second;
}

uint32 GetTalentSpellCost(TalentSpellPos const* pos)
{
    if (pos)
        return pos->rank + 1;

    return 0;
}

uint32 GetTalentSpellCost(uint32 spellId)
{
    return GetTalentSpellCost(GetTalentSpellPos(spellId));
}

int32 GetAreaFlagByAreaID(uint32 area_id)
{
    AreaTableEntry const* AreaEntry = sAreaStore.LookupEntry(area_id);
    if (!AreaEntry)
        return -1;

    return AreaEntry->exploreFlag;
}

uint32 GetAreaIdByLocalizedName(const std::string& name)
{
    for (uint32 i = 0; i <= sAreaStore.GetNumRows(); i++)
    {
        if (AreaTableEntry const* AreaEntry = sAreaStore.LookupEntry(i))
        {
            for (uint32 i = 0; i < MAX_LOCALE; ++i)
            {
                std::string area_name(AreaEntry->area_name[i]);
                if (area_name.size() > 0 && name.find(" - " + area_name) != std::string::npos)
                {
                    return AreaEntry->ID;
                }
            }
        }
    }
    return 0;
}

std::vector<WMOAreaTableEntry const*>& GetWMOAreaTableEntriesByTripple(int32 rootid, int32 adtid, int32 groupid)
{
    return sWMOAreaInfoByTripple[WMOAreaTableTripple(rootid, adtid, groupid)];
}

AreaTableEntry const* GetAreaEntryByAreaID(uint32 area_id)
{
    return sAreaStore.LookupEntry(area_id);
}

AreaTableEntry const* GetAreaEntryByAreaFlagAndMap(uint32 area_flag, uint32 map_id)
{
    // 1.12.1 areatable have duplicates for areaflag
    AreaTableEntry const* aEntry = nullptr;
    for (uint32 i = 0; i <= sAreaStore.GetNumRows(); i++)
    {
        if (area_flag != 0)
        {
            if (AreaTableEntry const* AreaEntry = sAreaStore.LookupEntry(i))
            {
                if (AreaEntry->exploreFlag == area_flag)
                {
                    // area_flag found but it lets test map_id too
                    if (AreaEntry->mapid == map_id)
                        return AreaEntry; // area_flag and map_id are ok so we can return value
                    // not same map_id so we store this entry and continue searching another better one
                    aEntry = AreaEntry;
                }
            }
        }
    }

    if (aEntry)
        return aEntry; // return last entry found if exist (not same map_id but it seem ok in some places)

    if (MapEntry const* mapEntry = sMapStore.LookupEntry(map_id))
        return GetAreaEntryByAreaID(mapEntry->linked_zone);

    return nullptr;
}

uint32 GetAreaFlagByMapId(uint32 mapid)
{
    AreaFlagByMapID::iterator i = sAreaFlagByMapID.find(mapid);
    if (i == sAreaFlagByMapID.end())
        return 0;
    return i->second;
}

CharacterFacialHairStylesEntry const* GetCharFacialHairEntry(uint8 race, uint8 gender, uint8 facialHairId)
{
    auto itr = sCharFacialHairMap.find(uint32(race) | uint32(gender << 8) | uint32(facialHairId << 16));
    if (itr == sCharFacialHairMap.end())
        return nullptr;

    return itr->second;
}

CharSectionsEntry const* GetCharSectionEntry(uint8 race, CharSectionType genType, uint8 gender, uint8 type, uint8 color)
{
    auto eqr = sCharSectionMap.equal_range(uint32(genType) | uint32(gender << 8) | uint32(race << 16));
    for (auto itr = eqr.first; itr != eqr.second; ++itr)
    {
        if (itr->second->VariationIndex == type && itr->second->ColorIndex == color && !itr->second->HasFlag(SECTION_FLAG_UNAVAILABLE))
            return itr->second;
    }

    return nullptr;
}

ChatChannelsEntry const* GetChatChannelsEntryFor(const std::string& name, uint32 channel_id/* = 0*/)
{
    std::wstring wname;

    Utf8toWStr(name, wname);

    if (!channel_id && wname.empty())
        return nullptr;

    // not sorted, numbering index from 0
    for (uint32 i = 0; i < sChatChannelsStore.GetNumRows(); ++i)
    {
        if (ChatChannelsEntry const* entry = sChatChannelsStore.LookupEntry(i))
        {
            std::wstring wpattern;

            // try to match by name first, avoid creating custom channels with same name
            if (!wname.empty())
            {
                for (uint32 i = 0; i < MAX_LOCALE; ++i)
                {
                    Utf8toWStr(entry->pattern[i], wpattern);

                    if (wpattern.empty())
                        continue;

                    size_t argpos = wpattern.find(L"%s");

                    // formatting arg present: strip and attempt partial match
                    if (argpos != std::wstring::npos)
                    {
                        wpattern.replace(argpos, 2, L"");

                        if (wname.find(wpattern) != std::wstring::npos)
                            return entry;
                    }
                    // attempt full match
                    else if (wname.compare(wpattern) == 0)
                        return entry;
                }
            }

            // name still not found, but channel id is provided: possibly no dbc data for client locale
            if (channel_id && channel_id == entry->ChannelID)
                return entry;
        }
    }
    return nullptr;
}

/*[-ZERO]
bool IsTotemCategoryCompatiableWith(uint32 itemTotemCategoryId, uint32 requiredTotemCategoryId)
{
    if(requiredTotemCategoryId==0)
        return true;
    if(itemTotemCategoryId==0)
        return false;

    TotemCategoryEntry const* itemEntry = sTotemCategoryStore.LookupEntry(itemTotemCategoryId);
    if(!itemEntry)
        return false;
    TotemCategoryEntry const* reqEntry = sTotemCategoryStore.LookupEntry(requiredTotemCategoryId);
    if(!reqEntry)
        return false;

    if(itemEntry->categoryType!=reqEntry->categoryType)
        return false;

    return (itemEntry->categoryMask & reqEntry->categoryMask)==reqEntry->categoryMask;
}
*/

bool MapCoordinateVsZoneCheck(float x, float y, uint32 mapid, uint32 zone)
{
    WorldMapAreaEntry const* WmaEntry = sWorldMapAreaStore.LookupEntry(zone);
    if (WmaEntry && mapid == WmaEntry->map_id &&
            x >= WmaEntry->x2 && x <= WmaEntry->x1 && y >= WmaEntry->y2 && y <= WmaEntry->y1)
        return true;
    return false;
}

bool Zone2MapCoordinates(float& x, float& y, uint32 zone)
{
    WorldMapAreaEntry const* maEntry = sWorldMapAreaStore.LookupEntry(zone);

    // if not listed then map coordinates (instance)
    if (!maEntry || maEntry->x2 == maEntry->x1 || maEntry->y2 == maEntry->y1)
        return false;

    std::swap(x, y);                                        // at client map coords swapped
    x = x * ((maEntry->x2 - maEntry->x1) / 100) + maEntry->x1;
    y = y * ((maEntry->y2 - maEntry->y1) / 100) + maEntry->y1; // client y coord from top to down

    return true;
}

bool Map2ZoneCoordinates(float& x, float& y, uint32 zone)
{
    WorldMapAreaEntry const* maEntry = sWorldMapAreaStore.LookupEntry(zone);

    // if not listed then map coordinates (instance)
    if (!maEntry || maEntry->x2 == maEntry->x1 || maEntry->y2 == maEntry->y1)
        return false;

    x = (x - maEntry->x1) / ((maEntry->x2 - maEntry->x1) / 100);
    y = (y - maEntry->y1) / ((maEntry->y2 - maEntry->y1) / 100); // client y coord from top to down
    std::swap(x, y);                                        // client have map coords swapped

    return true;
}

uint32 GetTalentInspectBitPosInTab(uint32 talentId)
{
    TalentInspectMap::const_iterator itr = sTalentPosInInspect.find(talentId);
    if (itr == sTalentPosInInspect.end())
        return 0;

    return itr->second;
}

uint32 GetTalentTabInspectBitSize(uint32 talentTabId)
{
    TalentInspectMap::const_iterator itr = sTalentTabSizeInInspect.find(talentTabId);
    if (itr == sTalentTabSizeInInspect.end())
        return 0;

    return itr->second;
}

uint32 const* GetTalentTabPages(uint32 cls)
{
    return sTalentTabPages[cls];
}

bool IsPointInAreaTriggerZone(AreaTriggerEntry const* atEntry, uint32 mapid, float x, float y, float z, float delta)
{
    if (mapid != atEntry->mapid)
        return false;

    if (atEntry->radius > 0)
    {
        // if we have radius check it
        float dist2 = (x - atEntry->x) * (x - atEntry->x) + (y - atEntry->y) * (y - atEntry->y) + (z - atEntry->z) * (z - atEntry->z);
        if (dist2 > (atEntry->radius + delta) * (atEntry->radius + delta))
            return false;
    }
    else
    {
        // we have only extent

        // rotate the players position instead of rotating the whole cube, that way we can make a simplified
        // is-in-cube check and we have to calculate only one point instead of 4

        // 2PI = 360, keep in mind that ingame orientation is counter-clockwise
        double rotation = 2 * M_PI - atEntry->box_orientation;
        double sinVal = sin(rotation);
        double cosVal = cos(rotation);

        float playerBoxDistX = x - atEntry->x;
        float playerBoxDistY = y - atEntry->y;

        float rotPlayerX = float(atEntry->x + playerBoxDistX * cosVal - playerBoxDistY * sinVal);
        float rotPlayerY = float(atEntry->y + playerBoxDistY * cosVal + playerBoxDistX * sinVal);

        // box edges are parallel to coordiante axis, so we can treat every dimension independently :D
        float dz = z - atEntry->z;
        float dx = rotPlayerX - atEntry->x;
        float dy = rotPlayerY - atEntry->y;
        if ((fabs(dx) > atEntry->box_x / 2 + delta) ||
                (fabs(dy) > atEntry->box_y / 2 + delta) ||
                (fabs(dz) > atEntry->box_z / 2 + delta))
        {
            return false;
        }
    }

    return true;
}

uint32 GetCreatureModelRace(uint32 model_id)
{
    CreatureDisplayInfoEntry const* displayEntry = sCreatureDisplayInfoStore.LookupEntry(model_id);
    if (!displayEntry)
        return 0;
    CreatureDisplayInfoExtraEntry const* extraEntry = sCreatureDisplayInfoExtraStore.LookupEntry(displayEntry->ExtendedDisplayInfoID);
    return extraEntry ? extraEntry->Race : 0;
}

// script support functions
DBCStorage <SoundEntriesEntry>  const* GetSoundEntriesStore()   { return &sSoundEntriesStore;   }
DBCStorage <SpellRangeEntry>    const* GetSpellRangeStore()     { return &sSpellRangeStore;     }
DBCStorage <FactionEntry>       const* GetFactionStore()        { return &sFactionStore;        }
DBCStorage <CreatureDisplayInfoEntry> const* GetCreatureDisplayStore() { return &sCreatureDisplayInfoStore; }
DBCStorage <EmotesEntry>        const* GetEmotesStore()         { return &sEmotesStore;         }
DBCStorage <EmotesTextEntry>    const* GetEmotesTextStore()     { return &sEmotesTextStore;     }

#ifdef ENABLE_PLAYERBOTS
EmotesTextSoundEntry const* FindTextSoundEmoteFor(uint32 emote, uint32 race, uint32 gender)
{
    auto itr = sEmotesTextSoundMap.find(EmotesTextSoundKey(emote, race, gender));
    return itr != sEmotesTextSoundMap.end() ? itr->second : nullptr;
}
#endif