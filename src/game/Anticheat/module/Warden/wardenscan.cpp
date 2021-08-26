/*
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 *
 * This is private software and may not be shared under any circumstances,
 * absent permission of namreeb.
 */

#include "WardenScan.hpp"
#include "WardenWin.hpp"
#include "WardenModule.hpp"
#include "ByteBuffer.h"
#include "Util.h"
#include "Auth/HMACSHA1.h"

#include <openssl/sha.h>

#include <string>
#include <algorithm>
#include <functional>

WindowsModuleScan::WindowsModuleScan(const std::string &module, bool wanted, const std::string &comment, uint32 flags)
    : _module(module), _wanted(wanted),
    WindowsScan(
    // builder
    [this](const Warden *warden, std::vector<std::string> &, ByteBuffer &scan)
    {
        auto const winWarden = reinterpret_cast<const WardenWin *>(warden);
        auto const seed = static_cast<uint32>(urand());

        scan << static_cast<uint8>(winWarden->GetModule()->opcodes[FIND_MODULE_BY_NAME] ^ winWarden->GetXor()) << seed;

        HMACSHA1 hash(sizeof(seed), reinterpret_cast<const uint8*>(&seed));
        hash.UpdateData(this->_module);
        hash.Finalize();

        scan.append(hash.GetDigest(), hash.GetLength());
    },
    // checker
    [this](const Warden *, ByteBuffer &buff)
    {
        auto const found = buff.read<uint8>() == ModuleFound;
        return found != this->_wanted;
    }, sizeof(uint8) + sizeof(uint32) + SHA_DIGEST_LENGTH, sizeof(uint8), comment, flags)
{
    // the game depends on uppercase module names being sent
    std::transform(_module.begin(), _module.end(), _module.begin(), ::toupper);
}

WindowsModuleScan::WindowsModuleScan(const std::string &module, CheckT checker, const std::string &comment, uint32 flags)
    : _module(module),
    WindowsScan(
    // builder
    [this](const Warden *warden, std::vector<std::string> &, ByteBuffer &scan)
    {
        auto const winWarden = reinterpret_cast<const WardenWin *>(warden);
        auto const seed = static_cast<uint32>(urand());

        scan << static_cast<uint8>(winWarden->GetModule()->opcodes[FIND_MODULE_BY_NAME] ^ winWarden->GetXor()) << seed;

        HMACSHA1 hash(sizeof(seed), reinterpret_cast<const uint8*>(&seed));
        hash.UpdateData(this->_module);
        hash.Finalize();

        scan.append(hash.GetDigest(), hash.GetLength());
    },
    checker, sizeof(uint8) + sizeof(uint32) + SHA_DIGEST_LENGTH, sizeof(uint8), comment, flags)
{
    // the game depends on uppercase module names being sent
    std::transform(_module.begin(), _module.end(), _module.begin(), ::toupper);
}

WindowsMemoryScan::WindowsMemoryScan(uint32 offset, const void *expected, size_t length, const std::string &comment, uint32 flags)
    : _expected(length), _offset(offset),
    WindowsScan(
    // builder
    [this](const Warden *warden, std::vector<std::string> &, ByteBuffer &scan)
    {
        auto const winWarden = reinterpret_cast<const WardenWin *>(warden);

        scan << static_cast<uint8>(winWarden->GetModule()->opcodes[READ_MEMORY] ^ winWarden->GetXor())
             << static_cast<uint8>(0)   // no string associated with this form of the constructor
             << this->_offset
             << static_cast<uint8>(this->_expected.size());
    },
    // checker
    [this](const Warden *, ByteBuffer &buff)
    {
        // non-zero value indicates failure
        if (!!buff.read<uint8>())
            return true;

        auto const result = !!memcmp(buff.contents() + buff.rpos(), &this->_expected[0], this->_expected.size());
        buff.rpos(buff.rpos() + this->_expected.size());
        return result;
    }, sizeof(uint8) + sizeof(uint8) + sizeof(uint32) + sizeof(uint8), sizeof(uint8) + length, comment, flags)
{
    // must fit within uint8
    MANGOS_ASSERT(_expected.size() <= 0xFF);

    ::memcpy(&_expected[0], expected, _expected.size());
}

