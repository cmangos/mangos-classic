#include "playerbot/playerbot.h"
#include "playerbot/AiFactory.h"
#include "strategy/AiObjectContext.h"
#include "strategy/ReactionEngine.h"

#include "strategy/priest/PriestAiObjectContext.h"
#include "strategy/mage/MageAiObjectContext.h"
#include "strategy/warlock/WarlockAiObjectContext.h"
#include "strategy/warrior/WarriorAiObjectContext.h"
#include "strategy/shaman/ShamanAiObjectContext.h"
#include "strategy/paladin/PaladinAiObjectContext.h"
#include "strategy/druid/DruidAiObjectContext.h"
#include "strategy/hunter/HunterAiObjectContext.h"
#include "strategy/rogue/RogueAiObjectContext.h"
#include "strategy/deathknight/DKAiObjectContext.h"
#include "Entities/Player.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/RandomPlayerbotMgr.h"
#include "BattleGround/BattleGroundMgr.h"

AiObjectContext* AiFactory::createAiObjectContext(Player* player, PlayerbotAI* ai)
{
    switch (player->getClass())
    {
        case CLASS_PRIEST:
        {
            return new PriestAiObjectContext(ai);
            break;
        }

        case CLASS_MAGE:
        {
            return new MageAiObjectContext(ai);
            break;
        }

        case CLASS_WARLOCK:
        {
            return new WarlockAiObjectContext(ai);
            break;
        }

        case CLASS_WARRIOR:
        {
            return new WarriorAiObjectContext(ai);
            break;
        }

        case CLASS_SHAMAN:
        {
            return new ShamanAiObjectContext(ai);
            break;
        }

        case CLASS_PALADIN:
        {
            return new PaladinAiObjectContext(ai);
            break;
        }

        case CLASS_DRUID:
        {
            return new DruidAiObjectContext(ai);
            break;
        }

        case CLASS_HUNTER:
        {
            return new HunterAiObjectContext(ai);
            break;
        }

        case CLASS_ROGUE:
        {
            return new RogueAiObjectContext(ai);
            break;
        }

#ifdef MANGOSBOT_TWO
        case CLASS_DEATH_KNIGHT:
        {
            return new DKAiObjectContext(ai);
            break;
        }
#endif
    }
    return new AiObjectContext(ai);
}

int AiFactory::GetPlayerSpecTab(const Player* bot)
{
    std::map<uint32, int32> tabs = GetPlayerSpecTabs(bot);

    if (bot->GetLevel() >= 10 && ((tabs[0] + tabs[1] + tabs[2]) > 0))
    {
        int tab = -1, max = 0;
        for (uint32 i = 0; i < uint32(3); i++)
        {
            if (tab == -1 || max < tabs[i])
            {
                tab = i;
                max = tabs[i];
            }
        }
        return tab;
    }
    else
    {
        int tab = 0;

        switch (bot->getClass())
        {
        case CLASS_MAGE:
            tab = 1;
            break;
        case CLASS_PALADIN:
            tab = 2;
            break;
        case CLASS_PRIEST:
            tab = 1;
            break;
        case CLASS_WARRIOR:
            tab = 2;
            break;
        }
        return tab;
    }
}

std::map<uint32, int32> AiFactory::GetPlayerSpecTabs(const Player* bot)
{
    std::map<uint32, int32> tabs;
    for (uint32 i = 0; i < uint32(3); i++)
        tabs[i] = 0;

    uint32 classMask = bot->getClassMask();
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo)
            continue;

        if ((classMask & talentTabInfo->ClassMask) == 0)
            continue;

        int maxRank = 0;
        for (int rank = MAX_TALENT_RANK - 1; rank >= 0; --rank)
        {
            if (!talentInfo->RankID[rank])
                continue;

            uint32 spellid = talentInfo->RankID[rank];
            if (spellid && bot->HasSpell(spellid))
                maxRank = rank + 1;

        }
        tabs[talentTabInfo->tabpage] += maxRank;
    }

    return tabs;
}

