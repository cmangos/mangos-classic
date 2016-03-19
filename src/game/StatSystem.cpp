/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
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

#include "Unit.h"
#include "Player.h"
#include "Pet.h"
#include "Creature.h"
#include "SharedDefines.h"
#include "SpellAuras.h"

/*#######################################
########                         ########
########   PLAYERS STAT SYSTEM   ########
########                         ########
#######################################*/

bool Player::UpdateStats(Stats stat)
{
    if (stat > STAT_SPIRIT)
        return false;

    // value = ((base_value * base_pct) + total_value) * total_pct
    float value  = GetTotalStatValue(stat);

    SetStat(stat, int32(value));

    switch (stat)
    {
        case STAT_STRENGTH:
            break;
        case STAT_AGILITY:
            UpdateArmor();
            UpdateAllCritPercentages();
            UpdateDodgePercentage();
            break;
        case STAT_STAMINA:   UpdateMaxHealth(); break;
        case STAT_INTELLECT:
            UpdateMaxPower(POWER_MANA);
            UpdateAllSpellCritChances();
            UpdateArmor();                                  // SPELL_AURA_MOD_RESISTANCE_OF_INTELLECT_PERCENT, only armor currently
            break;

        case STAT_SPIRIT:
            break;

        default:
            break;
    }
    // Need update (exist AP from stat auras)
    UpdateAttackPowerAndDamage();
    UpdateAttackPowerAndDamage(true);

    UpdateSpellDamageAndHealingBonus();
    UpdateManaRegen();

    return true;
}

void Player::UpdateSpellDamageAndHealingBonus()
{
    // Magic damage modifiers implemented in Unit::SpellDamageBonusDone
    // This information for client side use only
    // Get healing bonus for all schools
    // Get damage bonus for all schools
    for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
        SetStatInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + i, SpellBaseDamageBonusDone(GetSchoolMask(i)));
}

bool Player::UpdateAllStats()
{
    for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
    {
        float value = GetTotalStatValue(Stats(i));
        SetStat(Stats(i), (int32)value);
    }

    UpdateAttackPowerAndDamage();
    UpdateAttackPowerAndDamage(true);
    UpdateArmor();
    UpdateMaxHealth();

    for (int i = POWER_MANA; i < MAX_POWERS; ++i)
        UpdateMaxPower(Powers(i));

    UpdateAllCritPercentages();
    UpdateAllSpellCritChances();
    UpdateDefenseBonusesMod();
    UpdateSpellDamageAndHealingBonus();
    UpdateManaRegen();
    for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
        UpdateResistances(i);

    return true;
}

void Player::UpdateResistances(uint32 school)
{
    if (school > SPELL_SCHOOL_NORMAL)
    {
        float value  = GetTotalAuraModValue(UnitMods(UNIT_MOD_RESISTANCE_START + school));
        SetResistance(SpellSchools(school), int32(value));
    }
    else
        UpdateArmor();
}

void Player::UpdateArmor()
{
    float value;
    UnitMods unitMod = UNIT_MOD_ARMOR;

    value  = GetModifierValue(unitMod, BASE_VALUE);         // base armor (from items)
    value *= GetModifierValue(unitMod, BASE_PCT);           // armor percent from items
    value += GetStat(STAT_AGILITY) * 2.0f;                  // armor bonus from stats
    value += GetModifierValue(unitMod, TOTAL_VALUE);

    // add dynamic flat mods
    AuraList const& mResbyIntellect = GetAurasByType(SPELL_AURA_MOD_RESISTANCE_OF_STAT_PERCENT);
    for (AuraList::const_iterator i = mResbyIntellect.begin(); i != mResbyIntellect.end(); ++i)
    {
        Modifier* mod = (*i)->GetModifier();
        if (mod->m_miscvalue & SPELL_SCHOOL_MASK_NORMAL)
            value += int32(GetStat(STAT_INTELLECT) * mod->m_amount / 100.0f);
    }

    value *= GetModifierValue(unitMod, TOTAL_PCT);

    SetArmor(int32(value));
}

