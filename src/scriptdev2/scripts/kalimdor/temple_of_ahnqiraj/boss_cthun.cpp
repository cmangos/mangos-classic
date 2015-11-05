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
EndScriptData */

#include "precompiled.h"
#include "temple_of_ahnqiraj.h"

enum
{
    EMOTE_WEAKENED                  = -1531011,

    // ***** Phase 1 ********
    SPELL_EYE_BEAM                  = 26134,
    // SPELL_DARK_GLARE              = 26029,
    SPELL_ROTATE_TRIGGER            = 26137,                // phase switch spell - triggers 26009 or 26136. These trigger the Dark Glare spell - 26029
    SPELL_ROTATE_360_LEFT           = 26009,
    SPELL_ROTATE_360_RIGHT          = 26136,

    // ***** Phase 2 ******
    // SPELL_CARAPACE_CTHUN         = 26156,                // Was removed from client dbcs
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

    // Npcs
    // Phase 1 npcs
    NPC_CLAW_TENTACLE               = 15725,                // summoned by missing spell 26140
    NPC_EYE_TENTACLE                = 15726,                // summoned by spells 26144 - 26151; script effect of 26152 - triggered every 45 secs
    NPC_TENTACLE_PORTAL             = 15904,                // summoned by missing spell 26396

    // Phase 2 npcs
    NPC_GIANT_CLAW_TENTACLE         = 15728,                // summoned by missing spell 26216 every 60 secs - also teleported by spell 26191
    NPC_GIANT_EYE_TENTACLE          = 15334,                // summoned by missing spell 26768 every 60 secs
    NPC_FLESH_TENTACLE              = 15802,                // summoned by missing spell 26237 every 10 secs
    NPC_GIANT_TENTACLE_PORTAL       = 15910,                // summoned by missing spell 26477
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

    uint32 m_uiClawTentacleTimer;
    uint32 m_uiEyeTentacleTimer;
    uint32 m_uiBeamTimer;
    uint32 m_uiDarkGlareTimer;
    uint32 m_uiDarkGlareEndTimer;

    GuidList m_lEyeTentaclesList;

