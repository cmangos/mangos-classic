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

#include "Auth/HMACSHA1.h"
#include "BigNumber.h"

HMACSHA1::HMACSHA1(uint32 len, uint8 const* seed)
{
    memcpy(&m_key, seed, len);
    m_mac = EVP_MAC_fetch(nullptr, "HMAC", nullptr);
    m_ctx = EVP_MAC_CTX_new(m_mac);

    OSSL_PARAM params[2];
    params[0] = OSSL_PARAM_construct_utf8_string("digest", const_cast<char*>("SHA1"), 0);
    params[1] = OSSL_PARAM_construct_end();

    EVP_MAC_init(m_ctx, m_key, len, params);
}

HMACSHA1::HMACSHA1(uint32 len, uint8 const* seed, bool) // to get over the default constructor
{
    m_mac = EVP_MAC_fetch(nullptr, "HMAC", nullptr);
    m_ctx = EVP_MAC_CTX_new(m_mac);

    OSSL_PARAM params[2];
    params[0] = OSSL_PARAM_construct_utf8_string("digest", const_cast<char*>("SHA1"), 0);
    params[1] = OSSL_PARAM_construct_end();

    EVP_MAC_init(m_ctx, seed, len, params);
}

HMACSHA1::~HMACSHA1()
{
    memset(&m_key, 0x00, SEED_KEY_SIZE);
    EVP_MAC_CTX_free(m_ctx);
    EVP_MAC_free(m_mac);
}

void HMACSHA1::UpdateBigNumber(BigNumber* bn)
{
    UpdateData(bn->AsByteArray().data(), bn->GetNumBytes());
}

void HMACSHA1::UpdateData(const uint8* data, int length)
{
    EVP_MAC_update(m_ctx, data, length);
}

void HMACSHA1::UpdateData(const std::string& str)
{
    UpdateData(reinterpret_cast<const uint8*>(str.c_str()), str.length());
}

void HMACSHA1::Initialize()
{
    OSSL_PARAM params[2];
    params[0] = OSSL_PARAM_construct_utf8_string("digest", const_cast<char*>("SHA1"), 0);
    params[1] = OSSL_PARAM_construct_end();

    EVP_MAC_init(m_ctx, m_key, SEED_KEY_SIZE, params);
}

void HMACSHA1::Finalize()
{
    size_t length = 0;
    EVP_MAC_final(m_ctx, m_digest, &length, sizeof(m_digest));
    MANGOS_ASSERT(length == SHA_DIGEST_LENGTH);
}

uint8* HMACSHA1::ComputeHash(BigNumber* bn)
{
    UpdateData(bn->AsByteArray().data(), bn->GetNumBytes());
    Finalize();
    return m_digest;
}