float Player::GetHealthBonusFromStamina()
{
    float stamina = GetStat(STAT_STAMINA);

    float baseStam = stamina < 20 ? stamina : 20;
    float moreStam = stamina - baseStam;

    return baseStam + (moreStam * 10.0f);
}

float Player::GetManaBonusFromIntellect()
{
    float intellect = GetStat(STAT_INTELLECT);

    float baseInt = intellect < 20 ? intellect : 20;
    float moreInt = intellect - baseInt;

    return baseInt + (moreInt * 15.0f);
}

void Player::UpdateMaxHealth()
{
    UnitMods unitMod = UNIT_MOD_HEALTH;

    float value = GetModifierValue(unitMod, BASE_VALUE) + GetCreateHealth();
    value *= GetModifierValue(unitMod, BASE_PCT);
    value += GetModifierValue(unitMod, TOTAL_VALUE) + GetHealthBonusFromStamina();
    value *= GetModifierValue(unitMod, TOTAL_PCT);

    SetMaxHealth((uint32)value);
}

void Player::UpdateMaxPower(Powers power)
{
    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + power);

    uint32 create_power = GetCreatePowers(power);

    // ignore classes without mana
    float bonusPower = (power == POWER_MANA && create_power > 0) ? GetManaBonusFromIntellect() : 0;

    float value = GetModifierValue(unitMod, BASE_VALUE) + create_power;
    value *= GetModifierValue(unitMod, BASE_PCT);
    value += GetModifierValue(unitMod, TOTAL_VALUE) +  bonusPower;
    value *= GetModifierValue(unitMod, TOTAL_PCT);

    SetMaxPower(power, uint32(value));
}

