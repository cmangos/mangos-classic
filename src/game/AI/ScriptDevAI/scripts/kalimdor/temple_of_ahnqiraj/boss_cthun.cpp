/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Boss_Cthun
SD%Complete: 95
SDComment: Transform spell has some minor core issues. Eject from stomach event contains workarounds because of the missing spells. Digestive Acid should be handled in core.
SDCategory: Temple of Ahn'Qiraj
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "temple_of_ahnqiraj.h"

enum
{
    EMOTE_WEAKENED                  = -1531011,

    // ***** Phase 1 ********
    SPELL_EYE_BEAM                  = 26134,
    SPELL_DARK_GLARE                = 26029,
    SPELL_ROTATE_TRIGGER            = 26137,                // phase switch spell - triggers 26009 or 26136. These trigger the Dark Glare spell - 26029
    SPELL_ROTATE_360_LEFT           = 26009,
    SPELL_ROTATE_360_RIGHT          = 26136,

    SPELL_SUMMON_HOOK_TENTACLES     = 26397,                // Periodically triggers spell 26398 which cast spell 26140 to summon NPC 15725
    SPELL_SUMMON_HOOK_TENTACLE      = 26140,

    SPELL_SUMMON_EYE_TENTACLES      = 26152,
    SPELL_SUMMON_EYE_TENTACLE_1     = 26144,
    SPELL_SUMMON_EYE_TENTACLE_2     = 26145,
    SPELL_SUMMON_EYE_TENTACLE_3     = 26146,
    SPELL_SUMMON_EYE_TENTACLE_4     = 26147,
    SPELL_SUMMON_EYE_TENTACLE_5     = 26148,
    SPELL_SUMMON_EYE_TENTACLE_6     = 26149,
    SPELL_SUMMON_EYE_TENTACLE_7     = 26150,
    SPELL_SUMMON_EYE_TENTACLE_8     = 26151,

    SPELL_DESPAWN_TENTACLES         = 26399,

    // ***** Phase 2 ******
    SPELL_CARAPACE_CTHUN            = 26156,                // Was removed from client dbcs
    SPELL_TRANSFORM                 = 26232,
    SPELL_CTHUN_VULNERABLE          = 26235,
    SPELL_MOUTH_TENTACLE            = 26332,                // prepare target to teleport to stomach
    SPELL_DIGESTIVE_ACID_TELEPORT   = 26220,                // stomach teleport spell
    SPELL_EXIT_STOMACH_KNOCKBACK    = 25383,                // spell id is wrong
    SPELL_EXIT_STOMACH_JUMP         = 26224,                // should make the player jump to the ceiling - not used yet
    SPELL_EXIT_STOMACH_EFFECT       = 26230,                // used to complete the eject effect from the stomach - not used yet
    SPELL_PORT_OUT_STOMACH_EFFECT   = 26648,                // used to kill players inside the stomach on evade
    SPELL_DIGESTIVE_ACID            = 26476,                // damage spell - should be handled by the map
    // SPELL_EXIT_STOMACH            = 26221,               // summons 15800

    // ***** Summoned spells *****
    // Giant Claw tentacles
    SPELL_GIANT_GROUND_RUPTURE      = 26478,
    // SPELL_MASSIVE_GROUND_RUPTURE  = 26100,               // spell not confirmed
    SPELL_GROUND_TREMOR             = 6524,
    SPELL_HAMSTRING                 = 26211,
    SPELL_THRASH                    = 3391,
    SPELL_SUBMERGE_VISUAL           = 28819,

    NPC_EXIT_TRIGGER                = 15800,

    DISPLAY_ID_CTHUN_BODY           = 15786,                // Helper display id; This is needed in order to have the proper transform animation. ToDo: remove this when auras are fixed in core.

    AREATRIGGER_STOMACH_1           = 4033,
    AREATRIGGER_STOMACH_2           = 4034,

    MAX_FLESH_TENTACLES             = 2,
    MAX_EYE_TENTACLES               = 8,
};

static const float afCthunLocations[4][4] =
{
    { -8571.0f, 1990.0f, -98.0f, 1.22f},        // flesh tentacles locations
    { -8525.0f, 1994.0f, -98.0f, 2.12f},
    { -8562.0f, 2037.0f, -70.0f, 5.05f},        // stomach teleport location
    { -8545.6f, 1987.7f, -32.9f, 0.0f},         // stomach eject location
};

