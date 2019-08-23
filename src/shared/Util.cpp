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

#include "Util.h"
#include "Timer.h"
#include <utf8.h>
#include "TSS.h"

#include <boost/asio.hpp>

#include <chrono>
#include <cstdarg>

std::mt19937* initRand()
{
    std::seed_seq seq = { size_t(std::time(nullptr)), size_t(std::clock()) };
    return new std::mt19937(seq);
}

static MaNGOS::thread_local_ptr<std::mt19937> mtRand(&initRand);

std::mt19937* GetRandomGenerator()
{
    return mtRand.get();
}

uint32 WorldTimer::m_iTime = 0;
uint32 WorldTimer::m_iPrevTime = 0;

uint32 WorldTimer::tickTime() { return m_iTime; }
uint32 WorldTimer::tickPrevTime() { return m_iPrevTime; }

uint32 WorldTimer::tick()
{
    // save previous world tick time
    m_iPrevTime = m_iTime;

    // get the new one and don't forget to persist current system time in m_SystemTickTime
    m_iTime = WorldTimer::getMSTime();

    // return tick diff
    return getMSTimeDiff(m_iPrevTime, m_iTime);
}

uint32 WorldTimer::getMSTime()
{
    static auto const start_time = std::chrono::system_clock::now();
    return static_cast<uint32>((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) - std::chrono::duration_cast<std::chrono::milliseconds>(start_time.time_since_epoch())).count());
}

//////////////////////////////////////////////////////////////////////////
int32 irand(int32 min, int32 max)
{
    std::uniform_int_distribution<int32> dist(min, max);
    return dist(*mtRand.get());
}

uint32 urand(uint32 min, uint32 max)
{
    std::uniform_int_distribution<uint32> dist(min, max);
    return dist(*mtRand.get());
}

float frand(float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
    return dist(*mtRand.get());
}

int32 irand()
{
    std::uniform_int_distribution<int32> dist;
    return dist(*mtRand.get());
}

uint32 urand()
{
    std::uniform_int_distribution<uint32> dist;
    return dist(*mtRand.get());
}

double rand_norm()
{
    std::uniform_real_distribution<double> dist(0, 1.0f);
    return dist(*mtRand.get());
}

float rand_norm_f()
{
    std::uniform_real_distribution<float> dist(0, 1.0f);
    return dist(*mtRand.get());
}

double rand_chance()
{
    std::uniform_real_distribution<double> dist(0, 100.0);
    return dist(*mtRand.get());
}

float rand_chance_f()
{
    std::uniform_real_distribution<double> dist(0, 100.0);
    return float(dist(*mtRand.get()));
}

Tokens StrSplit(const std::string& src, const std::string& sep)
{
    Tokens r;
    std::string s;
    for (char i : src)
    {
        if (sep.find(i) != std::string::npos)
        {
            if (s.length()) r.push_back(s);
            s.clear();
        }
        else
        {
            s += i;
        }
    }
    if (s.length()) r.push_back(s);
    return r;
}

uint32 GetUInt32ValueFromArray(Tokens const& data, uint16 index)
{
    if (index >= data.size())
        return 0;

    return (uint32)atoi(data[index].c_str());
}

float GetFloatValueFromArray(Tokens const& data, uint16 index)
{
    float result;
    uint32 temp = GetUInt32ValueFromArray(data, index);
    memcpy(&result, &temp, sizeof(result));

    return result;
}

void stripLineInvisibleChars(std::string& str)
{
    static std::string invChars = " \t\7\n";

    size_t wpos = 0;

    bool space = false;
    for (size_t pos = 0; pos < str.size(); ++pos)
    {
        if (invChars.find(str[pos]) != std::string::npos)
        {
            if (!space)
            {
                str[wpos++] = ' ';
                space = true;
            }
        }
        else
        {
            if (wpos != pos)
                str[wpos++] = str[pos];
            else
                ++wpos;
            space = false;
        }
    }

    if (wpos < str.size())
        str.erase(wpos, str.size());
}

std::string secsToTimeString(time_t timeInSecs, bool shortText, bool hoursOnly)
{
    time_t secs    = timeInSecs % MINUTE;
    time_t minutes = timeInSecs % HOUR / MINUTE;
    time_t hours   = timeInSecs % DAY  / HOUR;
    time_t days    = timeInSecs / DAY;

    std::ostringstream ss;
    if (days)
        ss << days << (shortText ? "d" : " Day(s) ");
    if (hours || hoursOnly)
        ss << hours << (shortText ? "h" : " Hour(s) ");
    if (!hoursOnly)
    {
        if (minutes)
            ss << minutes << (shortText ? "m" : " Minute(s) ");
        if (secs || (!days && !hours && !minutes))
            ss << secs << (shortText ? "s" : " Second(s).");
    }

    return ss.str();
}

uint32 TimeStringToSecs(const std::string& timestring)
{
    uint32 secs       = 0;
    uint32 buffer     = 0;
    uint32 multiplier = 0;

    for (char itr : timestring)
    {
        if (isdigit(itr))
        {
            buffer *= 10;
            buffer += itr - '0';
        }
        else
        {
            switch (itr)
            {
                case 'd': multiplier = DAY;     break;
                case 'h': multiplier = HOUR;    break;
                case 'm': multiplier = MINUTE;  break;
                case 's': multiplier = 1;       break;
                default : return 0;                         // bad format
            }
            buffer *= multiplier;
            secs += buffer;
            buffer = 0;
        }
    }

    return secs;
}