void Player::UpdateAttackPowerAndDamage(bool ranged)
{
    float val2 = 0.0f;
    float level = float(getLevel());

    UnitMods unitMod = ranged ? UNIT_MOD_ATTACK_POWER_RANGED : UNIT_MOD_ATTACK_POWER;

    uint16 index = UNIT_FIELD_ATTACK_POWER;
    uint16 index_mod = UNIT_FIELD_ATTACK_POWER_MODS;
    uint16 index_mult = UNIT_FIELD_ATTACK_POWER_MULTIPLIER;

    if (ranged)
    {
        index = UNIT_FIELD_RANGED_ATTACK_POWER;
        index_mod = UNIT_FIELD_RANGED_ATTACK_POWER_MODS;
        index_mult = UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER;

        switch (getClass())
        {
            case CLASS_HUNTER: val2 = level * 2.0f + GetStat(STAT_AGILITY) * 2.0f - 10.0f;    break;
            case CLASS_ROGUE:  val2 = level        + GetStat(STAT_AGILITY) - 10.0f;    break;
            case CLASS_WARRIOR: val2 = level        + GetStat(STAT_AGILITY) - 10.0f;    break;
            case CLASS_DRUID:
                switch (GetShapeshiftForm())
                {
                    case FORM_CAT:
                    case FORM_BEAR:
                    case FORM_DIREBEAR:
                        val2 = 0.0f; break;
                    default:
                        val2 = GetStat(STAT_AGILITY) - 10.0f; break;
                }
                break;
            default: val2 = GetStat(STAT_AGILITY) - 10.0f; break;
        }
    }
    else
    {
        switch (getClass())
        {
            case CLASS_WARRIOR:      val2 = level * 3.0f + GetStat(STAT_STRENGTH) * 2.0f                    - 20.0f; break;
            case CLASS_PALADIN:      val2 = level * 3.0f + GetStat(STAT_STRENGTH) * 2.0f                    - 20.0f; break;
            case CLASS_ROGUE:        val2 = level * 2.0f + GetStat(STAT_STRENGTH) + GetStat(STAT_AGILITY) - 20.0f; break;
            case CLASS_HUNTER:       val2 = level * 2.0f + GetStat(STAT_STRENGTH) + GetStat(STAT_AGILITY) - 20.0f; break;
            case CLASS_SHAMAN:       val2 = level * 2.0f + GetStat(STAT_STRENGTH) * 2.0f                    - 20.0f; break;
            case CLASS_DRUID:
            {
                ShapeshiftForm form = GetShapeshiftForm();
                // Check if Predatory Strikes is skilled
                float mLevelMult = 0.0;
                switch (form)
                {
                    case FORM_CAT:
                    case FORM_BEAR:
                    case FORM_DIREBEAR:
                    case FORM_MOONKIN:
                    {
                        Unit::AuraList const& mDummy = GetAurasByType(SPELL_AURA_DUMMY);
                        for (Unit::AuraList::const_iterator itr = mDummy.begin(); itr != mDummy.end(); ++itr)
                        {
                            // Predatory Strikes
                            if ((*itr)->GetSpellProto()->SpellIconID == 1563)
                            {
                                mLevelMult = (*itr)->GetModifier()->m_amount / 100.0f;
                                break;
                            }
                        }
                        break;
                    }
                    default: break;
                }

                switch (form)
                {
                    case FORM_CAT:
                        val2 = getLevel() * (mLevelMult + 2.0f) + GetStat(STAT_STRENGTH) * 2.0f + GetStat(STAT_AGILITY) - 20.0f; break;
                    case FORM_BEAR:
                    case FORM_DIREBEAR:
                        val2 = getLevel() * (mLevelMult + 3.0f) + GetStat(STAT_STRENGTH) * 2.0f - 20.0f; break;
                    case FORM_MOONKIN:
                        val2 = getLevel() * (mLevelMult + 1.5f) + GetStat(STAT_STRENGTH) * 2.0f - 20.0f; break;
                    default:
                        val2 = GetStat(STAT_STRENGTH) * 2.0f - 20.0f; break;
                }
                break;
            }
            case CLASS_MAGE:    val2 =              GetStat(STAT_STRENGTH)                         - 10.0f; break;
            case CLASS_PRIEST:  val2 =              GetStat(STAT_STRENGTH)                         - 10.0f; break;
            case CLASS_WARLOCK: val2 =              GetStat(STAT_STRENGTH)                         - 10.0f; break;
        }
    }

    SetModifierValue(unitMod, BASE_VALUE, val2);

    float base_attPower  = GetModifierValue(unitMod, BASE_VALUE) * GetModifierValue(unitMod, BASE_PCT);
    float attPowerMod = GetModifierValue(unitMod, TOTAL_VALUE);

    float attPowerMultiplier = GetModifierValue(unitMod, TOTAL_PCT) - 1.0f;

    SetInt32Value(index, (uint32)base_attPower);            // UNIT_FIELD_(RANGED)_ATTACK_POWER field
    SetInt32Value(index_mod, (uint32)attPowerMod);          // UNIT_FIELD_(RANGED)_ATTACK_POWER_MODS field
    SetFloatValue(index_mult, attPowerMultiplier);          // UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER field

    // automatically update weapon damage after attack power modification
    if (ranged)
        UpdateDamagePhysical(RANGED_ATTACK);
    else
    {
        UpdateDamagePhysical(BASE_ATTACK);
        if (CanDualWield() && haveOffhandWeapon())          // allow update offhand damage only if player knows DualWield Spec and has equipped offhand weapon
            UpdateDamagePhysical(OFF_ATTACK);
    }
}

