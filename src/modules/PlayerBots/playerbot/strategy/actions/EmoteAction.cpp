
#include "playerbot/playerbot.h"
#include "EmoteAction.h"
#include "playerbot/PlayerbotTextMgr.h"

#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
using namespace ai;

std::map<std::string, uint32> EmoteActionBase::emotes;
std::map<std::string, uint32> EmoteActionBase::textEmotes;
char *strstri(const char *haystack, const char *needle);

EmoteActionBase::EmoteActionBase(PlayerbotAI* ai, std::string name) : Action(ai, name)
{
    if (emotes.empty()) InitEmotes();
}

EmoteAction::EmoteAction(PlayerbotAI* ai) : EmoteActionBase(ai, "emote"), Qualified()
{
}

void EmoteActionBase::InitEmotes()
{
    emotes["dance"] = EMOTE_ONESHOT_DANCE;
    emotes["drown"] = EMOTE_ONESHOT_DROWN;
    emotes["land"] = EMOTE_ONESHOT_LAND;
    emotes["liftoff"] = EMOTE_ONESHOT_LIFTOFF;
    emotes["loot"] = EMOTE_ONESHOT_LOOT;
    emotes["no"] = EMOTE_ONESHOT_NO;
    emotes["roar"] = EMOTE_STATE_ROAR;
    emotes["salute"] = EMOTE_ONESHOT_SALUTE;
    emotes["stomp"] = EMOTE_ONESHOT_STOMP;
    emotes["train"] = EMOTE_ONESHOT_TRAIN;
    emotes["yes"] = EMOTE_ONESHOT_YES;
    emotes["applaud"] = EMOTE_ONESHOT_APPLAUD;
    emotes["beg"] = EMOTE_ONESHOT_BEG;
    emotes["bow"] = EMOTE_ONESHOT_BOW;
    emotes["cheer"] = EMOTE_ONESHOT_CHEER;
    emotes["chicken"] = EMOTE_ONESHOT_CHICKEN;
    emotes["cry"] = EMOTE_ONESHOT_CRY;
    emotes["dance"] = EMOTE_STATE_DANCE;
    emotes["eat"] = EMOTE_ONESHOT_EAT;
    emotes["exclamation"] = EMOTE_ONESHOT_EXCLAMATION;
    emotes["flex"] = EMOTE_ONESHOT_FLEX;
    emotes["kick"] = EMOTE_ONESHOT_KICK;
    emotes["kiss"] = EMOTE_ONESHOT_KISS;
    emotes["kneel"] = EMOTE_ONESHOT_KNEEL;
    emotes["laugh"] = EMOTE_ONESHOT_LAUGH;
    emotes["point"] = EMOTE_ONESHOT_POINT;
    emotes["question"] = EMOTE_ONESHOT_QUESTION;
    emotes["ready1h"] = EMOTE_ONESHOT_READY1H;
    emotes["roar"] = EMOTE_ONESHOT_ROAR;
    emotes["rude"] = EMOTE_ONESHOT_RUDE;
    emotes["shout"] = EMOTE_ONESHOT_SHOUT;
    emotes["shy"] = EMOTE_ONESHOT_SHY;
    emotes["sleep"] = EMOTE_STATE_SLEEP;
    emotes["talk"] = EMOTE_ONESHOT_TALK;
    emotes["wave"] = EMOTE_ONESHOT_WAVE;
    emotes["wound"] = EMOTE_ONESHOT_WOUND;

    textEmotes["bored"] = TEXTEMOTE_BORED;
    textEmotes["bye"] = TEXTEMOTE_BYE;
    textEmotes["cheer"] = TEXTEMOTE_CHEER;
    textEmotes["congratulate"] = TEXTEMOTE_CONGRATULATE;
    textEmotes["hello"] = TEXTEMOTE_HELLO;
    textEmotes["no"] = TEXTEMOTE_NO;
    textEmotes["nod"] = TEXTEMOTE_NOD; // yes
    textEmotes["sigh"] = TEXTEMOTE_SIGH;
    textEmotes["thank"] = TEXTEMOTE_THANK;
    textEmotes["welcome"] = TEXTEMOTE_WELCOME; // you are welcome
    textEmotes["whistle"] = TEXTEMOTE_WHISTLE;
    textEmotes["yawn"] = TEXTEMOTE_YAWN;
    textEmotes["oom"] = 323;
    textEmotes["follow"] = 324;
    textEmotes["wait"] = 325;
    textEmotes["healme"] = 326;
    textEmotes["openfire"] = 327;
    textEmotes["helpme"] = 303;
    textEmotes["flee"] = 306;
    textEmotes["danger"] = 304;
    textEmotes["charge"] = 305;
    textEmotes["help"] = 307;
    textEmotes["train"] = 264;
    textEmotes["joke"] = TEXTEMOTE_JOKE;
    textEmotes["silly"] = TEXTEMOTE_JOKE;
    textEmotes["anecdote"] = TEXTEMOTE_JOKE;
    textEmotes["cry"] = TEXTEMOTE_CRY;
    textEmotes["jk"] = TEXTEMOTE_JK;
    textEmotes["introduce"] = TEXTEMOTE_INTRODUCE;
    textEmotes["hug"] = TEXTEMOTE_HUG;
    textEmotes["kneel"] = TEXTEMOTE_KNEEL;
    textEmotes["kiss"] = TEXTEMOTE_KISS;
    textEmotes["point"] = TEXTEMOTE_POINT;
    textEmotes["roar"] = TEXTEMOTE_ROAR;
    textEmotes["rude"] = TEXTEMOTE_RUDE;
    textEmotes["chicken"] = TEXTEMOTE_CHICKEN;
    textEmotes["flirt"] = TEXTEMOTE_FLIRT;
}