    void Reset() override
    {
        m_Phase                 = PHASE_EYE_NORMAL;

        m_uiDarkGlareTimer      = 45000;
        m_uiDarkGlareEndTimer   = 40000;
        m_uiClawTentacleTimer   = urand(10000, 15000);
        m_uiBeamTimer           = 0;
        m_uiEyeTentacleTimer    = 45000;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_CTHUN, IN_PROGRESS);
    }

    void JustDied(Unit* pKiller) override
    {
        // Allow the body to begin the transition
        if (m_pInstance)
        {
            if (Creature* pCthun = m_pInstance->GetSingleCreatureFromStorage(NPC_CTHUN))
                pCthun->AI()->AttackStart(pKiller);
            else
                script_error_log("C'thun could not be found. Please check your database!");
        }
    }

    void JustReachedHome() override
    {
        // Despawn Eye tentacles on evade
        DoDespawnEyeTentacles();

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

                pSummoned->SummonCreature(NPC_TENTACLE_PORTAL, pSummoned->GetPositionX(), pSummoned->GetPositionY(), pSummoned->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
                break;
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned) override
    {
        // Despawn the tentacle portal - this applies to all the summoned tentacles
        if (Creature* pPortal = GetClosestCreatureWithEntry(pSummoned, NPC_TENTACLE_PORTAL, 5.0f))
            pPortal->ForcedDespawn();
    }

    void SummonedCreatureDespawn(Creature* pSummoned) override
    {
        // Used only after evade
        if (SelectHostileTarget())
            return;

        // Despawn the tentacle portal - this applies to all the summoned tentacles for evade case (which is handled by creature linking)
        if (Creature* pPortal = GetClosestCreatureWithEntry(pSummoned, NPC_TENTACLE_PORTAL, 5.0f))
            pPortal->ForcedDespawn();
    }

    // Wrapper to kill the eye tentacles before summoning new ones
    void DoDespawnEyeTentacles()
    {
        for (GuidList::const_iterator itr = m_lEyeTentaclesList.begin(); itr != m_lEyeTentaclesList.end(); ++itr)
        {
            if (Creature* pTemp = m_creature->GetMap()->GetCreature(*itr))
                pTemp->DealDamage(pTemp, pTemp->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NONE, nullptr, false);
        }

        m_lEyeTentaclesList.clear();
    }

    // Custom threat management
    bool SelectHostileTarget()
    {
        Unit* pTarget = nullptr;
        Unit* pOldTarget = m_creature->getVictim();

        if (!m_creature->getThreatManager().isThreatListEmpty())
            pTarget = m_creature->getThreatManager().getHostileTarget();

        if (pTarget)
        {
            if (pOldTarget != pTarget && m_Phase == PHASE_EYE_NORMAL)
                AttackStart(pTarget);

            // Set victim to old target (if not while Dark Glare)
            if (pOldTarget && pOldTarget->isAlive() && m_Phase == PHASE_EYE_NORMAL)
            {
                m_creature->SetTargetGuid(pOldTarget->GetObjectGuid());
                m_creature->SetInFront(pOldTarget);
            }

            return true;
        }

        // Will call EnterEvadeMode if fit
        return m_creature->SelectHostileTarget();
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!SelectHostileTarget())
            return;

        switch (m_Phase)
        {
            case PHASE_EYE_NORMAL:

                if (m_uiBeamTimer < uiDiff)
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    {
                        if (DoCastSpellIfCan(pTarget, SPELL_EYE_BEAM) == CAST_OK)
                        {
                            m_creature->SetTargetGuid(pTarget->GetObjectGuid());
                            m_uiBeamTimer = 3000;
                        }
                    }
                }
                else
                    m_uiBeamTimer -= uiDiff;

                if (m_uiDarkGlareTimer < uiDiff)
                {
                    // Cast the rotation spell
                    if (DoCastSpellIfCan(m_creature, SPELL_ROTATE_TRIGGER) == CAST_OK)
                    {
                        // Remove the target focus but allow the boss to face the current victim
                        m_creature->SetTargetGuid(ObjectGuid());
                        m_creature->SetFacingToObject(m_creature->getVictim());

                        // Switch to Dark Glare phase
                        m_uiDarkGlareTimer    = 45000;
                        m_Phase               = PHASE_EYE_DARK_GLARE;
                    }
                }
                else
                    m_uiDarkGlareTimer -= uiDiff;

                break;
            case PHASE_EYE_DARK_GLARE:

                if (m_uiDarkGlareEndTimer < uiDiff)
                {
                    // Remove rotation auras
                    m_creature->RemoveAurasDueToSpell(SPELL_ROTATE_360_LEFT);
                    m_creature->RemoveAurasDueToSpell(SPELL_ROTATE_360_RIGHT);

                    // Switch to Eye Beam
                    m_uiDarkGlareEndTimer = 40000;
                    m_uiBeamTimer         = 1000;
                    m_Phase               = PHASE_EYE_NORMAL;
                }
                else
                    m_uiDarkGlareEndTimer -= uiDiff;

                break;
            default:
                break;
        }

        if (m_uiClawTentacleTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                // Spawn claw tentacle on the random target on both phases
                m_creature->SummonCreature(NPC_CLAW_TENTACLE, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                m_uiClawTentacleTimer = urand(7000, 13000);
            }
        }
        else
            m_uiClawTentacleTimer -= uiDiff;

        if (m_uiEyeTentacleTimer <= uiDiff)
        {
            // Despawn the eye tentacles if necessary
            DoDespawnEyeTentacles();

            // Spawn 8 Eye Tentacles
            float fX, fY, fZ;
            for (uint8 i = 0; i < MAX_EYE_TENTACLES; ++i)
            {
                m_creature->GetNearPoint(m_creature, fX, fY, fZ, 0, 30.0f, M_PI_F / 4 * i);
                m_creature->SummonCreature(NPC_EYE_TENTACLE, fX, fY, fZ, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
            }

            m_uiEyeTentacleTimer = 45000;
        }
        else
            m_uiEyeTentacleTimer -= uiDiff;
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

        m_uiPhaseTimer              = 5000;
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
    }

    void DamageTaken(Unit* /*pDealer*/, uint32& uiDamage) override
    {
        // Ignore damage reduction when vulnerable
        if (m_Phase == PHASE_CTHUN_WEAKENED)
            return;

        // Not weakened so reduce damage by 99% - workaround for missing spell 26156
        if (uiDamage / 99 > 0)
            uiDamage /= 99;
        else
            uiDamage = 1;

        // Prevent death in non-weakened state
        if (uiDamage >= m_creature->GetHealth())
            uiDamage = 0;
    }

    void EnterEvadeMode() override
    {
        // Kill any player from the stomach on evade - this is becuase C'thun cannot be soloed.
        for (GuidList::const_iterator itr = m_lPlayersInStomachList.begin(); itr != m_lPlayersInStomachList.end(); ++itr)
        {
            if (Player* pPlayer = m_creature->GetMap()->GetPlayer(*itr))
                pPlayer->CastSpell(pPlayer, SPELL_PORT_OUT_STOMACH_EFFECT, true);
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
                pSummoned->SummonCreature(NPC_TENTACLE_PORTAL, pSummoned->GetPositionX(), pSummoned->GetPositionY(), pSummoned->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
                break;
            case NPC_GIANT_EYE_TENTACLE:
            case NPC_GIANT_CLAW_TENTACLE:
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    pSummoned->AI()->AttackStart(pTarget);

                pSummoned->SummonCreature(NPC_GIANT_TENTACLE_PORTAL, pSummoned->GetPositionX(), pSummoned->GetPositionY(), pSummoned->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
                break;
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned) override
    {
        switch (pSummoned->GetEntry())
        {
                // Handle portal despawn on tentacle kill
            case NPC_EYE_TENTACLE:
                if (Creature* pPortal = GetClosestCreatureWithEntry(pSummoned, NPC_TENTACLE_PORTAL, 5.0f))
                    pPortal->ForcedDespawn();
                break;
            case NPC_GIANT_EYE_TENTACLE:
            case NPC_GIANT_CLAW_TENTACLE:
                if (Creature* pPortal = GetClosestCreatureWithEntry(pSummoned, NPC_GIANT_TENTACLE_PORTAL, 5.0f))
                    pPortal->ForcedDespawn();
                break;
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
            m_creature->SummonCreature(NPC_FLESH_TENTACLE, afCthunLocations[i][0], afCthunLocations[i][1], afCthunLocations[i][2], afCthunLocations[i][3], TEMPSUMMON_DEAD_DESPAWN, 0);
    }

    // Wrapper to kill the eye tentacles before summoning new ones
    void DoDespawnEyeTentacles()
    {
        for (GuidList::const_iterator itr = m_lEyeTentaclesList.begin(); itr != m_lEyeTentaclesList.end(); ++itr)
        {
            if (Creature* pTemp = m_creature->GetMap()->GetCreature(*itr))
                pTemp->DealDamage(pTemp, pTemp->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NONE, nullptr, false);
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
        Unit* pOldTarget = m_creature->getVictim();

        if (!m_creature->getThreatManager().isThreatListEmpty())
            pTarget = m_creature->getThreatManager().getHostileTarget();

        if (pTarget)
        {
            if (pOldTarget != pTarget)
                AttackStart(pTarget);

            // Set victim to old target
            if (pOldTarget && pOldTarget->isAlive())
            {
                m_creature->SetTargetGuid(pOldTarget->GetObjectGuid());
                m_creature->SetInFront(pOldTarget);
            }

            return true;
        }

        // Will call EnterEvadeMode if fit
        return m_creature->SelectHostileTarget();
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!SelectHostileTarget())
            return;

        switch (m_Phase)
        {
            case PHASE_TRANSITION:

                if (m_uiPhaseTimer < uiDiff)
                {
                    // Note: we need to set the display id before casting the transform spell, in order to get the proper animation
                    m_creature->SetDisplayId(DISPLAY_ID_CTHUN_BODY);

                    // Transform and start C'thun phase
                    if (DoCastSpellIfCan(m_creature, SPELL_TRANSFORM) == CAST_OK)
                    {
                        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        DoSpawnFleshTentacles();

                        m_Phase        = PHASE_CTHUN;
                        m_uiPhaseTimer = 0;
                    }
                }
                else
                    m_uiPhaseTimer -= uiDiff;

                break;
            case PHASE_CTHUN:

                if (m_uiMouthTentacleTimer < uiDiff)
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, uint32(0), SELECT_FLAG_IN_LOS))
                    {
                        // Cast the spell using the target as source
                        pTarget->InterruptNonMeleeSpells(false);
                        pTarget->CastSpell(pTarget, SPELL_MOUTH_TENTACLE, true, nullptr, nullptr, m_creature->GetObjectGuid());
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
                            pPlayer->CastSpell(pPlayer, SPELL_DIGESTIVE_ACID_TELEPORT, true);
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
                m_creature->SummonCreature(NPC_GIANT_CLAW_TENTACLE, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), 0, TEMPSUMMON_DEAD_DESPAWN, 0);

            m_uiGiantClawTentacleTimer = 60000;
        }
        else
            m_uiGiantClawTentacleTimer -= uiDiff;

        if (m_uiGiantEyeTentacleTimer < uiDiff)
        {
            // Summon 1 Giant Eye Tentacle every 60 seconds
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, uint32(0), SELECT_FLAG_IN_LOS))
                m_creature->SummonCreature(NPC_GIANT_EYE_TENTACLE, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), 0, TEMPSUMMON_DEAD_DESPAWN, 0);

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
                m_creature->SummonCreature(NPC_EYE_TENTACLE, fX, fY, fZ, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
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
                    pPlayer->CastSpell(pPlayer, SPELL_DIGESTIVE_ACID, true, nullptr, nullptr, m_creature->GetObjectGuid());
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
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiDistCheckTimer < uiDiff)
        {
            // If there is nobody in range, spawn a new tentacle at a new target location
            if (!m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0, uint32(0), SELECT_FLAG_IN_MELEE_RANGE) && m_pInstance)
            {
                if (Creature* pCthun = m_pInstance->GetSingleCreatureFromStorage(NPC_CTHUN))
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, uint32(0), SELECT_FLAG_NOT_IN_MELEE_RANGE))
                    {
                        pCthun->SummonCreature(NPC_GIANT_CLAW_TENTACLE, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), 0, TEMPSUMMON_DEAD_DESPAWN, 0);

                        // Self kill when a new tentacle is spawned
                        m_creature->DealDamage(m_creature, m_creature->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NONE, nullptr, false);
                        return;
                    }
                }
            }
            else
                m_uiDistCheckTimer = 5000;
        }
        else
            m_uiDistCheckTimer -= uiDiff;

        if (m_uiThrashTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_THRASH) == CAST_OK)
                m_uiThrashTimer = 10000;
        }
        else
            m_uiThrashTimer -= uiDiff;

        if (m_uiHamstringTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_HAMSTRING) == CAST_OK)
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
        if (pPlayer->isGameMaster() || !pPlayer->isAlive())
            return false;

        // Summon the exit trigger which should push the player outside the stomach - not used because missing eject spells
        // if (!GetClosestCreatureWithEntry(pPlayer, NPC_EXIT_TRIGGER, 10.0f))
        //    pPlayer->CastSpell(pPlayer, SPELL_EXIT_STOMACH, true);

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
                pPlayer->CastSpell(pPlayer, SPELL_EXIT_STOMACH_KNOCKBACK, true, nullptr, nullptr, pCthun->GetObjectGuid());
            }
        }
    }

    return false;
}

CreatureAI* GetAI_boss_eye_of_cthun(Creature* pCreature)
{
    return new boss_eye_of_cthunAI(pCreature);
}

CreatureAI* GetAI_boss_cthun(Creature* pCreature)
{
    return new boss_cthunAI(pCreature);
}

CreatureAI* GetAI_npc_giant_claw_tentacle(Creature* pCreature)
{
    return new npc_giant_claw_tentacleAI(pCreature);
}

void AddSC_boss_cthun()
{
    Script* pNewScript;

    pNewScript = new Script;
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
}
