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

#include "Auth/Sha1.h"
#include "Auth/BigNumber.h"
#include <stdarg.h>

Sha1Hash::Sha1Hash()
{
    m_ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(m_ctx, EVP_sha1(), nullptr);
}

Sha1Hash::Sha1Hash(const Sha1Hash& other) : Sha1Hash()  // copy
{
    EVP_MD_CTX_copy_ex(m_ctx, other.m_ctx);
    std::memcpy(m_digest, other.m_digest, SHA_DIGEST_LENGTH);
}

Sha1Hash::Sha1Hash(Sha1Hash&& other) : Sha1Hash()       // move
{
    swap(other);
}

Sha1Hash& Sha1Hash::operator=(Sha1Hash other)           // assign
{
    swap(other);
    return *this;
}

Sha1Hash::~Sha1Hash()
{
    EVP_MD_CTX_free(m_ctx);
}

void Sha1Hash::swap(Sha1Hash& other) throw()
{
    std::swap(m_ctx, other.m_ctx);
    std::swap(m_digest, other.m_digest);
}

void Sha1Hash::UpdateData(const uint8* dta, int len)
{
    EVP_DigestUpdate(m_ctx, dta, len);
}

void Sha1Hash::UpdateData(const std::vector<uint8>& data)
{
    UpdateData(data.data(), data.size());
}

void Sha1Hash::UpdateData(const std::string& str)
{
    UpdateData((uint8 const*)str.c_str(), str.length());
}

void Sha1Hash::UpdateBigNumbers(BigNumber* bn0, ...)
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

void Sha1Hash::Initialize()
{
    EVP_DigestInit(m_ctx, EVP_sha1());
}

void Sha1Hash::Finalize(void)
{
    uint32 length = SHA_DIGEST_LENGTH;
    EVP_DigestFinal_ex(m_ctx, m_digest, &length);
}