enum CThunPhase
{
    PHASE_EYE_NORMAL            = 0,
    PHASE_EYE_DARK_GLARE        = 1,
    PHASE_TRANSITION            = 2,
    PHASE_CTHUN                 = 3,
    PHASE_CTHUN_WEAKENED        = 4,
};

/*######
## boss_eye_of_cthun
######*/

struct boss_eye_of_cthunAI : public Scripted_NoMovementAI
{
    boss_eye_of_cthunAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    CThunPhase m_Phase;

    uint32 m_uiBeamTimer;
    uint32 m_uiDarkGlareTimer;
    uint32 m_uiDarkGlareEndTimer;

    GuidList m_lEyeTentaclesList;

    void Reset() override
    {
        m_Phase                 = PHASE_EYE_NORMAL;

        m_uiDarkGlareTimer      = 45 * IN_MILLISECONDS;
        m_uiDarkGlareEndTimer   = 40 * IN_MILLISECONDS;
        m_uiBeamTimer           = 0;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_CTHUN, IN_PROGRESS);

        // Start periodically summoning Eye and Claw (Hook) Tentacles
        DoCastSpellIfCan(m_creature, SPELL_SUMMON_EYE_TENTACLES, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT );
        DoCastSpellIfCan(m_creature, SPELL_SUMMON_HOOK_TENTACLES, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void JustDied(Unit* pKiller) override
    {
        // Allow the body to begin the transition (internal 5 secs delay)
        if (m_pInstance)
        {
            if (Creature* pCthun = m_pInstance->GetSingleCreatureFromStorage(NPC_CTHUN))
                m_creature->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, m_creature, pCthun);
            else
                script_error_log("C'Thun could not be found. Please check your database!");
        }
    }

    void JustReachedHome() override
    {
        // Despawn all tentacles and portals
        DoCastSpellIfCan(m_creature, SPELL_DESPAWN_TENTACLES, TRIGGERED_OLD_TRIGGERED);

        m_creature->RemoveAurasDueToSpell(SPELL_SUMMON_HOOK_TENTACLES);
        m_creature->RemoveAurasDueToSpell(SPELL_SUMMON_EYE_TENTACLES);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_CTHUN, FAIL);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        switch (pSummoned->GetEntry())
        {
            case NPC_EYE_TENTACLE:
                m_lEyeTentaclesList.push_back(pSummoned->GetObjectGuid());
            // no break;
            case NPC_CLAW_TENTACLE:
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    pSummoned->AI()->AttackStart(pTarget);
                break;
        }
    }

    // Wrapper to kill the eye tentacles before summoning new ones - Note: based on sniff I think this is a bad approach
    void DoDespawnEyeTentacles()
    {
        for (GuidList::const_iterator itr = m_lEyeTentaclesList.begin(); itr != m_lEyeTentaclesList.end(); ++itr)
        {
            if (Creature* pTemp = m_creature->GetMap()->GetCreature(*itr))
                pTemp->Suicide();
        }

        m_lEyeTentaclesList.clear();
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // We ignore targets during Dark Glare to control Eye of C'Thun's orientation ourselves
        // So we need custom AI management for this sub phase
        if (m_Phase == PHASE_EYE_DARK_GLARE)
        {
            if (m_uiDarkGlareEndTimer < uiDiff)
            {
                // Remove rotation auras
                m_creature->RemoveAurasDueToSpell(SPELL_ROTATE_360_LEFT);
                m_creature->RemoveAurasDueToSpell(SPELL_ROTATE_360_RIGHT);

                // Switch to Eye Beam
                m_uiDarkGlareEndTimer = 40 * IN_MILLISECONDS;
                m_uiBeamTimer         = 1 * IN_MILLISECONDS;
                m_Phase               = PHASE_EYE_NORMAL;
            }
            else
                m_uiDarkGlareEndTimer -= uiDiff;

            return;
        }

        // No target: do nothing more
        if (!m_creature->SelectHostileTarget())
            return;

        // Eye Beam
        if (m_uiBeamTimer < uiDiff)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(target, SPELL_EYE_BEAM) == CAST_OK)
                    m_uiBeamTimer = urand(2 * IN_MILLISECONDS, 3 * IN_MILLISECONDS);
            }
        }
        else
            m_uiBeamTimer -= uiDiff;

        // Dark Glare
        if (m_uiDarkGlareTimer < uiDiff)
        {
            // Cast the rotation spell
            if (DoCastSpellIfCan(m_creature, SPELL_ROTATE_TRIGGER) == CAST_OK)
            {
                // Switch to Dark Glare phase
                m_uiDarkGlareTimer    = 45 * IN_MILLISECONDS;
                m_Phase               = PHASE_EYE_DARK_GLARE;
            }
        }
        else
            m_uiDarkGlareTimer -= uiDiff;

        // No melee cast: Eye of C'Thun does not melee
    }
};

