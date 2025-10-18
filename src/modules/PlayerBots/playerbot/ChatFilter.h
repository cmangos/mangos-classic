#pragma once

namespace ai
{
    class ChatFilter : public PlayerbotAIAware
    {
    public:
        ChatFilter(PlayerbotAI* ai) : PlayerbotAIAware(ai) {}
        virtual std::string Filter(std::string message) { return Filter(message, ""); }
        virtual std::string Filter(std::string message, std::string filter);
		virtual ~ChatFilter() {}

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "dummy"; }
        virtual std::unordered_map<std::string, std::string> GetFilterExamples() {return {};}
        virtual std::string GetHelpDescription() { return ""; }
#endif
    };

    class CompositeChatFilter : public ChatFilter
    {
    public:
        CompositeChatFilter(PlayerbotAI* ai);
        virtual ~CompositeChatFilter();
        std::string Filter(std::string message);
#ifdef GenerateBotHelp
        virtual std::list<ChatFilter*> GetFilters() { return filters;}
#endif
    private:
    std::list<ChatFilter*> filters;
    };
};
