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

#ifndef MANGOS_FORMULAS_H
#define MANGOS_FORMULAS_H

#include "World/World.h"
// TODO: Required for classic honor system type HonorStandingList, needs redesign:
#include "Globals/ObjectMgr.h"

struct HonorScores
{
    float FX[15];
    float FY[15];
    float BRK[14];
};

namespace MaNGOS
{
    namespace Honor
    {
        inline float hk_honor_at_level(uint32 level, uint32 count = 1)
        {
            return (float)ceil(count * (-0.53177f + 0.59357f * exp((level + 23.54042f) / 26.07859f)));
        }

        // set passed rank info to default
        inline void InitRankInfo(HonorRankInfo& prk)
        {
            prk.positive = true;
            prk.rank = 0;
            prk.visualRank = 0;
            prk.maxRP = 2000.00f;
            prk.minRP = 0.00f;
        }

        inline HonorRankInfo CalculateRankInfo(HonorRankInfo prk)
        {
            if (prk.rank != 0)
            {
                int8 rank = prk.positive ? prk.rank - NEGATIVE_HONOR_RANK_COUNT - 1 : prk.rank - NEGATIVE_HONOR_RANK_COUNT;
                prk.maxRP = (rank) * 5000.00f;
                if (prk.maxRP < 0) // in negative rank case
                    prk.maxRP *= -1;
                prk.minRP = prk.maxRP > 5000.0f ? prk.maxRP  - 5000.00f : 2000.00f;

                prk.visualRank = prk.rank > NEGATIVE_HONOR_RANK_COUNT ? prk.rank - NEGATIVE_HONOR_RANK_COUNT : prk.rank * -1;
            }
            else
                InitRankInfo(prk);

            return prk;
        }

        // What is Player's rank... private, scout...
        inline HonorRankInfo CalculateHonorRank(float honor_points)
        {
            HonorRankInfo prk;
            InitRankInfo(prk);

            // rank none
            if (honor_points == 0)
                return prk;

            prk.positive = honor_points > 0;
            if (!prk.positive)
                honor_points *= -1;

            uint8 rCount = prk.positive ? POSITIVE_HONOR_RANK_COUNT - 2 : NEGATIVE_HONOR_RANK_COUNT;
            uint8 firstRank = prk.positive ? NEGATIVE_HONOR_RANK_COUNT + 1 : 1;

            if (honor_points < 2000.00f)
                prk.rank = prk.positive ? firstRank : NEGATIVE_HONOR_RANK_COUNT;
            else
            {
                if (honor_points > (rCount - 1) * 5000.00f)
                    prk.rank = prk.positive ? HONOR_RANK_COUNT - 1 : firstRank;
                else
                {
                    prk.rank = uint32(honor_points / 5000.00f) + firstRank;
                    prk.rank = (prk.positive ? prk.rank  + 1 : NEGATIVE_HONOR_RANK_COUNT - prk.rank);
                }
            }

            prk = CalculateRankInfo(prk);

            return prk;
        }

