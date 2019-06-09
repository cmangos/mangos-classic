//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include "Filelist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#	include <io.h>
#else
#	include <dirent.h>
#endif

void scanDirectory(const char* path, const char* ext, FileList& list)
{
    list.clear();

#ifdef WIN32
	_finddata_t dir;
	char pathWithExt[260];
    intptr_t fh;
	strcpy(pathWithExt, path);
	strcat(pathWithExt, "/");
	strcat(pathWithExt, ext);
	fh = _findfirst(pathWithExt, &dir);
	if (fh == -1L)
		return;
	do
	{
        list.push_back(dir.name);
	}
	while (_findnext(fh, &dir) == 0);
	_findclose(fh);
#else
	dirent* current = 0;
	DIR* dp = opendir(path);
	if (!dp)
		return;

	while ((current = readdir(dp)) != 0)
	{
		int len = strlen(current->d_name);
		if (len > 4 && strncmp(current->d_name+len-4, ext, 4) == 0) // TODO:: may not work due to joker(*) added
		{
            list.push_back(current->d_name);
		}
	}
	closedir(dp);
#endif
}