void Player::CalculateMinMaxDamage(WeaponAttackType attType, bool normalized, float& min_damage, float& max_damage)
{
    UnitMods unitMod;

    switch (attType)
    {
        case BASE_ATTACK:
        default:
            unitMod = UNIT_MOD_DAMAGE_MAINHAND;
            break;
        case OFF_ATTACK:
            unitMod = UNIT_MOD_DAMAGE_OFFHAND;
            break;
        case RANGED_ATTACK:
            unitMod = UNIT_MOD_DAMAGE_RANGED;
            break;
    }

    float att_speed = GetAPMultiplier(attType, normalized);

    float base_value  = GetModifierValue(unitMod, BASE_VALUE) + GetTotalAttackPowerValue(attType) / 14.0f * att_speed;
    float base_pct    = GetModifierValue(unitMod, BASE_PCT);
    float total_value = GetModifierValue(unitMod, TOTAL_VALUE);
    float total_pct   = GetModifierValue(unitMod, TOTAL_PCT);

    float weapon_mindamage = GetWeaponDamageRange(attType, MINDAMAGE);
    float weapon_maxdamage = GetWeaponDamageRange(attType, MAXDAMAGE);

    if (IsInFeralForm())                                    // check if player is druid and in cat or bear forms, non main hand attacks not allowed for this mode so not check attack type
    {
        uint32 lvl = getLevel();
        if (lvl > 60)
            lvl = 60;

        weapon_mindamage = lvl * 0.85f * att_speed;
        weapon_maxdamage = lvl * 1.25f * att_speed;
    }
    else if (!CanUseEquippedWeapon(attType))                // check if player not in form but still can't use weapon (broken/etc)
    {
        weapon_mindamage = BASE_MINDAMAGE;
        weapon_maxdamage = BASE_MAXDAMAGE;
    }
    else if (attType == RANGED_ATTACK)                      // add ammo DPS to ranged damage
    {
        weapon_mindamage += GetAmmoDPS() * att_speed;
        weapon_maxdamage += GetAmmoDPS() * att_speed;
    }

    min_damage = ((base_value + weapon_mindamage) * base_pct + total_value) * total_pct;
    max_damage = ((base_value + weapon_maxdamage) * base_pct + total_value) * total_pct;
}

void Player::UpdateDamagePhysical(WeaponAttackType attType)
{
    float mindamage;
    float maxdamage;

    CalculateMinMaxDamage(attType, false, mindamage, maxdamage);

    switch (attType)
    {
        case BASE_ATTACK:
        default:
            SetStatFloatValue(UNIT_FIELD_MINDAMAGE, mindamage);
            SetStatFloatValue(UNIT_FIELD_MAXDAMAGE, maxdamage);
            break;
        case OFF_ATTACK:
            SetStatFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, mindamage);
            SetStatFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, maxdamage);
            break;
        case RANGED_ATTACK:
            SetStatFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, mindamage);
            SetStatFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, maxdamage);
            break;
    }
}

void Player::UpdateDefenseBonusesMod()
{
    UpdateBlockPercentage();
    UpdateParryPercentage();
    UpdateDodgePercentage();
}

void Player::UpdateBlockPercentage()
{
    // No block
    float value = 0.0f;
    if (CanBlock())
    {
        // Base value
        value = 5.0f;
        // Modify value from defense skill
        value += (int32(GetDefenseSkillValue()) - int32(GetMaxSkillValueForLevel())) * 0.04f;
        // Increase from SPELL_AURA_MOD_BLOCK_PERCENT aura
        value += GetTotalAuraModifier(SPELL_AURA_MOD_BLOCK_PERCENT);
        value = value < 0.0f ? 0.0f : value;
    }
    SetStatFloatValue(PLAYER_BLOCK_PERCENTAGE, value);
}

void Player::UpdateCritPercentage(WeaponAttackType attType)
{
    BaseModGroup modGroup;
    uint16 index;

    switch (attType)
    {
        case RANGED_ATTACK:
            modGroup = RANGED_CRIT_PERCENTAGE;
            index = PLAYER_RANGED_CRIT_PERCENTAGE;
            break;
        case BASE_ATTACK:
            modGroup = CRIT_PERCENTAGE;
            index = PLAYER_CRIT_PERCENTAGE;
            break;
        case OFF_ATTACK:                                    // client have only main hand crit
        default:
            return;
    }

    float value = GetTotalPercentageModValue(modGroup);
    // Modify crit from weapon skill and maximized defense skill of same level victim difference
    value += (int32(GetWeaponSkillValue(attType)) - int32(GetMaxSkillValueForLevel())) * 0.04f;
    value = value < 0.0f ? 0.0f : value;
    SetStatFloatValue(index, value);
}

