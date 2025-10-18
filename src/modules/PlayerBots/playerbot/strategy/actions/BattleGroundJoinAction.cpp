#include "Entities/ObjectGuid.h"

#include "playerbot/playerbot.h"
#include "playerbot/PlayerbotAI.h"
#include "LfgActions.h"
#include "playerbot/AiFactory.h"
//#include "playerbot/PlayerbotAIConfig.h"
//#include "playerbot/strategy/ItemVisitors.h"
#include "playerbot/RandomPlayerbotMgr.h"
//#include "../../../../game/LFGMgr.h"
//#include "strategy/values/PositionValue.h"
//#include "playerbot/ServerFacade.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "Entities/Object.h"
#include "Globals/ObjectMgr.h"
#include "playerbot/strategy/values/LastMovementValue.h"
#include "playerbot/strategy/actions/LogLevelAction.h"
#include "playerbot/strategy/values/LastSpellCastValue.h"
#include "playerbot/strategy/values/PositionValue.h"
#include "MovementActions.h"
#include "MotionGenerators/MotionMaster.h"
#include "MotionGenerators/MovementGenerator.h"
//#include "playerbot/strategy/values/PositionValue.h"
#include "MotionGenerators/TargetedMovementGenerator.h"
#include "BattleGround/BattleGround.h"
#include "BattleGround/BattleGroundMgr.h"
#include "BattleGroundJoinAction.h"
#ifndef MANGOSBOT_ZERO
#ifdef CMANGOS
#include "Arena/ArenaTeam.h"
#endif
#ifdef MANGOS
#include "ArenaTeam.h"
#endif
#endif

using namespace ai;


bool BGJoinAction::Execute(Event& event)
{
    uint32 queueType = AI_VALUE(uint32, "bg type");
    if (!queueType) // force join to fill bg
    {
        if (bgList.empty())
            return false;

        BattleGroundQueueTypeId queueTypeId = (BattleGroundQueueTypeId)bgList[urand(0, bgList.size() - 1)];
        BattleGroundTypeId bgTypeId = sServerFacade.BgTemplateId(queueTypeId);
        BattleGroundBracketId bracketId;
        bool isArena = false;
        bool isRated = false;

        BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
        if (!bg)
            return false;

#ifndef MANGOSBOT_TWO
        bracketId = sBattleGroundMgr.GetBattleGroundBracketIdFromLevel(bgTypeId, bot->GetLevel());
#endif
#ifdef MANGOSBOT_TWO
        uint32 mapId = bg->GetMapId();
        PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, bot->GetLevel());
        if (!pvpDiff)
            return false;

        bracketId = pvpDiff->GetBracketId();
#endif

        // Find BattleMaster by Entry
        //uint32 BmEntry = sRandomPlayerbotMgr.GetBattleMasterEntry(bot, bgTypeId, true);
        //if (!BmEntry)
        //{
        //    sLog.outError("Bot #%d <%s> could not find Battlemaster for %d", bot->GetGUIDLow(), bot->GetName(), bgTypeId);
        //    return false;
        //}

#ifndef MANGOSBOT_ZERO
        if (ArenaType type = sServerFacade.BgArenaType(queueTypeId))
        {
            isArena = true;

            std::vector<uint32>::iterator i = find(ratedList.begin(), ratedList.end(), queueTypeId);
            if (i != ratedList.end())
                isRated = true;

            if (isRated && !gatherArenaTeam(type))
            {
                sLog.outDetail("Team %dv%d, captain #%d <%s>: error making arena group!", uint32(type), uint32(type), bot->GetGUIDLow(), bot->GetName());
                return false;
            }

            ai->GetAiObjectContext()->GetValue<uint32>("arena type")->Set(isRated);
        }
#endif

        // set bg type and bm guid
        //ai->GetAiObjectContext()->GetValue<ObjectGuid>("bg master")->Set(BmGuid);

        std::string _bgType;

        switch (bgTypeId)
        {
        case BATTLEGROUND_AV:
            _bgType = "AV";
            break;
        case BATTLEGROUND_WS:
            _bgType = "WSG";
            break;
        case BATTLEGROUND_AB:
            _bgType = "AB";
            break;
#ifndef MANGOSBOT_ZERO
        case BATTLEGROUND_EY:
            _bgType = "EotS";
            break;
#endif
#ifdef MANGOSBOT_TWO
        case BATTLEGROUND_RB:
            _bgType = "Random";
            break;
        case BATTLEGROUND_SA:
            _bgType = "SotA";
            break;
        case BATTLEGROUND_IC:
            _bgType = "IoC";
            break;
#endif
        default:
            break;
        }

        ai->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(queueTypeId);
        queueType = queueTypeId;
        sPlayerbotAIConfig.logEvent(ai, "BGJoinAction", _bgType, std::to_string(queueTypeId));
    }

   return JoinQueue(queueType);
}

