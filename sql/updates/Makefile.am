# Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

## Process this file with automake to produce Makefile.in

## Sub-directories to parse
SUBDIRS = before_upgrade_to_0.13

## Change installation location
#  datadir = mangos/sql/updates
pkgdatadir = $(datadir)/mangos/sql/updates

## Files to be installed
#  Install basic SQL files to datadir
pkgdata_DATA = \
	001_6939_01_mangos_quest_template.sql \
	002_6944_01_mangos_mangos_string.sql \
	003_6961_01_mangos_command.sql \
	004_6970_01_mangos_playercreateinfo.sql \
	005_7107_01_mangos_string.sql \
	006_7214_01_mangos_command.sql \
	007_7252_01_mangos_command.sql \
	007_7252_02_mangos_mangos_string.sql \
	008_7290_01_mangos_command.sql \
	009_7314_01_characters_guild_rank.sql \
	010_7331_01_mangos_command.sql \
	011_7332_01_mangos_command.sql \
	012_7422_01_mangos_mangos_string.sql \
	013_7493_01_mangos_command.sql \
	014_7495_01_mangos_mangos_string.sql \
	015_7503_01_mangos_command.sql \
	016_7565_01_mangos_mangos_string.sql \
	017_7615_01_mangos_command.sql \
	018_7616_01_mangos_mangos_string.sql \
	018_7616_02_mangos_command.sql \
	019_7622_01_mangos_creature_ai_scripts.sql \
	019_7622_02_mangos_creature_ai_summons.sql \
	019_7622_03_mangos_creature_ai_texts.sql \
	020_7643_01_mangos_db_version.sql \
	020_7643_02_mangos_mangos_string.sql \
	021_01_mangos_spell_proc_event.sql \
	022_01_mangos_spell_proc_event.sql \
	023_01_mangos_spell_proc_event.sql \
	024_01_mangos_spell_proc_event.sql \
	025_7705_01_mangos_command.sql \
	026_7706_01_mangos_command.sql \
	027_7714_01_mangos_command.sql \
	028_7720_01_mangos_mangos_string.sql \
	029_7393_01_mangos_game_event.sql \
	030_7303_01_mangos_pools.sql \
	031_7796_01_mangos_command.sql \
	031_7796_02_mangos_mangos_string.sql \
	032_7059_01_characters_character_spell.sql \
	032_7059_02_characters_pet_spell.sql \
	033_7839_01_mangos_mangos_string.sql \
	033_7839_02_mangos_command.sql \
	034_7850_01_mangos_command.sql \
	035_7855_01_mangos_pools.sql \
	036_7867_01_realmd_account.sql \
	037_7893_01_mangos_command.sql \
	038_7886_01_mangos_petcreateinfo_spell.sql \
	039_7896_01_mangos_creature_template.sql \
	040_7902_01_mangos_pool_creature.sql \
	040_7902_02_mangos_pool_gameobject.sql \
	041_7932_01_characters_character_pet.sql \
	042_7938_01_realmd_account.sql \
	043_7945_01_mangos_quest_template.sql \
	044_7980_01_mangos_item_required_target.sql \
	045_8098_01_characters_character_action.sql \
	045_8098_02_mangos_playercreateinfo_action.sql \
	045_8098_03_characters_character_pet.sql \
	045_8098_04_characters_pet_spell.sql \
	046_7349_01_mangos_spell_area.sql \
	047_7376_01_mangos_spell_area.sql \
	048_7382_01_mangos_creature_template.sql \
        049_8104_01_characters.sql \
	050_8115_01_mangos_playercreateinfo_action.sql \
	051_8158_01_mangos_playercreateinfo_action.sql \
	052_8072_01_characters_characters.sql \
	052_8072_02_characters_characters.sql \
	README

## Additional files to include when running 'make dist'
#  SQL update files, to upgrade database schema from older revisions
EXTRA_DIST = \
	001_6939_01_mangos_quest_template.sql \
	002_6944_01_mangos_mangos_string.sql \
	003_6961_01_mangos_command.sql \
	004_6970_01_mangos_playercreateinfo.sql \
	005_7107_01_mangos_string.sql \
	006_7214_01_mangos_command.sql \
	007_7252_01_mangos_command.sql \
	007_7252_02_mangos_mangos_string.sql \
	008_7290_01_mangos_command.sql \
	009_7314_01_characters_guild_rank.sql \
	010_7331_01_mangos_command.sql \
	011_7332_01_mangos_command.sql \
	012_7422_01_mangos_mangos_string.sql \
	013_7493_01_mangos_command.sql \
	014_7495_01_mangos_mangos_string.sql \
	015_7503_01_mangos_command.sql \
	016_7565_01_mangos_mangos_string.sql \
	017_7615_01_mangos_command.sql \
	018_7616_01_mangos_mangos_string.sql \
	018_7616_02_mangos_command.sql \
	019_7622_01_mangos_creature_ai_scripts.sql \
	019_7622_02_mangos_creature_ai_summons.sql \
	019_7622_03_mangos_creature_ai_texts.sql \
	020_7643_01_mangos_db_version.sql \
	020_7643_02_mangos_mangos_string.sql \
	021_01_mangos_spell_proc_event.sql \
	022_01_mangos_spell_proc_event.sql \
	023_01_mangos_spell_proc_event.sql \
	024_01_mangos_spell_proc_event.sql \
	025_7705_01_mangos_command.sql \
	026_7706_01_mangos_command.sql \
	027_7714_01_mangos_command.sql \
	028_7720_01_mangos_mangos_string.sql \
	029_7393_01_mangos_game_event.sql \
	030_7303_01_mangos_pools.sql \
	031_7796_01_mangos_command.sql \
	031_7796_02_mangos_mangos_string.sql \
	032_7059_01_characters_character_spell.sql \
	032_7059_02_characters_pet_spell.sql \
	033_7839_01_mangos_mangos_string.sql \
	033_7839_02_mangos_command.sql \
	034_7850_01_mangos_command.sql \
	035_7855_01_mangos_pools.sql \
	036_7867_01_realmd_account.sql \
	037_7893_01_mangos_command.sql \
	038_7886_01_mangos_petcreateinfo_spell.sql \
	039_7896_01_mangos_creature_template.sql \
	040_7902_01_mangos_pool_creature.sql \
	040_7902_02_mangos_pool_gameobject.sql \
	041_7932_01_characters_character_pet.sql \
	042_7938_01_realmd_account.sql \
	043_7945_01_mangos_quest_template.sql \
	044_7980_01_mangos_item_required_target.sql \
	045_8098_01_characters_character_action.sql \
	045_8098_02_mangos_playercreateinfo_action.sql \
	045_8098_03_characters_character_pet.sql \
	045_8098_04_characters_pet_spell.sql \
	046_7349_01_mangos_spell_area.sql \
	047_7376_01_mangos_spell_area.sql \
	048_7382_01_mangos_creature_template.sql \
        049_8104_01_characters.sql \
	050_8115_01_mangos_playercreateinfo_action.sql \
	051_8158_01_mangos_playercreateinfo_action.sql \
	052_8072_01_characters_characters.sql \
	052_8072_02_characters_characters.sql \
	README
