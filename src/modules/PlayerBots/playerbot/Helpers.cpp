
#include "playerbot/playerbot.h"
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

void split(std::vector<std::string>& dest, const std::string& str, const char* delim)
{
    char* pTempStr = strdup( str.c_str() );
    char* pWord = strtok(pTempStr, delim);
    while(pWord != NULL)
    {
        dest.push_back(pWord);
        pWord = strtok(NULL, delim);
    }

    free(pTempStr);
}

std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

char *strstri(const char *haystack, const char *needle)
{
    if ( !*needle )
    {
        return (char*)haystack;
    }
    for ( ; *haystack; ++haystack )
    {
        if ( tolower(*haystack) == tolower(*needle) )
        {
            const char *h = haystack, *n = needle;
            for ( ; *h && *n; ++h, ++n )
            {
                if ( tolower(*h) != tolower(*n) )
                {
                    break;
                }
            }
            if ( !*n )
            {
                return (char*)haystack;
            }
        }
    }
    return 0;
}



uint64 extractGuid(WorldPacket& packet)
{
    uint8 mask;
    packet >> mask;
    uint64 guid = 0;
    uint8 bit = 0;
    uint8 testMask = 1;
    while (true)
    {
        if (mask & testMask)
        {
            uint8 word;
            packet >> word;
            guid += (word << bit);
        }
        if (bit == 7)
            break;
        ++bit;
        testMask <<= 1;
    }
    return guid;
}

std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !std::isspace(c);}));
        return s;
}

std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) {return !std::isspace(c); }).base(), s.end());
        return s;
}

std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}