#ifndef MANGOSBOT_ZERO
bool BGJoinAction::gatherArenaTeam(ArenaType type)
{
    ArenaTeam* arenateam = nullptr;
    Group* leaderGroup = nullptr;
    uint32 needMembers = (uint32)type;
    //if (bot->GetGroup() && bot->GetGroup()->IsLeader(bot->GetObjectGuid()))
    //    leaderGroup = bot->GetGroup();

    for (uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
    {
        ArenaTeam* temp = sObjectMgr.GetArenaTeamById(bot->GetArenaTeamId(arena_slot));
        if (!temp)
            continue;

        if (temp->GetCaptainGuid() != bot->GetObjectGuid())
            continue;

        if (temp->GetType() != type)
            continue;

        //if ((int)temp->GetMembersSize() < (int)needMembers)
        //    continue;

        arenateam = temp;
    }
    if (!arenateam)
    {
        sLog.outDetail("Bot #%d <%s>: No proper arena team found!", bot->GetGUIDLow(), bot->GetName());
        return false;
    }

    std::vector<uint32> members;

    // search for arena team members and make them online
    for (ArenaTeam::MemberList::iterator itr = arenateam->GetMembers().begin(); itr != arenateam->GetMembers().end(); ++itr)
    {
        if ((int)members.size() >= (int)needMembers)
            break;

        bool offline = false;
        Player* member = sObjectMgr.GetPlayer(itr->guid);
        if (!member)
        {
            offline = true;
        }
        //if (!member && !sObjectMgr.GetPlayerAccountIdByGUID(itr->guid))
        //    continue;

        if (offline)
        {
            if (!sRandomPlayerbotMgr.AddRandomBot(itr->guid.GetCounter()))
            {
                sLog.outDetail("Team #%d <%s>: error logging in an offline member!", arenateam->GetId(), arenateam->GetName().c_str());
                return false;
            }
            else
                continue;
        }

        //member = sObjectMgr.GetPlayer(itr->guid);

        if (member)
        {
            if (!member->GetPlayerbotAI())
                continue;

            if (member->GetGroup() && member->GetPlayerbotAI()->HasRealPlayerMaster())
                continue;

            if (!sPlayerbotAIConfig.IsInRandomAccountList(member->GetSession()->GetAccountId()))
                continue;

            if (member->GetObjectGuid() == bot->GetObjectGuid())
                continue;

            if (member->IsInCombat())
                member->CombatStop(true);

            if (member->GetGroup() && member->GetGroup() != leaderGroup)
                member->GetGroup()->RemoveMember(member->GetObjectGuid(), 0);

            member->TeleportTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), 0);

            member->GetPlayerbotAI()->Reset();
        }

        if (member)
            members.push_back(member->GetGUIDLow());
    }

    if (!members.size() || (int)members.size() < (int)(needMembers - 1))
    {
        sLog.outDetail("Team #%d <%s> has not enough members for match", arenateam->GetId(), arenateam->GetName().c_str());
        return false;
    }

    if (!leaderGroup)
    {
#ifndef MANGOSBOT_TWO
        Group* group = new Group();
#else
        Group* group = new Group();//Group* group = new Group(GROUPTYPE_NORMAL);
#endif
        uint32 count = 1;

        // disband leaders group
        if (bot->GetGroup())
            bot->GetGroup()->RemoveMember(bot->GetObjectGuid(), 0);

        if (!group->Create(bot->GetObjectGuid(), bot->GetName()))
        {
            sLog.outDetail("Team #%d <%s>: Can't create group for arena queue", arenateam->GetId(), arenateam->GetName().c_str());
            delete group;
            return false;
        }
        else
        {
            sObjectMgr.AddGroup(group);
            leaderGroup = group;
        }
    }

    sLog.outDetail("Bot #%d <%s>: Leader of <%s>", bot->GetGUIDLow(), bot->GetName(), arenateam->GetName().c_str());

    for (auto i = begin(members); i != end(members); ++i)
    {
        if (*i == bot->GetGUIDLow())
            continue;

        //if (count >= (int)arenateam->GetType())
        //    break;

        if (leaderGroup->GetMembersCount() >= needMembers)
            break;

        Player* member = sObjectMgr.GetPlayer(ObjectGuid(HIGHGUID_PLAYER, *i));
        if (!member)
            continue;

        if (member->GetLevel() < DEFAULT_MAX_LEVEL)
            continue;

        if (!member->GetPlayerbotAI())
            continue;

        if (member->GetGroup() == leaderGroup)
            continue;

        if (!leaderGroup->AddMember(ObjectGuid(HIGHGUID_PLAYER, *i), member->GetName()))
            continue;

        member->GetPlayerbotAI()->Reset(true);

        if (!member->IsWithinDistInMap(bot, sPlayerbotAIConfig.sightDistance, false))
            member->TeleportTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), 0);

        sLog.outDetail("Bot #%d <%s>: Member of <%s>", member->GetGUIDLow(), member->GetName(), arenateam->GetName().c_str());
    }

    if (leaderGroup && leaderGroup->GetMembersCount() >= needMembers)
    {
        sLog.outBasic("Team #%d <%s>: Group is ready for match", arenateam->GetId(), arenateam->GetName().c_str());
        return true;
    }
    else if (leaderGroup && leaderGroup->GetMembersCount() < needMembers)
    {
        sLog.outDetail("Team #%d <%s>: Group is not ready for match (not enough members)", arenateam->GetId(), arenateam->GetName().c_str());
        leaderGroup->Disband();
        return false;
    }
    return false;
}
#endif

bool BGJoinAction::canJoinBg(Player* player, BattleGroundQueueTypeId queueTypeId, BattleGroundBracketId bracketId)
{
    // check if bot can join this bg/bracket

    BattleGroundTypeId bgTypeId = sServerFacade.BgTemplateId(queueTypeId);

    // check if already in queue
    if (player->InBattleGroundQueueForBattleGroundQueueType(queueTypeId))
        return false;

    // check too low/high level
    if (!player->GetBGAccessByLevel(bgTypeId))
        return false;

    // check bracket
#ifndef MANGOSBOT_TWO
    if (sBattleGroundMgr.GetBattleGroundBracketIdFromLevel(bgTypeId, bot->GetLevel()) != bracketId)
        return false;
#endif
#ifdef MANGOSBOT_TWO
    BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
    uint32 mapId = bg->GetMapId();
    PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, player->GetLevel());
    if (!pvpDiff)
        return false;

    BattleGroundBracketId bracket_temp = pvpDiff->GetBracketId();

    if (bracket_temp != bracketId)
        return false;
#endif
    return true;
}

bool BGJoinAction::shouldJoinBg(BattleGroundQueueTypeId queueTypeId, BattleGroundBracketId bracketId)
{
    // check if bot should join (queue has real players)

    BattleGroundTypeId bgTypeId = sServerFacade.BgTemplateId(queueTypeId);
    BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
    if (!bg)
        return false;

    bool isArena = false;
    bool isRated = false;
    bool hasTeam = false;
    bool isTeamLead = false;
    bool noLag = sWorld.GetAverageDiff() < (sRandomPlayerbotMgr.GetPlayers().empty() ? sPlayerbotAIConfig.diffEmpty : sPlayerbotAIConfig.diffWithPlayer) * 1.1;

    if (sPlayerbotAIConfig.disableActivityPriorities)
    {
        noLag = true;
    }

#ifndef MANGOSBOT_ZERO
    ArenaType type = sServerFacade.BgArenaType(queueTypeId);
    if (type != ARENA_TYPE_NONE)
        isArena = true;

    for (uint8 i = 0; i < MAX_ARENA_SLOT; ++i)
    {
        if (uint32 a_id = bot->GetArenaTeamId(i))
        {
            hasTeam = true;
            break;
        }
    }

    isTeamLead = sObjectMgr.GetArenaTeamByCaptain(bot->GetObjectGuid());
#endif
    bool hasPlayers = (sRandomPlayerbotMgr.BgPlayers[queueTypeId][bracketId][0] + sRandomPlayerbotMgr.BgPlayers[queueTypeId][bracketId][1]) > 0;
    bool hasBots = (sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][0] + sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][1]) >= bg->GetMinPlayersPerTeam();
    if (!sPlayerbotAIConfig.randomBotAutoJoinBG && !hasPlayers)
        return false;

    if (!hasPlayers && !noLag)
        return false;

    if (ai->HasRealPlayerMaster())
        return false;

#ifndef MANGOSBOT_ZERO
    if (!hasPlayers && isArena && !hasTeam)
        return false;

    if (!hasPlayers && !isArena && hasTeam)
        return false;
#endif

#ifdef MANGOSBOT_TWO
    if (!hasPlayers && bgTypeId == BATTLEGROUND_RB)
        return false;

    if (!hasPlayers && bgTypeId == BATTLEGROUND_SA)
        return false;
