/*
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 *
 * This is private software and may not be shared under any circumstances,
 * absent permission of namreeb.
 */

#ifndef __WARDENSCAN_HPP_
#define __WARDENSCAN_HPP_

#include "Util/ByteBuffer.h"

#include <openssl/sha.h>

#include <functional>
#include <vector>
#include <memory>
#include <string>

enum ScanFlags
{
    None            = 0x00,

    FromDatabase    = 0x01,     // this scan came from the database, and should be deleted on reload

    WinBuild5875    = 0x00002,  // 1.12.1
    WinBuild6005    = 0x00004,  // 1.12.2
    WinBuild6141    = 0x00008,  // 1.12.3

    MacBuild5875    = 0x00010,  // 1.12.1

    InitialLogin    = 0x20,     // scans when world session is first created
    InWorld         = 0x40,     // scans run whenever the player is in the world

    AdvancedScan    = 0x80,     // advanced scans will not be sent to low level players

    WinAllBuild     = (WinBuild5875 | WinBuild6005 | WinBuild6141),
    MacAllBuild     = (MacBuild5875),
};

enum WindowsScanType
{
    READ_MEMORY = 0,
    FIND_MODULE_BY_NAME,
    FIND_MEM_IMAGE_CODE_BY_HASH,
    FIND_CODE_BY_HASH,
    HASH_CLIENT_FILE,
    GET_LUA_VARIABLE,
    API_CHECK,
    FIND_DRIVER_BY_NAME,
    CHECK_TIMING_VALUES,
    MAX_SCAN_TYPE
};

class Warden;

class Scan
{
    public:
        using BuildT = std::function<void(const Warden *, std::vector<std::string> &, ByteBuffer &)>;
        using CheckT = std::function<bool(const Warden *, ByteBuffer &)>;

    private:
        BuildT _builder;
        CheckT _checker;

    protected:  // should not be called by the user
        Scan(BuildT builder, CheckT checker, uint32 f, size_t req, size_t rep, const std::string &c)
            : _builder(builder), _checker(checker), flags(f), comment(c), requestSize(req), replySize(rep)
        { MANGOS_ASSERT(!((flags & WinAllBuild) && (flags & MacAllBuild))); }

    public:
        uint32 flags;
        std::string comment;
        size_t requestSize;     // maximum size of request
        size_t replySize;       // maximum size of reply

        Scan() = delete;
        virtual ~Scan() = default;

        void Build(const Warden *warden, std::vector<std::string> &strings, ByteBuffer &scan) const { _builder(warden, strings, scan); }

        // return true when the scan revealed a hack
        bool Check(const Warden *warden, ByteBuffer &buff) const { return _checker(warden, buff); }
};

class MacScan : public Scan
{
    public:
        MacScan(BuildT builder, CheckT checker, size_t requestSize, size_t replySize, const std::string &comment, uint32 flags = MacAllBuild)
            : Scan(builder, checker, flags, requestSize, replySize, comment) {}
};

class WindowsScan : public Scan
{
    public:
        WindowsScan(BuildT builder, CheckT checker, size_t requestSize, size_t replySize, const std::string &comment, uint32 flags = WinAllBuild)
            : Scan(builder, checker, flags, requestSize, replySize, comment) {}

        WindowsScan() = delete;
};

// check to see if a module is loaded
class WindowsModuleScan : public WindowsScan
{
    private:
        std::string _module;
        bool _wanted;

    public:
        static constexpr uint8 ModuleFound = 0x4A;

        WindowsModuleScan(const std::string &module, bool wanted, const std::string &comment, uint32 flags = WinAllBuild);
        WindowsModuleScan(const std::string &module, CheckT checker, const std::string &comment, uint32 flags = WinAllBuild);
};

// read arbitrary memory from the client, optionally offset from the base of a module by name
class WindowsMemoryScan : public WindowsScan
{
    private:
        std::vector<uint8> _expected;
        uint32 _offset;
        std::string _module;

