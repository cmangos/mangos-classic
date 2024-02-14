/*
 * Contains interface and calls that have to be implemented by the anticheat lib.
 */

#ifndef __ANTICHEAT_HPP_
#define __ANTICHEAT_HPP_

#include "Server/WorldPacket.h"
#include "Auth/BigNumber.h"
#include "Server/Opcodes.h"
#include "Chat/Chat.h"
#include "Util/Util.h"

#include <memory>
#include <string>
#include <zlib.h>

enum CheatAction
{
    // take no action
    CHEAT_ACTION_NONE           = 0x00,

    // informational logging to the anticheat table of the log database, and gm notification
    CHEAT_ACTION_INFO_LOG       = 0x01,

    // log to the anticheat table of the log database, notify gms, and prompt for action
    // NOTE: other actions combined with this one are considered acceptable candidates for
    // MANUAL action and are NOT performed automatically!
    CHEAT_ACTION_PROMPT_LOG     = 0x02,

    // disconnect the offending world session
    CHEAT_ACTION_KICK           = 0x04,

    // permanently ban the offending account
    CHEAT_ACTION_BAN_ACCOUNT    = 0x08,

    // permanently ban the offending ip address
    CHEAT_ACTION_BAN_IP         = 0x10,

    // permanently silences the offending account from whispers, mail, say, channels, and yells
    CHEAT_ACTION_SILENCE        = 0x20,
};

enum NormalizeFlags
{
    NF_CUT_COLOR            = 0x001,
    NF_REPLACE_WORDS        = 0x002,
    NF_CUT_SPACE            = 0x004,
    NF_CUT_CTRL             = 0x008,
    NF_CUT_PUNCT            = 0x010,
    NF_CUT_NUMBERS          = 0x020,
    NF_REPLACE_UNICODE      = 0x040,
    NF_REMOVE_REPEATS       = 0x080,
    NF_REMOVE_NON_LATIN     = 0x100
};

class WorldSession;
class Player;
struct AreaTableEntry;

// interface to anticheat session (one for each world session)
class SessionAnticheatInterface
{
    public:
        virtual ~SessionAnticheatInterface() = default;

        virtual void Update(uint32 diff) = 0;

        virtual bool IsSilenced() const = 0;

        // character enum packet has been built and is ready to send
        virtual void SendCharEnum(WorldPacket &&packet) = 0;

        virtual void NewPlayer() = 0;
        virtual void LeaveWorld() = 0;
        virtual void Disconnect() = 0;

        // addon checksum verification
        virtual bool ReadAddonInfo(WorldPacket *, WorldPacket &) = 0;

        // chat
        virtual void SendPlayerInfo(ChatHandler *) const = 0;

        // miscellaneous cheat entry
        virtual void RecordCheat(uint32 actionMask, const char *detector, const char *format, ...) = 0;

        // movement cheats
        virtual bool Movement(MovementInfo &mi, const WorldPacket &) = 0;
        virtual void TimeSkipped(const ObjectGuid &mover, uint32 ms) = 0;
        virtual bool ExtrapolateMovement(MovementInfo const& mi, uint32 diffMs, Position &pos) = 0;
        virtual bool SpeedChangeAck(MovementInfo &mi, const WorldPacket &packet, float newSpeed) = 0;
        virtual bool IsInKnockBack() const = 0;
        virtual void KnockBack(float speedxy, float speedz, float cos, float sin) = 0;
        virtual void OnExplore(const AreaTableEntry *p) = 0;
        virtual void Teleport(const Position &pos) = 0;

        virtual void OrderSent(uint16 opcode, uint32 counter) = 0;
        virtual void OrderAck(uint16 opcode, uint32 counter) = 0;

        // warden
        virtual void WardenPacket(WorldPacket &packet) = 0;

        // antispam
        virtual void AutoReply(const std::string &msg) = 0;
        virtual void Whisper(const std::string &msg, const ObjectGuid &to) = 0;
        virtual void Say(const std::string &msg) = 0;
        virtual void Yell(const std::string &msg) = 0;
        virtual void Channel(const std::string &msg) = 0;
        virtual void Mail(const std::string &subject, const std::string &body, const ObjectGuid &to) = 0;
        virtual void ChannelInvite(const std::string &channelName, const ObjectGuid &to) = 0;
        virtual void PartyInvite(const ObjectGuid& to) = 0;
};

// interface to anticheat system
class AnticheatLibInterface
{
    public:
        // this function needs to support executing at any time
        virtual void Reload() = 0;

        // run only on startup
        virtual void Initialize() = 0;

        // create anticheat session for a new world session
        virtual std::unique_ptr<SessionAnticheatInterface> NewSession(WorldSession *session, const BigNumber &K) = 0;

        // anti spam
        virtual bool ValidateGuildName(const std::string &name) const = 0;
        virtual std::string NormalizeString(const std::string &message, uint32 mask) = 0;