#endif

    uint32 BracketSize = bg->GetMaxPlayers();
    uint32 TeamSize = bg->GetMaxPlayersPerTeam();

    uint32 ACount = sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][0] + sRandomPlayerbotMgr.BgPlayers[queueTypeId][bracketId][0];
    uint32 HCount = sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][1] + sRandomPlayerbotMgr.BgPlayers[queueTypeId][bracketId][1];

    uint32 BgCount = ACount + HCount;
    uint32 SCount = 0;
    uint32 RCount = 0;

    uint32 TeamId = bot->GetTeam() == ALLIANCE ? 0 : 1;

    //if (!hasPlayers && !isArena)
    //{
    //    if (BgCount >= bg->GetMaxPlayers())
    //        return false;
    //}

#ifndef MANGOSBOT_ZERO
    if (isArena)
    {
        uint32 rated_players = sRandomPlayerbotMgr.BgPlayers[queueTypeId][bracketId][1];
        if (rated_players)
        {
            isRated = true;
        }
        if (!hasPlayers && isTeamLead)
            isRated = true;

        // do not queue skirmish without real players
        if (!hasPlayers && !(isTeamLead || isRated))
            return false;

        isArena = true;
        BracketSize = (uint32)(type * 2);
        TeamSize = type;
        ACount = sRandomPlayerbotMgr.ArenaBots[queueTypeId][bracketId][isRated][0];
        HCount = sRandomPlayerbotMgr.ArenaBots[queueTypeId][bracketId][isRated][1];
        BgCount = sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][isRated] + sRandomPlayerbotMgr.BgPlayers[queueTypeId][bracketId][isRated];
        SCount = sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][0] + sRandomPlayerbotMgr.BgPlayers[queueTypeId][bracketId][0];
        RCount = sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][1] + sRandomPlayerbotMgr.BgPlayers[queueTypeId][bracketId][1];
    }

    // do not try if not a captain of arena team

    if (isRated)
    {
        if (!isTeamLead)
            return false;

        // check if bot has correct team
        ArenaTeam* arenateam = nullptr;
        for (uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
        {
            ArenaTeam* temp = sObjectMgr.GetArenaTeamById(bot->GetArenaTeamId(arena_slot));
            if (!temp)
                continue;

            if (temp->GetType() != type)
                continue;

            arenateam = temp;
        }

        if (!arenateam)
            return false;

        ratedList.push_back(queueTypeId);
    }
#endif

    // hack fix crash in queue remove event
    if (!isArena && bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetObjectGuid()))
        return false;

    bool needBots = sRandomPlayerbotMgr.NeedBots[queueTypeId][bracketId][isArena ? isRated : GetTeamIndexByTeamId(bot->GetTeam())];

    // add more bots if players are not invited or if 1st BG instance is full
    if (needBots || (hasPlayers && BgCount > BracketSize && (BgCount % BracketSize) != 0))
    {
        bool needAlly = HCount >= ACount;
        bool needHorde = ACount >= HCount;
        uint32 needMoreBots = 0;
        if (BgCount > BracketSize)
            needMoreBots = ((uint32(BgCount / BracketSize) + 1) * BracketSize) - (uint32(BgCount / BracketSize) * BracketSize) - (BgCount % BracketSize);
        else
            needMoreBots = BracketSize - BgCount;

        if ((bot->GetTeam() == ALLIANCE && needAlly) || (bot->GetTeam() == HORDE && needHorde))
        {
            if (BgCount > BracketSize)
                sLog.outBasic("%s %u, bracket %u needs more bots. Total in queue: %u, BraketSize: %u. Need %u bots more", isArena ? "Arena" : "BG", queueTypeId, bracketId, BgCount, BracketSize, needMoreBots);

            return true;
        }
    }

    // do not join if BG queue is full
    if (BgCount >= BracketSize && (ACount >= TeamSize) && (HCount >= TeamSize))
    {
        return false;
    }

    if (!isArena && ((ACount >= TeamSize && TeamId == 0) || (HCount >= TeamSize && TeamId == 1)))
    {
        return false;
    }

    if (isArena && (((ACount >= TeamSize && HCount > 0) && TeamId == 0) || ((HCount >= TeamSize && ACount > 0) && TeamId == 1)))
    {
        return false;
    }

    if (isArena && (((ACount > TeamSize && HCount == 0) && TeamId == 1) || ((HCount > TeamSize && ACount == 0) && TeamId == 0)))
    {
        return false;
    }

    if (isArena && ((!isRated && SCount >= BracketSize) || (isRated && RCount >= BracketSize)))
    {
        return false;
    }

    return true;
}

bool BGJoinAction::isUseful()
{
    // do not try if BG bots disabled
    if (!sPlayerbotAIConfig.randomBotJoinBG)
        return false;

    // can't queue in BG
    if (bot->InBattleGround())
        return false;

    // do not try right after login
    if ((time(0) - bot->GetInGameTime()) < 30)
        return false;

    // check level
    if (bot->GetLevel() < 10)
        return false;

#ifdef MANGOSBOT_TWO
    if (bot->getClass() == CLASS_DEATH_KNIGHT && bot->GetLevel() < 60)
        return false;
#endif

    // do not try if with player master or in combat/group
    if (bot->GetPlayerbotAI()->HasActivePlayerMaster())
        return false;

    //if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetObjectGuid()))
    //    return false;

    if (bot->IsInCombat())
        return false;

    // check Deserter debuff
    if (!bot->CanJoinToBattleground())
        return false;

    // check if has free queue slots
    if (!bot->HasFreeBattleGroundQueueId())
        return false;

    // reduce amount of healers in BG
    if ((ai->IsTank(bot, false) || ai->IsHeal(bot, false)) && urand(0, 100) > 20)
        return false;

    // do not try if in dungeon
    //Map* map = bot->GetMap();
    //if (map && map->Instanceable())
    //    return false;

    bgList.clear();
    ratedList.clear();

    for (int i = BG_BRACKET_ID_FIRST; i < MAX_BATTLEGROUND_BRACKETS; ++i)
    {
        for (int j = BATTLEGROUND_QUEUE_AV; j < MAX_BATTLEGROUND_QUEUE_TYPES; ++j)
        {
            BattleGroundQueueTypeId queueTypeId = BattleGroundQueueTypeId(j);
            BattleGroundTypeId bgTypeId = sServerFacade.BgTemplateId(queueTypeId);
            BattleGroundBracketId bracketId = BattleGroundBracketId(i);

            Group* group = bot->GetGroup();
            if (group)
            {
                bool partyMemberCanNotJoinBG = false;

                for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                {
                    Player* member = ref->getSource();

                    if (!canJoinBg(member, queueTypeId, bracketId))
                    {
                        partyMemberCanNotJoinBG = true;
                        break;
                    }
                }

                if (partyMemberCanNotJoinBG)
                    continue;
            }

            if (!canJoinBg(bot, queueTypeId, bracketId))
                continue;

            if (shouldJoinBg(queueTypeId, bracketId))
                bgList.push_back(queueTypeId);
        }
    }

    if (!bgList.empty())
        return true;

    return false;
}

