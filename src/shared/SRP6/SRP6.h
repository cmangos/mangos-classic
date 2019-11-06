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

#ifndef _SRP6_H
#define _SRP6_H

#include "Common.h"
#include "Auth/BigNumber.h"
#include "Auth/Sha1.h"
#include "ByteBuffer.h"

#define HMAC_RES_SIZE 20

/*! Secure Remote Password protocol */
class SRP6
{
    public:
        const static int s_BYTE_SIZE = 32; /*!< length of salt */

        //! Initializes SRP with a predefined prime (N) and generator module (g)
        SRP6(void);

        //! calculates the host public ephemeral (B)
        /*!
          generates also a random number as host private ephemeral (b)
        */
        void CalculateHostPublicEphemeral(void);

        //! calculates proof (M) of the strong session key (K)
        /*!
          \param username the unique identity of the account to authenticate
        */
        void CalculateProof(std::string username);

        //! calculates a session key (S) based on client public ephemeral (A)
        /*!
          safeguard conditions are (A != 0) and (A % N != 0)
          \param lp_A the client public ephemeral (A)
          \param l the length of client public ephemeral (A)
          \return true on valid safeguard conditions otherwise false
        */
        bool CalculateSessionKey(uint8* lp_A, int l);

        //! calculates the password verifier (v)
        /*!
          \param rI a sha1 hash of USERNAME:PASSWORD
          \return true on success otherwise false if s is faulty
        */
        bool CalculateVerifier(const std::string& rI);

        //! calculates the password verifier (v) based on a predefined salt (s)
        /*!
          \param rI a sha1 hash of USERNAME:PASSWORD
          \param salt a predefined salt (s)
          \return true on success otherwise false if s is faulty
        */
        bool CalculateVerifier(const std::string& rI, const char* salt);

        //! generates a strong session key (K) of session key (S)
        void HashSessionKey(void);

        //! compares proof (M) of strong session key (K)
        /*!
          \param lp_M client proof (M) of the strong session key (K)
          \param l the length of client proof (M)
          \return true if client and server proof matches otherwise false
        */
        bool Proof(uint8* lp_M, int l);

        //! compare password verifier (v)
        /*!
          verifies if provided password matches the password verifier (v)
          requires to use the same salt (s) which was initially used to compute v.
          \param vC predefined password verifier (v) read from database
          \return true if password verifier matches otherwise false
        */
        bool ProofVerifier(std::string vC);

        //! generate hash for proof of strong session key (K)
        /*!
          this hash has to be send to the client for client-side proof.
          client has to show it's proof first. If the server detects an incorrect proof
          it must abort without showing it's proof.
          \param sha reference to an empty Sha1Hash object
        */
        void Finalize(Sha1Hash& sha);

        BigNumber GetHostPublicEphemeral(void) { return B; };
        BigNumber GetGeneratorModulo(void) { return g; };
        BigNumber GetPrime(void) { return N; };
        BigNumber GetProof(void) { return M; };
        BigNumber GetSalt(void) { return s; };
        BigNumber GetStrongSessionKey(void) { return K; };
        BigNumber GetVerifier(void) { return v; };

        bool SetSalt(const char* new_s);
        void SetStrongSessionKey(const char* new_K) { K.SetHexStr(new_K); };
        bool SetVerifier(const char* new_v);

    private:
        BigNumber A, u, S;
        BigNumber N, s, g, v;
        BigNumber b, B;
        BigNumber K;
        BigNumber M;
};
#endif
