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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Server/DBCStores.h"
#include "WorldPacket.h"
#include "Server/WorldSession.h"
#include "World/World.h"
#include "Globals/ObjectMgr.h"
#include "Entities/Player.h"
#include "Server/Opcodes.h"
#include "Chat/Chat.h"
#include "Chat/ChannelMgr.h"
#include "Log.h"
#include "Globals/ObjectAccessor.h"
#include "Tools/Language.h"
#include "Grids/CellImpl.h"
#include "Maps/MapManager.h"
#include "Maps/GridDefines.h"
#include "Maps/MapPersistentStateMgr.h"
#include "Mails/Mail.h"
#include "Util.h"
#include "AI/ScriptDevAI/ScriptDevAIMgr.h"
#include "Spells/SpellMgr.h"
#include "Entities/Transports.h"
#ifdef _DEBUG_VMAPS
#include "VMapFactory.h"
#endif

//-----------------------Npc Commands-----------------------
bool ChatHandler::HandleNpcSayCommand(char* args)
{
    if (!*args)
        return false;

    Creature* pCreature = getSelectedCreature();
    if (!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->MonsterSay(args, LANG_UNIVERSAL, m_session->GetPlayer());

    return true;
}

bool ChatHandler::HandleNpcYellCommand(char* args)
{
    if (!*args)
        return false;

    Creature* pCreature = getSelectedCreature();
    if (!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->MonsterYell(args, LANG_UNIVERSAL, m_session->GetPlayer());

    return true;
}

// show text emote by creature in chat
bool ChatHandler::HandleNpcTextEmoteCommand(char* args)
{
    if (!*args)
        return false;

    Creature* pCreature = getSelectedCreature();

    if (!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->MonsterTextEmote(args, nullptr);

    return true;
}

// make npc whisper to player
bool ChatHandler::HandleNpcWhisperCommand(char* args)
{
    Player* target;
    if (!ExtractPlayerTarget(&args, &target))
        return false;

    ObjectGuid guid = m_session->GetPlayer()->GetSelectionGuid();
    if (!guid)
        return false;

    Creature* pCreature = m_session->GetPlayer()->GetMap()->GetCreature(guid);

    if (!pCreature || !target || !*args)
        return false;

    // check online security
    if (HasLowerSecurity(target))
        return false;

    pCreature->MonsterWhisper(args, target);

    return true;
}
//----------------------------------------------------------

// global announce
bool ChatHandler::HandleAnnounceCommand(char* args)
{
    if (!*args)
        return false;

    sWorld.SendWorldText(LANG_SYSTEMMESSAGE, args);
    return true;
}

// notification player at the screen
bool ChatHandler::HandleNotifyCommand(char* args)
{
    if (!*args)
        return false;

    std::string str = GetMangosString(LANG_GLOBAL_NOTIFY);
    str += args;

    WorldPacket data(SMSG_NOTIFICATION, (str.size() + 1));
    data << str;
    sWorld.SendGlobalMessage(data);

    return true;
}

// Enable\Dissable GM Mode
bool ChatHandler::HandleGMCommand(char* args)
{
    if (!*args)
    {
        if (m_session->GetPlayer()->IsGameMaster())
            m_session->SendNotification(LANG_GM_ON);
        else
            m_session->SendNotification(LANG_GM_OFF);
        return true;
    }

    bool value;
    if (!ExtractOnOff(&args, value))
    {
        SendSysMessage(LANG_USE_BOL);
        SetSentErrorMessage(true);
        return false;
    }

    if (value)
    {
        m_session->GetPlayer()->SetGameMaster(true);
        m_session->SendNotification(LANG_GM_ON);
    }
    else
    {
        m_session->GetPlayer()->SetGameMaster(false);
        m_session->SendNotification(LANG_GM_OFF);
    }

    return true;
}

// Acquire random unusual land mount or visually mount displayid or selected creature
bool ChatHandler::HandleGMMountUpCommand(char* args)
{
    if (*args)
    {
        if (ExtractLiteralArg(&args, "target"))
        {
            if (Unit* unit = getSelectedUnit())
            {
                if (unit->GetTypeId() == TYPEID_UNIT)
                {
                    m_session->GetPlayer()->Mount(unit->GetDisplayId());
                    return true;
                }
            }

            SendSysMessage(LANG_COMMAND_NOCREATUREFOUND);
            SetSentErrorMessage(true);
            return false;
        }
        else
        {
            uint32 displayid;

            if (ExtractUInt32(&args, displayid))
            {
                if (sCreatureDisplayInfoStore.LookupEntry(displayid))
                {
                    m_session->GetPlayer()->Mount(displayid);
                    return true;
                }

                SendSysMessage(LANG_BAD_VALUE);
                SetSentErrorMessage(true);
                return false;
            }
        }
    }

    return ModifyMountCommandHelper(m_session->GetPlayer(), args);
}

// Enable\Dissable Invisible mode
bool ChatHandler::HandleGMVisibleCommand(char* args)
{
    if (!*args)
    {
        PSendSysMessage(LANG_YOU_ARE, m_session->GetPlayer()->isGMVisible() ?  GetMangosString(LANG_VISIBLE) : GetMangosString(LANG_INVISIBLE));
        return true;
    }

    bool value;
    if (!ExtractOnOff(&args, value))
    {
        SendSysMessage(LANG_USE_BOL);
        SetSentErrorMessage(true);
        return false;
    }

    Player* player = m_session->GetPlayer();
    SpellEntry const* invisibleAuraInfo = sSpellTemplate.LookupEntry<SpellEntry>(sWorld.getConfig(CONFIG_UINT32_GM_INVISIBLE_AURA));
    if (!invisibleAuraInfo || !IsSpellAppliesAura(invisibleAuraInfo))
        invisibleAuraInfo = nullptr;

    if (value)
    {
        player->SetGMVisible(true);
        m_session->SendNotification(LANG_INVISIBLE_VISIBLE);
        if (invisibleAuraInfo)
            player->RemoveAurasDueToSpell(invisibleAuraInfo->Id);
    }
    else
    {
        m_session->SendNotification(LANG_INVISIBLE_INVISIBLE);
        player->SetGMVisible(false);
        if (invisibleAuraInfo)
            player->CastSpell(player, invisibleAuraInfo, TRIGGERED_OLD_TRIGGERED);
    }

    return true;
}

bool ChatHandler::HandleGPSCommand(char* args)
{
    WorldObject* obj = nullptr;
    if (*args)
    {
        if (ObjectGuid guid = ExtractGuidFromLink(&args))
            obj = (WorldObject*)m_session->GetPlayer()->GetObjectByTypeMask(guid, TYPEMASK_CREATURE_OR_GAMEOBJECT);

        if (!obj)
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }
    }
    else
    {
        obj = getSelectedUnit();

        if (!obj)
        {
            SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            SetSentErrorMessage(true);
            return false;
        }
    }
    CellPair cell_val = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    Cell cell(cell_val);

    uint32 zone_id, area_id;
    obj->GetZoneAndAreaId(zone_id, area_id);

    MapEntry const* mapEntry = sMapStore.LookupEntry(obj->GetMapId());
    AreaTableEntry const* zoneEntry = GetAreaEntryByAreaID(zone_id);
    AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(area_id);

    float zone_x = obj->GetPositionX();
    float zone_y = obj->GetPositionY();

    if (!Map2ZoneCoordinates(zone_x, zone_y, zone_id))
    {
        zone_x = 0;
        zone_y = 0;
    }

    Map const* map = obj->GetMap();
    float ground_z = map->GetHeight(obj->GetPositionX(), obj->GetPositionY(), MAX_HEIGHT);
    float floor_z = map->GetHeight(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ());

    GridPair p = MaNGOS::ComputeGridPair(obj->GetPositionX(), obj->GetPositionY());

    int gx = 63 - p.x_coord;
    int gy = 63 - p.y_coord;

    uint32 have_map = GridMap::ExistMap(obj->GetMapId(), gx, gy) ? 1 : 0;
    uint32 have_vmap = GridMap::ExistVMap(obj->GetMapId(), gx, gy) ? 1 : 0;

    TerrainInfo const* terrain = obj->GetTerrain();

    if (have_vmap)
    {
        if (terrain->IsOutdoors(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ()))
            PSendSysMessage("You are OUTdoor");
        else
            PSendSysMessage("You are INdoor");
    }
    else PSendSysMessage("no VMAP available for area info");

    PSendSysMessage(LANG_MAP_POSITION,
                    obj->GetMapId(), (mapEntry ? mapEntry->name[GetSessionDbcLocale()] : "<unknown>"),
                    zone_id, (zoneEntry ? zoneEntry->area_name[GetSessionDbcLocale()] : "<unknown>"),
                    area_id, (areaEntry ? areaEntry->area_name[GetSessionDbcLocale()] : "<unknown>"),
                    obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation(),
                    cell.GridX(), cell.GridY(), cell.CellX(), cell.CellY(), obj->GetInstanceId(),
                    zone_x, zone_y, ground_z, floor_z, have_map, have_vmap);

    if (GenericTransport* transport = obj->GetTransport())
    {
        Position pos = obj->GetPosition(transport);
        PSendSysMessage("Transport coords: %f %f %f %f", pos.x, pos.y, pos.z, pos.o);
    }

    DEBUG_LOG("Player %s GPS call for %s '%s' (%s: %u):",
              m_session ? GetNameLink().c_str() : GetMangosString(LANG_CONSOLE_COMMAND),
              (obj->GetTypeId() == TYPEID_PLAYER ? "player" : "creature"), obj->GetName(),
              (obj->GetTypeId() == TYPEID_PLAYER ? "GUID" : "Entry"), (obj->GetTypeId() == TYPEID_PLAYER ? obj->GetGUIDLow() : obj->GetEntry()));

    DEBUG_LOG(GetMangosString(LANG_MAP_POSITION),
              obj->GetMapId(), (mapEntry ? mapEntry->name[sWorld.GetDefaultDbcLocale()] : "<unknown>"),
              zone_id, (zoneEntry ? zoneEntry->area_name[sWorld.GetDefaultDbcLocale()] : "<unknown>"),
              area_id, (areaEntry ? areaEntry->area_name[sWorld.GetDefaultDbcLocale()] : "<unknown>"),
              obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation(),
              cell.GridX(), cell.GridY(), cell.CellX(), cell.CellY(), obj->GetInstanceId(),
              zone_x, zone_y, ground_z, floor_z, have_map, have_vmap);

    GridMapLiquidData liquid_status;
    GridMapLiquidStatus res = terrain->getLiquidStatus(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), MAP_ALL_LIQUIDS, &liquid_status);
    if (res)
    {
        PSendSysMessage(LANG_LIQUID_STATUS, liquid_status.level, liquid_status.depth_level, liquid_status.type_flags, uint32(res));
    }

    // Additional vmap debugging help
#ifdef _DEBUG_VMAPS
    PSendSysMessage("Static terrain height (maps only): %f", obj->GetTerrain()->GetHeightStatic(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), false));

    if (VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager())
        PSendSysMessage("Vmap Terrain Height %f", vmgr->getHeight(obj->GetMapId(), obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ() + 2.0f, 10000.0f));

    PSendSysMessage("Static map height (maps and vmaps): %f", obj->GetTerrain()->GetHeightStatic(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ()));
#endif

    return true;
}