bool BGJoinAction::JoinQueue(uint32 type)
{
    // ignore if player is already in BG
    if (bot->InBattleGround())
        return false;

    // get BG TypeId
    BattleGroundQueueTypeId queueTypeId = BattleGroundQueueTypeId(type);
    BattleGroundTypeId bgTypeId = sServerFacade.BgTemplateId(queueTypeId);
    BattleGroundBracketId bracketId;

    BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
    if (!bg)
        return false;

#ifdef MANGOSBOT_TWO
    uint32 mapId = bg->GetMapId();
    PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, bot->GetLevel());
    if (!pvpDiff)
        return false;

    bracketId = pvpDiff->GetBracketId();
#endif
#ifndef MANGOSBOT_TWO
    bracketId = sBattleGroundMgr.GetBattleGroundBracketIdFromLevel(bgTypeId, bot->GetLevel());
#endif

    uint32 BracketSize = bg->GetMaxPlayers();
    uint32 TeamSize = bg->GetMaxPlayersPerTeam();
    uint32 TeamId = GetTeamIndexByTeamId(bot->GetTeam());

    // check if already in queue
    if (bot->InBattleGroundQueueForBattleGroundQueueType(queueTypeId))
        return false;

    // check bg req level
    if (!bot->GetBGAccessByLevel(bgTypeId))
        return false;

    // get BattleMaster unit
    // Find BattleMaster by Entry
    /*uint32 BmEntry = sRandomPlayerbotMgr.GetBattleMasterEntry(bot, bgTypeId, true);
    if (!BmEntry)
    {
        sLog.outError("Bot #%d <%s> could not find Battlemaster for %d", bot->GetGUIDLow(), bot->GetName(), bgTypeId);
        return false;
    }
    // check bm map
    CreatureDataPair const* dataPair = sRandomPlayerbotMgr.GetCreatureDataByEntry(BmEntry);
    CreatureData const* data = &dataPair->second;
    ObjectGuid BmGuid = ObjectGuid(HIGHGUID_UNIT, BmEntry, dataPair->first);
    if (data->mapid != bot->GetMapId())
    {
        sLog.outError("Bot #%d <%s> : Battlemaster is not in map for BG %d", bot->GetGUIDLow(), bot->GetName(), bgTypeId);
        return false;
    }*/

   // get BG MapId
#ifdef MANGOSBOT_ZERO
   uint32 mapId = GetBattleGrounMapIdByTypeId(bgTypeId);
#else
   uint32 bgTypeId_ = bgTypeId;
#endif
   uint32 instanceId = 0; // 0 = First Available
   uint8 joinAsGroup = bot->GetGroup() && bot->GetGroup()->IsLeader(bot->GetObjectGuid());
   bool isPremade = false;
   bool isArena = false;
   bool isRated = false;
   uint8 arenaslot = 0;
   uint8 asGroup = false;
   std::string _bgType;

// check if arena
#ifndef MANGOSBOT_ZERO
   ArenaType arenaType = sServerFacade.BgArenaType(queueTypeId);
   if (arenaType != ARENA_TYPE_NONE)
       isArena = true;
#endif

   // get battlemaster
   Unit* unit = ai->GetUnit(AI_VALUE2(CreatureDataPair const*, "bg master", bgTypeId));
#ifndef MANGOSBOT_TWO
   if (!unit)
#else
   if (!unit && isArena)
#endif
   {
       sLog.outError("Bot %d could not find Battlemaster to join", bot->GetGUIDLow());
       return false;
   }
// in wotlk only arena requires battlemaster guid
#ifndef MANGOSBOT_TWO
   ObjectGuid guid = unit->GetObjectGuid();
#else
   ObjectGuid guid = isArena ? unit->GetObjectGuid() : bot->GetObjectGuid();
#endif

   switch (bgTypeId)
   {
   case BATTLEGROUND_AV:
       _bgType = "AV";
       break;
   case BATTLEGROUND_WS:
       _bgType = "WSG";
       break;
   case BATTLEGROUND_AB:
       _bgType = "AB";
       break;
#ifndef MANGOSBOT_ZERO
   case BATTLEGROUND_EY:
       _bgType = "EotS";
       break;
#endif
#ifdef MANGOSBOT_TWO
   case BATTLEGROUND_RB:
       _bgType = "Random";
       break;
   case BATTLEGROUND_SA:
       _bgType = "SotA";
       break;
   case BATTLEGROUND_IC:
       _bgType = "IoC";
       break;
#endif
   default:
       break;
}

#ifndef MANGOSBOT_ZERO
   if (isArena)
   {
       isArena = true;
       BracketSize = type * 2;
       TeamSize = type;
       isRated = ai->GetAiObjectContext()->GetValue<uint32>("arena type")->Get();

       if (joinAsGroup)
           asGroup = true;

       switch (arenaType)
       {
       case ARENA_TYPE_2v2:
           arenaslot = 0;
           _bgType = "2v2";
           break;
       case ARENA_TYPE_3v3:
           arenaslot = 1;
           _bgType = "3v3";
           break;
       case ARENA_TYPE_5v5:
           arenaslot = 2;
           _bgType = "5v5";
           break;
       default:
           break;
       }
   }
#endif

   // refresh food/regs
   sRandomPlayerbotMgr.Refresh(bot);

   if (isArena)
   {
       if (isRated)
       {
           sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][isRated] += TeamSize;
           sRandomPlayerbotMgr.ArenaBots[queueTypeId][bracketId][isRated][TeamId] += TeamSize;
       }
       else
       {
           sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][isRated]++;
           sRandomPlayerbotMgr.ArenaBots[queueTypeId][bracketId][isRated][TeamId]++;
       }
   }
   else if (!joinAsGroup)
       sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][TeamId]++;
   else
       sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][TeamId] += bot->GetGroup()->GetMembersCount();

   ai->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(0);

   WorldPacket packet(CMSG_BATTLEMASTER_JOIN, 20);
#ifdef MANGOSBOT_ZERO
   packet << unit->GetObjectGuid() << mapId << instanceId << joinAsGroup;
   sLog.outBasic("Bot #%d %s:%d <%s> queued %s", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), _bgType.c_str());