bool EmoteActionBase::Emote(Unit* target, uint32 type, bool textEmote)
{
    if (target && !sServerFacade.IsInFront(bot, target, sPlayerbotAIConfig.sightDistance, M_PI_F))
        sServerFacade.SetFacingTo(bot, target);

    ObjectGuid oldSelection = bot->GetSelectionGuid();
    if (target)
    {
        bot->SetSelectionGuid(target->GetObjectGuid());
        Player* player = dynamic_cast<Player*>(target);
        if (player && player->GetPlayerbotAI() && !sServerFacade.IsInFront(player, bot, sPlayerbotAIConfig.sightDistance, M_PI_F))
            sServerFacade.SetFacingTo(player, bot);
    }

    if (textEmote)
    {
        WorldPacket data(SMSG_TEXT_EMOTE);
        data << type;
        data << urand(0, GetNumberOfEmoteVariants((TextEmotes)type, bot->getRace(), bot->getGender()) - 1);
        data << ((bot->GetSelectionGuid() && urand(0, 1)) ? bot->GetSelectionGuid() : ObjectGuid());
        bot->GetSession()->HandleTextEmoteOpcode(data);
    }
    else
        bot->HandleEmoteCommand(type);

    if (oldSelection)
        bot->SetSelectionGuid(oldSelection);

    return true;
}

Unit* EmoteActionBase::GetTarget()
{
    Unit* target = NULL;

    std::list<ObjectGuid> nfp = *context->GetValue<std::list<ObjectGuid> >("nearest friendly players");
    std::vector<Unit*> targets;
    for (std::list<ObjectGuid>::iterator i = nfp.begin(); i != nfp.end(); ++i)
    {
        Unit* unit = ai->GetUnit(*i);
        if (unit && sServerFacade.GetDistance2d(bot, unit) < sPlayerbotAIConfig.tooCloseDistance) targets.push_back(unit);
    }

    if (!targets.empty())
        target = targets[urand(0, targets.size() - 1)];

    return target;
}