void Player::UpdateAllCritPercentages()
{
    float value = GetMeleeCritFromAgility();

    SetBaseModValue(CRIT_PERCENTAGE, PCT_MOD, value);
    SetBaseModValue(OFFHAND_CRIT_PERCENTAGE, PCT_MOD, value);
    SetBaseModValue(RANGED_CRIT_PERCENTAGE, PCT_MOD, value);

    UpdateCritPercentage(BASE_ATTACK);
    UpdateCritPercentage(OFF_ATTACK);
    UpdateCritPercentage(RANGED_ATTACK);
}

void Player::UpdateParryPercentage()
{
    // No parry
    float value = 0.0f;
    if (CanParry())
    {
        // Base parry
        value  = 5.0f;
        // Modify value from defense skill
        value += (int32(GetDefenseSkillValue()) - int32(GetMaxSkillValueForLevel())) * 0.04f;
        // Parry from SPELL_AURA_MOD_PARRY_PERCENT aura
        value += GetTotalAuraModifier(SPELL_AURA_MOD_PARRY_PERCENT);
        value = value < 0.0f ? 0.0f : value;
    }
    SetStatFloatValue(PLAYER_PARRY_PERCENTAGE, value);
}

void Player::UpdateDodgePercentage()
{
    // Dodge from agility
    float value = GetDodgeFromAgility();
    // Modify value from defense skill
    value += (int32(GetDefenseSkillValue()) - int32(GetMaxSkillValueForLevel())) * 0.04f;
    // Dodge from SPELL_AURA_MOD_DODGE_PERCENT aura
    value += GetTotalAuraModifier(SPELL_AURA_MOD_DODGE_PERCENT);
    value = value < 0.0f ? 0.0f : value;
    SetStatFloatValue(PLAYER_DODGE_PERCENTAGE, value);
}

void Player::UpdateSpellCritChance(uint32 school)
{
    // For normal school set zero crit chance
    if (school == SPELL_SCHOOL_NORMAL)
    {
        m_SpellCritPercentage[1] = 0.0f;
        return;
    }
    // For others recalculate it from:
    float crit = 0.0f;
    // Crit from Intellect
    crit += GetSpellCritFromIntellect();
    // Increase crit from SPELL_AURA_MOD_SPELL_CRIT_CHANCE
    crit += GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_CRIT_CHANCE);
    // Increase crit by school from SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL
    crit += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL, 1 << school);

    // Store crit value
    m_SpellCritPercentage[school] = crit;
}

void Player::UpdateAllSpellCritChances()
{
    for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
        UpdateSpellCritChance(i);
}

void Player::UpdateManaRegen()
{
    // Mana regen from spirit
    float power_regen = OCTRegenMPPerSpirit();
    // Apply PCT bonus from SPELL_AURA_MOD_POWER_REGEN_PERCENT aura on spirit base regen
    power_regen *= GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_POWER_REGEN_PERCENT, POWER_MANA);

    // Mana regen from SPELL_AURA_MOD_POWER_REGEN aura
    float power_regen_mp5 = GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_POWER_REGEN, POWER_MANA) / 5.0f;

    // Set regen rate in cast state apply only on spirit based regen
    int32 modManaRegenInterrupt = GetTotalAuraModifier(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT);
    if (modManaRegenInterrupt > 100)
        modManaRegenInterrupt = 100;

    m_modManaRegenInterrupt = power_regen_mp5 + power_regen * modManaRegenInterrupt / 100.0f;

    m_modManaRegen = power_regen_mp5 + power_regen;
}

