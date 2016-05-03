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
 SDName: Sunken_Temple
 SD%Complete: 100
 SDComment: Quest support: 8733
 SDCategory: Sunken Temple
 EndScriptData */

 /* ContentData
 at_shade_of_eranikus
 npc_malfurion_stormrage
 event_antalarion_statue_activation
 event_avatar_of_hakkar
 go_eternal_flame
 effectDummy_summon_hakkar
 EndContentData */

#include "precompiled.h"
#include "sunken_temple.h"
#include "Spell.h"

enum
{
	QUEST_THE_CHARGE_OF_DRAGONFLIGHTS = 8555,
	QUEST_ERANIKUS_TYRANT_OF_DREAMS = 8733
};

bool AreaTrigger_at_shade_of_eranikus(Player* pPlayer, AreaTriggerEntry const* /*pAt*/)
{
	if (ScriptedInstance* pInstance = (ScriptedInstance*)pPlayer->GetInstanceData())
	{
		// Only do stuff, if the player has finished the PreQuest
		if (pPlayer->GetQuestRewardStatus(QUEST_THE_CHARGE_OF_DRAGONFLIGHTS) &&
			!pPlayer->GetQuestRewardStatus(QUEST_ERANIKUS_TYRANT_OF_DREAMS) &&
			pPlayer->GetQuestStatus(QUEST_ERANIKUS_TYRANT_OF_DREAMS) != QUEST_STATUS_COMPLETE)
		{
			if (pInstance->GetData(TYPE_MALFURION) != DONE)
			{
				pPlayer->SummonCreature(NPC_MALFURION, aSunkenTempleLocation[2].m_fX, aSunkenTempleLocation[2].m_fY, aSunkenTempleLocation[2].m_fZ, aSunkenTempleLocation[2].m_fO, TEMPSUMMON_DEAD_DESPAWN, 0);
				pInstance->SetData(TYPE_MALFURION, DONE);
			}
		}
	}
	return false;
}

/*######
## npc_malfurion_stormrage
######*/

enum
{
	EMOTE_MALFURION1 = -1109000,
	SAY_MALFURION1 = -1109001,
	SAY_MALFURION2 = -1109002,
	SAY_MALFURION3 = -1109003,
	SAY_MALFURION4 = -1109004,

	MAX_MALFURION_TEMPLE_SPEECHES = 6
};

struct npc_malfurionAI : public ScriptedAI
{
	npc_malfurionAI(Creature* pCreature) : ScriptedAI(pCreature)
	{
		// Only in Sunken Temple
		if (m_creature->GetMap()->IsDungeon())
		{
			DoScriptText(EMOTE_MALFURION1, m_creature);
			m_uiSpeech = 0;
			m_uiSayTimer = 3000;
		}

		m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
	}

	uint32 m_uiSayTimer;
	uint32 m_uiSpeech;

	void Reset() override {}
	void UpdateAI(const uint32 uiDiff) override
	{
		// We are in Sunken Temple
		if (m_creature->GetMap()->IsDungeon())
		{
			if (m_uiSpeech < MAX_MALFURION_TEMPLE_SPEECHES)
			{
				if (m_uiSayTimer <= uiDiff)
				{
					switch (m_uiSpeech)
					{
					case 0:
						m_creature->HandleEmote(EMOTE_ONESHOT_BOW);
						m_uiSayTimer = 2000;
						break;
					case 1:
						DoScriptText(SAY_MALFURION1, m_creature);
						m_creature->HandleEmote(EMOTE_STATE_TALK);
						m_uiSayTimer = 12000;
						break;
					case 2:
						DoScriptText(SAY_MALFURION2, m_creature);
						m_uiSayTimer = 12000;
						break;
					case 3:
						DoScriptText(SAY_MALFURION3, m_creature);
						m_uiSayTimer = 11000;
						break;
					case 4:
						DoScriptText(SAY_MALFURION4, m_creature);
						m_uiSayTimer = 4000;
						break;
					case 5:
						m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
						m_creature->HandleEmote(EMOTE_STATE_NONE);
						break;
					}

					++m_uiSpeech;
				}
				else
					m_uiSayTimer -= uiDiff;
			}
		}
	}
};