// Summon Player
bool ChatHandler::HandleNamegoCommand(char* args)
{
    Player* target;
    ObjectGuid target_guid;
    std::string target_name;
    if (!ExtractPlayerTarget(&args, &target, &target_guid, &target_name))
        return false;

    Player* player = m_session->GetPlayer();
    if (target == player || target_guid == player->GetObjectGuid())
    {
        PSendSysMessage(LANG_CANT_TELEPORT_SELF);
        SetSentErrorMessage(true);
        return false;
    }

    if (target)
    {
        std::string nameLink = playerLink(target_name);
        // check online security
        if (HasLowerSecurity(target))
            return false;

        // make sure player is in world
        if (!target->IsInWorld() || target->IsBeingTeleported())
        {
            PSendSysMessage(LANG_IS_TELEPORTED, nameLink.c_str());
            SetSentErrorMessage(true);
            return false;
        }

        Map* pMap = player->GetMap();

        if (!pMap)
            return false;

        if (pMap->IsBattleGround())
        {
            // only allow if gm mode is on
            if (!player->IsGameMaster())
            {
                PSendSysMessage(LANG_CANNOT_GO_TO_BG_GM, nameLink.c_str());
                SetSentErrorMessage(true);
                return false;
            }
            // if both players are in different bgs
            if (target->GetBattleGroundId() && player->GetBattleGroundId() != target->GetBattleGroundId())
            {
                PSendSysMessage(LANG_CANNOT_GO_TO_BG_FROM_BG, nameLink.c_str());
                SetSentErrorMessage(true);
                return false;
            }
            // all's well, set bg id
            // when porting out from the bg, it will be reset to 0
            target->SetBattleGroundId(player->GetBattleGroundId(), player->GetBattleGroundTypeId());
            // remember current position as entry point for return at bg end teleportation
            if (!target->GetMap()->IsBattleGround())
                target->SetBattleGroundEntryPoint();
        }
        else if (pMap->IsDungeon())
        {
            Map* cMap = target->GetMap();
            if (cMap->Instanceable() && cMap->GetInstanceId() != pMap->GetInstanceId())
            {
                // cannot summon from instance to instance
                PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST, nameLink.c_str());
                SetSentErrorMessage(true);
                return false;
            }

            // we are in instance, and can summon only player in our group with us as lead
            if (!player->GetGroup() || !target->GetGroup() ||
                    (target->GetGroup()->GetLeaderGuid() != player->GetObjectGuid()) ||
                    (player->GetGroup()->GetLeaderGuid() != player->GetObjectGuid()))
                // the last check is a bit excessive, but let it be, just in case
            {
                PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST, nameLink.c_str());
                SetSentErrorMessage(true);
                return false;
            }
        }

        PSendSysMessage(LANG_SUMMONING, nameLink.c_str(), "");
        if (needReportToTarget(target))
            ChatHandler(target).PSendSysMessage(LANG_SUMMONED_BY, playerLink(player->GetName()).c_str());

        if (!target->TaxiFlightInterrupt())
            target->SaveRecallPosition();

        // before GM
        float x, y, z;
        player->GetClosePoint(x, y, z, target->GetObjectBoundingRadius());
        target->TeleportTo(player->GetMapId(), x, y, z, target->GetOrientation());
    }
    else
    {
        // check offline security
        if (HasLowerSecurity(nullptr, target_guid))
            return false;

        std::string nameLink = playerLink(target_name);

        PSendSysMessage(LANG_SUMMONING, nameLink.c_str(), GetMangosString(LANG_OFFLINE));

        // in point where GM stay
        Player::SavePositionInDB(target_guid, player->GetMapId(),
                                 player->GetPositionX(),
                                 player->GetPositionY(),
                                 player->GetPositionZ(),
                                 player->GetOrientation(),
                                 player->GetZoneId());
    }

    return true;
}

