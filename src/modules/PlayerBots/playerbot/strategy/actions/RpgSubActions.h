#pragma once

#include "playerbot/strategy/Action.h"
#include "MovementActions.h"
#include "ChooseRpgTargetAction.h"
#include "UseItemAction.h"
#include "playerbot/strategy/values/LastMovementValue.h"
#include "SayAction.h"

namespace ai
{
    class RpgHelper : public AiObject
    {
    public:
        RpgHelper(PlayerbotAI* ai) : AiObject(ai) {}

        void BeforeExecute();
        void AfterExecute(bool doDelay = true,  bool waitForGroup = false, std::string nextAction = "rpg");
        void OnCancel() { resetFacing(guidP()); if (bot->GetTradeData()) bot->TradeCancel(true); };

        virtual GuidPosition guidP() { return AI_VALUE(GuidPosition, "rpg target"); }
        virtual ObjectGuid guid() { return (ObjectGuid)guidP(); }        
        virtual bool InRange() { return AI_VALUE2(float, "distance", "rpg target") <= INTERACTION_DISTANCE * 1.5; }
        void setDelay(bool waitForGroup);
    private:
        void setFacingTo(GuidPosition guidPosition);
        void setFacing(GuidPosition guidPosition);
        void resetFacing(GuidPosition guidPosition);
    };

    class RpgEnabled
    {
    public:
        RpgEnabled(PlayerbotAI* ai) { rpg = std::make_unique<RpgHelper>(ai); }
    protected:
        std::unique_ptr<RpgHelper> rpg;
    };

    class RpgSubAction : public Action, public RpgEnabled
    {
    public:
        RpgSubAction(PlayerbotAI* ai, std::string name = "rpg sub") : Action(ai, name), RpgEnabled(ai) {}

        //Long range is possible?
        virtual bool isPossible() { return rpg->guidP() && rpg->guidP().GetWorldObject(bot->GetInstanceId());}
        //Short range can we do the action now?
        virtual bool isUseful() { return rpg->InRange(); }

        virtual bool Execute(Event& event) { rpg->BeforeExecute();  bool doAction = ai->DoSpecificAction(ActionName(), ActionEvent(event), true); rpg->AfterExecute(doAction, true); DoDelay(); return doAction; }
    protected:
        void DoDelay(){ SetDuration(ai->GetAIInternalUpdateDelay()); }
        virtual std::string ActionName() { return "none"; }
        virtual Event ActionEvent(Event event) { return event; }
    };        

    class RpgStayAction : public RpgSubAction
    {
    public:
        RpgStayAction(PlayerbotAI* ai, std::string name = "rpg stay") : RpgSubAction(ai, name) {}

        //virtual bool isUseful() { return rpg->InRange() && !ai->HasRealPlayerMaster(); }

        virtual bool Execute(Event& event) { rpg->BeforeExecute(); if (bot->GetPlayerMenu()) bot->GetPlayerMenu()->CloseGossip(); rpg->AfterExecute(); DoDelay(); return true; };
    };   

    class RpgWorkAction : public RpgSubAction
    {
    public:
        RpgWorkAction(PlayerbotAI* ai, std::string name = "rpg work") : RpgSubAction(ai, name ) {}

        //virtual bool isUseful() { return rpg->InRange() && !ai->HasRealPlayerMaster(); }

        virtual bool Execute(Event& event) { rpg->BeforeExecute(); bot->HandleEmoteCommand(EMOTE_STATE_USESTANDING); rpg->AfterExecute(); DoDelay(); return true; };
    };

    class RpgEmoteAction : public RpgSubAction
    {
    public:
        RpgEmoteAction(PlayerbotAI* ai, std::string name = "rpg emote") : RpgSubAction(ai, name) {}

        //virtual bool isUseful() { return rpg->InRange() && !ai->HasRealPlayerMaster(); }

        virtual bool Execute(Event& event);
    };

    class RpgCancelAction : public RpgSubAction
    {
    public:
        RpgCancelAction(PlayerbotAI* ai, std::string name = "rpg cancel") : RpgSubAction(ai, name) {}

        virtual bool isUseful() {return rpg->InRange();}

        virtual bool Execute(Event& event);
    };

    class RpgTaxiAction : public RpgSubAction
    {
    public:
        RpgTaxiAction(PlayerbotAI* ai, std::string name = "rpg taxi") : RpgSubAction(ai, name) {}

        virtual bool isUseful() { return rpg->InRange() && !ai->HasRealPlayerMaster() && bot->GetGroup(); }

        virtual bool Execute(Event& event);
    };

    class RpgDiscoverAction : public RpgTaxiAction
    {
    public:
        RpgDiscoverAction(PlayerbotAI* ai, std::string name = "rpg discover") : RpgTaxiAction(ai,name) {}

        virtual bool isUseful() { return rpg->InRange(); }

