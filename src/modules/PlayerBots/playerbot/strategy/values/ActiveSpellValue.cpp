
#include "ActiveSpellValue.h"
#include "playerbot/playerbot.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

uint32 ActiveSpellValue::Calculate()
{
    Player* bot = ai->GetBot();
    for (int type = CURRENT_MELEE_SPELL; type <= CURRENT_CHANNELED_SPELL; ++type)
    {
        Spell *spell = bot->GetCurrentSpell((CurrentSpellTypes)type);
        if (spell && spell->m_spellInfo) return spell->m_spellInfo->Id;
    }

    return 0;
}