/*######
## boss_cthun
######*/

struct boss_cthunAI : public Scripted_NoMovementAI
{
    boss_cthunAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        // Set active in order to be used during the instance progress
        m_creature->SetActiveObjectState(true);
        Reset();
    }

    ScriptedInstance* m_pInstance;

    CThunPhase m_Phase;

    // Global variables
    uint32 m_uiPhaseTimer;
    uint8 m_uiFleshTentaclesKilled;
    uint32 m_uiEyeTentacleTimer;
    uint32 m_uiGiantClawTentacleTimer;
    uint32 m_uiGiantEyeTentacleTimer;
    uint32 m_uiDigestiveAcidTimer;

    // Body Phase
    uint32 m_uiMouthTentacleTimer;
    uint32 m_uiStomachEnterTimer;

    GuidList m_lEyeTentaclesList;
    GuidList m_lPlayersInStomachList;

    ObjectGuid m_stomachEnterTargetGuid;

    void Reset() override
    {
        // Phase information
        m_Phase                     = PHASE_TRANSITION;

        m_uiPhaseTimer              = 0;
        m_uiFleshTentaclesKilled    = 0;
        m_uiEyeTentacleTimer        = 35000;
        m_uiGiantClawTentacleTimer  = 20000;
        m_uiGiantEyeTentacleTimer   = 50000;
        m_uiDigestiveAcidTimer      = 4000;

        // Body Phase
        m_uiMouthTentacleTimer      = 15000;
        m_uiStomachEnterTimer       = 0;

        // Clear players in stomach
        m_lPlayersInStomachList.clear();

        // Reset flags
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        SetReactState(REACT_PASSIVE);
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*sender*/, Unit* /*invoker*/, uint32 /*miscValue*/) override
    {
        if (eventType == AI_EVENT_CUSTOM_A)
            m_uiPhaseTimer = 5 * IN_MILLISECONDS;
    }

    void DamageTaken(Unit* /*dealer*/, uint32& damage, DamageEffectType /*damagetype*/, SpellEntry const* /*spellInfo*/) override
    {
        // Ignore damage reduction when vulnerable
        if (m_Phase == PHASE_CTHUN_WEAKENED)
            return;

        // Prevent death in non-weakened state
        if (damage >= m_creature->GetHealth())
            damage = std::min(damage, m_creature->GetHealth() - 1);
    }

    void EnterEvadeMode() override
    {
        // Kill any player from the stomach on evade - this is becuase C'thun cannot be soloed.
        for (GuidList::const_iterator itr = m_lPlayersInStomachList.begin(); itr != m_lPlayersInStomachList.end(); ++itr)
        {
            if (Player* pPlayer = m_creature->GetMap()->GetPlayer(*itr))
                pPlayer->CastSpell(pPlayer, SPELL_PORT_OUT_STOMACH_EFFECT, TRIGGERED_OLD_TRIGGERED);
        }

        Scripted_NoMovementAI::EnterEvadeMode();
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_CTHUN, FAIL);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        m_creature->SetActiveObjectState(false);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_CTHUN, DONE);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        switch (pSummoned->GetEntry())
        {
            case NPC_EYE_TENTACLE:
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    pSummoned->AI()->AttackStart(pTarget);

                m_lEyeTentaclesList.push_back(pSummoned->GetObjectGuid());
                break;
            case NPC_GIANT_EYE_TENTACLE:
            case NPC_GIANT_CLAW_TENTACLE:
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    pSummoned->AI()->AttackStart(pTarget);
                break;
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned) override
    {
        switch (pSummoned->GetEntry())
        {
            // Handle the stomach tentacles kill
            case NPC_FLESH_TENTACLE:
                ++m_uiFleshTentaclesKilled;
                if (m_uiFleshTentaclesKilled == MAX_FLESH_TENTACLES)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_CTHUN_VULNERABLE, CAST_INTERRUPT_PREVIOUS) == CAST_OK)
                    {
                        DoScriptText(EMOTE_WEAKENED, m_creature);
                        m_uiPhaseTimer = 45000;
                        m_Phase        = PHASE_CTHUN_WEAKENED;
                    }
                }
                break;
        }
    }

    // Wrapper to handle the Flesh Tentacles spawn
    void DoSpawnFleshTentacles()
    {
        m_uiFleshTentaclesKilled = 0;

        // Spawn 2 flesh tentacles
        for (uint8 i = 0; i < MAX_FLESH_TENTACLES; ++i)
            m_creature->SummonCreature(NPC_FLESH_TENTACLE, afCthunLocations[i][0], afCthunLocations[i][1], afCthunLocations[i][2], afCthunLocations[i][3], TEMPSPAWN_DEAD_DESPAWN, 0);
    }

    // Wrapper to kill the eye tentacles before summoning new ones - Note: based on sniff I think this is a bad approach
    void DoDespawnEyeTentacles()
    {
        for (GuidList::const_iterator itr = m_lEyeTentaclesList.begin(); itr != m_lEyeTentaclesList.end(); ++itr)
        {
            if (Creature* pTemp = m_creature->GetMap()->GetCreature(*itr))
                pTemp->Suicide();
        }

        m_lEyeTentaclesList.clear();
    }

    // Wrapper to remove a stored player from the stomach
    void DoRemovePlayerFromStomach(Player* pPlayer)
    {
        if (pPlayer)
            m_lPlayersInStomachList.remove(pPlayer->GetObjectGuid());
    }

    // Custom threat management
    bool SelectHostileTarget()
    {
        Unit* pTarget = nullptr;
        Unit* pOldTarget = m_creature->GetVictim();

        if (!m_creature->getThreatManager().isThreatListEmpty())
            pTarget = m_creature->getThreatManager().getHostileTarget();

        if (pTarget)
        {
            if (pOldTarget != pTarget)
                AttackStart(pTarget);

            // Set victim to old target
            if (pOldTarget && pOldTarget->IsAlive())
            {
                m_creature->SetTarget(pOldTarget);
                m_creature->SetInFront(pOldTarget);
            }

            return true;
        }

        // Will call EnterEvadeMode if fit
        return m_creature->SelectHostileTarget();
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_Phase == PHASE_TRANSITION && m_uiPhaseTimer)
        {
            if (m_uiPhaseTimer < uiDiff)
            {
                // Note: we need to set the display id before casting the transform spell, in order to get the proper animation
                m_creature->SetDisplayId(DISPLAY_ID_CTHUN_BODY);

                // Transform and start C'thun phase
                if (DoCastSpellIfCan(m_creature, SPELL_TRANSFORM) == CAST_OK)
                {
                    m_creature->CastSpell(nullptr, SPELL_CARAPACE_CTHUN, TRIGGERED_OLD_TRIGGERED);
                    m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    SetReactState(REACT_AGGRESSIVE);
                    m_creature->SetInCombatWithZone();
                    DoSpawnFleshTentacles();

                    m_Phase        = PHASE_CTHUN;
                    m_uiPhaseTimer = 0;
                }
            }
            else
                m_uiPhaseTimer -= uiDiff;
        }

        if (!SelectHostileTarget())
            return;

        switch (m_Phase)
        {
            case PHASE_CTHUN:

                if (m_uiMouthTentacleTimer < uiDiff)
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, uint32(0), SELECT_FLAG_IN_LOS))
                    {
                        // Cast the spell using the target as source
                        pTarget->InterruptNonMeleeSpells(false);
                        pTarget->CastSpell(pTarget, SPELL_MOUTH_TENTACLE, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, m_creature->GetObjectGuid());
                        m_stomachEnterTargetGuid = pTarget->GetObjectGuid();

                        m_uiStomachEnterTimer  = 3800;
                        m_uiMouthTentacleTimer = urand(13000, 15000);
                    }
                }
                else
                    m_uiMouthTentacleTimer -= uiDiff;

                // Teleport the target to the stomach after a few seconds
                if (m_uiStomachEnterTimer)
                {
                    if (m_uiStomachEnterTimer <= uiDiff)
                    {
                        // Check for valid player
                        if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_stomachEnterTargetGuid))
                        {
                            pPlayer->CastSpell(pPlayer, SPELL_DIGESTIVE_ACID_TELEPORT, TRIGGERED_OLD_TRIGGERED);
                            m_lPlayersInStomachList.push_back(pPlayer->GetObjectGuid());
                        }

                        m_stomachEnterTargetGuid.Clear();
                        m_uiStomachEnterTimer = 0;
                    }
                    else
                        m_uiStomachEnterTimer -= uiDiff;
                }

                break;
            case PHASE_CTHUN_WEAKENED:

                // Handle Flesh Tentacles respawn when the vulnerability spell expires
                if (m_uiPhaseTimer < uiDiff)
                {
                    DoSpawnFleshTentacles();

                    m_uiPhaseTimer = 0;
                    m_Phase        = PHASE_CTHUN;
                }
                else
                    m_uiPhaseTimer -= uiDiff;

                break;
            default:
                break;
        }

        if (m_uiGiantClawTentacleTimer < uiDiff)
        {
            // Summon 1 Giant Claw Tentacle every 60 seconds
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, uint32(0), SELECT_FLAG_IN_LOS))
                m_creature->SummonCreature(NPC_GIANT_CLAW_TENTACLE, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), 0, TEMPSPAWN_DEAD_DESPAWN, 0);

            m_uiGiantClawTentacleTimer = 60000;
        }
        else
            m_uiGiantClawTentacleTimer -= uiDiff;

        if (m_uiGiantEyeTentacleTimer < uiDiff)
        {
            // Summon 1 Giant Eye Tentacle every 60 seconds
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, uint32(0), SELECT_FLAG_IN_LOS))
                m_creature->SummonCreature(NPC_GIANT_EYE_TENTACLE, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), 0, TEMPSPAWN_DEAD_DESPAWN, 0);

            m_uiGiantEyeTentacleTimer = 60000;
        }
        else
            m_uiGiantEyeTentacleTimer -= uiDiff;

        if (m_uiEyeTentacleTimer < uiDiff)
        {
            DoDespawnEyeTentacles();

            // Spawn 8 Eye Tentacles every 30 seconds
            float fX, fY, fZ;
            for (uint8 i = 0; i < MAX_EYE_TENTACLES; ++i)
            {
                m_creature->GetNearPoint(m_creature, fX, fY, fZ, 0, 30.0f, M_PI_F / 4 * i);
                m_creature->SummonCreature(NPC_EYE_TENTACLE, fX, fY, fZ, 0, TEMPSPAWN_DEAD_DESPAWN, 0);
            }

            m_uiEyeTentacleTimer = 30000;
        }
        else
            m_uiEyeTentacleTimer -= uiDiff;

        // Note: this should be applied by the teleport spell
        if (m_uiDigestiveAcidTimer < uiDiff)
        {
            // Iterate the Stomach players list and apply the Digesti acid debuff on them every 4 sec
            for (GuidList::const_iterator itr = m_lPlayersInStomachList.begin(); itr != m_lPlayersInStomachList.end(); ++itr)
            {
                if (Player* pPlayer = m_creature->GetMap()->GetPlayer(*itr))
                    pPlayer->CastSpell(pPlayer, SPELL_DIGESTIVE_ACID, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, m_creature->GetObjectGuid());
            }
            m_uiDigestiveAcidTimer = 4000;
        }
        else
            m_uiDigestiveAcidTimer -= uiDiff;
    }
};