bool EmoteActionBase::ReceiveEmote(Player* requester, Player* source, uint32 emote, bool verbal)
{
    uint32 emoteId = 0;
    uint32 textEmote = 0;
    std::string emoteText;
    std::string emoteYell;
    switch (emote)
    {
    case TEXTEMOTE_BONK:
        emoteId = EMOTE_ONESHOT_CRY;
        textEmote = TEXTEMOTE_CRY;
        break;
    case TEXTEMOTE_SALUTE:
        emoteId = EMOTE_ONESHOT_SALUTE;
        textEmote = TEXTEMOTE_SALUTE;
        break;
    case 325:
        if (requester == source)
        {
            ai->ChangeStrategy("-follow,+stay", BotState::BOT_STATE_NON_COMBAT);
            ai->TellPlayerNoFacing(requester, "Fine.. I'll stay right here..", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        }
        break;
    case TEXTEMOTE_BECKON:
    case 324:
        if (requester == source)
        {
            ai->ChangeStrategy("+follow", BotState::BOT_STATE_NON_COMBAT);
            ai->TellPlayerNoFacing(requester, "Wherever you go, I'll follow..", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        }
        break;
    case TEXTEMOTE_WAVE:
    case TEXTEMOTE_GREET:
    case TEXTEMOTE_HAIL:
    case TEXTEMOTE_HELLO:
    case TEXTEMOTE_WELCOME:
    case TEXTEMOTE_INTRODUCE:
        emoteText = "Hey there!";
        emoteId = EMOTE_ONESHOT_WAVE;
        textEmote = TEXTEMOTE_HELLO;
        break;
    case TEXTEMOTE_DANCE:
        emoteText = "Shake what your mama gave you!";
        emoteId = EMOTE_ONESHOT_DANCE;
        textEmote = TEXTEMOTE_DANCE;
        break;
    case TEXTEMOTE_FLIRT:
    case TEXTEMOTE_KISS:
    case TEXTEMOTE_HUG:
    case TEXTEMOTE_BLUSH:
    case TEXTEMOTE_SMILE:
    case TEXTEMOTE_LOVE:
        //case TEXTEMOTE_HOLDHAND:
        emoteText = "Awwwww...";
        emoteId = EMOTE_ONESHOT_SHY;
        textEmote = TEXTEMOTE_SHY;
        break;
    case TEXTEMOTE_FLEX:
        emoteText = "Hercules! Hercules!";
        emoteId = EMOTE_ONESHOT_APPLAUD;
        textEmote = TEXTEMOTE_APPLAUD;
        break;
    case TEXTEMOTE_ANGRY:
        //case TEXTEMOTE_FACEPALM:
    case TEXTEMOTE_GLARE:
    case TEXTEMOTE_BLAME:
        //case TEXTEMOTE_FAIL:
        //case TEXTEMOTE_REGRET:
        //case TEXTEMOTE_SCOLD:
        //case TEXTEMOTE_CROSSARMS:
        emoteText = "Did I do thaaaaat?";
        emoteId = EMOTE_ONESHOT_QUESTION;
        textEmote = TEXTEMOTE_SHRUG;
        break;
    case TEXTEMOTE_FART:
    case TEXTEMOTE_BURP:
    case TEXTEMOTE_GASP:
    case TEXTEMOTE_NOSEPICK:
    case TEXTEMOTE_SNIFF:
    case TEXTEMOTE_STINK:
        emoteText = "Wasn't me! Just sayin'..";
        emoteId = EMOTE_ONESHOT_POINT;
        textEmote = TEXTEMOTE_POINT;
        break;
    case TEXTEMOTE_JOKE:
        emoteId = EMOTE_ONESHOT_LAUGH;
        textEmote = TEXTEMOTE_LAUGH;
        emoteText = "Oh.. was I not supposed to laugh so soon?";
        break;
    case TEXTEMOTE_CHICKEN:
        emoteText = "We'll see who's chicken soon enough!";
        emoteId = EMOTE_ONESHOT_RUDE;
        textEmote = TEXTEMOTE_RUDE;
        break;
    case TEXTEMOTE_APOLOGIZE:
        emoteId = EMOTE_ONESHOT_POINT;
        textEmote = TEXTEMOTE_APOLOGIZE;
        emoteText = "You damn right you're sorry!";
        break;
    case TEXTEMOTE_APPLAUD:
    case TEXTEMOTE_CLAP:
    case TEXTEMOTE_CONGRATULATE:
    case TEXTEMOTE_HAPPY:
        //case TEXTEMOTE_GOLFCLAP:
        emoteId = EMOTE_ONESHOT_BOW;
        textEmote = TEXTEMOTE_BOW;
        emoteText = "Thank you.. Thank you.. I'm here all week.";
        break;
    case TEXTEMOTE_BEG:
    case TEXTEMOTE_GROVEL:
    case TEXTEMOTE_PLEAD:
        emoteId = EMOTE_ONESHOT_NO;
        textEmote = TEXTEMOTE_NO;
        emoteText = "Beg all you want.. I have nothing for you.";
        break;
    case TEXTEMOTE_BITE:
    case TEXTEMOTE_POKE:
    case TEXTEMOTE_SCRATCH:
        //case TEXTEMOTE_PINCH:
        //case TEXTEMOTE_PUNCH:
        emoteId = EMOTE_ONESHOT_ROAR;
        textEmote = TEXTEMOTE_ROAR;
        emoteYell = "OUCH! Dammit, that hurt!";
        break;
    case TEXTEMOTE_BORED:
        emoteId = EMOTE_ONESHOT_NO;
        textEmote = TEXTEMOTE_NO;
        emoteText = "My job description doesn't include entertaining you..";
        break;
    case TEXTEMOTE_BOW:
    case TEXTEMOTE_CURTSEY:
        emoteId = EMOTE_ONESHOT_BOW;
        textEmote = TEXTEMOTE_BOW;
        break;
    case TEXTEMOTE_BRB:
    case TEXTEMOTE_SIT:
        emoteId = EMOTE_ONESHOT_EAT;
        textEmote = TEXTEMOTE_EAT;
        emoteText = "Looks like time for an AFK break..";
        break;
    case TEXTEMOTE_AGREE:
    case TEXTEMOTE_NOD:
        emoteId = EMOTE_ONESHOT_EXCLAMATION;
        textEmote = TEXTEMOTE_NOD;
        emoteText = "At least SOMEONE agrees with me!";
        break;
    case TEXTEMOTE_AMAZE:
    case TEXTEMOTE_COWER:
    case TEXTEMOTE_CRINGE:
    case TEXTEMOTE_EYE:
    case TEXTEMOTE_KNEEL:
    case TEXTEMOTE_PEER:
    case TEXTEMOTE_SURRENDER:
    case TEXTEMOTE_PRAISE:
    case TEXTEMOTE_SCARED:
    case TEXTEMOTE_COMMEND:
        //case TEXTEMOTE_AWE:
        //case TEXTEMOTE_JEALOUS:
        //case TEXTEMOTE_PROUD:
        emoteId = EMOTE_ONESHOT_FLEX;
        textEmote = TEXTEMOTE_FLEX;
        emoteText = "Yes, Yes. I know I'm amazing..";
        break;
    case TEXTEMOTE_BLEED:
    case TEXTEMOTE_MOURN:
    case TEXTEMOTE_FLOP:
        //case TEXTEMOTE_FAINT:
        //case TEXTEMOTE_PULSE:
        emoteId = EMOTE_ONESHOT_KNEEL;
        textEmote = TEXTEMOTE_KNEEL;
        emoteText = "MEDIC! Stat!";
        break;
    case TEXTEMOTE_BLINK:
        emoteId = EMOTE_ONESHOT_KICK;
        emoteText = "What? You got something in your eye?";
        break;
    case TEXTEMOTE_BOUNCE:
    case TEXTEMOTE_BARK:
        emoteId = EMOTE_ONESHOT_POINT;
        textEmote = TEXTEMOTE_POINT;
        emoteText = "Who's a good doggy? You're a good doggy!";
        break;
    case TEXTEMOTE_BYE:
        emoteId = EMOTE_ONESHOT_WAVE;
        textEmote = TEXTEMOTE_WAVE;
        emoteText = "Umm.... wait! Where are you going?!";
        break;
    case TEXTEMOTE_CACKLE:
    case TEXTEMOTE_LAUGH:
    case TEXTEMOTE_CHUCKLE:
    case TEXTEMOTE_GIGGLE:
    case TEXTEMOTE_GUFFAW:
    case TEXTEMOTE_ROFL:
    case TEXTEMOTE_SNICKER:
        //case TEXTEMOTE_SNORT:
        emoteId = EMOTE_ONESHOT_LAUGH;
        textEmote = TEXTEMOTE_LAUGH;
        emoteText = "Wait... what are we laughing at again?";
        break;
    case TEXTEMOTE_CONFUSED:
    case TEXTEMOTE_CURIOUS:
    case TEXTEMOTE_FIDGET:
    case TEXTEMOTE_FROWN:
    case TEXTEMOTE_SHRUG:
    case TEXTEMOTE_SIGH:
    case TEXTEMOTE_STARE:
    case TEXTEMOTE_TAP:
    case TEXTEMOTE_SURPRISED:
    case TEXTEMOTE_WHINE:
    case TEXTEMOTE_BOGGLE:
    case TEXTEMOTE_LOST:
    case TEXTEMOTE_PONDER:
    case TEXTEMOTE_SNUB:
    case TEXTEMOTE_SERIOUS:
    case TEXTEMOTE_EYEBROW:
        emoteId = EMOTE_ONESHOT_QUESTION;
        textEmote = TEXTEMOTE_SHRUG;
        emoteText = "Don't look at  me.. I just work here";
        break;
    case TEXTEMOTE_COUGH:
    case TEXTEMOTE_DROOL:
    case TEXTEMOTE_SPIT:
    case TEXTEMOTE_LICK:
    case TEXTEMOTE_BREATH:
        //case TEXTEMOTE_SNEEZE:
        //case TEXTEMOTE_SWEAT:
        emoteId = EMOTE_ONESHOT_POINT;
        textEmote = TEXTEMOTE_POINT;
        emoteText = "Ewww! Keep your nasty germs over there!";
        break;
    case TEXTEMOTE_CRY:
        emoteId = EMOTE_ONESHOT_CRY;
        textEmote = TEXTEMOTE_CRY;
        emoteText = "Don't you start crying or it'll make me start crying!";
        break;
    case TEXTEMOTE_CRACK:
        emoteId = EMOTE_ONESHOT_ROAR;
        textEmote = TEXTEMOTE_ROAR;
        emoteText = "It's clobbering time!";
        break;
    case TEXTEMOTE_EAT:
    case TEXTEMOTE_DRINK:
        emoteId = EMOTE_ONESHOT_EAT;
        textEmote = TEXTEMOTE_EAT;
        emoteText = "I hope you brought enough for the whole class...";
        break;
    case TEXTEMOTE_GLOAT:
    case TEXTEMOTE_MOCK:
    case TEXTEMOTE_TEASE:
    case TEXTEMOTE_EMBARRASS:
        emoteId = EMOTE_ONESHOT_CRY;
        textEmote = TEXTEMOTE_CRY;
        emoteText = "Doesn't mean you need to be an ass about it..";
        break;
    case TEXTEMOTE_HUNGRY:
        emoteId = EMOTE_ONESHOT_EAT;
        textEmote = TEXTEMOTE_EAT;
        emoteText = "What? You want some of this?";
        break;
    case TEXTEMOTE_LAYDOWN:
    case TEXTEMOTE_TIRED:
    case TEXTEMOTE_YAWN:
        emoteId = EMOTE_ONESHOT_KNEEL;
        textEmote = TEXTEMOTE_KNEEL;
        emoteText = "Is it break time already?";
        break;
    case TEXTEMOTE_MOAN:
    case TEXTEMOTE_MOON:
    case TEXTEMOTE_SEXY:
    case TEXTEMOTE_SHAKE:
    case TEXTEMOTE_WHISTLE:
    case TEXTEMOTE_CUDDLE:
    case TEXTEMOTE_PURR:
    case TEXTEMOTE_SHIMMY:
    case TEXTEMOTE_SMIRK:
    case TEXTEMOTE_WINK:
        //case TEXTEMOTE_CHARM:
        emoteId = EMOTE_ONESHOT_NO;
        textEmote = TEXTEMOTE_NO;
        emoteText = "Keep it in your pants, boss..";
        break;
    case TEXTEMOTE_NO:
    case TEXTEMOTE_VETO:
    case TEXTEMOTE_DISAGREE:
    case TEXTEMOTE_DOUBT:
        emoteId = EMOTE_ONESHOT_QUESTION;
        textEmote = TEXTEMOTE_SHRUG;
        emoteText = "Aww.... why not?!";
        break;
    case TEXTEMOTE_PANIC:
        emoteId = EMOTE_ONESHOT_EXCLAMATION;
        textEmote = TEXTEMOTE_CALM;
        emoteText = "Now is NOT the time to panic!";
        break;
    case TEXTEMOTE_POINT:
        emoteId = EMOTE_ONESHOT_POINT;
        textEmote = TEXTEMOTE_POINT;
        emoteText = "What?! I can do that TOO!";
        break;
    case TEXTEMOTE_RUDE:
    case TEXTEMOTE_RASP:
        emoteId = EMOTE_ONESHOT_RUDE;
        textEmote = TEXTEMOTE_RASP;
        emoteText = "Right back at you, bub!";
        break;
    case TEXTEMOTE_ROAR:
    case TEXTEMOTE_THREATEN:
    case TEXTEMOTE_CALM:
    case TEXTEMOTE_DUCK:
    case TEXTEMOTE_TAUNT:
    case TEXTEMOTE_PITY:
    case TEXTEMOTE_GROWL:
        //case TEXTEMOTE_TRAIN:
        //case TEXTEMOTE_INCOMING:
        //case TEXTEMOTE_CHARGE:
        //case TEXTEMOTE_FLEE:
        //case TEXTEMOTE_ATTACKMYTARGET:
    case TEXTEMOTE_OPENFIRE:
    case TEXTEMOTE_ENCOURAGE:
    case TEXTEMOTE_ENEMY:
        //case TEXTEMOTE_CHALLENGE:
        //case TEXTEMOTE_REVENGE:
        //case TEXTEMOTE_SHAKEFIST:
        emoteId = EMOTE_ONESHOT_ROAR;
        textEmote = TEXTEMOTE_ROAR;
        emoteYell = "RAWR!";
        break;
    case TEXTEMOTE_TALK:
        emoteId = EMOTE_ONESHOT_TALK;
        textEmote = TEXTEMOTE_LISTEN;
        break;
    case TEXTEMOTE_TALKEX:
        emoteId = EMOTE_ONESHOT_YES;
        textEmote = TEXTEMOTE_AGREE;
        break;
    case TEXTEMOTE_TALKQ:
    case TEXTEMOTE_LISTEN:
        emoteId = EMOTE_ONESHOT_TALK;
        textEmote = TEXTEMOTE_TALKQ;
        emoteText = "Blah Blah Blah Yakety Smackety..";
        break;
    case TEXTEMOTE_THANK:
        emoteId = EMOTE_ONESHOT_BOW;
        textEmote = TEXTEMOTE_BOW;
        emoteText = "You are quite welcome!";
        break;
    case TEXTEMOTE_VICTORY:
    case TEXTEMOTE_CHEER:
    case TEXTEMOTE_TOAST:
        //case TEXTEMOTE_HIGHFIVE:
        //case TEXTEMOTE_DING:
        emoteId = EMOTE_ONESHOT_CHEER;
        textEmote = TEXTEMOTE_CHEER;
        emoteText = "Yay!";
        break;
    case TEXTEMOTE_COLD:
    case TEXTEMOTE_SHIVER:
    case TEXTEMOTE_THIRSTY:
        //case TEXTEMOTE_OOM:
        //case TEXTEMOTE_HEALME:
        //case TEXTEMOTE_POUT:
        emoteId = EMOTE_ONESHOT_QUESTION;
        textEmote = TEXTEMOTE_PUZZLE;
        emoteText = "And what exactly am I supposed to do about that?";
        break;
    case TEXTEMOTE_COMFORT:
    case TEXTEMOTE_SOOTHE:
    case TEXTEMOTE_PAT:
        emoteId = EMOTE_ONESHOT_CRY;
        textEmote = TEXTEMOTE_CRY;
        emoteText = "Thanks...";
        break;
    case TEXTEMOTE_INSULT:
        emoteId = EMOTE_ONESHOT_CRY;
        textEmote = TEXTEMOTE_CRY;
        emoteText = "You hurt my feelings..";
        break;
    case TEXTEMOTE_JK:
        emoteId = EMOTE_ONESHOT_POINT;
        textEmote = TEXTEMOTE_POINT;
        emoteText = "You.....";
        break;
    case TEXTEMOTE_RAISE:
        emoteId = EMOTE_ONESHOT_POINT;
        textEmote = TEXTEMOTE_POINT;
        emoteText = "Yes.. you.. at the back of the class..";
        break;
    case TEXTEMOTE_READY:
        emoteId = EMOTE_ONESHOT_SALUTE;
        textEmote = TEXTEMOTE_SALUTE;
        emoteText = "Ready here, too!";
        break;
    case TEXTEMOTE_SHOO:
        emoteId = EMOTE_ONESHOT_KICK;
        textEmote = TEXTEMOTE_SHOO;
        emoteText = "Shoo yourself!";
        break;
    case TEXTEMOTE_SLAP:
        //case TEXTEMOTE_SMACK:
        emoteId = EMOTE_ONESHOT_CRY;
        textEmote = TEXTEMOTE_CRY;
        emoteText = "What did I do to deserve that?";
        break;
    case TEXTEMOTE_STAND:
        emoteId = EMOTE_ONESHOT_NONE;
        textEmote = TEXTEMOTE_STAND;
        emoteText = "What? Break time's over? Fine..";
        break;
    case TEXTEMOTE_TICKLE:
        emoteId = EMOTE_ONESHOT_LAUGH;
        textEmote = TEXTEMOTE_GIGGLE;
        emoteText = "Hey! Stop that!";
        break;
    case TEXTEMOTE_VIOLIN:
        emoteId = EMOTE_ONESHOT_TALK;
        textEmote = TEXTEMOTE_SIGH;
        emoteText = "Har Har.. very funny..";
        break;
        //case TEXTEMOTE_HELPME:
        //    bot->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
        //    bot->Yell("Quick! Someone HELP!", LANG_UNIVERSAL);
        //    break;
    case TEXTEMOTE_GOODLUCK:
        //case TEXTEMOTE_LUCK:
        emoteId = EMOTE_ONESHOT_TALK;
        textEmote = TEXTEMOTE_THANK;
        emoteText = "Thanks... I'll need it..";
        break;
    case TEXTEMOTE_BRANDISH:
        //case TEXTEMOTE_MERCY:
        emoteId = EMOTE_ONESHOT_BEG;
        textEmote = TEXTEMOTE_BEG;
        emoteText = "Please don't kill me!";
        break;
        /*case TEXTEMOTE_BADFEELING:
            bot->HandleEmoteCommand(EMOTE_ONESHOT_QUESTION);
            bot->Say("I'm just waiting for the ominous music now...", LANG_UNIVERSAL);
            break;
        case TEXTEMOTE_MAP:
            bot->HandleEmoteCommand(EMOTE_ONESHOT_NO);
            bot->Say("Noooooooo.. you just couldn't ask for directions, huh?", LANG_UNIVERSAL);
            break;
        case TEXTEMOTE_IDEA:
        case TEXTEMOTE_THINK:
            bot->HandleEmoteCommand(EMOTE_ONESHOT_NO);
            bot->Say("Oh boy.. another genius idea...", LANG_UNIVERSAL);
            break;
        case TEXTEMOTE_OFFER:
            bot->HandleEmoteCommand(EMOTE_ONESHOT_NO);
            bot->Say("No thanks.. I had some back at the last village", LANG_UNIVERSAL);
            break;
        case TEXTEMOTE_PET:
            bot->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
            bot->Say("Do I look like a dog to you?!", LANG_UNIVERSAL);
            break;
        case TEXTEMOTE_ROLLEYES:
            bot->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
            bot->Say("Keep doing that and I'll roll those eyes right out of your head..", LANG_UNIVERSAL);
            break;
        case TEXTEMOTE_SING:
            bot->HandleEmoteCommand(EMOTE_ONESHOT_APPLAUD);
            bot->Say("Lovely... just lovely..", LANG_UNIVERSAL);
            break;
        case TEXTEMOTE_COVEREARS:
            bot->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);
            bot->Yell("You think that's going to help you?!", LANG_UNIVERSAL);
            break;*/
    default:
        //return false;
        //bot->HandleEmoteCommand(EMOTE_ONESHOT_QUESTION);
        //bot->Say("Mmmmmkaaaaaay...", LANG_UNIVERSAL);
        break;
    }

    if (source && !sServerFacade.isMoving(bot) && !sServerFacade.IsInFront(bot, source, sPlayerbotAIConfig.farDistance, M_PI_F))
        sServerFacade.SetFacingTo(bot, source);

    if (verbal)
    {
        if (emoteText.size())
            bot->Say(emoteText, (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));

        if (emoteYell.size())
            bot->Yell(emoteYell, (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));
    }

    if (textEmote)
    {
        WorldPacket data(SMSG_TEXT_EMOTE);
        data << textEmote;
        data << urand(0, GetNumberOfEmoteVariants((TextEmotes)textEmote, bot->getRace(), bot->getGender()) - 1);
        data << ((source && urand(0, 1)) ? source->GetObjectGuid() : ObjectGuid());
        bot->GetSession()->HandleTextEmoteOpcode(data);
    }
    else
    {
        if (emoteId)
            bot->HandleEmoteCommand(emoteId);
    }

    return true;
}

bool EmoteAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    WorldPacket p(event.getPacket());
    uint32 emote = 0;
    Player* pSource = NULL;
    bool isReact = false;
    bool isTarget = false;
    if (!p.empty() && p.GetOpcode() == SMSG_TEXT_EMOTE)
    {
        isReact = true;
        ObjectGuid source;
        uint32 text_emote;
        uint32 emote_num;
        uint32 namlen;
        std::string nam;
        p.rpos(0);
        p >> source >> text_emote >> emote_num >> namlen;
        if (namlen > 1)
            p.read(nam, namlen);

        isTarget = nam.find(bot->GetName()) != std::string::npos;

        pSource = sObjectMgr.GetPlayer(source);

        bool allowEmote = true;

        if (text_emote == TEXTEMOTE_TALKQ)
            allowEmote = false;

        if (text_emote == TEXTEMOTE_TALK)
            allowEmote = false;

        if (text_emote == TEXTEMOTE_TALKEX)
            allowEmote = false;

        if (text_emote == TEXTEMOTE_EAT)
            allowEmote = false;

        if (allowEmote && pSource && (pSource->GetObjectGuid() != bot->GetObjectGuid()))
        {
            sLog.outDetail("Bot #%d %s:%d <%s> received SMSG_TEXT_EMOTE %d from player #%d <%s>", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), text_emote, pSource->GetGUIDLow(), pSource->GetName());
            emote = text_emote;
        }
    }

    if (!p.empty() && p.GetOpcode() == SMSG_EMOTE)
    {
        isReact = true;
        ObjectGuid source;
        uint32 emoteId;
        p.rpos(0);
        p >> emoteId >> source;

        pSource = sObjectMgr.GetPlayer(source);
        if (pSource && pSource != bot && sServerFacade.GetDistance2d(bot, pSource) < sPlayerbotAIConfig.farDistance && emoteId != EMOTE_ONESHOT_NONE)
        {
            if ((pSource->GetObjectGuid() != bot->GetObjectGuid()) && (pSource->GetSelectionGuid() == bot->GetObjectGuid() || (urand(0, 1) && sServerFacade.IsInFront(pSource, bot, 10.0f, M_PI_F))))
            {
                sLog.outDetail("Bot #%d %s:%d <%s> received SMSG_EMOTE %d from player #%d <%s>", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName(), emoteId, pSource->GetGUIDLow(), pSource->GetName());
                std::vector<uint32> types;
                for (int32 i = sEmotesTextStore.GetNumRows(); i >= 0; --i)
                {
                    EmotesTextEntry const* em = sEmotesTextStore.LookupEntry(uint32(i));
                    if (!em)
                        continue;

                    if (em->textid == EMOTE_ONESHOT_TALK)
                        continue;

                    if (em->textid == EMOTE_ONESHOT_QUESTION)
                        continue;

                    if(em->textid == EMOTE_ONESHOT_EXCLAMATION)
                        continue;

                    if (em->textid == EMOTE_ONESHOT_EAT)
                        continue;

                    if (em->textid == emoteId)
                    {
                        types.push_back(em->Id);
                    }
                }
                if (types.size())
                    emote = types[urand(0, types.size() - 1)];
            }
        }
    }

    if (isReact && !emote)
        return false;

    // less often emotes
    if (isReact && pSource)
    {
        // try to always reply to real player
        time_t lastEmote = AI_VALUE2(time_t, "last emote", qualifier);
        bool isPaused = time(0) < lastEmote;
        bool shouldReply = true;
        bool isRandomBot = false;
        uint32 accountId = pSource->GetSession()->GetAccountId();
        isRandomBot = sPlayerbotAIConfig.IsInRandomAccountList(accountId);

        // random bot speaks, emote CD
        if (isRandomBot && isPaused)
            shouldReply = false;

        if (ai->HasRealPlayerMaster())
            shouldReply = false;

        if (isRandomBot && urand(0, 20))
            shouldReply = false;

        if (!((isRandomBot && !isPaused && (!urand(0, 30)) || (!isRandomBot && (isTarget || !urand(0, 4))))))
        {
            shouldReply = false;
        }

        if (!shouldReply)
            return false;
    }

    std::string param = event.getParam();
    if ((!isReact && param.empty()) || emote)
    {
        time_t lastEmote = AI_VALUE2(time_t, "last emote", qualifier);
        ai->GetAiObjectContext()->GetValue<time_t>("last emote", qualifier)->Set(time(0) + urand(5, 25));
        param = qualifier;
    }

    if (emote)
        return ReceiveEmote(requester, pSource, emote, bot->InBattleGround() ? false : urand(0, 1));

    if (param.find("sound") == 0)
    {
        return ai->PlaySound(atoi(param.substr(5).c_str()));
    }

    if (!param.empty() && textEmotes.find(param) != textEmotes.end())
    {
        WorldPacket data(SMSG_TEXT_EMOTE);
        data << textEmotes[param];
        data << urand(0, GetNumberOfEmoteVariants((TextEmotes)textEmotes[param], bot->getRace(), bot->getGender()) - 1);
        data << ((bot->GetSelectionGuid() && urand(0, 1)) ? bot->GetSelectionGuid() : ObjectGuid());
        bot->GetSession()->HandleTextEmoteOpcode(data);

        if (emotes.find(param) != emotes.end())
            bot->HandleEmoteCommand(emotes[param]);

        return true;
    }

    if (param.empty() || emotes.find(param) == emotes.end())
    {
        int index = rand() % emotes.size();
        for (std::map<std::string, uint32>::iterator i = emotes.begin(); i != emotes.end() && index; ++i, --index)
            emote = i->second;
    }
    else
    {
        emote = emotes[param];
    }

    if (param.find("text") == 0)
    {
        emote = atoi(param.substr(4).c_str());
    }

    return Emote(GetTarget(), emote);
}

