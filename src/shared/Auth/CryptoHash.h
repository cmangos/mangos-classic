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

#ifndef _AUTH_CRYPTOHASH_H
#define _AUTH_CRYPTOHASH_H

#include "Common.h"
#include "Auth/BigNumber.h"

#include <cstdarg>
#include <array>
#include <algorithm>
#include <iterator>

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

struct CryptoHashImpl
{
    typedef EVP_MD const* (*Algorithm)();
};

template <CryptoHashImpl::Algorithm Algorithm, size_t DigestSize>
class CryptoHash
{
    public:
        CryptoHash() : m_ctx(EVP_MD_CTX_new())
        {
            EVP_DigestInit_ex(m_ctx, Algorithm(), nullptr);
        }
        CryptoHash(CryptoHash const& other) : m_ctx(EVP_MD_CTX_new())   // copy
        {
            EVP_MD_CTX_copy_ex(m_ctx, other.m_ctx);
            std::copy(std::begin(other.m_digest), std::end(other.m_digest), std::begin(m_digest));
        }
        CryptoHash(CryptoHash&& other)                                  // move
        {
            swap(other);
        }
        CryptoHash& operator=(CryptoHash other)                         // assign
        {
            swap(other);
            return *this;
        }
        ~CryptoHash()
        {
            EVP_MD_CTX_free(m_ctx);
        }

        void swap(CryptoHash& other)
        {
            std::swap(m_ctx, other.m_ctx);
            std::swap(m_digest, other.m_digest);
        }
        friend void swap(CryptoHash& left, CryptoHash& right)
        {
            left.swap(right);
        }

        void UpdateBigNumbers(BigNumber* bn0, ...)
        {
            va_list v;

            va_start(v, bn0);
            BigNumber* bn = bn0;
            while (bn)
            {
                UpdateData(bn->AsByteArray());
                bn = va_arg(v, BigNumber*);
            }
            va_end(v);
        }
        int UpdateData(const uint8* data, int len)      { return EVP_DigestUpdate(m_ctx, data, len); }
        int UpdateData(const std::vector<uint8>& data)  { return UpdateData(data.data(), data.size()); }
        int UpdateData(const std::string& str)          { return UpdateData(reinterpret_cast<const uint8*>(str.data()), str.length()); }

        int Initialize()                                { return EVP_DigestInit(m_ctx, Algorithm()); }
        int Finalize()                                  { return EVP_DigestFinal_ex(m_ctx, m_digest.data(), nullptr); }

        uint8* GetDigest()                              { return m_digest.data(); }
        constexpr static inline size_t GetLength()      { return DigestSize; }

    private:
        EVP_MD_CTX* m_ctx;
        std::array<uint8, DigestSize> m_digest;
};

using Sha1Hash = CryptoHash<EVP_sha1, SHA_DIGEST_LENGTH>;
using MD5Hash = CryptoHash<EVP_md5, MD5_DIGEST_LENGTH>;

#endif