// Teleport to Player
bool ChatHandler::HandleGonameCommand(char* args)
{
    Player* target;
    ObjectGuid target_guid;
    std::string target_name;
    if (!ExtractPlayerTarget(&args, &target, &target_guid, &target_name))
        return false;

    Player* _player = m_session->GetPlayer();
    if (target == _player || target_guid == _player->GetObjectGuid())
    {
        SendSysMessage(LANG_CANT_TELEPORT_SELF);
        SetSentErrorMessage(true);
        return false;
    }


    if (target)
    {
        // check online security
        if (HasLowerSecurity(target))
            return false;

        std::string chrNameLink = playerLink(target_name);

        Map* cMap = target->GetMap();
        if (cMap->IsBattleGround())
        {
            // only allow if gm mode is on
            if (!_player->IsGameMaster())
            {
                PSendSysMessage(LANG_CANNOT_GO_TO_BG_GM, chrNameLink.c_str());
                SetSentErrorMessage(true);
                return false;
            }
            // if both players are in different bgs
            if (_player->GetBattleGroundId() && _player->GetBattleGroundId() != target->GetBattleGroundId())
            {
                PSendSysMessage(LANG_CANNOT_GO_TO_BG_FROM_BG, chrNameLink.c_str());
                SetSentErrorMessage(true);
                return false;
            }
            // all's well, set bg id
            // when porting out from the bg, it will be reset to 0
            _player->SetBattleGroundId(target->GetBattleGroundId(), target->GetBattleGroundTypeId());
            // remember current position as entry point for return at bg end teleportation
            if (!_player->GetMap()->IsBattleGround())
                _player->SetBattleGroundEntryPoint();
        }
        else if (cMap->IsDungeon())
        {
            // we have to go to instance, and can go to player only if:
            //   1) we are in his group (either as leader or as member)
            //   2) we are not bound to any group and have GM mode on
            if (_player->GetGroup())
            {
                // we are in group, we can go only if we are in the player group
                if (_player->GetGroup() != target->GetGroup())
                {
                    PSendSysMessage(LANG_CANNOT_GO_TO_INST_PARTY, chrNameLink.c_str());
                    SetSentErrorMessage(true);
                    return false;
                }
            }
            else
            {
                // we are not in group, let's verify our GM mode
                if (!_player->IsGameMaster())
                {
                    PSendSysMessage(LANG_CANNOT_GO_TO_INST_GM, chrNameLink.c_str());
                    SetSentErrorMessage(true);
                    return false;
                }
            }

            // if the player or the player's group is bound to another instance
            // the player will not be bound to another one
            InstancePlayerBind* pBind = _player->GetBoundInstance(target->GetMapId());
            if (!pBind)
            {
                Group* group = _player->GetGroup();
                // if no bind exists, create a solo bind
                InstanceGroupBind* gBind = group ? group->GetBoundInstance(target->GetMapId()) : nullptr;
                // if no bind exists, create a solo bind
                if (!gBind)
                {
                    DungeonPersistentState* save = ((DungeonMap*)target->GetMap())->GetPersistanceState();

                    // if player is group leader then we need add group bind
                    if (group && group->IsLeader(_player->GetObjectGuid()))
                        group->BindToInstance(save, !save->CanReset());
                    else
                        _player->BindToInstance(save, !save->CanReset());
                }
            }
        }

        PSendSysMessage(LANG_APPEARING_AT, chrNameLink.c_str());
        if (needReportToTarget(target))
            ChatHandler(target).PSendSysMessage(LANG_APPEARING_TO, GetNameLink().c_str());

        // stop flight if need
        if (!_player->TaxiFlightInterrupt())
            _player->SaveRecallPosition();

        // to point to see at target with same orientation
        float x, y, z;
        target->GetContactPoint(target, x, y, z);
        
        if (GenericTransport* transport = target->GetTransport())
            transport->CalculatePassengerOffset(x, y, z);
        _player->TeleportTo(target->GetMapId(), x, y, z, _player->GetAngle(target), TELE_TO_GM_MODE, nullptr, target->GetTransport());
    }
    else
    {
        // check offline security
        if (HasLowerSecurity(nullptr, target_guid))
            return false;

        std::string nameLink = playerLink(target_name);

        PSendSysMessage(LANG_APPEARING_AT, nameLink.c_str());

        // to point where player stay (if loaded)
        float x, y, z, o;
        uint32 map;
        bool in_flight;
        if (!Player::LoadPositionFromDB(target_guid, map, x, y, z, o, in_flight))
            return false;

        return HandleGoHelper(_player, map, x, y, &z);
    }

    return true;
}

// Teleport player to last position
bool ChatHandler::HandleRecallCommand(char* args)
{
    Player* target;
    if (!ExtractPlayerTarget(&args, &target))
        return false;

    // check online security
    if (HasLowerSecurity(target))
        return false;

    if (target->IsBeingTeleported())
    {
        PSendSysMessage(LANG_IS_TELEPORTED, GetNameLink(target).c_str());
        SetSentErrorMessage(true);
        return false;
    }

    return HandleGoHelper(target, target->m_recallMap, target->m_recallX, target->m_recallY, &target->m_recallZ, &target->m_recallO);
}

// Edit Player HP
bool ChatHandler::HandleModifyHPCommand(char* args)
{
    if (!*args)
        return false;

    int32 hp = atoi(args);
    int32 hpm = atoi(args);

    if (hp <= 0 || hpm <= 0 || hpm < hp)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player* chr = getSelectedPlayer();
    if (chr == nullptr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // check online security
    if (HasLowerSecurity(chr))
        return false;

    PSendSysMessage(LANG_YOU_CHANGE_HP, GetNameLink(chr).c_str(), hp, hpm);
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_HP_CHANGED, GetNameLink().c_str(), hp, hpm);

    chr->SetMaxHealth(hpm);
    chr->SetHealth(hp);

    return true;
}

// Edit Player Mana
bool ChatHandler::HandleModifyManaCommand(char* args)
{
    if (!*args)
        return false;

    int32 mana = atoi(args);
    int32 manam = atoi(args);

    if (mana <= 0 || manam <= 0 || manam < mana)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player* chr = getSelectedPlayer();
    if (chr == nullptr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // check online security
    if (HasLowerSecurity(chr))
        return false;

    PSendSysMessage(LANG_YOU_CHANGE_MANA, GetNameLink(chr).c_str(), mana, manam);
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_MANA_CHANGED, GetNameLink().c_str(), mana, manam);

    chr->SetMaxPower(POWER_MANA, manam);
    chr->SetPower(POWER_MANA, mana);

    return true;
}

// Edit Player Energy
bool ChatHandler::HandleModifyEnergyCommand(char* args)
{
    if (!*args)
        return false;

    int32 energy = atoi(args) * 10;
    int32 energym = atoi(args) * 10;

    if (energy <= 0 || energym <= 0 || energym < energy)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player* chr = getSelectedPlayer();
    if (!chr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // check online security
    if (HasLowerSecurity(chr))
        return false;

    PSendSysMessage(LANG_YOU_CHANGE_ENERGY, GetNameLink(chr).c_str(), energy / 10, energym / 10);
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_ENERGY_CHANGED, GetNameLink().c_str(), energy / 10, energym / 10);

    chr->SetMaxPower(POWER_ENERGY, energym);
    chr->SetPower(POWER_ENERGY, energy);

    DETAIL_LOG(GetMangosString(LANG_CURRENT_ENERGY), chr->GetMaxPower(POWER_ENERGY));

    return true;
}