/*######
## npc_giant_claw_tentacle
######*/

struct npc_giant_claw_tentacleAI : public Scripted_NoMovementAI
{
    npc_giant_claw_tentacleAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiThrashTimer;
    uint32 m_uiHamstringTimer;
    uint32 m_uiDistCheckTimer;

    void Reset() override
    {
        m_uiHamstringTimer  = 2000;
        m_uiThrashTimer     = 5000;
        m_uiDistCheckTimer  = 5000;

        DoCastSpellIfCan(m_creature, SPELL_GIANT_GROUND_RUPTURE);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Check if we have a target
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        if (m_uiDistCheckTimer)
        {
            if (m_uiDistCheckTimer < uiDiff)
            {
                // If there is nobody in range, spawn a new tentacle at a new target location
                if (!m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0, uint32(0), SELECT_FLAG_IN_MELEE_RANGE) && m_pInstance)
                {
                    if (Creature* pCthun = m_pInstance->GetSingleCreatureFromStorage(NPC_CTHUN))
                    {
                        if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, uint32(0), SELECT_FLAG_NOT_IN_MELEE_RANGE))
                        {
                            pCthun->SummonCreature(NPC_GIANT_CLAW_TENTACLE, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), 0, TEMPSPAWN_DEAD_DESPAWN, 0);

                            m_uiDistCheckTimer = 0;

                            // Self kill when a new tentacle is spawned
                            SetCombatScriptStatus(true);
                            m_creature->SetTarget(nullptr);
                            m_creature->CastSpell(nullptr, SPELL_SUBMERGE_VISUAL, TRIGGERED_OLD_TRIGGERED);
                            m_creature->ForcedDespawn(1500);
                            if (Creature* portal = GetClosestCreatureWithEntry(m_creature, NPC_GIANT_TENTACLE_PORTAL, 5.0f))
                                portal->ForcedDespawn(1500);
                            return;
                        }
                    }
                }
                else
                    m_uiDistCheckTimer = 5000;
            }
            else
                m_uiDistCheckTimer -= uiDiff;
        }

        if (m_uiThrashTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_THRASH) == CAST_OK)
                m_uiThrashTimer = 10000;
        }
        else
            m_uiThrashTimer -= uiDiff;

        if (m_uiHamstringTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_HAMSTRING) == CAST_OK)
                m_uiHamstringTimer = 10000;
        }
        else
            m_uiHamstringTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