BotRoles AiFactory::GetPlayerRoles(const Player* player)
{
    BotRoles role = BOT_ROLE_NONE;
    int tab = GetPlayerSpecTab(player);
    switch (player->getClass())
    {
        case CLASS_PRIEST:
        {
            if (tab == 2)
            {
                role = BOT_ROLE_DPS;
            }
            else
            {
                role = BOT_ROLE_HEALER;
            }

            break;
        }

        case CLASS_SHAMAN:
        {
            if (tab == 2)
            {
                role = BOT_ROLE_HEALER;
            }
            else
            {
                role = BOT_ROLE_DPS;
            }

            break;
        }

        case CLASS_WARRIOR:
        {
            if (tab == 2 || player->HasAura(71)) // Defensive stance
            {
                role = BOT_ROLE_TANK;
            }
            else
            {
                role = BOT_ROLE_DPS;
            }

            break;
        }

        case CLASS_PALADIN:
        {
            if (tab == 1 || player->HasAura(25780)) // Righteous fury
            {
                role = BOT_ROLE_TANK;
            }
            else if (tab == 0)
            {
                role = BOT_ROLE_HEALER;  
            }
            else if (tab == 2)
            {
                role = BOT_ROLE_DPS;
            }

            break;
        }

        case CLASS_DRUID:
        {
            if (player->HasAura(5487) || player->HasAura(9634)) // Bear form, Dire bear form
            {
                role = BOT_ROLE_TANK;
            }
            else if (tab == 0)
            {
                role = BOT_ROLE_DPS;
            }
            else if (tab == 1)
            {
                role = BOT_ROLE_DPS;
            }
            else if (tab == 2)
            {
                role = BOT_ROLE_HEALER;
            }

            break;
        }

#ifdef MANGOSBOT_TWO
        case CLASS_DEATH_KNIGHT:
        {
            if (player->HasAura(48263)) // Frost presence
            {
                role = BOT_ROLE_TANK;
            }
            else if (tab == 0)
            {
                role = BOT_ROLE_TANK;
            }
            else if (tab == 1)
            {
                role = (BotRoles)(BOT_ROLE_TANK | BOT_ROLE_DPS);
            }
            else if (tab == 2)
            {
                role = BOT_ROLE_DPS;
            }

            break;
        }
#endif
        default:
        {
            role = BOT_ROLE_DPS;
            break;
        }
    }


    return role;
}