        // GM .anticheat command handler
        //virtual bool ChatCommand(ChatHandler *handler, const std::string &args) = 0;
};

AnticheatLibInterface* GetAnticheatLib();
#define sAnticheatLib (GetAnticheatLib())

class NullSessionAnticheat : public SessionAnticheatInterface
{
    private:
        WorldSession * const _session;
        bool _inKnockBack;

    public:
        NullSessionAnticheat(WorldSession *session) : _session(session), _inKnockBack(false) {}

        virtual void Update(uint32) override {}

        virtual bool IsSilenced() const override { return false; }

        // character enum packet has been built and is ready to send
        virtual void SendCharEnum(WorldPacket &&packet) override { _session->SendPacket(packet, true); }

        virtual void NewPlayer() override {} 
        virtual void LeaveWorld() override {};
        virtual void Disconnect() override {};

        // addon checksum verification
        virtual bool ReadAddonInfo(WorldPacket* Source, WorldPacket& Target) override
        {
            ByteBuffer AddOnPacked;
            uLongf AddonRealSize;
            uint32 CurrentPosition;
            uint32 TempValue;

            unsigned char tdata[256] =
            {
                0xC3, 0x5B, 0x50, 0x84, 0xB9, 0x3E, 0x32, 0x42, 0x8C, 0xD0, 0xC7, 0x48, 0xFA, 0x0E, 0x5D, 0x54,
                0x5A, 0xA3, 0x0E, 0x14, 0xBA, 0x9E, 0x0D, 0xB9, 0x5D, 0x8B, 0xEE, 0xB6, 0x84, 0x93, 0x45, 0x75,
                0xFF, 0x31, 0xFE, 0x2F, 0x64, 0x3F, 0x3D, 0x6D, 0x07, 0xD9, 0x44, 0x9B, 0x40, 0x85, 0x59, 0x34,
                0x4E, 0x10, 0xE1, 0xE7, 0x43, 0x69, 0xEF, 0x7C, 0x16, 0xFC, 0xB4, 0xED, 0x1B, 0x95, 0x28, 0xA8,
                0x23, 0x76, 0x51, 0x31, 0x57, 0x30, 0x2B, 0x79, 0x08, 0x50, 0x10, 0x1C, 0x4A, 0x1A, 0x2C, 0xC8,
                0x8B, 0x8F, 0x05, 0x2D, 0x22, 0x3D, 0xDB, 0x5A, 0x24, 0x7A, 0x0F, 0x13, 0x50, 0x37, 0x8F, 0x5A,
                0xCC, 0x9E, 0x04, 0x44, 0x0E, 0x87, 0x01, 0xD4, 0xA3, 0x15, 0x94, 0x16, 0x34, 0xC6, 0xC2, 0xC3,
                0xFB, 0x49, 0xFE, 0xE1, 0xF9, 0xDA, 0x8C, 0x50, 0x3C, 0xBE, 0x2C, 0xBB, 0x57, 0xED, 0x46, 0xB9,
                0xAD, 0x8B, 0xC6, 0xDF, 0x0E, 0xD6, 0x0F, 0xBE, 0x80, 0xB3, 0x8B, 0x1E, 0x77, 0xCF, 0xAD, 0x22,
                0xCF, 0xB7, 0x4B, 0xCF, 0xFB, 0xF0, 0x6B, 0x11, 0x45, 0x2D, 0x7A, 0x81, 0x18, 0xF2, 0x92, 0x7E,
                0x98, 0x56, 0x5D, 0x5E, 0x69, 0x72, 0x0A, 0x0D, 0x03, 0x0A, 0x85, 0xA2, 0x85, 0x9C, 0xCB, 0xFB,
                0x56, 0x6E, 0x8F, 0x44, 0xBB, 0x8F, 0x02, 0x22, 0x68, 0x63, 0x97, 0xBC, 0x85, 0xBA, 0xA8, 0xF7,
                0xB5, 0x40, 0x68, 0x3C, 0x77, 0x86, 0x6F, 0x4B, 0xD7, 0x88, 0xCA, 0x8A, 0xD7, 0xCE, 0x36, 0xF0,
                0x45, 0x6E, 0xD5, 0x64, 0x79, 0x0F, 0x17, 0xFC, 0x64, 0xDD, 0x10, 0x6F, 0xF3, 0xF5, 0xE0, 0xA6,
                0xC3, 0xFB, 0x1B, 0x8C, 0x29, 0xEF, 0x8E, 0xE5, 0x34, 0xCB, 0xD1, 0x2A, 0xCE, 0x79, 0xC3, 0x9A,
                0x0D, 0x36, 0xEA, 0x01, 0xE0, 0xAA, 0x91, 0x20, 0x54, 0xF0, 0x72, 0xD8, 0x1E, 0xC7, 0x89, 0xD2
            };

            // broken addon packet, can't be received from real client
            if (Source->rpos() + 4 > Source->size())
                return false;

            (*Source) >> TempValue;                                   // get real size of the packed structure

            // empty addon packet, nothing process, can't be received from real client
            if (!TempValue)
                return false;

            if (TempValue > 0xFFFFF)
            {
                // sLog.outError("WorldSession::ReadAddonsInfo addon info too big, size %u", TempValue);
                return false;
            }

            AddonRealSize = TempValue;                              // temp value because ZLIB only excepts uLongf

            CurrentPosition = Source->rpos();                       // get the position of the pointer in the structure

            AddOnPacked.resize(AddonRealSize);                      // resize target for zlib action

            if (uncompress(const_cast<uint8*>(AddOnPacked.contents()), &AddonRealSize, const_cast<uint8*>(Source->contents() + CurrentPosition), Source->size() - CurrentPosition) == Z_OK)
            {
                Target.Initialize(SMSG_ADDON_INFO);

                while (AddOnPacked.rpos() < AddOnPacked.size())
                {
                    std::string AddonNames;
                    uint8 unk6;
                    uint32 crc, unk7;

                    AddOnPacked >> AddonNames;

                    AddOnPacked >> crc >> unk7 >> unk6;

                    // sLog.outDebug("ADDON: Name:%s CRC:%x Unknown1 :%x Unknown2 :%x", AddonNames.c_str(), crc, unk7, unk6);

                    Target << (uint8)2;

                    uint8 unk1 = 1;
                    Target << (uint8)unk1;
                    if (unk1)
                    {
                        uint8 unk2 = crc != uint64(0x1c776d01);    // If addon is Standard addon CRC
                        Target << (uint8)unk2;
                        if (unk2)
                            Target.append(tdata, sizeof(tdata));

                        Target << (uint32)0;
                    }

                    uint8 unk3 = 0;
                    Target << (uint8)unk3;
                    if (unk3)
                    {
                        // String, 256
                    }
                }
            }
            else
            {
                //sLog.outError("Addon packet uncompress error :(");
                return false;
            }
            return true;
        }

