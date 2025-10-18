
#include "playerbot/playerbot.h"
#include "CheatAction.h"

using namespace ai;

bool CheatAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (requester && requester->GetSession()->GetSecurity() >= SEC_GAMEMASTER)
    {
        std::string param = event.getParam();
        uint32 cheatMask = (uint32)ai->GetCheat();
        std::vector<std::string> splitted = split(param, ',');
        for (std::vector<std::string>::iterator i = splitted.begin(); i != splitted.end(); i++)
        {
            const char* name = i->c_str();
            BotCheatMask newCheat = GetCheatMask(name + 1);

            switch (name[0])
            {
            case '+':
                AddCheat(newCheat);
                cheatMask |= (uint32)newCheat;
                break;
            case '-':
                RemCheat(newCheat);
                cheatMask &= ~(uint32)newCheat;
                break;
            case '~':
                if (ai->HasCheat(newCheat))
                    RemCheat(newCheat);
                else
                    AddCheat(newCheat);

                cheatMask ^= (uint32)newCheat;
                break;
            case '?':
                ListCheats(requester);
                return true;
            }
        }

        ai->SetCheat(BotCheatMask(cheatMask));
        return true;
    }

    return false;
}

BotCheatMask CheatAction::GetCheatMask(std::string cheat)
{
    for (int i = 0; i < log2((uint32)BotCheatMask::maxMask); i++)
    {
        if (sPlayerbotAIConfig.BotCheatMaskName[i] == cheat)
            return BotCheatMask(1 << i);
    }

    return BotCheatMask::none;
}

std::string CheatAction::GetCheatName(BotCheatMask cheatMask)
{
    return sPlayerbotAIConfig.BotCheatMaskName[log2(((uint32)cheatMask))];
}

void CheatAction::ListCheats(Player* requester)
{
    std::ostringstream out;
    for (int i = 0; i < log2((uint32)BotCheatMask::maxMask); i++)
    {
        BotCheatMask cheatMask = BotCheatMask(1 << i);
       if ((uint32)cheatMask & (uint32)sPlayerbotAIConfig.botCheatMask)
           out << "[conf:" << GetCheatName(BotCheatMask(cheatMask)) << "]";
       else if (ai->HasCheat(cheatMask))
           out << "[" << GetCheatName(BotCheatMask(cheatMask)) << "]";
    }

    std::streampos pos = out.tellp();  // store current location
    out.seekp(0, std::ios_base::end);  // go to end
    if (out.tellp() == std::streampos(0))
        out << "No cheats enabled.";
    out.seekp(pos);                    // restore location

    ai->TellPlayerNoFacing(requester, out);
}

void CheatAction::AddCheat(BotCheatMask cheatMask)
{
    switch(cheatMask)
    {
    case BotCheatMask::attackspeed:
        int32 amount = 9999;

        uint32 SPELL_MOD_MELEE_HASTE = 15181;
        uint32 SPELL_MOD_RANGE_HASTE = 15182;
        uint32 SPELL_MOD_SPELL_HASTE = 15183;

        bot->RemoveAurasDueToSpell(SPELL_MOD_MELEE_HASTE);
        bot->CastCustomSpell(bot, SPELL_MOD_MELEE_HASTE, &amount, &amount, nullptr, TRIGGERED_OLD_TRIGGERED);
        bot->RemoveAurasDueToSpell(SPELL_MOD_RANGE_HASTE);
        bot->CastCustomSpell(bot, SPELL_MOD_RANGE_HASTE, &amount, &amount, nullptr, TRIGGERED_OLD_TRIGGERED);
        bot->RemoveAurasDueToSpell(SPELL_MOD_SPELL_HASTE);
        bot->CastCustomSpell(bot, SPELL_MOD_SPELL_HASTE, &amount, &amount, nullptr, TRIGGERED_OLD_TRIGGERED);

        break;
    }
}

void CheatAction::RemCheat(BotCheatMask cheatMask)
{
    switch (cheatMask)
    {
    case BotCheatMask::movespeed:
        bot->UpdateSpeed(MOVE_WALK, true, 1);
        bot->UpdateSpeed(MOVE_RUN, true, 1);
        bot->UpdateSpeed(MOVE_SWIM, true, 1);
        break;
    case BotCheatMask::attackspeed:
        uint32 SPELL_MOD_MELEE_HASTE = 15181;
        uint32 SPELL_MOD_RANGE_HASTE = 15182;
        uint32 SPELL_MOD_SPELL_HASTE = 15183;

        bot->RemoveAurasDueToSpell(SPELL_MOD_MELEE_HASTE);
        bot->RemoveAurasDueToSpell(SPELL_MOD_RANGE_HASTE);
        bot->RemoveAurasDueToSpell(SPELL_MOD_SPELL_HASTE);
        break;
    }
}