WindowsMemoryScan::WindowsMemoryScan(const std::string &module, uint32 offset, const void *expected, size_t length, const std::string &comment, uint32 flags)
    : _expected(length), _offset(offset), _module(module),
    WindowsScan(
    // builder
    [this](const Warden *warden, std::vector<std::string> &strings, ByteBuffer &scan)
    {
        MANGOS_ASSERT(strings.size() < 0xFF);

        strings.emplace_back(this->_module);

        auto const winWarden = reinterpret_cast<const WardenWin *>(warden);

        scan << static_cast<uint8>(winWarden->GetModule()->opcodes[READ_MEMORY] ^ winWarden->GetXor())
             << static_cast<uint8>(strings.size())
             << this->_offset
             << static_cast<uint8>(this->_expected.size());
    },
    // checker
    [this](const Warden *, ByteBuffer &buff)
    {
        // non-zero value indicates failure
        if (!!buff.read<uint8>())
            return true;

        auto const result = !!memcmp(buff.contents() + buff.rpos(), &this->_expected[0], this->_expected.size());
        buff.rpos(buff.rpos() + this->_expected.size());
        return result;
    }, module.length() + sizeof(uint8) + sizeof(uint8) + sizeof(uint32) + sizeof(uint8), sizeof(uint8) + length, comment, flags)
{
    // must fit within uint8
    MANGOS_ASSERT(_expected.size() <= 0xFF);

    ::memcpy(&_expected[0], expected, _expected.size());

    // since this scan uses GetModuleHandle() rather than GetFirstModule()/GetNextModule(), this is case insensitive.
    // but still it seems prudent to be consistent
    std::transform(_module.begin(), _module.end(), _module.begin(), ::toupper);
}

WindowsMemoryScan::WindowsMemoryScan(uint32 offset, size_t length, CheckT checker, const std::string &comment, uint32 flags)
    : _expected(length), _offset(offset),
    WindowsScan(
    // builder
    [this](const Warden *warden, std::vector<std::string> &, ByteBuffer &scan)
    {
        auto const winWarden = reinterpret_cast<const WardenWin *>(warden);

        scan << static_cast<uint8>(winWarden->GetModule()->opcodes[READ_MEMORY] ^ winWarden->GetXor())
             << static_cast<uint8>(0)   // no string associated with this form of the constructor
             << this->_offset
             << static_cast<uint8>(this->_expected.size());
    }, checker, sizeof(uint8) + sizeof(uint8) + sizeof(uint32) + sizeof(uint8), sizeof(uint8) + length, comment, flags)
{
    MANGOS_ASSERT(_expected.size() <= 0xFF);
}

WindowsMemoryScan::WindowsMemoryScan(const std::string &module, uint32 offset, size_t length, CheckT checker, const std::string &comment, uint32 flags)
    : _expected(length), _offset(offset), _module(module),
    WindowsScan(
    // builder
    [this](const Warden *warden, std::vector<std::string> &strings, ByteBuffer &scan)
    {
        MANGOS_ASSERT(strings.size() < 0xFF);

        strings.emplace_back(this->_module);

        auto const winWarden = reinterpret_cast<const WardenWin *>(warden);

        scan << static_cast<uint8>(winWarden->GetModule()->opcodes[READ_MEMORY] ^ winWarden->GetXor())
             << static_cast<uint8>(strings.size())
             << this->_offset
             << static_cast<uint8>(this->_expected.size());
    }, checker, module.length() + sizeof(uint8) + sizeof(uint8) + sizeof(uint32) + sizeof(uint8), sizeof(uint8) + length, comment, flags) {}

WindowsCodeScan::WindowsCodeScan(uint32 offset, const std::vector<uint8> &pattern, bool memImageOnly, bool wanted, const std::string &comment, uint32 flags)
    : _offset(offset), _pattern(pattern), _memImageOnly(memImageOnly), _wanted(wanted),
    WindowsScan(
    // builder
    [this](const Warden *warden, std::vector<std::string> &, ByteBuffer &scan)
    {
        auto const winWarden = reinterpret_cast<const WardenWin *>(warden);
        auto const seed = static_cast<uint32>(urand());

        scan << static_cast<uint8>(winWarden->GetModule()->opcodes[this->_memImageOnly ? FIND_MEM_IMAGE_CODE_BY_HASH : FIND_CODE_BY_HASH] ^ winWarden->GetXor())
             << seed;

        HMACSHA1 hash(sizeof(seed), reinterpret_cast<const uint8*>(&seed));
        hash.UpdateData(&this->_pattern[0], this->_pattern.size());
        hash.Finalize();
        
        scan.append(hash.GetDigest(), hash.GetLength());

        scan << this->_offset << static_cast<uint8>(this->_pattern.size());
    },
    [this](const Warden *, ByteBuffer &buff)
    {
        auto const found = buff.read<uint8>() == PatternFound;
        return found != this->_wanted;
    }, sizeof(uint8) + sizeof(uint32) + SHA_DIGEST_LENGTH + sizeof(uint32) + sizeof(uint8), sizeof(uint8), comment, flags)
{
    MANGOS_ASSERT(_pattern.size() <= 0xFF);
}