#else
   sLog.outBasic("Bot #%d %s:%d <%s> queued %s %s", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), _bgType.c_str(), isRated ? "Rated Arena" : isArena ? "Arena" : "");
   if (!isArena)
   {
       packet << guid << bgTypeId_ << instanceId << joinAsGroup;
   }
   else
   {
       WorldPacket arena_packet(CMSG_BATTLEMASTER_JOIN_ARENA, 20);
       arena_packet << guid << arenaslot << asGroup << uint8(isRated);
       bot->GetSession()->HandleBattlemasterJoinArena(arena_packet);
       return true;
   }
#endif

   ai->QueuePacket(packet);
   return true;
}

bool FreeBGJoinAction::shouldJoinBg(BattleGroundQueueTypeId queueTypeId, BattleGroundBracketId bracketId)
{
    if (!sPlayerbotAIConfig.randomBotAutoJoinBG)
        return false;

    BattleGroundTypeId bgTypeId = sServerFacade.BgTemplateId(queueTypeId);
    BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
    if (!bg)
        return false;

    bool isArena = false;
    bool isRated = false;
    bool hasTeam = false;
    bool noLag = sWorld.GetAverageDiff() < (sRandomPlayerbotMgr.GetPlayers().empty() ? sPlayerbotAIConfig.diffEmpty : sPlayerbotAIConfig.diffWithPlayer) * 1.5;

    if (sPlayerbotAIConfig.disableActivityPriorities)
    {
        noLag = true;
    }

#ifndef MANGOSBOT_ZERO
    ArenaType type = sServerFacade.BgArenaType(queueTypeId);
    if (type != ARENA_TYPE_NONE)
        isArena = true;

    for (uint8 i = 0; i < MAX_ARENA_SLOT; ++i)
    {
        if (uint32 a_id = bot->GetArenaTeamId(i))
        {
            hasTeam = true;
            break;
        }
    }
#endif

    bool hasPlayers = (sRandomPlayerbotMgr.BgPlayers[queueTypeId][bracketId][0] + sRandomPlayerbotMgr.BgPlayers[queueTypeId][bracketId][1]) > 0;
    bool hasBots = (sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][0] + sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][1]) >= bg->GetMinPlayersPerTeam();

    if (!hasPlayers && !noLag && !(isArena && hasTeam))
        return false;

#ifndef MANGOSBOT_ZERO
    if (!hasPlayers && isArena && !hasTeam)
        return false;

    if (!hasPlayers && !isArena && hasTeam)
        return false;
#endif

#ifdef MANGOSBOT_TWO
    if (!hasPlayers && bgTypeId == BATTLEGROUND_RB)
        return false;

    if (!hasPlayers && bgTypeId == BATTLEGROUND_SA)
        return false;
#endif

    uint32 BracketSize = bg->GetMaxPlayers();
    uint32 TeamSize = bg->GetMaxPlayersPerTeam();

    uint32 ACount = sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][0] + sRandomPlayerbotMgr.BgPlayers[queueTypeId][bracketId][0];
    uint32 HCount = sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][1] + sRandomPlayerbotMgr.BgPlayers[queueTypeId][bracketId][1];

    uint32 BgCount = ACount + HCount;
    uint32 SCount = 0;
    uint32 RCount = 0;

    uint32 TeamId = bot->GetTeam() == ALLIANCE ? 0 : 1;

#ifndef MANGOSBOT_ZERO
    if (isArena)
    {
        uint32 rated_players = sRandomPlayerbotMgr.BgPlayers[queueTypeId][bracketId][1];
        if (rated_players)
        {
            isRated = true;
        }
        isArena = true;
        BracketSize = (uint32)(type * 2);
        TeamSize = type;
        ACount = sRandomPlayerbotMgr.ArenaBots[queueTypeId][bracketId][isRated][0];
        HCount = sRandomPlayerbotMgr.ArenaBots[queueTypeId][bracketId][isRated][1];
        BgCount = sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][isRated] + sRandomPlayerbotMgr.BgPlayers[queueTypeId][bracketId][isRated];
        SCount = sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][0] + sRandomPlayerbotMgr.BgPlayers[queueTypeId][bracketId][0];
        RCount = sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][1] + sRandomPlayerbotMgr.BgPlayers[queueTypeId][bracketId][1];
    }

    // do not try if not a captain of arena team

    if (isRated)
    {
        if (!sObjectMgr.GetArenaTeamByCaptain(bot->GetObjectGuid()))
            return false;

        // check if bot has correct team
        ArenaTeam* arenateam = nullptr;
        for (uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
        {
            ArenaTeam* temp = sObjectMgr.GetArenaTeamById(bot->GetArenaTeamId(arena_slot));
            if (!temp)
                continue;

            if (temp->GetType() != type)
                continue;

            arenateam = temp;
        }

        if (!arenateam)
            return false;

        ratedList.push_back(queueTypeId);
    }
#endif

    // hack fix crash in queue remove event
    //if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetObjectGuid()))
    //    return false;

    bool needBots = sRandomPlayerbotMgr.NeedBots[queueTypeId][bracketId][isArena ? isRated : GetTeamIndexByTeamId(bot->GetTeam())];

    // add more bots if players are not invited or 1st BG instance is full
    if (needBots/* || (hasPlayers && BgCount > BracketSize && (BgCount % BracketSize) != 0)*/)
        return true;

    // do not join if BG queue is full
    if (BgCount >= BracketSize && (ACount >= TeamSize) && (HCount >= TeamSize))
    {
        return false;
    }

    if (!isArena && ((ACount >= TeamSize && TeamId == 0) || (HCount >= TeamSize && TeamId == 1)))
    {
        return false;
    }

    if (isArena && (((ACount >= TeamSize && HCount > 0) && TeamId == 0) || ((HCount >= TeamSize && ACount > 0) && TeamId == 1)))
    {
        return false;
    }

    if (isArena && (((ACount > TeamSize && HCount == 0) && TeamId == 1) || ((HCount > TeamSize && ACount == 0) && TeamId == 0)))
    {
        return false;
    }

    if (isArena && ((!isRated && SCount >= BracketSize) || (isRated && RCount >= BracketSize)))
    {
        return false;
    }

    return true;
}