void Player::_ApplyAllStatBonuses()
{
    SetCanModifyStats(false);

    _ApplyAllAuraMods();
    _ApplyAllItemMods();

    SetCanModifyStats(true);

    UpdateAllStats();
}

void Player::_RemoveAllStatBonuses()
{
    SetCanModifyStats(false);

    _RemoveAllItemMods();
    _RemoveAllAuraMods();

    SetCanModifyStats(true);

    UpdateAllStats();
}

/*#######################################
########                         ########
########    MOBS STAT SYSTEM     ########
########                         ########
#######################################*/

bool Creature::UpdateStats(Stats /*stat*/)
{
    return true;
}

bool Creature::UpdateAllStats()
{
    UpdateMaxHealth();
    UpdateAttackPowerAndDamage();

    for (int i = POWER_MANA; i < MAX_POWERS; ++i)
        UpdateMaxPower(Powers(i));

    for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
        UpdateResistances(i);

    return true;
}

void Creature::UpdateResistances(uint32 school)
{
    if (school > SPELL_SCHOOL_NORMAL)
    {
        float value  = GetTotalAuraModValue(UnitMods(UNIT_MOD_RESISTANCE_START + school));
        SetResistance(SpellSchools(school), int32(value));
    }
    else
        UpdateArmor();
}

void Creature::UpdateArmor()
{
    float value = GetTotalAuraModValue(UNIT_MOD_ARMOR);
    SetArmor(int32(value));
}

void Creature::UpdateMaxHealth()
{
    float value = GetTotalAuraModValue(UNIT_MOD_HEALTH);
    SetMaxHealth((uint32)value);
}

void Creature::UpdateMaxPower(Powers power)
{
    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + power);

    float value  = GetTotalAuraModValue(unitMod);
    SetMaxPower(power, uint32(value));
}

void Creature::UpdateAttackPowerAndDamage(bool ranged)
{
    UnitMods unitMod = ranged ? UNIT_MOD_ATTACK_POWER_RANGED : UNIT_MOD_ATTACK_POWER;

    uint16 index = UNIT_FIELD_ATTACK_POWER;
    uint16 index_mod = UNIT_FIELD_ATTACK_POWER_MODS;
    uint16 index_mult = UNIT_FIELD_ATTACK_POWER_MULTIPLIER;

    if (ranged)
    {
        index = UNIT_FIELD_RANGED_ATTACK_POWER;
        index_mod = UNIT_FIELD_RANGED_ATTACK_POWER_MODS;
        index_mult = UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER;
    }

    float base_attPower  = GetModifierValue(unitMod, BASE_VALUE) * GetModifierValue(unitMod, BASE_PCT);
    float attPowerMod = GetModifierValue(unitMod, TOTAL_VALUE);
    float attPowerMultiplier = GetModifierValue(unitMod, TOTAL_PCT) - 1.0f;

    SetInt32Value(index, (uint32)base_attPower);            // UNIT_FIELD_(RANGED)_ATTACK_POWER field
    SetInt32Value(index_mod, (uint32)attPowerMod);          // UNIT_FIELD_(RANGED)_ATTACK_POWER_MODS field
    SetFloatValue(index_mult, attPowerMultiplier);          // UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER field

    if (ranged)
        return;

    // automatically update weapon damage after attack power modification
    UpdateDamagePhysical(BASE_ATTACK);
    UpdateDamagePhysical(OFF_ATTACK);
}

