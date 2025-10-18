
#include "playerbot/playerbot.h"
#include "Action.h"
#include "Queue.h"

#include "playerbot/PlayerbotAIConfig.h"
using namespace ai;


void Queue::Push(ActionBasket *action)
{
	if (action)
    {
        for (std::list<ActionBasket*>::iterator iter = actions.begin(); iter != actions.end(); iter++)
        {
            ActionBasket* basket = *iter;
            if (action->getAction()->getName() == basket->getAction()->getName())
            {
                if (basket->getRelevance() < action->getRelevance())
                {
                    basket->setRelevance(action->getRelevance());
                    basket->setEvent(action->getEvent());
                }
				ActionNode *actionNode = action->getAction();
				if (actionNode)
				    delete actionNode;
                delete action;
                return;
            }
        }
		actions.push_back(action);
    }
}

ActionNode* Queue::Pop(ActionBasket* action)
{
    ActionBasket* selection = action;
    if (selection == nullptr)
    {
        float max = -400;
        for (std::list<ActionBasket*>::iterator iter = actions.begin(); iter != actions.end(); iter++)
        {
            ActionBasket* basket = *iter;
            if (basket->getRelevance() > max)
            {
                max = basket->getRelevance();
                selection = basket;
            }
        }
    }

    if (selection != nullptr)
    {
        ActionNode* action = selection->getAction();
        actions.remove(selection);
        delete selection;
        return action;
    }

    return nullptr;
}

ActionBasket* Queue::Peek()
{
    float max = -400;
    ActionBasket* selection = NULL;
    for (std::list<ActionBasket*>::iterator iter = actions.begin(); iter != actions.end(); iter++)
    {
        ActionBasket* basket = *iter;
        if (basket->getRelevance() > max)
        {
            max = basket->getRelevance();
            selection = basket;
        }
    }
    return selection;
}

int Queue::Size()
{
	return actions.size();
}

void Queue::RemoveExpired()
{
    std::list<ActionBasket*> expired;
    for (std::list<ActionBasket*>::iterator iter = actions.begin(); iter != actions.end(); iter++)
    {
        ActionBasket* basket = *iter;
        if (sPlayerbotAIConfig.expireActionTime && basket->isExpired(sPlayerbotAIConfig.expireActionTime / 1000))
            expired.push_back(basket);
    }

    for (std::list<ActionBasket*>::iterator iter = expired.begin(); iter != expired.end(); iter++)
    {
        ActionBasket* basket = *iter;
        actions.remove(basket);
        ActionNode* action = basket->getAction();
        if (action)
        {
            sLog.outDebug("Action %s is expired", action->getName().c_str());
            delete action;
        }
        delete basket;
    }
}