void AiFactory::AddDefaultCombatStrategies(Player* player, PlayerbotAI* const facade, Engine* combatEngine)
{
    int tab = GetPlayerSpecTab(player);

    combatEngine->addStrategies("mount", NULL);
    combatEngine->addStrategy("avoid mobs");

    if (!player->InBattleGround())
    {
        combatEngine->addStrategies("racials", "default", "quest", "duel", "pvp", NULL);
    }

    switch (player->getClass())
    {
        case CLASS_PRIEST:
        {
            if (tab == 0)
            {
                combatEngine->addStrategy("discipline");
            }
            else if (tab == 1)
            {
                combatEngine->addStrategy("holy");
            }
            else
            {
                combatEngine->addStrategy("shadow");
            }

            combatEngine->addStrategies("dps assist", "flee", "cure", "ranged", "cc", "buff", "aoe", "boost", NULL);
            break;
        }

        case CLASS_MAGE:
        {
            if (tab == 0)
            {
                combatEngine->addStrategy("arcane");
            }
            else if (tab == 1)
            {
                combatEngine->addStrategy("fire");
            }
            else
            {
                combatEngine->addStrategy("frost");
            }

            combatEngine->addStrategies("dps assist", "flee", "cure", "ranged", "cc", "buff", "aoe", "boost", NULL);
            break;
        }

        case CLASS_WARRIOR:
        {
            if (tab == 2)
            {
                combatEngine->addStrategies("protection", "tank assist", "pull", "pull back", "mark rti", NULL);
            }
            else if (player->GetLevel() < 30 || tab == 0)
            {
                combatEngine->addStrategies("arms", "dps assist", "behind", NULL);
            }
            else
            {
                combatEngine->addStrategies("fury", "dps assist", "behind", NULL);
            }

            combatEngine->addStrategies("aoe", "cc", "buff", "boost", NULL);
            break;
        }

        case CLASS_SHAMAN:
        {
            if (tab == 0)
            {
                combatEngine->addStrategies("elemental", "aoe", "cc", "flee", "ranged", NULL);
            }
            else if (tab == 2)
            {
                combatEngine->addStrategies("restoration", "flee", "ranged", NULL);
            }
            else
            {
                combatEngine->addStrategies("enhancement", "aoe", "cc", "close", NULL);
            }

            combatEngine->addStrategies("dps assist", "cure", "totems", "buff", "boost", NULL);
            break;
        }

        case CLASS_PALADIN:
        {
            if (tab == 1)
            {
                combatEngine->addStrategies("protection", "tank assist", "pull", "pull back", "close", NULL);
			}
            else if(tab == 0)
            {
                combatEngine->addStrategies("holy", "dps assist", "flee", "ranged", NULL);
            }
            else
            {
                combatEngine->addStrategies("retribution", "dps assist", "close", NULL);
            }

            combatEngine->addStrategies("cure", "aoe", "cc", "buff", "boost", "aura", "blessing", NULL);
            break;
        }

        case CLASS_DRUID:
        {
            if (tab == 1)
            {
                combatEngine->addStrategies("tank feral", "tank assist", "pull", "pull back", "close", "behind", NULL);
            }
            else if (tab == 2)
            {
                combatEngine->addStrategies("restoration", "dps assist", "flee", "ranged", NULL);
            }
            else
            {
                combatEngine->addStrategies("balance", "dps assist", "flee", "ranged", NULL);
            }

            combatEngine->addStrategies("cure", "aoe", "cc", "buff", "boost", NULL);
            break;
        }

        case CLASS_HUNTER:
        {
            if (tab == 0)
            {
                combatEngine->addStrategy("beast mastery");
            }
            else if (tab == 2)
            {
                combatEngine->addStrategy("survival");
            }
            else
            {
                combatEngine->addStrategy("marksmanship");
            }

            combatEngine->addStrategies("dps assist", "ranged", "cc", "aoe", "buff", "boost", "aspect", "sting", "pet", NULL);
            break;
        }

        case CLASS_ROGUE:
        {
            if (tab == 0)
            {
                combatEngine->addStrategy("assassination");
            }
            else if (tab == 2)
            {
                combatEngine->addStrategy("subtlety");
            }
            else
            {
                combatEngine->addStrategy("combat");
            }

            combatEngine->addStrategies("dps assist", "aoe", "close", "cc", "behind", "stealth", "poisons", "buff", "boost", NULL);
            break;
        }

        case CLASS_WARLOCK:
        {
            if (tab == 0)
            {
                combatEngine->addStrategy("affliction");
            }
            else if (tab == 1)
            {
                combatEngine->addStrategy("demonology");
            }
            else
            {
                combatEngine->addStrategy("destruction");
            }

            combatEngine->addStrategies("dps assist", "flee", "ranged", "cc", "pet", "aoe", "buff", "boost", "curse", NULL);
            break;
        }

#ifdef MANGOSBOT_TWO
        case CLASS_DEATH_KNIGHT:
        {
            if (tab == 0)
            {
                combatEngine->addStrategies("blood", "tank assist", "pull", "pull back", NULL);
            }
            else if (tab == 1)
            {
                combatEngine->addStrategies("frost", "frost aoe", "dps assist", NULL);
            }
            else
            {
                combatEngine->addStrategies("unholy", "unholy aoe", "dps assist", NULL);
            }

            combatEngine->addStrategies("dps assist", "flee", "close", "cc", NULL);
            break;
        }
#endif
    }

    if (facade->IsRealPlayer() || sRandomPlayerbotMgr.IsFreeBot(player))
	{
        combatEngine->addStrategy("roll");

        if (!player->GetGroup())
        {
            combatEngine->addStrategy("flee");
            combatEngine->addStrategy("boost");
            
            if (player->getClass() == CLASS_DRUID && tab == 2)
            {
                combatEngine->addStrategies("balance", "ranged", NULL);
                combatEngine->removeStrategy("close");
            }

            if (player->getClass() == CLASS_DRUID && tab == 1 && urand(0, 100) > 50 && player->GetLevel() >= 20)
            {
                combatEngine->addStrategies("dps feral", "close" "stealth", "behind", NULL);
                combatEngine->removeStrategy("ranged");
            }

            if (player->getClass() == CLASS_PRIEST && tab < 2)
            {
                combatEngine->addStrategy("offdps");
            }

            if (player->getClass() == CLASS_SHAMAN && tab == 2)
            {
                combatEngine->addStrategies("elemental", "aoe", "cc", NULL);
            }

            if (player->getClass() == CLASS_PALADIN && tab == 0)
            {
                combatEngine->addStrategies("retribution", "close", NULL);
                combatEngine->removeStrategy("ranged");
            }
        }

        if ((facade->HasRealPlayerMaster() && sPlayerbotAIConfig.jumpWithPlayer) ||
            (player->InBattleGround() && sPlayerbotAIConfig.jumpInBg) ||
            (!player->InBattleGround() && !facade->HasRealPlayerMaster()))
        {
            if (combatEngine->HasStrategy("close") && frand(0.0f, 1.0f) < sPlayerbotAIConfig.jumpMeleeInCombatChance)
            {
                combatEngine->addStrategy("rpg jump");
            }
        }

        if (sPlayerbotAIConfig.jumpChase)
            combatEngine->addStrategy("chase jump");

        // remove threat for now
        //engine->removeStrategy("threat");

        combatEngine->ChangeStrategy(sPlayerbotAIConfig.randomBotCombatStrategies);
    }
    else
    {
        combatEngine->ChangeStrategy(sPlayerbotAIConfig.combatStrategies);
    }

    // Battleground switch
    if (player->InBattleGround())
    {
        BattleGroundTypeId bgType = player->GetBattleGroundTypeId();

#ifdef MANGOSBOT_TWO
        if (bgType == BATTLEGROUND_RB)
        {
            bgType = player->GetBattleGround()->GetTypeId(true);
        }

        if (bgType == BATTLEGROUND_IC)
        {
            combatEngine->addStrategy("isle");
        }
#endif

        if (bgType == BATTLEGROUND_WS)
        {
            combatEngine->addStrategy("warsong");
        }

        if (bgType == BATTLEGROUND_AB)
        {
            combatEngine->addStrategy("arathi");
        }

        if(bgType == BATTLEGROUND_AV)
        {
            combatEngine->addStrategy("alterac");
        }

#ifndef MANGOSBOT_ZERO
        if (bgType == BATTLEGROUND_EY)
        {
            combatEngine->addStrategy("eye");
        }
#endif

#ifndef MANGOSBOT_ZERO
        if (!player->IsBeingTeleported() && player->InArena())
        {
            combatEngine->addStrategy("arena");
        }
#endif
        combatEngine->addStrategies("boost", "racials", "default", "aoe", "dps assist", "pvp", NULL);
        combatEngine->removeStrategy("custom::say");
        combatEngine->removeStrategy("flee");
        combatEngine->removeStrategy("threat");
        combatEngine->removeStrategy("follow");
        combatEngine->removeStrategy("conserve mana");
        combatEngine->removeStrategy("cast time");

        if (player->getClass() == CLASS_SHAMAN && tab == 2)
        {
            combatEngine->addStrategies("elemental", "aoe", "cc", NULL);
        }

        if (player->getClass() == CLASS_DRUID && tab == 2)
        {
            combatEngine->addStrategies("balance", NULL);
        }

        if (player->getClass() == CLASS_DRUID && tab == 1)
        {
            combatEngine->addStrategies("behind", "dps feral", "stealth", NULL);
        }
        
        if (player->getClass() == CLASS_ROGUE)
        {
            combatEngine->addStrategies("behind", "stealth", "poisons", "buff", NULL);
        }
    }
}