WindowsFileHashScan::WindowsFileHashScan(const std::string &file, const void *expected, bool wanted, const std::string &comment, uint32 flags)
    : _file(file), _wanted(wanted), _hashMatch(!!expected),
    WindowsScan(
    // builder
    [this](const Warden *warden, std::vector<std::string> &strings, ByteBuffer &scan)
    {
        MANGOS_ASSERT(strings.size() < 0xFF);

        strings.emplace_back(this->_file);

        auto const winWarden = reinterpret_cast<const WardenWin *>(warden);

        scan << static_cast<uint8>(winWarden->GetModule()->opcodes[HASH_CLIENT_FILE] ^ winWarden->GetXor())
             << static_cast<uint8>(strings.size());
    },
    // checker
    [this](const Warden *, ByteBuffer &buff)
    {
        auto const success = !buff.read<uint8>();

        // either we wanted it but didn't find it, or didn't want it but did find it
        if (this->_wanted != success)
            return true;

        // if we didn't want it and didn't find it, succeed
        if (!this->_wanted && !success)
            return false;

        uint8 hash[SHA_DIGEST_LENGTH];

        buff.read(hash, sizeof(hash));

        // if a hash was given, check it (some checks may only be interested in existence)
        return this->_hashMatch && !!memcmp(hash, this->_expected, sizeof(hash));
    }, sizeof(uint8) + sizeof(uint8) + file.length(), sizeof(uint8) + SHA_DIGEST_LENGTH, comment, flags)
{
    if (_hashMatch)
        ::memcpy(_expected, expected, sizeof(_expected));
}

WindowsLuaScan::WindowsLuaScan(const std::string &lua, bool wanted, const std::string &comment, uint32 flags)
    : _lua(lua), _wanted(wanted),
    WindowsScan(
    // builder
    [this](const Warden *warden, std::vector<std::string> &strings, ByteBuffer &scan)
    {
        MANGOS_ASSERT(strings.size() < 0xFF);

        strings.emplace_back(this->_lua);

        auto const winWarden = reinterpret_cast<const WardenWin *>(warden);

        scan << static_cast<uint8>(winWarden->GetModule()->opcodes[GET_LUA_VARIABLE] ^ winWarden->GetXor())
            << static_cast<uint8>(strings.size());
    },
    // checker
    [this](const Warden *, ByteBuffer &buff)
    {
        auto const found = !buff.read<uint8>();

        // if its found we have to 'read' the string, even if we don't care what it is
        if (found)
        {
            auto const length = buff.read<uint8>();
            buff.rpos(buff.rpos() + length);
        }

        return found != this->_wanted;
    }, sizeof(uint8) + sizeof(uint8) + lua.length(), sizeof(uint8) + 0xFF, comment, flags) {}

WindowsLuaScan::WindowsLuaScan(const std::string &lua, const std::string &expectedValue, const std::string &comment, uint32 flags)
    : _lua(lua), _expectedValue(expectedValue),
    WindowsScan(
    // builder
    [this](const Warden *warden, std::vector<std::string> &strings, ByteBuffer &scan)
    {
        MANGOS_ASSERT(strings.size() < 0xFF);

        strings.emplace_back(this->_lua);

        auto const winWarden = reinterpret_cast<const WardenWin *>(warden);

        scan << static_cast<uint8>(winWarden->GetModule()->opcodes[GET_LUA_VARIABLE] ^ winWarden->GetXor())
            << static_cast<uint8>(strings.size());
    },
    // checker
    [this](const Warden *, ByteBuffer &buff)
    {
        auto const result = buff.read<uint8>();

        if (!!result)
            return true;

        const size_t len = buff.read<uint8>();

        const std::string str(reinterpret_cast<const char *>(buff.contents() + buff.rpos()), len);

        buff.rpos(buff.rpos() + len);

        return str == this->_expectedValue;
    }, sizeof(uint8) + sizeof(uint8) + lua.length(), sizeof(uint8) + 0xFF, comment, flags)
{
    MANGOS_ASSERT(expectedValue.length() <= 0xFF);
}