    public:
        WindowsMemoryScan(uint32 offset, const void *expected, size_t length, const std::string &comment, uint32 flags);
        WindowsMemoryScan(const std::string &module, uint32 offset, const void *expected, size_t length, const std::string &comment, uint32 flags);
        WindowsMemoryScan(uint32 offset, size_t length, CheckT checker, const std::string &comment, uint32 flags);
        WindowsMemoryScan(const std::string &module, uint32 offset, size_t length, CheckT checker, const std::string &comment, uint32 flags);
};

// scan the start of each executable memory segment, optionally filtered by those belonging to regions marked by type MEM_IMAGE (as reported by VirtualQuery())
class WindowsCodeScan : public WindowsScan
{
    private:
        uint32 _offset;
        std::vector<uint8> _pattern;
        bool _memImageOnly;
        bool _wanted;

    public:
        static constexpr uint8 PatternFound = 0x4A;

        WindowsCodeScan(uint32 offset, const std::vector<uint8> &pattern, bool memImageOnly, bool wanted, const std::string &comment, uint32 flags = WinAllBuild);
};

// reads the specified file from the client MPQs, hashes it, and sends back the hash
class WindowsFileHashScan : public WindowsScan
{
    private:
        std::string _file;
        uint8 _expected[SHA_DIGEST_LENGTH];
        bool _hashMatch;
        bool _wanted;

    public:
        WindowsFileHashScan(const std::string &file, const void *expected, bool wanted, const std::string &comment, uint32 flags = WinAllBuild);
};

// reads the value of a lua variable and returns it.  keep in mind when using this that different states
// of the game have different variables set.
class WindowsLuaScan : public WindowsScan
{
    private:
        std::string _lua;
        std::string _expectedValue;
        bool _wanted;

    public:
        WindowsLuaScan(const std::string &lua, bool wanted, const std::string &comment, uint32 flags = WinAllBuild);
        WindowsLuaScan(const std::string &lua, const std::string &expectedValue, const std::string &comment, uint32 flags = WinAllBuild);
        WindowsLuaScan(const std::string &lua, CheckT checker, const std::string &comment, uint32 flags = WinAllBuild);
};

// this scan will examine a procedure defined by its module and export name (for GetModuleHandle() and GetProcAddress()).
// it will skip any NOPs it encounters at the start of the function and at the first non-NOP (0x90) it encounters, if
// a JMP (0xE9) is encountered, it will read the requested number of bytes ('length') from the jump target, and compare
// a hash of those bytes to the given hash.
class WindowsHookScan : public WindowsScan
{
    private:
        static constexpr uint8 Detoured = 0x4A;

        std::string _module;
        std::string _proc;
        uint8 _hash[SHA_DIGEST_LENGTH];
        uint32 _offset;
        size_t _length;

    public:
        WindowsHookScan(const std::string &module, const std::string &proc, const void *hash, uint32 offset, size_t length, const std::string &comment, uint32 flags = WinAllBuild);
};

// this scan will search for call kernel32!QueryDosDevice() and search for a device with the given name.
// if the named device was found, and its full path matches the given string, this scan will succeed.
// it is believed that this scan was created in order to target WoWGlider.
class WindowsDriverScan : public WindowsScan
{
    private:
        static constexpr uint8 Found = 0x4A;

        std::string _name;
        std::string _targetPath;
        bool _wanted;

    public:
        WindowsDriverScan(const std::string &name, const std::string &targetPath, bool wanted, const std::string &comment, uint32 flags = WinAllBuild);
};

// this scan will call the game's function 'OsGetAsyncTimeMs()' as well as kernel32!GetTickCount() up to five times
// if the two functions ever return a different value, the first portion of the reply will indicate a failure.
// NOTE: this can happen when the client runs on a newer version of Windows under Windows XP emulation mode, and
// therefore is not sufficient grounds to consider a hack as being detected.
class WindowsTimeScan : public WindowsScan
{
    public:
        WindowsTimeScan(CheckT checker, const std::string &comment, uint32 flags = WinAllBuild);
};
#endif /*!__WARDENSCAN_HPP_*/
