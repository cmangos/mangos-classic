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

#ifndef VMAPEXPORT_H
#define VMAPEXPORT_H

#include <string>
#include <set>

typedef std::set<std::string> StringSet;

enum ModelFlags
{
    MOD_M2 = 1,
    MOD_WORLDSPAWN = 1 << 1,
    MOD_HAS_BOUND = 1 << 2
};

extern const char* szWorkDirWmo;
extern const char* szRawVMAPMagic;                          // vmap magic string for extracted raw vmap data

bool FileExists(const char* file);
void strToLower(char* str);

bool ExtractSingleWmo(std::string& fname);

/* @param origPath = original path of the model, cleaned with fixnamen and fixname2
 * @param fixedName = will store the translated name (if changed)
 * @param failedPaths = Set to collect errors
 */
bool ExtractSingleModel(std::string& origPath, std::string& fixedName, StringSet& failedPaths);

void ExtractGameobjectModels();

#endif