Engine* AiFactory::createCombatEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext)
{
	Engine* engine = new Engine(facade, AiObjectContext, BotState::BOT_STATE_COMBAT);
    AddDefaultCombatStrategies(player, facade, engine);
    return engine;
}

void AiFactory::AddDefaultNonCombatStrategies(Player* player, PlayerbotAI* const facade, Engine* nonCombatEngine)
{
    const int tab = GetPlayerSpecTab(player);
    switch (player->getClass())
    {
        case CLASS_PRIEST:
        {
            if (tab == 0)
            {
                nonCombatEngine->addStrategy("discipline");
            }
            else if (tab == 1)
            {
                nonCombatEngine->addStrategy("holy");
            }
            else
            {
                nonCombatEngine->addStrategy("shadow");
            }

            nonCombatEngine->addStrategies("dps assist", "cure", "buff", "boost", NULL);
            break;
        }

        case CLASS_PALADIN:
        {
            if (tab == 1)
            {
                nonCombatEngine->addStrategies("protection", "tank assist", NULL);
            }
            else if (tab == 0)
            {
                nonCombatEngine->addStrategies("holy", "dps assist", NULL);
            }
            else
            {
                nonCombatEngine->addStrategies("retribution", "dps assist", NULL);
            }

            nonCombatEngine->addStrategies("cure", "buff", "boost", "blessing", "aura", NULL);
            break;
        }

        case CLASS_HUNTER:
        {
            if (tab == 0)
            {
                nonCombatEngine->addStrategy("beast mastery");
            }
            else if (tab == 2)
            {
                nonCombatEngine->addStrategy("survival");
            }
            else
            {
                nonCombatEngine->addStrategy("marksmanship");
            }

            nonCombatEngine->addStrategies("dps assist", "cc", "aoe", "buff", "boost", "aspect", "pet", NULL);
            break;
        }

        case CLASS_SHAMAN:
        {
            if (tab == 0)
            {
                nonCombatEngine->addStrategies("elemental", "aoe", "cc", NULL);
            }
            else if (tab == 2)
            {
                nonCombatEngine->addStrategies("restoration", NULL);
            }
            else
            {
                nonCombatEngine->addStrategies("enhancement", "aoe", "cc", NULL);
            }

            nonCombatEngine->addStrategies("dps assist", "cure", "totems", "buff", "boost", NULL);
            break;
        }

        case CLASS_ROGUE:
        {
            if (tab == 0)
            {
                nonCombatEngine->addStrategy("assassination");
            }
            else if (tab == 2)
            {
                nonCombatEngine->addStrategy("subtlety");
            }
            else
            {
                nonCombatEngine->addStrategy("combat");
            }

            nonCombatEngine->addStrategies("dps assist", "poisons", "stealth", "buff", "boost", NULL);
            break;
        }

        case CLASS_WARLOCK:
        {
            if (tab == 0)
            {
                nonCombatEngine->addStrategy("affliction");
            }
            else if (tab == 1)
            {
                nonCombatEngine->addStrategy("demonology");
            }
            else
            {
                nonCombatEngine->addStrategy("destruction");
            }

            nonCombatEngine->addStrategies("dps assist", "pet", "buff", "boost", NULL);
            break;
        }

        case CLASS_MAGE:
        {
            if (tab == 0)
            {
                nonCombatEngine->addStrategy("arcane");
            }
            else if (tab == 1)
            {
                nonCombatEngine->addStrategy("fire");
            }
            else
            {
                nonCombatEngine->addStrategy("frost");
            }

            nonCombatEngine->addStrategies("dps assist", "cure", "buff", "boost", NULL);
            break;
        }

        case CLASS_DRUID:
        {
            if (tab == 1)
            {
                nonCombatEngine->addStrategies("tank feral", "tank assist", NULL);
            }
            else if (tab == 2)
            {
                nonCombatEngine->addStrategies("restoration", "dps assist", NULL);
            }
            else
            {
                nonCombatEngine->addStrategies("balance", "dps assist", NULL);
            }

            nonCombatEngine->addStrategies("cure", "aoe", "cc", "buff", "boost", NULL);
            break;
        }

        case CLASS_WARRIOR:
        {
            if (tab == 2)
            {
                nonCombatEngine->addStrategies("protection", "tank assist", NULL);
            }
            else if(player->GetLevel() < 30 || tab == 0)
            {
                nonCombatEngine->addStrategies("arms", "dps assist", NULL);
            }
            else
            {
                nonCombatEngine->addStrategies("fury", "dps assist", NULL);
            }

            nonCombatEngine->addStrategies("buff", "boost", NULL);
            break;
        }

#ifdef MANGOSBOT_TWO
        case CLASS_DEATH_KNIGHT:
        {
            if (tab == 0)
            {
                nonCombatEngine->addStrategy("tank assist");
            }
            else
            {
                nonCombatEngine->addStrategy("dps assist");
            }

            break;
        }
#endif

        default:
        {
            nonCombatEngine->addStrategy("dps assist");
            break;
        }
    }

    nonCombatEngine->addStrategies("wbuff", NULL);
    nonCombatEngine->addStrategy("avoid mobs");

    if(sPlayerbotAIConfig.llmEnabled == 2)
        nonCombatEngine->addStrategy("ai chat");

    if (!player->InBattleGround())
    {
        nonCombatEngine->addStrategies("racials", "nc", "food", "follow", "default", "quest", "loot", "gather", "duel", "emote", "buff", "mount", NULL);
    }

    if ((facade->HasRealPlayerMaster() && sPlayerbotAIConfig.jumpWithPlayer) ||
        (player->InBattleGround() && sPlayerbotAIConfig.jumpInBg) ||
        (!player->InBattleGround() && !facade->HasRealPlayerMaster()))
    {
        if (frand(0.0f, 1.0f) < sPlayerbotAIConfig.jumpNoCombatChance)
        {
            nonCombatEngine->addStrategy("rpg jump");
        }
    }

    if (!player->InBattleGround() && sPlayerbotAIConfig.jumpFollow)
        nonCombatEngine->addStrategy("follow jump");

    if ((facade->IsRealPlayer() || sRandomPlayerbotMgr.IsFreeBot(player)) && !player->InBattleGround())
    {   
        Player* master = facade->GetMaster();

        nonCombatEngine->addStrategy("roll");
#ifdef MANGOSBOT_TWO
        nonCombatEngine->addStrategy("glyph");
#endif

        // let 25% of free bots start duels.
        if (!urand(0, 3))
        {
            nonCombatEngine->addStrategy("start duel");
        }

        if (sPlayerbotAIConfig.randomBotJoinLfg)
        {
            nonCombatEngine->addStrategy("lfg");
        }

        if (!player->GetGroup() || facade->IsGroupLeader())
        {
            // let 25% of random not grouped (or group leader) bots help other players
            if (!urand(0, 3))
            {
                nonCombatEngine->addStrategy("attack tagged");
            }

            nonCombatEngine->addStrategy("collision");
            nonCombatEngine->addStrategy("grind");            
            nonCombatEngine->addStrategy("group");
            nonCombatEngine->addStrategy("guild");

            if (sPlayerbotAIConfig.autoDoQuests)
            {
                nonCombatEngine->addStrategy("travel");
                nonCombatEngine->addStrategy("tfish");
                nonCombatEngine->addStrategy("rpg");                
                nonCombatEngine->removeStrategy("rpg craft");
            }

            if (sPlayerbotAIConfig.randomBotJoinBG)
            {
                nonCombatEngine->addStrategy("bg");
            }

            if(!master || master->GetPlayerbotAI())
            {
                nonCombatEngine->addStrategy("maintenance");
            }

            nonCombatEngine->ChangeStrategy(sPlayerbotAIConfig.randomBotNonCombatStrategies);
        }
        else 
        {
            if (facade)
            {
                if (master)
                {
                    if (master->GetPlayerbotAI() || sRandomPlayerbotMgr.IsFreeBot(player))
                    {
                        nonCombatEngine->addStrategy("collision");
                        nonCombatEngine->addStrategy("grind");
                        nonCombatEngine->addStrategy("group");
                        nonCombatEngine->addStrategy("guild");

                        if (sPlayerbotAIConfig.autoDoQuests)
                        {
                            nonCombatEngine->addStrategy("travel");
                            nonCombatEngine->addStrategy("tfish");
                            nonCombatEngine->addStrategy("rpg");
                            nonCombatEngine->removeStrategy("rpg craft");

                        }

                        if (!master || master->GetPlayerbotAI())
                        {
                            nonCombatEngine->addStrategy("maintenance");
                        }

                        nonCombatEngine->ChangeStrategy(sPlayerbotAIConfig.randomBotNonCombatStrategies);
                    }
                    else
                    {
                        nonCombatEngine->ChangeStrategy(sPlayerbotAIConfig.nonCombatStrategies);
                    }
                }
            }
        }
    }
    else
    {
        nonCombatEngine->ChangeStrategy(sPlayerbotAIConfig.nonCombatStrategies);
    }

    // Battleground switch
    if (player->InBattleGround())
    {
        nonCombatEngine->addStrategies("racials", "nc", "default", "buff", "food", "mount", "collision", "dps assist", "attack tagged", "emote", NULL);
        nonCombatEngine->removeStrategy("custom::say");
        nonCombatEngine->removeStrategy("travel");
        nonCombatEngine->removeStrategy("tfish");
        nonCombatEngine->removeStrategy("rpg");
        nonCombatEngine->removeStrategy("rpg craft");
        nonCombatEngine->removeStrategy("follow");

        nonCombatEngine->removeStrategy("grind");

        BattleGroundTypeId bgType = player->GetBattleGroundTypeId();
#ifdef MANGOSBOT_TWO
        if (bgType == BATTLEGROUND_RB)
        {
            bgType = player->GetBattleGround()->GetTypeId(true);
        }
#endif

        bool isArena = false;

#ifndef MANGOSBOT_ZERO
        if (!player->IsBeingTeleported() && player->InArena())
        {
            isArena = true;
        }
#endif
        if (isArena)
        {
            nonCombatEngine->addStrategy("arena");
            nonCombatEngine->removeStrategy("mount");
        }
        else
        {
#ifndef MANGOSBOT_ZERO
#ifdef MANGOSBOT_TWO
            if (bgType <= BATTLEGROUND_EY || bgType == BATTLEGROUND_IC) // do not add for not supported bg
            {
                nonCombatEngine->addStrategy("battleground");
            }
#else
            if (bgType <= BATTLEGROUND_EY) // do not add for not supported bg
            {
                nonCombatEngine->addStrategy("battleground");
            }
#endif
#else
            if (bgType <= BATTLEGROUND_AB) // do not add for not supported bg
            {
                nonCombatEngine->addStrategy("battleground");
            }
#endif

            if (bgType == BATTLEGROUND_WS)
            {
                nonCombatEngine->addStrategy("warsong");
            }

            if (bgType == BATTLEGROUND_AV)
            {
                nonCombatEngine->addStrategy("alterac");
            }

            if (bgType == BATTLEGROUND_AB)
            {
                nonCombatEngine->addStrategy("arathi");
            }

#ifndef MANGOSBOT_ZERO
            if (bgType == BATTLEGROUND_EY)
            {
                nonCombatEngine->addStrategy("eye");
            }
#endif
#ifdef MANGOSBOT_TWO
            if (bgType == BATTLEGROUND_IC)
            {
                nonCombatEngine->addStrategy("isle");
            }
#endif
        }
    }
}