bool BGLeaveAction::Execute(Event& event)
{
    if (!(bot->InBattleGroundQueue() || bot->InBattleGround()))
        return false;

    BattleGroundQueueTypeId queueTypeId = bot->GetBattleGroundQueueTypeId(0);
    BattleGroundTypeId _bgTypeId = sServerFacade.BgTemplateId(queueTypeId);
    uint8 type = false;
    uint16 unk = 0x1F90;
    uint8 unk2 = 0x0;
    bool isArena = false;
    bool IsRandomBot = sRandomPlayerbotMgr.IsRandomBot(bot);

#ifndef MANGOSBOT_ZERO
    ArenaType arenaType = sServerFacade.BgArenaType(queueTypeId);
    if (arenaType != ARENA_TYPE_NONE)
    {
        isArena = true;
        type = arenaType;
    }

    if (bot->InBattleGround())
    {
        sLog.outDetail("Bot #%d %s:%d <%s> leaves %s", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), isArena ? "Arena" : "BG");

        WorldPacket leave(CMSG_LEAVE_BATTLEFIELD);
        leave << uint8(0) << uint8(0) << uint32(0) << uint16(0);
        bot->GetSession()->HandleLeaveBattlefieldOpcode(leave);

        if (sRandomPlayerbotMgr.IsFreeBot(bot))
            ai->SetMaster(NULL);

        ai->ResetStrategies();
        ai->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(0);
        ai->GetAiObjectContext()->GetValue<uint32>("bg role")->Set(0);
        ai->GetAiObjectContext()->GetValue<uint32>("arena type")->Set(0);
        ai::PositionMap& posMap = context->GetValue<ai::PositionMap&>("position")->Get();
        ai::PositionEntry pos = context->GetValue<ai::PositionMap&>("position")->Get()["bg objective"];
        pos.Reset();
        posMap["bg objective"] = pos;

        return true;
    }
#endif

    uint32 queueType = AI_VALUE(uint32, "bg type");
    if (!queueType && event.getSource().empty())
        return false;

    sLog.outDetail("Bot #%d %s:%d <%s> leaves %s queue", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), isArena ? "Arena" : "BG");

    WorldPacket packet(CMSG_BATTLEFIELD_PORT, 20);
#ifdef MANGOSBOT_ZERO
    uint32 mapId = GetBattleGrounMapIdByTypeId(_bgTypeId);
    packet << mapId << uint8(0);
#else
    packet << type << unk2 << (uint32)_bgTypeId << unk << uint8(0);
#endif

    if (!event.getSource().empty())
    {
        bot->GetSession()->HandleLeaveBattlefieldOpcode(packet);
    }
    else
        bot->GetSession()->HandleBattlefieldPortOpcode(packet);

    if (sRandomPlayerbotMgr.IsFreeBot(bot))
        ai->SetMaster(NULL);

    ai->ResetStrategies();
    ai->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(0);
    ai->GetAiObjectContext()->GetValue<uint32>("bg role")->Set(0);
    ai->GetAiObjectContext()->GetValue<uint32>("arena type")->Set(0);
    ai::PositionMap& posMap = context->GetValue<ai::PositionMap&>("position")->Get();
    ai::PositionEntry pos = context->GetValue<ai::PositionMap&>("position")->Get()["bg objective"];
    pos.Reset();
    posMap["bg objective"] = pos;
    return true;
}

bool BGStatusAction::isUseful()
{
    return bot->InBattleGroundQueue();
}

bool BGStatusAction::Execute(Event& event)
{
    uint32 QueueSlot;
    uint32 instanceId;
    uint32 mapId;
    uint32 statusid;
    uint32 Time1;
    uint32 Time2;
    std::string _bgType;
    uint8 isRated = 0;

#ifndef MANGOSBOT_ZERO
    //uint64 arenatype;
    uint64 arenaByte;
    uint8 arenaTeam;
    //uint64 x1f90;
    //uint64 bgTypeId;
    //uint32 battleId;
#else
    uint8 unk1;
#endif

#ifdef MANGOSBOT_TWO
    uint64 unk0;
    uint8 minlevel;
    uint8 maxlevel;
#endif

    WorldPacket p(event.getPacket());
    statusid = 0;
#ifndef MANGOSBOT_ZERO
    p >> QueueSlot; // queue id (0...2) - player can be in 3 queues in time
    p >> arenaByte;
    if (arenaByte == 0)
        return false;
#ifdef MANGOSBOT_TWO
    p >> minlevel;
    p >> maxlevel;
#endif
    p >> instanceId;
    p >> isRated;
    p >> statusid;

    // check status
    switch (statusid)
    {
    case STATUS_WAIT_QUEUE:                  // status_in_queue
        p >> Time1;                         // average wait time, milliseconds
        p >> Time2;                        // time in queue, updated every minute!, milliseconds
        break;
    case STATUS_WAIT_JOIN:                   // status_invite
        p >> mapId;                         // map id
#ifdef MANGOSBOT_TWO
        p >> unk0;
#endif
        p >> Time1;                            // time to remove from queue, milliseconds
        break;
    case STATUS_IN_PROGRESS:                  // status_in_progress
        p >> mapId;                          // map id
#ifdef MANGOSBOT_TWO
        p >> unk0;
#endif
        p >> Time1;                         // time to bg auto leave, 0 at bg start, 120000 after bg end, milliseconds
        p >> Time2;                        // time from bg start, milliseconds
        p >> arenaTeam;
        break;
    default:
        sLog.outError("Unknown BG status!");
        break;
    }
#else
    p >> QueueSlot; // queue id (0...2) - player can be in 3 queues in time
    p >> mapId;     // MapID
    if (mapId == 0)
        return false;
    p >> unk1;      // Unknown
    p >> instanceId;
    p >> statusid;  // status

    // check status
    switch (statusid)
    {
    case STATUS_WAIT_QUEUE:                  // status_in_queue
        p >> Time1;                         // average wait time, milliseconds
        p >> Time2;                        // time in queue, updated every minute!, milliseconds
        break;
    case STATUS_WAIT_JOIN:                   // status_invite
        p >> Time1;                         // time to remove from queue, milliseconds
        break;
    case STATUS_IN_PROGRESS:                 // status_in_progress
        p >> Time1;                         // time to bg auto leave, 0 at bg start, 120000 after bg end, milliseconds
        p >> Time2;                        // time from bg start, milliseconds
        break;
    default:
        sLog.outError("Unknown BG status!");
        break;
    }
#endif

    bool IsRandomBot = sRandomPlayerbotMgr.IsRandomBot(bot);
    BattleGroundQueueTypeId queueTypeId = bot->GetBattleGroundQueueTypeId(QueueSlot);
    BattleGroundTypeId _bgTypeId = sServerFacade.BgTemplateId(queueTypeId);
    BattleGroundBracketId bracketId;
#ifndef MANGOSBOT_ZERO    
    BattleGroundTypeId bgTypeId = BattleGroundTypeId((arenaByte >> 16) & 0xFFFFFFFF);    
    _bgTypeId = bgTypeId;
#endif

    if (!queueTypeId)
        return false;

#ifndef MANGOSBOT_TWO
    BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(_bgTypeId);
    if (!bg)
        return false;
    bracketId = sBattleGroundMgr.GetBattleGroundBracketIdFromLevel(_bgTypeId, bot->GetLevel());
#endif
#ifdef MANGOSBOT_TWO
    BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(_bgTypeId);
    if (!bg)
        return false;

    mapId = bg->GetMapId();
    PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, bot->GetLevel());
    if (!pvpDiff)
        return false;

    bracketId = pvpDiff->GetBracketId();
#endif

    bool isArena = false;
    uint8 type = false;
    uint16 unk = 0x1F90;
    uint8 unk2 = 0x0;
    uint8 action = 0x1;