/*######
## at_stomach_cthun
######*/

bool AreaTrigger_at_stomach_cthun(Player* pPlayer, AreaTriggerEntry const* pAt)
{
    if (pAt->id == AREATRIGGER_STOMACH_1)
    {
        if (pPlayer->isGameMaster() || !pPlayer->IsAlive())
            return false;

        // Summon the exit trigger which should push the player outside the stomach - not used because missing eject spells
        // if (!GetClosestCreatureWithEntry(pPlayer, NPC_EXIT_TRIGGER, 10.0f))
        //    pPlayer->CastSpell(pPlayer, SPELL_EXIT_STOMACH, TRIGGERED_OLD_TRIGGERED);

        // Note: because of the missing spell id 26224, we will use basic jump movement
        // Disabled because of the missing jump effect
        // pPlayer->GetMotionMaster()->MoveJump(afCthunLocations[3][0], afCthunLocations[3][1], afCthunLocations[3][2], pPlayer->GetSpeed(MOVE_RUN)*5, 0);

        // Note: this should actually be handled by at_stomach_2!
        if (ScriptedInstance* pInstance = (ScriptedInstance*)pPlayer->GetInstanceData())
        {
            if (Creature* pCthun = pInstance->GetSingleCreatureFromStorage(NPC_CTHUN))
            {
                // Remove player from the Stomach
                if (boss_cthunAI* pBossAI = dynamic_cast<boss_cthunAI*>(pCthun->AI()))
                    pBossAI->DoRemovePlayerFromStomach(pPlayer);

                // Teleport back to C'thun and remove the Digestive Acid
                pPlayer->RemoveAurasDueToSpell(SPELL_DIGESTIVE_ACID);
                pPlayer->NearTeleportTo(pCthun->GetPositionX(), pCthun->GetPositionY(), pCthun->GetPositionZ() + 15.0f, frand(0, 2 * M_PI_F));

                // Note: the real knockback spell id should be 26230
                pPlayer->CastSpell(pPlayer, SPELL_EXIT_STOMACH_KNOCKBACK, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, pCthun->GetObjectGuid());
            }
        }
    }

    return false;
}

