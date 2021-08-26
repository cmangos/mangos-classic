#include "Chat/Chat.h"
#include "World/World.h"
#include "Entities/Player.h"
#include "libanticheat.hpp"
#include "config.hpp"
#include "Antispam/antispammgr.hpp"
#include "Antispam/antispam.hpp"
#include "Globals/ObjectMgr.h"

bool ChatHandler::HandleAnticheatInfoCommand(char* args)
{
    Player *target = nullptr;

    if (!ExtractPlayerTarget(&args, &target))
    {
        return false;
    }

    if (auto const anticheat = dynamic_cast<const NamreebAnticheat::SessionAnticheat *>(target->GetSession()->GetAnticheat()))
    {
        PSendSysMessage("Anticheat info for %s", target->GetGuidStr().c_str());
        anticheat->SendCheatInfo(this);
    }
    else
    {
        PSendSysMessage("No anticheat session for %s, they may be a bot or anticheat is disabled", target->GetGuidStr().c_str());
    }
    return true;
}

bool ChatHandler::HandleAnticheatEnableCommand(char* args)
{
    PSendSysMessage("Anticheat Late March version");
    return true;
}

bool ChatHandler::HandleAnticheatSilenceCommand(char* args)
{
    uint32 AccountId = 0;
    if (!ExtractUInt32Base(&args, AccountId, 10))
    {
        return false;
    }

    sAntispamMgr.Silence(AccountId);

    PSendSysMessage("Silenced account %u", AccountId);
    return true;
}

bool ChatHandler::HandleAnticheatSpaminfoCommand(char* args)
{
    Player *target = nullptr;
    ObjectGuid playerGuid;

    if (!ExtractPlayerTarget(&args, &target, &playerGuid))
        return false;

    std::shared_ptr<NamreebAnticheat::Antispam> antispam;

    if (target)
        if (auto const anticheat = dynamic_cast<const NamreebAnticheat::SessionAnticheat *>(target->GetSession()->GetAnticheat()))
            antispam = anticheat->GetAntispam();

    if (!antispam)
    {
        // if we reach here, lookup cached information instead
        auto const accountId = sObjectMgr.GetPlayerAccountIdByGUID(playerGuid);

        antispam = sAntispamMgr.CheckCache(accountId);
    }

    if (!antispam)
        SendSysMessage("No antispam info available");
    else
        SendSysMessage(antispam->GetInfo().c_str());

    return true;
}

bool ChatHandler::HandleAnticheatFingerprintListCommand(char* args)
{
    uint32 fingerprintNum = 0;

    if (!ExtractUInt32Base(&args, fingerprintNum, 16))
    {
        return false;
    }

    // search all session with specified fingerprint
    sWorld.GetMessager().AddMessage([session = GetSession(), fingerprintNum](World* world)
    {
        int32 count = 0;

        std::stringstream str;

        world->ExecuteForAllSessions([&](auto& data)
        {
            const WorldSession& sess = data;
            const NamreebAnticheat::SessionAnticheat* anticheat = dynamic_cast<const NamreebAnticheat::SessionAnticheat*>(sess.GetAnticheat());

            if (!anticheat)
                return;

            if (anticheat->GetFingerprint() == fingerprintNum)
            {
                str << "Account: " << sess.GetAccountName() << " ID: " << sess.GetAccountId() << " IP: " << sess.GetRemoteAddress();

                if (auto const player = sess.GetPlayer())
                    str << " Player name: " << player->GetName();

                str << "\n";

                ++count;
            }
        });

        ChatHandler(session).PSendSysMessage("%s\nEnd of listing for fingerprint 0x%x.  Found %d matches.", str.str().data(), fingerprintNum, count);
    });
    return true;
}

bool ChatHandler::HandleAnticheatFingerprintHistoryCommand(char* args)
{
    uint32 fingerprintNum = 0;

    if (!ExtractUInt32Base(&args, fingerprintNum, 16))
    {
        return false;
    }

    PSendSysMessage("Listing history for fingerprint 0x%x.  Maximum history length from config: %u", fingerprintNum, sAnticheatConfig.GetFingerprintHistory());

    std::unique_ptr<QueryResult> result(LoginDatabase.PQuery("SELECT account, ip, realm, time FROM system_fingerprint_usage WHERE fingerprint = %u ORDER BY `time` DESC", fingerprintNum));

    int count = 0;
    if (result)
    {
        do
        {
            const Field* fields = result->Fetch();

            uint32 accountId = fields[0].GetUInt32();
            std::string ip = fields[1].GetCppString();
            uint32 realm = fields[2].GetUInt32();
            std::string time = fields[3].GetCppString();

            PSendSysMessage("Account ID: %u IP: %s Realm: %u Time: %s", accountId, ip.c_str(), realm, time.c_str());

            ++count;
        } while (result->NextRow());
    }

    PSendSysMessage("End of history for fingerprint 0x%x.  Found %d matches", fingerprintNum, count);
    return true;
}

