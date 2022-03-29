/*
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 *
 * This is private software and may not be shared under any circumstances,
 * absent permission of namreeb.
 */

#ifndef __WARDENMODULE_HPP_
#define __WARDENMODULE_HPP_

#include "Platform/Define.h"

#include <string>
#include <vector>

#pragma pack(push, 1)
struct ChallengeResponseEntry
{
    uint8 seed[16];
    uint8 reply[20];
    uint8 clientKey[16];
    uint8 serverKey[16];
};
#pragma pack(pop)

class WardenModule
{
private:
    static constexpr size_t SignatureSize = 260;
    static constexpr size_t KeySize = 16;
    static constexpr size_t ScanTypeCount = 9;

public:
    // windows module
    WardenModule(std::string const &binary, std::string const &key, std::string const &cr);

    WardenModule(WardenModule &&other) = default;

    // true when this module is for windows (otherwise it is for Mac x86)
    bool Windows() const;

    // compressed and rc4 encrypted form of the module (as it should be sent to the client)
    std::vector<uint8> binary;

    // rc4 key to send to client
    std::vector<uint8> key;

    // md5 hash of 'm_binary'
    std::vector<uint8> hash;

    // offset into module of the memory reading function which is modified by some hacks
    std::uint32_t memoryRead;

    // offset into module of the page scan check function which is modified by some hacks
    std::uint32_t pageScanCheck;

    // mapping of scan function to the numerical value of the opcode
    uint8_t opcodes[ScanTypeCount];

    // pregenerated challenge, responses, and encryption keys for this module
    std::vector<ChallengeResponseEntry> crk;
};

#endif /*!__WARDENMODULE_HPP_*/