void Creature::UpdateDamagePhysical(WeaponAttackType attType)
{
    if (attType > OFF_ATTACK)
        return;

    UnitMods unitMod = (attType == BASE_ATTACK ? UNIT_MOD_DAMAGE_MAINHAND : UNIT_MOD_DAMAGE_OFFHAND);

    /* difference in AP between current attack power and base value from DB */
    float att_pwr_change = GetTotalAttackPowerValue(attType) - GetCreatureInfo()->MeleeAttackPower;
    float base_value  = GetModifierValue(unitMod, BASE_VALUE) + (att_pwr_change * GetAPMultiplier(attType, false) / 14.0f);
    float base_pct    = GetModifierValue(unitMod, BASE_PCT);
    float total_value = GetModifierValue(unitMod, TOTAL_VALUE);
    float total_pct   = GetModifierValue(unitMod, TOTAL_PCT);
    float dmg_multiplier = GetCreatureInfo()->DamageMultiplier;

    float weapon_mindamage = GetWeaponDamageRange(attType, MINDAMAGE);
    float weapon_maxdamage = GetWeaponDamageRange(attType, MAXDAMAGE);

    float mindamage = ((base_value + weapon_mindamage) * dmg_multiplier * base_pct + total_value) * total_pct;
    float maxdamage = ((base_value + weapon_maxdamage) * dmg_multiplier * base_pct + total_value) * total_pct;

    SetStatFloatValue(attType == BASE_ATTACK ? UNIT_FIELD_MINDAMAGE : UNIT_FIELD_MINOFFHANDDAMAGE, mindamage);
    SetStatFloatValue(attType == BASE_ATTACK ? UNIT_FIELD_MAXDAMAGE : UNIT_FIELD_MAXOFFHANDDAMAGE, maxdamage);
}

/*#######################################
########                         ########
########    PETS STAT SYSTEM     ########
########                         ########
#######################################*/

bool Pet::UpdateStats(Stats stat)
{
    if (stat > STAT_SPIRIT)
        return false;

    // value = ((base_value * base_pct) + total_value) * total_pct
    float value  = GetTotalStatValue(stat);
    SetStat(stat, int32(value));

    switch (stat)
    {
        case STAT_STRENGTH:         UpdateAttackPowerAndDamage();        break;
        case STAT_AGILITY:          UpdateArmor();                       break;
        case STAT_STAMINA:          UpdateMaxHealth();                   break;
        case STAT_INTELLECT:        UpdateMaxPower(POWER_MANA);          break;
        case STAT_SPIRIT:
        default:
            break;
    }

    return true;
}

bool Pet::UpdateAllStats()
{
    for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        UpdateStats(Stats(i));

    for (int i = POWER_MANA; i < MAX_POWERS; ++i)
        UpdateMaxPower(Powers(i));

    for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
        UpdateResistances(i);

    return true;
}

void Pet::UpdateResistances(uint32 school)
{
    if (school > SPELL_SCHOOL_NORMAL)
    {
        float value  = GetTotalAuraModValue(UnitMods(UNIT_MOD_RESISTANCE_START + school));
        SetResistance(SpellSchools(school), int32(value));
    }
    else
        UpdateArmor();
}

void Pet::UpdateArmor()
{
    float value = 0.0f;
    UnitMods unitMod = UNIT_MOD_ARMOR;

    value  = GetModifierValue(unitMod, BASE_VALUE);
    value *= GetModifierValue(unitMod, BASE_PCT);
    value += GetStat(STAT_AGILITY) * 2.0f;
    value += GetModifierValue(unitMod, TOTAL_VALUE);
    value *= GetModifierValue(unitMod, TOTAL_PCT);

    SetArmor(int32(value));
}

void Pet::UpdateMaxHealth()
{
    UnitMods unitMod = UNIT_MOD_HEALTH;
    float stamina = GetStat(STAT_STAMINA) - GetCreateStat(STAT_STAMINA);

    float value   = GetModifierValue(unitMod, BASE_VALUE) + GetCreateHealth();
    value  *= GetModifierValue(unitMod, BASE_PCT);
    value  += GetModifierValue(unitMod, TOTAL_VALUE) + stamina * 10.0f;
    value  *= GetModifierValue(unitMod, TOTAL_PCT);

    SetMaxHealth((uint32)value);
}

