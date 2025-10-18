#pragma once
#include "playerbot/PlayerbotAIAware.h"

class Player;
class PlayerbotAI;

namespace ai
{
    class AiObjectContext;
    class ChatHelper;

    class AiObject : public PlayerbotAIAware
	{
	public:
        AiObject(PlayerbotAI* ai);

    protected:
        Player* bot;
        Player* GetMaster();
        AiObjectContext* context;
        ChatHelper* chat;
	};

    class AiNamedObject : public AiObject
    {
    public:
        AiNamedObject(PlayerbotAI* ai, std::string name) : AiObject(ai), name(name) {}

    public:
        virtual std::string getName() { return name; }

    protected:
        std::string name;
    };
}

// MACROS GO HERE

//
// TRIGGERS
//

#define NEXT_TRIGGERS(name, relevance) \
    virtual NextAction* getNextAction() { return new NextAction(name, relevance); }

#define BEGIN_TRIGGER(clazz, super) \
class clazz : public super \
    { \
    public: \
        clazz(PlayerbotAI* ai) : super(ai) {} \
    public: \
        virtual bool IsActive();

#define END_TRIGGER() \
    };

#define BUFF_TRIGGER(clazz, spell) \
    class clazz : public BuffTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : BuffTrigger(ai, spell) {} \
    }

#define BUFF_TRIGGER_TIME(clazz, spell, time) \
    class clazz : public BuffTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : BuffTrigger(ai, spell, time) {} \
    }

#define MY_BUFF_TRIGGER(clazz, spell) \
    class clazz : public MyBuffTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : MyBuffTrigger(ai, spell) {} \
    }

#define BUFF_TRIGGER_A(clazz, spell) \
    class clazz : public BuffTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : BuffTrigger(ai, spell) {} \
        virtual bool IsActive(); \
    }

#define MY_BUFF_TRIGGER_A(clazz, spell) \
    class clazz : public MyBuffTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : MyBuffTrigger(ai, spell) {} \
        virtual bool IsActive(); \
    }

#define BUFF_PARTY_TRIGGER(clazz, spell) \
    class clazz : public BuffOnPartyTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : BuffOnPartyTrigger(ai, spell) {}  \
    }

#define BUFF_PARTY_TRIGGER_A(clazz, spell) \
    class clazz : public BuffOnPartyTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : BuffOnPartyTrigger(ai, spell) {}  \
        virtual bool IsActive(); \
    }

#define DEBUFF_TRIGGER(clazz, spell) \
    class clazz : public DebuffTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : DebuffTrigger(ai, spell) {} \
    }

#define DEBUFF_ONLY_TRIGGER(clazz, spell) \
    class clazz : public DebuffTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : DebuffTrigger(ai, spell) {} \
    }

#define DEBUFF_TRIGGER_A(clazz, spell) \
    class clazz : public DebuffTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : DebuffTrigger(ai, spell) {} \
        virtual bool IsActive(); \
    }

#define DEBUFF_ENEMY_TRIGGER(clazz, spell) \
    class clazz : public DebuffOnAttackerTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : DebuffOnAttackerTrigger(ai, spell) {} \
    }

#define DEBUFF_ENEMY_TRIGGER_A(clazz, spell) \
    class clazz : public DebuffOnAttackerTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : DebuffOnAttackerTrigger(ai, spell) {} \
        virtual bool IsActive(); \
    }

#define CURE_TRIGGER(clazz, spell, dispel) \
    class clazz : public NeedCureTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : NeedCureTrigger(ai, spell, dispel) {} \
    }

#define CURE_PARTY_TRIGGER(clazz, spell, dispel) \
    class clazz : public PartyMemberNeedCureTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : PartyMemberNeedCureTrigger(ai, spell, dispel) {} \
    }

#define CAN_CAST_TRIGGER(clazz, spell) \
    class clazz : public SpellCanBeCastedTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : SpellCanBeCastedTrigger(ai, spell) {} \
    }

#define CAN_CAST_TRIGGER_A(clazz, spell) \
    class clazz : public SpellCanBeCastedTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : SpellCanBeCastedTrigger(ai, spell) {} \
        virtual bool IsActive(); \
    }

#define CD_TRIGGER(clazz, spell) \
    class clazz : public SpellNoCooldownTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : SpellNoCooldownTrigger(ai, spell) {} \
    }

#define INTERRUPT_TRIGGER(clazz, spell) \
    class clazz : public InterruptSpellTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : InterruptSpellTrigger(ai, spell) {} \
    }

#define INTERRUPT_TRIGGER_A(clazz, spell) \
    class clazz : public InterruptSpellTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : InterruptSpellTrigger(ai, spell) {} \
        virtual bool IsActive(); \
    }

#define HAS_AURA_TRIGGER(clazz, spell) \
    class clazz : public HasAuraTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : HasAuraTrigger(ai, spell) {} \
    }

#define HAS_AURA_TRIGGER_A(clazz, spell) \
    class clazz : public HasAuraTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : HasAuraTrigger(ai, spell) {} \
        virtual bool IsActive(); \
    }

