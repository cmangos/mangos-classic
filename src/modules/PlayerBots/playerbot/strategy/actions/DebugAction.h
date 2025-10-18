#pragma once
#include "GenericActions.h"

namespace ai
{
    class DebugAction : public ChatCommandAction
    {
    public:
        DebugAction(PlayerbotAI* ai) : ChatCommandAction(ai, "debug") {}
        virtual bool Execute(Event& event) override;

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "debug"; }
        virtual std::string GetHelpDescription()
        {
            return "This command enables debug functionalities for testing bot behaviors.\n"
                   "Usage: cdebug [subcommand]\n"
                   "Please refer to the source code for specific uses.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 

        void FakeSpell(uint32 spellId, Unit* truecaster, Unit* caster, ObjectGuid target = ObjectGuid(), std::list<ObjectGuid> otherTargets = {}, std::list<ObjectGuid> missTargets = {}, WorldPosition source = WorldPosition(), WorldPosition dest = WorldPosition(), bool forceDest = false);
        void addAura(uint32 spellId, Unit* target);
    private:
        // Handler methods for each debug command
        bool HandleAvoidScan(Event& event, Player* requester, const std::string& text);
        bool HandleAvoidAdd(Event& event, Player* requester, const std::string& text);
        bool HandleMount(Event& event, Player* requester, const std::string& text);
        bool HandleUnmount(Event& event, Player* requester, const std::string& text);
        bool HandleArea(Event& event, Player* requester, const std::string& text);
        bool HandleLLM(Event& event, Player* requester, const std::string& text);
        bool HandleChatReplyDo(Event& event, Player* requester, const std::string& text);
        bool HandleMonsterTalk(Event& event, Player* requester, const std::string& text);
        bool HandleGY(Event& event, Player* requester, const std::string& text);
        bool HandleGrid(Event& event, Player* requester, const std::string& text);
        bool HandleTest(Event& event, Player* requester, const std::string& text);
        bool HandleValues(Event& event, Player* requester, const std::string& text);
        bool HandleGPS(Event& event, Player* requester, const std::string& text);
        bool HandleSetValueUInt32(Event& event, Player* requester, const std::string& text);
        bool HandleDo(Event& event, Player* requester, const std::string& text);
        bool HandleTrade(Event& event, Player* requester, const std::string& text);
        bool HandleMail(Event& event, Player* requester, const std::string& text);
        bool HandlePOI(Event& event, Player* requester, const std::string& text);
        bool HandleMotion(Event& event, Player* requester, const std::string& text);
        bool HandleTransport(Event& event, Player* requester, const std::string& text);
        bool HandleOnTrans(Event& event, Player* requester, const std::string& text);
        bool HandleOffTrans(Event& event, Player* requester, const std::string& text);
        bool HandlePathable(Event& event, Player* requester, const std::string& text);
        bool HandleRandomSpot(Event& event, Player* requester, const std::string& text);
        bool HandlePrintMap(Event& event, Player* requester, const std::string& text);
        bool HandleCorpse(Event& event, Player* requester, const std::string& text);
        bool HandleLogoutTime(Event& event, Player* requester, const std::string& text);
        bool HandleLevel(Event& event, Player* requester, const std::string& text);
        bool HandleNPC(Event& event, Player* requester, const std::string& text);
        bool HandleGO(Event& event, Player* requester, const std::string& text);
        bool HandleFind(Event& event, Player* requester, const std::string& text);
        bool HandleItem(Event& event, Player* requester, const std::string& text);
        bool HandleRPG(Event& event, Player* requester, const std::string& text);
        bool HandleRPGTargets(Event& event, Player* requester, const std::string& text);
        bool HandleTravel(Event& event, Player* requester, const std::string& text);
        bool HandlePrintTravel(Event& event, Player* requester, const std::string& text);
        bool HandleLoot(Event& event, Player* requester, const std::string& text);
        bool HandleDrops(Event& event, Player* requester, const std::string& text);
        bool HandleTaxi(Event& event, Player* requester, const std::string& text);
        bool HandlePrice(Event& event, Player* requester, const std::string& text);
        bool HandleNC(Event& event, Player* requester, const std::string& text);
        bool HandleAddNode(Event& event, Player* requester, const std::string& text);
        bool HandleRemNode(Event& event, Player* requester, const std::string& text);
        bool HandleResetNode(Event& event, Player* requester, const std::string& text);
        bool HandleResetPath(Event& event, Player* requester, const std::string& text);
        bool HandleGenNode(Event& event, Player* requester, const std::string& text);
        bool HandleGenPath(Event& event, Player* requester, const std::string& text);
        bool HandleCropPath(Event& event, Player* requester, const std::string& text);
        bool HandleSaveNode(Event& event, Player* requester, const std::string& text);
        bool HandleLoadNode(Event& event, Player* requester, const std::string& text);
        bool HandleShowNode(Event& event, Player* requester, const std::string& text);
        bool HandleDSpell(Event& event, Player* requester, const std::string& text);
        bool HandleVSpell(Event& event, Player* requester, const std::string& text);
        bool HandleASpell(Event& event, Player* requester, const std::string& text);
        bool HandleCSpell(Event& event, Player* requester, const std::string& text);
        bool HandleFSpell(Event& event, Player* requester, const std::string& text);
        bool HandleSpell(Event& event, Player* requester, const std::string& text);
        bool HandleTSpellMap(Event& event, Player* requester, const std::string& text);
        bool HandleUSpellMap(Event& event, Player* requester, const std::string& text);
        bool HandleDSpellMap(Event& event, Player* requester, const std::string& text);
        bool HandleVSpellMap(Event& event, Player* requester, const std::string& text);
        bool HandleISpellMap(Event& event, Player* requester, const std::string& text);
        bool HandleCSpellMap(Event& event, Player* requester, const std::string& text);
        bool HandleASpellMap(Event& event, Player* requester, const std::string& text);
        bool HandleGSpellMap(Event& event, Player* requester, const std::string& text);
        bool HandleMSpellMap(Event& event, Player* requester, const std::string& text);
        bool HandleSoundMap(Event& event, Player* requester, const std::string& text);
        bool HandleSounds(Event& event, Player* requester, const std::string& text);
        bool HandleDSound(Event& event, Player* requester, const std::string& text);
        bool HandleSound(Event& event, Player* requester, const std::string& text);
    };
}
