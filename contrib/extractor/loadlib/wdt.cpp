#define _CRT_SECURE_NO_DEPRECATE

#include "wdt.h"
#include <string.h>

bool wdt_MWMO::prepareLoadedData()
{
    if (!strncmp(fcc_txt, "MWMO", 4))
        return false;
    return true;
}

bool wdt_MPHD::prepareLoadedData()
{
    if (!strncmp(fcc_txt, "MPHD", 4))
        return false;
    return true;
}

bool wdt_MAIN::prepareLoadedData()
{
    if (!strncmp(fcc_txt, "MAIN", 4))
        return false;
    return true;
}

WDT_file::WDT_file()
{
    mphd = 0;
    main = 0;
    wmo  = 0;
}

WDT_file::~WDT_file()
{
    free();
}

void WDT_file::free()
{
    mphd = 0;
    main = 0;
    wmo  = 0;
    FileLoader::free();
}

bool WDT_file::prepareLoadedData()
{
    // Check parent
    if (!FileLoader::prepareLoadedData())
        return false;

    mphd = (wdt_MPHD*)((uint8*)version + version->size + 8);
    if (!mphd->prepareLoadedData())
        return false;
    main = (wdt_MAIN*)((uint8*)mphd + mphd->size + 8);
    if (!main->prepareLoadedData())
        return false;
    wmo = (wdt_MWMO*)((uint8*)main + main->size + 8);
    if (!wmo->prepareLoadedData())
        return false;
    return true;
}
