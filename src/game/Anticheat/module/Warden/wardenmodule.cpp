/*
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 *
 * This is private software and may not be shared under any circumstances,
 * absent permission of namreeb.
 */

#include "WardenModule.hpp"

#include "Platform/Define.h"
#include "Auth/CryptoHash.h"

#include <zlib.h>

#include <string>
#include <vector>
#include <fstream>
#include <exception>

WardenModule::WardenModule(std::string const &bin, std::string const &kf, std::string const &cr)
{
    std::ifstream b(bin, std::ios::binary | std::ios::ate);

    if (b.fail())
        throw std::runtime_error("Failed to open Warden module binary");

    if (b.tellg() < SignatureSize + 4)
        throw std::runtime_error("Warden module too small.  Must be at least 264 bytes.");

    binary.resize(static_cast<size_t>(b.tellg()));
    b.seekg(0, std::ios::beg);

    if (!b.read(reinterpret_cast<char *>(&binary[0]), binary.size()))
        throw std::runtime_error("Failed to open: " + bin);

    b.close();

    // compute md5 hash of encrypted/compressed data
    MD5Hash md5;
    md5.UpdateData(&binary[0], binary.size());
    md5.Finalize();
    hash.resize(md5.GetLength());
    std::memcpy(hash.data(), md5.GetDigest(), md5.GetLength());

    std::ifstream k(kf, std::ios::binary | std::ios::ate);

    if (k.fail())
        throw std::runtime_error("Failed to open Warden module key file");

    if (static_cast<size_t>(k.tellg()) != KeySize)
        throw std::runtime_error("Key file " + kf + " is incorrect size!");

    key.resize(KeySize);
    k.seekg(0, std::ios::beg);

    if (!k.read(reinterpret_cast<char *>(&key[0]), key.size()))
        throw std::runtime_error("Failed to open " + kf);

    k.close();

    std::ifstream c(cr, std::ios::binary | std::ios::ate);

    if (c.fail())
        throw std::runtime_error("Failed to open Warden challenge/response file");

    auto const crSize = static_cast<size_t>(c.tellg()) - sizeof(opcodes) - sizeof(memoryRead) - sizeof(pageScanCheck);

    if (!!(crSize % sizeof(ChallengeResponseEntry)))
        throw std::runtime_error("Unexpected data in Warden challenge/response file");

    crk.resize(crSize / sizeof(ChallengeResponseEntry));

    c.seekg(0, std::ios::beg);

    c.read(reinterpret_cast<char *>(&memoryRead), sizeof(memoryRead));
    c.read(reinterpret_cast<char *>(&pageScanCheck), sizeof(pageScanCheck));
    c.read(reinterpret_cast<char *>(&opcodes), sizeof(opcodes));
    c.read(reinterpret_cast<char *>(&crk[0]), crSize);

    c.close();

    if (Windows())
    {
        if (!memoryRead)
            throw std::runtime_error("Module data does not include memory read information");

        if (!pageScanCheck)
            throw std::runtime_error("Module data does not include page scan check information");
    }
}

bool WardenModule::Windows() const
{
    // these values will be all zero for OSX modules

    for (auto i = 0; i < sizeof(opcodes); ++i)
        if (!!opcodes[i])
            return true;

    return false;
}
