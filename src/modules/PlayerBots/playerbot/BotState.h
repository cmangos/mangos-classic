#pragma once

enum class BotState : uint8
{
    BOT_STATE_COMBAT = 0,
    BOT_STATE_NON_COMBAT = 1,
    BOT_STATE_DEAD = 2,
    BOT_STATE_REACTION = 3,
    BOT_STATE_ALL
};