CreatureAI* GetAI_npc_malfurion(Creature* pCreature)
{
	return new npc_malfurionAI(pCreature);
}

uint32 SUPRESSOR_YELLS[3] =
{
	-1999947,
	-1999948,
	-1999949
};

struct npc_nightmare_suppressorAI : public ScriptedAI
{
	npc_nightmare_suppressorAI(Creature* pCreature) : ScriptedAI(pCreature)
	{
		Reset();
		m_uiYellTimer = 2 * IN_MILLISECONDS;
	}

	uint32 m_uiChannelingTimer;
	uint32 m_uiYellTimer;
	bool m_bEvent;

	void Reset() override
	{
		m_bEvent = false;
		if (instance_sunken_temple* pInstance = (instance_sunken_temple*)m_creature->GetInstanceData())
			if (pInstance->GetData(TYPE_AVATAR) == IN_PROGRESS || pInstance->GetData(TYPE_AVATAR) == SPECIAL)
				m_bEvent = true;
		if (m_bEvent)
		{
			m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
			m_creature->SetWalk(false);
			m_uiChannelingTimer = 10 * IN_MILLISECONDS;
		}
	}

	void UpdateAI(const uint32 uiDiff) override
	{
		if (m_uiYellTimer)
		{
			if (m_uiYellTimer <= uiDiff)
			{
				m_uiYellTimer = 0;
				if (m_bEvent)
					DoScriptText(SUPRESSOR_YELLS[urand(0, 2)], m_creature);
			}
			else
				m_uiYellTimer -= uiDiff;
		}

		if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
		{
			if (!m_bEvent)
				return;
			if (m_creature->GetCurrentSpell(CURRENT_CHANNELED_SPELL)
				&& m_creature->GetCurrentSpell(CURRENT_CHANNELED_SPELL)->m_spellInfo->Id == 12623)
			{
				if (m_uiChannelingTimer <= uiDiff)
				{
					if (instance_sunken_temple* pInstance = (instance_sunken_temple*)m_creature->GetInstanceData())
						pInstance->SetData(TYPE_AVATAR, FAIL);
					Reset();
				}
				else
					m_uiChannelingTimer -= uiDiff;
			}
			else if (m_creature->GetMotionMaster()->GetCurrentMovementGeneratorType() == IDLE_MOTION_TYPE)
			{
				if (instance_sunken_temple* pInstance = (instance_sunken_temple*)m_creature->GetInstanceData())
					if (Creature* pShade = pInstance->GetSingleCreatureFromStorage(NPC_SHADE_OF_HAKKAR))
					{
						float x, y, z;
						pShade->GetNearPoint(m_creature, x, y, z, m_creature->GetObjectBoundingRadius(), 10.f, pShade->GetAngle(m_creature));
						m_creature->GetMotionMaster()->MovePoint(0, x, y, z);
					}
			}
			return;
		}

		DoMeleeAttackIfReady();
	}

	void MovementInform(uint32, uint32) override
	{
		if (instance_sunken_temple* pInstance = (instance_sunken_temple*)m_creature->GetInstanceData())
			if (Creature* pShade = pInstance->GetSingleCreatureFromStorage(NPC_SHADE_OF_HAKKAR))
				DoCastSpellIfCan(pShade, 12623, CAST_TRIGGERED);
	}
};

CreatureAI* GetAI_npc_nightmare_suppressor(Creature* pCreature)
{
	return new npc_nightmare_suppressorAI(pCreature);
}

/*######
## event_antalarion_statues
######*/

