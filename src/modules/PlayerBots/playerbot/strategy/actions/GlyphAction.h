#pragma once
#include "playerbot/strategy/Action.h"

namespace ai
{
    class GlyphAction : public Action, public Qualified
    {
    public:
       GlyphAction(PlayerbotAI* ai, std::string name = "set glyph") : Action(ai, name), Qualified() {}
        virtual bool Execute(Event& event) override;
        typedef void (GlyphAction::* ProcessGlyphAction)(uint32, uint8, std::ostringstream& msg);
        void Iterate(Player* requester, ProcessGlyphAction action, const std::vector<uint32> glyphs, const std::vector<uint8> glyphSlots);  
        void List(uint32 itemId, uint8 slotId, std::ostringstream& msg);
        void Set(uint32 itemId, uint8 wantedSlotId, std::ostringstream& msg);
        void Remove(uint32 itemId, uint8 removeSlotId, std::ostringstream& msg);

        virtual bool isUsefulWhenStunned() override { return true; }
    private:
        uint8 GetEquipedGlyphSlot(uint32 itemId);
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "set glyph"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This action will list or apply new glyphs to the bot..\n"
                "glyph [itemlink] : equip glyph in a open slot.\n"
                "glyph [itemlink] <slotnumber> : replace current glyph in slot with new glyph\n"
                "glyph remove: remove all current glyphs\n"
                "glyph remove <slotnumber>: remove glyph from slot\n"
                "glyph all/wanted/equiped: list glyphs";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    };    

    class AutoSetGlyphAction : public GlyphAction
    {
    public:
        AutoSetGlyphAction(PlayerbotAI* ai, std::string name = "auto set glyph") : GlyphAction(ai, name) {}
        virtual bool Execute(Event& event) override;

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "auto set glyph"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This action will automatically apply new glyphs to the bot based on the config.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    };
}