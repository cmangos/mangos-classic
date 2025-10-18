
#include "playerbot/playerbot.h"
#include "ReadyCheckAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

std::string formatPercent(std::string name, uint8 value, float percent)
{
    std::ostringstream out;

    std::string color;
    if (percent > 75)
        color = "|cff00ff00";
    else if (percent > 50)
        color = "|cffffff00";
    else
        color = "|cffff0000";

    out << "|cffffffff[" << name << "]" << color << "x" << (int)value;
    return out.str();
}

class ReadyChecker
{
public:
    virtual bool Check(Player* requester, PlayerbotAI *ai, AiObjectContext* context) = 0;
    virtual std::string GetName() = 0;
    virtual bool PrintAlways() { return true; }

    static std::list<ReadyChecker*> checkers;
};

std::list<ReadyChecker*> ReadyChecker::checkers;

class HealthChecker : public ReadyChecker
{
public:
    bool Check(Player* requester, PlayerbotAI *ai, AiObjectContext* context) override
    {
        return AI_VALUE2(uint8, "health", "self target") > sPlayerbotAIConfig.almostFullHealth;
    }

    virtual std::string GetName() { return "HP"; }
};

class ManaChecker : public ReadyChecker
{
public:
    bool Check(Player* requester, PlayerbotAI *ai, AiObjectContext* context) override
    {
        return !AI_VALUE2(bool, "has mana", "self target") || AI_VALUE2(uint8, "mana", "self target") > sPlayerbotAIConfig.mediumHealth;
    }
    virtual std::string GetName() { return "MP"; }
};

class DistanceChecker : public ReadyChecker
{
public:
    bool Check(Player* requester, PlayerbotAI *ai, AiObjectContext* context) override
    {
        Player* bot = ai->GetBot();
        if (requester)
        {
            bool distance = sServerFacade.GetDistance2d(bot, requester) <= sPlayerbotAIConfig.sightDistance;
            if (!distance)
            {
                return false;
            }
        }

        return true;
    }

    virtual bool PrintAlways() { return false; }
    virtual std::string GetName() { return "Far away"; }
};

class HunterChecker : public ReadyChecker
{
public:
    bool Check(Player* requester, PlayerbotAI *ai, AiObjectContext* context) override
    {
        Player* bot = ai->GetBot();
        if (bot->getClass() == CLASS_HUNTER)
        {
            if (!bot->GetUInt32Value(PLAYER_AMMO_ID))
            {
                ai->TellError(requester, "Out of ammo!");
                return false;
            }

            if (!bot->GetPet())
            {
                ai->TellError(requester, "No pet!");
                return false;
            }

            if (bot->GetPet()->GetHappinessState() == UNHAPPY)
            {
                ai->TellError(requester, "Pet is unhappy!");
                return false;
            }
        }

        return true;
    }

    virtual bool PrintAlways() { return false; }
    virtual std::string GetName() { return "Far away"; }
};


class ItemCountChecker : public ReadyChecker
{
public:
    ItemCountChecker(std::string item, std::string name) { this->item = item; this->name = name; }

    bool Check(Player* requester, PlayerbotAI *ai, AiObjectContext* context) override
    {
        return AI_VALUE2(uint32, "item count", item) > 0;
    }

    virtual std::string GetName() { return name; }

private:
    std::string item, name;
};

class ManaPotionChecker : public ItemCountChecker
{
public:
    ManaPotionChecker(std::string item, std::string name) : ItemCountChecker(item, name) {}

    bool Check(Player* requester, PlayerbotAI *ai, AiObjectContext* context) override
    {
        return !AI_VALUE2(bool, "has mana", "self target") || ItemCountChecker::Check(requester, ai, context);
    }
};

bool ReadyCheckAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    WorldPacket p = event.getPacket();
    ObjectGuid player;
    p.rpos(0);
    if (!p.empty())
    {
        p >> player;
        if (player == bot->GetObjectGuid())
            return false;
    }

	return ReadyCheck(requester);
}

bool ReadyCheckAction::ReadyCheck(Player* requester)
{
    if (ReadyChecker::checkers.empty())
    {
        ReadyChecker::checkers.push_back(new HealthChecker());
        ReadyChecker::checkers.push_back(new ManaChecker());
        ReadyChecker::checkers.push_back(new DistanceChecker());
        ReadyChecker::checkers.push_back(new HunterChecker());

        ReadyChecker::checkers.push_back(new ItemCountChecker("food", "Food"));
        ReadyChecker::checkers.push_back(new ManaPotionChecker("drink", "Water"));
        ReadyChecker::checkers.push_back(new ItemCountChecker("healing potion", "Hpot"));
        ReadyChecker::checkers.push_back(new ManaPotionChecker("mana potion", "Mpot"));
    }

    bool result = true;
    for (std::list<ReadyChecker*>::iterator i = ReadyChecker::checkers.begin(); i != ReadyChecker::checkers.end(); ++i)
    {
        ReadyChecker* checker = *i;
        bool ok = checker->Check(requester, ai, context);
        result = result && ok;
    }

    std::ostringstream out;

    uint32 hp = AI_VALUE2(uint32, "item count", "healing potion");
    out << formatPercent("Hp", hp, 100.0 * hp / 5);

    out << ", ";
    uint32 food = AI_VALUE2(uint32, "item count", "food");
    out << formatPercent("Food", food, 100.0 * food / 20);

    if (AI_VALUE2(bool, "has mana", "self target"))
    {
        out << ", ";
        uint32 mp = AI_VALUE2(uint32, "item count", "mana potion");
        out << formatPercent("Mp", mp, 100.0 * mp / 5);

        out << ", ";
        uint32 water = AI_VALUE2(uint32, "item count", "water");
        out << formatPercent("Water", water, 100.0 * water / 20);
    }

    ai->TellPlayer(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

    WorldPacket packet(MSG_RAID_READY_CHECK);
    packet << uint8(1);
    bot->GetSession()->HandleRaidReadyCheckOpcode(packet);

    ai->ChangeStrategy("-ready check", BotState::BOT_STATE_NON_COMBAT);

    return true;
}

bool FinishReadyCheckAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    return ReadyCheck(requester);
}