UnitAI* GetAI_boss_eye_of_cthun(Creature* pCreature)
{
    return new boss_eye_of_cthunAI(pCreature);
}

UnitAI* GetAI_boss_cthun(Creature* pCreature)
{
    return new boss_cthunAI(pCreature);
}

UnitAI* GetAI_npc_giant_claw_tentacle(Creature* pCreature)
{
    return new npc_giant_claw_tentacleAI(pCreature);
}

struct SummonHookTentacle : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_0)
        {
            if (Unit* target = spell->GetUnitTarget())
                target->CastSpell(target, SPELL_SUMMON_HOOK_TENTACLE, TRIGGERED_OLD_TRIGGERED);
        }
    }
};

struct PeriodicSummonEyeTrigger : public AuraScript
{
    void OnPeriodicTrigger(Aura* aura, PeriodicTriggerData& data) const override
    {
        if (Unit* caster = aura->GetCaster())
        {
            if (caster->GetEntry() != NPC_EYE_OF_CTHUN)
                return;
            if (auto* eyeOfCThunAI = dynamic_cast<boss_eye_of_cthunAI*>(caster->AI()))
                eyeOfCThunAI->DoDespawnEyeTentacles();
            else
                return; // Something went wrong

            uint32 eyeTentaclesSpells[] = { SPELL_SUMMON_EYE_TENTACLE_1, SPELL_SUMMON_EYE_TENTACLE_2, SPELL_SUMMON_EYE_TENTACLE_3, SPELL_SUMMON_EYE_TENTACLE_4, SPELL_SUMMON_EYE_TENTACLE_5, SPELL_SUMMON_EYE_TENTACLE_6, SPELL_SUMMON_EYE_TENTACLE_7, SPELL_SUMMON_EYE_TENTACLE_8 };
            for (auto spellId:eyeTentaclesSpells)
                caster->CastSpell(nullptr, spellId, TRIGGERED_OLD_TRIGGERED);
        }
    }
};

