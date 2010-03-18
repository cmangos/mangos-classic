/*
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2010 MaNGOSZero <http://github.com/mangoszero/mangoszero/>
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

#include "World.h"


namespace MaNGOS
{
    namespace Honor
    {
        inline float hk_honor_at_level(uint32 level, uint32 count=1)
        {
            return (float)ceil(count*(-0.53177f + 0.59357f * exp((level +23.54042f) / 26.07859f )));
        }
    }

    namespace Honor
    {

        //What is Player's rank... private, scout...
        inline uint32 CalculateHonorRank(float honor_points,uint32 totalKills)
        {
            uint32 rank = 0;
            // you can modify this formula on negative values to show old dishonored ranks too
            if(honor_points <=    0.00 || totalKills < HONOR_STANDING_MIN_KILL)
                rank = 0;
            else if(honor_points <  2000.00 && totalKills >= HONOR_STANDING_MIN_KILL)
                rank = 1;
            else if(honor_points > ((HONOR_RANK_COUNT-2)-1)*5000)
                rank = HONOR_RANK_COUNT-2;
            else
                rank = uint32(honor_points / 5000) + 2;

            return rank;
        }

        inline float CalculateRPearning(HonorStandingList standingList,HonorStanding *StandingInfo,uint32 team,uint32 RP)
        {
            float CP = StandingInfo->honorPoints;

            if (standingList.empty())
               return CP;

            float BRK[14], FX[15], FY[15];
            // initialize the breakpoint values
            BRK[13] = 0.002f;
            BRK[12] = 0.007f;
            BRK[11] = 0.017f;
            BRK[10] = 0.037f;
            BRK[ 9] = 0.077f;
            BRK[ 8] = 0.137f;       
            BRK[ 7] = 0.207f;       
            BRK[ 6] = 0.287f;       
            BRK[ 5] = 0.377f;       
            BRK[ 4] = 0.477f;       
            BRK[ 3] = 0.587f;      
            BRK[ 2] = 0.715f;      
            BRK[ 1] = 0.858f;
            BRK[ 0] = 1.000f;

            // get the WS scores at the top of each break point
            for (uint8 group=1; group<14; group++) {
              BRK[group] = finiteAlways( BRK[group] * standingList.size() );
            }     

            // set the high point
            FX[14] = standingList.begin()->honorPoints;   // top scorer
            FY[14] = 13000;   // ... gets 13000 RP

            // set the low point
            FX[ 0] = 0;
            FY[ 0] = 0;

            // the Y values for each breakpoint are fixed
            FY[ 1] = 400;
            for (uint8 i=2;i<=13;i++) {
              FY[i] = (i-2) * 1000;
            }

            // the X values for each breakpoint are found from the CP scores
            // of the players around that point in the WS scores
            HonorStanding *tempSt;
            float honor;
            for (uint8 i=1;i<=13;i++) {
              honor = 0.0f;
              tempSt = sObjectMgr.GetHonorStandingByPosition( BRK[i],team );
              if (tempSt)
                  honor += tempSt->honorPoints;
              tempSt = sObjectMgr.GetHonorStandingByPosition( BRK[i]+1, team );
              if (tempSt)
                  honor += tempSt->honorPoints;

              FX[i] = honor / 2;
            }

            // search the function for the two points that bound the given CP
            uint8 i = 15;
            while (i>0 && FX[i-1] > CP) {
              i--;
            }

            // we now have i such that FX[i] > CP >= FX[i-1]
            // so interpolate
            float rpEarning = (FY[i] - FY[i-1]) * (CP - FX[i-1]) / (FX[i] - FX[i-1]) + FY[i-1];

            float Decay = finiteAlways(0.2 * RP);
            float Delta = rpEarning - Decay;
            if (Delta < 0) {
               Delta = Delta / 2;
            }
            if (Delta < -2500) {
               Delta = -2500;
            }
            return Delta;
        }

        inline float DishonorableKillPoints(int level)
        {
            float result = 10;
            if(level >= 30 && level <= 35)
                result = result + 1.5 * (level - 29);
            if(level >= 36 && level <= 41)
                result = result + 9 + 2 * (level - 35);
            if(level >= 42 && level <= 50)
                result = result + 21 + 3.2 * (level - 41);
            if(level >= 51)
                result = result + 50 + 4 * (level - 50);
            if(result > 100)
                return 100.0;
            else
                return result;
        }

        inline float HonorableKillPoints( Player *killer, Player *victim, uint32 groupsize)
        {
            if (!killer || !victim || !groupsize)
                return 0.0;

            uint32 today = sWorld.GetDateToday();

            int total_kills  = killer->CalculateTotalKills(victim,today,today);
            //int k_rank       = killer->CalculateHonorRank( killer->GetTotalHonor() );
            uint32 v_rank    = victim->GetHonorRank();
            uint32 k_level   = killer->getLevel();
            //int v_level      = victim->getLevel();
            float diff_honor = (victim->GetRankPoints() /(killer->GetRankPoints()+1))+1;
            float diff_level = (victim->getLevel()*(1.0/( killer->getLevel() )));

            int f = (10 - total_kills) >= 0 ? (10 - total_kills) : 0;
            int honor_points = int(((float)(f * 0.25)*(float)((k_level+(v_rank*5+1))*(1+0.05*diff_honor)*diff_level)));
            return (honor_points <= 400 ? honor_points : 400) / groupsize;
        }

    }


    namespace XP
    {
        typedef enum XPColorChar { RED, ORANGE, YELLOW, GREEN, GRAY };

        inline uint32 GetGrayLevel(uint32 pl_level)
        {
            if( pl_level <= 5 )
                return 0;
            else if( pl_level <= 39 )
                return pl_level - 5 - pl_level/10;
            else if( pl_level <= 59 )
                return pl_level - 1 - pl_level/5;
            else
                return pl_level - 9;
        }

        inline XPColorChar GetColorCode(uint32 pl_level, uint32 mob_level)
        {
            if( mob_level >= pl_level + 5 )
                return RED;
            else if( mob_level >= pl_level + 3 )
                return ORANGE;
            else if( mob_level >= pl_level - 2 )
                return YELLOW;
            else if( mob_level > GetGrayLevel(pl_level) )
                return GREEN;
            else
                return GRAY;
        }

        inline uint32 GetZeroDifference(uint32 pl_level)
        {
            if( pl_level < 8 )  return 5;
            if( pl_level < 10 ) return 6;
            if( pl_level < 12 ) return 7;
            if( pl_level < 16 ) return 8;
            if( pl_level < 20 ) return 9;
            if( pl_level < 30 ) return 11;
            if( pl_level < 40 ) return 12;
            if( pl_level < 45 ) return 13;
            if( pl_level < 50 ) return 14;
            if( pl_level < 55 ) return 15;
            if( pl_level < 60 ) return 16;
            return 17;
        }

        inline uint32 BaseGain(uint32 pl_level, uint32 mob_level)
        {
            const uint32 nBaseExp = 45;
            if( mob_level >= pl_level )
            {
                uint32 nLevelDiff = mob_level - pl_level;
                if (nLevelDiff > 4)
                    nLevelDiff = 4;
                return ((pl_level*5 + nBaseExp) * (20 + nLevelDiff)/10 + 1)/2;
            }
            else
            {
                uint32 gray_level = GetGrayLevel(pl_level);
                if( mob_level > gray_level )
                {
                    uint32 ZD = GetZeroDifference(pl_level);
                    return (pl_level*5 + nBaseExp) * (ZD + mob_level - pl_level)/ZD;
                }
                return 0;
            }
        }

        inline uint32 Gain(Player *pl, Unit *u)
        {
            if(u->GetTypeId()==TYPEID_UNIT && (
                ((Creature*)u)->isTotem() || ((Creature*)u)->isPet() ||
                (((Creature*)u)->GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_NO_XP_AT_KILL) ))
                return 0;

            uint32 xp_gain= BaseGain(pl->getLevel(), u->getLevel());
            if( xp_gain == 0 )
                return 0;

            if(u->GetTypeId()==TYPEID_UNIT && ((Creature*)u)->isElite())
                xp_gain *= 2;

            return (uint32)(xp_gain*sWorld.getConfig(CONFIG_FLOAT_RATE_XP_KILL));
        }

        inline float xp_in_group_rate(uint32 count, bool isRaid)
        {
            if(isRaid)
            {
                // FIX ME: must apply decrease modifiers dependent from raid size
                return 1.0f;
            }
            else
            {
                switch(count)
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
                    default:
                        return 1.4f;
                }
            }
        }
    }
}
#endif
