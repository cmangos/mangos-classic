/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

const char AreaTableEntryfmt[]="iiinixxxxxissssssssxixxxx";
const char AreaTriggerEntryfmt[]="niffffffff";
const char AuctionHouseEntryfmt[]="niiixxxxxxxxxxxxxxxxx";
const char BankBagSlotPricesEntryfmt[]="ni";
const char ChrClassesEntryfmt[]="nxxixssssssssixxx";
const char ChrRacesEntryfmt[]="nxixiixxixxxxxixissssssssxxxx";
const char CharStartOutfitEntryfmt[]="diiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char ChatChannelsEntryfmt[]="iixssssssssxxxxxxxxxx";
                                                            // ChatChannelsEntryfmt, index not used (more compact store)
const char CinematicSequencesEntryfmt[]="nxxxxxxxxx";
const char CreatureDisplayInfofmt[]="nxxxfxxxxxxx";
const char CreatureFamilyfmt[]="nfifiiiissssssssxx";
const char CreatureSpellDatafmt[]="nxxxxxxxx";
const char DurabilityCostsfmt[]="niiiiiiiiiiiiiiiiiiiiiiiiiiiii";
const char DurabilityQualityfmt[]="nf";
const char EmotesEntryfmt[]="nxxiiix";
const char EmotesTextEntryfmt[]="nxixxxxxxxxxxxxxxxx";
const char FactionEntryfmt[]="niiiiiiiiiiiiiiiiiissssssssxxxxxxxxxx";
const char FactionTemplateEntryfmt[]="niiiiiiiiiiiii";
const char GameObjectDisplayInfofmt[]="nxxxxxxxxxxx";

const char ItemBagFamilyfmt[]="nxxxxxxxxxxxxxxxxx";
//const char ItemDisplayTemplateEntryfmt[]="nxxxxxxxxxxixxxxxxxxxxx";
const char ItemRandomPropertiesfmt[]="nxiiixxxxxxxxxxx";
const char ItemSetEntryfmt[]="dssssssssxxxxxxxxxxxxxxxxxxiiiiiiiiiiiiiiiiii";
const char LockEntryfmt[]="niiiiixxxiiiiixxxiixxxxxxxxxxxxxx";
const char MailTemplateEntryfmt[]="nxxxxxxxxx";
const char MapEntryfmt[]="nxixssssssssxxxxxxxixxxxxxxxxxxxxxxxxxixxx";
const char QuestSortEntryfmt[]="nxxxxxxxxx";
const char SkillLinefmt[]="nixssssssssxxxxxxxxxxi";
const char SkillLineAbilityfmt[]="niiiixxiiiiixxi";
const char SoundEntriesfmt[]="nxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

const char SpellCastTimefmt[]="nixx";
const char SpellDurationfmt[]="niii";
const char SpellEntryfmt[]="niixiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiifiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiffffffiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiifffixiixssssssssxssssssssxxxxxxxxxxxxxxxxxxxiiiiiiiiiixfffxxx";
const char SpellFocusObjectfmt[]="nxxxxxxxxxxxxxxxxx";
const char SpellItemEnchantmentfmt[]="niiiiiixxxiiissssssssssssssssxiiii";
const char SpellItemEnchantmentConditionfmt[]="nbbbbbxxxxxbbbbbbbbbbiiiiiXXXXX";
const char SpellRadiusfmt[]="nfxf";
const char SpellRangefmt[]="nffxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char SpellShapeshiftfmt[]="nxxxxxxxxxxxxxxxxxxiixixxxxxxxxxxxx";
const char StableSlotPricesfmt[] = "ni";

//const char SummonPropertiesfmt[] = "niiiii";
const char TalentEntryfmt[]="niiiiiiiixxxxixxixxxi";
const char TalentTabEntryfmt[]="nxxxxxxxxxxxxxxxxxxxiix";
const char TaxiNodesEntryfmt[]="nifffssssssssssssssssxii";
const char TaxiPathEntryfmt[]="niii";
const char TaxiPathNodeEntryfmt[]="diiifffiixx";
const char TotemCategoryEntryfmt[]="nxxxxxxxxxxxxxxxxxii";
const char WorldMapAreaEntryfmt[]="xinxffffi";
//const char WorldMapOverlayEntryfmt[]="nxiiiixxxxxxxxxxx";
const char WorldSafeLocsEntryfmt[]="nifffxxxxxxxxxxxxxxxxx";

#endif