bool EmoteAction::isUseful()
{
    if (!ai->AllowActivity())
        return false;

    time_t lastEmote = AI_VALUE2(time_t, "last emote", qualifier);
    return time(0) >= lastEmote;
}


bool TalkAction::Execute(Event& event)
{
    Unit* target = ai->GetUnit(AI_VALUE(ObjectGuid, "talk target"));
    if (!target)
        target = GetTarget();

    if (!urand(0, 100))
    {
        target = NULL;
        context->GetValue<ObjectGuid>("talk target")->Set(ObjectGuid());
        return true;
    }

    if (target)
    {
        Player* player = dynamic_cast<Player*>(target);
        if (player && player->GetPlayerbotAI())
            player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<ObjectGuid>("talk target")->Set(bot->GetObjectGuid());

        context->GetValue<ObjectGuid>("talk target")->Set(target->GetObjectGuid());
        //return Emote(target, GetRandomEmote(target, true), true);
        uint32 emote = GetRandomEmote(target, true);
        WorldPacket data(SMSG_TEXT_EMOTE);
        data << emote;
        data << urand(0, GetNumberOfEmoteVariants((TextEmotes)emote, bot->getRace(), bot->getGender()) - 1);
        data << ((target && urand(0, 1)) ? target->GetObjectGuid() : ObjectGuid());
        bot->GetSession()->HandleTextEmoteOpcode(data);
        return true;
    }

    return false;
}

