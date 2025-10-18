#pragma once
#include "GenericActions.h"

namespace ai
{
    class MailProcessor
    {
    public:
        virtual bool Before(Player* requester, PlayerbotAI* ai) { return true; }
        virtual bool Process(Player* requester, int index, Mail* mail, PlayerbotAI* ai, Event& event) = 0;
        virtual bool After(Player* requester, PlayerbotAI* ai) { return true; }

    public:
        static ObjectGuid FindMailbox(PlayerbotAI* ai);

    protected:
        void RemoveMail(Player* bot, uint32 id, ObjectGuid mailbox);
    };


    class MailAction : public ChatCommandAction
    {
    public:
        MailAction(PlayerbotAI* ai) : ChatCommandAction(ai, "mail") {}
        virtual bool Execute(Event& event) override;

    private:
        bool CheckMailbox();

    private:
        static std::map<std::string, MailProcessor*> processors;
    };

}