bool ChatHandler::HandleAnticheatFingerprintAHistoryCommand(char* args)
{
    uint32 AccountId = 0;
    if (!ExtractUInt32Base(&args, AccountId, 10))
    {
        return false;
    }

    PSendSysMessage("Listing history for account %u.  Maximum length: %u", AccountId, sAnticheatConfig.GetFingerprintHistory());

    std::unique_ptr<QueryResult> result(LoginDatabase.PQuery(
        "SELECT fingerprint, ip, realm, time FROM system_fingerprint_usage WHERE account = %u ORDER BY `time` DESC LIMIT %u",
        AccountId, sAnticheatConfig.GetFingerprintHistory()));

    int count = 0;
    if (result)
    {
        do
        {
            const Field* fields = result->Fetch();

            uint32 fingerprint = fields[0].GetUInt32();
            std::string ip = fields[1].GetCppString();
            uint32 realm = fields[2].GetUInt32();
            std::string time = fields[3].GetCppString();

            PSendSysMessage("Fingerprint: 0x%x IP: %s Realm: %u Time: %s", fingerprint, ip.c_str(), realm, time.c_str());

            ++count;
        } while (result->NextRow());
    }

    PSendSysMessage("End of history for account %u.  Found %d matches", AccountId, count);
    return true;
}

bool ChatHandler::HandleAnticheatCheatinformCommand(char* args)
{
    // ACCOUNT FLAGS
    WorldSession* session = GetSession();

    if (!session->HasAccountFlag(ACCOUNT_FLAG_SHOW_ANTICHEAT))
    {
        session->AddAccountFlag(ACCOUNT_FLAG_SHOW_ANTICHEAT);
        LoginDatabase.PExecute("UPDATE account SET flags = flags | 0x%x WHERE id = %u", session->GetAccountId(), ACCOUNT_FLAG_SHOW_ANTICHEAT);

        SendSysMessage("Anticheat messages will be shown");
    }
    else
    {
        session->RemoveAccountFlag(ACCOUNT_FLAG_SHOW_ANTICHEAT);
        LoginDatabase.PExecute("UPDATE account SET flags = flags & ~0x%x WHERE id = %u", session->GetAccountId(), ACCOUNT_FLAG_SHOW_ANTICHEAT);

        SendSysMessage("Anticheat messages will be hidden");
    }
    return true;
}

bool ChatHandler::HandleAnticheatSpaminformCommand(char* args)
{
    // ACCOUNT FLAGS
    WorldSession* session = GetSession();

    if (!session->HasAccountFlag(ACCOUNT_FLAG_SHOW_ANTISPAM))
    {
        session->AddAccountFlag(ACCOUNT_FLAG_SHOW_ANTISPAM);
        LoginDatabase.PExecute("UPDATE account SET flags = flags | 0x%x WHERE id = %u", session->GetAccountId(), ACCOUNT_FLAG_SHOW_ANTISPAM);

        SendSysMessage("Antispam messages will be shown");
    }
    else
    {
        session->RemoveAccountFlag(ACCOUNT_FLAG_SHOW_ANTISPAM);
        LoginDatabase.PExecute("UPDATE account SET flags = flags & ~0x%x WHERE id = %u", session->GetAccountId(), ACCOUNT_FLAG_SHOW_ANTISPAM);

        SendSysMessage("Antispam messages will be hidden");
    }
    return true;
}

bool ChatHandler::HandleAnticheatBlacklistCommand(char* args)
{
    sAntispamMgr.BlacklistAdd(args);

    SendSysMessage("Blacklist add submitted");
    return true;
}

bool ChatHandler::HandleAnticheatUnsilenceCommand(char* args)
{
    uint32 AccountId = 0;
    if (!ExtractUInt32Base(&args, AccountId, 10))
    {
        return false;
    }

    sAntispamMgr.Unsilence(AccountId);

    PSendSysMessage("Unsilenced account %u", AccountId);
    return true;
}

bool ChatHandler::HandleAnticheatDebugExtrapCommand(char* args)
{
    uint32 seconds;    
    if (!ExtractUInt32Base(&args, seconds, 10))
        seconds = 30;

    if (seconds > HOUR)
    {
        SendSysMessage("Do not enable this for more than an hour");
        return false;
    }

    auto const anticheat = dynamic_cast<NamreebAnticheat::AnticheatLib *>(GetAnticheatLib());
    if (!anticheat)
    {
        SendSysMessage("No anticheat lib present");
        return false;
    }

    anticheat->EnableExtrapolationDebug(seconds);

    PSendSysMessage("Extrapolation debug enabled for %u seconds", seconds);

    return true;
}