Engine* AiFactory::createNonCombatEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext) 
{
	Engine* nonCombatEngine = new Engine(facade, AiObjectContext, BotState::BOT_STATE_NON_COMBAT);
    AddDefaultNonCombatStrategies(player, facade, nonCombatEngine);
	return nonCombatEngine;
}

void AiFactory::AddDefaultDeadStrategies(Player* player, PlayerbotAI* const facade, Engine* deadEngine)
{
    deadEngine->addStrategies("dead", "stay", "default", "follow", "group", NULL);
    if (sRandomPlayerbotMgr.IsFreeBot(player) && !player->GetGroup())
    {
        deadEngine->removeStrategy("follow");
    }

    const int tab = GetPlayerSpecTab(player);
    switch (player->getClass())
    {
        case CLASS_SHAMAN:
        {
            if (tab == 0)
            {
                deadEngine->addStrategy("elemental");
            }
            else if (tab == 2)
            {
                deadEngine->addStrategy("restoration");
            }
            else
            {
                deadEngine->addStrategy("enhancement");
            }

            break;
        }

        case CLASS_ROGUE:
        {
            if (tab == 0)
            {
                deadEngine->addStrategy("assassination");
            }
            else if (tab == 2)
            {
                deadEngine->addStrategy("subtlety");
            }
            else
            {
                deadEngine->addStrategy("combat");
            }

            break;
        }

        case CLASS_WARLOCK:
        {
            if (tab == 0)
            {
                deadEngine->addStrategy("affliction");
            }
            else if (tab == 1)
            {
                deadEngine->addStrategy("demonology");
            }
            else
            {
                deadEngine->addStrategy("destruction");
            }

            break;
        }

        case CLASS_MAGE:
        {
            if (tab == 0)
            {
                deadEngine->addStrategy("arcane");
            }
            else if (tab == 1)
            {
                deadEngine->addStrategy("fire");
            }
            else
            {
                deadEngine->addStrategy("frost");
            }

            break;
        }

        case CLASS_PALADIN:
        {
            if (tab == 1)
            {
                deadEngine->addStrategy("protection");
            }
            else if (tab == 0)
            {
                deadEngine->addStrategy("holy");
            }
            else
            {
                deadEngine->addStrategy("retribution");
            }

            break;
        }

        case CLASS_WARRIOR:
        {
            if (tab == 2)
            {
                deadEngine->addStrategy("protection");
            }
            else if (tab == 0)
            {
                deadEngine->addStrategy("arms");
            }
            else
            {
                deadEngine->addStrategy("fury");
            }

            break;
        }

        case CLASS_DRUID:
        {
            if (tab == 1)
            {
                deadEngine->addStrategy("tank feral");
            }
            else if (tab == 2)
            {
                deadEngine->addStrategy("restoration");
            }
            else
            {
                deadEngine->addStrategy("balance");
            }

            break;
        }
        
        case CLASS_HUNTER:
        {
            if (tab == 0)
            {
                deadEngine->addStrategy("beast mastery");
            }
            else if (tab == 2)
            {
                deadEngine->addStrategy("survival");
            }
            else
            {
                deadEngine->addStrategy("marksmanship");
            }

            break;
        }

        case CLASS_PRIEST:
        {
            if (tab == 0)
            {
                deadEngine->addStrategy("discipline");
            }
            else if (tab == 1)
            {
                deadEngine->addStrategy("holy");
            }
            else
            {
                deadEngine->addStrategy("shadow");
            }

            break;
        }
    }

    if (facade->IsRealPlayer() || sRandomPlayerbotMgr.IsFreeBot(player))
    {
        deadEngine->ChangeStrategy(sPlayerbotAIConfig.randomBotDeadStrategies);
    }
    else
    {
        deadEngine->ChangeStrategy(sPlayerbotAIConfig.deadStrategies);
    }
}

