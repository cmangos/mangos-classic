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

## Change installation location
#  datadir = mangos/sql/updates
pkgdatadir = $(datadir)/mangos/sql/updates/before_upgrade_to_0.13

## Files to be installed
#  Install basic SQL files to datadir
pkgdata_DATA = \
	020_7643_01_mangos_db_version.sql \
	029_7393_01_mangos_game_event.sql \
	032_7059_01_characters.sql \
	035_7855_01_mangos_pools.sql \
	039_7896_01_mangos_creature_template.sql \
	040_7902_01_mangos_pool.sql \
	041_7932_01_characters_character_pet.sql \
	043_7945_01_mangos_quest_template.sql \
	045_8098_01_characters_character_action.sql \
	045_8098_02_mangos_playercreateinfo_action.sql \
	045_8098_03_characters_character_pet.sql \
	045_8098_04_characters_pet_spell.sql \
	048_7382_01_mangos_creature_template.sql \
	README

## Additional files to include when running 'make dist'
#  SQL update files, to upgrade database schema from older revisions
EXTRA_DIST = \
	020_7643_01_mangos_db_version.sql \
	029_7393_01_mangos_game_event.sql \
	032_7059_01_characters.sql \
	035_7855_01_mangos_pools.sql \
	039_7896_01_mangos_creature_template.sql \
	040_7902_01_mangos_pool.sql \
	041_7932_01_characters_character_pet.sql \
	043_7945_01_mangos_quest_template.sql \
	045_8098_01_characters_character_action.sql \
	045_8098_02_mangos_playercreateinfo_action.sql \
	045_8098_03_characters_character_pet.sql \
	045_8098_04_characters_pet_spell.sql \
	048_7382_01_mangos_creature_template.sql \
	README
