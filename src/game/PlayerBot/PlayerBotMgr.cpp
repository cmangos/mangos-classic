#include "PlayerBotMgr.h"

void PlayerBotMgr::UpdateAI(const uint32 diff)
{
    if (m_playerBots.size() < 1)
        return;

    for (auto itr = m_playerBots.begin(); itr != m_playerBots.end(); ++itr)
    {
        itr->second.UpdateAI(diff);
    }
}

bool PlayerBotMgr::LoginBot(uint32 botId, ObjectGuid characterId)
{
    auto itr = m_playerBots.find(botId);

    if (itr != m_playerBots.end())
    {
        return itr->second.Login(characterId);
    }
    else
        return false;
}

bool PlayerBotMgr::LogoutBot(uint32 botId)
{
    auto itr = m_playerBots.find(botId);

    if (itr != m_playerBots.end())
    {
        return itr->second.Logout();
    }
    else
        return false;
}

void PlayerBotMgr::LogoutAllBots()
{
    if (m_playerBots.size() < 1)
        return;

    for (auto itr = m_playerBots.begin(); itr != m_playerBots.end(); ++itr)
    {
        LogoutBot(itr->first);
    }
}

PlayerBotMap* PlayerBotMgr::GetBots()
{
    return &m_playerBots;
}

uint32 PlayerBotMgr::CreateBot(uint32 accountId)
{
    auto existingBot = m_playerBots.find(accountId);

    if (existingBot != m_playerBots.end())
        return accountId;

    m_playerBots.insert({ accountId, PlayerBot(accountId) });

    return accountId;
}

bool PlayerBotMgr::DestroyBot(uint32 botId)
{
    auto itr = m_playerBots.find(botId);

    if (itr != m_playerBots.end())
    {
        m_playerBots.erase(botId);
        return true;
    }
    else
        return false;
}