void Pet::UpdateMaxPower(Powers power)
{
    UnitMods unitMod = UnitMods(UNIT_MOD_POWER_START + power);

    float addValue = (power == POWER_MANA) ? GetStat(STAT_INTELLECT) - GetCreateStat(STAT_INTELLECT) : 0.0f;

    float value  = GetModifierValue(unitMod, BASE_VALUE) + GetCreatePowers(power);
    value *= GetModifierValue(unitMod, BASE_PCT);
    value += GetModifierValue(unitMod, TOTAL_VALUE) +  addValue * 15.0f;
    value *= GetModifierValue(unitMod, TOTAL_PCT);

    SetMaxPower(power, uint32(value));
}

void Pet::UpdateAttackPowerAndDamage(bool ranged)
{
    if (ranged)
        return;

    float val = 0.0f;
    UnitMods unitMod = UNIT_MOD_ATTACK_POWER;

    if (GetEntry() == 416)                                  // imp's attack power
        val = GetStat(STAT_STRENGTH) - 10.0f;
    else
        val = 2 * GetStat(STAT_STRENGTH) - 20.0f;

    SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, val);

    // in BASE_VALUE of UNIT_MOD_ATTACK_POWER for creatures we store data of meleeattackpower field in DB
    float base_attPower  = GetModifierValue(unitMod, BASE_VALUE) * GetModifierValue(unitMod, BASE_PCT);
    float attPowerMod = GetModifierValue(unitMod, TOTAL_VALUE);
    float attPowerMultiplier = GetModifierValue(unitMod, TOTAL_PCT) - 1.0f;

    // UNIT_FIELD_(RANGED)_ATTACK_POWER field
    SetInt32Value(UNIT_FIELD_ATTACK_POWER, (int32)base_attPower);
    // UNIT_FIELD_(RANGED)_ATTACK_POWER_MODS field
    SetInt32Value(UNIT_FIELD_ATTACK_POWER_MODS, (int32)attPowerMod);
    // UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER field
    SetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER, attPowerMultiplier);

    // automatically update weapon damage after attack power modification
    UpdateDamagePhysical(BASE_ATTACK);
}

void Pet::UpdateDamagePhysical(WeaponAttackType attType)
{
    if (attType > BASE_ATTACK)
        return;

    UnitMods unitMod = UNIT_MOD_DAMAGE_MAINHAND;

    float att_speed = float(GetAttackTime(BASE_ATTACK)) / 1000.0f;

    float base_value  = GetModifierValue(unitMod, BASE_VALUE) + GetTotalAttackPowerValue(attType) / 14.0f * att_speed;
    float base_pct    = GetModifierValue(unitMod, BASE_PCT);
    float total_value = GetModifierValue(unitMod, TOTAL_VALUE);
    float total_pct   = GetModifierValue(unitMod, TOTAL_PCT);

    float weapon_mindamage = GetWeaponDamageRange(BASE_ATTACK, MINDAMAGE);
    float weapon_maxdamage = GetWeaponDamageRange(BASE_ATTACK, MAXDAMAGE);

    float mindamage = ((base_value + weapon_mindamage) * base_pct + total_value) * total_pct;
    float maxdamage = ((base_value + weapon_maxdamage) * base_pct + total_value) * total_pct;

    //  Pet's base damage changes depending on happiness
    if (getPetType() == HUNTER_PET && attType == BASE_ATTACK)
    {
        switch (GetHappinessState())
        {
            case HAPPY:
                // 125% of normal damage
                mindamage = mindamage * 1.25f;
                maxdamage = maxdamage * 1.25f;
                break;
            case CONTENT:
                // 100% of normal damage, nothing to modify
                break;
            case UNHAPPY:
                // 75% of normal damage
                mindamage = mindamage * 0.75f;
                maxdamage = maxdamage * 0.75f;
                break;
        }
    }

    SetStatFloatValue(UNIT_FIELD_MINDAMAGE, mindamage);
    SetStatFloatValue(UNIT_FIELD_MAXDAMAGE, maxdamage);
}