WindowsLuaScan::WindowsLuaScan(const std::string &lua, CheckT checker, const std::string &comment, uint32 flags) : _lua(lua),
    WindowsScan(
    // builder
    [this](const Warden *warden, std::vector<std::string> &strings, ByteBuffer &scan)
    {
        MANGOS_ASSERT(strings.size() < 0xFF);

        strings.emplace_back(this->_lua);

        auto const winWarden = reinterpret_cast<const WardenWin *>(warden);

        scan << static_cast<uint8>(winWarden->GetModule()->opcodes[GET_LUA_VARIABLE] ^ winWarden->GetXor())
            << static_cast<uint8>(strings.size());
    }, checker, sizeof(uint8) + sizeof(uint8) + lua.length(), sizeof(uint8) + 0xFF, comment, flags)
{
    MANGOS_ASSERT(checker);
}

WindowsHookScan::WindowsHookScan(const std::string &module, const std::string &proc, const void *hash,
    uint32 offset, size_t length, const std::string &comment, uint32 flags)
    : _module(module), _proc(proc), _offset(offset), _length(length),
    WindowsScan(
    // builder
    [this](const Warden *warden, std::vector<std::string> &strings, ByteBuffer &scan)
    {
        MANGOS_ASSERT(strings.size() < 0xFE);

        strings.emplace_back(this->_module);
        strings.emplace_back(this->_proc);

        auto const winWarden = reinterpret_cast<const WardenWin *>(warden);
        auto const seed = static_cast<uint32>(urand());

        scan << static_cast<uint8>(winWarden->GetModule()->opcodes[API_CHECK] ^ winWarden->GetXor()) << seed;

        scan.append(this->_hash, sizeof(this->_hash));
        scan << static_cast<uint8>(strings.size() - 1)
             << static_cast<uint8>(strings.size())
             << this->_offset
             << static_cast<uint8>(this->_length);
    },
    // checker
    [](const Warden *, ByteBuffer &buff)
    {
        return buff.read<uint8>() == Detoured;
    },
        sizeof(uint8) + sizeof(uint32) + SHA_DIGEST_LENGTH + sizeof(uint8) + sizeof(uint8) + sizeof(uint32) + sizeof(uint8),
        sizeof(uint8), comment, flags)
{
    MANGOS_ASSERT(length <= 0xFF);
    ::memcpy(_hash, hash, sizeof(_hash));
}

WindowsDriverScan::WindowsDriverScan(const std::string &name, const std::string &targetPath, bool wanted, const std::string &comment, uint32 flags)
    : _name(name), _wanted(wanted), _targetPath(targetPath),
    WindowsScan(
    // builder
    [this](const Warden *warden, std::vector<std::string> &strings, ByteBuffer &scan)
    {
        MANGOS_ASSERT(strings.size() < 0xFF);

        strings.emplace_back(this->_name);

        auto const winWarden = reinterpret_cast<const WardenWin *>(warden);
        auto const seed = static_cast<uint32>(urand());

        scan << static_cast<uint8>(winWarden->GetModule()->opcodes[FIND_DRIVER_BY_NAME] ^ winWarden->GetXor()) << seed;

        HMACSHA1 hash(sizeof(seed), reinterpret_cast<const uint8*>(&seed));
        hash.UpdateData(this->_targetPath);
        hash.Finalize();

        scan.append(hash.GetDigest(), hash.GetLength());
        scan << static_cast<uint8>(strings.size());
    },
    // checker
    [this](const Warden *, ByteBuffer &buff)
    {
        auto const found = buff.read<uint8>() == Found;

        return found != this->_wanted;
    }, sizeof(uint8) + sizeof(uint32) + SHA_DIGEST_LENGTH + sizeof(uint8) + name.length(), sizeof(uint8), comment, flags) {}

WindowsTimeScan::WindowsTimeScan(CheckT checker, const std::string &comment, uint32 flags) :
    WindowsScan(
    // builder
    [this](const Warden *warden, std::vector<std::string> &, ByteBuffer &scan)
    {
        auto const winWarden = reinterpret_cast<const WardenWin *>(warden);
        scan << static_cast<uint8>(winWarden->GetModule()->opcodes[CHECK_TIMING_VALUES] ^ winWarden->GetXor());
    }, checker, sizeof(uint8), sizeof(uint8) + sizeof(uint32), comment, flags)
{
    MANGOS_ASSERT(!!checker);
}