#ifndef MANGOSBOT_ZERO
    ArenaType arenaType = sServerFacade.BgArenaType(queueTypeId);
    if (arenaType != ARENA_TYPE_NONE)
    {
        isArena = true;
        type = arenaType;
    }
#endif

    switch (_bgTypeId)
    {
    case BATTLEGROUND_AV:
        _bgType = "AV";
        break;
    case BATTLEGROUND_WS:
        _bgType = "WSG";
        break;
    case BATTLEGROUND_AB:
        _bgType = "AB";
        break;
#ifndef MANGOSBOT_ZERO
    case BATTLEGROUND_EY:
        _bgType = "EotS";
        break;
#endif
#ifdef MANGOSBOT_TWO
    case BATTLEGROUND_RB:
        _bgType = "Random";
        break;
    case BATTLEGROUND_SA:
        _bgType = "SotA";
        break;
    case BATTLEGROUND_IC:
        _bgType = "IoC";
        break;
#endif
    default:
        break;
    }

#ifndef MANGOSBOT_ZERO
    switch (arenaType)
    {
    case ARENA_TYPE_2v2:
        _bgType = "2v2";
        break;
    case ARENA_TYPE_3v3:
        _bgType = "3v3";
        break;
    case ARENA_TYPE_5v5:
        _bgType = "5v5";
        break;
    default:
        break;
    }
#endif

    if (Time1 == TIME_TO_AUTOREMOVE) //battleground is over, bot needs to leave
    {
        sLog.outDetail("Bot #%u <%s> (%u %s): Received BG status TIME_REMOVE for %s %s", bot->GetGUIDLow(), bot->GetName(), bot->GetLevel(), bot->GetTeam() == ALLIANCE ? "A" : "H", isArena ? "Arena" : "BG", _bgType.c_str());
        BattleGround* bg = bot->GetBattleGround();
        if (bg)
        {
            uint32 TeamId = bot->GetTeam() == ALLIANCE ? 0 : 1;
#ifndef MANGOSBOT_ZERO
            if (isArena)
            {
                sRandomPlayerbotMgr.ArenaBots[queueTypeId][bracketId][isRated][TeamId]--;
                TeamId = isRated ? 1 : 0;
            }
#endif
            sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][TeamId]--;
            sRandomPlayerbotMgr.BgPlayers[queueTypeId][bracketId][TeamId] = 0;
        }

        // remove warsong strategy
        if (sRandomPlayerbotMgr.IsFreeBot(bot))
            ai->SetMaster(NULL);

        ai->ChangeStrategy("-warsong", BotState::BOT_STATE_COMBAT);
        ai->ChangeStrategy("-warsong", BotState::BOT_STATE_NON_COMBAT);
        ai->ChangeStrategy("-arathi", BotState::BOT_STATE_COMBAT);
        ai->ChangeStrategy("-arathi", BotState::BOT_STATE_NON_COMBAT);
        ai->ChangeStrategy("-eye", BotState::BOT_STATE_COMBAT);
        ai->ChangeStrategy("-eye", BotState::BOT_STATE_NON_COMBAT);
        ai->ChangeStrategy("-isle", BotState::BOT_STATE_COMBAT);
        ai->ChangeStrategy("-isle", BotState::BOT_STATE_NON_COMBAT);
        ai->ChangeStrategy("-battleground", BotState::BOT_STATE_COMBAT);
        ai->ChangeStrategy("-battleground", BotState::BOT_STATE_NON_COMBAT);
        ai->ChangeStrategy("-arena", BotState::BOT_STATE_COMBAT);
        ai->ChangeStrategy("-arena", BotState::BOT_STATE_NON_COMBAT);
        sLog.outBasic("Bot #%d %s:%d <%s> leaves %s - %s", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), isArena ? "Arena" : "BG", _bgType.c_str());

        WorldPacket packet(CMSG_LEAVE_BATTLEFIELD);
        packet << uint8(0);
        packet << uint8(0);                           // BattleGroundTypeId-1 ?
#ifdef MANGOSBOT_ZERO
        packet << uint16(0);
#else
        packet << uint32(0);
        packet << uint16(0);
#endif
        bot->GetSession()->HandleLeaveBattlefieldOpcode(packet);
        ai->ResetStrategies();
        ai->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(0);
        ai->GetAiObjectContext()->GetValue<uint32>("bg role")->Set(0);
        ai->GetAiObjectContext()->GetValue<uint32>("arena type")->Set(0);
        ai::PositionMap& posMap = context->GetValue<ai::PositionMap&>("position")->Get();
        ai::PositionEntry pos = context->GetValue<ai::PositionMap&>("position")->Get()["bg objective"];
        pos.Reset();
        posMap["bg objective"] = pos;
    }
    if (statusid == STATUS_WAIT_QUEUE) //bot is in queue
    {
        sLog.outDetail("Bot #%u %s:%d <%s>: Received BG status WAIT_QUEUE (wait time: %u) for %s %s", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), Time2, isArena ? "Arena" : "BG", _bgType.c_str());
        // temp fix for crash
         //return true;

