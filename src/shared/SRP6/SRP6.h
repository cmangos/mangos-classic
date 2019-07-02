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

class SRP6
{
    public:
        const static int s_BYTE_SIZE = 32;

        SRP6(void);
        ~SRP6(void);

        void CalculateVerifier(const std::string& rI);

        const char* GetSalt(void) { return s_hex; };
        const char* GetVerifier(void) { return v_hex; };

    private:
        BigNumber N, s, g, v;

        const char* v_hex;
        const char* s_hex;
};
#endif