#define HAS_AURA_TRIGGER_TIME(clazz, spell, time) \
    class clazz : public HasAuraTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : HasAuraTrigger(ai, spell, time) {} \
    }

#define SNARE_TRIGGER(clazz, spell) \
    class clazz : public SnareTargetTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : SnareTargetTrigger(ai, spell) {} \
    }

#define SNARE_TRIGGER_A(clazz, spell) \
    class clazz : public SnareTargetTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : SnareTargetTrigger(ai, spell) {} \
        virtual bool IsActive(); \
    }

#define PROTECT_TRIGGER(clazz, spell) \
    class clazz : public ProtectPartyMemberTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : ProtectPartyMemberTrigger(ai) {} \
    }

#define DEFLECT_TRIGGER(clazz, spell) \
    class clazz : public DeflectSpellTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : DeflectSpellTrigger(ai, spell) {} \
    }

#define BOOST_TRIGGER(clazz, spell) \
    class clazz : public BoostTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : BoostTrigger(ai, spell) {} \
    }

#define BOOST_TRIGGER_A(clazz, spell) \
    class clazz : public BoostTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : BoostTrigger(ai, spell) {} \
        virtual bool IsActive(); \
    }

#define INTERRUPT_HEALER_TRIGGER(clazz, spell) \
    class clazz : public InterruptEnemyHealerTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : InterruptEnemyHealerTrigger(ai, spell) {} \
    }

#define INTERRUPT_HEALER_TRIGGER_A(clazz, spell) \
    class clazz : public InterruptEnemyHealerTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : InterruptEnemyHealerTrigger(ai, spell) {} \
        virtual bool IsActive(); \
    }

#define CC_TRIGGER(clazz, spell) \
    class clazz : public HasCcTargetTrigger \
    { \
    public: \
        clazz(PlayerbotAI* ai) : HasCcTargetTrigger(ai, spell) {} \
    }

//
// ACTIONS
//

#define MELEE_ACTION(clazz, spell) \
    class clazz : public CastMeleeSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastMeleeSpellAction(ai, spell) {} \
    }

#define MELEE_ACTION_U(clazz, spell, useful) \
    class clazz : public CastMeleeSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastMeleeSpellAction(ai, spell) {} \
        virtual bool isUseful() override { return useful; } \
    }

#define SPELL_ACTION(clazz, spell) \
    class clazz : public CastSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastSpellAction(ai, spell) {} \
    }

#define SPELL_ACTION_U(clazz, spell, useful) \
    class clazz : public CastSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastSpellAction(ai, spell) {} \
        virtual bool isUseful() override { return useful; } \
    }

#define HEAL_ACTION(clazz, spell) \
    class clazz : public CastHealingSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastHealingSpellAction(ai, spell) {} \
    }

#define HEAL_ACTION_U(clazz, spell, useful) \
    class clazz : public CastHealingSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastHealingSpellAction(ai, spell) {} \
        virtual bool isUseful() override { return useful; } \
    }

#define HEAL_PARTY_ACTION(clazz, spell) \
    class clazz : public HealPartyMemberAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : HealPartyMemberAction(ai, spell) {} \
    }

#define HEAL_HOT_PARTY_ACTION(clazz, spell) \
    class clazz : public HealHotPartyMemberAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : HealHotPartyMemberAction(ai, spell) {} \
    }

#define AOE_HEAL_ACTION(clazz, spell) \
    class clazz : public CastAoeHealSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastAoeHealSpellAction(ai, spell) {} \
    }

#define REMOVE_BUFF_ACTION(clazz, spell) \
    class clazz : public RemoveBuffAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : RemoveBuffAction(ai, spell) {} \
    }

#define BUFF_ACTION(clazz, spell) \
    class clazz : public CastBuffSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastBuffSpellAction(ai, spell) {} \
    }

#define BUFF_ACTION_U(clazz, spell, useful) \
    class clazz : public CastBuffSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastBuffSpellAction(ai, spell) {} \
        virtual bool isUseful() override { return useful; } \
    }

#define BUFF_PARTY_ACTION(clazz, spell) \
    class clazz : public BuffOnPartyAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : BuffOnPartyAction(ai, spell) {} \
    }

#define GREATER_BUFF_PARTY_ACTION(clazz, spell) \
    class clazz : public GreaterBuffOnPartyAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : GreaterBuffOnPartyAction(ai, spell) {} \
    }

#define CURE_ACTION(clazz, spell) \
    class clazz : public CastCureSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastCureSpellAction(ai, spell) {} \
    }

#define CURE_PARTY_ACTION(clazz, spell, dispel) \
    class clazz : public CurePartyMemberAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CurePartyMemberAction(ai, spell, dispel) {} \
    }

#define RESS_ACTION(clazz, spell) \
    class clazz : public ResurrectPartyMemberAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : ResurrectPartyMemberAction(ai, spell) {} \
    }

#define MELEE_DEBUFF_ACTION(clazz, spell) \
    class clazz : public CastMeleeDebuffSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastMeleeDebuffSpellAction(ai, spell) {} \
    }

#define RANGED_DEBUFF_ACTION(clazz, spell) \
    class clazz : public CastRangedDebuffSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, spell) {} \
    }

