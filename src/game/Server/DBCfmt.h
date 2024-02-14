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

#ifndef MANGOS_DBCSFRM_H
#define MANGOS_DBCSFRM_H

const char AreaTableEntryfmt[] = "niiiixxxxxissssssssxixxxi";
const char AreaTriggerEntryfmt[] = "niffffffff";
const char AuctionHouseEntryfmt[] = "niiixxxxxxxxx";
const char BankBagSlotPricesEntryfmt[] = "ni";
char const CharSectionsEntryfmt[] = "diiiiixxxi";
char const CharacterFacialHairStylesfmt[] = "iiixxxxxx";
const char ChrClassesEntryfmt[] = "nxxixssssssssxxix";
const char ChrRacesEntryfmt[] = "nxixiixxixxxxxixissssssssxxxx";
const char CharStartOutfitEntryfmt[] = "diiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char ChatChannelsEntryfmt[] = "iixssssssssxxxxxxxxxx";// ChatChannelsEntryfmt, index not used (more compact store)
const char CinematicCameraEntryfmt[] = "nsiffff";
const char CinematicSequencesEntryfmt[] = "nxixxxxxxx";
const char CreatureDisplayInfofmt[] = "nixifxxxxxxx";
const char CreatureDisplayInfoExtrafmt[] = "nixxxxxxxxxxxxxxxxx";
const char CreatureFamilyfmt[] = "nfifiiiissssssssxx";
const char CreatureModelDatafmt[] = "nixxfxxxxxxxxxff";
const char CreatureSpellDatafmt[] = "niiiixxxx";
const char CreatureTypefmt[] = "nxxxxxxxxxx";
const char DurabilityCostsfmt[] = "niiiiiiiiiiiiiiiiiiiiiiiiiiiii";
const char DurabilityQualityfmt[] = "nf";
const char EmotesEntryfmt[] = "nxxiiix";
#ifdef ENABLE_PLAYERBOTS
char const EmotesTextSoundEntryfmt[] = "niiii";
#endif
const char EmotesTextEntryfmt[] = "nxixxxxxxxxxxxxxxxx";
const char FactionEntryfmt[] = "niiiiiiiiiiiiiiiiiissssssssxxxxxxxxxx";
const char FactionTemplateEntryfmt[] = "niiiiiiiiiiiii";
const char GameObjectArtKitfmt[] = "nxxxxxxx";
const char GameObjectDisplayInfofmt[] = "nsxxxxxxxxxx";
const char GMSurveyCurrentSurveyfmt[] = "ni";
const char GMSurveySurveysfmt[] = "niiiiiiiiii";
const char GMSurveyQuestionsfmt[] = "nssssssssx";
const char GMTicketCategoryfmt[] = "nssssssssx";
const char ItemBagFamilyfmt[] = "nxxxxxxxxx";
const char ItemClassfmt[] = "nxxssssssssx";
// const char ItemDisplayTemplateEntryfmt[]="nxxxxxxxxxxixxxxxxxxxxx";
#ifdef ENABLE_PLAYERBOTS
const char ItemRandomPropertiesfmt[] = "nxiiixxssssssssx";
#else
const char ItemRandomPropertiesfmt[] = "nxiiixxxxxxxxxxx";
#endif
const char ItemSetEntryfmt[] = "dssssssssxxxxxxxxxxxxxxxxxxiiiiiiiiiiiiiiiiii";
const char LiquidTypefmt[] = "niii";
const char LockEntryfmt[] = "niiiiiiiiiiiiiiiiiiiiiiiixxxxxxxx";
const char MailTemplateEntryfmt[] = "nxxxxxxxxx";
const char MapEntryfmt[] = "nxixssssssssxxxxxxxixxxxxxxxxxxxxxxxxxixxx";
const char QuestSortEntryfmt[] = "nxxxxxxxxx";
const char SkillLinefmt[] = "nixssssssssxxxxxxxxxxi";
const char SkillLineAbilityfmt[] = "niiiixxiiiiixxi";
const char SkillRaceClassInfofmt[] = "diiiiiix";
const char SkillTiersfmt[] = "niiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii";
const char SoundEntriesfmt[] = "nxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char SpellCastTimefmt[] = "niii";
const char SpellDurationfmt[] = "niii";
const char SpellFocusObjectfmt[] = "nxxxxxxxxx";
const char SpellItemEnchantmentfmt[] = "niiiiiixxxiiissssssssxii";
const char SpellRadiusfmt[] = "nfxx";
const char SpellRangefmt[] = "nffixxxxxxxxxxxxxxxxxx";
const char SpellShapeshiftfmt[] = "nxxxxxxxxxxiix";
const char StableSlotPricesfmt[] = "ni";
const char TalentEntryfmt[] = "niiiiiiiixxxxixxixxxi";
const char TalentTabEntryfmt[] = "nxxxxxxxxxxxiix";
const char TaxiNodesEntryfmt[] = "nifffssssssssxii";
const char TaxiPathEntryfmt[] = "niii";
const char TaxiPathNodeEntryfmt[] = "diiifffii";
const char WMOAreaTableEntryfmt[] = "niiixxxxxiissssssssx";
const char WorldMapAreaEntryfmt[] = "xinxffff";
const char TransportAnimationfmt[] = "diifffx";
// const char WorldMapOverlayEntryfmt[]="nxiiiixxxxxxxxxxx";
const char WorldSafeLocsEntryfmt[] = "nifffxxxxxxxxx";

#endif
