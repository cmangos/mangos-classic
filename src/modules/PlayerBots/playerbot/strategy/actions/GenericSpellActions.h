#pragma once
#include "playerbot/strategy/Action.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/PlayerbotAI.h"

namespace ai
{
    class CastSpellAction : public Action
    {
    public:
        CastSpellAction(PlayerbotAI* ai, std::string spell);
        virtual ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_SINGLE; }
        virtual bool Execute(Event& event) override;
        virtual bool isPossible() override;
		virtual bool isUseful() override;

        // Used when this action is executed as a reaction
        bool ShouldReactionInterruptCast() const override { return true; }

        bool HasReachAction() { return !GetReachActionName().empty(); }
        
    protected:
        const uint32& GetSpellID() const { return spellId; }
        const std::string& GetSpellName() const { return spellName; }
        void SetSpellName(const std::string& name, std::string spellIDContextName = "spell id", bool force = false);

        Unit* GetTarget() override;
        virtual std::string GetTargetName() override { return "current target"; }
        virtual std::string GetTargetQualifier() { return ""; }
        virtual std::string GetReachActionName() { return "reach spell"; }

        virtual NextAction** getPrerequisites() override;

    protected:
		float range;

    private:
        std::string spellName;
        uint32 spellId;
    };

    class CastPetSpellAction : public CastSpellAction
    {
    public:
        CastPetSpellAction(PlayerbotAI* ai, std::string spell) : CastSpellAction(ai, spell) {}

        virtual bool isPossible() override;

    protected:
        virtual std::string GetTargetName() override { return "current target"; }
        std::string GetReachActionName() override { return ""; }
    };

	//---------------------------------------------------------------------------------------------------------------------
	class CastAuraSpellAction : public CastSpellAction
	{
	public:
		CastAuraSpellAction(PlayerbotAI* ai, std::string spell, bool isOwner = false) : CastSpellAction(ai, spell), isOwner(isOwner) {}
		virtual bool isUseful() override;

    protected:
        virtual std::string GetReachActionName() override { return "reach spell"; }

    protected:
        bool isOwner;
	};

    //---------------------------------------------------------------------------------------------------------------------
    class CastMeleeSpellAction : public CastSpellAction
    {
    public:
        CastMeleeSpellAction(PlayerbotAI* ai, std::string spell) : CastSpellAction(ai, spell) 
        {
            range = ATTACK_DISTANCE;
        }

    protected:
        virtual std::string GetReachActionName() override { return "reach melee"; }
    };

    //---------------------------------------------------------------------------------------------------------------------
    class CastMeleeAoeSpellAction : public CastSpellAction
    {
    public:
        CastMeleeAoeSpellAction(PlayerbotAI* ai, std::string spell, float radius) : CastSpellAction(ai, spell), radius(radius)
        {
            range = ATTACK_DISTANCE;
        }

        virtual bool isUseful() override;

    protected:
        virtual std::string GetReachActionName() override { return ""; }

    protected:
        float radius;
    };

    //---------------------------------------------------------------------------------------------------------------------
    class CastMeleeDebuffSpellAction : public CastAuraSpellAction
    {
    public:
        CastMeleeDebuffSpellAction(PlayerbotAI* ai, std::string spell, bool isOwner = true) : CastAuraSpellAction(ai, spell, isOwner)
        {
            range = ATTACK_DISTANCE;
        }
    
    protected:
        virtual std::string GetReachActionName() override { return "reach melee"; }
    };
    
    class CastRangedDebuffSpellAction : public CastAuraSpellAction
    {
    public:
        CastRangedDebuffSpellAction(PlayerbotAI* ai, std::string spell, bool isOwner = true) : CastAuraSpellAction(ai, spell, isOwner) {}
    
    protected:
        virtual std::string GetReachActionName() override { return "reach spell"; }
    };

    class CastMeleeDebuffSpellOnAttackerAction : public CastAuraSpellAction
    {
    public:
        CastMeleeDebuffSpellOnAttackerAction(PlayerbotAI* ai, std::string spell, bool isOwner = true) : CastAuraSpellAction(ai, spell, isOwner)
        {
            range = ATTACK_DISTANCE;
        }

    protected:
        std::string GetReachActionName() override { return "reach melee"; }
        std::string GetTargetName() override { return "attacker without aura"; }
        std::string GetTargetQualifier() override { return GetSpellName(); }
        virtual std::string getName() override { return GetSpellName() + " on attacker"; }
        virtual ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_AOE; }
    };

    class CastRangedDebuffSpellOnAttackerAction : public CastAuraSpellAction
    {
    public:
        CastRangedDebuffSpellOnAttackerAction(PlayerbotAI* ai, std::string spell, bool isOwner = true) : CastAuraSpellAction(ai, spell, isOwner) {}
        
    protected:
        virtual std::string GetReachActionName() override { return "reach spell"; }
        virtual std::string GetTargetName() override { return "attacker without aura"; }
        virtual std::string GetTargetQualifier() override { return GetSpellName(); }
        virtual std::string getName() override { return GetSpellName() + " on attacker"; }
        virtual ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_AOE; }
    };

	class CastBuffSpellAction : public CastAuraSpellAction
	{
	public:
		CastBuffSpellAction(PlayerbotAI* ai, std::string spell) : CastAuraSpellAction(ai, spell) { }
        virtual std::string GetTargetName() override { return "self target"; }
	};

    class CastSpellTargetAction : public CastSpellAction
    {
    public:
        CastSpellTargetAction(PlayerbotAI* ai, std::string spell, std::string targetsValue, bool aliveCheck = false, bool auraCheck = false) : CastSpellAction(ai, spell), targetsValue(targetsValue), aliveCheck(aliveCheck), auraCheck(auraCheck) {}
        virtual std::string GetTargetName() override { return "self target"; }
        virtual bool IsTargetValid(Unit* target);
        Unit* GetTarget() override;

    private:
        std::string targetsValue;
        bool aliveCheck;
        bool auraCheck;
    };

    class CastItemTargetAction : public CastSpellTargetAction
    {
    public:
        CastItemTargetAction(PlayerbotAI* ai, std::string targetsValue, bool aliveCheck = false, bool auraCheck = false) : CastSpellTargetAction(ai, "item target", targetsValue, aliveCheck, false), itemAuraCheck(auraCheck) {}
        virtual bool IsTargetValid(Unit* target) override;
        virtual uint32 GetItemId() = 0;

    protected:
        virtual bool isUseful() override;
        virtual bool isPossible() override;
        virtual bool Execute(Event& event) override;

    private:
        bool HasSpellCooldown(uint32 itemId);

    private:
        bool itemAuraCheck;
    };

	class CastEnchantItemAction : public CastSpellAction
	{
	public:
	    CastEnchantItemAction(PlayerbotAI* ai, std::string spell) : CastSpellAction(ai, spell) { }
        virtual std::string GetTargetName() override { return "self target"; }
        virtual bool isPossible() override;
	};

    //---------------------------------------------------------------------------------------------------------------------

    class CastHealingSpellAction : public CastAuraSpellAction
    {
    public:
        CastHealingSpellAction(PlayerbotAI* ai, std::string spell, uint8 estAmount = 15.0f) : CastAuraSpellAction(ai, spell, true), estAmount(estAmount) {}
        
    protected:
        virtual ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_AOE; }
        virtual std::string GetTargetName() override { return "self target"; }
        virtual std::string GetReachActionName() override { return "reach party member to heal"; }

    protected:
        uint8 estAmount;
    };

    class CastAoeHealSpellAction : public CastHealingSpellAction
    {
    public:
    	CastAoeHealSpellAction(PlayerbotAI* ai, std::string spell, uint8 estAmount = 15.0f) : CastHealingSpellAction(ai, spell, estAmount) {}
		virtual std::string GetTargetName() override { return "party member to heal"; }
        virtual bool isUseful() override;
    };

	class CastCureSpellAction : public CastSpellAction
	{
	public:
		CastCureSpellAction(PlayerbotAI* ai, std::string spell) : CastSpellAction(ai, spell) {}
		virtual std::string GetTargetName() override { return "self target"; }
	};

	class PartyMemberActionNameSupport 
    {
	public:
		PartyMemberActionNameSupport(std::string spell) : name(spell + " on party") {}
		std::string getName() { return name; }

	private:
		std::string name;
	};

    class HealPartyMemberAction : public CastHealingSpellAction, public PartyMemberActionNameSupport
    {
    public:
        HealPartyMemberAction(PlayerbotAI* ai, std::string spell, uint8 estAmount = 15.0f) : CastHealingSpellAction(ai, spell, estAmount), PartyMemberActionNameSupport(spell) {}
        virtual std::string getName() override { return PartyMemberActionNameSupport::getName(); }
		virtual std::string GetTargetName() override { return "party member to heal"; }
    };

    class HealHotPartyMemberAction : public HealPartyMemberAction
    {
    public:
        HealHotPartyMemberAction(PlayerbotAI* ai, std::string spell) : HealPartyMemberAction(ai, spell) {}
        virtual bool isUseful() override;
    };

	class ResurrectPartyMemberAction : public CastSpellAction
	{
	public:
		ResurrectPartyMemberAction(PlayerbotAI* ai, std::string spell) : CastSpellAction(ai, spell) {}
		
    protected:
        virtual std::string GetTargetName() override { return "party member to resurrect"; }
        virtual std::string GetReachActionName() override { return "reach party member to heal"; }
	};
    //---------------------------------------------------------------------------------------------------------------------

    class CurePartyMemberAction : public CastSpellAction, public PartyMemberActionNameSupport
    {
    public:
        CurePartyMemberAction(PlayerbotAI* ai, std::string spell, uint32 dispelType) : CastSpellAction(ai, spell), PartyMemberActionNameSupport(spell), dispelType(dispelType) {}
    
    protected:
        virtual std::string GetReachActionName() override { return "reach party member to heal"; }
        virtual std::string getName() override { return PartyMemberActionNameSupport::getName(); }
        virtual std::string GetTargetName() override { return "party member to dispel"; }
        virtual std::string GetTargetQualifier() override { return std::to_string(dispelType); }

    protected:
        uint32 dispelType;
    };

    //---------------------------------------------------------------------------------------------------------------------

    class BuffOnPartyAction : public CastBuffSpellAction, public PartyMemberActionNameSupport
    {
    public:
        BuffOnPartyAction(PlayerbotAI* ai, std::string spell, bool ignoreTanks = false) : CastBuffSpellAction(ai, spell), PartyMemberActionNameSupport(spell), ignoreTanks(ignoreTanks) {}
        
    protected:
        virtual std::string getName() override { return PartyMemberActionNameSupport::getName(); }
        virtual std::string GetTargetName() override { return "friendly unit without aura"; }
        virtual std::string GetTargetQualifier() override { return GetSpellName() + "-" + (ignoreTanks ? "1" : "0"); }

    protected:
        bool ignoreTanks;
    };

    //---------------------------------------------------------------------------------------------------------------------

    class GreaterBuffOnPartyAction : public CastBuffSpellAction, public PartyMemberActionNameSupport
    {
    public:
        GreaterBuffOnPartyAction(PlayerbotAI* ai, std::string spell, bool ignoreTanks = false) : CastBuffSpellAction(ai, spell), PartyMemberActionNameSupport(spell), ignoreTanks(ignoreTanks) {}

    protected:
        virtual std::string getName() override { return PartyMemberActionNameSupport::getName(); }
        virtual std::string GetTargetName() override { return "party member without aura"; }
        virtual std::string GetTargetQualifier() override { return GetSpellName() + "-" + (ignoreTanks ? "1" : "0"); }

    private:
        bool ignoreTanks;
    };

    //---------------------------------------------------------------------------------------------------------------------

    class PartyTankActionNameSupport
    {
    public:
        PartyTankActionNameSupport(std::string spell) : name(spell + " on tank") {}
        std::string getName() { return name; }

    private:
        std::string name;
    };

    class BuffOnTankAction : public CastBuffSpellAction, public PartyMemberActionNameSupport
    {
    public:
        BuffOnTankAction(PlayerbotAI* ai, std::string spell) : CastBuffSpellAction(ai, spell), PartyMemberActionNameSupport(spell) {}

    protected:
        virtual std::string getName() override { return PartyMemberActionNameSupport::getName(); }
        virtual std::string GetTargetName() override { return "party tank without aura"; }
        virtual std::string GetTargetQualifier() override { return GetSpellName(); }
    };

    class CastShootAction : public CastSpellAction
    {
    public:
        CastShootAction(PlayerbotAI* ai) : CastSpellAction(ai, "shoot"), rangedWeapon(nullptr), weaponDelay(0), needsAmmo(false) {}
        ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_NONE; }
        bool Execute(Event& event) override;
        bool isPossible() override;

    protected:
        virtual std::string GetReachActionName() override { return "reach spell"; }

    private:
        void UpdateWeaponInfo();

    private:
        const Item* rangedWeapon;
        uint32 weaponDelay;
        bool needsAmmo;
    };

    class RemoveBuffAction : public Action
    {
    public:
        RemoveBuffAction(PlayerbotAI* ai, std::string spell) : Action(ai, "remove aura"), name(spell) {}
        virtual std::string getName() override { return "remove " + name; }
        virtual bool isUseful() override;
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }
    private:
        std::string name;
    };

    // racials

    // heal