Engine* AiFactory::createDeadEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext)
{
    Engine* deadEngine = new Engine(facade, AiObjectContext, BotState::BOT_STATE_DEAD);
    AddDefaultDeadStrategies(player, facade, deadEngine);
    return deadEngine;
}

void AiFactory::AddDefaultReactionStrategies(Player* player, PlayerbotAI* const facade, ReactionEngine* reactionEngine)
{
    reactionEngine->addStrategies("react", "chat", "avoid aoe", "potions", NULL);

    const int tab = GetPlayerSpecTab(player);
    switch (player->getClass())
    {
        case CLASS_SHAMAN:
        {
            if (tab == 0)
            {
                reactionEngine->addStrategy("elemental");
            }
            else if (tab == 2)
            {
                reactionEngine->addStrategy("restoration");
            }
            else
            {
                reactionEngine->addStrategy("enhancement");
            }

            break;
        }

        case CLASS_ROGUE:
        {
            if (tab == 0)
            {
                reactionEngine->addStrategy("assassination");
            }
            else if (tab == 2)
            {
                reactionEngine->addStrategy("subtlety");
            }
            else
            {
                reactionEngine->addStrategy("combat");
            }

            break;
        }

        case CLASS_WARLOCK:
        {
            if (tab == 0)
            {
                reactionEngine->addStrategy("affliction");
            }
            else if (tab == 1)
            {
                reactionEngine->addStrategy("demonology");
            }
            else
            {
                reactionEngine->addStrategy("destruction");
            }

            break;
        }

        case CLASS_MAGE:
        {
            if (tab == 0)
            {
                reactionEngine->addStrategy("arcane");
            }
            else if (tab == 1)
            {
                reactionEngine->addStrategy("fire");
            }
            else
            {
                reactionEngine->addStrategy("frost");
            }

            break;
        }

        case CLASS_PALADIN:
        {
            if (tab == 1)
            {
                reactionEngine->addStrategy("protection");
            }
            else if (tab == 0)
            {
                reactionEngine->addStrategy("holy");
            }
            else
            {
                reactionEngine->addStrategy("retribution");
            }

            break;
        }

        case CLASS_WARRIOR:
        {
            if (tab == 2)
            {
                reactionEngine->addStrategy("protection");
            }
            else if (tab == 0)
            {
                reactionEngine->addStrategy("arms");
            }
            else
            {
                reactionEngine->addStrategy("fury");
            }

            break;
        }

        case CLASS_DRUID:
        {
            if (tab == 1)
            {
                reactionEngine->addStrategy("tank feral");
            }
            else if (tab == 2)
            {
                reactionEngine->addStrategy("restoration");
            }
            else
            {
                reactionEngine->addStrategy("balance");
            }

            break;
        }

        case CLASS_HUNTER:
        {
            if (tab == 0)
            {
                reactionEngine->addStrategy("beast mastery");
            }
            else if (tab == 2)
            {
                reactionEngine->addStrategy("survival");
            }
            else
            {
                reactionEngine->addStrategy("marksmanship");
            }

            break;
        }

        case CLASS_PRIEST:
        {
            if (tab == 0)
            {
                reactionEngine->addStrategy("discipline");
            }
            else if (tab == 1)
            {
                reactionEngine->addStrategy("holy");
            }
            else
            {
                reactionEngine->addStrategy("shadow");
            }

            break;
        }
    }

    if (facade->IsRealPlayer() || sRandomPlayerbotMgr.IsFreeBot(player))
    {
        reactionEngine->ChangeStrategy(sPlayerbotAIConfig.randomBotReactStrategies);
    }
    else
    {
        reactionEngine->ChangeStrategy(sPlayerbotAIConfig.reactStrategies);
    }
}

ReactionEngine* AiFactory::createReactionEngine(Player* player, PlayerbotAI* const facade, AiObjectContext* AiObjectContext)
{
    ReactionEngine* reactionEngine = new ReactionEngine(facade, AiObjectContext, BotState::BOT_STATE_REACTION);
    AddDefaultReactionStrategies(player, facade, reactionEngine);
    return reactionEngine;
}
