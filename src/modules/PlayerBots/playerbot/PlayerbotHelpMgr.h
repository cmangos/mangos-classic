#ifndef _PlayerbotHelpMgr_H
#define _PlayerbotHelpMgr_H

#include "Common.h"
#include "PlayerbotAIBase.h"

#define BOT_HELP(name) sPlayerbotHelpMgr.GetBotText(name)

struct BotHelpEntry
{
    BotHelpEntry() { m_new = true; }
    BotHelpEntry(std::string templateText, std::string text, std::map<int32, std::string> text_locales) : m_templateText(templateText), m_text(text), m_text_locales(text_locales) {}
    std::string m_templateText;
    bool m_new = false;
    bool m_templateChanged = false;
    std::string m_text;
    std::map<int32, std::string> m_text_locales;
};

class PlayerbotHelpMgr
{
    public:
        PlayerbotHelpMgr();
        virtual ~PlayerbotHelpMgr();
        static PlayerbotHelpMgr& instance()
        {
            static PlayerbotHelpMgr instance;
            return instance;
        }

	public:
        static void replace(std::string& text, const std::string what, const std::string with);
        static std::string makeList(std::vector<std::string>const parts, std::string partFormat = "<part>", uint32 maxLength = 254);
#ifdef GenerateBotHelp
        PlayerbotAI* ai;
        AiObjectContext* genericContext;
        typedef std::unordered_map<Action*, float> actionMap;
        typedef std::unordered_map<Trigger*, actionMap> triggerMap;
        typedef std::unordered_map<BotState, triggerMap> stateMap;
        typedef std::unordered_map<Strategy*, stateMap> strategyMap;
        std::unordered_map<std::string, strategyMap> classMap;
        std::unordered_map<std::string, AiObjectContext*> classContext;

        std::unordered_map<BotState, std::string> states = { {BotState::BOT_STATE_COMBAT, "combat"},  {BotState::BOT_STATE_NON_COMBAT, "non combat"},  {BotState::BOT_STATE_DEAD, "dead state"},  {BotState::BOT_STATE_REACTION, "reaction"},  {BotState::BOT_STATE_ALL, "all"} };

        std::unordered_map<std::string, std::string> supportedActionName;

        typedef std::unordered_map<std::string, bool> nameCoverageMap;
        std::unordered_map<std::string, nameCoverageMap> coverageMap;

        static std::string initcap(std::string st) { std::string s = st; s[0] = toupper(s[0]); return s; }
        static std::string formatFloat(float num);

        bool IsGenericSupported(PlayerbotAIAware* object);
        std::string GetObjectName(PlayerbotAIAware* object, std::string className);
        std::string GetObjectLink(PlayerbotAIAware* object, std::string className);

        void LoadStrategies(std::string className, AiObjectContext* context);
        void LoadAllStrategies();

        std::string GetStrategyBehaviour(std::string className, Strategy* strategy);
        void GenerateStrategyHelp();

        std::string GetTriggerBehaviour(std::string className, Trigger* trigger);
        void GenerateTriggerHelp();

        std::string GetActionBehaviour(std::string className, Action* Action);
        void GenerateActionHelp();

        void GenerateValueHelp();

        void GenerateChatFilterHelp();

        void PrintCoverage();

        void SaveTemplates();
        
        void GenerateHelp();
#endif
        static void FormatHelpTopic(std::string& text);
        void FormatHelpTopics();
        void LoadBotHelpTexts();

        std::string GetBotText(std::string name);
        bool GetBotText(std::string name, std::string& text);
        std::vector<std::string> FindBotText(std::string name);
    private:
    std::unordered_map<std::string, BotHelpEntry> botHelpText;
};

#define sPlayerbotHelpMgr PlayerbotHelpMgr::instance()

#endif
