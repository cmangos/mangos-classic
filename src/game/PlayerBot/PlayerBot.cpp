#include "PlayerBot.h"
#include "World.h"

PlayerBot::PlayerBot(uint32 accountId)
{
    InitializeSession(accountId);
}

PlayerBot::~PlayerBot()
{
    Logout();
}

void PlayerBot::InitializeSession(uint32 accountId)
{
    QueryResult* result =
        LoginDatabase.PQuery("SELECT "
            "gmlevel, "                 //1
            "expansion, "               //7
            "mutetime, "                //8
            "locale "                   //9
            "FROM account "
            "WHERE id = '%u'",
            accountId);

    // Stop if the account is not found
    if (!result)
    {
        return;
    }

    Field* fields = result->Fetch();
    uint32 security = fields[0].GetUInt16();
    uint8 expansion = fields[1].GetUInt8();
    time_t mutetime = time_t(fields[2].GetUInt64());
    LocaleConstant locale = LocaleConstant(fields[3].GetUInt8());
    WorldSocket* sock = nullptr;

    WorldSession* session = new WorldSession(accountId, sock, AccountTypes(security), expansion, mutetime, locale);
    if (!session)
    {
        return;
    }

    session->SetIsBot(true);
    sWorld.AddSession(session);

    m_session = session;
}

void PlayerBot::UpdateAI(const uint32 diff)
{
    if (!IsLoggedIn())
        return;

    // do stuff
}

bool PlayerBot::Login(ObjectGuid characterId)
{
    if (IsLoggedIn())
        return false;

    m_session->HandleBotPlayerLogin(characterId);

    return true;
}

bool PlayerBot::Logout()
{
    if (!IsLoggedIn())
        return true;

    m_session->LogoutPlayer(true);

    return true;

}

bool PlayerBot::IsLoggedIn()
{
    if (!m_session)
        return false;

    return m_session->GetPlayer();
}

Player* PlayerBot::GetPlayer()
{
    return IsLoggedIn() ? m_session->GetPlayer() : nullptr;
}