struct RotateTrigger : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_0)
        {
            if (Unit* target = spell->GetUnitTarget())
            {
                // Clear target before changing orientation
                target->SetTarget(nullptr);
                target->CastSpell(target, urand(0, 1) ? SPELL_ROTATE_360_LEFT : SPELL_ROTATE_360_RIGHT, TRIGGERED_OLD_TRIGGERED);
            }
        }
    }
};

struct PeriodicRotate : public AuraScript
{
    void OnPeriodicTrigger(Aura* aura, PeriodicTriggerData& data) const override
    {
        if (Unit* target = aura->GetTarget())
        {
            // Clear target before changing orientation
            target->SetTarget(nullptr);

            float newAngle = target->GetOrientation();
            if (aura->GetId() == SPELL_ROTATE_360_RIGHT)
                newAngle -= M_PI_F / 40;
            else
                newAngle += M_PI_F / 40;
            newAngle = MapManager::NormalizeOrientation(newAngle);
            target->SetFacingTo(newAngle);

            data.spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(SPELL_DARK_GLARE);
            data.caster = aura->GetCaster();
            data.target = nullptr;
        }
    }
};

void AddSC_boss_cthun()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_eye_of_cthun";
    pNewScript->GetAI = &GetAI_boss_eye_of_cthun;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_cthun";
    pNewScript->GetAI = &GetAI_boss_cthun;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_giant_claw_tentacle";
    pNewScript->GetAI = &GetAI_npc_giant_claw_tentacle;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "at_stomach_cthun";
    pNewScript->pAreaTrigger = &AreaTrigger_at_stomach_cthun;
    pNewScript->RegisterSelf();

    RegisterSpellScript<SummonHookTentacle>("spell_cthun_hook_tentacle");
    RegisterSpellScript<RotateTrigger>("spell_cthun_rotate_trigger");
    RegisterAuraScript<PeriodicSummonEyeTrigger>("spell_cthun_periodic_eye_trigger");
    RegisterAuraScript<PeriodicRotate>("spell_cthun_periodic_rotate");
}
