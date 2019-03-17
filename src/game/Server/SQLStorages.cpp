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

#include "Server/SQLStorages.h"

const char CreatureInfosrcfmt[] = "issiiiiiiifiiiiliiiiiffiiiiiiiffffffiiiiffffiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiss";
const char CreatureInfodstfmt[] = "issiiiiiiifiiiiliiiiiffiiiiiiiffffffiiiiffffiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiisi";
const char CreatureDataAddonInfofmt[] = "iiibbiis";
const char CreatureConditionalSpawnSrcFmt[] = "iiix";
const char CreatureConditionalSpawnDstFmt[] = "iii";
const char CreatureModelfmt[] = "iffffbii";
const char CreatureInfoAddonInfofmt[] = "iiibbiis";
const char EquipmentInfofmt[] = "iiii";
const char GameObjectInfosrcfmt[] = "iiisiiifiiiiiiiiiiiiiiiiiiiiiiiiiiis";
const char GameObjectInfodstfmt[] = "iiisiiifiiiiiiiiiiiiiiiiiiiiiiiiiiii";
const char ItemPrototypesrcfmt[] = "iiisiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiffiffiffiffiffiiiiiiiiiifiiifiiiiiifiiiiiifiiiiiifiiiiiifiiiisiiiiiiiiiiiiiisiiiiii";
const char ItemPrototypedstfmt[] = "iiisiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiffiffiffiffiffiiiiiiiiiifiiifiiiiiifiiiiiifiiiiiifiiiiiifiiiisiiiiiiiiiiiiiiiiiiiii";
const char PageTextfmt[] = "isi";
const char InstanceTemplatesrcfmt[] = "iiiiiiiffsl";
const char InstanceTemplatedstfmt[] = "iiiiiiiffil";
const char WorldTemplatesrcfmt[] = "is";
const char WorldTemplatedstfmt[] = "ii";
const char ConditionsFmt[] = "iiiix";
const char CreatureTemplateSpellsFmt[] = "iiiiiiiii";
const char SpellScriptTargetFmt[] = "iiii";
const char SpellEntryfmt[] = "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiifiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiffffffiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiifffiiiissssssssssssssssiiiiiLiiiifffiiiii";
const char SpellConefmt[] = "ii";
const char DungeonEncounterFmt[] = "iiiiissssssssssssssssii";

SQLStorage sCreatureStorage(CreatureInfosrcfmt, CreatureInfodstfmt, "entry", "creature_template");
SQLStorage sCreatureDataAddonStorage(CreatureDataAddonInfofmt, "guid", "creature_addon");
SQLStorage sCreatureModelStorage(CreatureModelfmt, "modelid", "creature_model_info");
SQLStorage sCreatureInfoAddonStorage(CreatureInfoAddonInfofmt, "entry", "creature_template_addon");
SQLStorage sEquipmentStorage(EquipmentInfofmt, "entry", "creature_equip_template");
SQLStorage sItemStorage(ItemPrototypesrcfmt, ItemPrototypedstfmt, "entry", "item_template");
SQLStorage sPageTextStore(PageTextfmt, "entry", "page_text");
SQLStorage sInstanceTemplate(InstanceTemplatesrcfmt, InstanceTemplatedstfmt, "map", "instance_template");
SQLStorage sWorldTemplate(WorldTemplatesrcfmt, WorldTemplatedstfmt, "map", "world_template");
SQLStorage sConditionStorage(ConditionsFmt, "condition_entry", "conditions");
SQLStorage sSpellTemplate(SpellEntryfmt, "id", "spell_template");
SQLStorage sSpellCones(SpellConefmt, "id", "spell_cone");
SQLStorage sDungeonEncounterStore(DungeonEncounterFmt, "id", "instance_dungeon_encounters");
SQLStorage sCreatureConditionalSpawnStore(CreatureConditionalSpawnSrcFmt, CreatureConditionalSpawnDstFmt, "guid", "creature_conditional_spawn");

SQLHashStorage sGOStorage(GameObjectInfosrcfmt, GameObjectInfodstfmt, "entry", "gameobject_template");
SQLHashStorage sCreatureTemplateSpellsStorage(CreatureTemplateSpellsFmt, "entry", "creature_template_spells");

SQLMultiStorage sSpellScriptTargetStorage(SpellScriptTargetFmt, "entry", "spell_script_target");

SQLStorage const* GetSpellStore() { return &sSpellTemplate; }