        inline HonorScores GenerateScores(HonorStandingList standingList, uint32 team)
        {
            HonorScores sc;

            // initialize the breakpoint values
            sc.BRK[13] = 0.002f;
            sc.BRK[12] = 0.007f;
            sc.BRK[11] = 0.017f;
            sc.BRK[10] = 0.037f;
            sc.BRK[ 9] = 0.077f;
            sc.BRK[ 8] = 0.137f;
            sc.BRK[ 7] = 0.207f;
            sc.BRK[ 6] = 0.287f;
            sc.BRK[ 5] = 0.377f;
            sc.BRK[ 4] = 0.477f;
            sc.BRK[ 3] = 0.587f;
            sc.BRK[ 2] = 0.715f;
            sc.BRK[ 1] = 0.858f;
            sc.BRK[ 0] = 1.000f;

            // get the WS scores at the top of each break point
            for (uint8 group = 0; group < 14; group++)
                sc.BRK[group] = floor((sc.BRK[group] * standingList.size()) + 0.5f);

            // initialize RP array
            // set the low point
            sc.FY[ 0] = 0;

            // the Y values for each breakpoint are fixed
            sc.FY[ 1] = 400;
            for (uint8 i = 2; i <= 13; i++)
            {
                sc.FY[i] = (i - 1) * 1000;
            }

            // and finally
            sc.FY[14] = 13000;   // ... gets 13000 RP

            // the X values for each breakpoint are found from the CP scores
            // of the players around that point in the WS scores
            HonorStanding* tempSt;
            float honor;

            // initialize CP array
            sc.FX[ 0] = 0;

            for (uint8 i = 1; i <= 13; i++)
            {
                honor = 0.0f;
                tempSt = sObjectMgr.GetHonorStandingByPosition(sc.BRK[i], team);
                if (tempSt)
                {
                    honor += tempSt->honorPoints;
                    tempSt = sObjectMgr.GetHonorStandingByPosition(sc.BRK[i] + 1, team);
                    if (tempSt)
                        honor += tempSt->honorPoints;
                }

                sc.FX[i] = honor ? honor / 2 : 0;
            }

            // set the high point if FX full filled before
            sc.FX[14] = sc.FX[13] ? standingList.begin()->honorPoints : 0;   // top scorer

            return sc;
        }

        inline float CalculateRpEarning(float CP, HonorScores sc)
        {
            // search the function for the two points that bound the given CP
            uint8 i = 0;
            while (i < 14 && sc.BRK[i] > 0 && sc.FX[i] <= CP)
                i++;

            // we now have i such that FX[i] > CP >= FX[i-1]
            // so interpolate
            return (sc.FY[i] - sc.FY[i - 1]) * (CP - sc.FX[i - 1]) / (sc.FX[i] - sc.FX[i - 1]) + sc.FY[i - 1];
        }

        inline float CalculateRpDecay(float rpEarning, float RP)
        {
            float Decay = floor((0.2f * RP) + 0.5f);
            float Delta = rpEarning - Decay;
            if (Delta < 0)
            {
                Delta = Delta / 2;
            }
            if (Delta < -2500)
            {
                Delta = -2500;
            }
            return RP + Delta;
        }

        inline float DishonorableKillPoints(int level)
        {
            float result = 10;
            if (level >= 30 && level <= 35)
                result = result + 1.5 * (level - 29);
            if (level >= 36 && level <= 41)
                result = result + 9 + 2 * (level - 35);
            if (level >= 42 && level <= 50)
                result = result + 21 + 3.2 * (level - 41);
            if (level >= 51)
                result = result + 50 + 4 * (level - 50);
            if (result > 100)
                return 100.0;
            else
                return result;
        }

        inline float HonorableKillPoints(Player* killer, Player* victim, uint32 groupsize)
        {
            if (!killer || !victim || !groupsize)
                return 0.0;

            uint32 today = sWorld.GetDateToday();

            int total_kills  = killer->CalculateTotalKills(victim, today, today);
            // int k_rank       = killer->CalculateHonorRank( killer->GetTotalHonor() );
            uint32 v_rank    = victim->GetHonorRankInfo().visualRank;
            uint32 k_level   = killer->getLevel();
            // int v_level      = victim->getLevel();
            float diff_honor = (victim->GetRankPoints() / (killer->GetRankPoints() + 1)) + 1;
            float diff_level = (victim->getLevel() * (1.0 / (killer->getLevel())));

            int f = (10 - total_kills) >= 0 ? (10 - total_kills) : 0;
            int honor_points = int(((float)(f * 0.25) * (float)((k_level + (v_rank * 5 + 1)) * (1 + 0.05 * diff_honor) * diff_level)));
            return (honor_points <= 400 ? honor_points : 400) / groupsize;
        }
    }


