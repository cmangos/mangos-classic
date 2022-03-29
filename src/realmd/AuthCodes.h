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

/** \file
    \ingroup realmd
*/

#ifndef _AUTHCODES_H
#define _AUTHCODES_H

#include "Common.h"

enum eAuthCmd
{
    CMD_AUTH_LOGON_CHALLENGE        = 0x00,
    CMD_AUTH_LOGON_PROOF            = 0x01,
    CMD_AUTH_RECONNECT_CHALLENGE    = 0x02,
    CMD_AUTH_RECONNECT_PROOF        = 0x03,
    CMD_REALM_LIST                  = 0x10,
    CMD_XFER_INITIATE               = 0x30,
    CMD_XFER_DATA                   = 0x31,
    // these opcodes no longer exist in currently supported client
    CMD_XFER_ACCEPT                 = 0x32,
    CMD_XFER_RESUME                 = 0x33,
    CMD_XFER_CANCEL                 = 0x34
};

// not used by us currently
enum eAuthSrvCmd
{
    CMD_GRUNT_AUTH_CHALLENGE        = 0x0,
    CMD_GRUNT_AUTH_VERIFY           = 0x2,
    CMD_GRUNT_CONN_PING             = 0x10,
    CMD_GRUNT_CONN_PONG             = 0x11,
    CMD_GRUNT_HELLO                 = 0x20,
    CMD_GRUNT_PROVESESSION          = 0x21,
    CMD_GRUNT_KICK                  = 0x24,
    CMD_GRUNT_PCWARNING             = 0x29,
    CMD_GRUNT_STRINGS               = 0x41,
    CMD_GRUNT_SUNKENUPDATE          = 0x44,
    CMD_GRUNT_SUNKEN_ONLINE         = 0x46
};

enum AuthLogonResult : uint8
{
    AUTH_LOGON_SUCCESS                      = 0x00,
    AUTH_LOGON_FAILED_UNKNOWN0              = 0x01,                 ///< ? Unable to connect
    AUTH_LOGON_FAILED_UNKNOWN1              = 0x02,                 ///< ? Unable to connect
    AUTH_LOGON_FAILED_BANNED                = 0x03,                 ///< This <game> account has been closed and is no longer available for use. Please go to <site>/banned.html for further information.
    AUTH_LOGON_FAILED_UNKNOWN_ACCOUNT       = 0x04,                 ///< The information you have entered is not valid. Please check the spelling of the account name and password. If you need help in retrieving a lost or stolen password, see <site> for more information
    AUTH_LOGON_FAILED_INCORRECT_PASSWORD    = 0x05,                 ///< The information you have entered is not valid. Please check the spelling of the account name and password. If you need help in retrieving a lost or stolen password, see <site> for more information
    // client reject next login attempts after this error, so in code used AUTH_LOGON_FAILED_UNKNOWN_ACCOUNT for both cases
    AUTH_LOGON_FAILED_ALREADY_ONLINE        = 0x06,                 ///< This account is already logged into <game>. Please check the spelling and try again.
    AUTH_LOGON_FAILED_NO_TIME               = 0x07,                 ///< You have used up your prepaid time for this account. Please purchase more to continue playing
    AUTH_LOGON_FAILED_DB_BUSY               = 0x08,                 ///< Could not log in to <game> at this time. Please try again later.
    AUTH_LOGON_FAILED_VERSION_INVALID       = 0x09,                 ///< Unable to validate game version. This may be caused by file corruption or interference of another program. Please visit <site> for more information and possible solutions to this issue.
    AUTH_LOGON_FAILED_VERSION_UPDATE        = 0x0A,                 ///< Downloading
    AUTH_LOGON_FAILED_INVALID_SERVER        = 0x0B,                 ///< Unable to connect
    AUTH_LOGON_FAILED_SUSPENDED             = 0x0C,                 ///< This <game> account has been temporarily suspended. Please go to <site>/banned.html for further information
    AUTH_LOGON_FAILED_FAIL_NOACCESS         = 0x0D,                 ///< Unable to connect
    AUTH_LOGON_SUCCESS_SURVEY               = 0x0E,                 ///< Connected.
    AUTH_LOGON_FAILED_PARENTCONTROL         = 0x0F,                 ///< Access to this account has been blocked by parental controls. Your settings may be changed in your account preferences at <site>
    AUTH_LOGON_FAILED_LOCKED_ENFORCED       = 0x10,                 ///< You have applied a lock to your account. You can change your locked status by calling your account lock phone number.
    AUTH_LOGON_FAILED_TRIAL_ENDED           = 0x11,                 ///< Your trial subscription has expired. Please visit <site> to upgrade your account.
    AUTH_LOGON_FAILED_USE_BNET              = 0x12,                 ///< AUTH_LOGON_FAILED_OTHER This account is now attached to a BNet account. Please login with your BNet account email address and password.
    // AUTH_LOGON_FAILED_OVERMIND_CONVERTED
    // AUTH_LOGON_FAILED_ANTI_INDULGENCE
    // AUTH_LOGON_FAILED_EXPIRED
    // AUTH_LOGON_FAILED_NO_GAME_ACCOUNT
    // AUTH_LOGON_FAILED_BILLING_LOCK
    // AUTH_LOGON_FAILED_IGR_WITHOUT_BNET
    // AUTH_LOGON_FAILED_AA_LOCK
    // AUTH_LOGON_FAILED_UNLOCKABLE_LOCK
    // AUTH_LOGON_FAILED_MUST_USE_BNET
    // AUTH_LOGON_FAILED_OTHER
};

#endif