// Edit Player Rage
bool ChatHandler::HandleModifyRageCommand(char* args)
{
    if (!*args)
        return false;

    int32 rage = atoi(args) * 10;
    int32 ragem = atoi(args) * 10;

    if (rage <= 0 || ragem <= 0 || ragem < rage)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player* chr = getSelectedPlayer();
    if (chr == nullptr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // check online security
    if (HasLowerSecurity(chr))
        return false;

    PSendSysMessage(LANG_YOU_CHANGE_RAGE, GetNameLink(chr).c_str(), rage / 10, ragem / 10);
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_RAGE_CHANGED, GetNameLink().c_str(), rage / 10, ragem / 10);

    chr->SetMaxPower(POWER_RAGE, ragem);
    chr->SetPower(POWER_RAGE, rage);

    return true;
}

// Edit Player Faction
bool ChatHandler::HandleModifyFactionCommand(char* args)
{
    Creature* chr = getSelectedCreature();
    if (!chr)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if (!*args)
    {
        if (chr)
        {
            uint32 factionid = chr->getFaction();
            uint32 flag      = chr->GetUInt32Value(UNIT_FIELD_FLAGS);
            uint32 npcflag   = chr->GetUInt32Value(UNIT_NPC_FLAGS);
            uint32 dyflag    = chr->GetUInt32Value(UNIT_DYNAMIC_FLAGS);
            PSendSysMessage(LANG_CURRENT_FACTION, chr->GetGUIDLow(), factionid, flag, npcflag, dyflag);
        }
        return true;
    }

    if (!chr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 factionid;
    if (!ExtractUint32KeyFromLink(&args, "Hfaction", factionid))
        return false;

    if (!sFactionTemplateStore.LookupEntry(factionid))
    {
        PSendSysMessage(LANG_WRONG_FACTION, factionid);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 flag;
    if (!ExtractOptUInt32(&args, flag, chr->GetUInt32Value(UNIT_FIELD_FLAGS)))
        return false;

    uint32 npcflag;
    if (!ExtractOptUInt32(&args, npcflag, chr->GetUInt32Value(UNIT_NPC_FLAGS)))
        return false;

    uint32  dyflag;
    if (!ExtractOptUInt32(&args, dyflag, chr->GetUInt32Value(UNIT_DYNAMIC_FLAGS)))
        return false;

    PSendSysMessage(LANG_YOU_CHANGE_FACTION, chr->GetGUIDLow(), factionid, flag, npcflag, dyflag);

    chr->setFaction(factionid);
    chr->SetUInt32Value(UNIT_FIELD_FLAGS, flag);
    chr->SetUInt32Value(UNIT_NPC_FLAGS, npcflag);
    chr->SetUInt32Value(UNIT_DYNAMIC_FLAGS, dyflag);

    return true;
}

// Edit Player TP
bool ChatHandler::HandleModifyTalentCommand(char* args)
{
    if (!*args)
        return false;

    int tp = atoi(args);
    if (tp < 0)
        return false;

    Player* target = getSelectedPlayer();
    if (!target)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // check online security
    if (HasLowerSecurity(target))
        return false;

    target->SetFreeTalentPoints(tp);
    return true;
}

// Enable On\OFF all taxi paths
bool ChatHandler::HandleTaxiCheatCommand(char* args)
{
    bool value;
    if (!ExtractOnOff(&args, value))
    {
        SendSysMessage(LANG_USE_BOL);
        SetSentErrorMessage(true);
        return false;
    }

    Player* chr = getSelectedPlayer();
    if (!chr)
        chr = m_session->GetPlayer();
    // check online security
    else if (HasLowerSecurity(chr))
        return false;

    if (value)
    {
        chr->SetTaxiCheater(true);
        PSendSysMessage(LANG_YOU_GIVE_TAXIS, GetNameLink(chr).c_str());
        if (needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_TAXIS_ADDED, GetNameLink().c_str());
    }
    else
    {
        chr->SetTaxiCheater(false);
        PSendSysMessage(LANG_YOU_REMOVE_TAXIS, GetNameLink(chr).c_str());
        if (needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_TAXIS_REMOVED, GetNameLink().c_str());
    }

    return true;
}

// Edit Player Aspeed
bool ChatHandler::HandleModifyASpeedCommand(char* args)
{
    if (!*args)
        return false;

    float modSpeed = (float)atof(args);

    if (modSpeed > 10 || modSpeed < 0.1)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player* chr = getSelectedPlayer();
    if (chr == nullptr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // check online security
    if (HasLowerSecurity(chr))
        return false;

    std::string chrNameLink = GetNameLink(chr);

    if (chr->IsTaxiFlying())
    {
        PSendSysMessage(LANG_CHAR_IN_FLIGHT, chrNameLink.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_ASPEED, modSpeed, chrNameLink.c_str());
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_ASPEED_CHANGED, GetNameLink().c_str(), modSpeed);

    chr->UpdateSpeed(MOVE_WALK,   true, modSpeed);
    chr->UpdateSpeed(MOVE_RUN,    true, modSpeed);
    chr->UpdateSpeed(MOVE_SWIM,   true, modSpeed);
    // chr->UpdateSpeed(MOVE_TURN,   true, modSpeed);
    return true;
}

// Edit Player Speed
bool ChatHandler::HandleModifySpeedCommand(char* args)
{
    if (!*args)
        return false;

    float modSpeed = (float)atof(args);

    if (modSpeed > 10 || modSpeed < 0.1)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player* chr = getSelectedPlayer();
    if (chr == nullptr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // check online security
    if (HasLowerSecurity(chr))
        return false;

    std::string chrNameLink = GetNameLink(chr);

    if (chr->IsTaxiFlying())
    {
        PSendSysMessage(LANG_CHAR_IN_FLIGHT, chrNameLink.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_SPEED, modSpeed, chrNameLink.c_str());
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_SPEED_CHANGED, GetNameLink().c_str(), modSpeed);

    chr->UpdateSpeed(MOVE_RUN, true, modSpeed);

    return true;
}

// Edit Player Swim Speed
bool ChatHandler::HandleModifySwimCommand(char* args)
{
    if (!*args)
        return false;

    float modSpeed = (float)atof(args);

    if (modSpeed > 10.0f || modSpeed < 0.01f)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player* chr = getSelectedPlayer();
    if (chr == nullptr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // check online security
    if (HasLowerSecurity(chr))
        return false;

    std::string chrNameLink = GetNameLink(chr);

    if (chr->IsTaxiFlying())
    {
        PSendSysMessage(LANG_CHAR_IN_FLIGHT, chrNameLink.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_SWIM_SPEED, modSpeed, chrNameLink.c_str());
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_SWIM_SPEED_CHANGED, GetNameLink().c_str(), modSpeed);

    chr->UpdateSpeed(MOVE_SWIM, true, modSpeed);

    return true;
}

// Edit Player Walk Speed
bool ChatHandler::HandleModifyBWalkCommand(char* args)
{
    if (!*args)
        return false;

    float modSpeed = (float)atof(args);

    if (modSpeed > 10.0f || modSpeed < 0.1f)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player* chr = getSelectedPlayer();
    if (chr == nullptr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // check online security
    if (HasLowerSecurity(chr))
        return false;

    std::string chrNameLink = GetNameLink(chr);

    if (chr->IsTaxiFlying())
    {
        PSendSysMessage(LANG_CHAR_IN_FLIGHT, chrNameLink.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_BACK_SPEED, modSpeed, chrNameLink.c_str());
    if (needReportToTarget(chr))
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_BACK_SPEED_CHANGED, GetNameLink().c_str(), modSpeed);

    chr->UpdateSpeed(MOVE_RUN_BACK, true, modSpeed);

    return true;
}

// Edit Player Scale
bool ChatHandler::HandleModifyScaleCommand(char* args)
{
    if (!*args)
        return false;

    float Scale = (float)atof(args);
    if (Scale > 10.0f || Scale <= 0.0f)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Unit* target = getSelectedUnit();
    if (target == nullptr)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if (target->GetTypeId() == TYPEID_PLAYER)
    {
        // check online security
        if (HasLowerSecurity((Player*)target))
            return false;

        PSendSysMessage(LANG_YOU_CHANGE_SIZE, Scale, GetNameLink((Player*)target).c_str());
        if (needReportToTarget((Player*)target))
            ChatHandler((Player*)target).PSendSysMessage(LANG_YOURS_SIZE_CHANGED, GetNameLink().c_str(), Scale);
    }

    target->SetObjectScale(Scale);
    target->UpdateModelData();

    return true;
}

// Provide Player a random unusual mount
bool ChatHandler::HandleModifyMountCommand(char* args)
{
    return ModifyMountCommandHelper(getSelectedPlayer(), args);
}

// Edit Player money
bool ChatHandler::HandleModifyMoneyCommand(char* args)
{
    if (!*args)
        return false;

    Player* chr = getSelectedPlayer();
    if (chr == nullptr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // check online security
    if (HasLowerSecurity(chr))
        return false;

    int32 addmoney = atoi(args);

    uint32 moneyuser = chr->GetMoney();

    if (addmoney < 0)
    {
        int32 newmoney = int32(moneyuser) + addmoney;

        DETAIL_LOG(GetMangosString(LANG_CURRENT_MONEY), moneyuser, addmoney, newmoney);
        if (newmoney <= 0)
        {
            PSendSysMessage(LANG_YOU_TAKE_ALL_MONEY, GetNameLink(chr).c_str());
            if (needReportToTarget(chr))
                ChatHandler(chr).PSendSysMessage(LANG_YOURS_ALL_MONEY_GONE, GetNameLink().c_str());

            chr->SetMoney(0);
        }
        else
        {
            if (newmoney > MAX_MONEY_AMOUNT)
                newmoney = MAX_MONEY_AMOUNT;

            PSendSysMessage(LANG_YOU_TAKE_MONEY, abs(addmoney), GetNameLink(chr).c_str());
            if (needReportToTarget(chr))
                ChatHandler(chr).PSendSysMessage(LANG_YOURS_MONEY_TAKEN, GetNameLink().c_str(), abs(addmoney));
            chr->SetMoney(newmoney);
        }
    }
    else
    {
        PSendSysMessage(LANG_YOU_GIVE_MONEY, addmoney, GetNameLink(chr).c_str());
        if (needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_MONEY_GIVEN, GetNameLink().c_str(), addmoney);

        if (addmoney >= MAX_MONEY_AMOUNT)
            chr->SetMoney(MAX_MONEY_AMOUNT);
        else
            chr->ModifyMoney(addmoney);
    }

    DETAIL_LOG(GetMangosString(LANG_NEW_MONEY), moneyuser, addmoney, chr->GetMoney());

    return true;
}

bool ChatHandler::HandleTeleCommand(char* args)
{
    if (!*args)
        return false;

    Player* _player = m_session->GetPlayer();

    // id, or string, or [name] Shift-click form |color|Htele:id|h[name]|h|r
    GameTele const* tele = ExtractGameTeleFromLink(&args);

    if (!tele)
    {
        SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    return HandleGoHelper(_player, tele->mapId, tele->position_x, tele->position_y, &tele->position_z, &tele->orientation);
}

bool ChatHandler::HandleLookupAreaCommand(char* args)
{
    if (!*args)
        return false;

    std::string namepart = args;
    std::wstring wnamepart;

    if (!Utf8toWStr(namepart, wnamepart))
        return false;

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    // converting string that we try to find to lower case
    wstrToLower(wnamepart);

    // Search in AreaTable.dbc
    for (uint32 areaid = 0; areaid <= sAreaStore.GetNumRows(); ++areaid)
    {
        AreaTableEntry const* areaEntry = sAreaStore.LookupEntry(areaid);
        if (areaEntry)
        {
            int loc = GetSessionDbcLocale();
            std::string name = areaEntry->area_name[loc];
            if (name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for (; loc < MAX_LOCALE; ++loc)
                {
                    if (loc == GetSessionDbcLocale())
                        continue;

                    name = areaEntry->area_name[loc];
                    if (name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if (loc < MAX_LOCALE)
            {
                // send area in "id - [name]" format
                std::ostringstream ss;
                if (m_session)
                    ss << areaEntry->ID << " - |cffffffff|Harea:" << areaEntry->ID << "|h[" << name << " " << localeNames[loc] << "]|h|r";
                else
                    ss << areaEntry->ID << " - " << name << " " << localeNames[loc];

                SendSysMessage(ss.str().c_str());

                ++counter;
            }
        }
    }

    if (counter == 0)                                      // if counter == 0 then we found nth
        SendSysMessage(LANG_COMMAND_NOAREAFOUND);

    return true;
}

// Find tele in game_tele order by name
bool ChatHandler::HandleLookupTeleCommand(char* args)
{
    if (!*args)
    {
        SendSysMessage(LANG_COMMAND_TELE_PARAMETER);
        SetSentErrorMessage(true);
        return false;
    }

    std::string namepart = args;
    std::wstring wnamepart;

    if (!Utf8toWStr(namepart, wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower(wnamepart);

    std::ostringstream reply;

    GameTeleMap const& teleMap = sObjectMgr.GetGameTeleMap();
    for (const auto& itr : teleMap)
    {
        GameTele const* tele = &itr.second;

        if (tele->wnameLow.find(wnamepart) == std::wstring::npos)
            continue;

        if (m_session)
            reply << "  |cffffffff|Htele:" << itr.first << "|h[" << tele->name << "]|h|r\n";
        else
            reply << "  " << itr.first << " " << tele->name << "\n";
    }

    if (reply.str().empty())
        SendSysMessage(LANG_COMMAND_TELE_NOLOCATION);
    else
        PSendSysMessage(LANG_COMMAND_TELE_LOCATION, reply.str().c_str());

    return true;
}

// Enable\Dissable accept whispers (for GM)
bool ChatHandler::HandleWhispersCommand(char* args)
{
    if (!*args)
    {
        PSendSysMessage(LANG_COMMAND_WHISPERACCEPTING, GetOnOffStr(m_session->GetPlayer()->isAcceptWhispers()));
        return true;
    }

    bool value;
    if (!ExtractOnOff(&args, value))
    {
        SendSysMessage(LANG_USE_BOL);
        SetSentErrorMessage(true);
        return false;
    }

    // whisper on
    if (value)
    {
        m_session->GetPlayer()->SetAcceptWhispers(true);
        SendSysMessage(LANG_COMMAND_WHISPERON);
    }
    // whisper off
    else
    {
        m_session->GetPlayer()->SetAcceptWhispers(false);
        SendSysMessage(LANG_COMMAND_WHISPEROFF);
    }

    return true;
}

// Save all players in the world
bool ChatHandler::HandleSaveAllCommand(char* /*args*/)
{
    sObjectAccessor.SaveAllPlayers();
    SendSysMessage(LANG_PLAYERS_SAVED);
    return true;
}

// Send mail by command
bool ChatHandler::HandleSendMailCommand(char* args)
{
    // format: name "subject text" "mail text"
    Player* target;
    ObjectGuid target_guid;
    std::string target_name;
    if (!ExtractPlayerTarget(&args, &target, &target_guid, &target_name))
        return false;

    MailDraft draft;

    // fill draft
    if (!HandleSendMailHelper(draft, args))
        return false;

    // from console show nonexistent sender
    MailSender sender(MAIL_NORMAL, m_session ? m_session->GetPlayer()->GetObjectGuid().GetCounter() : 0, MAIL_STATIONERY_GM);

    draft.SendMailTo(MailReceiver(target, target_guid), sender);

    std::string nameLink = playerLink(target_name);
    PSendSysMessage(LANG_MAIL_SENT, nameLink.c_str());
    return true;
}

// teleport player to given game_tele.entry
bool ChatHandler::HandleTeleNameCommand(char* args)
{
    char* nameStr = ExtractOptNotLastArg(&args);

    Player* target;
    ObjectGuid target_guid;
    std::string target_name;
    if (!ExtractPlayerTarget(&nameStr, &target, &target_guid, &target_name))
        return false;

    // id, or string, or [name] Shift-click form |color|Htele:id|h[name]|h|r
    GameTele const* tele = ExtractGameTeleFromLink(&args);
    if (!tele)
    {
        SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    if (target)
    {
        // check online security
        if (HasLowerSecurity(target))
            return false;

        std::string chrNameLink = playerLink(target_name);

        if (target->IsBeingTeleported())
        {
            PSendSysMessage(LANG_IS_TELEPORTED, chrNameLink.c_str());
            SetSentErrorMessage(true);
            return false;
        }

        PSendSysMessage(LANG_TELEPORTING_TO, chrNameLink.c_str(), "", tele->name.c_str());
        if (needReportToTarget(target))
            ChatHandler(target).PSendSysMessage(LANG_TELEPORTED_TO_BY, GetNameLink().c_str());

        return HandleGoHelper(target, tele->mapId, tele->position_x, tele->position_y, &tele->position_z, &tele->orientation);
    }
    // check offline security
    if (HasLowerSecurity(nullptr, target_guid))
        return false;

    std::string nameLink = playerLink(target_name);

    PSendSysMessage(LANG_TELEPORTING_TO, nameLink.c_str(), GetMangosString(LANG_OFFLINE), tele->name.c_str());
    Player::SavePositionInDB(target_guid, tele->mapId,
        tele->position_x, tele->position_y, tele->position_z, tele->orientation,
        sTerrainMgr.GetZoneId(tele->mapId, tele->position_x, tele->position_y, tele->position_z));

    return true;
}

// Teleport group to given game_tele.entry
bool ChatHandler::HandleTeleGroupCommand(char* args)
{
    if (!*args)
        return false;

    Player* player = getSelectedPlayer();
    if (!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // check online security
    if (HasLowerSecurity(player))
        return false;

    // id, or string, or [name] Shift-click form |color|Htele:id|h[name]|h|r
    GameTele const* tele = ExtractGameTeleFromLink(&args);
    if (!tele)
    {
        SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    std::string nameLink = GetNameLink(player);

    Group* grp = player->GetGroup();
    if (!grp)
    {
        PSendSysMessage(LANG_NOT_IN_GROUP, nameLink.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    for (GroupReference* itr = grp->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* pl = itr->getSource();

        if (!pl || !pl->GetSession())
            continue;

        // check online security
        if (HasLowerSecurity(pl))
            return false;

        std::string plNameLink = GetNameLink(pl);

        if (pl->IsBeingTeleported())
        {
            PSendSysMessage(LANG_IS_TELEPORTED, plNameLink.c_str());
            continue;
        }

        PSendSysMessage(LANG_TELEPORTING_TO, plNameLink.c_str(), "", tele->name.c_str());
        if (needReportToTarget(pl))
            ChatHandler(pl).PSendSysMessage(LANG_TELEPORTED_TO_BY, nameLink.c_str());

        // stop flight if need
        if (!pl->TaxiFlightInterrupt())
            pl->SaveRecallPosition();

        pl->TeleportTo(tele->mapId, tele->position_x, tele->position_y, tele->position_z, tele->orientation);
    }

    return true;
}

// Summon group of player
bool ChatHandler::HandleGroupgoCommand(char* args)
{
    Player* target;
    if (!ExtractPlayerTarget(&args, &target))
        return false;

    // check online security
    if (HasLowerSecurity(target))
        return false;

    Group* grp = target->GetGroup();

    std::string nameLink = GetNameLink(target);

    if (!grp)
    {
        PSendSysMessage(LANG_NOT_IN_GROUP, nameLink.c_str());
        SetSentErrorMessage(true);
        return false;
    }

    Player* player = m_session->GetPlayer();
    Map* gmMap = player->GetMap();
    bool to_instance =  gmMap->Instanceable();

    // we are in instance, and can summon only player in our group with us as lead
    if (to_instance && (
                !player->GetGroup() || (grp->GetLeaderGuid() != player->GetObjectGuid()) ||
                (player->GetGroup()->GetLeaderGuid() != player->GetObjectGuid())))
        // the last check is a bit excessive, but let it be, just in case
    {
        SendSysMessage(LANG_CANNOT_SUMMON_TO_INST);
        SetSentErrorMessage(true);
        return false;
    }

    for (GroupReference* itr = grp->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* pl = itr->getSource();

        if (!pl || pl == m_session->GetPlayer() || !pl->GetSession())
            continue;

        // check online security
        if (HasLowerSecurity(pl))
            return false;

        std::string plNameLink = GetNameLink(pl);

        if (pl->IsBeingTeleported())
        {
            PSendSysMessage(LANG_IS_TELEPORTED, plNameLink.c_str());
            SetSentErrorMessage(true);
            return false;
        }

        if (to_instance)
        {
            Map* plMap = pl->GetMap();

            if (plMap->Instanceable() && plMap->GetInstanceId() != gmMap->GetInstanceId())
            {
                // cannot summon from instance to instance
                PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST, plNameLink.c_str());
                SetSentErrorMessage(true);
                return false;
            }
        }

        PSendSysMessage(LANG_SUMMONING, plNameLink.c_str(), "");
        if (needReportToTarget(pl))
            ChatHandler(pl).PSendSysMessage(LANG_SUMMONED_BY, nameLink.c_str());

        // stop flight if need
        if (!pl->TaxiFlightInterrupt())
            pl->SaveRecallPosition();

        // before GM
        float x, y, z;
        m_session->GetPlayer()->GetClosePoint(x, y, z, pl->GetObjectBoundingRadius());
        pl->TeleportTo(m_session->GetPlayer()->GetMapId(), x, y, z, pl->GetOrientation());
    }

    return true;
}

bool ChatHandler::HandleGoHelper(Player* player, uint32 mapid, float x, float y, float const* zPtr, float const* ortPtr)
{
    float z;
    float ort = player->GetOrientation();

    if (zPtr)
    {
        z = *zPtr;

        if (ortPtr)
            ort = *ortPtr;

        // check full provided coordinates
        if (!MapManager::IsValidMapCoord(mapid, x, y, z, ort))
        {
            PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapid);
            SetSentErrorMessage(true);
            return false;
        }
    }
    else
    {
        // we need check x,y before ask Z or can crash at invalide coordinates
        if (!MapManager::IsValidMapCoord(mapid, x, y))
        {
            PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapid);
            SetSentErrorMessage(true);
            return false;
        }

        TerrainInfo const* map = sTerrainMgr.LoadTerrain(mapid);
        float groundZ = player->GetMap()->GetHeight(x, y, 0.f);
        z = map->GetWaterOrGroundLevel(x, y, MAX_HEIGHT, groundZ);
    }

    // stop flight if need
    if (!player->TaxiFlightInterrupt())
        player->SaveRecallPosition();

    player->TeleportTo(mapid, x, y, z, ort);

    return true;
}

bool ChatHandler::HandleGoTaxinodeCommand(char* args)
{
    Player* _player = m_session->GetPlayer();

    uint32 nodeId;
    if (!ExtractUint32KeyFromLink(&args, "Htaxinode", nodeId))
        return false;

    TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(nodeId);
    if (!node)
    {
        PSendSysMessage(LANG_COMMAND_GOTAXINODENOTFOUND, nodeId);
        SetSentErrorMessage(true);
        return false;
    }

    if (node->x == 0.0f && node->y == 0.0f && node->z == 0.0f)
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD, node->x, node->y, node->map_id);
        SetSentErrorMessage(true);
        return false;
    }

    return HandleGoHelper(_player, node->map_id, node->x, node->y, &node->z);
}

bool ChatHandler::HandleGoCommand(char* args)
{
    if (!*args)
        return false;

    Player* _player = m_session->GetPlayer();

    uint32 mapid;
    float x, y, z;

    // raw coordinates case
    if (ExtractFloat(&args, x))
    {
        if (!ExtractFloat(&args, y))
            return false;

        if (!ExtractFloat(&args, z))
            return false;

        if (!ExtractOptUInt32(&args, mapid, _player->GetMapId()))
            return false;
    }
    // link case
    else if (!ExtractLocationFromLink(&args, mapid, x, y, z))
        return false;

    return HandleGoHelper(_player, mapid, x, y, &z);
}



// teleport at coordinates
bool ChatHandler::HandleGoXYCommand(char* args)
{
    Player* _player = m_session->GetPlayer();

    float x;
    if (!ExtractFloat(&args, x))
        return false;

    float y;
    if (!ExtractFloat(&args, y))
        return false;

    uint32 mapid;
    if (!ExtractOptUInt32(&args, mapid, _player->GetMapId()))
        return false;

    return HandleGoHelper(_player, mapid, x, y);
}

// teleport at coordinates, including Z
bool ChatHandler::HandleGoXYZCommand(char* args)
{
    if (!*args)
        return false;

    Player* _player = m_session->GetPlayer();

    char* px = strtok((char*)args, " ");
    char* py = strtok(nullptr, " ");
    char* pz = strtok(nullptr, " ");
    char* pmapid = strtok(nullptr, " ");

    if (!px || !py || !pz)
        return false;

    float x = (float)atof(px);
    float y = (float)atof(py);
    float z = (float)atof(pz);
    uint32 mapid;
    if (pmapid)
        mapid = (uint32)atoi(pmapid);
    else
        mapid = _player->GetMapId();

    if (!MapManager::IsValidMapCoord(mapid, x, y, z))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, mapid);
        SetSentErrorMessage(true);
        return false;
    }

    return HandleGoHelper(_player, mapid, x, y, &z);
}

// teleport at coordinates
bool ChatHandler::HandleGoZoneXYCommand(char* args)
{
    Player* _player = m_session->GetPlayer();

    float x;
    if (!ExtractFloat(&args, x))
        return false;

    float y;
    if (!ExtractFloat(&args, y))
        return false;

    uint32 areaid;
    if (*args)
    {
        if (!ExtractUint32KeyFromLink(&args, "Harea", areaid))
            return false;
    }
    else
        areaid = _player->GetZoneId();

    AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(areaid);

    if (x < 0 || x > 100 || y < 0 || y > 100 || !areaEntry)
    {
        PSendSysMessage(LANG_INVALID_ZONE_COORD, x, y, areaid);
        SetSentErrorMessage(true);
        return false;
    }

    // update to parent zone if exist (client map show only zones without parents)
    AreaTableEntry const* zoneEntry = areaEntry->zone ? GetAreaEntryByAreaID(areaEntry->zone) : areaEntry;

    MapEntry const* mapEntry = sMapStore.LookupEntry(zoneEntry->mapid);

    if (mapEntry->Instanceable())
    {
        PSendSysMessage(LANG_INVALID_ZONE_MAP, areaEntry->ID, areaEntry->area_name[GetSessionDbcLocale()],
                        mapEntry->MapID, mapEntry->name[GetSessionDbcLocale()]);
        SetSentErrorMessage(true);
        return false;
    }

    if (!Zone2MapCoordinates(x, y, zoneEntry->ID))
    {
        PSendSysMessage(LANG_INVALID_ZONE_MAP, areaEntry->ID, areaEntry->area_name[GetSessionDbcLocale()],
                        mapEntry->MapID, mapEntry->name[GetSessionDbcLocale()]);
        SetSentErrorMessage(true);
        return false;
    }

    return HandleGoHelper(_player, mapEntry->MapID, x, y);
}

// teleport to grid
bool ChatHandler::HandleGoGridCommand(char* args)
{
    Player* _player = m_session->GetPlayer();

    float grid_x;
    if (!ExtractFloat(&args, grid_x))
        return false;

    float grid_y;
    if (!ExtractFloat(&args, grid_y))
        return false;

    uint32 mapid;
    if (!ExtractOptUInt32(&args, mapid, _player->GetMapId()))
        return false;

    // center of grid
    float x = (grid_x - CENTER_GRID_ID + 0.5f) * SIZE_OF_GRIDS;
    float y = (grid_y - CENTER_GRID_ID + 0.5f) * SIZE_OF_GRIDS;

    return HandleGoHelper(_player, mapid, x, y);
}

bool ChatHandler::HandleModifyDrunkCommand(char* args)
{
    if (!*args)    return false;

    uint32 drunklevel = (uint32)atoi(args);
    if (drunklevel > 100)
        drunklevel = 100;

    uint16 drunkMod = drunklevel * 0xFFFF / 100;

    m_session->GetPlayer()->SetDrunkValue(drunkMod);

    return true;
}

// Enables or disables hiding of the staff badge
bool ChatHandler::HandleGMChatCommand(char* args)
{
    if (!*args)
    {
        if (m_session->GetPlayer()->isGMChat())
            m_session->SendNotification(LANG_GM_CHAT_ON);
        else
            m_session->SendNotification(LANG_GM_CHAT_OFF);
        return true;
    }

    bool value;
    if (!ExtractOnOff(&args, value))
    {
        SendSysMessage(LANG_USE_BOL);
        SetSentErrorMessage(true);
        return false;
    }

    if (value)
    {
        m_session->GetPlayer()->SetGMChat(true);
        m_session->SendNotification(LANG_GM_CHAT_ON);
    }
    else
    {
        m_session->GetPlayer()->SetGMChat(false);
        m_session->SendNotification(LANG_GM_CHAT_OFF);
    }

    return true;
}

bool ChatHandler::ModifyMountCommandHelper(Player* target, char* args)
{
    if (!target)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (HasLowerSecurity(target))
        return false;

    if (target->IsTaxiFlying() || target->IsFlying())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    bool fast = false, slow = false;

    if (ExtractLiteralArg(&args, "fast"))
        fast = true;
    else if (ExtractLiteralArg(&args, "slow"))
        slow = true;
    else
    {
        const uint32 level = target->getLevel();
        fast = (level >= 60);
        slow = (!fast && level >= 40);
    }

    std::deque<uint32> pool;

    // Land mounts - rarity is proportional to security level of the invoker
    switch (m_session->GetSecurity())
    {
        case SEC_ADMINISTRATOR:     // Mounts that were never obtainable and have unique properties
        {
            if (fast)
                pool.push_back(24576);      // Chromatic Mount
        }
        case SEC_GAMEMASTER:        // Mounts that were never obtainable by players, cut during development
        {
            if (fast)
            {
                if (target->GetTeam() == ALLIANCE)
                    pool.push_back(23220);  // Swift Dawnsaber
            }
            else if (slow)
            {
                if (target->GetTeam() == ALLIANCE)
                {
                    pool.push_back(468);    // White Stallion
                    pool.push_back(471);    // Palomino Stallion
                    pool.push_back(6896);   // Black Ram
                    pool.push_back(6897);   // Blue Ram
                    pool.push_back(10787);  // Panther
                    pool.push_back(10788);  // Leopard
                    pool.push_back(10790);  // Tiger
                    pool.push_back(10792);  // Spotted Panther
                    pool.push_back(15781);  // Steel Mechanostrider
                    pool.push_back(16058);  // Primal Leopard
                    pool.push_back(16059);  // Tawny Sabercat
                    pool.push_back(16060);  // Golden Sabercat
                    pool.push_back(17455);  // Purple Mechanostrider
                    pool.push_back(17456);  // Red & Blue Mechanostrider
                    pool.push_back(17458);  // Fluorescent Green Mechanostrider
                }
                else
                {
                    pool.push_back(459);    // Gray Wolf
                    pool.push_back(578);    // Black Wolf
                    pool.push_back(581);    // Winter Wolf
                    pool.push_back(8980);   // Skeletal Horse
                    pool.push_back(10795);  // Ivory Raptor
                    pool.push_back(10798);  // Obsidian Raptor
                    pool.push_back(18363);  // Riding Kodo
                }

                pool.push_back(25675);      // Reindeer
            }
            else
            {
                pool.push_back(10800);      // Summon Brown Tallstrider
                pool.push_back(10801);      // Summon Gray Tallstrider
                pool.push_back(10802);      // Summon Pink Tallstrider
                pool.push_back(10803);      // Summon Purple Tallstrider
                pool.push_back(10804);      // Summon Turquoise Tallstrider
            }
        }
        case SEC_MODERATOR:         // Mounts that were obtainable by players at some pont in the past and now extremely rare
        {
            if (fast)
            {
                if (target->GetTeam() == ALLIANCE)
                {
                    pool.push_back(15779);  // White Mechanostrider
                    pool.push_back(16055);  // Nightsaber
                    pool.push_back(16056);  // Frostsaber
                    pool.push_back(16082);  // Palomino Stallion
                    pool.push_back(16083);  // White Stallion
                    pool.push_back(17459);  // Icy Blue Mechanostrider
                    pool.push_back(17460);  // Frost Ram
                    pool.push_back(17461);  // Black Ram
                }
                else
                {
                    pool.push_back(16080);  // Red Wolf
                    pool.push_back(16081);  // Arctic Wolf
                    pool.push_back(16084);  // Mottled Red Raptor
                    pool.push_back(17450);  // Ivory Raptor
                    pool.push_back(18991);  // Green Kodo
                    pool.push_back(18992);  // Teal Kodo
                };

                pool.push_back(26656);      // Summon Black Qiraji Battle Tank
            }
        }
        default:                        // Mounts that are obtainable by players but rare or somewhat restricted
        {
            if (fast)
            {
                pool.push_back(17229);      // Winterspring Frostsaber
                pool.push_back(17481);      // Deathcharger
                pool.push_back(24242);      // Swift Razzashi Raptor
                pool.push_back(24252);      // Swift Zulian Tiger
                pool.push_back(25859);      // Reindeer
            }
            else if (slow)
                pool.push_back(25858);      // Reindeer
            else
                pool.push_back(30174);      // Riding Turtle
        }
    }

    const uint32 flags = (TRIGGERED_OLD_TRIGGERED | TRIGGERED_INSTANT_CAST | TRIGGERED_DO_NOT_PROC);

    // Cast a random unusual in-game mount
    while (!pool.empty())
    {
        uint32 index = urand(0, (pool.size() - 1));
        uint32 spellid = pool.at(index);

        if (!target->HasAura(spellid))
        {
            if (SPELL_CAST_OK == target->CastSpell(target, spellid, flags))
            {
                PSendSysMessage(LANG_YOU_GIVE_MOUNT, GetNameLink(target).c_str());

                if (needReportToTarget(target))
                    ChatHandler(target).PSendSysMessage(LANG_MOUNT_GIVED, GetNameLink().c_str());

                return true;
            }
        }

        pool.erase(pool.begin() + index);
    }

    SendSysMessage(LANG_NO_MOUNT);
    SetSentErrorMessage(true);
    return false;
}

bool ChatHandler::HandleSetViewCommand(char* /*args*/)
{
    if (Unit* unit = getSelectedUnit())
        m_session->GetPlayer()->GetCamera().SetView(unit);
    else
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    return true;
}

bool ChatHandler::HandleChannelListCommand(char* args)
{
    uint32 max = 10;

    ExtractUInt32(&args, max);
    const bool statics = ExtractLiteralArg(&args, "static");

    auto const& map = channelMgr(GetSession()->GetPlayer()->GetTeam())->GetChannels();

    std::list<Channel const*> list;

    for (auto const& pair : map)
    {
        if (pair.second->IsConstant() || pair.second->IsStatic() != statics)
            continue;

        list.push_back(pair.second);
    }

    if (list.empty())
        PSendSysMessage(LANG_COMMAND_CHANNELS_NO_CHANNELS);
    else
    {
        PSendSysMessage(LANG_COMMAND_CHANNELS_LIST_HEADER, max);

        list.sort([] (Channel const* a, Channel const* b) { return (a->GetNumPlayers() > b->GetNumPlayers()); });

        const size_t count = std::min(list.size(), size_t(max));

        size_t i = 0;

        for (auto itr = list.begin(); (itr != list.end() && i < count); ++itr, ++i)
        {
            std::ostringstream output;

            output << "* " << '"' << (*itr)->GetName() << '"' << " - "  << (*itr)->GetNumPlayers();

            if ((*itr)->IsStatic())
                output << " " << GetMangosString(LANG_CHANNEL_CUSTOM_DETAILS_STATIC);

            if (!(*itr)->GetPassword().empty())
                output << " " << GetMangosString(LANG_CHANNEL_CUSTOM_DETAILS_PASSWORD);

            PSendSysMessage("%s", output.str().c_str());
        }
    }

    return true;
}

bool ChatHandler::HandleChannelStaticCommand(char* args)
{
    char* name = ExtractLiteralArg(&args);

    if (!name)
        return false;

    bool state;

    if (!ExtractOnOff(&args, state))
        return false;

    Player* player = GetSession()->GetPlayer();
    ChannelMgr* manager = (player ? channelMgr(player->GetTeam()) : nullptr);
    Channel* channel = (name && manager ? manager->GetChannel(name, player) : nullptr);

    if (!channel)
    {
        // Error sent via packet by ChannelMgr::GetChannel()
        SetSentErrorMessage(true);
        return false;
    }
    else if (channel->IsStatic() != state)
    {
        if (!channel->SetStatic(state, true))
        {
            if (!channel->GetPassword().empty())
                PSendSysMessage(LANG_COMMAND_CHANNEL_STATIC_PASSWORD, channel->GetName().c_str());
            else
                PSendSysMessage(LANG_COMMAND_CHANNEL_STATIC_GLOBAL, channel->GetName().c_str());
            SetSentErrorMessage(true);
            return false;
        }
        PSendSysMessage(LANG_COMMAND_CHANNEL_STATIC_SUCCESS, channel->GetName().c_str(), GetMangosString((state ? LANG_ON : LANG_OFF)));
    }

    return true;
}
