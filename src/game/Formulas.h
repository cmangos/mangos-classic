/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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
        inline uint32 hk_honor_at_level(uint32 level, uint32 count=1)
        {
            return (uint32)ceil(count*(-0.53177f + 0.59357f * exp((level +23.54042f) / 26.07859f )));
        }
    }

    namespace Honor
    {
        //TODO: Fix this formula, for now the weekly rating is how many honor player gain all life time
        inline float CalculeRating(Player *plr)
        {
            return plr->GetTotalHonor()+plr->GetStoredHonor();
        }

        //TODO: Implement this function
        // NOTE: DO NOT IMPLEMENT A FUNCTION THAT USES A QUERY, SAVE THE DATA
        inline int32 CalculeStanding(Player *plr)
        {
            uint64 guid = 0;
            int standing = 0;

            float m_rating = CalculeRating(plr);
            QueryResult *result = CharacterDatabase.PQuery("SELECT count(*) as cnt FROM `characters` WHERE `honor_rating` >= '%f'", m_rating );
            if(result)
            {
                Field *fields = result->Fetch();
                standing = fields[0].GetUInt32();
                delete result;
            }
            return standing;
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

        // THIS FUNCTION WILL NEVER BE FAST as it uses a mysql query
        // TODO: Load it at player load and modify it while the player is playing
        // then when player save, save it!!!
        inline float HonorableKillPoints( Player *killer, Player *victim )
        {
            int total_kills  = killer->CalculateTotalKills(victim);
            //int k_rank       = killer->CalculateHonorRank( killer->GetTotalHonor() );
            uint32 v_rank    = victim->CalculateHonorRank( victim->GetTotalHonor() );
            uint32 k_level   = killer->getLevel();
            //int v_level      = victim->getLevel();
            float diff_honor = (victim->GetTotalHonor() /(killer->GetTotalHonor()+1))+1;
            float diff_level = (victim->getLevel()*(1.0/( killer->getLevel() )));

            int f = (4 - total_kills) >= 0 ? (4 - total_kills) : 0;
            int honor_points = int(((float)(f * 0.25)*(float)((k_level+(v_rank*5+1))*(1+0.05*diff_honor)*diff_level)));
            return (honor_points <= 400 ? honor_points : 400);
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

            return (uint32)(xp_gain*sWorld.getRate(RATE_XP_KILL));
        }

        inline uint32 xp_Diff(uint32 lvl)
        {
            if( lvl < 29 )
                return 0;
            if( lvl == 29 )
                return 1;
            if( lvl == 30 )
                return 3;
            if( lvl == 31 )
                return 6;
            else
                return (5*(lvl-30));
        }

        inline uint32 mxp(uint32 lvl)
        {
            if (lvl < 60)
            {
                return (45 + (5*lvl));
            }
            else
            {
                return (235 + (5*lvl));
            }
        }

        inline uint32 xp_to_level(uint32 lvl)
        {
            uint32 xp = 0;
            if (lvl < 60)
            {
                xp = (8*lvl + xp_Diff(lvl)) * mxp(lvl);
            }
            else if (lvl == 60)
            {
                xp = (155 + mxp(lvl) * (1344 - 70 - ((69 - lvl) * (7 + (69 - lvl) * 8 - 1)/2)));
            }
            else
            {
                // level higher than 60 is not supported
                xp = (uint32)(779700 * (pow(sWorld.getRate(RATE_XP_PAST_60), (int32)lvl - 59)));
                return ((xp < 0x7fffffff) ? xp : 0x7fffffff);
            }

            // The XP to Level is always rounded to the nearest 100 points (50 rounded to high).
            xp = ((xp + 50) / 100) * 100;                   // use additional () for prevent free association operations in C++

            return xp;
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