        virtual bool Execute(Event& event);
    };

    class RpgStartQuestAction : public RpgSubAction
    {
    public:
        RpgStartQuestAction(PlayerbotAI* ai, std::string name = "rpg start quest") : RpgSubAction(ai, name) {}
        virtual bool Execute(Event& event) { rpg->BeforeExecute();  bool doAction = ai->DoSpecificAction(ActionName(), ActionEvent(event), true); rpg->AfterExecute(doAction, true, ""); DoDelay(); return doAction; }
    private:
        virtual std::string ActionName() { return "accept all quests"; }
        virtual Event ActionEvent(Event event) {WorldPacket p(CMSG_QUESTGIVER_ACCEPT_QUEST); p << rpg->guid(); p.rpos(0);  return  Event("rpg action", p); }
    };

    class RpgEndQuestAction : public RpgStartQuestAction
    {
    public:
        RpgEndQuestAction(PlayerbotAI* ai, std::string name = "rpg end quest") : RpgStartQuestAction(ai, name) {}

    private:
        virtual std::string ActionName() { return "talk to quest giver"; }
        virtual Event ActionEvent(Event event) { WorldPacket p(CMSG_QUESTGIVER_COMPLETE_QUEST); p << rpg->guid(); p.rpos(0);  return  Event("rpg action", p); }
    };

    class RpgBuyAction : public RpgSubAction
    {
    public:
        RpgBuyAction(PlayerbotAI* ai, std::string name = "rpg buy") : RpgSubAction(ai, name) {}

    private:
        virtual std::string ActionName() { return "buy"; }
        virtual Event ActionEvent(Event event) { return Event("rpg action", "vendor"); }
    };
   
    class RpgSellAction : public RpgSubAction
    {
    public:
        RpgSellAction(PlayerbotAI* ai, std::string name = "rpg sell") : RpgSubAction(ai, name) {}

    private:
        virtual std::string ActionName() { return "sell"; }
        virtual Event ActionEvent(Event event) { return Event("rpg action", "vendor"); }
    };

    class RpgAHSellAction : public RpgSubAction
    {
    public:
        RpgAHSellAction(PlayerbotAI* ai, std::string name = "rpg ah sell") : RpgSubAction(ai, name) {}

    private:
        virtual std::string ActionName() { return "ah"; }
        virtual Event ActionEvent(Event event) { return Event("rpg action", "vendor"); }
    };

    class RpgAHBuyAction : public RpgSubAction
    {
    public:
        RpgAHBuyAction(PlayerbotAI* ai, std::string name = "rpg ah buy") : RpgSubAction(ai, name) {}

    private:
        virtual std::string ActionName() { return "ah bid"; }
        virtual Event ActionEvent(Event event) { return Event("rpg action", "vendor"); }
    };

    class RpgGetMailAction : public RpgSubAction
    {
    public:
        RpgGetMailAction(PlayerbotAI* ai, std::string name = "rpg get mail") : RpgSubAction(ai, name) {}

    private:
        virtual std::string ActionName() { return "mail"; }
        virtual Event ActionEvent(Event event) { return Event("rpg action", "take"); }
       
        virtual bool Execute(Event& event) { bool doAction = RpgSubAction::Execute(event); if (doAction) ai->DoSpecificAction("equip upgrades", event, true); return doAction; }
    };

    class RpgRepairAction : public RpgSubAction
    {
    public:
        RpgRepairAction(PlayerbotAI* ai, std::string name = "rpg repair") : RpgSubAction(ai, name) {}

    private:
        virtual std::string ActionName() { return "repair"; }
    };

    class RpgTrainAction : public RpgSubAction
    {
    public:
        RpgTrainAction(PlayerbotAI* ai, std::string name = "rpg train") : RpgSubAction(ai, name) {}

    private:
        virtual std::string ActionName() { return "trainer"; }
        virtual Event ActionEvent(Event event) { return Event("rpg action",rpg->guidP()); }
    };

    class RpgHealAction : public RpgSubAction
    {
    public:
        RpgHealAction(PlayerbotAI* ai, std::string name = "rpg heal") : RpgSubAction(ai, name) {}

        virtual bool Execute(Event& event);
    };

    class RpgHomeBindAction : public RpgSubAction
    {
    public:
        RpgHomeBindAction(PlayerbotAI* ai, std::string name = "rpg home bind") : RpgSubAction(ai, name) {}

    private:
        virtual std::string ActionName() { return "home"; }
    };

    class RpgQueueBgAction : public RpgSubAction
    {
    public:
        RpgQueueBgAction(PlayerbotAI* ai, std::string name = "rpg queue bg") : RpgSubAction(ai, name) {}

    private:
        virtual std::string ActionName() { SET_AI_VALUE(uint32, "bg type", (uint32)AI_VALUE(BattleGroundTypeId, "rpg bg type")); return "free bg join"; }
    };