bool ProcessEventId_event_antalarion_statue_activation(uint32 uiEventId, Object* pSource, Object* pTarget, bool /*bIsStart*/)
{
	if (pSource->GetTypeId() == TYPEID_PLAYER && pTarget->GetTypeId() == TYPEID_GAMEOBJECT)
	{
		if (instance_sunken_temple* pInstance = (instance_sunken_temple*)((Player*)pSource)->GetInstanceData())
		{
			// return if event completed
			if (pInstance->GetData(TYPE_ATALARION) != NOT_STARTED)
				return true;

			// Send the event id to process
			if (pInstance->ProcessStatueEvent(uiEventId))
			{
				// Activate the green light if the correct statue is activated
				if (GameObject* pLight = GetClosestGameObjectWithEntry((GameObject*)pTarget, GO_ATALAI_LIGHT, INTERACTION_DISTANCE))
					pInstance->DoRespawnGameObject(pLight->GetObjectGuid(), 30 * MINUTE);
			}
			else
			{
				// If the wrong statue was activated, then trigger trap
				// We don't know actually which trap goes to which statue so we need to search for each
				if (GameObject* pTrap = GetClosestGameObjectWithEntry((GameObject*)pTarget, GO_ATALAI_TRAP_1, INTERACTION_DISTANCE))
					pTrap->Use((Unit*)pSource);
				else if (GameObject* pTrap = GetClosestGameObjectWithEntry((GameObject*)pTarget, GO_ATALAI_TRAP_2, INTERACTION_DISTANCE))
					pTrap->Use((Unit*)pSource);
				else if (GameObject* pTrap = GetClosestGameObjectWithEntry((GameObject*)pTarget, GO_ATALAI_TRAP_3, INTERACTION_DISTANCE))
					pTrap->Use((Unit*)pSource);
			}

			return true;
		}
	}
	return false;
}

/*######
## event_avatar_of_hakkar
######*/
bool ProcessEventId_event_avatar_of_hakkar(uint32 /*uiEventId*/, Object* pSource, Object* /*pTarget*/, bool /*bIsStart*/)
{
	if (pSource->GetTypeId() == TYPEID_PLAYER)
	{
		if (instance_sunken_temple* pInstance = (instance_sunken_temple*)((Player*)pSource)->GetInstanceData())
		{
			// return if not NOT_STARTED
			if (pInstance->GetData(TYPE_AVATAR) != NOT_STARTED)
				return true;

			pInstance->SetData(TYPE_AVATAR, IN_PROGRESS);

			return true;
		}
	}
	return false;
}

/*######
## go_eternal_flame
######*/
bool GOUse_go_eternal_flame(Player* /*pPlayer*/, GameObject* pGo)
{
	instance_sunken_temple* pInstance = (instance_sunken_temple*)pGo->GetInstanceData();

	if (!pInstance)
		return false;

	if (pInstance->GetData(TYPE_AVATAR) != IN_PROGRESS)
		return false;

	// Set data to special when flame is used
	pInstance->SetData(TYPE_AVATAR, SPECIAL);
	pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);

	return true;
}

enum
{
	SPELL_CAUSE_INSANITY = 12888,
	SPELL_CURSE_OF_TONGUES = 12889,
	SPELL_LASH = 6607
};

/*######
## npc_shade_of_hakkarAI
######*/
struct npc_shade_of_hakkarAI : public ScriptedAI
{
	npc_shade_of_hakkarAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

	uint32 m_uiInsanityTimer;
	uint32 m_uiCurseTimer;
	uint32 m_uiLashTimer;

	void Reset() override
	{
		m_uiInsanityTimer = 15 * IN_MILLISECONDS;
		m_uiCurseTimer = 10 * IN_MILLISECONDS;
		m_uiLashTimer = urand(3, 9) * IN_MILLISECONDS;
	}

	void UpdateAI(const uint32 uiDiff) override
	{
		if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
			return;

		if (m_uiInsanityTimer < uiDiff)
		{
			if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
				if (DoCastSpellIfCan(pTarget, SPELL_CAUSE_INSANITY) == CAST_OK)
					m_uiInsanityTimer = urand(13000, 17000);
		}
		else
			m_uiInsanityTimer -= uiDiff;

		if (m_uiCurseTimer < uiDiff)
		{
			if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, nullptr, SELECT_FLAG_POWER_MANA))
				if (DoCastSpellIfCan(pTarget, SPELL_CURSE_OF_TONGUES) == CAST_OK)
					m_uiCurseTimer = 16000;
		}
		else
			m_uiCurseTimer -= uiDiff;

		if (m_uiLashTimer < uiDiff)
		{
			if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0))
				if (DoCastSpellIfCan(pTarget, SPELL_LASH) == CAST_OK)
					m_uiLashTimer = urand(5000, 8000);
		}
		else
			m_uiLashTimer -= uiDiff;

		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_shade_of_hakkar(Creature* pCreature)
{
	return new npc_shade_of_hakkarAI(pCreature);
}

