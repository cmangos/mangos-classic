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

#include "Common.h"
#include "Auth/CryptoHash.h"
#include "SRP6.h"

SRP6::SRP6()
{
    N.SetHexStr("894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7");
    g.SetDword(7);
}

void SRP6::CalculateHostPublicEphemeral(void)
{
    b.SetRand(19 * 8);
    BigNumber gmod = g.ModExp(b, N);
    B = ((v * 3) + gmod) % N;

    MANGOS_ASSERT(gmod.GetNumBytes() <= 32);
}

void SRP6::CalculateProof(std::string username)
{
    uint8 hash[20];

    Sha1Hash sha;
    sha.Initialize();
    sha.UpdateBigNumbers(&N, nullptr);
    sha.Finalize();
    memcpy(hash, sha.GetDigest(), 20);
    sha.Initialize();
    sha.UpdateBigNumbers(&g, nullptr);
    sha.Finalize();
    for (int i = 0; i < 20; ++i)
    {
        hash[i] ^= sha.GetDigest()[i];
    }

    sha.Initialize();
    sha.UpdateData(username);
    sha.Finalize();
    uint8 t4[Sha1Hash::GetLength()];
    memcpy(t4, sha.GetDigest(), Sha1Hash::GetLength());

    // The client hashes s, A and B on 32 bytes and K on 40 bytes, whatever their value: a number with leading zero
    // bytes must not be hashed shorter
    sha.Initialize();
    sha.UpdateData(hash, 20);
    sha.UpdateData(t4, Sha1Hash::GetLength());
    sha.UpdateData(s.AsByteArray(32));
    sha.UpdateData(A.AsByteArray(32));
    sha.UpdateData(B.AsByteArray(32));
    sha.UpdateData(K.AsByteArray(40));
    sha.Finalize();
    M.SetBinary(sha.GetDigest(), 20);
}

bool SRP6::CalculateSessionKey(uint8* lp_A, int l)
{
    A.SetBinary(lp_A, l);

    // SRP safeguard: abort if A==0
    if (A.isZero())
        return false;

    if ((A % N).isZero())
        return false;

    Sha1Hash sha;
    sha.UpdateData(A.AsByteArray(32));
    sha.UpdateData(B.AsByteArray(32));
    sha.Finalize();
    u.SetBinary(sha.GetDigest(), 20);
    S = (A * (v.ModExp(u, N))).ModExp(b, N);

    return true;
}

bool SRP6::CalculateVerifier(const std::string& rI)
{
    BigNumber salt;
    salt.SetRand(s_BYTE_SIZE * 8);
    const char* _salt = salt.AsHexStr();
    bool ret = CalculateVerifier(rI, _salt);
    OPENSSL_free((void*)_salt);
    return ret;
}

bool SRP6::CalculateVerifier(const std::string& rI, const char* salt)
{
    if (s.SetHexStr(salt) == 0 || s.isZero())
        return false;

    BigNumber I;
    I.SetHexStr(rI.c_str());

    // in case of leading zeros in the rI hash, restore them
    uint8 mDigest[Sha1Hash::GetLength()];
    memset(mDigest, 0, Sha1Hash::GetLength());
    if (I.GetNumBytes() <= Sha1Hash::GetLength())
    {
        auto const vect_I = I.AsByteArray();
        memcpy(mDigest, vect_I.data(), vect_I.size());
    }

    std::reverse(mDigest, mDigest + Sha1Hash::GetLength());

    Sha1Hash sha;
    sha.UpdateData(s.AsByteArray());
    sha.UpdateData(mDigest, Sha1Hash::GetLength());
    sha.Finalize();
    BigNumber x;
    x.SetBinary(sha.GetDigest(), Sha1Hash::GetLength());
    v = g.ModExp(x, N);

    return true;
}

void SRP6::HashSessionKey(void)
{
    // Same interleave as the client: S as 32 little-endian bytes, without its leading zero bytes, and without one more
    // byte if the remaining length is odd. The even and odd bytes are hashed apart and interleaved back into K (40 bytes).
    const std::vector<uint8> t = S.AsByteArray(32);
    size_t skip = 0;
    while (skip < t.size() && t[skip] == 0)
        ++skip;
    if ((t.size() - skip) % 2)
        ++skip;

    const size_t half = (t.size() - skip) / 2;
    std::vector<uint8> even(half), odd(half);
    for (size_t i = 0; i < half; ++i)
    {
        even[i] = t[skip + i * 2];
        odd[i] = t[skip + i * 2 + 1];
    }

    uint8 vK[40];
    Sha1Hash sha;
    sha.Initialize();
    sha.UpdateData(even);
    sha.Finalize();
    for (int i = 0; i < 20; ++i)
        vK[i * 2] = sha.GetDigest()[i];

    sha.Initialize();
    sha.UpdateData(odd);
    sha.Finalize();
    for (int i = 0; i < 20; ++i)
        vK[i * 2 + 1] = sha.GetDigest()[i];

    K.SetBinary(vK, 40);
}

bool SRP6::Proof(uint8* lp_M, int l)
{
    // Needs a minimum length l for M's byte array to
    // avoid memcmp accessing data out of bounds!
    if (!memcmp(M.AsByteArray(l).data(), lp_M, l))
        return false;

    return true;
}

bool SRP6::ProofVerifier(std::string vC)
{
    const char* vC_hex = vC.c_str();
    const char* v_hex = v.AsHexStr();

    if (memcmp(vC_hex, v_hex, strlen(vC_hex)) == 0)
    {
        OPENSSL_free((void*)v_hex);
        return true;
    }

    OPENSSL_free((void*)v_hex);
    return false;
}

void SRP6::Finalize(Sha1Hash& sha)
{
    // A on 32 bytes, M on 20, K on 40: the sizes the client uses
    sha.Initialize();
    sha.UpdateData(A.AsByteArray(32));
    sha.UpdateData(M.AsByteArray(20));
    sha.UpdateData(K.AsByteArray(40));
    sha.Finalize();
}

bool SRP6::SetSalt(const char* new_s)
{
    if (s.SetHexStr(new_s) == 0 || s.isZero())
        return false;
    return true;
}
bool SRP6::SetVerifier(const char* new_v)
{
    if (v.SetHexStr(new_v) == 0 || v.isZero())
        return false;
    return true;
}