uint32 TalkAction::GetRandomEmote(Unit* unit, bool textEmote)
{
    std::vector<uint32> types;
    if (textEmote)
    {
        if (!urand(0, 20))
        {
            // expressions
            types.push_back(TEXTEMOTE_BOW);
            types.push_back(TEXTEMOTE_RUDE);
            types.push_back(TEXTEMOTE_CRY);
            types.push_back(TEXTEMOTE_LAUGH);
            types.push_back(TEXTEMOTE_POINT);
            types.push_back(TEXTEMOTE_CHEER);
            types.push_back(TEXTEMOTE_SHY);
            types.push_back(TEXTEMOTE_JOKE);
        }
        else
        {
            // talk
            types.push_back(TEXTEMOTE_TALK);
            types.push_back(TEXTEMOTE_TALKEX);
            types.push_back(TEXTEMOTE_TALKQ);
            if (unit && (unit->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER) || unit->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER)))
            {
                types.push_back(TEXTEMOTE_SALUTE);
            }
        }
        return types[urand(0, types.size() - 1)];
    }

    if (!urand(0, 20))
    {
        // expressions
        types.push_back(EMOTE_ONESHOT_BOW);
        types.push_back(EMOTE_ONESHOT_RUDE);
        types.push_back(EMOTE_ONESHOT_CRY);
        types.push_back(EMOTE_ONESHOT_LAUGH);
        types.push_back(EMOTE_ONESHOT_POINT);
        types.push_back(EMOTE_ONESHOT_CHEER);
        types.push_back(EMOTE_ONESHOT_SHY);
    }
    else
    {
        // talk
        types.push_back(EMOTE_ONESHOT_TALK);
        types.push_back(EMOTE_ONESHOT_EXCLAMATION);
        types.push_back(EMOTE_ONESHOT_QUESTION);
        if (unit && (unit->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER) || unit->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER)))
        {
            types.push_back(EMOTE_ONESHOT_SALUTE);
        }
    }
    return types[urand(0, types.size() - 1)];
}