//enum
//{
//	SPELL_EARTHGRAB_TOTEM = 8376,
//	SPELL_FLAMESTRIKE = 12468,
//	SPELL_HEALING_WAVE = 12492,
//	SPELL_SHADOW_WORD_PAIN = 10893,
//	SPELL_HEX = 12479,
//
//	YELL_AGGRO = -1999950,
//	YELL_HEX = -1999951,
//	YELL_LOW_HEALTH = -1999952
//};
//
//struct boss_jammalan_the_prophetAI : public ScriptedAI
//{
//	boss_jammalan_the_prophetAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }
//
//	bool m_bLowHealth;
//	uint32 m_uiTotemTimer;
//	uint32 m_uiHexTimer;
//	uint32 m_uiFlamestrikeTimer;
//	uint32 m_uiHealWaveTimer;
//	uint32 m_uiShadowWordPainTimer;
//
//	void Aggro(Unit*) override
//	{
//		DoScriptText(YELL_AGGRO, m_creature);
//	}
//
//	void KilledUnit(Unit* pVictim) override
//	{
//		m_creature->PlayDirectSound(5863);
//	}
//
//	void JustDied(Unit* pKiller) override
//	{
//
//		Player* pPlayer = pKiller->GetCharmerOrOwnerPlayerOrPlayerItself();
//		if (!pPlayer)
//			return;
//
//		if (Creature* pSummon = pPlayer->SummonCreature(5720, aSunkenTempleLocation[3].m_fX, aSunkenTempleLocation[3].m_fY, aSunkenTempleLocation[3].m_fZ, aSunkenTempleLocation[3].m_fO, TEMPSUMMON_DEAD_DESPAWN, 0))
//			pSummon->GetMotionMaster()->MoveRandomAroundPoint(aSunkenTempleLocation[3].m_fX, aSunkenTempleLocation[3].m_fY, aSunkenTempleLocation[3].m_fZ, 5.f);
//
//		if (Creature* pSummon = pPlayer->SummonCreature(5721, aSunkenTempleLocation[4].m_fX, aSunkenTempleLocation[4].m_fY, aSunkenTempleLocation[4].m_fZ, aSunkenTempleLocation[4].m_fO, TEMPSUMMON_DEAD_DESPAWN, 0))
//			pSummon->GetMotionMaster()->MoveRandomAroundPoint(aSunkenTempleLocation[4].m_fX, aSunkenTempleLocation[4].m_fY, aSunkenTempleLocation[4].m_fZ, 5.f);
//	}
//
//	void Reset() override
//	{
//		m_bLowHealth = false;
//		m_uiTotemTimer = 3 * IN_MILLISECONDS;
//		m_uiHexTimer = 10 * IN_MILLISECONDS;
//		m_uiFlamestrikeTimer = urand(5, 15) * IN_MILLISECONDS;
//		m_uiHealWaveTimer = 15 * IN_MILLISECONDS;
//		m_uiShadowWordPainTimer = urand(5, 15) * IN_MILLISECONDS;
//	}
//
//	void UpdateAI(const uint32 uiDiff) override
//	{
//		if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
//			return;
//
//		if (!m_bLowHealth && m_creature->GetHealthPercent() < 30.f)
//		{
//			DoScriptText(YELL_LOW_HEALTH, m_creature);
//			m_bLowHealth = true;
//		}
//
//		if (m_uiTotemTimer < uiDiff)
//		{
//			if (DoCastSpellIfCan(m_creature, SPELL_EARTHGRAB_TOTEM))
//				m_uiTotemTimer = urand(15000, 17000);
//		}
//		else
//			m_uiTotemTimer -= uiDiff;
//
//		if (m_uiHexTimer < uiDiff)
//		{
//			if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
//				if (DoCastSpellIfCan(pTarget, SPELL_HEX) == CAST_OK)
//				{
//					DoScriptText(YELL_HEX, m_creature);
//					m_uiHexTimer = 20000;
//				}
//		}
//		else
//			m_uiHexTimer -= uiDiff;
//
//		if (m_uiShadowWordPainTimer < uiDiff)
//		{
//			if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
//				if (DoCastSpellIfCan(pTarget, SPELL_SHADOW_WORD_PAIN) == CAST_OK)
//					m_uiShadowWordPainTimer = urand(5, 15) * IN_MILLISECONDS;
//		}
//		else
//			m_uiShadowWordPainTimer -= uiDiff;
//
//		if (m_uiFlamestrikeTimer < uiDiff)
//		{
//			if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
//				if (DoCastSpellIfCan(pTarget, SPELL_FLAMESTRIKE) == CAST_OK)
//					m_uiFlamestrikeTimer = urand(5, 15) * IN_MILLISECONDS;
//		}
//		else
//			m_uiFlamestrikeTimer -= uiDiff;
//
//		if (m_uiHealWaveTimer < uiDiff)
//		{
//			if (Unit* pTarget = DoSelectLowestHpFriendly(20.f, 100))
//				if (DoCastSpellIfCan(pTarget, SPELL_HEALING_WAVE) == CAST_OK)
//					m_uiHealWaveTimer = urand(7000, 8000);
//		}
//		else
//			m_uiHealWaveTimer -= uiDiff;
//
//		DoMeleeAttackIfReady();
//	}
//};
//
//CreatureAI* GetAI_boss_jammalan_the_prophet(Creature* pCreature)
//{
//	return new boss_jammalan_the_prophetAI(pCreature);
//}
//
//