/*
// TO DO: Fix for wotlk
#ifdef MANGOSBOT_TWO
        BattleGroundQueue& bgQueue = sServerFacade.bgQueue(queueTypeId);
        GroupQueueInfo ginfo;
        if (bgQueue.GetPlayerGroupInfoData(bot->GetObjectGuid(), &ginfo))
        {
            if (ginfo.isInvitedToBgInstanceGuid && !bot->InBattleGround())
            {
                BattleGround* bg = sBattleGroundMgr.GetBattleGround(ginfo.isInvitedToBgInstanceGuid, BATTLEGROUND_TYPE_NONE);
                if (bg)
                {
#ifndef MANGOSBOT_ZERO
                    if (isArena)
                    {
                        _bgTypeId = bg->GetTypeId();
                    }
#endif
                    sLog.outBasic("Bot #%u %s:%d <%s>: Force join %s %s", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), isArena ? "Arena" : "BG", _bgType.c_str());
                    
                    ai->Unmount();

                    action = 0x1;
                    // bg started so players should get invites by now
                    sRandomPlayerbotMgr.NeedBots[queueTypeId][bracketId][isArena ? isRated : GetTeamIndexByTeamId(bot->GetTeam())] = false;

                    WorldPacket packet(CMSG_BATTLEFIELD_PORT, 20);
#ifdef MANGOSBOT_ZERO
                    packet << mapId << action;
#else
                    packet << type << unk2 << (uint32)_bgTypeId << unk << action;
#endif
                    bot->GetSession()->HandleBattlefieldPortOpcode(packet);

                    ai->ResetStrategies(false);
                    context->GetValue<uint32>("bg role")->Set(urand(0, 9));
                    ai::PositionMap& posMap = context->GetValue<ai::PositionMap&>("position")->Get();
                    ai::PositionEntry pos = context->GetValue<ai::PositionMap&>("position")->Get()["bg objective"];
                    pos.Reset();
                    posMap["bg objective"] = pos;

                    return true;
                }
            }
        }

        BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(_bgTypeId);
        if (!bg)
            return false;

        bool leaveQ = false;
        uint32 timer;
        if (isArena)
            timer = TIME_TO_AUTOREMOVE;
        else
            timer = TIME_TO_AUTOREMOVE + 1000 * (bg->GetMaxPlayers() * 4);

        if (Time2 > timer && isArena) // disabled for BG
            leaveQ = true;

        if (leaveQ && ((bot->GetGroup() && bot->GetGroup()->IsLeader(bot->GetObjectGuid())) || !(bot->GetGroup() || ai->GetMaster())))
        {
            uint32 TeamId = bot->GetTeam() == ALLIANCE ? 0 : 1;
            bool realPlayers = sRandomPlayerbotMgr.BgPlayers[queueTypeId][bracketId][TeamId];
            if (realPlayers)
                return false;
            sLog.outBasic("Bot #%u %s:%d <%s> waited too long and leaves queue (%s %s).", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), isArena ? "Arena" : "BG", _bgType.c_str());
            WorldPacket packet(CMSG_BATTLEFIELD_PORT, 20);
            action = 0;
#ifdef MANGOSBOT_ZERO
            packet << mapId << action;
#else
            packet << type << unk2 << (uint32)_bgTypeId << unk << action;
#endif
#ifdef MANGOS
            bot->GetSession()->HandleBattleFieldPortOpcode(packet);
#endif
#ifdef CMANGOS
            bot->GetSession()->HandleBattlefieldPortOpcode(packet);
#endif
            ai->ResetStrategies();
            ai->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(0);
            ai->GetAiObjectContext()->GetValue<uint32>("bg role")->Set(0);
            ai->GetAiObjectContext()->GetValue<uint32>("arena type")->Set(0);
            sRandomPlayerbotMgr.BgBots[queueTypeId][bracketId][TeamId]--;
            return true;
        }
#else
*/
        return true;
//#endif
    }
    if (statusid == STATUS_IN_PROGRESS) // placeholder for Leave BG if it takes too long
    {
        sLog.outDetail("Bot #%u %s:%d <%s>: Received BG status IN_PROGRESS for %s %s", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), isArena ? "Arena" : "BG", _bgType.c_str());
        return false;
    }
    if (statusid == STATUS_WAIT_JOIN) //bot may join
    {
        sLog.outDetail("Bot #%u %s:%d <%s>: Received BG status WAIT_JOIN for %s %s", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), isArena ? "Arena" : "BG", _bgType.c_str());
#ifndef MANGOSBOT_ZERO
        if (isArena)
        {
#ifdef MANGOSBOT_TWOx
            isArena = true;
            BattleGroundQueue& bgQueue = sServerFacade.bgQueue(queueTypeId);
            GroupQueueInfo ginfo;
            if (!bgQueue.GetPlayerGroupInfoData(bot->GetObjectGuid(), &ginfo))
            {
                sLog.outError("Bot #%u %s:%d <%s>: Missing QueueInfo for %s %s", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), isArena ? "Arena" : "BG", _bgType.c_str());
                return false;
            }

            if (ginfo.isInvitedToBgInstanceGuid)
            {
                BattleGround* bg = sBattleGroundMgr.GetBattleGround(ginfo.isInvitedToBgInstanceGuid, BATTLEGROUND_TYPE_NONE);
                if (!bg)
                {
                    sLog.outError("Bot #%u %s:%d <%s>: Missing QueueInfo for %s %s", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), isArena ? "Arena" : "BG", _bgType.c_str());
                    return false;
                }

                _bgTypeId = bg->GetTypeId();
            }
#endif
        }
#endif

        ai->Unmount();

        action = 0x1;
        // bg started so players should get invites by now
        sRandomPlayerbotMgr.NeedBots[queueTypeId][bracketId][isArena ? isRated : GetTeamIndexByTeamId(bot->GetTeam())] = false;

        WorldPacket packet(CMSG_BATTLEFIELD_PORT, 20);
#ifdef MANGOSBOT_ZERO
        packet << mapId << action;
#else
        packet << type << unk2 << (uint32)_bgTypeId << unk << action;
#endif

        if (bot->InBattleGround() && bot->GetBattleGroundTypeId() != _bgTypeId)
        {
            //We are already in a BG. Joining a new one directly causes a crash when SetBattleGroundId is reset during leave right before teleporting in the new one.
            //Here we leave the current BG proper so we can join the next fresh.
            WorldPacket leave(CMSG_LEAVE_BATTLEFIELD);
            leave << uint8(0) << uint8(0) << uint32(0) << uint16(0);
            bot->GetSession()->HandleLeaveBattlefieldOpcode(leave);
            //Queue the event again to try to join next tick.
            //ai->HandleBotOutgoingPacket(event.getPacket());
            return true;
        }

#ifdef MANGOSBOT_ZERO
        sLog.outBasic("Bot #%d <%s> (%u %s) joined BG (%s)", bot->GetGUIDLow(), bot->GetName(), bot->GetLevel(), bot->GetTeam() == ALLIANCE ? "A" : "H", _bgType.c_str());
#else
        sLog.outBasic("Bot #%d %s:%d <%s> joined %s - %s", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), isArena ? "Arena" : "BG", _bgType.c_str());
#endif

        bot->GetSession()->HandleBattlefieldPortOpcode(packet);

        ai->ResetStrategies(false);
        context->GetValue<uint32>("bg role")->Set(urand(0, 9));
        ai::PositionMap& posMap = context->GetValue<ai::PositionMap&>("position")->Get();
        ai::PositionEntry pos = context->GetValue<ai::PositionMap&>("position")->Get()["bg objective"];
        ai::PositionEntry pos2 = context->GetValue<ai::PositionMap&>("position")->Get()["bg siege"];
        pos.Reset();
        pos2.Reset();
        posMap["bg objective"] = pos;
        posMap["bg siege"] = pos2;

        return true;
    }
    return true;
}

bool BGStatusCheckAction::Execute(Event& event)
{
    if (bot->IsBeingTeleported())
        return false;

    WorldPacket packet(CMSG_BATTLEFIELD_STATUS);
    bot->GetSession()->HandleBattlefieldStatusOpcode(packet);
    sLog.outDetail("Bot #%d <%s> (%u %s) : Checking BG invite status", bot->GetGUIDLow(), bot->GetName(), bot->GetLevel(), bot->GetTeam() == ALLIANCE ? "A" : "H");
    return true;
}
bool BGStatusCheckAction::isUseful()
{
    return bot->InBattleGroundQueue();
}