        // chat
        virtual void SendPlayerInfo(ChatHandler *) const override {}

        // miscellaneous action
        virtual void RecordCheat(uint32 actionMask, const char *detector, const char *format, ...) override
        {
            if (!!(actionMask & CHEAT_ACTION_KICK))
                _session->KickPlayer();
        }

        // movement cheats
        virtual bool Movement(MovementInfo &, const WorldPacket &packet) override
        {
            if (packet.GetOpcode() == MSG_MOVE_FALL_LAND)
                _inKnockBack = false;

            return true;
        }
        virtual void TimeSkipped(const ObjectGuid &mover, uint32 ms) override {}
        virtual bool ExtrapolateMovement(MovementInfo const& mi, uint32 diffMs, Position &pos) override { return false; }
        virtual bool SpeedChangeAck(MovementInfo &, const WorldPacket &, float) override { return true; }
        virtual bool IsInKnockBack() const override { return _inKnockBack; }
        virtual void KnockBack(float speedxy, float speedz, float cos, float sin) override { _inKnockBack = true; }
        virtual void OnExplore(const AreaTableEntry *) override {}
        virtual void Teleport(const Position &) override {}

        virtual void OrderSent(uint16, uint32) override {}
        virtual void OrderAck(uint16, uint32) override {}

        // warden
        virtual void WardenPacket(WorldPacket &) override {}

        // antispam
        virtual void AutoReply(const std::string &msg) override {}
        virtual void Whisper(const std::string &msg, const ObjectGuid &to) override {}
        virtual void Say(const std::string &msg) override {}
        virtual void Yell(const std::string &msg) override {}
        virtual void Channel(const std::string &msg) override {}
        virtual void Mail(const std::string &subject, const std::string &body, const ObjectGuid &to) override {}
        virtual void ChannelInvite(const std::string& channelName, const ObjectGuid& to) override {}
        virtual void PartyInvite(const ObjectGuid& to) override {}
};

#ifdef USE_ANTICHEAT
#include "module/libanticheat.hpp"
#else
class NullAnticheatLib : public AnticheatLibInterface
{
    public:
        virtual void Reload() {}
        virtual void Initialize() {}

        virtual std::unique_ptr<SessionAnticheatInterface> NewSession(WorldSession *session, const BigNumber &)
        {
            return std::make_unique<NullSessionAnticheat>(session);
        }

        // anti spam
        virtual bool ValidateGuildName(const std::string &) const { return false; }
        virtual std::string NormalizeString(const std::string &message, uint32) { return message; }
        virtual void AddMessage(const std::string &, uint32, Player *, Player *) {}

        // GM .anticheat command handler
        virtual bool ChatCommand(ChatHandler *, const std::string &) { return false; }
};
#endif

#endif /*!__ANTICHEAT_HPP_*/