std::string TimeToTimestampStr(time_t t)
{
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    char buf[20];
    snprintf(buf, 20, "%04d-%02d-%02d_%02d-%02d-%02d", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
    return std::string(buf);
}

/// Check if the string is a valid ip address representation
bool IsIPAddress(char const* ipaddress)
{
    if (!ipaddress)
        return false;

    // Let the big boys do it.
    // Drawback: all valid ip address formats are recognized e.g.: 12.23,121234,0xABCD)
    boost::system::error_code ec;
    boost::asio::ip::address::from_string(ipaddress, ec);
    return ec.value() == 0;
}

/// create PID file
uint32 CreatePIDFile(const std::string& filename)
{
    FILE* pid_file = fopen(filename.c_str(), "w");
    if (pid_file == nullptr)
        return 0;

#ifdef _WIN32
    DWORD pid = GetCurrentProcessId();
#else
    pid_t pid = getpid();
#endif

    fprintf(pid_file, "%d", pid);
    fclose(pid_file);

    return (uint32)pid;
}

size_t utf8length(std::string& utf8str)
{
    try
    {
        return utf8::distance(utf8str.c_str(), utf8str.c_str() + utf8str.size());
    }
    catch (const std::exception&)
    {
        utf8str = "";
        return 0;
    }
}

bool Utf8toWStr(const std::string& utf8str, std::wstring& wstr, size_t max_len)
{
    if (utf8str.empty())
    {
        wstr = std::wstring();
        return true;
    }

    try
    {
        // A UTF8 string can have a maximum of 4 octets per character
        // A 4 octet char can take up to two UTF16 characters (4*8 = 32 / 16 = 2)
        // The UTF8 string may also actually be ASCII, in which case no truncation
        // takes place! The final string length is therefore unknown. Reserve
        // as long as the OG string, and back-insert
        wstr.resize(utf8str.size());

        auto end = utf8::utf8to16(utf8str.cbegin(), utf8str.cend(), wstr.begin());

        if (end != wstr.end())
            wstr.erase(end, wstr.end());

        // truncate to max len
        if (!!max_len && wstr.size() > max_len)
        {
            wstr.resize(max_len);
        }
    }
    catch (const std::exception&)
    {
        wstr = L"";
        return false;
    }

    return true;
}

void utf8truncate(std::string& utf8str, size_t len)
{
    try
    {
        std::wstring wstr;
        Utf8toWStr(utf8str, wstr, len);
        WStrToUtf8(wstr, utf8str);
    }
    catch (std::exception)
    {
        utf8str = "";
    }
}

bool WStrToUtf8(const std::wstring& wstr, std::string& utf8str)
{
    try
    {
        std::string utf8str2;
        utf8str2.resize(wstr.size() * 4);                   // allocate for most long case

        auto end = utf8::utf16to8(wstr.cbegin(), wstr.cend(), utf8str2.begin());
        if (end != utf8str2.end())
            utf8str2.erase(end, utf8str2.end());

        utf8str = utf8str2;
    }
    catch (const std::exception&)
    {
        utf8str = "";
        return false;
    }

    return true;
}

typedef wchar_t const* const* wstrlist;

bool utf8ToConsole(const std::string& utf8str, std::string& conStr)
{
#if PLATFORM == PLATFORM_WINDOWS
    std::wstring wstr;
    if (!Utf8toWStr(utf8str, wstr))
        return false;

    conStr.resize(wstr.size());
    CharToOemBuffW(&wstr[0], &conStr[0], wstr.size());
#else
    // not implemented yet
    conStr = utf8str;
#endif

    return true;
}

bool consoleToUtf8(const std::string& conStr, std::string& utf8str)
{
#if PLATFORM == PLATFORM_WINDOWS
    std::wstring wstr;
    wstr.resize(conStr.size());
    OemToCharBuffW(&conStr[0], &wstr[0], conStr.size());

    return WStrToUtf8(wstr, utf8str);
#else
    // not implemented yet
    utf8str = conStr;
    return true;
#endif
}

bool Utf8FitTo(const std::string& str, const std::wstring& search)
{
    std::wstring temp;

    if (!Utf8toWStr(str, temp))
        return false;

    // converting to lower case
    wstrToLower(temp);

    return temp.find(search) != std::wstring::npos;
}

void utf8printf(FILE* out, const char* str, ...)
{
    va_list ap;
    va_start(ap, str);
    vutf8printf(out, str, &ap);
    va_end(ap);
}

void vutf8printf(FILE* out, const char* str, va_list* ap)
{
#if PLATFORM == PLATFORM_WINDOWS
    std::string temp_buf;
    temp_buf.resize(32 * 1024);
    std::wstring wtemp_buf;

    size_t temp_len = vsnprintf(&temp_buf[0], 32 * 1024, str, *ap);
    temp_buf.resize(strlen(temp_buf.c_str())); // Resize to match the formatted string

    if (!temp_buf.empty())
    {
        Utf8toWStr(temp_buf, wtemp_buf, 32 * 1024);
        wtemp_buf.push_back('\0');

        CharToOemBuffW(&wtemp_buf[0], &temp_buf[0], wtemp_buf.size());
    }
    fprintf(out, "%s", temp_buf.c_str());
#else
    vfprintf(out, str, *ap);
#endif
}

void hexEncodeByteArray(uint8* bytes, uint32 arrayLen, std::string& result)
{
    std::ostringstream ss;
    for (uint32 i = 0; i < arrayLen; ++i)
    {
        for (uint8 j = 0; j < 2; ++j)
        {
            unsigned char nibble = 0x0F & (bytes[i] >> ((1 - j) * 4));
            char encodedNibble;
            if (nibble < 0x0A)
                encodedNibble = '0' + nibble;
            else
                encodedNibble = 'A' + nibble - 0x0A;
            ss << encodedNibble;
        }
    }
    result = ss.str();
}