#define MELEE_DEBUFF_ACTION_U(clazz, spell, useful) \
    class clazz : public CastMeleeDebuffSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastMeleeDebuffSpellAction(ai, spell) {} \
        virtual bool isUseful() override { return useful; } \
    }

#define RANGED_DEBUFF_ACTION_U(clazz, spell, useful) \
    class clazz : public CastRangedDebuffSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, spell) {} \
        virtual bool isUseful() override { return useful; } \
    }

#define MELEE_DEBUFF_ACTION_R(clazz, spell, distance) \
    class clazz : public CastMeleeDebuffSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastMeleeDebuffSpellAction(ai, spell) { \
            range = distance; \
        } \
    }

#define RANGED_DEBUFF_ACTION_R(clazz, spell, distance) \
    class clazz : public CastRangedDebuffSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, spell) { \
            range = distance; \
        } \
    }

#define MELEE_DEBUFF_ENEMY_ACTION(clazz, spell) \
    class clazz : public CastMeleeDebuffSpellOnAttackerAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastMeleeDebuffSpellOnAttackerAction(ai, spell) {} \
    }

#define RANGED_DEBUFF_ENEMY_ACTION(clazz, spell) \
    class clazz : public CastRangedDebuffSpellOnAttackerAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastRangedDebuffSpellOnAttackerAction(ai, spell) {} \
    }

#define REACH_ACTION(clazz, spell, range) \
    class clazz : public CastReachTargetSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastReachTargetSpellAction(ai, spell, range) {} \
    }

#define REACH_ACTION_U(clazz, spell, range, useful) \
    class clazz : public CastReachTargetSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastReachTargetSpellAction(ai, spell, range) {} \
        virtual bool isUseful() override { return useful; } \
    }

#define ENEMY_HEALER_ACTION(clazz, spell) \
    class clazz : public CastSpellOnEnemyHealerAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastSpellOnEnemyHealerAction(ai, spell) {} \
    }


#define SNARE_ACTION(clazz, spell) \
    class clazz : public CastSnareSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastSnareSpellAction(ai, spell) {} \
    }

#define CC_ACTION(clazz, spell) \
    class clazz : public CastCrowdControlSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastCrowdControlSpellAction(ai, spell) {} \
    }

#define PROTECT_ACTION(clazz, spell) \
    class clazz : public CastProtectSpellAction \
    { \
    public: \
        clazz(PlayerbotAI* ai) : CastProtectSpellAction(ai, spell) {} \
    }

#define END_RANGED_SPELL_ACTION() \
    };

#define BEGIN_SPELL_ACTION(clazz, name) \
class clazz : public CastSpellAction \
        { \
        public: \
        clazz(PlayerbotAI* ai) : CastSpellAction(ai, name) {} \


#define END_SPELL_ACTION() \
    };

#define BEGIN_MELEE_DEBUFF_ACTION(clazz, name) \
class clazz : public CastMeleeDebuffSpellAction \
        { \
        public: \
        clazz(PlayerbotAI* ai) : CastMeleeDebuffSpellAction(ai, name) {} \

#define BEGIN_RANGED_DEBUFF_ACTION(clazz, name) \
class clazz : public CastRangedDebuffSpellAction \
        { \
        public: \
        clazz(PlayerbotAI* ai) : CastRangedDebuffSpellAction(ai, name) {} \

#define BEGIN_RANGED_SPELL_ACTION(clazz, name) \
class clazz : public CastSpellAction \
        { \
        public: \
        clazz(PlayerbotAI* ai) : CastSpellAction(ai, name) {} \

#define BEGIN_MELEE_SPELL_ACTION(clazz, name) \
class clazz : public CastMeleeSpellAction \
        { \
        public: \
        clazz(PlayerbotAI* ai) : CastMeleeSpellAction(ai, name) {} \


#define END_RANGED_SPELL_ACTION() \
    };


#define BEGIN_BUFF_ON_PARTY_ACTION(clazz, name) \
class clazz : public BuffOnPartyAction \
        { \
        public: \
        clazz(PlayerbotAI* ai) : BuffOnPartyAction(ai, name) {}

//
// Action node
//

// node_name , action, prerequisite
#define ACTION_NODE_P(name, spell, pre) \
static ActionNode* name(PlayerbotAI* ai) \
    { \
    return new ActionNode(spell, \
        /*P*/ NextAction::array(0, new NextAction(pre), NULL), \
        /*A*/ NULL, \
        /*C*/ NULL); \
    }

// node_name , action, alternative
#define ACTION_NODE_A(name, spell, alt) \
static ActionNode* name(PlayerbotAI* ai) \
    { \
    return new ActionNode(spell, \
        /*P*/ NULL, \
        /*A*/ NextAction::array(0, new NextAction(alt), NULL), \
        /*C*/ NULL); \
    }

// node_name , action, continuer
#define ACTION_NODE_C(name, spell, con) \
static ActionNode* name(PlayerbotAI* ai) \
    { \
    return new ActionNode(spell, \
        /*P*/ NULL, \
        /*A*/ NULL, \
        /*C*/ NextAction::array(0, new NextAction(con), NULL)); \
    }