/*######
## effectDummy_summon_hakkar
######*/
bool EffectDummyCreature_summon_hakkar(Unit* pCaster, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Creature* /*pCreatureTarget*/, ObjectGuid /*originalCasterGuid*/)
{
	// Always check spellid and effectindex
	if (uiSpellId == SPELL_SUMMON_AVATAR && uiEffIndex == EFFECT_INDEX_0)
	{
		if (!pCaster || pCaster->GetTypeId() != TYPEID_UNIT)
			return true;

		// Update entry to avatar of Hakkar and cast some visuals
		((Creature*)pCaster)->UpdateEntry(NPC_AVATAR_OF_HAKKAR);
		pCaster->CastSpell(pCaster, SPELL_AVATAR_SUMMONED, true);
		DoScriptText(SAY_AVATAR_SPAWN, pCaster);

		// Always return true when we are handling this spell and effect
		return true;
	}

	return false;
}

void AddSC_sunken_temple()
{
	Script* pNewScript;

	pNewScript = new Script;
	pNewScript->Name = "at_shade_of_eranikus";
	pNewScript->pAreaTrigger = &AreaTrigger_at_shade_of_eranikus;
	pNewScript->RegisterSelf();

	pNewScript = new Script;
	pNewScript->Name = "npc_malfurion_stormrage";
	pNewScript->GetAI = &GetAI_npc_malfurion;
	pNewScript->RegisterSelf();

	pNewScript = new Script;
	pNewScript->Name = "event_antalarion_statue_activation";
	pNewScript->pProcessEventId = &ProcessEventId_event_antalarion_statue_activation;
	pNewScript->RegisterSelf();

	pNewScript = new Script;
	pNewScript->Name = "event_avatar_of_hakkar";
	pNewScript->pProcessEventId = &ProcessEventId_event_avatar_of_hakkar;
	pNewScript->RegisterSelf();

	pNewScript = new Script;
	pNewScript->Name = "go_eternal_flame";
	pNewScript->pGOUse = &GOUse_go_eternal_flame;
	pNewScript->RegisterSelf();

	pNewScript = new Script;
	pNewScript->Name = "npc_shade_of_hakkar";
	pNewScript->GetAI = &GetAI_npc_shade_of_hakkar;
	pNewScript->RegisterSelf();

	pNewScript = new Script;
	pNewScript->Name = "npc_nightmare_suppressor";
	pNewScript->GetAI = &GetAI_npc_nightmare_suppressor;
	pNewScript->RegisterSelf();

}
