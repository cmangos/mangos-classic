/*
 *  * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *   *
 *    * This program is free software; you can redistribute it and/or modify
 *     * it under the terms of the GNU General Public License as published by
 *      * the Free Software Foundation; either version 2 of the License, or
 *       * (at your option) any later version.
 *        *
 *         * This program is distributed in the hope that it will be useful,
 *          * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *           * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *            * GNU General Public License for more details.
 *             *
 *              * You should have received a copy of the GNU General Public License
 *               * along with this program; if not, write to the Free Software
 *                * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *                 */

#include "Auth/Hmac.h"
#include "BigNumber.h"

HmacHash::HmacHash(const uint8* data, int length)
{
	    HMAC_CTX_init(&m_ctx);
	        HMAC_Init_ex(&m_ctx, data, length, EVP_sha1(), NULL);
}

HmacHash::~HmacHash()
{
	    HMAC_CTX_cleanup(&m_ctx);
}

void HmacHash::UpdateBigNumber(BigNumber* bn)
{
	    UpdateData(bn->AsByteArray(), bn->GetNumBytes());
}

void HmacHash::UpdateData(const uint8* data, int length)
{
	    HMAC_Update(&m_ctx, data, length);
}

void HmacHash::Finalize()
{
	    uint32 length = 0;
	        HMAC_Final(&m_ctx, m_digest, &length);
		    MANGOS_ASSERT(length == SHA_DIGEST_LENGTH);
}