#ifndef MANGOSBOT_ZERO
    HEAL_ACTION(CastGiftOfTheNaaruAction, "gift of the naaru");
#endif
    HEAL_ACTION(CastCannibalizeAction, "cannibalize");

    // buff

    BUFF_ACTION(CastShadowmeldAction, "shadowmeld");
    BUFF_ACTION(CastBerserkingAction, "berserking");
    BUFF_ACTION(CastBloodFuryAction, "blood fury");
    BUFF_ACTION(CastStoneformAction, "stoneform");
    BUFF_ACTION(CastPerceptionAction, "perception");

    // spells

#ifndef MANGOSBOT_ZERO
    SPELL_ACTION(CastManaTapAction, "mana tap");
    SPELL_ACTION(CastArcaneTorrentAction, "arcane torrent");
#endif

    class CastWarStompAction : public CastSpellAction
    {
    public:
        CastWarStompAction(PlayerbotAI* ai) : CastSpellAction(ai, "war stomp") {}
    };

    //cc breakers

    BUFF_ACTION(CastWillOfTheForsakenAction, "will of the forsaken");
    BUFF_ACTION_U(CastEscapeArtistAction, "escape artist", !ai->HasAura("stealth", AI_VALUE(Unit*, "self target")));

#ifdef MANGOSBOT_TWO
    SPELL_ACTION(CastEveryManforHimselfAction, "every man for himself");
