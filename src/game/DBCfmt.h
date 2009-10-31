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
const char AreaTriggerEntryfmt[]="niffffffff";                                       // new
const char AuctionHouseEntryfmt[]="niiixxxxxxxxx";                                   // new
const char BankBagSlotPricesEntryfmt[]="ni";
const char ChrClassesEntryfmt[]="nxxixssssssssixxx";
const char ChrRacesEntryfmt[]="nxixiixxixxxxxixissssssssxxxx";
const char CharStartOutfitEntryfmt[]="diiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxxxxx";    // new
const char ChatChannelsEntryfmt[]="iixssssssssxxxxxxxxxx";                           // new
const char CreatureDisplayInfofmt[]="nxxxfxxxxxxx";                                  // new
const char CreatureFamilyfmt[]="nfifiiiissssssssxx";
const char CreatureSpellDatafmt[]="nxxxxxxxx";                                       // new
const char DurabilityCostsfmt[]="niiiiiiiiiiiiiiiiiiiiiiiiiiiii";                    // new
const char DurabilityQualityfmt[]="nf";                                              // new
const char EmotesEntryfmt[]="nxxiiix";
const char EmotesTextEntryfmt[]="nxixxxxxxxxxxxxxxxx";
const char FactionEntryfmt[]="niiiiiiiiiiiiiiiiiissssssssxxxxxxxxxx";
const char FactionTemplateEntryfmt[]="niiiiiiiiiiiii";
const char ItemDisplayTemplateEntryfmt[]="ixxxxxxxxxxixxxxxxxxxxx";
const char ItemRandomPropertiesfmt[]="nxiiixxxxxxxxxxx";
const char ItemSetEntryfmt[]="dssssssssxxxxxxxxxxxxxxxxxxiiiiiiiiiiiiiiiiii";
const char LockEntryfmt[]="niiiiixxxiiiiixxxiixxxxxxxxxxxxxx";
const char MailTemplateEntryfmt[]="nxxxxxxxxx";                                      // new
const char MapEntryfmt[]="nxixssssssssxxxxxxxixxxxxxxxxxxxxxxxxxixxx";
const char QuestSortEntryfmt[]="nxxxxxxxxx";                                         // new
const char SkillLinefmt[]="nixssssssssxxxxxxxxxxi";
const char SkillLineAbilityfmt[]="niiiixxiiiiixxi";
const char SoundEntriesfmt[]="nxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char SpellCastTimefmt[]="nixx";
const char SpellDurationfmt[]="niii";
const char SpellEntryfmt[]="niixiiiiiiiiiiiixxiiiiiiiiiiiiiiiiiiifiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiffffffiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiifffixiixssssssssxssssssssxxxxxxxxxxxxxxxxxxxiiiiiiiiiixfffxxx";
const char SpellFocusObjectfmt[]="nxxxxxxxxx";
const char SpellItemEnchantmentfmt[]="niiiiiixxxiiissssssssxii";
const char SpellRadiusfmt[]="nfxf";
const char SpellRangefmt[]="nffixxxxxxxxxxxxxxxxxx";                     // new
const char SpellShapeshiftfmt[]="nxxxxxxxxxxixx";                        // new
const char StableSlotPricesfmt[] = "ni";                                 // new
const char TalentEntryfmt[]="niiiiiiiixxxxixxixxxi";
const char TalentTabEntryfmt[]="nxxxxxxxxxxxiix";
const char TaxiNodesEntryfmt[]="nifffssssssssxii";
const char TaxiPathEntryfmt[]="niii";
const char TaxiPathNodeEntryfmt[]="diiifffii";
const char WorldMapAreaEntryfmt[]="xinxffff";
const char WorldSafeLocsEntryfmt[]="nifffxxxxxxxxx";

#endif
