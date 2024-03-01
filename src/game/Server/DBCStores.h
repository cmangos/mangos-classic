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

#ifndef MANGOS_DBCSTORES_H
#define MANGOS_DBCSTORES_H

#include "Common.h"
#include "Database/DBCStore.h"
#include "Server/DBCStructure.h"

#include <list>
#include <vector>

bool IsAcceptableClientBuild(uint32 build);
std::string AcceptableClientBuildsListStr();

typedef std::list<uint32> SimpleFactionsList;

SimpleFactionsList const* GetFactionTeamList(uint32 faction);
char const* GetPetName(uint32 petfamily, uint32 dbclang);
uint32 GetTalentSpellCost(uint32 spellId);
uint32 GetTalentSpellCost(TalentSpellPos const* pos);
TalentSpellPos const* GetTalentSpellPos(uint32 spellId);

int32 GetAreaFlagByAreaID(uint32 area_id);                  // -1 if not found
uint32 GetAreaFlagByMapId(uint32 mapid);

std::vector<WMOAreaTableEntry const*>& GetWMOAreaTableEntriesByTripple(int32 rootid, int32 adtid, int32 groupid);

AreaTableEntry const* GetAreaEntryByAreaID(uint32 area_id);
AreaTableEntry const* GetAreaEntryByAreaFlagAndMap(uint32 area_flag, uint32 map_id);

uint32 GetVirtualMapForMapAndZone(uint32 mapid, uint32 zoneId);

ChatChannelsEntry const* GetChatChannelsEntryFor(const std::string& name, uint32 channel_id = 0);

CharacterFacialHairStylesEntry const* GetCharFacialHairEntry(uint8 race, uint8 gender, uint8 facialHairId);
CharSectionsEntry const* GetCharSectionEntry(uint8 race, CharSectionType genType, uint8 gender, uint8 type, uint8 color);

// [-ZERO] bool IsTotemCategoryCompatiableWith(uint32 itemTotemCategoryId, uint32 requiredTotemCategoryId);

bool MapCoordinateVsZoneCheck(float x, float y, uint32 mapid, uint32 zone);
bool Zone2MapCoordinates(float& x, float& y, uint32 zone);
bool Map2ZoneCoordinates(float& x, float& y, uint32 zone);

uint32 GetTalentInspectBitPosInTab(uint32 talentId);
uint32 GetTalentTabInspectBitSize(uint32 talentTabId);
uint32 const* /*[3]*/ GetTalentTabPages(uint32 cls);

bool IsPointInAreaTriggerZone(AreaTriggerEntry const* atEntry, uint32 mapid, float x, float y, float z, float delta = 0.0f);

uint32 GetCreatureModelRace(uint32 model_id);

#ifdef ENABLE_PLAYERBOTS
EmotesTextSoundEntry const* FindTextSoundEmoteFor(uint32 emote, uint32 race, uint32 gender);
CharSectionsEntry const* GetCharSectionEntry(uint8 race, CharSectionType genType, uint8 gender, uint8 type, uint8 color);
typedef std::multimap<uint32, CharSectionsEntry const*> CharSectionsMap;
extern CharSectionsMap sCharSectionMap;
#endif

extern DBCStorage <AreaTableEntry>               sAreaStore;// recommend access using functions
extern DBCStorage <AreaTriggerEntry>             sAreaTriggerStore;
extern DBCStorage <AuctionHouseEntry>            sAuctionHouseStore;
extern DBCStorage <BankBagSlotPricesEntry>       sBankBagSlotPricesStore;
#ifdef ENABLE_PLAYERBOTS
extern DBCStorage <ChatChannelsEntry>            sChatChannelsStore; //has function for access aswell
#else
// extern DBCStorage <ChatChannelsEntry>            sChatChannelsStore; //has function for access aswell, no usable index
#endif
extern DBCStorage <CharStartOutfitEntry>         sCharStartOutfitStore;
extern DBCStorage <ChatChannelsEntry>            sChatChannelsStore;
extern DBCStorage <CharacterFacialHairStylesEntry>  sCharacterFacialHairStylesStore;
extern DBCStorage <CharSectionsEntry>            sCharSectionsStore;
extern DBCStorage <ChrClassesEntry>              sChrClassesStore;
extern DBCStorage <ChrRacesEntry>                sChrRacesStore;
extern DBCStorage <CinematicCameraEntry>         sCinematicCameraStore;
extern DBCStorage <CinematicSequencesEntry>      sCinematicSequencesStore;
extern DBCStorage <CreatureDisplayInfoEntry>     sCreatureDisplayInfoStore;
extern DBCStorage <CreatureDisplayInfoExtraEntry>sCreatureDisplayInfoExtraStore;
extern DBCStorage <CreatureFamilyEntry>          sCreatureFamilyStore;
extern DBCStorage <CreatureModelDataEntry>       sCreatureModelDataStore;
extern DBCStorage <CreatureSpellDataEntry>       sCreatureSpellDataStore;
extern DBCStorage <CreatureTypeEntry>            sCreatureTypeStore;
extern DBCStorage <DurabilityCostsEntry>         sDurabilityCostsStore;
extern DBCStorage <DurabilityQualityEntry>       sDurabilityQualityStore;
extern DBCStorage <EmotesEntry>                  sEmotesStore;
extern DBCStorage <EmotesTextEntry>              sEmotesTextStore;
extern DBCStorage <FactionEntry>                 sFactionStore;
extern DBCStorage <FactionTemplateEntry>         sFactionTemplateStore;
extern DBCStorage <GameObjectArtKitEntry>   sGameObjectArtKitStore;
extern DBCStorage <GameObjectDisplayInfoEntry>   sGameObjectDisplayInfoStore;