    class RpgBuyPetitionAction : public RpgSubAction
    {
    public:
        RpgBuyPetitionAction(PlayerbotAI* ai, std::string name = "rpg buy petition") : RpgSubAction(ai, name) {}

    private:
        virtual std::string ActionName() { return "buy petition"; }
    };

    class RpgUseAction : public RpgSubAction
    {
    public:
        RpgUseAction(PlayerbotAI* ai, std::string name = "rpg use") : RpgSubAction(ai, name) {}

        virtual bool Execute(Event& event) { rpg->BeforeExecute();  return ai->DoSpecificAction(ActionName(), ActionEvent(event), true); rpg->AfterExecute(true); DoDelay();}
    private:
        virtual std::string ActionName() { if (rpg->guidP().IsGameObject() && rpg->guidP().GetGameObject(bot->GetInstanceId())->GetGoType() == GAMEOBJECT_TYPE_CHEST) return "add all loot";  return "use"; }
        virtual Event ActionEvent(Event event) { return Event("rpg action", chat->formatWorldobject(rpg->guidP().GetWorldObject(bot->GetInstanceId()))); }
    };

    class RpgAIChatAction : public RpgSubAction
    {
    public:
        RpgAIChatAction(PlayerbotAI* ai, std::string name = "rpg ai chat") : RpgSubAction(ai, name) {}

        void ManualChat(GuidPosition target, const std::string& line);
    private:
        virtual bool isUseful() override;

        bool SpeakLine();
        bool WaitForLines();
        bool RequestNewLines();
        virtual bool Execute(Event& event) override;

        std::queue<delayedPacket> packets;
        futurePackets futPackets;
    };

    class RpgSpellAction : public RpgSubAction
    {
    public:
        RpgSpellAction(PlayerbotAI* ai, std::string name = "rpg spell") : RpgSubAction(ai, name) {}

    private:
        virtual bool isUseful() { return true; }
        virtual std::string ActionName() { return "cast random spell"; }
        virtual Event ActionEvent(Event event) { return Event("rpg action", chat->formatWorldobject(rpg->guidP().GetWorldObject(bot->GetInstanceId()))); }
    };

    class RpgCraftAction : public RpgSubAction
    {
    public:
        RpgCraftAction(PlayerbotAI* ai, std::string name = "rpg craft") : RpgSubAction(ai, name) {}

    private:
        virtual std::string ActionName() { return "craft random item"; }
        virtual Event ActionEvent(Event event) { return Event("rpg action", chat->formatWorldobject(rpg->guidP().GetWorldObject(bot->GetInstanceId()))); }
    };

    class RpgTradeUsefulAction : public RpgSubAction
    {
    public:
        RpgTradeUsefulAction(PlayerbotAI* ai, std::string name = "rpg trade useful") : RpgSubAction(ai, name) {}

        bool IsTradingItem(uint32 entry);

        virtual bool Execute(Event& event);
    };

    class RpgEnchantAction : public RpgTradeUsefulAction
    {
    public:
        RpgEnchantAction(PlayerbotAI* ai, std::string name = "rpg enchant") : RpgTradeUsefulAction(ai, name) {}

        virtual bool Execute(Event& event);
    };

    class RpgDuelAction : public RpgSubAction
    {
    public:
        RpgDuelAction(PlayerbotAI* ai, std::string name = "rpg duel") : RpgSubAction(ai, name) {}

        virtual bool isUseful();
        virtual bool Execute(Event& event);
    };

    class RpgItemAction : public UseAction, public RpgEnabled
    {
    public:
        RpgItemAction(PlayerbotAI* ai, std::string name = "rpg item") : UseAction(ai, name), RpgEnabled(ai) {}

        //Long range is possible?
        virtual bool isPossible() { return rpg->guidP() && rpg->guidP().GetWorldObject(bot->GetInstanceId()); }
        //Short range can we do the action now?
        virtual bool isUseful() { return !urand(0,3) || rpg->InRange(); }

        virtual bool Execute(Event& event);
    };

    class RpgSpellClickAction : public RpgSubAction
    {
    public:
        RpgSpellClickAction(PlayerbotAI* ai, std::string name = "rpg spell click") : RpgSubAction(ai, name) {}

        virtual bool Execute(Event& event);
    };

    class RpgGossipTalkAction : public RpgSubAction
    {
    public:
        RpgGossipTalkAction(PlayerbotAI* ai, std::string name = "rpg gossip talk") : RpgSubAction(ai, name) {}

    private:
        virtual std::string ActionName() { return "gossip hello"; }
        virtual Event ActionEvent(Event event) { WorldPacket p(CMSG_GOSSIP_SELECT_OPTION); p << rpg->guid(); p.rpos(0); return Event("rpg action", p); }
    };
}