    namespace XP
    {
        inline bool IsTrivialLevelDifference(uint32 unitLvl, uint32 targetLvl)
        {
            if (unitLvl > targetLvl)
            {
                const uint32 diff = (unitLvl - targetLvl);
                switch (unitLvl / 5)
                {
                    case 0:     // 0-4
                    case 1:     // 5-9
                        return (diff > 4);
                    case 2:     // 10-14
                    case 3:     // 15-19
                        return (diff > 5);
                    case 4:     // 20-24
                    case 5:     // 25-29
                        return (diff > 6);
                    case 6:     // 30-34
                    case 7:     // 35-39
                        return (diff > 7);
                    case 8:     // 40-44
                        return (diff > 8);
                    case 9:     // 45-49
                        return (diff > 9);
                    case 10:    // 50-54
                        return (diff > 10);
                    case 11:    // 55-59
                        return (diff > 11);
                    default:    // 60+
                        return (diff > 12);
                }
            }
            return false;
        }

        enum XPColorChar { RED, ORANGE, YELLOW, GREEN, GRAY };

        inline uint32 GetGrayLevel(uint32 pl_level)
        {
            if (pl_level <= 5)
                return 0;
            if (pl_level <= 39)
                return pl_level - 5 - pl_level / 10;
            if (pl_level <= 59)
                return pl_level - 1 - pl_level / 5;
            return pl_level - 9;
        }

        inline XPColorChar GetColorCode(uint32 pl_level, uint32 mob_level)
        {
            if (mob_level >= pl_level + 5)
                return RED;
            if (mob_level >= pl_level + 3)
                return ORANGE;
            if (mob_level >= pl_level - 2)
                return YELLOW;
            if (mob_level > GetGrayLevel(pl_level))
                return GREEN;
            return GRAY;
        }

        inline uint32 GetZeroDifference(uint32 unit_level)
        {
            if (unit_level < 8)  return 5;
            if (unit_level < 10) return 6;
            if (unit_level < 12) return 7;
            if (unit_level < 16) return 8;
            if (unit_level < 20) return 9;
            if (unit_level < 30) return 11;
            if (unit_level < 40) return 12;
            if (unit_level < 45) return 13;
            if (unit_level < 50) return 14;
            if (unit_level < 55) return 15;
            if (unit_level < 60) return 16;
            return 17;
        }

        inline float BaseGain(uint32 unit_level, uint32 mob_level)
        {
            const uint32 nBaseExp = unit_level * 5 + 45;
            if (mob_level >= unit_level)
            {
                uint32 nLevelDiff = mob_level - unit_level;
                if (nLevelDiff > 4)
                    nLevelDiff = 4;
                return nBaseExp * (1.0f + (0.05f * nLevelDiff));
            }
            uint32 gray_level = GetGrayLevel(unit_level);
            if (mob_level > gray_level)
            {
                uint32 ZD = GetZeroDifference(unit_level);
                uint32 nLevelDiff = unit_level - mob_level;
                return nBaseExp * (1.0f - (float(nLevelDiff) / ZD));
            }
            return 0;
        }

        inline uint32 Gain(Unit const* unit, Creature* target)
        {
            if (target->IsTotem() || target->IsPet() || target->IsNoXp() || target->IsCritter())
                return 0;

            float xp_gain = BaseGain(unit->getLevel(), target->getLevel());
            if (xp_gain == 0.0f)
                return 0;

            if (target->IsElite())
            {
                if (target->GetMap()->IsNoRaid())
                    xp_gain *= 2.5f;
                else
                    xp_gain *= 2.0f;
            }

            xp_gain *= target->GetCreatureInfo()->ExperienceMultiplier;

            return (uint32)(std::nearbyint(xp_gain * sWorld.getConfig(CONFIG_FLOAT_RATE_XP_KILL)));
        }

        inline float xp_in_group_rate(uint32 count, bool /*isRaid*/)
        {
            // TODO: this formula is completely guesswork only based on a logical assumption
            switch (count)
            {
                case 0:
                case 1:
                case 2:
                    return 1.0f;
                case 3:
                    return 1.166f;
                case 4:
                    return 1.3f;
                case 5:
                    return 1.4f;
                default:
                    return std::max(1.f - count * 0.05f, 0.01f);
            }
        }
    }
}
#endif