#endif

    class CastSpellOnEnemyHealerAction : public CastSpellAction
    {
    public:
        CastSpellOnEnemyHealerAction(PlayerbotAI* ai, std::string spell) : CastSpellAction(ai, spell) {}
        
    protected:
        virtual std::string GetReachActionName() override { return "reach spell"; }
        virtual std::string GetTargetName() override { return "enemy healer target"; }
        virtual std::string GetTargetQualifier() override { return GetSpellName(); }
        virtual std::string getName() override { return GetSpellName() + " on enemy healer"; }
    };

    class CastSnareSpellAction : public CastRangedDebuffSpellAction
    {
    public:
        CastSnareSpellAction(PlayerbotAI* ai, std::string spell) : CastRangedDebuffSpellAction(ai, spell) {}
        
    protected:
        virtual std::string GetReachActionName() override { return "reach spell"; }
        virtual std::string GetTargetName() override { return "snare target"; }
        virtual std::string GetTargetQualifier() override { return GetSpellName(); }
        virtual std::string getName() override { return GetSpellName() + " on snare target"; }
        virtual ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_NONE; }
    };

    class CastCrowdControlSpellAction : public CastRangedDebuffSpellAction
    {
    public:
        CastCrowdControlSpellAction(PlayerbotAI* ai, std::string spell) : CastRangedDebuffSpellAction(ai, spell) {}
        
    private:
        virtual std::string GetReachActionName() override { return "reach spell"; }
        virtual std::string GetTargetName() override { return "cc target"; }
        virtual std::string GetTargetQualifier() override { return GetSpellName(); }
        virtual ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_NONE; }
    };

    class CastProtectSpellAction : public CastSpellAction
    {
    public:
        CastProtectSpellAction(PlayerbotAI* ai, std::string spell) : CastSpellAction(ai, spell) {}
        virtual bool isUseful() override { return CastSpellAction::isUseful() && !ai->HasAura(GetSpellName(), GetTarget()); }

    protected:
        virtual std::string GetReachActionName() override { return "reach spell"; }
        virtual std::string GetTargetName() override { return "party member to protect"; }
        virtual ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_NONE; }
    };

    class InterruptCurrentSpellAction : public Action
    {
    public:
        InterruptCurrentSpellAction(PlayerbotAI* ai) : Action(ai, "interrupt current spell") {}
        virtual bool isUseful() override;
        virtual bool Execute(Event& event) override;
    };

    //--------------------//
    //   Vehicle Actions  //
    //--------------------//

    class CastVehicleSpellAction : public CastSpellAction
    {
    public:
        CastVehicleSpellAction(PlayerbotAI* ai, std::string spell) : CastSpellAction(ai, spell)
        {
            range = 120.0f;
            speed = 30.0f;
            SetSpellName(spell, "vehicle spell id", !GetSpellID());
        }

        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override;
        virtual bool isPossible() override;

    protected:
        virtual ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_NONE; }
        virtual std::string GetTargetName() override { return "current target"; }
        virtual std::string GetReachActionName() override { return ""; }
        float speed;
        bool needTurn = true;
    };

    class CastHurlBoulderAction : public CastVehicleSpellAction
    {
    public:
        CastHurlBoulderAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "hurl boulder") {}
    };

    class CastSteamRushAction : public CastVehicleSpellAction
    {
    public:
        CastSteamRushAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "steam rush") {}
    };

    class CastRamAction : public CastVehicleSpellAction
    {
    public:
        CastRamAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "ram") {}
    };

    class CastNapalmAction : public CastVehicleSpellAction
    {
    public:
        CastNapalmAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "napalm") {}
    };

    class CastFireCannonAction : public CastVehicleSpellAction
    {
    public:
        CastFireCannonAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "fire cannon") {}
    };

    class CastSteamBlastAction : public CastVehicleSpellAction
    {
    public:
        CastSteamBlastAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "steam blast") {}
    };

    class CastIncendiaryRocketAction : public CastVehicleSpellAction
    {
    public:
        CastIncendiaryRocketAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "incendiary rocket") {}
    };

    class CastRocketBlastAction : public CastVehicleSpellAction
    {
    public:
        CastRocketBlastAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "rocket blast") {}
    };

    class CastGlaiveThrowAction : public CastVehicleSpellAction
    {
    public:
        CastGlaiveThrowAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "glaive throw") {}
    };

    class CastBladeSalvoAction : public CastVehicleSpellAction
    {
    public:
        CastBladeSalvoAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "blade salvo") {}
    };

    class CastDeliverStolenHorseAction : public CastVehicleSpellAction
    {
    public:
        CastDeliverStolenHorseAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "deliver stolen horse") {}
    protected:
        virtual std::string GetTargetName() override { return "self target"; }
    };

    class CastHorsemansCallAction : public CastVehicleSpellAction
    {
    public:
        CastHorsemansCallAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "horseman call") {
            range = 120.0f;
            SetSpellName("Horseman's Call", "vehicle spell id", true);
        }
    protected:
        virtual std::string GetTargetName() override { return "self target"; }
    };

    class CastScarletCannonAction : public CastVehicleSpellAction
    {
    public:
        CastScarletCannonAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "scarlet cannon") { speed = 3000.0f; }
    protected:
        virtual std::string GetTargetName() override { return "grind target"; }
    };

    class CastElectroMagneticPulseAction : public CastVehicleSpellAction
    {
    public:
        CastElectroMagneticPulseAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "electro - magnetic pulse") { range = 20.0f; }
    protected:
        virtual std::string GetTargetName() override { return "grind target"; }
    };

    class CastSkeletalGryphonEscapeAction : public CastVehicleSpellAction
    {
    public:
        CastSkeletalGryphonEscapeAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "skeletal gryphon escape") {}
    protected:
        virtual std::string GetTargetName() override { return "self target"; }
    };

    class CastFrozenDeathboltAction : public CastVehicleSpellAction
    {
    public:
        CastFrozenDeathboltAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "frozen deathbolt")
        {
            needTurn = false;
            speed = 3000.0f;
        };
        virtual bool isPossible() override;
    protected:
        virtual std::string GetTargetName() override { return "grind target"; }
    };

    class CastDevourHumanoidAction : public CastVehicleSpellAction
    {
    public:
        CastDevourHumanoidAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "devour humanoid")
        {
            range = 15.0f;
            needTurn = false;
        }
        virtual bool isPossible() override;
    protected:
        virtual std::string GetTargetName() override { return "grind target"; }
    };
}
