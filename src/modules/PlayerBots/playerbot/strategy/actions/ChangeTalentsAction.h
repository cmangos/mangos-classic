#pragma once

#include "playerbot/playerbot.h"
#include "playerbot/Talentspec.h"
#include "GenericActions.h"

namespace ai
{
    class ChangeTalentsAction : public ChatCommandAction
    {
    public:
        ChangeTalentsAction(PlayerbotAI* ai, std::string name = "talents") : ChatCommandAction(ai, name) {}

        virtual bool isUsefulWhenStunned() override { return true; }

    public:
        virtual bool Execute(Event& event) override;
        virtual bool AutoSelectTalents(std::ostringstream* out);
    private:
        std::vector<TalentPath*> getPremadePaths(std::string findName);
        std::vector<TalentPath*> getPremadePaths(TalentSpec* oldSpec);
        TalentPath* getPremadePath(int id);
        void listPremadePaths(std::vector<TalentPath*> paths, std::ostringstream* out);
        TalentPath* PickPremadePath(std::vector<TalentPath*> paths, bool useProbability);
        TalentSpec* GetBestPremadeSpec(int spec);
    };

    class AutoSetTalentsAction : public ChangeTalentsAction 
    {
    public:
        AutoSetTalentsAction(PlayerbotAI* ai) : ChangeTalentsAction(ai, "auto talents") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }
    };
}