uint32 EmoteActionBase::GetNumberOfEmoteVariants(TextEmotes emote, uint8 Race, uint8 Gender)
{
    if (emote == 304) // Incoming
    {
        switch (Race)
        {
        case RACE_HUMAN:
        case RACE_DWARF:
        case RACE_NIGHTELF:
        case RACE_UNDEAD:
        case RACE_TROLL:
        {
            return 2;
        }
        case RACE_GNOME:
        {
            return 1;
        }
        case RACE_ORC:
        {
            if (Gender == GENDER_MALE)
                return 3;
            return 2;
        }
        case RACE_TAUREN:
        {
            if (Gender == GENDER_MALE)
                return 2;
            return 3;
        }
        }
    }
    else if (emote == 305) // Charge
    {
        switch (Race)
        {
        case RACE_HUMAN:
        case RACE_DWARF:
        case RACE_UNDEAD:
        case RACE_TROLL:
        {
            return 2;
        }
        case RACE_NIGHTELF:
        {
            if (Gender == GENDER_MALE)
                return 2;
            return 3;
        }
        case RACE_GNOME:
        case RACE_TAUREN:
        {
            if (Gender == GENDER_MALE)
                return 3;
            return 2;
        }
        case RACE_ORC:
        {
            return 3;
        }
        }
    }
    else if (emote == 306) // Flee
    {
        switch (Race)
        {
        case RACE_HUMAN:
        case RACE_NIGHTELF:
        case RACE_ORC:
        case RACE_UNDEAD:
        case RACE_TAUREN:
        case RACE_TROLL:
        {
            return 2;
        }
        case RACE_DWARF:
        {
            if (Gender == GENDER_MALE)
                return 3;
            return 2;
        }
        case RACE_GNOME:
        {
            return 3;
        }
        }
    }
    else if (emote == TEXTEMOTE_HELLO)
    {
        switch (Race)
        {
        case RACE_HUMAN:
        case RACE_DWARF:
        case RACE_GNOME:
        {
            if (Gender == GENDER_MALE)
                return 4;
            return 3;
        }
        case RACE_NIGHTELF:
        {
            if (Gender == GENDER_MALE)
                return 3;
            return 4;
        }
        case RACE_ORC:
        case RACE_UNDEAD:
        case RACE_TAUREN:
        case RACE_TROLL:
        {
            return 3;
        }
        }
    }
    else if (emote == 323) // OOM
    {
        return 2;
    }
    else if (emote == 324) // Follow
    {
        switch (Race)
        {
        case RACE_HUMAN:
        case RACE_NIGHTELF:
        case RACE_ORC:
        case RACE_UNDEAD:
        case RACE_TAUREN:
        case RACE_TROLL:
        {
            return 2;
        }
        case RACE_DWARF:
        {
            if (Gender == GENDER_MALE)
                return 3;
            return 2;
        }
        case RACE_GNOME:
        {
            if (Gender == GENDER_MALE)
                return 2;
            return 1;
        }
        }
    }
    else if (emote == 325) // Wait
    {
        switch (Race)
        {
        case RACE_HUMAN:
        case RACE_TROLL:
        {
            return 3;
        }
        case RACE_DWARF:
        case RACE_TAUREN:
        {
            if (Gender == GENDER_MALE)
                return 3;
            return 2;
        }
        case RACE_NIGHTELF:
        case RACE_GNOME:
        case RACE_ORC:
        case RACE_UNDEAD:
        {
            return 2;
        }
        }
    }
    else if (emote == 326) // Heal me
    {
        switch (Race)
        {
        case RACE_HUMAN:
        case RACE_NIGHTELF:
        case RACE_GNOME:
        case RACE_ORC:
        case RACE_UNDEAD:
        case RACE_TAUREN:
        {
            return 2;
        }
        case RACE_DWARF:
        case RACE_TROLL:
        {
            return 3;
        }
        }
    }
    else if (emote == TEXTEMOTE_CHEER)
    {
        switch (Race)
        {
        case RACE_HUMAN:
        case RACE_NIGHTELF:
        case RACE_GNOME:
        case RACE_ORC:
        case RACE_UNDEAD:
        case RACE_TAUREN:
        {
            return 2;
        }
        case RACE_DWARF:
        case RACE_TROLL:
        {
            if (Gender == GENDER_MALE)
                return 3;
            return 2;
        }
        }
    }
    else if (emote == TEXTEMOTE_OPENFIRE)
    {
        switch (Race)
        {
        case RACE_HUMAN:
        case RACE_DWARF:
        case RACE_NIGHTELF:
        case RACE_UNDEAD:
        case RACE_TAUREN:
        case RACE_TROLL:
        case RACE_GNOME:
        {
            return 2;
        }
        case RACE_ORC:
        {
            if (Gender == GENDER_MALE)
                return 2;
            return 3;
        }
        }
    }
    else if (emote == TEXTEMOTE_BYE)
    {
        switch (Race)
        {
        case RACE_HUMAN:
        case RACE_DWARF:
        case RACE_NIGHTELF:
        case RACE_ORC:
        case RACE_UNDEAD:
        case RACE_TAUREN:
        case RACE_TROLL:
        {
            return 3;
        }
        case RACE_GNOME:
        {
            return 4;
        }
        }
    }
    else if (emote == TEXTEMOTE_NOD)
    {
        switch (Race)
        {
        case RACE_HUMAN:
        case RACE_NIGHTELF:
        case RACE_GNOME:
        case RACE_UNDEAD:
        case RACE_TAUREN:
        {
            return 3;
        }
        case RACE_DWARF:
        case RACE_TROLL:
        {
            if (Gender == GENDER_MALE)
                return 4;
            return 3;
        }
        case RACE_ORC:
        {
            return 4;
        }
        }
    }
    else if (emote == TEXTEMOTE_NO)
    {
        switch (Race)
        {
        case RACE_HUMAN:
        case RACE_DWARF:
        case RACE_TROLL:
        {
            if (Gender == GENDER_MALE)
                return 4;
            return 3;
        }
        case RACE_NIGHTELF:
        case RACE_GNOME:
        case RACE_ORC:
        case RACE_UNDEAD:
        case RACE_TAUREN:
        {
            return 3;
        }
        }
    }
    else if (emote == TEXTEMOTE_THANK)
    {
        switch (Race)
        {
        case RACE_HUMAN:
        case RACE_NIGHTELF:
        case RACE_GNOME:
        case RACE_ORC:
        case RACE_UNDEAD:
        case RACE_TROLL:
        {
            return 3;
        }
        case RACE_DWARF:
        {
            return 4;
        }
        case RACE_TAUREN:
        {
            if (Gender == GENDER_MALE)
                return 4;
            return 3;
        }
        }
    }
    else if (emote == TEXTEMOTE_WELCOME)
    {
        switch (Race)
        {
        case RACE_HUMAN:
        case RACE_DWARF:
        case RACE_NIGHTELF:
        case RACE_GNOME:
        case RACE_ORC:
        case RACE_TAUREN:
        case RACE_TROLL:
        {
            return 3;
        }
        case RACE_UNDEAD:
        {
            if (Gender == GENDER_MALE)
                return 2;
            return 3;
        }
        }
    }
    else if (emote == TEXTEMOTE_CONGRATULATE)
    {
        switch (Race)
        {
        case RACE_HUMAN:
        case RACE_NIGHTELF:
        case RACE_ORC:
        case RACE_TAUREN:
        case RACE_TROLL:
        {
            return 3;
        }
        case RACE_DWARF:
        {
            if (Gender == GENDER_MALE)
                return 5;
            return 4;
        }
        case RACE_GNOME:
        case RACE_UNDEAD:
        {
            if (Gender == GENDER_MALE)
                return 3;
            return 4;
        }
        }
    }
    else if (emote == TEXTEMOTE_FLIRT)
    {
        switch (Race)
        {
        case RACE_HUMAN:
        {
            if (Gender == GENDER_MALE)
                return 6;
            return 3;
        }
        case RACE_DWARF:
        case RACE_TAUREN:
        {
            if (Gender == GENDER_MALE)
                return 6;
            return 5;
        }
        case RACE_NIGHTELF:
        {
            if (Gender == GENDER_MALE)
                return 5;
            return 4;
        }
        case RACE_GNOME:
        case RACE_TROLL:
        {
            if (Gender == GENDER_MALE)
                return 4;
            return 5;
        }
        case RACE_ORC:
        case RACE_UNDEAD:
        {
            return 6;
        }
        }
    }
    else if (emote == TEXTEMOTE_JOKE)
    {
        switch (Race)
        {
        case RACE_HUMAN:
        {
            if (Gender == GENDER_MALE)
                return 5;
            return 6;
        }
        case RACE_DWARF:
        {
            if (Gender == GENDER_MALE)
                return 6;
            return 5;
        }
        case RACE_NIGHTELF:
        {
            if (Gender == GENDER_MALE)
                return 7;
            return 4;
        }
        case RACE_GNOME:
        {
            if (Gender == GENDER_MALE)
                return 5;
            return 3;
        }
        case RACE_ORC:
        {
            return 5;
        }
        case RACE_TAUREN:
        {
            if (Gender == GENDER_MALE)
                return 4;
            return 3;
        }
        case RACE_TROLL:
        {
            if (Gender == GENDER_MALE)
                return 5;
            return 4;
        }
        case RACE_UNDEAD:
        {
            if (Gender == GENDER_MALE)
                return 4;
            return 7;
        }
        }
    }
    return 1;
}

bool MountAnimAction::isUseful()
{
    return AI_VALUE2(bool, "mounted", "self target") && !AI_VALUE2(bool, "moving", "self target");
}

bool MountAnimAction::Execute(Event& event)
{
    WorldPacket p;
    bot->GetSession()->HandleMountSpecialAnimOpcode(p);

    return true;
}