extern DBCStorage <GMSurveyCurrentSurveyEntry>   sGMSurveyCurrentSurveyStore;
extern DBCStorage <GMSurveyQuestionsEntry>       sGMSurveyQuestionsStore;
extern DBCStorage <GMSurveyEntry>                sGMSurveySurveysStore;
extern DBCStorage <GMTicketCategoryEntry>        sGMTicketCategoryStore;

extern DBCStorage <ItemBagFamilyEntry>           sItemBagFamilyStore;
extern DBCStorage <ItemClassEntry>               sItemClassStore;
// extern DBCStorage <ItemDisplayInfoEntry>      sItemDisplayInfoStore; -- not used currently
extern DBCStorage <ItemRandomPropertiesEntry>    sItemRandomPropertiesStore;
extern DBCStorage <ItemSetEntry>                 sItemSetStore;
extern DBCStorage <LiquidTypeEntry>              sLiquidTypeStore;
extern DBCStorage <LockEntry>                    sLockStore;
extern DBCStorage <MailTemplateEntry>            sMailTemplateStore;
extern DBCStorage <MapEntry>                     sMapStore;
extern DBCStorage <QuestSortEntry>               sQuestSortStore;
extern DBCStorage <SkillLineEntry>               sSkillLineStore;
extern DBCStorage <SkillLineAbilityEntry>        sSkillLineAbilityStore;
extern DBCStorage <SkillRaceClassInfoEntry>      sSkillRaceClassInfoStore;
extern DBCStorage <SkillTiersEntry>              sSkillTiersStore;
extern DBCStorage <SoundEntriesEntry>            sSoundEntriesStore;
extern DBCStorage <SpellCastTimesEntry>          sSpellCastTimesStore;
extern DBCStorage <SpellDurationEntry>           sSpellDurationStore;
extern DBCStorage <SpellFocusObjectEntry>        sSpellFocusObjectStore;
extern DBCStorage <SpellItemEnchantmentEntry>    sSpellItemEnchantmentStore;
extern SpellCategoryStore                        sSpellCategoryStore;
extern ItemSpellCategoryStore                    sItemSpellCategoryStore;
extern PetFamilySpellsStore                      sPetFamilySpellsStore;
extern DBCStorage <SpellRadiusEntry>             sSpellRadiusStore;
extern DBCStorage <SpellRangeEntry>              sSpellRangeStore;
extern DBCStorage <SpellShapeshiftFormEntry>     sSpellShapeshiftFormStore;
extern DBCStorage <StableSlotPricesEntry>        sStableSlotPricesStore;
extern DBCStorage <TalentEntry>                  sTalentStore;
extern DBCStorage <TalentTabEntry>               sTalentTabStore;
extern DBCStorage <TaxiNodesEntry>               sTaxiNodesStore;
extern DBCStorage <TaxiPathEntry>                sTaxiPathStore;
extern TaxiMask                                  sTaxiNodesMask;
extern TaxiPathSetBySource                       sTaxiPathSetBySource;
extern TaxiPathNodesByPath                       sTaxiPathNodesByPath;
extern DBCStorage <TransportAnimationEntry>      sTransportAnimationStore;
extern DBCStorage <WMOAreaTableEntry>            sWMOAreaTableStore;
// extern DBCStorage <WorldMapAreaEntry>           sWorldMapAreaStore; -- use Zone2MapCoordinates and Map2ZoneCoordinates
// extern DBCStorage <WorldMapOverlayEntry>         sWorldMapOverlayStore;

void LoadDBCStores(const std::string& dataPath);

// script support functions
DBCStorage <SoundEntriesEntry>          const* GetSoundEntriesStore();
DBCStorage <SpellRangeEntry>            const* GetSpellRangeStore();
DBCStorage <FactionEntry>               const* GetFactionStore();
DBCStorage <CreatureDisplayInfoEntry>   const* GetCreatureDisplayStore();
DBCStorage <EmotesEntry>                const* GetEmotesStore();
DBCStorage <EmotesTextEntry>            const* GetEmotesTextStore();
#endif
