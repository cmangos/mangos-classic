#pragma once
#include "Event.h"
#include "Value.h"
#include "AiObject.h"
#include "AiObject.h"
#include "playerbot/PlayerbotAIConfig.h"

class Unit;

namespace ai
{
    class NextAction;
    template<class T> class Value;
}

namespace ai
{
    class NextAction
    {
    public:
        NextAction(std::string name, float relevance = 0.0f)
        {
            this->name = name;
            this->relevance = relevance;
        }
        NextAction(const NextAction& o)
        {
            this->name = o.name;
            this->relevance = o.relevance;
        }

    public:
        std::string getName() const { return name; }
        float getRelevance() const { return relevance; }

    public:
        static int size(NextAction** actions);
        static NextAction** clone(NextAction** actions);
        static NextAction** merge(NextAction** what, NextAction** with);
        static NextAction** array(uint32 n,...);
        static void destroy(NextAction** actions);

    private:
        float relevance;
        std::string name;
    };

    //---------------------------------------------------------------------------------------------------------------------

    class ActionBasket;

    enum class ActionThreatType : uint8
    {
        ACTION_THREAT_NONE = 0,
        ACTION_THREAT_SINGLE= 1,
        ACTION_THREAT_AOE = 2
    };

    class Action : public AiNamedObject
	{
	public:
        Action(PlayerbotAI* ai, std::string name = "action", uint32 duration = sPlayerbotAIConfig.reactDelay) : AiNamedObject(ai, name), verbose(false), duration(duration) {}
        virtual ~Action(void) {}

    public:
        virtual bool Execute(Event& event) { return true; }
        virtual bool isPossible() { return true; }
        virtual bool isUseful() { return true; }
        virtual bool isUsefulWhenStunned() { return false; }
        virtual NextAction** getPrerequisites() { return NULL; }
        virtual NextAction** getAlternatives() { return NULL; }
        virtual NextAction** getContinuers() { return NULL; }
        virtual ActionThreatType getThreatType() { return ActionThreatType::ACTION_THREAT_NONE; }
        void Update() {}  //Nonfunctional see AiObjectContext::Update() to enable.
        void Reset() {}
        virtual Unit* GetTarget();
        virtual Value<Unit*>* GetTargetValue();
        virtual std::string GetTargetName() { return "self target"; }
        void MakeVerbose(bool enabled) { verbose = enabled; }

        void setRelevance(float relevance1) { relevance = relevance1; };
        virtual float getRelevance() { return relevance; }

        bool IsReaction() const { return reaction; }
        void SetReaction(bool inReaction) { reaction = inReaction; }

        // Used when this action is executed as a reaction
        virtual bool ShouldReactionInterruptCast() const { return false; }
        virtual bool ShouldReactionInterruptMovement() const { return false; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "dummy"; } //Must equal internal name
        virtual std::string GetHelpDescription() { return "This is an action."; }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif

        uint32 GetDuration() const { return duration; }

    protected:
        void SetDuration(uint32 inDuration) { duration = inDuration; }

    protected:
        bool verbose;
        float relevance = 0;
        bool reaction = false;

    private:
        uint32 duration;
	};

    //---------------------------------------------------------------------------------------------------------------------

    class ActionNode
    {
    public:
        ActionNode(std::string name, NextAction** prerequisites = NULL, NextAction** alternatives = NULL, NextAction** continuers = NULL)
        {
            this->action = NULL;
            this->name = name;
            this->prerequisites = prerequisites;
            this->alternatives = alternatives;
            this->continuers = continuers;
        }

        virtual ~ActionNode()
        {
            NextAction::destroy(prerequisites);
            NextAction::destroy(alternatives);
            NextAction::destroy(continuers);
        }

    public:
        Action* getAction() { return action; }
        void setAction(Action* action) { this->action = action; }
        std::string getName() { return name; }

    public:
        NextAction** getContinuers() { return NextAction::merge(NextAction::clone(continuers), action->getContinuers()); }
        NextAction** getAlternatives() { return NextAction::merge(NextAction::clone(alternatives), action->getAlternatives()); }
        NextAction** getPrerequisites() { return NextAction::merge(NextAction::clone(prerequisites), action->getPrerequisites()); }

    private:
        std::string name;
        Action* action;
        NextAction** continuers;
        NextAction** alternatives;
        NextAction** prerequisites;
    };

    //---------------------------------------------------------------------------------------------------------------------

	class ActionBasket
	{
	public:
        ActionBasket(ActionNode* action, float relevance, bool skipPrerequisites, const Event& event) :
          action(action), relevance(relevance), skipPrerequisites(skipPrerequisites), event(event) {
            created = time(0);
        }
        virtual ~ActionBasket(void) {}
	public:
		float getRelevance() {return relevance;}
		ActionNode* getAction() {return action;}
        Event getEvent() { return event; }
        bool isSkipPrerequisites() { return skipPrerequisites; }
        void AmendRelevance(float k) {relevance *= k; }
        void setRelevance(float relevance) { this->relevance = relevance; }
        void setEvent(Event event) { this->event = event; }
        bool isExpired(time_t secs) { return time(0) - created >= secs; }
	private:
		ActionNode* action;
		float relevance;
        bool skipPrerequisites;
        Event event;
        time_t created;
	};
}