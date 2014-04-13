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

#include "Player.h"
#include "Language.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "Opcodes.h"
#include "SpellMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "UpdateMask.h"
#include "QuestDef.h"
#include "GossipDef.h"
#include "UpdateData.h"
#include "Channel.h"
#include "ChannelMgr.h"
#include "MapManager.h"
#include "MapPersistentStateMgr.h"
#include "InstanceData.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "ObjectMgr.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "Formulas.h"
#include "Group.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Pet.h"
#include "Util.h"
#include "Transports.h"
#include "Weather.h"
#include "BattleGround/BattleGround.h"
#include "BattleGround/BattleGroundMgr.h"
#include "BattleGround/BattleGroundAV.h"
#include "OutdoorPvP/OutdoorPvP.h"
#include "Chat.h"
#include "Database/DatabaseImpl.h"
#include "Spell.h"
#include "ScriptMgr.h"
#include "SocialMgr.h"
#include "Mail.h"
#include "DBCStores.h"
#include "SQLStorages.h"
#include "HookMgr.h"

#include <cmath>

#define ZONE_UPDATE_INTERVAL (1*IN_MILLISECONDS)

#define PLAYER_SKILL_INDEX(x)       (PLAYER_SKILL_INFO_1_1 + ((x)*3))
#define PLAYER_SKILL_VALUE_INDEX(x) (PLAYER_SKILL_INDEX(x)+1)
#define PLAYER_SKILL_BONUS_INDEX(x) (PLAYER_SKILL_INDEX(x)+2)

#define SKILL_VALUE(x)         PAIR32_LOPART(x)
#define SKILL_MAX(x)           PAIR32_HIPART(x)
#define MAKE_SKILL_VALUE(v, m) MAKE_PAIR32(v,m)

#define SKILL_TEMP_BONUS(x)    int16(PAIR32_LOPART(x))
#define SKILL_PERM_BONUS(x)    int16(PAIR32_HIPART(x))
#define MAKE_SKILL_BONUS(t, p) MAKE_PAIR32(t,p)

// [-ZERO] need recheck, some values known not existed in 1.12.1
enum CharacterFlags
{
    CHARACTER_FLAG_NONE                 = 0x00000000,
    CHARACTER_FLAG_UNK1                 = 0x00000001,
    CHARACTER_FLAG_UNK2                 = 0x00000002,
    CHARACTER_LOCKED_FOR_TRANSFER       = 0x00000004,
    CHARACTER_FLAG_UNK4                 = 0x00000008,
    CHARACTER_FLAG_UNK5                 = 0x00000010,
    CHARACTER_FLAG_UNK6                 = 0x00000020,
    CHARACTER_FLAG_UNK7                 = 0x00000040,
    CHARACTER_FLAG_UNK8                 = 0x00000080,
    CHARACTER_FLAG_UNK9                 = 0x00000100,
    CHARACTER_FLAG_UNK10                = 0x00000200,
    CHARACTER_FLAG_HIDE_HELM            = 0x00000400,
    CHARACTER_FLAG_HIDE_CLOAK           = 0x00000800,
    CHARACTER_FLAG_UNK13                = 0x00001000,
    CHARACTER_FLAG_GHOST                = 0x00002000,
    CHARACTER_FLAG_RENAME               = 0x00004000,
    CHARACTER_FLAG_UNK16                = 0x00008000,
    CHARACTER_FLAG_UNK17                = 0x00010000,
    CHARACTER_FLAG_UNK18                = 0x00020000,
    CHARACTER_FLAG_UNK19                = 0x00040000,
    CHARACTER_FLAG_UNK20                = 0x00080000,
    CHARACTER_FLAG_UNK21                = 0x00100000,
    CHARACTER_FLAG_UNK22                = 0x00200000,
    CHARACTER_FLAG_UNK23                = 0x00400000,
    CHARACTER_FLAG_UNK24                = 0x00800000,
    CHARACTER_FLAG_LOCKED_BY_BILLING    = 0x01000000,
    CHARACTER_FLAG_DECLINED             = 0x02000000,
    CHARACTER_FLAG_UNK27                = 0x04000000,
    CHARACTER_FLAG_UNK28                = 0x08000000,
    CHARACTER_FLAG_UNK29                = 0x10000000,
    CHARACTER_FLAG_UNK30                = 0x20000000,
    CHARACTER_FLAG_UNK31                = 0x40000000,
    CHARACTER_FLAG_UNK32                = 0x80000000
};

// corpse reclaim times
#define DEATH_EXPIRE_STEP (5*MINUTE)
#define MAX_DEATH_COUNT 3

static const uint32 corpseReclaimDelay[MAX_DEATH_COUNT] = {30, 60, 120};

//== PlayerTaxi ================================================

PlayerTaxi::PlayerTaxi()
{
    // Taxi nodes
    memset(m_taximask, 0, sizeof(m_taximask));
}

void PlayerTaxi::InitTaxiNodes(uint32 race, uint32 level)
{
    memset(m_taximask, 0, sizeof(m_taximask));
    // capital and taxi hub masks
    ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(race);
    m_taximask[0] = rEntry->startingTaxiMask;
}

void PlayerTaxi::LoadTaxiMask(const char* data)
{
    Tokens tokens = StrSplit(data, " ");

    int index;
    Tokens::iterator iter;
    for (iter = tokens.begin(), index = 0;
            (index < TaxiMaskSize) && (iter != tokens.end()); ++iter, ++index)
    {
        // load and set bits only for existing taxi nodes
        m_taximask[index] = sTaxiNodesMask[index] & uint32(atol((*iter).c_str()));
    }
}

void PlayerTaxi::AppendTaximaskTo(ByteBuffer& data, bool all)
{
    if (all)
    {
        for (uint8 i = 0; i < TaxiMaskSize; ++i)
            data << uint32(sTaxiNodesMask[i]);              // all existing nodes
    }
    else
    {
        for (uint8 i = 0; i < TaxiMaskSize; ++i)
            data << uint32(m_taximask[i]);                  // known nodes
    }
}

bool PlayerTaxi::LoadTaxiDestinationsFromString(const std::string& values, Team team)
{
    ClearTaxiDestinations();

    Tokens tokens = StrSplit(values, " ");

    for (Tokens::iterator iter = tokens.begin(); iter != tokens.end(); ++iter)
    {
        uint32 node = uint32(atol(iter->c_str()));
        AddTaxiDestination(node);
    }

    if (m_TaxiDestinations.empty())
        return true;

    // Check integrity
    if (m_TaxiDestinations.size() < 2)
        return false;

    for (size_t i = 1; i < m_TaxiDestinations.size(); ++i)
    {
        uint32 cost;
        uint32 path;
        sObjectMgr.GetTaxiPath(m_TaxiDestinations[i - 1], m_TaxiDestinations[i], path, cost);
        if (!path)
            return false;
    }

    // can't load taxi path without mount set (quest taxi path?)
    if (!sObjectMgr.GetTaxiMountDisplayId(GetTaxiSource(), team, true))
        return false;

    return true;
}

std::string PlayerTaxi::SaveTaxiDestinationsToString()
{
    if (m_TaxiDestinations.empty())
        return "";

    std::ostringstream ss;

    for (size_t i = 0; i < m_TaxiDestinations.size(); ++i)
        ss << m_TaxiDestinations[i] << " ";

    return ss.str();
}

uint32 PlayerTaxi::GetCurrentTaxiPath() const
{
    if (m_TaxiDestinations.size() < 2)
        return 0;

    uint32 path;
    uint32 cost;

    sObjectMgr.GetTaxiPath(m_TaxiDestinations[0], m_TaxiDestinations[1], path, cost);

    return path;
}

std::ostringstream& operator<< (std::ostringstream& ss, PlayerTaxi const& taxi)
{
    for (int i = 0; i < TaxiMaskSize; ++i)
        ss << taxi.m_taximask[i] << " ";
    return ss;
}

SpellModifier::SpellModifier(SpellModOp _op, SpellModType _type, int32 _value, SpellEntry const* spellEntry, SpellEffectIndex eff, int16 _charges /*= 0*/) : op(_op), type(_type), charges(_charges), value(_value), spellId(spellEntry->Id), lastAffected(NULL)
{
    mask = sSpellMgr.GetSpellAffectMask(spellEntry->Id, eff);
}

SpellModifier::SpellModifier(SpellModOp _op, SpellModType _type, int32 _value, Aura const* aura, int16 _charges /*= 0*/) : op(_op), type(_type), charges(_charges), value(_value), spellId(aura->GetId()), lastAffected(NULL)
{
    mask = sSpellMgr.GetSpellAffectMask(aura->GetId(), aura->GetEffIndex());
}

bool SpellModifier::isAffectedOnSpell(SpellEntry const* spell) const
{
    SpellEntry const* affect_spell = sSpellStore.LookupEntry(spellId);
    // False if affect_spell == NULL or spellFamily not equal
    if (!affect_spell || affect_spell->SpellFamilyName != spell->SpellFamilyName)
        return false;
    return spell->IsFitToFamilyMask(mask);
}

//== TradeData =================================================

TradeData* TradeData::GetTraderData() const
{
    return m_trader->GetTradeData();
}

Item* TradeData::GetItem(TradeSlots slot) const
{
    return m_items[slot] ? m_player->GetItemByGuid(m_items[slot]) : NULL;
}

bool TradeData::HasItem(ObjectGuid item_guid) const
{
    for (int i = 0; i < TRADE_SLOT_COUNT; ++i)
        if (m_items[i] == item_guid)
            return true;
    return false;
}


Item* TradeData::GetSpellCastItem() const
{
    return m_spellCastItem ?  m_player->GetItemByGuid(m_spellCastItem) : NULL;
}

void TradeData::SetItem(TradeSlots slot, Item* item)
{
    ObjectGuid itemGuid = item ? item->GetObjectGuid() : ObjectGuid();

    if (m_items[slot] == itemGuid)
        return;

    m_items[slot] = itemGuid;

    SetAccepted(false);
    GetTraderData()->SetAccepted(false);

    Update();

    // need remove possible trader spell applied to changed item
    if (slot == TRADE_SLOT_NONTRADED)
        GetTraderData()->SetSpell(0);

    // need remove possible player spell applied (possible move reagent)
    SetSpell(0);
}

void TradeData::SetSpell(uint32 spell_id, Item* castItem /*= NULL*/)
{
    ObjectGuid itemGuid = castItem ? castItem->GetObjectGuid() : ObjectGuid();

    if (m_spell == spell_id && m_spellCastItem == itemGuid)
        return;

    m_spell = spell_id;
    m_spellCastItem = itemGuid;

    SetAccepted(false);
    GetTraderData()->SetAccepted(false);

    Update(true);                                           // send spell info to item owner
    Update(false);                                          // send spell info to caster self
}

void TradeData::SetMoney(uint32 money)
{
    if (m_money == money)
        return;

    m_money = money;

    SetAccepted(false);
    GetTraderData()->SetAccepted(false);

    Update();
}

void TradeData::Update(bool for_trader /*= true*/)
{
    if (for_trader)
        m_trader->GetSession()->SendUpdateTrade(true);      // player state for trader
    else
        m_player->GetSession()->SendUpdateTrade(false);     // player state for player
}

void TradeData::SetAccepted(bool state, bool crosssend /*= false*/)
{
    m_accepted = state;

    if (!state)
    {
        if (crosssend)
            m_trader->GetSession()->SendTradeStatus(TRADE_STATUS_BACK_TO_TRADE);
        else
            m_player->GetSession()->SendTradeStatus(TRADE_STATUS_BACK_TO_TRADE);
    }
}

//== Player ====================================================

UpdateMask Player::updateVisualBits;

Player::Player(WorldSession* session): Unit(), m_mover(this), m_camera(this), m_reputationMgr(this)
{
    m_transport = 0;

    m_speakTime = 0;
    m_speakCount = 0;

    m_objectType |= TYPEMASK_PLAYER;
    m_objectTypeId = TYPEID_PLAYER;

    m_valuesCount = PLAYER_END;

    SetActiveObjectState(true);                             // player is always active object

    m_session = session;

    m_ExtraFlags = 0;
    if (GetSession()->GetSecurity() >= SEC_GAMEMASTER)
        SetAcceptTicket(true);

    // players always accept
    if (GetSession()->GetSecurity() == SEC_PLAYER)
        SetAcceptWhispers(true);

    m_comboPoints = 0;

    m_usedTalentCount = 0;

    m_modManaRegen = 0;
    m_modManaRegenInterrupt = 0;
    for (int s = 0; s < MAX_SPELL_SCHOOL; s++)
        m_SpellCritPercentage[s] = 0.0f;
    m_regenTimer = 0;
    m_weaponChangeTimer = 0;

    m_zoneUpdateId = 0;
    m_zoneUpdateTimer = 0;
    m_positionStatusUpdateTimer = 0;

    m_areaUpdateId = 0;

    m_nextSave = sWorld.getConfig(CONFIG_UINT32_INTERVAL_SAVE);

    // randomize first save time in range [CONFIG_UINT32_INTERVAL_SAVE] around [CONFIG_UINT32_INTERVAL_SAVE]
    // this must help in case next save after mass player load after server startup
    m_nextSave = urand(m_nextSave / 2, m_nextSave * 3 / 2);

    clearResurrectRequestData();

    m_SpellModRemoveCount = 0;

    memset(m_items, 0, sizeof(Item*)*PLAYER_SLOTS_COUNT);

    m_social = NULL;

    // group is initialized in the reference constructor
    SetGroupInvite(NULL);
    m_groupUpdateMask = 0;
    m_auraUpdateMask = 0;

    ClearHonorInfo();

    duel = NULL;

    m_GuildIdInvited = 0;

    m_atLoginFlags = AT_LOGIN_NONE;

    mSemaphoreTeleport_Near = false;
    mSemaphoreTeleport_Far = false;

    m_DelayedOperations = 0;
    m_bCanDelayTeleport = false;
    m_bHasDelayedTeleport = false;
    m_bHasBeenAliveAtDelayedTeleport = true;                // overwrite always at setup teleport data, so not used infact
    m_teleport_options = 0;

    m_trade = NULL;

    m_cinematic = 0;

    PlayerTalkClass = new PlayerMenu(GetSession());
    m_currentBuybackSlot = BUYBACK_SLOT_START;

    m_lastLiquid = NULL;

    for (int i = 0; i < MAX_TIMERS; ++i)
        m_MirrorTimer[i] = DISABLED_MIRROR_TIMER;

    m_MirrorTimerFlags = UNDERWATER_NONE;
    m_MirrorTimerFlagsLast = UNDERWATER_NONE;

    m_isInWater = false;
    m_drunkTimer = 0;
    m_drunk = 0;
    m_restTime = 0;
    m_deathTimer = 0;
    m_deathExpireTime = 0;

    m_swingErrorMsg = 0;

    m_DetectInvTimer = 1 * IN_MILLISECONDS;

    for (int j = 0; j < PLAYER_MAX_BATTLEGROUND_QUEUES; ++j)
    {
        m_bgBattleGroundQueueID[j].bgQueueTypeId  = BATTLEGROUND_QUEUE_NONE;
        m_bgBattleGroundQueueID[j].invitedToInstance = 0;
    }

    m_logintime = time(NULL);
    m_Last_tick = m_logintime;
    m_WeaponProficiency = 0;
    m_ArmorProficiency = 0;
    m_canParry = false;
    m_canBlock = false;
    m_canDualWield = false;
    m_ammoDPS = 0.0f;

    m_temporaryUnsummonedPetNumber = 0;

    //////////////////// Rest System/////////////////////
    time_inn_enter = 0;
    inn_trigger_id = 0;
    m_rest_bonus = 0;
    rest_type = REST_TYPE_NO;
    //////////////////// Rest System/////////////////////

    m_mailsUpdated = false;
    unReadMails = 0;
    m_nextMailDelivereTime = 0;

    m_resetTalentsCost = 0;
    m_resetTalentsTime = 0;
    m_itemUpdateQueueBlocked = false;

    for (int i = 0; i < MAX_MOVE_TYPE; ++i)
        m_forced_speed_changes[i] = 0;

    m_stableSlots = 0;

    /////////////////// Instance System /////////////////////

    m_HomebindTimer = 0;
    m_InstanceValid = true;

    for (int i = 0; i < BASEMOD_END; ++i)
    {
        m_auraBaseMod[i][FLAT_MOD] = 0.0f;
        m_auraBaseMod[i][PCT_MOD] = 1.0f;
    }

    // Player summoning
    m_summon_expire = 0;
    m_summon_mapid = 0;
    m_summon_x = 0.0f;
    m_summon_y = 0.0f;
    m_summon_z = 0.0f;

    m_contestedPvPTimer = 0;

    m_lastFallTime = 0;
    m_lastFallZ = 0;
}

Player::~Player()
{
    CleanupsBeforeDelete();

    // it must be unloaded already in PlayerLogout and accessed only for loggined player
    // m_social = NULL;

    // Note: buy back item already deleted from DB when player was saved
    for (int i = 0; i < PLAYER_SLOTS_COUNT; ++i)
    {
        delete m_items[i];
    }
    CleanupChannels();

    // all mailed items should be deleted, also all mail should be deallocated
    for (PlayerMails::const_iterator itr =  m_mail.begin(); itr != m_mail.end(); ++itr)
        delete *itr;

    for (ItemMap::const_iterator iter = mMitems.begin(); iter != mMitems.end(); ++iter)
        delete iter->second;                                // if item is duplicated... then server may crash ... but that item should be deallocated

    delete PlayerTalkClass;

    if (m_transport)
    {
        m_transport->RemovePassenger(this);
    }

    for (size_t x = 0; x < ItemSetEff.size(); ++x)
        delete ItemSetEff[x];

    // clean up player-instance binds, may unload some instance saves
    for (BoundInstancesMap::iterator itr = m_boundInstances.begin(); itr != m_boundInstances.end(); ++itr)
        itr->second.state->RemovePlayer(this);
}

void Player::CleanupsBeforeDelete()
{
    if (m_uint32Values)                                     // only for fully created Object
    {
        TradeCancel(false);
        DuelComplete(DUEL_INTERRUPTED);
    }

    // notify zone scripts for player logout
    sOutdoorPvPMgr.HandlePlayerLeaveZone(this, m_zoneUpdateId);

    Unit::CleanupsBeforeDelete();
}

bool Player::Create(uint32 guidlow, const std::string& name, uint8 race, uint8 class_, uint8 gender, uint8 skin, uint8 face, uint8 hairStyle, uint8 hairColor, uint8 facialHair, uint8 /*outfitId */)
{
    // FIXME: outfitId not used in player creating

    Object::_Create(guidlow, 0, HIGHGUID_PLAYER);

    m_name = name;

    PlayerInfo const* info = sObjectMgr.GetPlayerInfo(race, class_);
    if (!info)
    {
        sLog.outError("Player have incorrect race/class pair. Can't be loaded.");
        return false;
    }

    ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(class_);
    if (!cEntry)
    {
        sLog.outError("Class %u not found in DBC (Wrong DBC files?)", class_);
        return false;
    }

    // player store gender in single bit
    if (gender != uint8(GENDER_MALE) && gender != uint8(GENDER_FEMALE))
    {
        sLog.outError("Invalid gender %u at player creating", uint32(gender));
        return false;
    }

    for (int i = 0; i < PLAYER_SLOTS_COUNT; ++i)
        m_items[i] = NULL;

    SetLocationMapId(info->mapId);
    Relocate(info->positionX, info->positionY, info->positionZ, info->orientation);

    SetMap(sMapMgr.CreateMap(info->mapId, this));

    uint8 powertype = cEntry->powerType;

    setFactionForRace(race);

    SetByteValue(UNIT_FIELD_BYTES_0, 0, race);
    SetByteValue(UNIT_FIELD_BYTES_0, 1, class_);
    SetByteValue(UNIT_FIELD_BYTES_0, 2, gender);
    SetByteValue(UNIT_FIELD_BYTES_0, 3, powertype);

    InitDisplayIds();                                       // model, scale and model data

    // is it need, only in pre-2.x used and field byte removed later?
    if (powertype == POWER_RAGE || powertype == POWER_MANA)
        SetByteValue(UNIT_FIELD_BYTES_1, 1, 0xEE);

    SetByteValue(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_UNK3 | UNIT_BYTE2_FLAG_UNK5);
    SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);               // fix cast time showed in spell tooltip on client

    SetInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, -1);  // -1 is default value

    SetByteValue(PLAYER_BYTES, 0, skin);
    SetByteValue(PLAYER_BYTES, 1, face);
    SetByteValue(PLAYER_BYTES, 2, hairStyle);
    SetByteValue(PLAYER_BYTES, 3, hairColor);

    SetByteValue(PLAYER_BYTES_2, 0, facialHair);
    SetByteValue(PLAYER_BYTES_2, 3, REST_STATE_NORMAL);

    SetUInt16Value(PLAYER_BYTES_3, 0, gender);              // only GENDER_MALE/GENDER_FEMALE (1 bit) allowed, drunk state = 0
    SetByteValue(PLAYER_BYTES_3, 3, 0);                     // BattlefieldArenaFaction (0 or 1)

    SetUInt32Value(PLAYER_GUILDID, 0);
    SetUInt32Value(PLAYER_GUILDRANK, 0);
    SetUInt32Value(PLAYER_GUILD_TIMESTAMP, 0);

    // set starting level
    if (GetSession()->GetSecurity() >= SEC_MODERATOR)
        SetUInt32Value(UNIT_FIELD_LEVEL, sWorld.getConfig(CONFIG_UINT32_START_GM_LEVEL));
    else
        SetUInt32Value(UNIT_FIELD_LEVEL, sWorld.getConfig(CONFIG_UINT32_START_PLAYER_LEVEL));

    SetUInt32Value(PLAYER_FIELD_COINAGE, sWorld.getConfig(CONFIG_UINT32_START_PLAYER_MONEY));

    // Played time
    m_Last_tick = time(NULL);
    m_Played_time[PLAYED_TIME_TOTAL] = 0;
    m_Played_time[PLAYED_TIME_LEVEL] = 0;

    // base stats and related field values
    InitStatsForLevel();
    InitTaxiNodes();
    InitTalentForLevel();
    InitPrimaryProfessions();                               // to max set before any spell added

    // apply original stats mods before spell loading or item equipment that call before equip _RemoveStatsMods()
    UpdateMaxHealth();                                      // Update max Health (for add bonus from stamina)
    SetHealth(GetMaxHealth());

    if (getPowerType() == POWER_MANA)
    {
        UpdateMaxPower(POWER_MANA);                         // Update max Mana (for add bonus from intellect)
        SetPower(POWER_MANA, GetMaxPower(POWER_MANA));
    }

    // original spells
    learnDefaultSpells();

    // original action bar
    for (PlayerCreateInfoActions::const_iterator action_itr = info->action.begin(); action_itr != info->action.end(); ++action_itr)
        addActionButton(action_itr->button, action_itr->action, action_itr->type);

    // original items
    uint32 raceClassGender = GetUInt32Value(UNIT_FIELD_BYTES_0) & 0x00FFFFFF;

    CharStartOutfitEntry const* oEntry = NULL;
    for (uint32 i = 1; i < sCharStartOutfitStore.GetNumRows(); ++i)
    {
        if (CharStartOutfitEntry const* entry = sCharStartOutfitStore.LookupEntry(i))
        {
            if (entry->RaceClassGender == raceClassGender)
            {
                oEntry = entry;
                break;
            }
        }
    }

    if (oEntry)
    {
        for (int j = 0; j < MAX_OUTFIT_ITEMS; ++j)
        {
            if (oEntry->ItemId[j] <= 0)
                continue;

            uint32 item_id = oEntry->ItemId[j];

            // just skip, reported in ObjectMgr::LoadItemPrototypes
            ItemPrototype const* iProto = ObjectMgr::GetItemPrototype(item_id);
            if (!iProto)
                continue;

            // BuyCount by default
            int32 count = iProto->BuyCount;

            // special amount for foor/drink
            if (iProto->Class == ITEM_CLASS_CONSUMABLE && iProto->SubClass == ITEM_SUBCLASS_FOOD)
            {
                switch (iProto->Spells[0].SpellCategory)
                {
                    case 11:                                // food
                        if (iProto->Stackable > 4)
                            count = 4;
                        break;
                    case 59:                                // drink
                        if (iProto->Stackable > 2)
                            count = 2;
                        break;
                }
            }

            StoreNewItemInBestSlots(item_id, count);
        }
    }

    for (PlayerCreateInfoItems::const_iterator item_id_itr = info->item.begin(); item_id_itr != info->item.end(); ++item_id_itr)
        StoreNewItemInBestSlots(item_id_itr->item_id, item_id_itr->item_amount);

    // bags and main-hand weapon must equipped at this moment
    // now second pass for not equipped (offhand weapon/shield if it attempt equipped before main-hand weapon)
    // or ammo not equipped in special bag
    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            uint16 eDest;
            // equip offhand weapon/shield if it attempt equipped before main-hand weapon
            InventoryResult msg = CanEquipItem(NULL_SLOT, eDest, pItem, false);
            if (msg == EQUIP_ERR_OK)
            {
                RemoveItem(INVENTORY_SLOT_BAG_0, i, true);
                EquipItem(eDest, pItem, true);
            }
            // move other items to more appropriate slots (ammo not equipped in special bag)
            else
            {
                ItemPosCountVec sDest;
                msg = CanStoreItem(NULL_BAG, NULL_SLOT, sDest, pItem, false);
                if (msg == EQUIP_ERR_OK)
                {
                    RemoveItem(INVENTORY_SLOT_BAG_0, i, true);
                    pItem = StoreItem(sDest, pItem, true);
                }

                // if  this is ammo then use it
                msg = CanUseAmmo(pItem->GetEntry());
                if (msg == EQUIP_ERR_OK)
                    SetAmmo(pItem->GetEntry());
            }
        }
    }
    // all item positions resolved

    return true;
}

bool Player::StoreNewItemInBestSlots(uint32 titem_id, uint32 titem_amount)
{
    DEBUG_LOG("STORAGE: Creating initial item, itemId = %u, count = %u", titem_id, titem_amount);

    // attempt equip by one
    while (titem_amount > 0)
    {
        uint16 eDest;
        uint8 msg = CanEquipNewItem(NULL_SLOT, eDest, titem_id, false);
        if (msg != EQUIP_ERR_OK)
            break;

        EquipNewItem(eDest, titem_id, true);
        AutoUnequipOffhandIfNeed();
        --titem_amount;
    }

    if (titem_amount == 0)
        return true;                                        // equipped

    // attempt store
    ItemPosCountVec sDest;
    // store in main bag to simplify second pass (special bags can be not equipped yet at this moment)
    uint8 msg = CanStoreNewItem(INVENTORY_SLOT_BAG_0, NULL_SLOT, sDest, titem_id, titem_amount);
    if (msg == EQUIP_ERR_OK)
    {
        StoreNewItem(sDest, titem_id, true, Item::GenerateItemRandomPropertyId(titem_id));
        return true;                                        // stored
    }

    // item can't be added
    sLog.outError("STORAGE: Can't equip or store initial item %u for race %u class %u , error msg = %u", titem_id, getRace(), getClass(), msg);
    return false;
}

// helper function, mainly for script side, but can be used for simple task in mangos also.
Item* Player::StoreNewItemInInventorySlot(uint32 itemEntry, uint32 amount)
{
    ItemPosCountVec vDest;

    uint8 msg = CanStoreNewItem(INVENTORY_SLOT_BAG_0, NULL_SLOT, vDest, itemEntry, amount);

    if (msg == EQUIP_ERR_OK)
    {
        if (Item* pItem = StoreNewItem(vDest, itemEntry, true, Item::GenerateItemRandomPropertyId(itemEntry)))
            return pItem;
    }

    return NULL;
}

void Player::SendMirrorTimer(MirrorTimerType Type, uint32 MaxValue, uint32 CurrentValue, int32 Regen)
{
    if (int(MaxValue) == DISABLED_MIRROR_TIMER)
    {
        if (int(CurrentValue) != DISABLED_MIRROR_TIMER)
            StopMirrorTimer(Type);
        return;
    }
    WorldPacket data(SMSG_START_MIRROR_TIMER, (21));
    data << (uint32)Type;
    data << CurrentValue;
    data << MaxValue;
    data << Regen;
    data << (uint8)0;
    data << (uint32)0;                                      // spell id
    GetSession()->SendPacket(&data);
}

void Player::StopMirrorTimer(MirrorTimerType Type)
{
    m_MirrorTimer[Type] = DISABLED_MIRROR_TIMER;
    WorldPacket data(SMSG_STOP_MIRROR_TIMER, 4);
    data << (uint32)Type;
    GetSession()->SendPacket(&data);
}

uint32 Player::EnvironmentalDamage(EnvironmentalDamageType type, uint32 damage)
{
    if (!isAlive() || isGameMaster())
        return 0;

    // Absorb, resist some environmental damage type
    uint32 absorb = 0;
    uint32 resist = 0;
    if (type == DAMAGE_LAVA)
        CalculateDamageAbsorbAndResist(this, SPELL_SCHOOL_MASK_FIRE, DIRECT_DAMAGE, damage, &absorb, &resist);
    else if (type == DAMAGE_SLIME)
        CalculateDamageAbsorbAndResist(this, SPELL_SCHOOL_MASK_NATURE, DIRECT_DAMAGE, damage, &absorb, &resist);

    damage -= absorb + resist;

    DealDamageMods(this, damage, &absorb);

    WorldPacket data(SMSG_ENVIRONMENTALDAMAGELOG, (21));
    data << GetObjectGuid();
    data << uint8(type != DAMAGE_FALL_TO_VOID ? type : DAMAGE_FALL);
    data << uint32(damage);
    data << uint32(absorb);
    data << uint32(resist);
    SendMessageToSet(&data, true);

    uint32 final_damage = DealDamage(this, damage, NULL, SELF_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);

    if (type == DAMAGE_FALL && !isAlive())                  // DealDamage not apply item durability loss at self damage
    {
        DEBUG_LOG("We are fall to death, loosing 10 percents durability");
        DurabilityLossAll(0.10f, false);
        // durability lost message
        WorldPacket data2(SMSG_DURABILITY_DAMAGE_DEATH, 0);
        GetSession()->SendPacket(&data2);
    }

    return final_damage;
}

int32 Player::getMaxTimer(MirrorTimerType timer)
{
    switch (timer)
    {
        case FATIGUE_TIMER:
            if (GetSession()->GetSecurity() >= (AccountTypes)sWorld.getConfig(CONFIG_UINT32_TIMERBAR_FATIGUE_GMLEVEL))
                return DISABLED_MIRROR_TIMER;
            return sWorld.getConfig(CONFIG_UINT32_TIMERBAR_FATIGUE_MAX) * IN_MILLISECONDS;
        case BREATH_TIMER:
        {
            if (!isAlive() || HasAuraType(SPELL_AURA_WATER_BREATHING) ||
                    GetSession()->GetSecurity() >= (AccountTypes)sWorld.getConfig(CONFIG_UINT32_TIMERBAR_BREATH_GMLEVEL))
                return DISABLED_MIRROR_TIMER;
            int32 UnderWaterTime = sWorld.getConfig(CONFIG_UINT32_TIMERBAR_BREATH_MAX) * IN_MILLISECONDS;
            AuraList const& mModWaterBreathing = GetAurasByType(SPELL_AURA_MOD_WATER_BREATHING);
            for (AuraList::const_iterator i = mModWaterBreathing.begin(); i != mModWaterBreathing.end(); ++i)
                UnderWaterTime = uint32(UnderWaterTime * (100.0f + (*i)->GetModifier()->m_amount) / 100.0f);
            return UnderWaterTime;
        }
        case FIRE_TIMER:
        {
            if (!isAlive() || GetSession()->GetSecurity() >= (AccountTypes)sWorld.getConfig(CONFIG_UINT32_TIMERBAR_FIRE_GMLEVEL))
                return DISABLED_MIRROR_TIMER;
            return sWorld.getConfig(CONFIG_UINT32_TIMERBAR_FIRE_MAX) * IN_MILLISECONDS;
        }
        default:
            return 0;
    }
    return 0;
}

void Player::UpdateMirrorTimers()
{
    // Desync flags for update on next HandleDrowning
    if (m_MirrorTimerFlags)
        m_MirrorTimerFlagsLast = ~m_MirrorTimerFlags;
}

void Player::HandleDrowning(uint32 time_diff)
{
    if (!m_MirrorTimerFlags)
        return;

    // In water
    if (m_MirrorTimerFlags & UNDERWATER_INWATER)
    {
        // Breath timer not activated - activate it
        if (m_MirrorTimer[BREATH_TIMER] == DISABLED_MIRROR_TIMER)
        {
            m_MirrorTimer[BREATH_TIMER] = getMaxTimer(BREATH_TIMER);
            SendMirrorTimer(BREATH_TIMER, m_MirrorTimer[BREATH_TIMER], m_MirrorTimer[BREATH_TIMER], -1);
        }
        else
        {
            m_MirrorTimer[BREATH_TIMER] -= time_diff;
            // Timer limit - need deal damage
            if (m_MirrorTimer[BREATH_TIMER] < 0)
            {
                m_MirrorTimer[BREATH_TIMER] += 2 * IN_MILLISECONDS;
                // Calculate and deal damage
                // TODO: Check this formula
                uint32 damage = GetMaxHealth() / 5 + urand(0, getLevel() - 1);
                EnvironmentalDamage(DAMAGE_DROWNING, damage);
            }
            else if (!(m_MirrorTimerFlagsLast & UNDERWATER_INWATER))      // Update time in client if need
                SendMirrorTimer(BREATH_TIMER, getMaxTimer(BREATH_TIMER), m_MirrorTimer[BREATH_TIMER], -1);
        }
    }
    else if (m_MirrorTimer[BREATH_TIMER] != DISABLED_MIRROR_TIMER)        // Regen timer
    {
        int32 UnderWaterTime = getMaxTimer(BREATH_TIMER);
        // Need breath regen
        m_MirrorTimer[BREATH_TIMER] += 10 * time_diff;
        if (m_MirrorTimer[BREATH_TIMER] >= UnderWaterTime || !isAlive())
            StopMirrorTimer(BREATH_TIMER);
        else if (m_MirrorTimerFlagsLast & UNDERWATER_INWATER)
            SendMirrorTimer(BREATH_TIMER, UnderWaterTime, m_MirrorTimer[BREATH_TIMER], 10);
    }

    // In dark water
    if (m_MirrorTimerFlags & UNDERWATER_INDARKWATER)
    {
        // Fatigue timer not activated - activate it
        if (m_MirrorTimer[FATIGUE_TIMER] == DISABLED_MIRROR_TIMER)
        {
            m_MirrorTimer[FATIGUE_TIMER] = getMaxTimer(FATIGUE_TIMER);
            SendMirrorTimer(FATIGUE_TIMER, m_MirrorTimer[FATIGUE_TIMER], m_MirrorTimer[FATIGUE_TIMER], -1);
        }
        else
        {
            m_MirrorTimer[FATIGUE_TIMER] -= time_diff;
            // Timer limit - need deal damage or teleport ghost to graveyard
            if (m_MirrorTimer[FATIGUE_TIMER] < 0)
            {
                m_MirrorTimer[FATIGUE_TIMER] += 2 * IN_MILLISECONDS;
                if (isAlive())                              // Calculate and deal damage
                {
                    uint32 damage = GetMaxHealth() / 5 + urand(0, getLevel() - 1);
                    EnvironmentalDamage(DAMAGE_EXHAUSTED, damage);
                }
                else if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))       // Teleport ghost to graveyard
                    RepopAtGraveyard();
            }
            else if (!(m_MirrorTimerFlagsLast & UNDERWATER_INDARKWATER))
                SendMirrorTimer(FATIGUE_TIMER, getMaxTimer(FATIGUE_TIMER), m_MirrorTimer[FATIGUE_TIMER], -1);
        }
    }
    else if (m_MirrorTimer[FATIGUE_TIMER] != DISABLED_MIRROR_TIMER)       // Regen timer
    {
        int32 DarkWaterTime = getMaxTimer(FATIGUE_TIMER);
        m_MirrorTimer[FATIGUE_TIMER] += 10 * time_diff;
        if (m_MirrorTimer[FATIGUE_TIMER] >= DarkWaterTime || !isAlive())
            StopMirrorTimer(FATIGUE_TIMER);
        else if (m_MirrorTimerFlagsLast & UNDERWATER_INDARKWATER)
            SendMirrorTimer(FATIGUE_TIMER, DarkWaterTime, m_MirrorTimer[FATIGUE_TIMER], 10);
    }

    if (m_MirrorTimerFlags & (UNDERWATER_INLAVA /*| UNDERWATER_INSLIME*/) && !(m_lastLiquid && m_lastLiquid->SpellId))
    {
        // Breath timer not activated - activate it
        if (m_MirrorTimer[FIRE_TIMER] == DISABLED_MIRROR_TIMER)
            m_MirrorTimer[FIRE_TIMER] = getMaxTimer(FIRE_TIMER);
        else
        {
            m_MirrorTimer[FIRE_TIMER] -= time_diff;
            if (m_MirrorTimer[FIRE_TIMER] < 0)
            {
                m_MirrorTimer[FIRE_TIMER] += 2 * IN_MILLISECONDS;
                // Calculate and deal damage
                // TODO: Check this formula
                uint32 damage = urand(600, 700);
                if (m_MirrorTimerFlags & UNDERWATER_INLAVA)
                    EnvironmentalDamage(DAMAGE_LAVA, damage);
                // need to skip Slime damage in Undercity,
                // maybe someone can find better way to handle environmental damage
                //else if (m_zoneUpdateId != 1497)
                //    EnvironmentalDamage(DAMAGE_SLIME, damage);
            }
        }
    }
    else
        m_MirrorTimer[FIRE_TIMER] = DISABLED_MIRROR_TIMER;

    // Recheck timers flag
    m_MirrorTimerFlags &= ~UNDERWATER_EXIST_TIMERS;
    for (int i = 0; i < MAX_TIMERS; ++i)
        if (m_MirrorTimer[i] != DISABLED_MIRROR_TIMER)
        {
            m_MirrorTimerFlags |= UNDERWATER_EXIST_TIMERS;
            break;
        }
    m_MirrorTimerFlagsLast = m_MirrorTimerFlags;
}

/// The player sobers by 256 every 10 seconds
void Player::HandleSobering()
{
    m_drunkTimer = 0;

    uint32 drunk = (m_drunk <= 256) ? 0 : (m_drunk - 256);
    SetDrunkValue(drunk);
}

DrunkenState Player::GetDrunkenstateByValue(uint16 value)
{
    if (value >= 23000)
        return DRUNKEN_SMASHED;
    if (value >= 12800)
        return DRUNKEN_DRUNK;
    if (value & 0xFFFE)
        return DRUNKEN_TIPSY;
    return DRUNKEN_SOBER;
}

void Player::SetDrunkValue(uint16 newDrunkenValue, uint32 itemId)
{
    uint32 oldDrunkenState = Player::GetDrunkenstateByValue(m_drunk);

    m_drunk = newDrunkenValue;
    SetUInt16Value(PLAYER_BYTES_3, 0, uint16(getGender()) | (m_drunk & 0xFFFE));

    uint32 newDrunkenState = Player::GetDrunkenstateByValue(m_drunk);

    // special drunk invisibility detection
    if (newDrunkenState >= DRUNKEN_DRUNK)
        m_detectInvisibilityMask |= (1 << 6);
    else
        m_detectInvisibilityMask &= ~(1 << 6);
}

void Player::Update(uint32 update_diff, uint32 p_time)
{
    if (!IsInWorld())
        return;

    // Undelivered mail
    if (m_nextMailDelivereTime && m_nextMailDelivereTime <= time(NULL))
    {
        SendNewMail();
        ++unReadMails;

        // It will be recalculate at mailbox open (for unReadMails important non-0 until mailbox open, it also will be recalculated)
        m_nextMailDelivereTime = 0;
    }

    // Used to implement delayed far teleports
    SetCanDelayTeleport(true);
    Unit::Update(update_diff, p_time);
    SetCanDelayTeleport(false);

    // Update player only attacks
    if (uint32 ranged_att = getAttackTimer(RANGED_ATTACK))
        setAttackTimer(RANGED_ATTACK, (update_diff >= ranged_att ? 0 : ranged_att - update_diff));

    time_t now = time(NULL);

    UpdatePvPFlag(now);

    UpdateContestedPvP(update_diff);

    UpdateDuelFlag(now);

    CheckDuelDistance(now);

    // Update items that have just a limited lifetime
    if (now > m_Last_tick)
        UpdateItemDuration(uint32(now - m_Last_tick));

    if (!m_timedquests.empty())
    {
        QuestSet::iterator iter = m_timedquests.begin();
        while (iter != m_timedquests.end())
        {
            QuestStatusData& q_status = mQuestStatus[*iter];
            if (q_status.m_timer <= update_diff)
            {
                uint32 quest_id  = *iter;
                ++iter;                                     // Current iter will be removed in FailQuest
                FailQuest(quest_id);
            }
            else
            {
                q_status.m_timer -= update_diff;
                if (q_status.uState != QUEST_NEW) q_status.uState = QUEST_CHANGED;
                ++iter;
            }
        }
    }

    if (hasUnitState(UNIT_STAT_MELEE_ATTACKING))
    {
        UpdateMeleeAttackingState();

        Unit* pVictim = getVictim();
        if (pVictim && !IsNonMeleeSpellCasted(false))
        {
            Player* vOwner = pVictim->GetCharmerOrOwnerPlayerOrPlayerItself();
            if (vOwner && vOwner->IsPvP() && !IsInDuelWith(vOwner))
            {
                UpdatePvP(true);
                RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
            }
        }
    }

    if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING))
    {
        if (roll_chance_i(3) && GetTimeInnEnter() > 0)      // Freeze update
        {
            time_t time_inn = time(NULL) - GetTimeInnEnter();
            if (time_inn >= 10)                             // Freeze update
            {
                float bubble = 0.125f * sWorld.getConfig(CONFIG_FLOAT_RATE_REST_INGAME);
                // Speed collect rest bonus (section/in hour)
                SetRestBonus(float(GetRestBonus() + time_inn * (GetUInt32Value(PLAYER_NEXT_LEVEL_XP) / 72000) * bubble));
                UpdateInnerTime(time(NULL));
            }
        }
    }

    if (m_regenTimer)
    {
        if (update_diff >= m_regenTimer)
            m_regenTimer = 0;
        else
            m_regenTimer -= update_diff;
    }

    if (m_positionStatusUpdateTimer)
    {
        if (update_diff >= m_positionStatusUpdateTimer)
            m_positionStatusUpdateTimer = 0;
        else
            m_positionStatusUpdateTimer -= update_diff;
    }

    if (m_weaponChangeTimer > 0)
    {
        if (update_diff >= m_weaponChangeTimer)
            m_weaponChangeTimer = 0;
        else
            m_weaponChangeTimer -= update_diff;
    }

    if (m_zoneUpdateTimer > 0)
    {
        if (update_diff >= m_zoneUpdateTimer)
        {
            uint32 newzone, newarea;
            GetZoneAndAreaId(newzone, newarea);

            if (m_zoneUpdateId != newzone)
                UpdateZone(newzone, newarea);               // Also update area
            else
            {
                // Use area updates as well
                if (m_areaUpdateId != newarea)
                    UpdateArea(newarea);

                m_zoneUpdateTimer = ZONE_UPDATE_INTERVAL;
            }
        }
        else
            m_zoneUpdateTimer -= update_diff;
    }

    if (isAlive())
    {
        RegenerateAll();
    }

    if (m_deathState == JUST_DIED)
        KillPlayer();

    if (m_nextSave > 0)
    {
        if (update_diff >= m_nextSave)
        {
            // m_nextSave reseted in SaveToDB call
            // used by eluna
            sHookMgr->OnSave(this);
            SaveToDB();
            DETAIL_LOG("Player '%s' (GUID: %u) saved", GetName(), GetGUIDLow());
        }
        else
            m_nextSave -= update_diff;
    }

    // Handle Water/drowning
    HandleDrowning(update_diff);

    // Handle detect stealth players
    if (m_DetectInvTimer > 0)
    {
        if (update_diff >= m_DetectInvTimer)
        {
            HandleStealthedUnitsDetection();
            m_DetectInvTimer = 3000;
        }
        else
            m_DetectInvTimer -= update_diff;
    }

    // Played time
    if (now > m_Last_tick)
    {
        uint32 elapsed = uint32(now - m_Last_tick);
        m_Played_time[PLAYED_TIME_TOTAL] += elapsed;        // Total played time
        m_Played_time[PLAYED_TIME_LEVEL] += elapsed;        // Level played time
        m_Last_tick = now;
    }

    if (m_drunk)
    {
        m_drunkTimer += update_diff;

        if (m_drunkTimer > 10 * IN_MILLISECONDS)
            HandleSobering();
    }

    // Not auto-free ghost from body in instances
    if (m_deathTimer > 0  && !GetMap()->Instanceable())
    {
        if (p_time >= m_deathTimer)
        {
            m_deathTimer = 0;
            BuildPlayerRepop();
            RepopAtGraveyard();
        }
        else
            m_deathTimer -= p_time;
    }

    UpdateEnchantTime(update_diff);
    UpdateHomebindTime(update_diff);

    // Group update
    SendUpdateToOutOfRangeGroupMembers();

    Pet* pet = GetPet();
    if (pet && !pet->IsWithinDistInMap(this, GetMap()->GetVisibilityDistance()) && (GetCharmGuid() && (pet->GetObjectGuid() != GetCharmGuid())))
        pet->Unsummon(PET_SAVE_REAGENTS, this);

    if (IsHasDelayedTeleport())
        TeleportTo(m_teleport_dest, m_teleport_options);
}

void Player::SetDeathState(DeathState s)
{
    uint32 ressSpellId = 0;

    bool cur = isAlive();

    if (s == JUST_DIED && cur)
    {
        // drunken state is cleared on death
        SetDrunkValue(0);
        // lost combo points at any target (targeted combo points clear in Unit::SetDeathState)
        ClearComboPoints();

        clearResurrectRequestData();

        // remove form before other mods to prevent incorrect stats calculation
        RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT);

        // FIXME: is pet dismissed at dying or releasing spirit? if second, add SetDeathState(DEAD) to HandleRepopRequestOpcode and define pet unsummon here with (s == DEAD)
        RemovePet(PET_SAVE_REAGENTS);

        // remove uncontrolled pets
        RemoveMiniPet();

        // save value before aura remove in Unit::SetDeathState
        ressSpellId = GetUInt32Value(PLAYER_SELF_RES_SPELL);

        // passive spell
        if (!ressSpellId)
            ressSpellId = GetResurrectionSpellId();

        if (InstanceData* mapInstance = GetInstanceData())
            mapInstance->OnPlayerDeath(this);
    }

    Unit::SetDeathState(s);

    // restore resurrection spell id for player after aura remove
    if (s == JUST_DIED && cur && ressSpellId)
        SetUInt32Value(PLAYER_SELF_RES_SPELL, ressSpellId);

    if (isAlive() && !cur)
    {
        // clear aura case after resurrection by another way (spells will be applied before next death)
        SetUInt32Value(PLAYER_SELF_RES_SPELL, 0);

        // restore default warrior stance
        if (getClass() == CLASS_WARRIOR)
            CastSpell(this, SPELL_ID_PASSIVE_BATTLE_STANCE, true);
    }
}

bool Player::BuildEnumData(QueryResult* result, WorldPacket* p_data)
{
    //             0               1                2                3                 4                  5                       6                        7
    //    "SELECT characters.guid, characters.name, characters.race, characters.class, characters.gender, characters.playerBytes, characters.playerBytes2, characters.level, "
    //     8                9               10                     11                     12                     13                    14
    //    "characters.zone, characters.map, characters.position_x, characters.position_y, characters.position_z, guild_member.guildid, characters.playerFlags, "
    //    15                    16                   17                     18                   19
    //    "characters.at_login, character_pet.entry, character_pet.modelid, character_pet.level, characters.equipmentCache "

    Field* fields = result->Fetch();

    uint32 guid = fields[0].GetUInt32();
    uint8 pRace = fields[2].GetUInt8();
    uint8 pClass = fields[3].GetUInt8();

    PlayerInfo const* info = sObjectMgr.GetPlayerInfo(pRace, pClass);
    if (!info)
    {
        sLog.outError("Player %u has incorrect race/class pair. Don't build enum.", guid);
        return false;
    }

    *p_data << ObjectGuid(HIGHGUID_PLAYER, guid);
    *p_data << fields[1].GetString();                       // name
    *p_data << uint8(pRace);                                // race
    *p_data << uint8(pClass);                               // class
    *p_data << uint8(fields[4].GetUInt8());                 // gender

    uint32 playerBytes = fields[5].GetUInt32();
    *p_data << uint8(playerBytes);                          // skin
    *p_data << uint8(playerBytes >> 8);                     // face
    *p_data << uint8(playerBytes >> 16);                    // hair style
    *p_data << uint8(playerBytes >> 24);                    // hair color

    uint32 playerBytes2 = fields[6].GetUInt32();
    *p_data << uint8(playerBytes2 & 0xFF);                  // facial hair

    *p_data << uint8(fields[7].GetUInt8());                 // level
    *p_data << uint32(fields[8].GetUInt32());               // zone
    *p_data << uint32(fields[9].GetUInt32());               // map

    *p_data << fields[10].GetFloat();                       // x
    *p_data << fields[11].GetFloat();                       // y
    *p_data << fields[12].GetFloat();                       // z

    *p_data << uint32(fields[13].GetUInt32());              // guild id

    uint32 char_flags = 0;
    uint32 playerFlags = fields[14].GetUInt32();
    uint32 atLoginFlags = fields[15].GetUInt32();
    if (playerFlags & PLAYER_FLAGS_HIDE_HELM)
        char_flags |= CHARACTER_FLAG_HIDE_HELM;
    if (playerFlags & PLAYER_FLAGS_HIDE_CLOAK)
        char_flags |= CHARACTER_FLAG_HIDE_CLOAK;
    if (playerFlags & PLAYER_FLAGS_GHOST)
        char_flags |= CHARACTER_FLAG_GHOST;
    if (atLoginFlags & AT_LOGIN_RENAME)
        char_flags |= CHARACTER_FLAG_RENAME;

    *p_data << uint32(char_flags);                          // character flags

    // First login
    *p_data << uint8(atLoginFlags & AT_LOGIN_FIRST ? 1 : 0);

    // Pets info
    {
        uint32 petDisplayId = 0;
        uint32 petLevel   = 0;
        uint32 petFamily  = 0;

        // show pet at selection character in character list only for non-ghost character
        if (result && !(playerFlags & PLAYER_FLAGS_GHOST) && (pClass == CLASS_WARLOCK || pClass == CLASS_HUNTER))
        {
            uint32 entry = fields[16].GetUInt32();
            CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(entry);
            if (cInfo)
            {
                petDisplayId = fields[17].GetUInt32();
                petLevel     = fields[18].GetUInt32();
                petFamily    = cInfo->Family;
            }
        }

        *p_data << uint32(petDisplayId);
        *p_data << uint32(petLevel);
        *p_data << uint32(petFamily);
    }


    Tokens data = StrSplit(fields[19].GetCppString(), " ");
    for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        uint32 visualbase = slot * 2;                       // entry, perm ench., temp ench.
        uint32 item_id = GetUInt32ValueFromArray(data, visualbase);
        const ItemPrototype* proto = ObjectMgr::GetItemPrototype(item_id);
        if (!proto)
        {
            *p_data << uint32(0);
            *p_data << uint8(0);
            continue;
        }

        *p_data << uint32(proto->DisplayInfoID);
        *p_data << uint8(proto->InventoryType);
    }

    *p_data << uint32(0);                                   // first bag display id
    *p_data << uint8(0);                                    // first bag inventory type

    return true;
}

void Player::ToggleAFK()
{
    ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_AFK);

    // afk player not allowed in battleground
    if (isAFK() && InBattleGround())
        LeaveBattleground();
}

void Player::ToggleDND()
{
    ToggleFlag(PLAYER_FLAGS, PLAYER_FLAGS_DND);
}

ChatTagFlags Player::GetChatTag() const
{
    if (isGMChat())
        return CHAT_TAG_GM;

    if (isAFK())
        return CHAT_TAG_AFK;
    if (isDND())
        return CHAT_TAG_DND;

    return CHAT_TAG_NONE;
}

bool Player::TeleportTo(uint32 mapid, float x, float y, float z, float orientation, uint32 options /*=0*/, AreaTrigger const* at /*=NULL*/)
{
    if (!MapManager::IsValidMapCoord(mapid, x, y, z, orientation))
    {
        sLog.outError("TeleportTo: invalid map %d or absent instance template.", mapid);
        return false;
    }

    MapEntry const* mEntry = sMapStore.LookupEntry(mapid);  // Validity checked in IsValidMapCoord

    // preparing unsummon pet if lost (we must get pet before teleportation or will not find it later)
    Pet* pet = GetPet();

    // don't let enter battlegrounds without assigned battleground id (for example through areatrigger)...
    // don't let gm level > 1 either
    if (!InBattleGround() && mEntry->IsBattleGround())
        return false;

    // Get MapEntrance trigger if teleport to other -nonBG- map
    bool assignedAreaTrigger = false;
    if (GetMapId() != mapid && !mEntry->IsBattleGround() && !at)
    {
        at = sObjectMgr.GetMapEntranceTrigger(mapid);
        assignedAreaTrigger = true;
    }

    // Check requirements for teleport
    if (at)
    {
        uint32 miscRequirement = 0;
        AreaLockStatus lockStatus = GetAreaTriggerLockStatus(at, miscRequirement);
        if (lockStatus != AREA_LOCKSTATUS_OK)
        {
            SendTransferAbortedByLockStatus(mEntry, lockStatus, miscRequirement);
            return false;
        }
    }

    // if we were on a transport, leave
    if (!(options & TELE_TO_NOT_LEAVE_TRANSPORT) && m_transport)
    {
        m_transport->RemovePassenger(this);
        m_transport = NULL;
        m_movementInfo.ClearTransportData();
    }

    // The player was ported to another map and looses the duel immediately.
    // We have to perform this check before the teleport, otherwise the
    // ObjectAccessor won't find the flag.
    if (duel && GetMapId() != mapid)
        if (GetMap()->GetGameObject(GetGuidValue(PLAYER_DUEL_ARBITER)))
            DuelComplete(DUEL_FLED);

    // reset movement flags at teleport, because player will continue move with these flags after teleport
    m_movementInfo.SetMovementFlags(MOVEFLAG_NONE);
    DisableSpline();

    if ((GetMapId() == mapid) && (!m_transport))            // TODO the !m_transport might have unexpected effects when teleporting from transport to other place on same map
    {
        // lets reset far teleport flag if it wasn't reset during chained teleports
        SetSemaphoreTeleportFar(false);
        // setup delayed teleport flag
        // if teleport spell is casted in Unit::Update() func
        // then we need to delay it until update process will be finished
        if (SetDelayedTeleportFlagIfCan())
        {
            SetSemaphoreTeleportNear(true);
            // lets save teleport destination for player
            m_teleport_dest = WorldLocation(mapid, x, y, z, orientation);
            m_teleport_options = options;
            return true;
        }

        if (!(options & TELE_TO_NOT_UNSUMMON_PET))
        {
            // same map, only remove pet if out of range for new position
            if (pet && !pet->IsWithinDist3d(x, y, z, GetMap()->GetVisibilityDistance()))
                UnsummonPetTemporaryIfAny();
        }

        if (!(options & TELE_TO_NOT_LEAVE_COMBAT))
            CombatStop();

        // this will be used instead of the current location in SaveToDB
        m_teleport_dest = WorldLocation(mapid, x, y, z, orientation);
        SetFallInformation(0, z);

        // code for finish transfer called in WorldSession::HandleMovementOpcodes()
        // at client packet MSG_MOVE_TELEPORT_ACK
        SetSemaphoreTeleportNear(true);
        // near teleport, triggering send MSG_MOVE_TELEPORT_ACK from client at landing
        if (!GetSession()->PlayerLogout())
        {
            WorldPacket data;
            BuildTeleportAckMsg(data, x, y, z, orientation);
            GetSession()->SendPacket(&data);
        }
    }
    else
    {
        // far teleport to another map
        Map* oldmap = IsInWorld() ? GetMap() : NULL;
        // check if we can enter before stopping combat / removing pet / totems / interrupting spells

        // If the map is not created, assume it is possible to enter it.
        // It will be created in the WorldPortAck.
        DungeonPersistentState* state = GetBoundInstanceSaveForSelfOrGroup(mapid);
        Map* map = sMapMgr.FindMap(mapid, state ? state->GetInstanceId() : 0);
        if (!map || map->CanEnter(this))
        {
            // lets reset near teleport flag if it wasn't reset during chained teleports
            SetSemaphoreTeleportNear(false);
            // setup delayed teleport flag
            // if teleport spell is casted in Unit::Update() func
            // then we need to delay it until update process will be finished
            if (SetDelayedTeleportFlagIfCan())
            {
                SetSemaphoreTeleportFar(true);
                // lets save teleport destination for player
                m_teleport_dest = WorldLocation(mapid, x, y, z, orientation);
                m_teleport_options = options;
                return true;
            }

            SetSelectionGuid(ObjectGuid());

            CombatStop();

            ResetContestedPvP();

            // remove player from battleground on far teleport (when changing maps)
            if (BattleGround const* bg = GetBattleGround())
            {
                // Note: at battleground join battleground id set before teleport
                // and we already will found "current" battleground
                // just need check that this is targeted map or leave
                if (bg->GetMapId() != mapid)
                    LeaveBattleground(false);               // don't teleport to entry point
            }

            // remove pet on map change
            if (pet)
                UnsummonPetTemporaryIfAny();

            // remove all dyn objects
            RemoveAllDynObjects();

            // stop spellcasting
            // not attempt interrupt teleportation spell at caster teleport
            if (!(options & TELE_TO_SPELL))
                if (IsNonMeleeSpellCasted(true))
                    InterruptNonMeleeSpells(true);

            // remove auras before removing from map...
            RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_CHANGE_MAP | AURA_INTERRUPT_FLAG_MOVE | AURA_INTERRUPT_FLAG_TURNING);

            if (!GetSession()->PlayerLogout())
            {
                // send transfer packet to display load screen
                WorldPacket data(SMSG_TRANSFER_PENDING, (4 + 4 + 4));
                data << uint32(mapid);
                if (m_transport)
                {
                    data << uint32(m_transport->GetEntry());
                    data << uint32(GetMapId());
                }
                GetSession()->SendPacket(&data);
            }

            // remove from old map now
            if (oldmap)
                oldmap->Remove(this, false);

            // new final coordinates
            float final_x = x;
            float final_y = y;
            float final_z = z;
            float final_o = orientation;

            if (m_transport)
            {
                final_x += m_movementInfo.GetTransportPos()->x;
                final_y += m_movementInfo.GetTransportPos()->y;
                final_z += m_movementInfo.GetTransportPos()->z;
                final_o += m_movementInfo.GetTransportPos()->o;
            }

            m_teleport_dest = WorldLocation(mapid, final_x, final_y, final_z, final_o);
            SetFallInformation(0, final_z);
            // if the player is saved before worldport ack (at logout for example)
            // this will be used instead of the current location in SaveToDB

            // move packet sent by client always after far teleport
            // code for finish transfer to new map called in WorldSession::HandleMoveWorldportAckOpcode at client packet
            SetSemaphoreTeleportFar(true);

            if (!GetSession()->PlayerLogout())
            {
                // transfer finished, inform client to start load
                WorldPacket data(SMSG_NEW_WORLD, (20));
                data << uint32(mapid);
                if (m_transport)
                {
                    data << float(m_movementInfo.GetTransportPos()->x);
                    data << float(m_movementInfo.GetTransportPos()->y);
                    data << float(m_movementInfo.GetTransportPos()->z);
                    data << float(m_movementInfo.GetTransportPos()->o);
                }
                else
                {
                    data << float(final_x);
                    data << float(final_y);
                    data << float(final_z);
                    data << float(final_o);
                }

                GetSession()->SendPacket(&data);
                SendSavedInstances();
            }
        }
        else                                                // !map->CanEnter(this)
            return false;
    }
    return true;
}

bool Player::TeleportToBGEntryPoint()
{
    return TeleportTo(m_bgData.joinPos);
}

void Player::ProcessDelayedOperations()
{
    if (m_DelayedOperations == 0)
        return;

    if (m_DelayedOperations & DELAYED_RESURRECT_PLAYER)
    {
        ResurrectPlayer(0.0f, false);

        if (GetMaxHealth() > m_resurrectHealth)
            SetHealth(m_resurrectHealth);
        else
            SetHealth(GetMaxHealth());

        if (GetMaxPower(POWER_MANA) > m_resurrectMana)
            SetPower(POWER_MANA, m_resurrectMana);
        else
            SetPower(POWER_MANA, GetMaxPower(POWER_MANA));

        SetPower(POWER_RAGE, 0);
        SetPower(POWER_ENERGY, GetMaxPower(POWER_ENERGY));

        SpawnCorpseBones();
    }

    if (m_DelayedOperations & DELAYED_SAVE_PLAYER)
    {
        SaveToDB();
    }

    if (m_DelayedOperations & DELAYED_SPELL_CAST_DESERTER)
    {
        CastSpell(this, 26013, true);               // Deserter
    }

    // we have executed ALL delayed ops, so clear the flag
    m_DelayedOperations = 0;
}

void Player::AddToWorld()
{
    ///- Do not add/remove the player from the object storage
    ///- It will crash when updating the ObjectAccessor
    ///- The player should only be added when logging in
    Unit::AddToWorld();

    for (int i = PLAYER_SLOT_START; i < PLAYER_SLOT_END; ++i)
    {
        if (m_items[i])
            m_items[i]->AddToWorld();
    }
}

void Player::RemoveFromWorld()
{
    // cleanup
    if (IsInWorld())
    {
        ///- Release charmed creatures, unsummon totems and remove pets/guardians
        UnsummonAllTotems();
        RemoveMiniPet();
    }

    for (int i = PLAYER_SLOT_START; i < PLAYER_SLOT_END; ++i)
    {
        if (m_items[i])
            m_items[i]->RemoveFromWorld();
    }

    // remove duel before calling Unit::RemoveFromWorld
    // otherwise there will be an existing duel flag pointer but no entry in m_gameObj
    DuelComplete(DUEL_INTERRUPTED);

    ///- Do not add/remove the player from the object storage
    ///- It will crash when updating the ObjectAccessor
    ///- The player should only be removed when logging out
    if (IsInWorld())
        GetCamera().ResetView();

    Unit::RemoveFromWorld();
}

void Player::RewardRage(uint32 damage, bool attacker)
{
    float addRage;

    float rageconversion = float((0.0091107836 * getLevel() * getLevel()) + 3.225598133 * getLevel()) + 4.2652911f;

    if (attacker)
    {
        addRage = damage / rageconversion * 7.5f;
    }
    else
    {
        addRage = damage / rageconversion * 2.5f;

        // Berserker Rage effect
        if (HasAura(18499, EFFECT_INDEX_0))
            addRage *= 1.3f;
    }

    addRage *= sWorld.getConfig(CONFIG_FLOAT_RATE_POWER_RAGE_INCOME);

    ModifyPower(POWER_RAGE, uint32(addRage * 10));
}

void Player::RegenerateAll()
{
    if (m_regenTimer != 0)
        return;

    // Not in combat or they have regeneration
    if (!isInCombat() || HasAuraType(SPELL_AURA_MOD_REGEN_DURING_COMBAT) ||
            HasAuraType(SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT) || IsPolymorphed())
    {
        RegenerateHealth();
        if (!isInCombat() && !HasAuraType(SPELL_AURA_INTERRUPT_REGEN))
            Regenerate(POWER_RAGE);
    }

    Regenerate(POWER_ENERGY);

    Regenerate(POWER_MANA);

    m_regenTimer = REGEN_TIME_FULL;
}

void Player::Regenerate(Powers power)
{
    uint32 curValue = GetPower(power);
    uint32 maxValue = GetMaxPower(power);

    float addvalue = 0.0f;

    switch (power)
    {
        case POWER_MANA:
        {
            bool recentCast = IsUnderLastManaUseEffect();
            float ManaIncreaseRate = sWorld.getConfig(CONFIG_FLOAT_RATE_POWER_MANA);
            if (recentCast)
            {
                // Mangos Updates Mana in intervals of 2s, which is correct
                addvalue = m_modManaRegenInterrupt *  ManaIncreaseRate * 2.00f;
            }
            else
            {
                addvalue = m_modManaRegen * ManaIncreaseRate * 2.00f;
            }
        }   break;
        case POWER_RAGE:                                    // Regenerate rage
        {
            float RageDecreaseRate = sWorld.getConfig(CONFIG_FLOAT_RATE_POWER_RAGE_LOSS);
            addvalue = 20 * RageDecreaseRate;               // 2 rage by tick (= 2 seconds => 1 rage/sec)
        }   break;
        case POWER_ENERGY:                                  // Regenerate energy (rogue)
        {
            float EnergyRate = sWorld.getConfig(CONFIG_FLOAT_RATE_POWER_ENERGY);
            addvalue = 20 * EnergyRate;
            break;
        }
        case POWER_FOCUS:
        case POWER_HAPPINESS:
        case POWER_HEALTH:
            break;
    }

    // Mana regen calculated in Player::UpdateManaRegen()
    // Exist only for POWER_MANA, POWER_ENERGY, POWER_FOCUS auras
    if (power != POWER_MANA)
    {
        AuraList const& ModPowerRegenPCTAuras = GetAurasByType(SPELL_AURA_MOD_POWER_REGEN_PERCENT);
        for (AuraList::const_iterator i = ModPowerRegenPCTAuras.begin(); i != ModPowerRegenPCTAuras.end(); ++i)
            if ((*i)->GetModifier()->m_miscvalue == int32(power))
                addvalue *= ((*i)->GetModifier()->m_amount + 100) / 100.0f;
    }

    if (power != POWER_RAGE)
    {
        curValue += uint32(addvalue);
        if (curValue > maxValue)
            curValue = maxValue;
    }
    else
    {
        if (curValue <= uint32(addvalue))
            curValue = 0;
        else
            curValue -= uint32(addvalue);
    }
    SetPower(power, curValue);
}

void Player::RegenerateHealth()
{
    uint32 curValue = GetHealth();
    uint32 maxValue = GetMaxHealth();

    if (curValue >= maxValue) return;

    float HealthIncreaseRate = sWorld.getConfig(CONFIG_FLOAT_RATE_HEALTH);

    float addvalue = 0.0f;

    // polymorphed case
    if (IsPolymorphed())
        addvalue = (float)GetMaxHealth() / 3;
    // normal regen case (maybe partly in combat case)
    else if (!isInCombat() || HasAuraType(SPELL_AURA_MOD_REGEN_DURING_COMBAT))
    {
        addvalue = OCTRegenHPPerSpirit() * HealthIncreaseRate;
        if (!isInCombat())
        {
            AuraList const& mModHealthRegenPct = GetAurasByType(SPELL_AURA_MOD_HEALTH_REGEN_PERCENT);
            for (AuraList::const_iterator i = mModHealthRegenPct.begin(); i != mModHealthRegenPct.end(); ++i)
                addvalue *= (100.0f + (*i)->GetModifier()->m_amount) / 100.0f;
        }
        else if (HasAuraType(SPELL_AURA_MOD_REGEN_DURING_COMBAT))
            addvalue *= GetTotalAuraModifier(SPELL_AURA_MOD_REGEN_DURING_COMBAT) / 100.0f;

        if (!IsStandState())
            addvalue *= 1.5;
    }

    // always regeneration bonus (including combat)
    addvalue += GetTotalAuraModifier(SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT);

    if (addvalue < 0)
        addvalue = 0;

    ModifyHealth(int32(addvalue));
}

Creature* Player::GetNPCIfCanInteractWith(ObjectGuid guid, uint32 npcflagmask)
{
    // some basic checks
    if (!guid || !IsInWorld() || IsTaxiFlying())
        return NULL;

    // not in interactive state
    if (hasUnitState(UNIT_STAT_CAN_NOT_REACT_OR_LOST_CONTROL))
        return NULL;

    // exist (we need look pets also for some interaction (quest/etc)
    Creature* unit = GetMap()->GetAnyTypeCreature(guid);
    if (!unit)
        return NULL;

    // appropriate npc type
    if (npcflagmask && !unit->HasFlag(UNIT_NPC_FLAGS, npcflagmask))
        return NULL;

    if (npcflagmask == UNIT_NPC_FLAG_STABLEMASTER)
    {
        if (getClass() != CLASS_HUNTER)
            return NULL;
    }

    // if a dead unit should be able to talk - the creature must be alive and have special flags
    if (!unit->isAlive())
        return NULL;

    if (isAlive() && unit->isInvisibleForAlive())
        return NULL;

    // not allow interaction under control, but allow with own pets
    if (unit->GetCharmerGuid())
        return NULL;

    // not enemy
    if (unit->IsHostileTo(this))
        return NULL;

    // not too far
    if (!unit->IsWithinDistInMap(this, INTERACTION_DISTANCE))
        return NULL;

    return unit;
}

GameObject* Player::GetGameObjectIfCanInteractWith(ObjectGuid guid, uint32 gameobject_type) const
{
    // some basic checks
    if (!guid || !IsInWorld() || IsTaxiFlying())
        return NULL;

    // not in interactive state
    if (hasUnitState(UNIT_STAT_CAN_NOT_REACT_OR_LOST_CONTROL))
        return NULL;

    if (GameObject* go = GetMap()->GetGameObject(guid))
    {
        if (uint32(go->GetGoType()) == gameobject_type || gameobject_type == MAX_GAMEOBJECT_TYPE)
        {
            float maxdist;
            switch (go->GetGoType())
            {
                    // TODO: find out how the client calculates the maximal usage distance to spellless working
                    // gameobjects like mailboxes - 10.0 is a just an abitrary choosen number
                case GAMEOBJECT_TYPE_MAILBOX:
                    maxdist = 10.0f;
                    break;
                case GAMEOBJECT_TYPE_FISHINGHOLE:
                    maxdist = 20.0f + CONTACT_DISTANCE;     // max spell range
                    break;
                default:
                    maxdist = INTERACTION_DISTANCE;
                    break;
            }

            if (go->IsWithinDistInMap(this, maxdist) && go->isSpawned())
                return go;

            sLog.outError("GetGameObjectIfCanInteractWith: GameObject '%s' [GUID: %u] is too far away from player %s [GUID: %u] to be used by him (distance=%f, maximal %f is allowed)",
                          go->GetGOInfo()->name,  go->GetGUIDLow(), GetName(), GetGUIDLow(), go->GetDistance(this), maxdist);
        }
    }
    return NULL;
}

bool Player::IsUnderWater() const
{
    return GetTerrain()->IsUnderWater(GetPositionX(), GetPositionY(), GetPositionZ() + 2);
}

void Player::SetInWater(bool apply)
{
    if (m_isInWater == apply)
        return;

    // define player in water by opcodes
    // move player's guid into HateOfflineList of those mobs
    // which can't swim and move guid back into ThreatList when
    // on surface.
    // TODO: exist also swimming mobs, and function must be symmetric to enter/leave water
    m_isInWater = apply;

    // remove auras that need water/land
    RemoveAurasWithInterruptFlags(apply ? AURA_INTERRUPT_FLAG_NOT_ABOVEWATER : AURA_INTERRUPT_FLAG_NOT_UNDERWATER);

    getHostileRefManager().updateThreatTables();
}

struct SetGameMasterOnHelper
{
    explicit SetGameMasterOnHelper() {}
    void operator()(Unit* unit) const
    {
        unit->setFaction(35);
        unit->getHostileRefManager().setOnlineOfflineState(false);
    }
};

struct SetGameMasterOffHelper
{
    explicit SetGameMasterOffHelper(uint32 _faction) : faction(_faction) {}
    void operator()(Unit* unit) const
    {
        unit->setFaction(faction);
        unit->getHostileRefManager().setOnlineOfflineState(true);
    }
    uint32 faction;
};

void Player::SetGameMaster(bool on)
{
    if (on)
    {
        m_ExtraFlags |= PLAYER_EXTRA_GM_ON;
        setFaction(35);
        SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_GM);

        CallForAllControlledUnits(SetGameMasterOnHelper(), CONTROLLED_PET | CONTROLLED_TOTEMS | CONTROLLED_GUARDIANS | CONTROLLED_CHARM);

        SetFFAPvP(false);
        ResetContestedPvP();

        getHostileRefManager().setOnlineOfflineState(false);
        CombatStopWithPets();
    }
    else
    {
        m_ExtraFlags &= ~ PLAYER_EXTRA_GM_ON;
        setFactionForRace(getRace());
        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_GM);

        CallForAllControlledUnits(SetGameMasterOffHelper(getFaction()), CONTROLLED_PET | CONTROLLED_TOTEMS | CONTROLLED_GUARDIANS | CONTROLLED_CHARM);

        // restore FFA PvP Server state
        if (sWorld.IsFFAPvPRealm())
            SetFFAPvP(true);

        // restore FFA PvP area state, remove not allowed for GM mounts
        UpdateArea(m_areaUpdateId);

        getHostileRefManager().setOnlineOfflineState(true);
    }

    m_camera.UpdateVisibilityForOwner();
    UpdateObjectVisibility();
    UpdateForQuestWorldObjects();
}

void Player::SetGMVisible(bool on)
{
    if (on)
    {
        m_ExtraFlags &= ~PLAYER_EXTRA_GM_INVISIBLE;         // remove flag

        // Reapply stealth/invisibility if active or show if not any
        if (HasAuraType(SPELL_AURA_MOD_STEALTH))
            SetVisibility(VISIBILITY_GROUP_STEALTH);
        else if (HasAuraType(SPELL_AURA_MOD_INVISIBILITY))
            SetVisibility(VISIBILITY_GROUP_INVISIBILITY);
        else
            SetVisibility(VISIBILITY_ON);
    }
    else
    {
        m_ExtraFlags |= PLAYER_EXTRA_GM_INVISIBLE;          // add flag

        SetAcceptWhispers(false);
        SetGameMaster(true);

        SetVisibility(VISIBILITY_OFF);
    }
}

bool Player::IsGroupVisibleFor(Player* p) const
{
    switch (sWorld.getConfig(CONFIG_UINT32_GROUP_VISIBILITY))
    {
        default: return IsInSameGroupWith(p);
        case 1:  return IsInSameRaidWith(p);
        case 2:  return GetTeam() == p->GetTeam();
    }
}

bool Player::IsInSameGroupWith(Player const* p) const
{
    return (p == this || (GetGroup() != NULL &&
                          GetGroup()->SameSubGroup(this, p)));
}

///- If the player is invited, remove him. If the group if then only 1 person, disband the group.
/// \todo Shouldn't we also check if there is no other invitees before disbanding the group?
void Player::UninviteFromGroup()
{
    Group* group = GetGroupInvite();
    if (!group)
        return;

    group->RemoveInvite(this);

    if (group->GetMembersCount() <= 1)                      // group has just 1 member => disband
    {
        if (group->IsCreated())
        {
            group->Disband(true);
            sObjectMgr.RemoveGroup(group);
        }
        else
            group->RemoveAllInvites();

        delete group;
    }
}

void Player::RemoveFromGroup(Group* group, ObjectGuid guid)
{
    if (group)
    {
        if (group->RemoveMember(guid, 0) <= 1)
        {
            // group->Disband(); already disbanded in RemoveMember
            sObjectMgr.RemoveGroup(group);
            delete group;
            // removemember sets the player's group pointer to NULL
        }
    }
}

void Player::SendLogXPGain(uint32 GivenXP, Unit* victim, uint32 RestXP)
{
    WorldPacket data(SMSG_LOG_XPGAIN, 21);
    data << (victim ? victim->GetObjectGuid() : ObjectGuid());// guid
    data << uint32(GivenXP + RestXP);                       // given experience
    data << uint8(victim ? 0 : 1);                          // 00-kill_xp type, 01-non_kill_xp type
    if (victim)
    {
        data << uint32(GivenXP);                            // experience without rested bonus
        data << float(1);                                   // 1 - none 0 - 100% group bonus output
    }
    GetSession()->SendPacket(&data);
}

void Player::GiveXP(uint32 xp, Unit* victim)
{
    if (xp < 1)
        return;

    if (!isAlive())
        return;

    uint32 level = getLevel();

    // used by eluna
    sHookMgr->OnGiveXP(this, xp, victim);

    // XP to money conversion processed in Player::RewardQuest
    if (level >= sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
        return;

    // XP resting bonus for kill
    uint32 rested_bonus_xp = victim ? GetXPRestBonus(xp) : 0;

    SendLogXPGain(xp, victim, rested_bonus_xp);

    uint32 curXP = GetUInt32Value(PLAYER_XP);
    uint32 nextLvlXP = GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
    uint32 newXP = curXP + xp + rested_bonus_xp;

    while (newXP >= nextLvlXP && level < sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
    {
        newXP -= nextLvlXP;

        if (level < sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
            GiveLevel(level + 1);

        level = getLevel();
        nextLvlXP = GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
    }

    SetUInt32Value(PLAYER_XP, newXP);
}

// Update player to next level
// Current player experience not update (must be update by caller)
void Player::GiveLevel(uint32 level)
{
    uint8 oldLevel = getLevel();
    if (level == getLevel())
        return;

    PlayerLevelInfo info;
    sObjectMgr.GetPlayerLevelInfo(getRace(), getClass(), level, &info);

    PlayerClassLevelInfo classInfo;
    sObjectMgr.GetPlayerClassLevelInfo(getClass(), level, &classInfo);

    // send levelup info to client
    WorldPacket data(SMSG_LEVELUP_INFO, (4 + 4 + MAX_POWERS * 4 + MAX_STATS * 4));
    data << uint32(level);
    data << uint32(int32(classInfo.basehealth) - int32(GetCreateHealth()));
    // for(int i = 0; i < MAX_POWERS; ++i)                  // Powers loop (0-6)
    data << uint32(int32(classInfo.basemana)   - int32(GetCreateMana()));
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    data << uint32(0);
    // end for
    for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)         // Stats loop (0-4)
        data << uint32(int32(info.stats[i]) - GetCreateStat(Stats(i)));

    GetSession()->SendPacket(&data);

    SetUInt32Value(PLAYER_NEXT_LEVEL_XP, sObjectMgr.GetXPForLevel(level));

    // update level, max level of skills
    if (getLevel() != level)
        m_Played_time[PLAYED_TIME_LEVEL] = 0;               // Level Played Time reset
    SetLevel(level);
    UpdateSkillsForLevel();

    // save base values (bonuses already included in stored stats
    for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        SetCreateStat(Stats(i), info.stats[i]);

    SetCreateHealth(classInfo.basehealth);
    SetCreateMana(classInfo.basemana);

    InitTalentForLevel();

    UpdateAllStats();

    // set current level health and mana/energy to maximum after applying all mods.
    if (isAlive())
        SetHealth(GetMaxHealth());
    SetPower(POWER_MANA, GetMaxPower(POWER_MANA));
    SetPower(POWER_ENERGY, GetMaxPower(POWER_ENERGY));
    if (GetPower(POWER_RAGE) > GetMaxPower(POWER_RAGE))
        SetPower(POWER_RAGE, GetMaxPower(POWER_RAGE));
    SetPower(POWER_FOCUS, 0);
    SetPower(POWER_HAPPINESS, 0);

    // update level to hunter/summon pet
    if (Pet* pet = GetPet())
        pet->SynchronizeLevelWithOwner();

    // used by eluna
    sHookMgr->OnLevelChanged(this, oldLevel);
}

void Player::SetFreeTalentPoints(uint32 points)
{
    // used by eluna
    sHookMgr->OnFreeTalentPointsChanged(this, points);
    SetUInt32Value(PLAYER_CHARACTER_POINTS1, points);
}

void Player::UpdateFreeTalentPoints(bool resetIfNeed)
{
    uint32 level = getLevel();
    // talents base at level diff ( talents = level - 9 but some can be used already)
    if (level < 10)
    {
        // Remove all talent points
        if (m_usedTalentCount > 0)                          // Free any used talents
        {
            if (resetIfNeed)
                resetTalents(true);
            SetFreeTalentPoints(0);
        }
    }
    else
    {
        uint32 talentPointsForLevel = CalculateTalentsPoints();

        // if used more that have then reset
        if (m_usedTalentCount > talentPointsForLevel)
        {
            if (resetIfNeed && GetSession()->GetSecurity() < SEC_ADMINISTRATOR)
                resetTalents(true);
            else
                SetFreeTalentPoints(0);
        }
        // else update amount of free points
        else
            SetFreeTalentPoints(talentPointsForLevel - m_usedTalentCount);
    }
}

void Player::InitTalentForLevel()
{
    UpdateFreeTalentPoints();
}

void Player::InitStatsForLevel(bool reapplyMods)
{
    if (reapplyMods)                                        // reapply stats values only on .reset stats (level) command
        _RemoveAllStatBonuses();

    PlayerClassLevelInfo classInfo;
    sObjectMgr.GetPlayerClassLevelInfo(getClass(), getLevel(), &classInfo);

    PlayerLevelInfo info;
    sObjectMgr.GetPlayerLevelInfo(getRace(), getClass(), getLevel(), &info);

    SetUInt32Value(PLAYER_NEXT_LEVEL_XP, sObjectMgr.GetXPForLevel(getLevel()));

    // reset before any aura state sources (health set/aura apply)
    SetUInt32Value(UNIT_FIELD_AURASTATE, 0);

    UpdateSkillsForLevel();

    // set default cast time multiplier
    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);

    // save base values (bonuses already included in stored stats
    for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        SetCreateStat(Stats(i), info.stats[i]);

    for (int i = STAT_STRENGTH; i < MAX_STATS; ++i)
        SetStat(Stats(i), info.stats[i]);

    SetCreateHealth(classInfo.basehealth);

    // set create powers
    SetCreateMana(classInfo.basemana);

    SetArmor(int32(m_createStats[STAT_AGILITY] * 2));

    InitStatBuffMods();

    //[-ZERO] SetUInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS,0);
    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
    {
        SetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + i, 0);
        SetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + i, 0);
        SetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT + i, 1.00f);
    }

    // reset attack power, damage and attack speed fields
    SetFloatValue(UNIT_FIELD_BASEATTACKTIME, 2000.0f);
    SetFloatValue(UNIT_FIELD_BASEATTACKTIME + 1, 2000.0f);  // offhand attack time
    SetFloatValue(UNIT_FIELD_RANGEDATTACKTIME, 2000.0f);

    SetFloatValue(UNIT_FIELD_MINDAMAGE, 0.0f);
    SetFloatValue(UNIT_FIELD_MAXDAMAGE, 0.0f);
    SetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, 0.0f);
    SetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, 0.0f);
    SetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE, 0.0f);
    SetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE, 0.0f);

    SetInt32Value(UNIT_FIELD_ATTACK_POWER,            0);
    SetInt32Value(UNIT_FIELD_ATTACK_POWER_MODS,       0);
    SetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER, 0.0f);
    SetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER,     0);
    SetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS, 0);
    SetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER, 0.0f);

    // Base crit values (will be recalculated in UpdateAllStats() at loading and in _ApplyAllStatBonuses() at reset
    SetFloatValue(PLAYER_CRIT_PERCENTAGE, 0.0f);
    SetFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE, 0.0f);

    // Init spell schools (will be recalculated in UpdateAllStats() at loading and in _ApplyAllStatBonuses() at reset
    for (uint8 i = 0; i < MAX_SPELL_SCHOOL; ++i)
        m_SpellCritPercentage[i] = 0.0f;

    SetFloatValue(PLAYER_PARRY_PERCENTAGE, 0.0f);
    SetFloatValue(PLAYER_BLOCK_PERCENTAGE, 0.0f);

    // Dodge percentage
    SetFloatValue(PLAYER_DODGE_PERCENTAGE, 0.0f);

    // set armor (resistance 0) to original value (create_agility*2)
    SetArmor(int32(m_createStats[STAT_AGILITY] * 2));
    SetResistanceBuffMods(SpellSchools(0), true, 0.0f);
    SetResistanceBuffMods(SpellSchools(0), false, 0.0f);
    // set other resistance to original value (0)
    for (int i = 1; i < MAX_SPELL_SCHOOL; ++i)
    {
        SetResistance(SpellSchools(i), 0);
        SetResistanceBuffMods(SpellSchools(i), true, 0.0f);
        SetResistanceBuffMods(SpellSchools(i), false, 0.0f);
    }

//[-ZERO]    SetUInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE,0);
//[-ZERO]    SetUInt32Value(PLAYER_FIELD_MOD_TARGET_PHYSICAL_RESISTANCE,0);
    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
    {
        SetUInt32Value(UNIT_FIELD_POWER_COST_MODIFIER + i, 0);
        SetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER + i, 0.0f);
    }
    // Init data for form but skip reapply item mods for form
    InitDataForForm(reapplyMods);

    // save new stats
    for (int i = POWER_MANA; i < MAX_POWERS; ++i)
        SetMaxPower(Powers(i),  GetCreatePowers(Powers(i)));

    SetMaxHealth(classInfo.basehealth);                     // stamina bonus will applied later

    // cleanup mounted state (it will set correctly at aura loading if player saved at mount.
    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);

    // cleanup unit flags (will be re-applied if need at aura load).
    RemoveFlag(UNIT_FIELD_FLAGS,
               UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_DISABLE_MOVE | UNIT_FLAG_NOT_ATTACKABLE_1 |
               UNIT_FLAG_OOC_NOT_ATTACKABLE | UNIT_FLAG_PASSIVE  | UNIT_FLAG_LOOTING          |
               UNIT_FLAG_PET_IN_COMBAT  | UNIT_FLAG_SILENCED     | UNIT_FLAG_PACIFIED         |
               UNIT_FLAG_STUNNED        | UNIT_FLAG_IN_COMBAT    | UNIT_FLAG_DISARMED         |
               UNIT_FLAG_CONFUSED       | UNIT_FLAG_FLEEING      | UNIT_FLAG_NOT_SELECTABLE   |
               UNIT_FLAG_SKINNABLE      | UNIT_FLAG_TAXI_FLIGHT);
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);    // must be set

    // cleanup player flags (will be re-applied if need at aura load), to avoid have ghost flag without ghost aura, for example.
    RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_AFK | PLAYER_FLAGS_DND | PLAYER_FLAGS_GM | PLAYER_FLAGS_GHOST | PLAYER_FLAGS_FFA_PVP);

    // one form stealth modified bytes
    RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALL);

    // restore if need some important flags
    SetUInt32Value(PLAYER_FIELD_BYTES2, 0);                 // flags empty by default

    if (reapplyMods)                                        // reapply stats values only on .reset stats (level) command
        _ApplyAllStatBonuses();

    // set current level health and mana/energy to maximum after applying all mods.
    SetHealth(GetMaxHealth());
    SetPower(POWER_MANA, GetMaxPower(POWER_MANA));
    SetPower(POWER_ENERGY, GetMaxPower(POWER_ENERGY));
    if (GetPower(POWER_RAGE) > GetMaxPower(POWER_RAGE))
        SetPower(POWER_RAGE, GetMaxPower(POWER_RAGE));
    SetPower(POWER_FOCUS, 0);
    SetPower(POWER_HAPPINESS, 0);

    // update level to hunter/summon pet
    if (Pet* pet = GetPet())
        pet->SynchronizeLevelWithOwner();
}

void Player::SendInitialSpells()
{
    time_t curTime = time(NULL);
    time_t infTime = curTime + infinityCooldownDelayCheck;

    uint16 spellCount = 0;

    WorldPacket data(SMSG_INITIAL_SPELLS, (1 + 2 + 4 * m_spells.size() + 2 + m_spellCooldowns.size() * (2 + 2 + 2 + 4 + 4)));
    data << uint8(0);

    size_t countPos = data.wpos();
    data << uint16(spellCount);                             // spell count placeholder

    for (PlayerSpellMap::const_iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        if (itr->second.state == PLAYERSPELL_REMOVED)
            continue;

        if (!itr->second.active || itr->second.disabled)
            continue;

        data << uint16(itr->first);
        data << uint16(0);                                  // it's not slot id

        spellCount += 1;
    }

    data.put<uint16>(countPos, spellCount);                 // write real count value

    uint16 spellCooldowns = m_spellCooldowns.size();
    data << uint16(spellCooldowns);
    for (SpellCooldowns::const_iterator itr = m_spellCooldowns.begin(); itr != m_spellCooldowns.end(); ++itr)
    {
        SpellEntry const* sEntry = sSpellStore.LookupEntry(itr->first);
        if (!sEntry)
            continue;

        data << uint16(itr->first);

        data << uint16(itr->second.itemid);                 // cast item id
        data << uint16(sEntry->Category);                   // spell category

        // send infinity cooldown in special format
        if (itr->second.end >= infTime)
        {
            data << uint32(1);                              // cooldown
            data << uint32(0x80000000);                     // category cooldown
            continue;
        }

        time_t cooldown = itr->second.end > curTime ? (itr->second.end - curTime) * IN_MILLISECONDS : 0;

        if (sEntry->Category)                               // may be wrong, but anyway better than nothing...
        {
            data << uint32(0);                              // cooldown
            data << uint32(cooldown);                       // category cooldown
        }
        else
        {
            data << uint32(cooldown);                       // cooldown
            data << uint32(0);                              // category cooldown
        }
    }

    GetSession()->SendPacket(&data);

    DETAIL_LOG("CHARACTER: Sent Initial Spells");
}

void Player::RemoveMail(uint32 id)
{
    for (PlayerMails::iterator itr = m_mail.begin(); itr != m_mail.end(); ++itr)
    {
        if ((*itr)->messageID == id)
        {
            // do not delete item, because Player::removeMail() is called when returning mail to sender.
            m_mail.erase(itr);
            return;
        }
    }
}

void Player::SendMailResult(uint32 mailId, MailResponseType mailAction, MailResponseResult mailError, uint32 equipError, uint32 item_guid, uint32 item_count)
{
    WorldPacket data(SMSG_SEND_MAIL_RESULT, (4 + 4 + 4 + (mailError == MAIL_ERR_EQUIP_ERROR ? 4 : (mailAction == MAIL_ITEM_TAKEN ? 4 + 4 : 0))));
    data << (uint32) mailId;
    data << (uint32) mailAction;
    data << (uint32) mailError;
    if (mailError == MAIL_ERR_EQUIP_ERROR)
        data << (uint32) equipError;
    else if (mailAction == MAIL_ITEM_TAKEN)
    {
        data << (uint32) item_guid;                         // item guid low?
        data << (uint32) item_count;                        // item count?
    }
    GetSession()->SendPacket(&data);
}

void Player::SendNewMail()
{
    // deliver undelivered mail
    WorldPacket data(SMSG_RECEIVED_MAIL, 4);
    data << (uint32) 0;
    GetSession()->SendPacket(&data);
}

void Player::UpdateNextMailTimeAndUnreads()
{
    // calculate next delivery time (min. from non-delivered mails
    // and recalculate unReadMail
    time_t cTime = time(NULL);
    m_nextMailDelivereTime = 0;
    unReadMails = 0;
    for (PlayerMails::iterator itr = m_mail.begin(); itr != m_mail.end(); ++itr)
    {
        if ((*itr)->deliver_time > cTime)
        {
            if (!m_nextMailDelivereTime || m_nextMailDelivereTime > (*itr)->deliver_time)
                m_nextMailDelivereTime = (*itr)->deliver_time;
        }
        else if (((*itr)->checked & MAIL_CHECK_MASK_READ) == 0)
            ++unReadMails;
    }
}

void Player::AddNewMailDeliverTime(time_t deliver_time)
{
    if (deliver_time <= time(NULL))                         // ready now
    {
        ++unReadMails;
        SendNewMail();
    }
    else                                                    // not ready and no have ready mails
    {
        if (!m_nextMailDelivereTime || m_nextMailDelivereTime > deliver_time)
            m_nextMailDelivereTime =  deliver_time;
    }
}

bool Player::addSpell(uint32 spell_id, bool active, bool learning, bool dependent, bool disabled)
{
    SpellEntry const* spellInfo = sSpellStore.LookupEntry(spell_id);
    if (!spellInfo)
    {
        // do character spell book cleanup (all characters)
        if (!IsInWorld() && !learning)                      // spell load case
        {
            sLog.outError("Player::addSpell: nonexistent in SpellStore spell #%u request, deleting for all characters in `character_spell`.", spell_id);
            CharacterDatabase.PExecute("DELETE FROM character_spell WHERE spell = '%u'", spell_id);
        }
        else
            sLog.outError("Player::addSpell: nonexistent in SpellStore spell #%u request.", spell_id);

        return false;
    }

    if (!SpellMgr::IsSpellValid(spellInfo, this, false))
    {
        // do character spell book cleanup (all characters)
        if (!IsInWorld() && !learning)                      // spell load case
        {
            sLog.outError("Player::addSpell: Broken spell #%u learning not allowed, deleting for all characters in `character_spell`.", spell_id);
            CharacterDatabase.PExecute("DELETE FROM character_spell WHERE spell = '%u'", spell_id);
        }
        else
            sLog.outError("Player::addSpell: Broken spell #%u learning not allowed.", spell_id);

        return false;
    }

    PlayerSpellState state = learning ? PLAYERSPELL_NEW : PLAYERSPELL_UNCHANGED;

    bool dependent_set = false;
    bool disabled_case = false;

    PlayerSpellMap::iterator itr = m_spells.find(spell_id);
    if (itr != m_spells.end())
    {
        uint32 next_active_spell_id = 0;
        // fix activate state for non-stackable low rank (and find next spell for !active case)
        if (sSpellMgr.IsRankedSpellNonStackableInSpellBook(spellInfo))
        {
            SpellChainMapNext const& nextMap = sSpellMgr.GetSpellChainNext();
            for (SpellChainMapNext::const_iterator next_itr = nextMap.lower_bound(spell_id); next_itr != nextMap.upper_bound(spell_id); ++next_itr)
            {
                if (HasSpell(next_itr->second))
                {
                    // high rank already known so this must !active
                    active = false;
                    next_active_spell_id = next_itr->second;
                    break;
                }
            }
        }

        // not do anything if already known in expected state
        if (itr->second.state != PLAYERSPELL_REMOVED && itr->second.active == active &&
                itr->second.dependent == dependent && itr->second.disabled == disabled)
        {
            if (!IsInWorld() && !learning)                  // explicitly load from DB and then exist in it already and set correctly
                itr->second.state = PLAYERSPELL_UNCHANGED;

            return false;
        }

        // dependent spell known as not dependent, overwrite state
        if (itr->second.state != PLAYERSPELL_REMOVED && !itr->second.dependent && dependent)
        {
            itr->second.dependent = dependent;
            if (itr->second.state != PLAYERSPELL_NEW)
                itr->second.state = PLAYERSPELL_CHANGED;
            dependent_set = true;
        }

        // update active state for known spell
        if (itr->second.active != active && itr->second.state != PLAYERSPELL_REMOVED && !itr->second.disabled)
        {
            itr->second.active = active;

            if (!IsInWorld() && !learning && !dependent_set)// explicitly load from DB and then exist in it already and set correctly
                itr->second.state = PLAYERSPELL_UNCHANGED;
            else if (itr->second.state != PLAYERSPELL_NEW)
                itr->second.state = PLAYERSPELL_CHANGED;

            if (active)
            {
                if (IsNeedCastPassiveLikeSpellAtLearn(spellInfo))
                    CastSpell(this, spell_id, true);
            }
            else if (IsInWorld())
            {
                if (next_active_spell_id)
                {
                    // update spell ranks in spellbook and action bar
                    WorldPacket data(SMSG_SUPERCEDED_SPELL, (4));
                    data << uint16(spell_id);
                    data << uint16(next_active_spell_id);
                    GetSession()->SendPacket(&data);
                }
                else
                {
                    WorldPacket data(SMSG_REMOVED_SPELL, 4);
                    data << uint16(spell_id);
                    GetSession()->SendPacket(&data);
                }
            }

            return active;                                  // learn (show in spell book if active now)
        }

        if (itr->second.disabled != disabled && itr->second.state != PLAYERSPELL_REMOVED)
        {
            if (itr->second.state != PLAYERSPELL_NEW)
                itr->second.state = PLAYERSPELL_CHANGED;
            itr->second.disabled = disabled;

            if (disabled)
                return false;

            disabled_case = true;
        }
        else switch (itr->second.state)
            {
                case PLAYERSPELL_UNCHANGED:                 // known saved spell
                    return false;
                case PLAYERSPELL_REMOVED:                   // re-learning removed not saved spell
                {
                    m_spells.erase(itr);
                    state = PLAYERSPELL_CHANGED;
                    break;                                  // need re-add
                }
                default:                                    // known not saved yet spell (new or modified)
                {
                    // can be in case spell loading but learned at some previous spell loading
                    if (!IsInWorld() && !learning && !dependent_set)
                        itr->second.state = PLAYERSPELL_UNCHANGED;

                    return false;
                }
            }
    }

    TalentSpellPos const* talentPos = GetTalentSpellPos(spell_id);

    if (!disabled_case) // skip new spell adding if spell already known (disabled spells case)
    {
        // talent: unlearn all other talent ranks (high and low)
        if (talentPos)
        {
            if (TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentPos->talent_id))
            {
                for (int i = 0; i < MAX_TALENT_RANK; ++i)
                {
                    // skip learning spell and no rank spell case
                    uint32 rankSpellId = talentInfo->RankID[i];
                    if (!rankSpellId || rankSpellId == spell_id)
                        continue;

                    removeSpell(rankSpellId, false, false);
                }
            }
        }
        // non talent spell: learn low ranks (recursive call)
        else if (uint32 prev_spell = sSpellMgr.GetPrevSpellInChain(spell_id))
        {
            if (!IsInWorld() || disabled)                   // at spells loading, no output, but allow save
                addSpell(prev_spell, active, true, true, disabled);
            else                                            // at normal learning
                learnSpell(prev_spell, true);
        }

        PlayerSpell newspell;
        newspell.state     = state;
        newspell.active    = active;
        newspell.dependent = dependent;
        newspell.disabled  = disabled;

        // replace spells in action bars and spellbook to bigger rank if only one spell rank must be accessible
        if (newspell.active && !newspell.disabled && sSpellMgr.IsRankedSpellNonStackableInSpellBook(spellInfo))
        {
            for (PlayerSpellMap::iterator itr2 = m_spells.begin(); itr2 != m_spells.end(); ++itr2)
            {
                if (itr2->second.state == PLAYERSPELL_REMOVED) continue;
                SpellEntry const* i_spellInfo = sSpellStore.LookupEntry(itr2->first);
                if (!i_spellInfo) continue;

                if (sSpellMgr.IsRankSpellDueToSpell(spellInfo, itr2->first))
                {
                    if (itr2->second.active)
                    {
                        if (sSpellMgr.IsHighRankOfSpell(spell_id, itr2->first))
                        {
                            if (IsInWorld())                // not send spell (re-/over-)learn packets at loading
                            {
                                WorldPacket data(SMSG_SUPERCEDED_SPELL, (4));
                                data << uint16(itr2->first);
                                data << uint16(spell_id);
                                GetSession()->SendPacket(&data);
                            }

                            // mark old spell as disable (SMSG_SUPERCEDED_SPELL replace it in client by new)
                            itr2->second.active = false;
                            if (itr2->second.state != PLAYERSPELL_NEW)
                                itr2->second.state = PLAYERSPELL_CHANGED;
                        }
                        else if (sSpellMgr.IsHighRankOfSpell(itr2->first, spell_id))
                        {
                            if (IsInWorld())                // not send spell (re-/over-)learn packets at loading
                            {
                                WorldPacket data(SMSG_SUPERCEDED_SPELL, (4));
                                data << uint16(spell_id);
                                data << uint16(itr2->first);
                                GetSession()->SendPacket(&data);
                            }

                            // mark new spell as disable (not learned yet for client and will not learned)
                            newspell.active = false;
                            if (newspell.state != PLAYERSPELL_NEW)
                                newspell.state = PLAYERSPELL_CHANGED;
                        }
                    }
                }
            }
        }

        m_spells[spell_id] = newspell;

        // return false if spell disabled
        if (newspell.disabled)
            return false;
    }

    if (talentPos)
    {
        // update used talent points count
        m_usedTalentCount += GetTalentSpellCost(talentPos);
        UpdateFreeTalentPoints(false);
    }

    // update free primary prof.points (if any, can be none in case GM .learn prof. learning)
    if (uint32 freeProfs = GetFreePrimaryProfessionPoints())
    {
        if (sSpellMgr.IsPrimaryProfessionFirstRankSpell(spell_id))
            SetFreePrimaryProfessions(freeProfs - 1);
    }

    // cast talents with SPELL_EFFECT_LEARN_SPELL (other dependent spells will learned later as not auto-learned)
    // note: all spells with SPELL_EFFECT_LEARN_SPELL isn't passive
    if (talentPos && IsSpellHaveEffect(spellInfo, SPELL_EFFECT_LEARN_SPELL))
    {
        // ignore stance requirement for talent learn spell (stance set for spell only for client spell description show)
        CastSpell(this, spell_id, true);
    }
    // also cast passive (and passive like) spells (including all talents without SPELL_EFFECT_LEARN_SPELL) with additional checks
    else if (IsNeedCastPassiveLikeSpellAtLearn(spellInfo))
    {
        CastSpell(this, spell_id, true);
    }
    else if (IsSpellHaveEffect(spellInfo, SPELL_EFFECT_SKILL_STEP))
    {
        CastSpell(this, spell_id, true);
        return false;
    }

    // add dependent skills
    uint16 maxskill = GetMaxSkillValueForLevel();

    SpellLearnSkillNode const* spellLearnSkill = sSpellMgr.GetSpellLearnSkill(spell_id);

    if (spellLearnSkill)
    {
        uint32 skill_value = GetPureSkillValue(spellLearnSkill->skill);
        uint32 skill_max_value = GetPureMaxSkillValue(spellLearnSkill->skill);

        if (skill_value < spellLearnSkill->value)
            skill_value = spellLearnSkill->value;

        uint32 new_skill_max_value = spellLearnSkill->maxvalue == 0 ? maxskill : spellLearnSkill->maxvalue;

        if (skill_max_value < new_skill_max_value)
            skill_max_value =  new_skill_max_value;

        SetSkill(spellLearnSkill->skill, skill_value, skill_max_value, spellLearnSkill->step);
    }
    else
    {
        // not ranked skills
        SkillLineAbilityMapBounds skill_bounds = sSpellMgr.GetSkillLineAbilityMapBounds(spell_id);

        for (SkillLineAbilityMap::const_iterator _spell_idx = skill_bounds.first; _spell_idx != skill_bounds.second; ++_spell_idx)
        {
            SkillLineEntry const* pSkill = sSkillLineStore.LookupEntry(_spell_idx->second->skillId);
            if (!pSkill)
                continue;

            if (HasSkill(pSkill->id))
                continue;

            if (_spell_idx->second->learnOnGetSkill == ABILITY_LEARNED_ON_GET_RACE_OR_CLASS_SKILL ||
                    // poison special case, not have ABILITY_LEARNED_ON_GET_RACE_OR_CLASS_SKILL
                    pSkill->id == SKILL_POISONS && _spell_idx->second->max_value == 0 ||
                    // lockpicking special case, not have ABILITY_LEARNED_ON_GET_RACE_OR_CLASS_SKILL
                    pSkill->id == SKILL_LOCKPICKING && _spell_idx->second->max_value == 0)
            {
                switch (GetSkillRangeType(pSkill, _spell_idx->second->racemask != 0))
                {
                    case SKILL_RANGE_LANGUAGE:
                        SetSkill(pSkill->id, 300, 300);
                        break;
                    case SKILL_RANGE_LEVEL:
                        SetSkill(pSkill->id, 1, GetMaxSkillValueForLevel());
                        break;
                    case SKILL_RANGE_MONO:
                        SetSkill(pSkill->id, 1, 1);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    // learn dependent spells
    SpellLearnSpellMapBounds spell_bounds = sSpellMgr.GetSpellLearnSpellMapBounds(spell_id);

    for (SpellLearnSpellMap::const_iterator itr2 = spell_bounds.first; itr2 != spell_bounds.second; ++itr2)
    {
        if (!itr2->second.autoLearned)
        {
            if (!IsInWorld() || !itr2->second.active)       // at spells loading, no output, but allow save
                addSpell(itr2->second.spell, itr2->second.active, true, true, false);
            else                                            // at normal learning
                learnSpell(itr2->second.spell, true);
        }
    }

    // return true (for send learn packet) only if spell active (in case ranked spells) and not replace old spell
    return active && !disabled;
}

bool Player::IsNeedCastPassiveLikeSpellAtLearn(SpellEntry const* spellInfo) const
{
    ShapeshiftForm form = GetShapeshiftForm();

    if (IsNeedCastSpellAtFormApply(spellInfo, form))        // SPELL_ATTR_PASSIVE | SPELL_ATTR_UNK7 spells
        return true;                                        // all stance req. cases, not have auarastate cases

    if (!spellInfo->HasAttribute(SPELL_ATTR_PASSIVE))
        return false;

    // note: form passives activated with shapeshift spells be implemented by HandleShapeshiftBoosts instead of spell_learn_spell
    // talent dependent passives activated at form apply have proper stance data
    bool need_cast = !spellInfo->Stances || (!form && spellInfo->HasAttribute(SPELL_ATTR_EX2_NOT_NEED_SHAPESHIFT));

    // Check CasterAuraStates
    return need_cast && (!spellInfo->CasterAuraState || HasAuraState(AuraState(spellInfo->CasterAuraState)));
}

void Player::learnSpell(uint32 spell_id, bool dependent)
{
    PlayerSpellMap::iterator itr = m_spells.find(spell_id);

    bool disabled = (itr != m_spells.end()) ? itr->second.disabled : false;
    bool active = disabled ? itr->second.active : true;

    bool learning = addSpell(spell_id, active, true, dependent, false);

    // prevent duplicated entires in spell book, also not send if not in world (loading)
    if (learning && IsInWorld())
    {
        WorldPacket data(SMSG_LEARNED_SPELL, 4);
        data << uint32(spell_id);
        GetSession()->SendPacket(&data);
    }

    // learn all disabled higher ranks (recursive)
    if (disabled)
    {
        SpellChainMapNext const& nextMap = sSpellMgr.GetSpellChainNext();
        for (SpellChainMapNext::const_iterator i = nextMap.lower_bound(spell_id); i != nextMap.upper_bound(spell_id); ++i)
        {
            PlayerSpellMap::iterator iter = m_spells.find(i->second);
            if (iter != m_spells.end() && iter->second.disabled)
                learnSpell(i->second, false);
        }
    }
}

void Player::removeSpell(uint32 spell_id, bool disabled, bool learn_low_rank)
{
    PlayerSpellMap::iterator itr = m_spells.find(spell_id);
    if (itr == m_spells.end())
        return;

    if (itr->second.state == PLAYERSPELL_REMOVED || (disabled && itr->second.disabled))
        return;

    // unlearn non talent higher ranks (recursive)
    SpellChainMapNext const& nextMap = sSpellMgr.GetSpellChainNext();
    for (SpellChainMapNext::const_iterator itr2 = nextMap.lower_bound(spell_id); itr2 != nextMap.upper_bound(spell_id); ++itr2)
        if (HasSpell(itr2->second) && !GetTalentSpellPos(itr2->second))
            removeSpell(itr2->second, disabled, false);

    // re-search, it can be corrupted in prev loop
    itr = m_spells.find(spell_id);
    if (itr == m_spells.end() || itr->second.state == PLAYERSPELL_REMOVED)
        return;                                             // already unleared

    bool cur_active    = itr->second.active;
    bool cur_dependent = itr->second.dependent;

    if (disabled)
    {
        itr->second.disabled = disabled;
        if (itr->second.state != PLAYERSPELL_NEW)
            itr->second.state = PLAYERSPELL_CHANGED;
    }
    else
    {
        if (itr->second.state == PLAYERSPELL_NEW)
            m_spells.erase(itr);
        else
            itr->second.state = PLAYERSPELL_REMOVED;
    }

    RemoveAurasDueToSpell(spell_id);

    // remove pet auras
    if (PetAura const* petSpell = sSpellMgr.GetPetAura(spell_id))
        RemovePetAura(petSpell);

    TalentSpellPos const* talentPos = GetTalentSpellPos(spell_id);
    if (talentPos)
    {
        // free talent points
        uint32 talentCosts = GetTalentSpellCost(talentPos);

        if (talentCosts < m_usedTalentCount)
            m_usedTalentCount -= talentCosts;
        else
            m_usedTalentCount = 0;

        UpdateFreeTalentPoints(false);
    }

    // update free primary prof.points (if not overflow setting, can be in case GM use before .learn prof. learning)
    if (sSpellMgr.IsPrimaryProfessionFirstRankSpell(spell_id))
    {
        uint32 freeProfs = GetFreePrimaryProfessionPoints() + 1;
        uint32 maxProfs = GetSession()->GetSecurity() < AccountTypes(sWorld.getConfig(CONFIG_UINT32_TRADE_SKILL_GMIGNORE_MAX_PRIMARY_COUNT)) ? sWorld.getConfig(CONFIG_UINT32_MAX_PRIMARY_TRADE_SKILL) : 10;
        if (freeProfs <= maxProfs)
            SetFreePrimaryProfessions(freeProfs);
    }

    // remove dependent skill
    SpellLearnSkillNode const* spellLearnSkill = sSpellMgr.GetSpellLearnSkill(spell_id);
    if (spellLearnSkill)
    {
        uint32 prev_spell = sSpellMgr.GetPrevSpellInChain(spell_id);
        if (!prev_spell)                                    // first rank, remove skill
            SetSkill(spellLearnSkill->skill, 0, 0);
        else
        {
            // search prev. skill setting by spell ranks chain
            SpellLearnSkillNode const* prevSkill = sSpellMgr.GetSpellLearnSkill(prev_spell);
            while (!prevSkill && prev_spell)
            {
                prev_spell = sSpellMgr.GetPrevSpellInChain(prev_spell);
                prevSkill = sSpellMgr.GetSpellLearnSkill(sSpellMgr.GetFirstSpellInChain(prev_spell));
            }

            if (!prevSkill)                                 // not found prev skill setting, remove skill
                SetSkill(spellLearnSkill->skill, 0, 0);
            else                                            // set to prev. skill setting values
            {
                uint32 skill_value = GetPureSkillValue(prevSkill->skill);
                uint32 skill_max_value = GetPureMaxSkillValue(prevSkill->skill);

                if (skill_value >  prevSkill->value)
                    skill_value = prevSkill->value;

                uint32 new_skill_max_value = prevSkill->maxvalue == 0 ? GetMaxSkillValueForLevel() : prevSkill->maxvalue;

                if (skill_max_value > new_skill_max_value)
                    skill_max_value =  new_skill_max_value;

                SetSkill(prevSkill->skill, skill_value, skill_max_value, prevSkill->step);
            }
        }
    }
    else
    {
        // not ranked skills
        SkillLineAbilityMapBounds bounds = sSpellMgr.GetSkillLineAbilityMapBounds(spell_id);

        for (SkillLineAbilityMap::const_iterator _spell_idx = bounds.first; _spell_idx != bounds.second; ++_spell_idx)
        {
            SkillLineEntry const* pSkill = sSkillLineStore.LookupEntry(_spell_idx->second->skillId);
            if (!pSkill)
                continue;

            if ((_spell_idx->second->learnOnGetSkill == ABILITY_LEARNED_ON_GET_RACE_OR_CLASS_SKILL &&
                    pSkill->categoryId != SKILL_CATEGORY_CLASS) ||// not unlearn class skills (spellbook/talent pages)
                    // poisons/lockpicking special case, not have ABILITY_LEARNED_ON_GET_RACE_OR_CLASS_SKILL
                    ((pSkill->id == SKILL_POISONS || pSkill->id == SKILL_LOCKPICKING) && _spell_idx->second->max_value == 0))
            {
                // not reset skills for professions and racial abilities
                if ((pSkill->categoryId == SKILL_CATEGORY_SECONDARY || pSkill->categoryId == SKILL_CATEGORY_PROFESSION) &&
                        (IsProfessionSkill(pSkill->id) || _spell_idx->second->racemask != 0))
                    continue;

                SetSkill(pSkill->id, 0, 0);
            }
        }
    }

    // remove dependent spells
    SpellLearnSpellMapBounds spell_bounds = sSpellMgr.GetSpellLearnSpellMapBounds(spell_id);

    for (SpellLearnSpellMap::const_iterator itr2 = spell_bounds.first; itr2 != spell_bounds.second; ++itr2)
        removeSpell(itr2->second.spell, disabled);

    // activate lesser rank in spellbook/action bar, and cast it if need
    bool prev_activate = false;

    if (uint32 prev_id = sSpellMgr.GetPrevSpellInChain(spell_id))
    {
        SpellEntry const* spellInfo = sSpellStore.LookupEntry(spell_id);

        // if talent then lesser rank also talent and need learn
        if (talentPos)
        {
            if (learn_low_rank)
                learnSpell(prev_id, false);
        }
        // if ranked non-stackable spell: need activate lesser rank and update dependence state
        else if (cur_active && sSpellMgr.IsRankedSpellNonStackableInSpellBook(spellInfo))
        {
            // need manually update dependence state (learn spell ignore like attempts)
            PlayerSpellMap::iterator prev_itr = m_spells.find(prev_id);
            if (prev_itr != m_spells.end())
            {
                if (prev_itr->second.dependent != cur_dependent)
                {
                    prev_itr->second.dependent = cur_dependent;
                    if (prev_itr->second.state != PLAYERSPELL_NEW)
                        prev_itr->second.state = PLAYERSPELL_CHANGED;
                }

                // now re-learn if need re-activate
                if (cur_active && !prev_itr->second.active && learn_low_rank)
                {
                    if (addSpell(prev_id, true, false, prev_itr->second.dependent, prev_itr->second.disabled))
                    {
                        // downgrade spell ranks in spellbook and action bar
                        WorldPacket data(SMSG_SUPERCEDED_SPELL, 4);
                        data << uint16(spell_id);
                        data << uint16(prev_id);
                        GetSession()->SendPacket(&data);
                        prev_activate = true;
                    }
                }
            }
        }
    }

    // remove from spell book if not replaced by lesser rank
    if (!prev_activate)
    {
        WorldPacket data(SMSG_REMOVED_SPELL, 4);
        data << uint16(spell_id);
        GetSession()->SendPacket(&data);
    }
}

void Player::RemoveSpellCooldown(uint32 spell_id, bool update /* = false */)
{
    m_spellCooldowns.erase(spell_id);

    if (update)
        SendClearCooldown(spell_id, this);
}

void Player::RemoveSpellCategoryCooldown(uint32 cat, bool update /* = false */)
{
    SpellCategoryStore::const_iterator ct = sSpellCategoryStore.find(cat);
    if (ct == sSpellCategoryStore.end())
        return;

    const SpellCategorySet& ct_set = ct->second;
    for (SpellCooldowns::const_iterator i = m_spellCooldowns.begin(); i != m_spellCooldowns.end();)
    {
        if (ct_set.find(i->first) != ct_set.end())
            RemoveSpellCooldown((i++)->first, update);
        else
            ++i;
    }
}

void Player::RemoveAllSpellCooldown()
{
    if (!m_spellCooldowns.empty())
    {
        for (SpellCooldowns::const_iterator itr = m_spellCooldowns.begin(); itr != m_spellCooldowns.end(); ++itr)
            SendClearCooldown(itr->first, this);

        m_spellCooldowns.clear();
    }
}

void Player::_LoadSpellCooldowns(QueryResult* result)
{
    // some cooldowns can be already set at aura loading...

    // QueryResult *result = CharacterDatabase.PQuery("SELECT spell,item,time FROM character_spell_cooldown WHERE guid = '%u'",GetGUIDLow());

    if (result)
    {
        time_t curTime = time(NULL);

        do
        {
            Field* fields = result->Fetch();

            uint32 spell_id = fields[0].GetUInt32();
            uint32 item_id  = fields[1].GetUInt32();
            time_t db_time  = (time_t)fields[2].GetUInt64();

            if (!sSpellStore.LookupEntry(spell_id))
            {
                sLog.outError("Player %u has unknown spell %u in `character_spell_cooldown`, skipping.", GetGUIDLow(), spell_id);
                continue;
            }

            // skip outdated cooldown
            if (db_time <= curTime)
                continue;

            AddSpellCooldown(spell_id, item_id, db_time);

            DEBUG_LOG("Player (GUID: %u) spell %u, item %u cooldown loaded (%u secs).", GetGUIDLow(), spell_id, item_id, uint32(db_time - curTime));
        }
        while (result->NextRow());

        delete result;
    }
}

void Player::_SaveSpellCooldowns()
{
    static SqlStatementID deleteSpellCooldown ;
    static SqlStatementID insertSpellCooldown ;

    SqlStatement stmt = CharacterDatabase.CreateStatement(deleteSpellCooldown, "DELETE FROM character_spell_cooldown WHERE guid = ?");
    stmt.PExecute(GetGUIDLow());

    time_t curTime = time(NULL);
    time_t infTime = curTime + infinityCooldownDelayCheck;

    // remove outdated and save active
    for (SpellCooldowns::iterator itr = m_spellCooldowns.begin(); itr != m_spellCooldowns.end();)
    {
        if (itr->second.end <= curTime)
            m_spellCooldowns.erase(itr++);
        else if (itr->second.end <= infTime)                // not save locked cooldowns, it will be reset or set at reload
        {
            stmt = CharacterDatabase.CreateStatement(insertSpellCooldown, "INSERT INTO character_spell_cooldown (guid,spell,item,time) VALUES( ?, ?, ?, ?)");
            stmt.PExecute(GetGUIDLow(), itr->first, itr->second.itemid, uint64(itr->second.end));
            ++itr;
        }
        else
            ++itr;
    }
}

uint32 Player::resetTalentsCost() const
{
    // The first time reset costs 1 gold
    if (m_resetTalentsCost < 1 * GOLD)
        return 1 * GOLD;
    // then 5 gold
    else if (m_resetTalentsCost < 5 * GOLD)
        return 5 * GOLD;
    // After that it increases in increments of 5 gold
    else if (m_resetTalentsCost < 10 * GOLD)
        return 10 * GOLD;
    else
    {
        time_t months = (sWorld.GetGameTime() - m_resetTalentsTime) / MONTH;
        if (months > 0)
        {
            // This cost will be reduced by a rate of 5 gold per month
            int32 new_cost = int32((m_resetTalentsCost) - 5 * GOLD * months);
            // to a minimum of 10 gold.
            return uint32(new_cost < 10 * GOLD ? 10 * GOLD : new_cost);
        }
        else
        {
            // After that it increases in increments of 5 gold
            int32 new_cost = m_resetTalentsCost + 5 * GOLD;
            // until it hits a cap of 50 gold.
            if (new_cost > 50 * GOLD)
                new_cost = 50 * GOLD;
            return new_cost;
        }
    }
}

bool Player::resetTalents(bool no_cost)
{
    // used by eluna
    sHookMgr->OnTalentsReset(this, no_cost);

    // not need after this call
    if (HasAtLoginFlag(AT_LOGIN_RESET_TALENTS))
        RemoveAtLoginFlag(AT_LOGIN_RESET_TALENTS, true);

    if (m_usedTalentCount == 0)
    {
        UpdateFreeTalentPoints(false);                      // for fix if need counter
        return false;
    }

    uint32 cost = 0;

    if (!no_cost)
    {
        cost = resetTalentsCost();

        if (GetMoney() < cost)
        {
            SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, 0, 0, 0);
            return false;
        }
    }

    for (unsigned int i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);

        if (!talentInfo)
            continue;

        TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);

        if (!talentTabInfo)
            continue;

        // unlearn only talents for character class
        // some spell learned by one class as normal spells or know at creation but another class learn it as talent,
        // to prevent unexpected lost normal learned spell skip another class talents
        if ((getClassMask() & talentTabInfo->ClassMask) == 0)
            continue;

        for (int j = 0; j < MAX_TALENT_RANK; ++j)
            if (talentInfo->RankID[j])
                removeSpell(talentInfo->RankID[j], !IsPassiveSpell(talentInfo->RankID[j]), false);
    }

    UpdateFreeTalentPoints(false);

    if (!no_cost)
    {
        ModifyMoney(-(int32)cost);

        m_resetTalentsCost = cost;
        m_resetTalentsTime = time(NULL);
    }

    // FIXME: remove pet before or after unlearn spells? for now after unlearn to allow removing of talent related, pet affecting auras
    RemovePet(PET_SAVE_REAGENTS);
    return true;
}

Mail* Player::GetMail(uint32 id)
{
    for (PlayerMails::iterator itr = m_mail.begin(); itr != m_mail.end(); ++itr)
    {
        if ((*itr)->messageID == id)
        {
            return (*itr);
        }
    }
    return NULL;
}

void Player::_SetCreateBits(UpdateMask* updateMask, Player* target) const
{
    if (target == this)
    {
        Object::_SetCreateBits(updateMask, target);
    }
    else
    {
        for (uint16 index = 0; index < m_valuesCount; ++index)
        {
            if (GetUInt32Value(index) != 0 && updateVisualBits.GetBit(index))
                updateMask->SetBit(index);
        }
    }
}

void Player::_SetUpdateBits(UpdateMask* updateMask, Player* target) const
{
    if (target == this)
    {
        Object::_SetUpdateBits(updateMask, target);
    }
    else
    {
        Object::_SetUpdateBits(updateMask, target);
        *updateMask &= updateVisualBits;
    }
}

void Player::InitVisibleBits()
{
    updateVisualBits.SetCount(PLAYER_END);

    // TODO: really implement OWNER_ONLY and GROUP_ONLY. Flags can be found in UpdateFields.h

    updateVisualBits.SetBit(OBJECT_FIELD_GUID);
    updateVisualBits.SetBit(OBJECT_FIELD_TYPE);
    updateVisualBits.SetBit(OBJECT_FIELD_SCALE_X);
    updateVisualBits.SetBit(UNIT_FIELD_CHARM + 0);
    updateVisualBits.SetBit(UNIT_FIELD_CHARM + 1);
    updateVisualBits.SetBit(UNIT_FIELD_SUMMON + 0);
    updateVisualBits.SetBit(UNIT_FIELD_SUMMON + 1);
    updateVisualBits.SetBit(UNIT_FIELD_CHARMEDBY + 0);
    updateVisualBits.SetBit(UNIT_FIELD_CHARMEDBY + 1);
    updateVisualBits.SetBit(UNIT_FIELD_TARGET + 0);
    updateVisualBits.SetBit(UNIT_FIELD_TARGET + 1);
    updateVisualBits.SetBit(UNIT_FIELD_CHANNEL_OBJECT + 0);
    updateVisualBits.SetBit(UNIT_FIELD_CHANNEL_OBJECT + 1);
    updateVisualBits.SetBit(UNIT_FIELD_HEALTH);
    updateVisualBits.SetBit(UNIT_FIELD_POWER1);
    updateVisualBits.SetBit(UNIT_FIELD_POWER2);
    updateVisualBits.SetBit(UNIT_FIELD_POWER3);
    updateVisualBits.SetBit(UNIT_FIELD_POWER4);
    updateVisualBits.SetBit(UNIT_FIELD_POWER5);
    updateVisualBits.SetBit(UNIT_FIELD_MAXHEALTH);
    updateVisualBits.SetBit(UNIT_FIELD_MAXPOWER1);
    updateVisualBits.SetBit(UNIT_FIELD_MAXPOWER2);
    updateVisualBits.SetBit(UNIT_FIELD_MAXPOWER3);
    updateVisualBits.SetBit(UNIT_FIELD_MAXPOWER4);
    updateVisualBits.SetBit(UNIT_FIELD_MAXPOWER5);
    updateVisualBits.SetBit(UNIT_FIELD_LEVEL);
    updateVisualBits.SetBit(UNIT_FIELD_FACTIONTEMPLATE);
    updateVisualBits.SetBit(UNIT_FIELD_BYTES_0);
    updateVisualBits.SetBit(UNIT_FIELD_FLAGS);
    //[-ZERO] updateVisualBits.SetBit(UNIT_FIELD_FLAGS_2);
    for (uint16 i = UNIT_FIELD_AURA; i < UNIT_FIELD_AURASTATE; ++i)
        updateVisualBits.SetBit(i);
    updateVisualBits.SetBit(UNIT_FIELD_AURASTATE);
    updateVisualBits.SetBit(UNIT_FIELD_BASEATTACKTIME + 0);
    updateVisualBits.SetBit(UNIT_FIELD_BASEATTACKTIME + 1);
    updateVisualBits.SetBit(UNIT_FIELD_BOUNDINGRADIUS);
    updateVisualBits.SetBit(UNIT_FIELD_COMBATREACH);
    updateVisualBits.SetBit(UNIT_FIELD_DISPLAYID);
    updateVisualBits.SetBit(UNIT_FIELD_NATIVEDISPLAYID);
    updateVisualBits.SetBit(UNIT_FIELD_MOUNTDISPLAYID);
    updateVisualBits.SetBit(UNIT_FIELD_BYTES_1);
    updateVisualBits.SetBit(UNIT_FIELD_PETNUMBER);
    updateVisualBits.SetBit(UNIT_FIELD_PET_NAME_TIMESTAMP);
    updateVisualBits.SetBit(UNIT_DYNAMIC_FLAGS);
    updateVisualBits.SetBit(UNIT_CHANNEL_SPELL);
    updateVisualBits.SetBit(UNIT_MOD_CAST_SPEED);
    updateVisualBits.SetBit(UNIT_FIELD_BYTES_2);

    updateVisualBits.SetBit(PLAYER_DUEL_ARBITER + 0);
    updateVisualBits.SetBit(PLAYER_DUEL_ARBITER + 1);
    updateVisualBits.SetBit(PLAYER_FLAGS);
    updateVisualBits.SetBit(PLAYER_GUILDID);
    updateVisualBits.SetBit(PLAYER_GUILDRANK);
    updateVisualBits.SetBit(PLAYER_BYTES);
    updateVisualBits.SetBit(PLAYER_BYTES_2);
    updateVisualBits.SetBit(PLAYER_BYTES_3);
    updateVisualBits.SetBit(PLAYER_DUEL_TEAM);
    updateVisualBits.SetBit(PLAYER_GUILD_TIMESTAMP);

    // PLAYER_QUEST_LOG_x also visible bit on official (but only on party/raid)...
    for (uint16 i = PLAYER_QUEST_LOG_1_1; i < PLAYER_QUEST_LOG_LAST_3; i += MAX_QUEST_OFFSET)
        updateVisualBits.SetBit(i);

    // Players visible items are not inventory stuff
    // 431) = 884 (0x374) = main weapon
    for (uint16 i = 0; i < EQUIPMENT_SLOT_END; i++)
    {
        // item creator
        updateVisualBits.SetBit(PLAYER_VISIBLE_ITEM_1_CREATOR + (i * MAX_VISIBLE_ITEM_OFFSET) + 0);
        updateVisualBits.SetBit(PLAYER_VISIBLE_ITEM_1_CREATOR + (i * MAX_VISIBLE_ITEM_OFFSET) + 1);

        uint16 visual_base = PLAYER_VISIBLE_ITEM_1_0 + (i * MAX_VISIBLE_ITEM_OFFSET);

        // item entry
        updateVisualBits.SetBit(visual_base + 0);

        // item enchantment IDs
        for (uint8 j = 0; j < MAX_INSPECTED_ENCHANTMENT_SLOT; ++j)
            updateVisualBits.SetBit(visual_base + 1 + j);

        // random properties
        updateVisualBits.SetBit(PLAYER_VISIBLE_ITEM_1_PROPERTIES + 0 + (i * MAX_VISIBLE_ITEM_OFFSET));
        updateVisualBits.SetBit(PLAYER_VISIBLE_ITEM_1_PROPERTIES + 1 + (i * MAX_VISIBLE_ITEM_OFFSET));
    }
}

void Player::BuildCreateUpdateBlockForPlayer(UpdateData* data, Player* target) const
{
    if (target == this)
    {
        for (int i = 0; i < EQUIPMENT_SLOT_END; ++i)
        {
            if (m_items[i] == NULL)
                continue;

            m_items[i]->BuildCreateUpdateBlockForPlayer(data, target);
        }
        for (int i = INVENTORY_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        {
            if (m_items[i] == NULL)
                continue;

            m_items[i]->BuildCreateUpdateBlockForPlayer(data, target);
        }
        for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
        {
            if (m_items[i] == NULL)
                continue;

            m_items[i]->BuildCreateUpdateBlockForPlayer(data, target);
        }
    }

    Unit::BuildCreateUpdateBlockForPlayer(data, target);
}

void Player::DestroyForPlayer(Player* target) const
{
    Unit::DestroyForPlayer(target);

    for (int i = 0; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (m_items[i] == NULL)
            continue;

        m_items[i]->DestroyForPlayer(target);
    }

    if (target == this)
    {
        for (int i = INVENTORY_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        {
            if (m_items[i] == NULL)
                continue;

            m_items[i]->DestroyForPlayer(target);
        }
        for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
        {
            if (m_items[i] == NULL)
                continue;

            m_items[i]->DestroyForPlayer(target);
        }
    }
}

bool Player::HasSpell(uint32 spell) const
{
    PlayerSpellMap::const_iterator itr = m_spells.find(spell);
    return (itr != m_spells.end() && itr->second.state != PLAYERSPELL_REMOVED &&
            !itr->second.disabled);
}

bool Player::HasActiveSpell(uint32 spell) const
{
    PlayerSpellMap::const_iterator itr = m_spells.find(spell);
    return (itr != m_spells.end() && itr->second.state != PLAYERSPELL_REMOVED &&
            itr->second.active && !itr->second.disabled);
}

TrainerSpellState Player::GetTrainerSpellState(TrainerSpell const* trainer_spell, uint32 reqLevel) const
{
    if (!trainer_spell)
        return TRAINER_SPELL_RED;

    if (!trainer_spell->spell)
        return TRAINER_SPELL_RED;

    // exist, already checked at loading
    SpellEntry const* spell = sSpellStore.LookupEntry(trainer_spell->spell);
    SpellEntry const* TriggerSpell = sSpellStore.LookupEntry(spell->EffectTriggerSpell[0]);


    // known spell
    if (HasSpell(TriggerSpell->Id))
        return TRAINER_SPELL_GRAY;

    // check race/class requirement
    if (!IsSpellFitByClassAndRace(TriggerSpell->Id))
        return TRAINER_SPELL_RED;

    bool prof = SpellMgr::IsProfessionSpell(trainer_spell->spell);

    // check level requirement
    uint32 spellLevel = reqLevel ? reqLevel : TriggerSpell->spellLevel;
    if (getLevel() < spellLevel)
        return TRAINER_SPELL_RED;

    if (SpellChainNode const* spell_chain = sSpellMgr.GetSpellChainNode(TriggerSpell->Id))
    {
        // check prev.rank requirement
        if (spell_chain->prev && !HasSpell(spell_chain->prev))
            return TRAINER_SPELL_RED;

        // check additional spell requirement
        if (spell_chain->req && !HasSpell(spell_chain->req))
            return TRAINER_SPELL_RED;
    }

    // check skill requirement
    if (!prof || GetSession()->GetSecurity() < AccountTypes(sWorld.getConfig(CONFIG_UINT32_TRADE_SKILL_GMIGNORE_SKILL)))
        if (trainer_spell->reqSkill && GetBaseSkillValue(trainer_spell->reqSkill) < trainer_spell->reqSkillValue)
            return TRAINER_SPELL_RED;

    // exist, already checked at loading
    // SpellEntry const* spell = sSpellStore.LookupEntry(trainer_spell->spell);

    // secondary prof. or not prof. spell
    uint32 skill = spell->EffectMiscValue[1];

    if (spell->Effect[1] != SPELL_EFFECT_SKILL || !IsPrimaryProfessionSkill(skill))
        return TRAINER_SPELL_GREEN;

    // check primary prof. limit
    if (sSpellMgr.IsPrimaryProfessionFirstRankSpell(spell->Id) && GetFreePrimaryProfessionPoints() == 0)
        return TRAINER_SPELL_GREEN_DISABLED;

    return TRAINER_SPELL_GREEN;
}

/**
 * Deletes a character from the database
 *
 * The way, how the characters will be deleted is decided based on the config option.
 *
 * @see Player::DeleteOldCharacters
 *
 * @param playerguid       the low-GUID from the player which should be deleted
 * @param accountId        the account id from the player
 * @param updateRealmChars when this flag is set, the amount of characters on that realm will be updated in the realmlist
 * @param deleteFinally    if this flag is set, the config option will be ignored and the character will be permanently removed from the database
 */
void Player::DeleteFromDB(ObjectGuid playerguid, uint32 accountId, bool updateRealmChars, bool deleteFinally)
{
    // for nonexistent account avoid update realm
    if (accountId == 0)
        updateRealmChars = false;

    uint32 charDelete_method = sWorld.getConfig(CONFIG_UINT32_CHARDELETE_METHOD);
    uint32 charDelete_minLvl = sWorld.getConfig(CONFIG_UINT32_CHARDELETE_MIN_LEVEL);

    // if we want to finally delete the character or the character does not meet the level requirement, we set it to mode 0
    if (deleteFinally || Player::GetLevelFromDB(playerguid) < charDelete_minLvl)
        charDelete_method = 0;

    uint32 lowguid = playerguid.GetCounter();

    // convert corpse to bones if exist (to prevent exiting Corpse in World without DB entry)
    // bones will be deleted by corpse/bones deleting thread shortly
    sObjectAccessor.ConvertCorpseForPlayer(playerguid);

    // remove from guild
    if (uint32 guildId = GetGuildIdFromDB(playerguid))
    {
        if (Guild* guild = sGuildMgr.GetGuildById(guildId))
        {
            if (guild->DelMember(playerguid))
            {
                guild->Disband();
                delete guild;
            }
        }
    }

    // the player was uninvited already on logout so just remove from group
    QueryResult* resultGroup = CharacterDatabase.PQuery("SELECT groupId FROM group_member WHERE memberGuid='%u'", lowguid);
    if (resultGroup)
    {
        uint32 groupId = (*resultGroup)[0].GetUInt32();
        delete resultGroup;
        if (Group* group = sObjectMgr.GetGroupById(groupId))
            RemoveFromGroup(group, playerguid);
    }

    // remove signs from petitions (also remove petitions if owner);
    RemovePetitionsAndSigns(playerguid);

    switch (charDelete_method)
    {
            // completely remove from the database
        case 0:
        {
            // return back all mails with COD and Item                 0  1           2              3      4       5          6     7
            QueryResult* resultMail = CharacterDatabase.PQuery("SELECT id,messageType,mailTemplateId,sender,subject,itemTextId,money,has_items FROM mail WHERE receiver='%u' AND has_items<>0 AND cod<>0", lowguid);
            if (resultMail)
            {
                do
                {
                    Field* fields = resultMail->Fetch();

                    uint32 mail_id       = fields[0].GetUInt32();
                    uint16 mailType      = fields[1].GetUInt16();
                    uint16 mailTemplateId = fields[2].GetUInt16();
                    uint32 sender        = fields[3].GetUInt32();
                    std::string subject  = fields[4].GetCppString();
                    uint32 itemTextId    = fields[5].GetUInt32();
                    uint32 money         = fields[6].GetUInt32();
                    bool has_items       = fields[7].GetBool();

                    // we can return mail now
                    // so firstly delete the old one
                    CharacterDatabase.PExecute("DELETE FROM mail WHERE id = '%u'", mail_id);

                    // mail not from player
                    if (mailType != MAIL_NORMAL)
                    {
                        if (has_items)
                            CharacterDatabase.PExecute("DELETE FROM mail_items WHERE mail_id = '%u'", mail_id);
                        continue;
                    }

                    MailDraft draft;
                    if (mailTemplateId)
                        draft.SetMailTemplate(mailTemplateId, false);// items already included
                    else
                        draft.SetSubjectAndBodyId(subject, itemTextId);

                    if (has_items)
                    {
                        // data needs to be at first place for Item::LoadFromDB
                        //                                                          0    1         2
                        QueryResult* resultItems = CharacterDatabase.PQuery("SELECT data,item_guid,item_template FROM mail_items JOIN item_instance ON item_guid = guid WHERE mail_id='%u'", mail_id);
                        if (resultItems)
                        {
                            do
                            {
                                Field* fields2 = resultItems->Fetch();

                                uint32 item_guidlow = fields2[1].GetUInt32();
                                uint32 item_template = fields2[2].GetUInt32();

                                ItemPrototype const* itemProto = ObjectMgr::GetItemPrototype(item_template);
                                if (!itemProto)
                                {
                                    CharacterDatabase.PExecute("DELETE FROM item_instance WHERE guid = '%u'", item_guidlow);
                                    continue;
                                }

                                Item* pItem = NewItemOrBag(itemProto);
                                if (!pItem->LoadFromDB(item_guidlow, fields2, playerguid))
                                {
                                    pItem->FSetState(ITEM_REMOVED);
                                    pItem->SaveToDB();      // it also deletes item object !
                                    continue;
                                }

                                draft.AddItem(pItem);
                            }
                            while (resultItems->NextRow());

                            delete resultItems;
                        }
                    }

                    CharacterDatabase.PExecute("DELETE FROM mail_items WHERE mail_id = '%u'", mail_id);

                    uint32 pl_account = sObjectMgr.GetPlayerAccountIdByGUID(playerguid);

                    draft.SetMoney(money).SendReturnToSender(pl_account, playerguid, ObjectGuid(HIGHGUID_PLAYER, sender));
                }
                while (resultMail->NextRow());

                delete resultMail;
            }

            // unsummon and delete for pets in world is not required: player deleted from CLI or character list with not loaded pet.
            // Get guids of character's pets, will deleted in transaction
            QueryResult* resultPets = CharacterDatabase.PQuery("SELECT id FROM character_pet WHERE owner = '%u'", lowguid);

            // delete char from friends list when selected chars is online (non existing - error)
            QueryResult* resultFriend = CharacterDatabase.PQuery("SELECT DISTINCT guid FROM character_social WHERE friend = '%u'", lowguid);

            // NOW we can finally clear other DB data related to character
            CharacterDatabase.BeginTransaction();
            if (resultPets)
            {
                do
                {
                    Field* fields3 = resultPets->Fetch();
                    uint32 petguidlow = fields3[0].GetUInt32();
                    // do not create separate transaction for pet delete otherwise we will get fatal error!
                    Pet::DeleteFromDB(petguidlow, false);
                }
                while (resultPets->NextRow());
                delete resultPets;
            }

            // cleanup friends for online players, offline case will cleanup later in code
            if (resultFriend)
            {
                do
                {
                    Field* fieldsFriend = resultFriend->Fetch();
                    if (Player* sFriend = sObjectAccessor.FindPlayer(ObjectGuid(HIGHGUID_PLAYER, fieldsFriend[0].GetUInt32())))
                    {
                        if (sFriend->IsInWorld())
                        {
                            sFriend->GetSocial()->RemoveFromSocialList(playerguid, false);
                            sSocialMgr.SendFriendStatus(sFriend, FRIEND_REMOVED, playerguid, false);
                        }
                    }
                }
                while (resultFriend->NextRow());
                delete resultFriend;
            }

            CharacterDatabase.PExecute("DELETE FROM characters WHERE guid = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM character_action WHERE guid = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM character_aura WHERE guid = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM character_battleground_data WHERE guid = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM character_gifts WHERE guid = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM character_homebind WHERE guid = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM character_instance WHERE guid = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM group_instance WHERE leaderGuid = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM character_inventory WHERE guid = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM character_queststatus WHERE guid = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM character_reputation WHERE guid = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM character_skills WHERE guid = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM character_spell WHERE guid = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM character_spell_cooldown WHERE guid = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM character_ticket WHERE guid = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM item_instance WHERE owner_guid = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM character_social WHERE guid = '%u' OR friend='%u'", lowguid, lowguid);
            CharacterDatabase.PExecute("DELETE FROM mail WHERE receiver = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM mail_items WHERE receiver = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM character_pet WHERE owner = '%u'", lowguid);
            CharacterDatabase.PExecute("DELETE FROM guild_eventlog WHERE PlayerGuid1 = '%u' OR PlayerGuid2 = '%u'", lowguid, lowguid);
            CharacterDatabase.CommitTransaction();
            break;
        }
        // The character gets unlinked from the account, the name gets freed up and appears as deleted ingame
        case 1:
            CharacterDatabase.PExecute("UPDATE characters SET deleteInfos_Name=name, deleteInfos_Account=account, deleteDate='" UI64FMTD "', name='', account=0 WHERE guid=%u", uint64(time(NULL)), lowguid);
            break;
        default:
            sLog.outError("Player::DeleteFromDB: Unsupported delete method: %u.", charDelete_method);
    }

    if (updateRealmChars)
        sWorld.UpdateRealmCharCount(accountId);
}

/**
 * Characters which were kept back in the database after being deleted and are now too old (see config option "CharDelete.KeepDays"), will be completely deleted.
 *
 * @see Player::DeleteFromDB
 */
void Player::DeleteOldCharacters()
{
    uint32 keepDays = sWorld.getConfig(CONFIG_UINT32_CHARDELETE_KEEP_DAYS);
    if (!keepDays)
        return;

    Player::DeleteOldCharacters(keepDays);
}

/**
 * Characters which were kept back in the database after being deleted and are older than the specified amount of days, will be completely deleted.
 *
 * @see Player::DeleteFromDB
 *
 * @param keepDays overrite the config option by another amount of days
 */
void Player::DeleteOldCharacters(uint32 keepDays)
{
    sLog.outString("Player::DeleteOldChars: Deleting all characters which have been deleted %u days before...", keepDays);

    QueryResult* resultChars = CharacterDatabase.PQuery("SELECT guid, deleteInfos_Account FROM characters WHERE deleteDate IS NOT NULL AND deleteDate < '" UI64FMTD "'", uint64(time(NULL) - time_t(keepDays * DAY)));
    if (resultChars)
    {
        sLog.outString("Player::DeleteOldChars: Found %u character(s) to delete", uint32(resultChars->GetRowCount()));
        do
        {
            Field* charFields = resultChars->Fetch();
            ObjectGuid guid = ObjectGuid(HIGHGUID_PLAYER, charFields[0].GetUInt32());
            Player::DeleteFromDB(guid, charFields[1].GetUInt32(), true, true);
        }
        while (resultChars->NextRow());
        delete resultChars;
    }
}

void Player::SetRoot(bool enable)
{
    WorldPacket data(enable ? SMSG_FORCE_MOVE_ROOT : SMSG_FORCE_MOVE_UNROOT, GetPackGUID().size() + 4);
    data << GetPackGUID();
    data << uint32(0);
    GetSession()->SendPacket(&data);
}

void Player::SetWaterWalk(bool enable)
{
    WorldPacket data(enable ? SMSG_MOVE_WATER_WALK : SMSG_MOVE_LAND_WALK, GetPackGUID().size() + 4);
    data << GetPackGUID();
    data << uint32(0);
    GetSession()->SendPacket(&data);
}

/* Preconditions:
  - a resurrectable corpse must not be loaded for the player (only bones)
  - the player must be in world
*/
void Player::BuildPlayerRepop()
{
    if (getRace() == RACE_NIGHTELF)
        CastSpell(this, 20584, true);                       // auras SPELL_AURA_INCREASE_SPEED(+speed in wisp form), SPELL_AURA_INCREASE_SWIM_SPEED(+swim speed in wisp form), SPELL_AURA_TRANSFORM (to wisp form)
    CastSpell(this, 8326, true);                            // auras SPELL_AURA_GHOST, SPELL_AURA_INCREASE_SPEED(why?), SPELL_AURA_INCREASE_SWIM_SPEED(why?)

    // the player cannot have a corpse already, only bones which are not returned by GetCorpse
    if (GetCorpse())
    {
        sLog.outError("BuildPlayerRepop: player %s(%d) already has a corpse", GetName(), GetGUIDLow());
        MANGOS_ASSERT(false);
    }

    // create a corpse and place it at the player's location
    Corpse* corpse = CreateCorpse();
    if (!corpse)
    {
        sLog.outError("Error creating corpse for Player %s [%u]", GetName(), GetGUIDLow());
        return;
    }
    GetMap()->Add(corpse);

    // convert player body to ghost
    SetHealth(1);

    SetWaterWalk(true);
    if (!GetSession()->isLogingOut())
        SetRoot(false);

    // BG - remove insignia related
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);

    SendCorpseReclaimDelay();

    // to prevent cheating
    corpse->ResetGhostTime();

    StopMirrorTimers();                                     // disable timers(bars)

    // set and clear other
    SetByteValue(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND);
}

void Player::ResurrectPlayer(float restore_percent, bool applySickness)
{
    // remove death flag + set aura
    SetByteValue(UNIT_FIELD_BYTES_1, 3, 0x00);

    SetDeathState(ALIVE);

    if (getRace() == RACE_NIGHTELF)
        RemoveAurasDueToSpell(20584);                       // speed bonuses
    RemoveAurasDueToSpell(8326);                            // SPELL_AURA_GHOST

    SetWaterWalk(false);
    SetRoot(false);

    // set health/powers (0- will be set in caller)
    if (restore_percent > 0.0f)
    {
        SetHealth(uint32(GetMaxHealth()*restore_percent));
        SetPower(POWER_MANA, uint32(GetMaxPower(POWER_MANA)*restore_percent));
        SetPower(POWER_RAGE, 0);
        SetPower(POWER_ENERGY, uint32(GetMaxPower(POWER_ENERGY)*restore_percent));
    }

    // trigger update zone for alive state zone updates
    uint32 newzone, newarea;
    GetZoneAndAreaId(newzone, newarea);
    UpdateZone(newzone, newarea);

    m_deathTimer = 0;

    // update visibility of world around viewpoint
    m_camera.UpdateVisibilityForOwner();
    // update visibility of player for nearby cameras
    UpdateObjectVisibility();

    sHookMgr->OnResurrect(this);

    if (!applySickness)
        return;

    // Characters from level 1-10 are not affected by resurrection sickness.
    // Characters from level 11-19 will suffer from one minute of sickness
    // for each level they are above 10.
    // Characters level 20 and up suffer from ten minutes of sickness.
    int32 startLevel = sWorld.getConfig(CONFIG_INT32_DEATH_SICKNESS_LEVEL);

    if (int32(getLevel()) >= startLevel)
    {
        // set resurrection sickness
        CastSpell(this, SPELL_ID_PASSIVE_RESURRECTION_SICKNESS, true);

        // not full duration
        if (int32(getLevel()) < startLevel + 9)
        {
            int32 delta = (int32(getLevel()) - startLevel + 1) * MINUTE;

            if (SpellAuraHolder* holder = GetSpellAuraHolder(SPELL_ID_PASSIVE_RESURRECTION_SICKNESS))
            {
                holder->SetAuraDuration(delta * IN_MILLISECONDS);
                holder->UpdateAuraDuration();
            }
        }
    }
}

void Player::KillPlayer()
{
    SetRoot(true);

    StopMirrorTimers();                                     // disable timers(bars)

    SetDeathState(CORPSE);
    // SetFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_IN_PVP );

    SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_NONE);
    ApplyModByteFlag(PLAYER_FIELD_BYTES, 0, PLAYER_FIELD_BYTE_RELEASE_TIMER, !sMapStore.LookupEntry(GetMapId())->Instanceable());

    // 6 minutes until repop at graveyard
    m_deathTimer = 6 * MINUTE * IN_MILLISECONDS;

    UpdateCorpseReclaimDelay();                             // dependent at use SetDeathPvP() call before kill

    // don't create corpse at this moment, player might be falling

    // update visibility
    UpdateObjectVisibility();
}

Corpse* Player::CreateCorpse()
{
    // prevent existence 2 corpse for player
    SpawnCorpseBones();

    Corpse* corpse = new Corpse((m_ExtraFlags & PLAYER_EXTRA_PVP_DEATH) ? CORPSE_RESURRECTABLE_PVP : CORPSE_RESURRECTABLE_PVE);
    SetPvPDeath(false);

    if (!corpse->Create(sObjectMgr.GenerateCorpseLowGuid(), this))
    {
        delete corpse;
        return NULL;
    }

    uint8 skin       = GetByteValue(PLAYER_BYTES, 0);
    uint8 face       = GetByteValue(PLAYER_BYTES, 1);
    uint8 hairstyle  = GetByteValue(PLAYER_BYTES, 2);
    uint8 haircolor  = GetByteValue(PLAYER_BYTES, 3);
    uint8 facialhair = GetByteValue(PLAYER_BYTES_2, 0);

    corpse->SetByteValue(CORPSE_FIELD_BYTES_1, 1, getRace());
    corpse->SetByteValue(CORPSE_FIELD_BYTES_1, 2, getGender());
    corpse->SetByteValue(CORPSE_FIELD_BYTES_1, 3, skin);

    corpse->SetByteValue(CORPSE_FIELD_BYTES_2, 0, face);
    corpse->SetByteValue(CORPSE_FIELD_BYTES_2, 1, hairstyle);
    corpse->SetByteValue(CORPSE_FIELD_BYTES_2, 2, haircolor);
    corpse->SetByteValue(CORPSE_FIELD_BYTES_2, 3, facialhair);

    uint32 flags = CORPSE_FLAG_UNK2;
    if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM))
        flags |= CORPSE_FLAG_HIDE_HELM;
    if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK))
        flags |= CORPSE_FLAG_HIDE_CLOAK;
    if (InBattleGround())
        flags |= CORPSE_FLAG_LOOTABLE;                      // to be able to remove insignia
    corpse->SetUInt32Value(CORPSE_FIELD_FLAGS, flags);

    corpse->SetUInt32Value(CORPSE_FIELD_DISPLAY_ID, GetNativeDisplayId());

    corpse->SetUInt32Value(CORPSE_FIELD_GUILD, GetGuildId());

    uint32 iDisplayID;
    uint32 iIventoryType;
    uint32 _cfi;
    for (int i = 0; i < EQUIPMENT_SLOT_END; ++i)
    {
        if (m_items[i])
        {
            iDisplayID = m_items[i]->GetProto()->DisplayInfoID;
            iIventoryType = m_items[i]->GetProto()->InventoryType;

            _cfi =  iDisplayID | (iIventoryType << 24);
            corpse->SetUInt32Value(CORPSE_FIELD_ITEM + i, _cfi);
        }
    }

    // we not need saved corpses for BG/arenas
    if (!GetMap()->IsBattleGround())
        corpse->SaveToDB();

    // register for player, but not show
    sObjectAccessor.AddCorpse(corpse);
    return corpse;
}

void Player::SpawnCorpseBones()
{
    if (sObjectAccessor.ConvertCorpseForPlayer(GetObjectGuid()))
        if (!GetSession()->PlayerLogoutWithSave())          // at logout we will already store the player
            SaveToDB();                                     // prevent loading as ghost without corpse
}

Corpse* Player::GetCorpse() const
{
    return sObjectAccessor.GetCorpseForPlayerGUID(GetObjectGuid());
}

void Player::DurabilityLossAll(double percent, bool inventory)
{
    for (int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            DurabilityLoss(pItem, percent);

    if (inventory)
    {
        // bags not have durability
        // for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)

        for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
            if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                DurabilityLoss(pItem, percent);

        // keys not have durability
        // for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)

        for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
            if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                    if (Item* pItem = GetItemByPos(i, j))
                        DurabilityLoss(pItem, percent);
    }
}

void Player::DurabilityLoss(Item* item, double percent)
{
    if (!item)
        return;

    uint32 pMaxDurability =  item ->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);

    if (!pMaxDurability)
        return;

    uint32 pDurabilityLoss = uint32(pMaxDurability * percent);

    if (pDurabilityLoss < 1)
        pDurabilityLoss = 1;

    DurabilityPointsLoss(item, pDurabilityLoss);
}

void Player::DurabilityPointsLossAll(int32 points, bool inventory)
{
    for (int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            DurabilityPointsLoss(pItem, points);

    if (inventory)
    {
        // bags not have durability
        // for(int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)

        for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
            if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                DurabilityPointsLoss(pItem, points);

        // keys not have durability
        // for(int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)

        for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
            if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                    if (Item* pItem = GetItemByPos(i, j))
                        DurabilityPointsLoss(pItem, points);
    }
}

void Player::DurabilityPointsLoss(Item* item, int32 points)
{
    int32 pMaxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
    int32 pOldDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);
    int32 pNewDurability = pOldDurability - points;

    if (pNewDurability < 0)
        pNewDurability = 0;
    else if (pNewDurability > pMaxDurability)
        pNewDurability = pMaxDurability;

    if (pOldDurability != pNewDurability)
    {
        // modify item stats _before_ Durability set to 0 to pass _ApplyItemMods internal check
        if (pNewDurability == 0 && pOldDurability > 0 && item->IsEquipped())
            _ApplyItemMods(item, item->GetSlot(), false);

        item->SetUInt32Value(ITEM_FIELD_DURABILITY, pNewDurability);

        // modify item stats _after_ restore durability to pass _ApplyItemMods internal check
        if (pNewDurability > 0 && pOldDurability == 0 && item->IsEquipped())
            _ApplyItemMods(item, item->GetSlot(), true);

        item->SetState(ITEM_CHANGED, this);
    }
}

void Player::DurabilityPointLossForEquipSlot(EquipmentSlots slot)
{
    if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        DurabilityPointsLoss(pItem, 1);
}

uint32 Player::DurabilityRepairAll(bool cost, float discountMod)
{
    uint32 TotalCost = 0;
    // equipped, backpack, bags itself
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        TotalCost += DurabilityRepair(((INVENTORY_SLOT_BAG_0 << 8) | i), cost, discountMod);

    // bank, buyback and keys not repaired

    // items in inventory bags
    for (int j = INVENTORY_SLOT_BAG_START; j < INVENTORY_SLOT_BAG_END; ++j)
        for (int i = 0; i < MAX_BAG_SIZE; ++i)
            TotalCost += DurabilityRepair(((j << 8) | i), cost, discountMod);
    return TotalCost;
}

uint32 Player::DurabilityRepair(uint16 pos, bool cost, float discountMod)
{
    Item* item = GetItemByPos(pos);

    uint32 TotalCost = 0;
    if (!item)
        return TotalCost;

    uint32 maxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
    if (!maxDurability)
        return TotalCost;

    uint32 curDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);

    if (cost)
    {
        uint32 LostDurability = maxDurability - curDurability;
        if (LostDurability > 0)
        {
            ItemPrototype const* ditemProto = item->GetProto();

            DurabilityCostsEntry const* dcost = sDurabilityCostsStore.LookupEntry(ditemProto->ItemLevel);
            if (!dcost)
            {
                sLog.outError("RepairDurability: Wrong item lvl %u", ditemProto->ItemLevel);
                return TotalCost;
            }

            uint32 dQualitymodEntryId = (ditemProto->Quality + 1) * 2;
            DurabilityQualityEntry const* dQualitymodEntry = sDurabilityQualityStore.LookupEntry(dQualitymodEntryId);
            if (!dQualitymodEntry)
            {
                sLog.outError("RepairDurability: Wrong dQualityModEntry %u", dQualitymodEntryId);
                return TotalCost;
            }

            uint32 dmultiplier = dcost->multiplier[ItemSubClassToDurabilityMultiplierId(ditemProto->Class, ditemProto->SubClass)];
            uint32 costs = uint32(LostDurability * dmultiplier * double(dQualitymodEntry->quality_mod));

            costs = uint32(costs * discountMod);

            if (costs == 0)                                 // fix for ITEM_QUALITY_ARTIFACT
                costs = 1;

            if (GetMoney() < costs)
            {
                DEBUG_LOG("You do not have enough money");
                return TotalCost;
            }
            else
                ModifyMoney(-int32(costs));
        }
    }

    item->SetUInt32Value(ITEM_FIELD_DURABILITY, maxDurability);
    item->SetState(ITEM_CHANGED, this);

    // reapply mods for total broken and repaired item if equipped
    if (IsEquipmentPos(pos) && !curDurability)
        _ApplyItemMods(item, pos & 255, true);
    return TotalCost;
}

void Player::RepopAtGraveyard()
{
    // note: this can be called also when the player is alive
    // for example from WorldSession::HandleMovementOpcodes

    AreaTableEntry const* zone = GetAreaEntryByAreaID(GetAreaId());

    WorldSafeLocsEntry const* ClosestGrave = NULL;

    // Special handle for battleground maps
    if (BattleGround* bg = GetBattleGround())
        ClosestGrave = bg->GetClosestGraveYard(this);
    else
        ClosestGrave = sObjectMgr.GetClosestGraveYard(GetPositionX(), GetPositionY(), GetPositionZ(), GetMapId(), GetTeam());

    // stop countdown until repop
    m_deathTimer = 0;

    // if no grave found, stay at the current location
    // and don't show spirit healer location
    if (ClosestGrave)
    {
        bool updateVisibility = IsInWorld() && GetMapId() == ClosestGrave->map_id;
        TeleportTo(ClosestGrave->map_id, ClosestGrave->x, ClosestGrave->y, ClosestGrave->z, GetOrientation());
        if (updateVisibility && IsInWorld())
            UpdateVisibilityAndView();
    }
}

void Player::JoinedChannel(Channel* c)
{
    m_channels.push_back(c);
}

void Player::LeftChannel(Channel* c)
{
    m_channels.remove(c);
}

void Player::CleanupChannels()
{
    while (!m_channels.empty())
    {
        Channel* ch = *m_channels.begin();
        m_channels.erase(m_channels.begin());               // remove from player's channel list
        ch->Leave(this, false);                             // not send to client, not remove from player's channel list
        if (ChannelMgr* cMgr = channelMgr(GetTeam()))
            cMgr->LeftChannel(ch->GetName());               // deleted channel if empty
    }
    DEBUG_LOG("Player: channels cleaned up!");
}

void Player::UpdateLocalChannels(uint32 newZone)
{
    if (m_channels.empty())
        return;

    AreaTableEntry const* current_zone = GetAreaEntryByAreaID(newZone);
    if (!current_zone)
        return;

    ChannelMgr* cMgr = channelMgr(GetTeam());
    if (!cMgr)
        return;

    std::string current_zone_name = current_zone->area_name[GetSession()->GetSessionDbcLocale()];

    for (JoinedChannelsList::iterator i = m_channels.begin(), next; i != m_channels.end(); i = next)
    {
        next = i; ++next;

        // skip non built-in channels
        if (!(*i)->IsConstant())
            continue;

        ChatChannelsEntry const* ch = GetChannelEntryFor((*i)->GetChannelId());
        if (!ch)
            continue;

        if ((ch->flags & 4) == 4)                           // global channel without zone name in pattern
            continue;

        //  new channel
        char new_channel_name_buf[100];
        snprintf(new_channel_name_buf, 100, ch->pattern[m_session->GetSessionDbcLocale()], current_zone_name.c_str());
        Channel* new_channel = cMgr->GetJoinChannel(new_channel_name_buf, ch->ChannelID);

        if ((*i) != new_channel)
        {
            new_channel->Join(this, "");                    // will output Changed Channel: N. Name

            // leave old channel
            (*i)->Leave(this, false);                       // not send leave channel, it already replaced at client
            std::string name = (*i)->GetName();             // store name, (*i)erase in LeftChannel
            LeftChannel(*i);                                // remove from player's channel list
            cMgr->LeftChannel(name);                        // delete if empty
        }
    }
    DEBUG_LOG("Player: channels cleaned up!");
}

void Player::LeaveLFGChannel()
{
    for (JoinedChannelsList::iterator i = m_channels.begin(); i != m_channels.end(); ++i)
    {
        if ((*i)->IsLFG())
        {
            (*i)->Leave(this);
            break;
        }
    }
}

void Player::UpdateDefense()
{
    uint32 defense_skill_gain = sWorld.getConfig(CONFIG_UINT32_SKILL_GAIN_DEFENSE);

    if (UpdateSkill(SKILL_DEFENSE, defense_skill_gain))
    {
        // update dependent from defense skill part
        UpdateDefenseBonusesMod();
    }
}

void Player::HandleBaseModValue(BaseModGroup modGroup, BaseModType modType, float amount, bool apply)
{
    if (modGroup >= BASEMOD_END || modType >= MOD_END)
    {
        sLog.outError("ERROR in HandleBaseModValue(): nonexistent BaseModGroup of wrong BaseModType!");
        return;
    }

    float val = 1.0f;

    switch (modType)
    {
        case FLAT_MOD:
            m_auraBaseMod[modGroup][modType] += apply ? amount : -amount;
            break;
        case PCT_MOD:
            if (amount <= -100.0f)
                amount = -200.0f;

            val = (100.0f + amount) / 100.0f;
            m_auraBaseMod[modGroup][modType] *= apply ? val : (1.0f / val);
            break;
    }

    if (!CanModifyStats())
        return;

    switch (modGroup)
    {
        case CRIT_PERCENTAGE:              UpdateCritPercentage(BASE_ATTACK);                          break;
        case RANGED_CRIT_PERCENTAGE:       UpdateCritPercentage(RANGED_ATTACK);                        break;
        default: break;
    }
}

float Player::GetBaseModValue(BaseModGroup modGroup, BaseModType modType) const
{
    if (modGroup >= BASEMOD_END || modType > MOD_END)
    {
        sLog.outError("trial to access nonexistent BaseModGroup or wrong BaseModType!");
        return 0.0f;
    }

    if (modType == PCT_MOD && m_auraBaseMod[modGroup][PCT_MOD] <= 0.0f)
        return 0.0f;

    return m_auraBaseMod[modGroup][modType];
}

float Player::GetTotalBaseModValue(BaseModGroup modGroup) const
{
    if (modGroup >= BASEMOD_END)
    {
        sLog.outError("wrong BaseModGroup in GetTotalBaseModValue()!");
        return 0.0f;
    }

    if (m_auraBaseMod[modGroup][PCT_MOD] <= 0.0f)
        return 0.0f;

    return m_auraBaseMod[modGroup][FLAT_MOD] * m_auraBaseMod[modGroup][PCT_MOD];
}

uint32 Player::GetShieldBlockValue() const
{
    float value = (m_auraBaseMod[SHIELD_BLOCK_VALUE][FLAT_MOD] + GetStat(STAT_STRENGTH) / 20 - 1) * m_auraBaseMod[SHIELD_BLOCK_VALUE][PCT_MOD];

    value = (value < 0) ? 0 : value;

    return uint32(value);
}

float Player::GetMeleeCritFromAgility()
{
    // from mangos 3462 for 1.12
    float val = 0.0f, classrate = 0.0f;
    // critical
    switch (getClass())
    {
        case CLASS_PALADIN: classrate = 19.77f; break;
        case CLASS_SHAMAN:  classrate = 19.7f;  break;
        case CLASS_MAGE:    classrate = 19.44f; break;
        case CLASS_ROGUE:   classrate = 29.0f;  break;
        case CLASS_HUNTER:  classrate = 53.0f;  break;      // in 2.0.x = 33
        case CLASS_PRIEST:
        case CLASS_WARLOCK:
        case CLASS_DRUID:
        case CLASS_WARRIOR:
        default:            classrate = 20.0f; break;
    }

    val = GetStat(STAT_AGILITY) / classrate;
    return val;
}

float Player::GetDodgeFromAgility()
{
    // from mangos 3462 for 1.12
    float val = 0, classrate = 0;

    // dodge
    if (getClass() == CLASS_HUNTER) classrate = 26.5;
    else if (getClass() == CLASS_ROGUE)  classrate = 14.5;
    else classrate = 20;
    ///*+(Defense*0,04);
    if (getRace() == RACE_NIGHTELF)
        val = GetStat(STAT_AGILITY) / classrate + 1;
    else
        val = GetStat(STAT_AGILITY) / classrate;

    return val;

    /* [-ZERO]
    // Table for base dodge values
    const float dodge_base[MAX_CLASSES] =
    {
         0.0075f,   // Warrior
         0.00652f,  // Paladin
        -0.0545f,   // Hunter
        -0.0059f,   // Rogue
         0.03183f,  // Priest
         0.0114f,   // DK
         0.0167f,   // Shaman
         0.034575f, // Mage
         0.02011f,  // Warlock
         0.0f,      // ??
        -0.0187f    // Druid
    };
    // Crit/agility to dodge/agility coefficient multipliers
    const float crit_to_dodge[MAX_CLASSES] =
    {
         1.1f,      // Warrior
         1.0f,      // Paladin
         1.6f,      // Hunter
         2.0f,      // Rogue
         1.0f,      // Priest
         1.0f,      // DK?
         1.0f,      // Shaman
         1.0f,      // Mage
         1.0f,      // Warlock
         0.0f,      // ??
         1.7f       // Druid
    };

    uint32 level = getLevel();
    uint32 pclass = getClass();

    if (level>GT_MAX_LEVEL) level = GT_MAX_LEVEL;

    // Dodge per agility for most classes equal crit per agility (but for some classes need apply some multiplier)
    GtChanceToMeleeCritEntry  const *dodgeRatio = sGtChanceToMeleeCritStore.LookupEntry((pclass-1)*GT_MAX_LEVEL + level-1);
    if (dodgeRatio==NULL || pclass > MAX_CLASSES)
        return 0.0f;

    float dodge=dodge_base[pclass-1] + GetStat(STAT_AGILITY) * dodgeRatio->ratio * crit_to_dodge[pclass-1];
    return dodge*100.0f; */
}

float Player::GetSpellCritFromIntellect()
{
// Chance to crit is computed from INT and LEVEL as follows:
    //   chance = base + INT / (rate0 + rate1 * LEVEL)
    // The formula keeps the crit chance at %5 on every level unless the player
    // increases his intelligence by other means (enchants, buffs, talents, ...)

    //[TZERO] from mangos 3462 for 1.12 MUST BE CHECKED
    float val = 0.0f;

    static const struct
    {
        float base;
        float rate0, rate1;
    }
    crit_data[MAX_CLASSES] =
    {
        {   0.0f,   0.0f,  10.0f  },                        //  0: unused
        {   0.0f,   0.0f,  10.0f  },                        //  1: warrior
        {   3.70f, 14.77f,  0.65f },                        //  2: paladin
        {   0.0f,   0.0f,  10.0f  },                        //  3: hunter
        {   0.0f,   0.0f,  10.0f  },                        //  4: rogue
        {   2.97f, 10.03f,  0.82f },                        //  5: priest
        {   0.0f,   0.0f,  10.0f  },                        //  6: unused
        {   3.54f, 11.51f,  0.80f },                        //  7: shaman
        {   3.70f, 14.77f,  0.65f },                        //  8: mage
        {   3.18f, 11.30f,  0.82f },                        //  9: warlock
        {   0.0f,   0.0f,  10.0f  },                        // 10: unused
        {   3.33f, 12.41f,  0.79f }                         // 11: druid
    };
    float crit_chance;

    // only players use intelligence for critical chance computations
    if (GetTypeId() == TYPEID_PLAYER)
    {
        int my_class = getClass();
        float crit_ratio = crit_data[my_class].rate0 + crit_data[my_class].rate1 * getLevel();
        crit_chance = crit_data[my_class].base + GetStat(STAT_INTELLECT) / crit_ratio;
    }
    else
        crit_chance = m_baseSpellCritChance;

    crit_chance = crit_chance > 0.0 ? crit_chance : 0.0;

    return crit_chance;
}

float Player::OCTRegenHPPerSpirit()
{
    float regen = 0.0f;

    float Spirit = GetStat(STAT_SPIRIT);
    uint8 Class = getClass();

    switch (Class)
    {
        case CLASS_DRUID:   regen = (Spirit * 0.11 + 1);    break;
        case CLASS_HUNTER:  regen = (Spirit * 0.43 - 5.5);  break;
        case CLASS_MAGE:    regen = (Spirit * 0.11 + 1);    break;
        case CLASS_PALADIN: regen = (Spirit * 0.25);        break;
        case CLASS_PRIEST:  regen = (Spirit * 0.15 + 1.4);  break;
        case CLASS_ROGUE:   regen = (Spirit * 0.84 - 13);   break;
        case CLASS_SHAMAN:  regen = (Spirit * 0.28 - 3.6);  break;
        case CLASS_WARLOCK: regen = (Spirit * 0.12 + 1.5);  break;
        case CLASS_WARRIOR: regen = (Spirit * 1.26 - 22.6); break;
    }

    return regen;
}

float Player::OCTRegenMPPerSpirit()
{
    float addvalue = 0.0;

    float Spirit = GetStat(STAT_SPIRIT);
    uint8 Class = getClass();

    switch (Class)
    {
        case CLASS_DRUID:   addvalue = (Spirit / 5 + 15);   break;
        case CLASS_HUNTER:  addvalue = (Spirit / 5 + 15);   break;
        case CLASS_MAGE:    addvalue = (Spirit / 4 + 12.5); break;
        case CLASS_PALADIN: addvalue = (Spirit / 5 + 15);   break;
        case CLASS_PRIEST:  addvalue = (Spirit / 4 + 12.5); break;
        case CLASS_SHAMAN:  addvalue = (Spirit / 5 + 17);   break;
        case CLASS_WARLOCK: addvalue = (Spirit / 5 + 15);   break;
    }

    addvalue /= 2.0f;   // the above addvalue are given per tick which occurs every 2 seconds, hence this divide by 2

    return addvalue;
}

void Player::SetRegularAttackTime()
{
    for (int i = 0; i < MAX_ATTACK; ++i)
    {
        Item* tmpitem = GetWeaponForAttack(WeaponAttackType(i), true, false);
        if (tmpitem)
        {
            ItemPrototype const* proto = tmpitem->GetProto();
            if (proto->Delay)
                SetAttackTime(WeaponAttackType(i), proto->Delay);
            else
                SetAttackTime(WeaponAttackType(i), BASE_ATTACK_TIME);
        }
    }
}

// skill+step, checking for max value
bool Player::UpdateSkill(uint32 skill_id, uint32 step)
{
    if (!skill_id)
        return false;

    SkillStatusMap::iterator itr = mSkillStatus.find(skill_id);
    if (itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return false;

    uint32 valueIndex = PLAYER_SKILL_VALUE_INDEX(itr->second.pos);
    uint32 data = GetUInt32Value(valueIndex);
    uint32 value = SKILL_VALUE(data);
    uint32 max = SKILL_MAX(data);

    if ((!max) || (!value) || (value >= max))
        return false;

    if (value * 512 < max * urand(0, 512))
    {
        uint32 new_value = value + step;
        if (new_value > max)
            new_value = max;

        SetUInt32Value(valueIndex, MAKE_SKILL_VALUE(new_value, max));
        if (itr->second.uState != SKILL_NEW)
            itr->second.uState = SKILL_CHANGED;
        return true;
    }

    return false;
}

inline int SkillGainChance(uint32 SkillValue, uint32 GrayLevel, uint32 GreenLevel, uint32 YellowLevel)
{
    if (SkillValue >= GrayLevel)
        return sWorld.getConfig(CONFIG_UINT32_SKILL_CHANCE_GREY) * 10;
    if (SkillValue >= GreenLevel)
        return sWorld.getConfig(CONFIG_UINT32_SKILL_CHANCE_GREEN) * 10;
    if (SkillValue >= YellowLevel)
        return sWorld.getConfig(CONFIG_UINT32_SKILL_CHANCE_YELLOW) * 10;
    return sWorld.getConfig(CONFIG_UINT32_SKILL_CHANCE_ORANGE) * 10;
}

bool Player::UpdateCraftSkill(uint32 spellid)
{
    DEBUG_LOG("UpdateCraftSkill spellid %d", spellid);

    SkillLineAbilityMapBounds bounds = sSpellMgr.GetSkillLineAbilityMapBounds(spellid);

    for (SkillLineAbilityMap::const_iterator _spell_idx = bounds.first; _spell_idx != bounds.second; ++_spell_idx)
    {
        if (_spell_idx->second->skillId)
        {
            uint32 SkillValue = GetPureSkillValue(_spell_idx->second->skillId);

            uint32 craft_skill_gain = sWorld.getConfig(CONFIG_UINT32_SKILL_GAIN_CRAFTING);

            return UpdateSkillPro(_spell_idx->second->skillId, SkillGainChance(SkillValue,
                                  _spell_idx->second->max_value,
                                  (_spell_idx->second->max_value + _spell_idx->second->min_value) / 2,
                                  _spell_idx->second->min_value),
                                  craft_skill_gain);
        }
    }
    return false;
}

bool Player::UpdateGatherSkill(uint32 SkillId, uint32 SkillValue, uint32 RedLevel, uint32 Multiplicator)
{
    DEBUG_LOG("UpdateGatherSkill(SkillId %d SkillLevel %d RedLevel %d)", SkillId, SkillValue, RedLevel);

    uint32 gathering_skill_gain = sWorld.getConfig(CONFIG_UINT32_SKILL_GAIN_GATHERING);

    // For skinning and Mining chance decrease with level. 1-74 - no decrease, 75-149 - 2 times, 225-299 - 8 times
    switch (SkillId)
    {
        case SKILL_HERBALISM:
        case SKILL_LOCKPICKING:
            return UpdateSkillPro(SkillId, SkillGainChance(SkillValue, RedLevel + 100, RedLevel + 50, RedLevel + 25) * Multiplicator, gathering_skill_gain);
        case SKILL_SKINNING:
            if (sWorld.getConfig(CONFIG_UINT32_SKILL_CHANCE_SKINNING_STEPS) == 0)
                return UpdateSkillPro(SkillId, SkillGainChance(SkillValue, RedLevel + 100, RedLevel + 50, RedLevel + 25) * Multiplicator, gathering_skill_gain);
            else
                return UpdateSkillPro(SkillId, (SkillGainChance(SkillValue, RedLevel + 100, RedLevel + 50, RedLevel + 25) * Multiplicator) >> (SkillValue / sWorld.getConfig(CONFIG_UINT32_SKILL_CHANCE_SKINNING_STEPS)), gathering_skill_gain);
        case SKILL_MINING:
            if (sWorld.getConfig(CONFIG_UINT32_SKILL_CHANCE_MINING_STEPS) == 0)
                return UpdateSkillPro(SkillId, SkillGainChance(SkillValue, RedLevel + 100, RedLevel + 50, RedLevel + 25) * Multiplicator, gathering_skill_gain);
            else
                return UpdateSkillPro(SkillId, (SkillGainChance(SkillValue, RedLevel + 100, RedLevel + 50, RedLevel + 25) * Multiplicator) >> (SkillValue / sWorld.getConfig(CONFIG_UINT32_SKILL_CHANCE_MINING_STEPS)), gathering_skill_gain);
    }
    return false;
}

bool Player::UpdateFishingSkill()
{
    DEBUG_LOG("UpdateFishingSkill");

    uint32 SkillValue = GetPureSkillValue(SKILL_FISHING);

    int32 chance = SkillValue < 75 ? 100 : 2500 / (SkillValue - 50);

    uint32 gathering_skill_gain = sWorld.getConfig(CONFIG_UINT32_SKILL_GAIN_GATHERING);

    return UpdateSkillPro(SKILL_FISHING, chance * 10, gathering_skill_gain);
}

bool Player::UpdateSkillPro(uint16 SkillId, int32 Chance, uint32 step)
{
    DEBUG_LOG("UpdateSkillPro(SkillId %d, Chance %3.1f%%)", SkillId, Chance / 10.0);
    if (!SkillId)
        return false;

    if (Chance <= 0)                                        // speedup in 0 chance case
    {
        DEBUG_LOG("Player::UpdateSkillPro Chance=%3.1f%% missed", Chance / 10.0);
        return false;
    }

    SkillStatusMap::iterator itr = mSkillStatus.find(SkillId);
    if (itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return false;

    uint32 valueIndex = PLAYER_SKILL_VALUE_INDEX(itr->second.pos);

    uint32 data = GetUInt32Value(valueIndex);
    uint16 SkillValue = SKILL_VALUE(data);
    uint16 MaxValue   = SKILL_MAX(data);

    if (!MaxValue || !SkillValue || SkillValue >= MaxValue)
        return false;

    int32 Roll = irand(1, 1000);

    if (Roll <= Chance)
    {
        uint32 new_value = SkillValue + step;
        if (new_value > MaxValue)
            new_value = MaxValue;

        SetUInt32Value(valueIndex, MAKE_SKILL_VALUE(new_value, MaxValue));
        if (itr->second.uState != SKILL_NEW)
            itr->second.uState = SKILL_CHANGED;
        DEBUG_LOG("Player::UpdateSkillPro Chance=%3.1f%% taken", Chance / 10.0);
        return true;
    }

    DEBUG_LOG("Player::UpdateSkillPro Chance=%3.1f%% missed", Chance / 10.0);
    return false;
}

void Player::UpdateWeaponSkill(WeaponAttackType attType)
{
    // no skill gain in pvp
    Unit* pVictim = getVictim();
    if (pVictim && pVictim->IsCharmerOrOwnerPlayerOrPlayerItself())
        return;

    if (IsInFeralForm())
        return;                                             // always maximized SKILL_FERAL_COMBAT in fact

    if (GetShapeshiftForm() == FORM_TREE)
        return;                                             // use weapon but not skill up

    uint32 weaponSkillGain = sWorld.getConfig(CONFIG_UINT32_SKILL_GAIN_WEAPON);

    Item* pWeapon = GetWeaponForAttack(attType, true, true);
    if (pWeapon && pWeapon->GetProto()->SubClass != ITEM_SUBCLASS_WEAPON_FISHING_POLE)
        UpdateSkill(pWeapon->GetSkill(), weaponSkillGain);
    else if (!pWeapon && attType == BASE_ATTACK)
        UpdateSkill(SKILL_UNARMED, weaponSkillGain);

    UpdateAllCritPercentages();
}

void Player::UpdateCombatSkills(Unit* pVictim, WeaponAttackType attType, bool defence)
{
    uint32 plevel = getLevel();                             // if defense than pVictim == attacker
    uint32 greylevel = MaNGOS::XP::GetGrayLevel(plevel);
    uint32 moblevel = pVictim->GetLevelForTarget(this);
    if (moblevel < greylevel)
        return;

    if (moblevel > plevel + 5)
        moblevel = plevel + 5;

    uint32 lvldif = moblevel - greylevel;
    if (lvldif < 3)
        lvldif = 3;

    int32 skilldif = 5 * plevel - (defence ? GetBaseDefenseSkillValue() : GetBaseWeaponSkillValue(attType));

    // Max skill reached for level.
    // Can in some cases be less than 0: having max skill and then .level -1 as example.
    if (skilldif <= 0)
        return;

    float chance = float(3 * lvldif * skilldif) / plevel;
    if (!defence)
        chance *= 0.1f * GetStat(STAT_INTELLECT);

    chance = chance < 1.0f ? 1.0f : chance;                 // minimum chance to increase skill is 1%

    if (roll_chance_f(chance))
    {
        if (defence)
            UpdateDefense();
        else
            UpdateWeaponSkill(attType);
    }
    else
        return;
}

void Player::ModifySkillBonus(uint32 skillid, int32 val, bool talent)
{
    SkillStatusMap::const_iterator itr = mSkillStatus.find(skillid);
    if (itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return;

    uint32 bonusIndex = PLAYER_SKILL_BONUS_INDEX(itr->second.pos);

    uint32 bonus_val = GetUInt32Value(bonusIndex);
    int16 temp_bonus = SKILL_TEMP_BONUS(bonus_val);
    int16 perm_bonus = SKILL_PERM_BONUS(bonus_val);

    if (talent)                                         // permanent bonus stored in high part
        SetUInt32Value(bonusIndex, MAKE_SKILL_BONUS(temp_bonus, perm_bonus + val));
    else                                                // temporary/item bonus stored in low part
        SetUInt32Value(bonusIndex, MAKE_SKILL_BONUS(temp_bonus + val, perm_bonus));
}

void Player::UpdateSkillsForLevel()
{
    uint16 maxconfskill = sWorld.GetConfigMaxSkillValue();
    uint32 maxSkill = GetMaxSkillValueForLevel();

    bool alwaysMaxSkill = sWorld.getConfig(CONFIG_BOOL_ALWAYS_MAX_SKILL_FOR_LEVEL);

    for (SkillStatusMap::iterator itr = mSkillStatus.begin(); itr != mSkillStatus.end(); ++itr)
    {
        if (itr->second.uState == SKILL_DELETED)
            continue;

        uint32 pskill = itr->first;

        SkillLineEntry const* pSkill = sSkillLineStore.LookupEntry(pskill);
        if (!pSkill)
            continue;

        if (GetSkillRangeType(pSkill, false) != SKILL_RANGE_LEVEL)
            continue;

        uint32 valueIndex = PLAYER_SKILL_VALUE_INDEX(itr->second.pos);
        uint32 data = GetUInt32Value(valueIndex);
        uint32 max = SKILL_MAX(data);
        uint32 val = SKILL_VALUE(data);

        /// update only level dependent max skill values
        if (max != 1)
        {
            /// maximize skill always
            if (alwaysMaxSkill)
            {
                SetUInt32Value(valueIndex, MAKE_SKILL_VALUE(maxSkill, maxSkill));
                if (itr->second.uState != SKILL_NEW)
                    itr->second.uState = SKILL_CHANGED;
            }
            else if (max != maxconfskill)                   /// update max skill value if current max skill not maximized
            {
                SetUInt32Value(valueIndex, MAKE_SKILL_VALUE(val, maxSkill));
                if (itr->second.uState != SKILL_NEW)
                    itr->second.uState = SKILL_CHANGED;
            }
        }
    }
}

void Player::UpdateSkillsToMaxSkillsForLevel()
{
    for (SkillStatusMap::iterator itr = mSkillStatus.begin(); itr != mSkillStatus.end(); ++itr)
    {
        if (itr->second.uState == SKILL_DELETED)
            continue;

        uint32 pskill = itr->first;
        if (IsProfessionOrRidingSkill(pskill))
            continue;
        uint32 valueIndex = PLAYER_SKILL_VALUE_INDEX(itr->second.pos);
        uint32 data = GetUInt32Value(valueIndex);

        uint32 max = SKILL_MAX(data);

        if (max > 1)
        {
            SetUInt32Value(valueIndex, MAKE_SKILL_VALUE(max, max));
            if (itr->second.uState != SKILL_NEW)
                itr->second.uState = SKILL_CHANGED;
        }

        if (pskill == SKILL_DEFENSE)
            UpdateDefenseBonusesMod();
    }
}

// This functions sets a skill line value (and adds if doesn't exist yet)
// To "remove" a skill line, set it's values to zero
void Player::SetSkill(uint16 id, uint16 currVal, uint16 maxVal, uint16 step /*=0*/)
{
    if (!id)
        return;

    SkillStatusMap::iterator itr = mSkillStatus.find(id);

    // has skill
    if (itr != mSkillStatus.end() && itr->second.uState != SKILL_DELETED)
    {
        if (currVal)
        {
            if (step)                                      // need update step
                SetUInt32Value(PLAYER_SKILL_INDEX(itr->second.pos), MAKE_PAIR32(id, step));
            // update value
            SetUInt32Value(PLAYER_SKILL_VALUE_INDEX(itr->second.pos), MAKE_SKILL_VALUE(currVal, maxVal));
            if (itr->second.uState != SKILL_NEW)
                itr->second.uState = SKILL_CHANGED;
            // learnSkillRewardedSpells(id, currVal);       // pre-3.x have only 1 skill level req (so at learning only)
        }
        else                                                // remove
        {
            // clear skill fields
            SetUInt32Value(PLAYER_SKILL_INDEX(itr->second.pos), 0);
            SetUInt32Value(PLAYER_SKILL_VALUE_INDEX(itr->second.pos), 0);
            SetUInt32Value(PLAYER_SKILL_BONUS_INDEX(itr->second.pos), 0);

            // mark as deleted or simply remove from map if not saved yet
            if (itr->second.uState != SKILL_NEW)
                itr->second.uState = SKILL_DELETED;
            else
                mSkillStatus.erase(itr);

            // remove all spells that related to this skill
            for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
                if (SkillLineAbilityEntry const* pAbility = sSkillLineAbilityStore.LookupEntry(j))
                    if (pAbility->skillId == id)
                        removeSpell(sSpellMgr.GetFirstSpellInChain(pAbility->spellId));
        }
    }
    else if (currVal)                                       // add
    {
        for (int i = 0; i < PLAYER_MAX_SKILLS; ++i)
        {
            if (!GetUInt32Value(PLAYER_SKILL_INDEX(i)))
            {
                SkillLineEntry const* pSkill = sSkillLineStore.LookupEntry(id);
                if (!pSkill)
                {
                    sLog.outError("Skill not found in SkillLineStore: skill #%u", id);
                    return;
                }

                SetUInt32Value(PLAYER_SKILL_INDEX(i), MAKE_PAIR32(id, step));
                SetUInt32Value(PLAYER_SKILL_VALUE_INDEX(i), MAKE_SKILL_VALUE(currVal, maxVal));

                // insert new entry or update if not deleted old entry yet
                if (itr != mSkillStatus.end())
                {
                    itr->second.pos = i;
                    itr->second.uState = SKILL_CHANGED;
                }
                else
                    mSkillStatus.insert(SkillStatusMap::value_type(id, SkillStatusData(i, SKILL_NEW)));

                // apply skill bonuses
                SetUInt32Value(PLAYER_SKILL_BONUS_INDEX(i), 0);

                // temporary bonuses
                AuraList const& mModSkill = GetAurasByType(SPELL_AURA_MOD_SKILL);
                for (AuraList::const_iterator j = mModSkill.begin(); j != mModSkill.end(); ++j)
                    if ((*j)->GetModifier()->m_miscvalue == int32(id))
                        (*j)->ApplyModifier(true);

                // permanent bonuses
                AuraList const& mModSkillTalent = GetAurasByType(SPELL_AURA_MOD_SKILL_TALENT);
                for (AuraList::const_iterator j = mModSkillTalent.begin(); j != mModSkillTalent.end(); ++j)
                    if ((*j)->GetModifier()->m_miscvalue == int32(id))
                        (*j)->ApplyModifier(true);

                // Learn all spells for skill
                learnSkillRewardedSpells(id, currVal);
                return;
            }
        }
    }
}

bool Player::HasSkill(uint32 skill) const
{
    if (!skill)
        return false;

    SkillStatusMap::const_iterator itr = mSkillStatus.find(skill);
    return (itr != mSkillStatus.end() && itr->second.uState != SKILL_DELETED);
}

uint16 Player::GetSkillValue(uint32 skill) const
{
    if (!skill)
        return 0;

    SkillStatusMap::const_iterator itr = mSkillStatus.find(skill);
    if (itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return 0;

    uint32 bonus = GetUInt32Value(PLAYER_SKILL_BONUS_INDEX(itr->second.pos));

    int32 result = int32(SKILL_VALUE(GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(itr->second.pos))));
    result += SKILL_TEMP_BONUS(bonus);
    result += SKILL_PERM_BONUS(bonus);
    return result < 0 ? 0 : result;
}

uint16 Player::GetMaxSkillValue(uint32 skill) const
{
    if (!skill)
        return 0;

    SkillStatusMap::const_iterator itr = mSkillStatus.find(skill);
    if (itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return 0;

    uint32 bonus = GetUInt32Value(PLAYER_SKILL_BONUS_INDEX(itr->second.pos));

    int32 result = int32(SKILL_MAX(GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(itr->second.pos))));
    result += SKILL_TEMP_BONUS(bonus);
    result += SKILL_PERM_BONUS(bonus);
    return result < 0 ? 0 : result;
}

uint16 Player::GetPureMaxSkillValue(uint32 skill) const
{
    if (!skill)
        return 0;

    SkillStatusMap::const_iterator itr = mSkillStatus.find(skill);
    if (itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return 0;

    return SKILL_MAX(GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(itr->second.pos)));
}

uint16 Player::GetBaseSkillValue(uint32 skill) const
{
    if (!skill)
        return 0;

    SkillStatusMap::const_iterator itr = mSkillStatus.find(skill);
    if (itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return 0;

    int32 result = int32(SKILL_VALUE(GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(itr->second.pos))));
    result +=  SKILL_PERM_BONUS(GetUInt32Value(PLAYER_SKILL_BONUS_INDEX(itr->second.pos)));
    return result < 0 ? 0 : result;
}

uint16 Player::GetPureSkillValue(uint32 skill) const
{
    if (!skill)
        return 0;

    SkillStatusMap::const_iterator itr = mSkillStatus.find(skill);
    if (itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return 0;

    return SKILL_VALUE(GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(itr->second.pos)));
}

int16 Player::GetSkillPermBonusValue(uint32 skill) const
{
    if (!skill)
        return 0;

    SkillStatusMap::const_iterator itr = mSkillStatus.find(skill);
    if (itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return 0;

    return SKILL_PERM_BONUS(GetUInt32Value(PLAYER_SKILL_BONUS_INDEX(itr->second.pos)));
}

int16 Player::GetSkillTempBonusValue(uint32 skill) const
{
    if (!skill)
        return 0;

    SkillStatusMap::const_iterator itr = mSkillStatus.find(skill);
    if (itr == mSkillStatus.end() || itr->second.uState == SKILL_DELETED)
        return 0;

    return SKILL_TEMP_BONUS(GetUInt32Value(PLAYER_SKILL_BONUS_INDEX(itr->second.pos)));
}

void Player::SendInitialActionButtons() const
{
    DETAIL_LOG("Initializing Action Buttons for '%u'", GetGUIDLow());

    WorldPacket data(SMSG_ACTION_BUTTONS, (MAX_ACTION_BUTTONS * 4));
    for (uint8 button = 0; button < MAX_ACTION_BUTTONS; ++button)
    {
        ActionButtonList::const_iterator itr = m_actionButtons.find(button);
        if (itr != m_actionButtons.end() && itr->second.uState != ACTIONBUTTON_DELETED)
            data << uint32(itr->second.packedData);
        else
            data << uint32(0);
    }

    GetSession()->SendPacket(&data);
    DETAIL_LOG("Action Buttons for '%u' Initialized", GetGUIDLow());
}

bool Player::IsActionButtonDataValid(uint8 button, uint32 action, uint8 type, Player* player)
{
    if (button >= MAX_ACTION_BUTTONS)
    {
        if (player)
            sLog.outError("Action %u not added into button %u for player %s: button must be < %u", action, button, player->GetName(), MAX_ACTION_BUTTONS);
        else
            sLog.outError("Table `playercreateinfo_action` have action %u into button %u : button must be < %u", action, button, MAX_ACTION_BUTTONS);
        return false;
    }

    if (action >= MAX_ACTION_BUTTON_ACTION_VALUE)
    {
        if (player)
            sLog.outError("Action %u not added into button %u for player %s: action must be < %u", action, button, player->GetName(), MAX_ACTION_BUTTON_ACTION_VALUE);
        else
            sLog.outError("Table `playercreateinfo_action` have action %u into button %u : action must be < %u", action, button, MAX_ACTION_BUTTON_ACTION_VALUE);
        return false;
    }

    switch (type)
    {
        case ACTION_BUTTON_SPELL:
        {
            SpellEntry const* spellProto = sSpellStore.LookupEntry(action);
            if (!spellProto)
            {
                if (player)
                    sLog.outError("Spell action %u not added into button %u for player %s: spell not exist", action, button, player->GetName());
                else
                    sLog.outError("Table `playercreateinfo_action` have spell action %u into button %u: spell not exist", action, button);
                return false;
            }

            if (player)
            {
                if (!player->HasSpell(spellProto->Id))
                {
                    sLog.outError("Spell action %u not added into button %u for player %s: player don't known this spell", action, button, player->GetName());
                    return false;
                }
                else if (IsPassiveSpell(spellProto))
                {
                    sLog.outError("Spell action %u not added into button %u for player %s: spell is passive", action, button, player->GetName());
                    return false;
                }
            }
            break;
        }
        case ACTION_BUTTON_ITEM:
        {
            if (!ObjectMgr::GetItemPrototype(action))
            {
                if (player)
                    sLog.outError("Item action %u not added into button %u for player %s: item not exist", action, button, player->GetName());
                else
                    sLog.outError("Table `playercreateinfo_action` have item action %u into button %u: item not exist", action, button);
                return false;
            }
            break;
        }
        default:
            break;                                          // other cases not checked at this moment
    }

    return true;
}

ActionButton* Player::addActionButton(uint8 button, uint32 action, uint8 type)
{
    if (!IsActionButtonDataValid(button, action, type, this))
        return NULL;

    // it create new button (NEW state) if need or return existing
    ActionButton& ab = m_actionButtons[button];

    // set data and update to CHANGED if not NEW
    ab.SetActionAndType(action, ActionButtonType(type));

    DETAIL_LOG("Player '%u' Added Action '%u' (type %u) to Button '%u'", GetGUIDLow(), action, uint32(type), button);
    return &ab;
}

void Player::removeActionButton(uint8 button)
{
    ActionButtonList::iterator buttonItr = m_actionButtons.find(button);
    if (buttonItr == m_actionButtons.end())
        return;

    if (buttonItr->second.uState == ACTIONBUTTON_NEW)
        m_actionButtons.erase(buttonItr);                   // new and not saved
    else
        buttonItr->second.uState = ACTIONBUTTON_DELETED;    // saved, will deleted at next save

    DETAIL_LOG("Action Button '%u' Removed from Player '%u'", button, GetGUIDLow());
}

bool Player::SetPosition(float x, float y, float z, float orientation, bool teleport)
{
    // prevent crash when a bad coord is sent by the client
    if (!MaNGOS::IsValidMapCoord(x, y, z, orientation))
    {
        DEBUG_LOG("Player::SetPosition(%f, %f, %f, %f, %d) .. bad coordinates for player %d!", x, y, z, orientation, teleport, GetGUIDLow());
        return false;
    }

    Map* m = GetMap();

    const float old_x = GetPositionX();
    const float old_y = GetPositionY();
    const float old_z = GetPositionZ();
    const float old_r = GetOrientation();

    if (teleport || old_x != x || old_y != y || old_z != z || old_r != orientation)
    {
        if (teleport || old_x != x || old_y != y || old_z != z)
            RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_MOVE | AURA_INTERRUPT_FLAG_TURNING);
        else
            RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TURNING);

        RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

        // move and update visible state if need
        m->PlayerRelocation(this, x, y, z, orientation);

        // reread after Map::Relocation
        m = GetMap();
        x = GetPositionX();
        y = GetPositionY();
        z = GetPositionZ();

        // group update
        if (GetGroup() && (old_x != x || old_y != y))
            SetGroupUpdateFlag(GROUP_UPDATE_FLAG_POSITION);

        if (GetTrader() && !IsWithinDistInMap(GetTrader(), INTERACTION_DISTANCE))
            GetSession()->SendCancelTrade();   // will close both side trade windows
    }

    if (m_positionStatusUpdateTimer)                        // Update position's state only on interval
        return true;
    m_positionStatusUpdateTimer = 100;

    // code block for underwater state update
    UpdateUnderwaterState(m, x, y, z);

    // code block for outdoor state and area-explore check
    CheckAreaExploreAndOutdoor();

    return true;
}

void Player::SaveRecallPosition()
{
    m_recallMap = GetMapId();
    m_recallX = GetPositionX();
    m_recallY = GetPositionY();
    m_recallZ = GetPositionZ();
    m_recallO = GetOrientation();
}

void Player::SendMessageToSet(WorldPacket* data, bool self) const
{
    if (IsInWorld())
        GetMap()->MessageBroadcast(this, data, false);

    // if player is not in world and map in not created/already destroyed
    // no need to create one, just send packet for itself!
    if (self)
        GetSession()->SendPacket(data);
}

void Player::SendMessageToSetInRange(WorldPacket* data, float dist, bool self) const
{
    if (IsInWorld())
        GetMap()->MessageDistBroadcast(this, data, dist, false);

    if (self)
        GetSession()->SendPacket(data);
}

void Player::SendMessageToSetInRange(WorldPacket* data, float dist, bool self, bool own_team_only) const
{
    if (IsInWorld())
        GetMap()->MessageDistBroadcast(this, data, dist, false, own_team_only);

    if (self)
        GetSession()->SendPacket(data);
}

void Player::SendDirectMessage(WorldPacket* data) const
{
    GetSession()->SendPacket(data);
}

void Player::SendCinematicStart(uint32 CinematicSequenceId)
{
    WorldPacket data(SMSG_TRIGGER_CINEMATIC, 4);
    data << uint32(CinematicSequenceId);
    SendDirectMessage(&data);
}

void Player::CheckAreaExploreAndOutdoor()
{
    if (!isAlive())
        return;

    if (IsTaxiFlying())
        return;

    bool isOutdoor;
    uint16 areaFlag = GetTerrain()->GetAreaFlag(GetPositionX(), GetPositionY(), GetPositionZ(), &isOutdoor);

    if (isOutdoor)
    {
        if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) && GetRestType() == REST_TYPE_IN_TAVERN)
        {
            AreaTriggerEntry const* at = sAreaTriggerStore.LookupEntry(inn_trigger_id);
            if (!at || !IsPointInAreaTriggerZone(at, GetMapId(), GetPositionX(), GetPositionY(), GetPositionZ()))
            {
                // Player left inn (REST_TYPE_IN_CITY overrides REST_TYPE_IN_TAVERN, so just clear rest)
                SetRestType(REST_TYPE_NO);
            }
        }
        // Check if we need to reaply outdoor only passive spells
        const PlayerSpellMap& sp_list = GetSpellMap();
        for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
        {
            if (itr->second.state == PLAYERSPELL_REMOVED)
                continue;
            SpellEntry const* spellInfo = sSpellStore.LookupEntry(itr->first);
            if (!spellInfo || !IsNeedCastSpellAtOutdoor(spellInfo) || HasAura(itr->first))
                continue;
            CastSpell(this, itr->first, true, NULL);
        }
    }
    else if (sWorld.getConfig(CONFIG_BOOL_VMAP_INDOOR_CHECK) && !isGameMaster())
        RemoveAurasWithAttribute(SPELL_ATTR_OUTDOORS_ONLY);

    if (areaFlag == 0xffff)
        return;
    int offset = areaFlag / 32;

    if (offset >= PLAYER_EXPLORED_ZONES_SIZE)
    {
        sLog.outError("Wrong area flag %u in map data for (X: %f Y: %f) point to field PLAYER_EXPLORED_ZONES_1 + %u ( %u must be < %u ).", areaFlag, GetPositionX(), GetPositionY(), offset, offset, PLAYER_EXPLORED_ZONES_SIZE);
        return;
    }

    uint32 val = (uint32)(1 << (areaFlag % 32));
    uint32 currFields = GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);

    if (!(currFields & val))
    {
        SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields | val));

        AreaTableEntry const* p = GetAreaEntryByAreaFlagAndMap(areaFlag, GetMapId());
        if (!p)
        {
            sLog.outError("PLAYER: Player %u discovered unknown area (x: %f y: %f map: %u", GetGUIDLow(), GetPositionX(), GetPositionY(), GetMapId());
        }
        else if (p->area_level > 0)
        {
            uint32 area = p->ID;
            if (getLevel() >= sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
            {
                SendExplorationExperience(area, 0);
            }
            else
            {
                int32 diff = int32(getLevel()) - p->area_level;
                uint32 XP = 0;
                if (diff < -5)
                {
                    XP = uint32(sObjectMgr.GetBaseXP(getLevel() + 5) * sWorld.getConfig(CONFIG_FLOAT_RATE_XP_EXPLORE));
                }
                else if (diff > 5)
                {
                    int32 exploration_percent = (100 - ((diff - 5) * 5));
                    if (exploration_percent > 100)
                        exploration_percent = 100;
                    else if (exploration_percent < 0)
                        exploration_percent = 0;

                    XP = uint32(sObjectMgr.GetBaseXP(p->area_level) * exploration_percent / 100 * sWorld.getConfig(CONFIG_FLOAT_RATE_XP_EXPLORE));
                }
                else
                {
                    XP = uint32(sObjectMgr.GetBaseXP(p->area_level) * sWorld.getConfig(CONFIG_FLOAT_RATE_XP_EXPLORE));
                }

                GiveXP(XP, NULL);
                SendExplorationExperience(area, XP);
            }
            DETAIL_LOG("PLAYER: Player %u discovered a new area: %u", GetGUIDLow(), area);
        }
    }
}

Team Player::TeamForRace(uint8 race)
{
    ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(race);
    if (!rEntry)
    {
        sLog.outError("Race %u not found in DBC: wrong DBC files?", uint32(race));
        return ALLIANCE;
    }

    switch (rEntry->TeamID)
    {
        case 7: return ALLIANCE;
        case 1: return HORDE;
    }

    sLog.outError("Race %u have wrong teamid %u in DBC: wrong DBC files?", uint32(race), rEntry->TeamID);
    return TEAM_NONE;
}

uint32 Player::getFactionForRace(uint8 race)
{
    ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(race);
    if (!rEntry)
    {
        sLog.outError("Race %u not found in DBC: wrong DBC files?", uint32(race));
        return 0;
    }

    return rEntry->FactionID;
}

void Player::setFactionForRace(uint8 race)
{
    m_team = TeamForRace(race);
    setFaction(getFactionForRace(race));
}

ReputationRank Player::GetReputationRank(uint32 faction) const
{
    FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction);
    return GetReputationMgr().GetRank(factionEntry);
}

// Calculate total reputation percent player gain with quest/creature level
int32 Player::CalculateReputationGain(ReputationSource source, int32 rep, int32 faction, uint32 creatureOrQuestLevel, bool noAuraBonus)
{
    float percent = 100.0f;

    float repMod = noAuraBonus ? 0.0f : (float)GetTotalAuraModifier(SPELL_AURA_MOD_REPUTATION_GAIN);

    // faction specific auras only seem to apply to kills
    if (source == REPUTATION_SOURCE_KILL)
        repMod += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_FACTION_REPUTATION_GAIN, faction);

    percent += rep > 0 ? repMod : -repMod;

    float rate;
    switch (source)
    {
        case REPUTATION_SOURCE_KILL:
            rate = sWorld.getConfig(CONFIG_FLOAT_RATE_REPUTATION_LOWLEVEL_KILL);
            break;
        case REPUTATION_SOURCE_QUEST:
            rate = sWorld.getConfig(CONFIG_FLOAT_RATE_REPUTATION_LOWLEVEL_QUEST);
            break;
        case REPUTATION_SOURCE_SPELL:
        default:
            rate = 1.0f;
            break;
    }

    if (rate != 1.0f && creatureOrQuestLevel <= MaNGOS::XP::GetGrayLevel(getLevel()))
        percent *= rate;

    if (percent <= 0.0f)
        return 0;

    // Multiply result with the faction specific rate
    if (const RepRewardRate* repData = sObjectMgr.GetRepRewardRate(faction))
    {
        float repRate = 0.0f;
        switch (source)
        {
            case REPUTATION_SOURCE_KILL:
                repRate = repData->creature_rate;
                break;
            case REPUTATION_SOURCE_QUEST:
                repRate = repData->quest_rate;
                break;
            case REPUTATION_SOURCE_SPELL:
                repRate = repData->spell_rate;
                break;
        }

        // for custom, a rate of 0.0 will totally disable reputation gain for this faction/type
        if (repRate <= 0.0f)
            return 0;

        percent *= repRate;
    }

    return int32(sWorld.getConfig(CONFIG_FLOAT_RATE_REPUTATION_GAIN) * rep * percent / 100.0f);
}

// Calculates how many reputation points player gains in victim's enemy factions
void Player::RewardReputation(Unit* pVictim, float rate)
{
    if (!pVictim || pVictim->GetTypeId() == TYPEID_PLAYER)
        return;

    if (((Creature*)pVictim)->IsReputationGainDisabled())
        return;

    ReputationOnKillEntry const* Rep = sObjectMgr.GetReputationOnKillEntry(((Creature*)pVictim)->GetEntry());

    if (!Rep)
        return;

    if (Rep->repfaction1 && (!Rep->team_dependent || GetTeam() == ALLIANCE))
    {
        int32 donerep1 = CalculateReputationGain(REPUTATION_SOURCE_KILL, Rep->repvalue1, Rep->repfaction1, pVictim->getLevel());
        donerep1 = int32(donerep1 * rate);
        FactionEntry const* factionEntry1 = sFactionStore.LookupEntry(Rep->repfaction1);
        uint32 current_reputation_rank1 = GetReputationMgr().GetRank(factionEntry1);
        if (factionEntry1 && current_reputation_rank1 <= Rep->reputation_max_cap1)
            GetReputationMgr().ModifyReputation(factionEntry1, donerep1);

        // Wiki: Team factions value divided by 2
        if (factionEntry1 && Rep->is_teamaward1)
        {
            FactionEntry const* team1_factionEntry = sFactionStore.LookupEntry(factionEntry1->team);
            if (team1_factionEntry)
                GetReputationMgr().ModifyReputation(team1_factionEntry, donerep1 / 2);
        }
    }

    if (Rep->repfaction2 && (!Rep->team_dependent || GetTeam() == HORDE))
    {
        int32 donerep2 = CalculateReputationGain(REPUTATION_SOURCE_KILL, Rep->repvalue2, Rep->repfaction2, pVictim->getLevel());
        donerep2 = int32(donerep2 * rate);
        FactionEntry const* factionEntry2 = sFactionStore.LookupEntry(Rep->repfaction2);
        uint32 current_reputation_rank2 = GetReputationMgr().GetRank(factionEntry2);
        if (factionEntry2 && current_reputation_rank2 <= Rep->reputation_max_cap2)
            GetReputationMgr().ModifyReputation(factionEntry2, donerep2);

        // Wiki: Team factions value divided by 2
        if (factionEntry2 && Rep->is_teamaward2)
        {
            FactionEntry const* team2_factionEntry = sFactionStore.LookupEntry(factionEntry2->team);
            if (team2_factionEntry)
                GetReputationMgr().ModifyReputation(team2_factionEntry, donerep2 / 2);
        }
    }
}

// Calculate how many reputation points player gain with the quest
void Player::RewardReputation(Quest const* pQuest)
{
    // quest reputation reward/loss
    for (int i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)
    {
        if (!pQuest->RewRepFaction[i])
            continue;

        if (pQuest->RewRepValue[i])
        {
            int32 rep = CalculateReputationGain(REPUTATION_SOURCE_QUEST,  pQuest->RewRepValue[i], pQuest->RewRepFaction[i], GetQuestLevelForPlayer(pQuest));

            if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(pQuest->RewRepFaction[i]))
                GetReputationMgr().ModifyReputation(factionEntry, rep);
        }
    }

    // TODO: implement reputation spillover
}

// Update honor fields , cleanKills is only used during char saving
void Player::UpdateHonor()
{
    uint32 lastweek_honorableKills = 0;
    uint32 today_honorableKills = 0;
    uint32 today_dishonorableKills = 0;
    uint32 yesterdayKills = 0;
    uint32 thisWeekKills = 0;
    uint32 lastWeekKills = 0;

    float yesterdayHonor = 0.0f;
    float thisWeekHonor = 0.0f;
    float lastWeekHonor = 0.0f;
    float todayLostHonor = 0.0f;

    uint32 today = sWorld.GetDateToday();

    uint32 yesterday     = today - 1;
    uint32 thisWeekBegin = sWorld.GetDateLastMaintenanceDay();
    uint32 thisWeekEnd   = thisWeekBegin + 7;
    uint32 lastWeekBegin = thisWeekBegin - 7;
    uint32 lastWeekEnd   = lastWeekBegin + 7;

    DETAIL_LOG("PLAYER: UpdateHonor");

    uint32 total_dishonorableKills = GetHonorStoredKills(false);
    uint32 total_honorableKills = GetHonorStoredKills(true);

    for (HonorCPMap::iterator itr = m_honorCP.begin(); itr != m_honorCP.end(); ++itr)
    {
        if (itr->state == HK_DELETED)
            continue;

        if (itr->type == HONORABLE)
        {
            if (itr->isKill)
                total_honorableKills++;

            if (itr->isKill && itr->date == today)
                today_honorableKills++;

            if (itr->date == yesterday)
            {
                if (itr->isKill)
                    yesterdayKills++;
                yesterdayHonor += itr->honorPoints;
            }
            if ((itr->date >= thisWeekBegin) && (itr->date <= thisWeekEnd))
            {
                if (itr->isKill)
                    thisWeekKills++;
                thisWeekHonor += itr->honorPoints;
            }
            if ((itr->date >= lastWeekBegin) && (itr->date < lastWeekEnd))
            {
                if (itr->isKill)
                    lastWeekKills++;
                lastWeekHonor += itr->honorPoints;
            }
        }
        else if (itr->isKill && itr->type == DISHONORABLE)
        {
            total_dishonorableKills++;

            if (itr->date == today)
                today_dishonorableKills++;

            if (itr->date > today)
                itr->state = HK_OLD;
        }
    }

    // STANDING
    SetHonorLastWeekStandingPos(sObjectMgr.GetHonorStandingPositionByGUID(GetGUIDLow(), GetTeam()));

    // RANK POINTS
    HonorStanding* standing = sObjectMgr.GetHonorStandingByGUID(GetGUIDLow(), GetTeam());
    float rankP = GetStoredHonor();
    if (standing)
        rankP += standing->rpEarning;

    SetRankPoints(rankP);

    // RIGHEST RANK
    // If the new rank is highest then the old one, then m_highest_rank is updated
    HonorRankInfo prk =  MaNGOS::Honor::CalculateHonorRank(GetRankPoints());
    SetHonorRankInfo(prk);
    if (prk.visualRank > 0 && prk.visualRank > GetHonorHighestRankInfo().visualRank)
        SetHonorHighestRankInfo(prk);

    // rank points is sent to client with same size of uint8(255) for each rank
    // so we set it in correct rate:
    uint32 RP = uint32(GetRankPoints() >= 0 ? GetRankPoints() : -1 * GetRankPoints());
    RP = int8(((RP - prk.minRP) / (prk.maxRP - prk.minRP)) * (prk.positive ? 255 : -255));


    // NEXT RANK BAR
    // PLAYER_FIELD_HONOR_BAR
    SetByteValue(PLAYER_FIELD_BYTES2, 0, RP);
    // RANK (Patent)
    SetByteValue(PLAYER_BYTES_3, 3, GetHonorRankInfo().rank);
    // TODAY
    SetUInt16Value(PLAYER_FIELD_SESSION_KILLS, 0, today_honorableKills);
    SetUInt16Value(PLAYER_FIELD_SESSION_KILLS, 1, today_dishonorableKills);
    // YESTERDAY
    SetUInt32Value(PLAYER_FIELD_YESTERDAY_KILLS, yesterdayKills);
    SetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION, (uint32)(yesterdayHonor > 0.0f ? yesterdayHonor : 0.0f));
    // THIS WEEK
    SetUInt32Value(PLAYER_FIELD_THIS_WEEK_KILLS, thisWeekKills);
    SetUInt32Value(PLAYER_FIELD_THIS_WEEK_CONTRIBUTION, (uint32)(thisWeekHonor > 0.0f ? thisWeekHonor : 0.0f));
    // LAST WEEK
    SetUInt32Value(PLAYER_FIELD_LAST_WEEK_KILLS, lastWeekKills);
    SetUInt32Value(PLAYER_FIELD_LAST_WEEK_CONTRIBUTION, (uint32)(lastWeekHonor > 0.0f ? lastWeekHonor : 0.0f));
    SetUInt32Value(PLAYER_FIELD_LAST_WEEK_RANK, GetHonorLastWeekStandingPos());
    // LIFE TIME
    SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, total_honorableKills);
    SetUInt32Value(PLAYER_FIELD_LIFETIME_DISHONORABLE_KILLS, total_dishonorableKills);
    // TODO: Into what field we need to set it? Fix it!
    // SetUInt32Value(PLAYER_FIELD_PVP_MEDALS/*???*/, (GetHonorHighestRankInfo().rank << 24) | 0x0F0001);
    // ITEM FIELD RANK REQUIRED
    SetByteValue(PLAYER_FIELD_BYTES, 3, GetHonorHighestRankInfo().rank);
}

void Player::ResetHonor()
{
    // it will delete all honor permanently
    CharacterDatabase.PExecute("DELETE FROM character_honor_cp WHERE guid = '%u'", GetGUIDLow());
    ClearHonorInfo();
    UpdateHonor();
}

// set all honor info to default
void Player::ClearHonorInfo()
{
    m_honorCP.clear();
    SetHonorStoredKills(0, true);
    SetHonorStoredKills(0, false);
    SetStoredHonor(0);
    SetHonorLastWeekStandingPos(0);
    MaNGOS::Honor::InitRankInfo(m_honor_rank);
    MaNGOS::Honor::InitRankInfo(m_highest_rank);
}

// How many times Player kill pVictim... ( toDate shouldn't be > lastWeekBegin )
uint32 Player::CalculateTotalKills(Unit* Victim, uint32 fromDate, uint32 toDate) const
{
    uint32 total_kills = 0;
    uint32 ID = 0;

    if (!Victim)
        return 0;

    uint8 vType = Victim->GetTypeId();

    switch (vType)
    {
        case TYPEID_PLAYER:
            ID = ((Player*)Victim)->GetGUIDLow();
            break;
        case TYPEID_UNIT: // actually kills count not used for NPCs
            ID = Victim->GetEntry();
            break;
        default: // cheat?
            return 0;
    }

    for (HonorCPMap::const_iterator itr = m_honorCP.begin(); itr != m_honorCP.end(); ++itr)
        if (itr->victimType != TYPEID_OBJECT && itr->victimType == vType && itr->victimID == ID)
            if (itr->date >= fromDate && itr->date <= toDate)
                total_kills ++;

    return total_kills;
}

// How much honor Player gains/loses killing uVictim
bool Player::RewardHonor(Unit* uVictim, uint32 groupsize)
{
    float honor_points = 0;
    int kill_type = 0;

    DETAIL_LOG("PLAYER: RewardHonor");

    if (!uVictim)
        return false;

    if (uVictim->GetAura(2479, EFFECT_INDEX_0))             // Honorless Target
        return false;

    if (uVictim->GetTypeId() == TYPEID_UNIT)
    {
        Creature* cVictim = (Creature*)uVictim;
        if (cVictim->IsCivilian())
        {
            AddHonorCP(MaNGOS::Honor::DishonorableKillPoints(getLevel()), DISHONORABLE, cVictim->GetEntry(), TYPEID_UNIT);
            return true;
        }

        if (cVictim->IsRacialLeader())
        {
            // maybe uncorrect honor value but no source to get it actually
            AddHonorCP(398.0, HONORABLE, cVictim->GetEntry(), TYPEID_UNIT);
            return true;
        }
    }
    else if (uVictim->GetTypeId() == TYPEID_PLAYER)
    {
        Player* pVictim = (Player*)uVictim;

        if (GetTeam() == pVictim->GetTeam())
            return false;

        if (getLevel() < (pVictim->getLevel() + 5))
        {
            AddHonorCP(MaNGOS::Honor::HonorableKillPoints(this, pVictim, groupsize), HONORABLE, pVictim->GetGUIDLow(), TYPEID_PLAYER);
            return true;
        }
    }

    return false;
}

bool Player::AddHonorCP(float honor, uint8 type, uint32 victim, uint8 victimType)
{

    if (!honor)
        return false;

    // CharacterDatabase.PExecute("INSERT INTO `character_honor_cp` (`guid`,`victim`,`victim_type`,`honor`,`date`,`type`) VALUES (%u, %u, %u, %f, %u, %u)", (uint32)GetGUIDLow(), (uint32)uVictim->GetEntry(),uVictim->GetType() (float)honor_points, (uint32)today, (uint8)kill_type);

    HonorCP CP;
    CP.date = sWorld.GetDateToday();
    CP.honorPoints = honor;
    CP.victimID = victim;
    CP.victimType = victimType;
    CP.type = type;

    if (type == DISHONORABLE)
    {
        // DK penalties are subtracted from your RP score immediately
        // and are not included in weekly adjustment
        float RP = GetRankPoints() > CP.honorPoints ? GetRankPoints() - CP.honorPoints : 0; // remove this check to have negative ranks
        SetStoredHonor(RP);
    }

    CP.state  =  HK_NEW;
    CP.isKill =  isKill(victimType);

    m_honorCP.push_back(CP);

    UpdateHonor();
    return true;
}

uint32 Player::GetGuildIdFromDB(ObjectGuid guid)
{
    uint32 lowguid = guid.GetCounter();

    QueryResult* result = CharacterDatabase.PQuery("SELECT guildid FROM guild_member WHERE guid='%u'", lowguid);
    if (!result)
        return 0;

    uint32 id = result->Fetch()[0].GetUInt32();
    delete result;
    return id;
}

uint32 Player::GetRankFromDB(ObjectGuid guid)
{
    QueryResult* result = CharacterDatabase.PQuery("SELECT rank FROM guild_member WHERE guid='%u'", guid.GetCounter());
    if (result)
    {
        uint32 v = result->Fetch()[0].GetUInt32();
        delete result;
        return v;
    }
    else
        return 0;
}

uint32 Player::GetZoneIdFromDB(ObjectGuid guid)
{
    uint32 lowguid = guid.GetCounter();
    QueryResult* result = CharacterDatabase.PQuery("SELECT zone FROM characters WHERE guid='%u'", lowguid);
    if (!result)
        return 0;
    Field* fields = result->Fetch();
    uint32 zone = fields[0].GetUInt32();
    delete result;

    if (!zone)
    {
        // stored zone is zero, use generic and slow zone detection
        result = CharacterDatabase.PQuery("SELECT map,position_x,position_y,position_z FROM characters WHERE guid='%u'", lowguid);
        if (!result)
            return 0;
        fields = result->Fetch();
        uint32 map = fields[0].GetUInt32();
        float posx = fields[1].GetFloat();
        float posy = fields[2].GetFloat();
        float posz = fields[3].GetFloat();
        delete result;

        zone = sTerrainMgr.GetZoneId(map, posx, posy, posz);

        if (zone > 0)
            CharacterDatabase.PExecute("UPDATE characters SET zone='%u' WHERE guid='%u'", zone, lowguid);
    }

    return zone;
}

uint32 Player::GetLevelFromDB(ObjectGuid guid)
{
    uint32 lowguid = guid.GetCounter();

    QueryResult* result = CharacterDatabase.PQuery("SELECT level FROM characters WHERE guid='%u'", lowguid);
    if (!result)
        return 0;

    Field* fields = result->Fetch();
    uint32 level = fields[0].GetUInt32();
    delete result;

    return level;
}

void Player::UpdateArea(uint32 newArea)
{
    m_areaUpdateId    = newArea;

    AreaTableEntry const* area = GetAreaEntryByAreaID(newArea);

    // FFA_PVP flags are area and not zone id dependent
    // so apply them accordingly
    if (area && (area->flags & AREA_FLAG_ARENA))
    {
        if (!isGameMaster())
            SetFFAPvP(true);
    }
    else
    {
        // remove ffa flag only if not ffapvp realm
        // removal in sanctuaries and capitals is handled in zone update
        if (IsFFAPvP() && !sWorld.IsFFAPvPRealm())
            SetFFAPvP(false);
    }

    UpdateAreaDependentAuras();
}

bool Player::CanUseCapturePoint()
{
    return isAlive() &&                                     // living
           !HasStealthAura() &&                             // not stealthed
           !HasInvisibilityAura() &&                        // visible
           (IsPvP() || sWorld.IsPvPRealm()) &&
           !HasMovementFlag(MOVEFLAG_FLYING) &&
           !IsTaxiFlying() &&
           !isGameMaster();
}

void Player::UpdateZone(uint32 newZone, uint32 newArea)
{
    AreaTableEntry const* zone = GetAreaEntryByAreaID(newZone);
    if (!zone)
        return;

    if (m_zoneUpdateId != newZone)
    {
        // handle outdoor pvp zones
        sOutdoorPvPMgr.HandlePlayerLeaveZone(this, m_zoneUpdateId);
        sOutdoorPvPMgr.HandlePlayerEnterZone(this, newZone);

        SendInitWorldStates(newZone);                       // only if really enters to new zone, not just area change, works strange...

        if (sWorld.getConfig(CONFIG_BOOL_WEATHER))
        {
            if (Weather* wth = sWorld.FindWeather(zone->ID))
                wth->SendWeatherUpdateToPlayer(this);
            else if (!sWorld.AddWeather(zone->ID))
            {
                // send fine weather packet to remove old zone's weather
                Weather::SendFineWeatherUpdateToPlayer(this);
            }
        }
    }

    // used by eluna
    sHookMgr->OnUpdateZone(this, newZone, newArea);

    m_zoneUpdateId    = newZone;
    m_zoneUpdateTimer = ZONE_UPDATE_INTERVAL;

    // zone changed, so area changed as well, update it
    UpdateArea(newArea);

    // in PvP, any not controlled zone (except zone->team == 6, default case)
    // in PvE, only opposition team capital
    switch (zone->team)
    {
        case AREATEAM_ALLY:
            pvpInfo.inHostileArea = GetTeam() != ALLIANCE && (sWorld.IsPvPRealm() || zone->flags & AREA_FLAG_CAPITAL);
            break;
        case AREATEAM_HORDE:
            pvpInfo.inHostileArea = GetTeam() != HORDE && (sWorld.IsPvPRealm() || zone->flags & AREA_FLAG_CAPITAL);
            break;
        case AREATEAM_NONE:
            // overwrite for battlegrounds, maybe batter some zone flags but current known not 100% fit to this
            pvpInfo.inHostileArea = sWorld.IsPvPRealm() || InBattleGround();
            break;
        default:                                            // 6 in fact
            pvpInfo.inHostileArea = false;
            break;
    }

    if (pvpInfo.inHostileArea)                              // in hostile area
    {
        if (!IsPvP() || pvpInfo.endTimer != 0)
            UpdatePvP(true, true);
    }
    else                                                    // in friendly area
    {
        if (IsPvP() && !HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_IN_PVP) && pvpInfo.endTimer == 0)
            pvpInfo.endTimer = time(0);                     // start toggle-off
    }

    if (zone->flags & AREA_FLAG_CAPITAL)                    // in capital city
        SetRestType(REST_TYPE_IN_CITY);
    else if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) && GetRestType() != REST_TYPE_IN_TAVERN)
        // resting and not in tavern (leave city then); tavern leave handled in CheckAreaExploreAndOutdoor
        SetRestType(REST_TYPE_NO);

    // remove items with area/map limitations (delete only for alive player to allow back in ghost mode)
    // if player resurrected at teleport this will be applied in resurrect code
    if (isAlive())
        DestroyZoneLimitedItem(true, newZone);

    // recent client version not send leave/join channel packets for built-in local channels
    UpdateLocalChannels(newZone);

    // group update
    if (GetGroup())
        SetGroupUpdateFlag(GROUP_UPDATE_FLAG_ZONE);

    UpdateZoneDependentAuras();
}

// If players are too far way of duel flag... then player loose the duel
void Player::CheckDuelDistance(time_t currTime)
{
    if (!duel)
        return;

    GameObject* obj = GetMap()->GetGameObject(GetGuidValue(PLAYER_DUEL_ARBITER));
    if (!obj)
    {
        // player not at duel start map
        DuelComplete(DUEL_FLED);
        return;
    }

    if (duel->outOfBound == 0)
    {
        if (!IsWithinDistInMap(obj, 50))
        {
            duel->outOfBound = currTime;

            WorldPacket data(SMSG_DUEL_OUTOFBOUNDS, 0);
            GetSession()->SendPacket(&data);
        }
    }
    else
    {
        if (IsWithinDistInMap(obj, 40))
        {
            duel->outOfBound = 0;

            WorldPacket data(SMSG_DUEL_INBOUNDS, 0);
            GetSession()->SendPacket(&data);
        }
        else if (currTime >= (duel->outOfBound + 10))
        {
            DuelComplete(DUEL_FLED);
        }
    }
}

void Player::DuelComplete(DuelCompleteType type)
{
    // duel not requested
    if (!duel)
        return;

    WorldPacket data(SMSG_DUEL_COMPLETE, (1));
    data << (uint8)((type != DUEL_INTERRUPTED) ? 1 : 0);
    GetSession()->SendPacket(&data);
    duel->opponent->GetSession()->SendPacket(&data);

    if (type != DUEL_INTERRUPTED)
    {
        data.Initialize(SMSG_DUEL_WINNER, (1 + 20));        // we guess size
        data << (uint8)((type == DUEL_WON) ? 0 : 1);        // 0 = just won; 1 = fled
        data << duel->opponent->GetName();
        data << GetName();
        SendMessageToSet(&data, true);
    }

    // used by eluna
    sHookMgr->OnDuelEnd(duel->opponent, this, type);

    // Remove Duel Flag object
    if (GameObject* obj = GetMap()->GetGameObject(GetGuidValue(PLAYER_DUEL_ARBITER)))
        duel->initiator->RemoveGameObject(obj, true);

    /* remove auras */
    std::vector<uint32> auras2remove;
    SpellAuraHolderMap const& vAuras = duel->opponent->GetSpellAuraHolderMap();
    for (SpellAuraHolderMap::const_iterator i = vAuras.begin(); i != vAuras.end(); ++i)
    {
        if (!i->second->IsPositive() && i->second->GetCasterGuid() == GetObjectGuid() && i->second->GetAuraApplyTime() >= duel->startTime)
            auras2remove.push_back(i->second->GetId());
    }

    for (size_t i = 0; i < auras2remove.size(); ++i)
        duel->opponent->RemoveAurasDueToSpell(auras2remove[i]);

    auras2remove.clear();
    SpellAuraHolderMap const& auras = GetSpellAuraHolderMap();
    for (SpellAuraHolderMap::const_iterator i = auras.begin(); i != auras.end(); ++i)
    {
        if (!i->second->IsPositive() && i->second->GetCasterGuid() == duel->opponent->GetObjectGuid() && i->second->GetAuraApplyTime() >= duel->startTime)
            auras2remove.push_back(i->second->GetId());
    }
    for (size_t i = 0; i < auras2remove.size(); ++i)
        RemoveAurasDueToSpell(auras2remove[i]);

    // cleanup combo points
    if (GetComboTargetGuid() == duel->opponent->GetObjectGuid())
        ClearComboPoints();
    else if (GetComboTargetGuid() == duel->opponent->GetPetGuid())
        ClearComboPoints();

    if (duel->opponent->GetComboTargetGuid() == GetObjectGuid())
        duel->opponent->ClearComboPoints();
    else if (duel->opponent->GetComboTargetGuid() == GetPetGuid())
        duel->opponent->ClearComboPoints();

    // cleanups
    SetGuidValue(PLAYER_DUEL_ARBITER, ObjectGuid());
    SetUInt32Value(PLAYER_DUEL_TEAM, 0);
    duel->opponent->SetGuidValue(PLAYER_DUEL_ARBITER, ObjectGuid());
    duel->opponent->SetUInt32Value(PLAYER_DUEL_TEAM, 0);

    delete duel->opponent->duel;
    duel->opponent->duel = NULL;
    delete duel;
    duel = NULL;
}

//---------------------------------------------------------//

void Player::_ApplyItemMods(Item* item, uint8 slot, bool apply)
{
    if (slot >= INVENTORY_SLOT_BAG_END || !item)
        return;

    // not apply/remove mods for broken item
    if (item->IsBroken())
        return;

    ItemPrototype const* proto = item->GetProto();

    if (!proto)
        return;

    DETAIL_LOG("applying mods for item %u ", item->GetGUIDLow());

    uint32 attacktype = Player::GetAttackBySlot(slot);
    if (attacktype < MAX_ATTACK)
        _ApplyWeaponDependentAuraMods(item, WeaponAttackType(attacktype), apply);

    _ApplyItemBonuses(proto, slot, apply);

    if (slot == EQUIPMENT_SLOT_RANGED)
        _ApplyAmmoBonuses();

    ApplyItemEquipSpell(item, apply);
    ApplyEnchantment(item, apply);

    DEBUG_LOG("_ApplyItemMods complete.");
}

void Player::_ApplyItemBonuses(ItemPrototype const* proto, uint8 slot, bool apply)
{
    if (slot >= INVENTORY_SLOT_BAG_END || !proto)
        return;

    for (uint32 i = 0; i < MAX_ITEM_PROTO_STATS; ++i)
    {
        float val = float(proto->ItemStat[i].ItemStatValue);

        if (val == 0)
            continue;

        switch (proto->ItemStat[i].ItemStatType)
        {
            case ITEM_MOD_MANA:
                HandleStatModifier(UNIT_MOD_MANA, BASE_VALUE, float(val), apply);
                break;
            case ITEM_MOD_HEALTH:                           // modify HP
                HandleStatModifier(UNIT_MOD_HEALTH, BASE_VALUE, float(val), apply);
                break;
            case ITEM_MOD_AGILITY:                          // modify agility
                HandleStatModifier(UNIT_MOD_STAT_AGILITY, BASE_VALUE, float(val), apply);
                ApplyStatBuffMod(STAT_AGILITY, float(val), apply);
                break;
            case ITEM_MOD_STRENGTH:                         // modify strength
                HandleStatModifier(UNIT_MOD_STAT_STRENGTH, BASE_VALUE, float(val), apply);
                ApplyStatBuffMod(STAT_STRENGTH, float(val), apply);
                break;
            case ITEM_MOD_INTELLECT:                        // modify intellect
                HandleStatModifier(UNIT_MOD_STAT_INTELLECT, BASE_VALUE, float(val), apply);
                ApplyStatBuffMod(STAT_INTELLECT, float(val), apply);
                break;
            case ITEM_MOD_SPIRIT:                           // modify spirit
                HandleStatModifier(UNIT_MOD_STAT_SPIRIT, BASE_VALUE, float(val), apply);
                ApplyStatBuffMod(STAT_SPIRIT, float(val), apply);
                break;
            case ITEM_MOD_STAMINA:                          // modify stamina
                HandleStatModifier(UNIT_MOD_STAT_STAMINA, BASE_VALUE, float(val), apply);
                ApplyStatBuffMod(STAT_STAMINA, float(val), apply);
                break;
        }
    }

    if (proto->Armor)
        HandleStatModifier(UNIT_MOD_ARMOR, BASE_VALUE, float(proto->Armor), apply);

    if (proto->Block)
        HandleBaseModValue(SHIELD_BLOCK_VALUE, FLAT_MOD, float(proto->Block), apply);

    if (proto->HolyRes)
        HandleStatModifier(UNIT_MOD_RESISTANCE_HOLY, BASE_VALUE, float(proto->HolyRes), apply);

    if (proto->FireRes)
        HandleStatModifier(UNIT_MOD_RESISTANCE_FIRE, BASE_VALUE, float(proto->FireRes), apply);

    if (proto->NatureRes)
        HandleStatModifier(UNIT_MOD_RESISTANCE_NATURE, BASE_VALUE, float(proto->NatureRes), apply);

    if (proto->FrostRes)
        HandleStatModifier(UNIT_MOD_RESISTANCE_FROST, BASE_VALUE, float(proto->FrostRes), apply);

    if (proto->ShadowRes)
        HandleStatModifier(UNIT_MOD_RESISTANCE_SHADOW, BASE_VALUE, float(proto->ShadowRes), apply);

    if (proto->ArcaneRes)
        HandleStatModifier(UNIT_MOD_RESISTANCE_ARCANE, BASE_VALUE, float(proto->ArcaneRes), apply);

    WeaponAttackType attType = BASE_ATTACK;
    float damage = 0.0f;

    if (slot == EQUIPMENT_SLOT_RANGED && (
                proto->InventoryType == INVTYPE_RANGED || proto->InventoryType == INVTYPE_THROWN ||
                proto->InventoryType == INVTYPE_RANGEDRIGHT))
    {
        attType = RANGED_ATTACK;
    }
    else if (slot == EQUIPMENT_SLOT_OFFHAND)
    {
        attType = OFF_ATTACK;
    }

    if (proto->Damage[0].DamageMin > 0)
    {
        damage = apply ? proto->Damage[0].DamageMin : BASE_MINDAMAGE;
        SetBaseWeaponDamage(attType, MINDAMAGE, damage);
        // sLog.outError("applying mindam: assigning %f to weapon mindamage, now is: %f", damage, GetWeaponDamageRange(attType, MINDAMAGE));
    }

    if (proto->Damage[0].DamageMax  > 0)
    {
        damage = apply ? proto->Damage[0].DamageMax : BASE_MAXDAMAGE;
        SetBaseWeaponDamage(attType, MAXDAMAGE, damage);
    }

    if (!CanUseEquippedWeapon(attType))
        return;

    if (proto->Delay)
    {
        if (slot == EQUIPMENT_SLOT_RANGED)
            SetAttackTime(RANGED_ATTACK, apply ? proto->Delay : BASE_ATTACK_TIME);
        else if (slot == EQUIPMENT_SLOT_MAINHAND)
            SetAttackTime(BASE_ATTACK, apply ? proto->Delay : BASE_ATTACK_TIME);
        else if (slot == EQUIPMENT_SLOT_OFFHAND)
            SetAttackTime(OFF_ATTACK, apply ? proto->Delay : BASE_ATTACK_TIME);
    }

    if (CanModifyStats() && (damage || proto->Delay))
        UpdateDamagePhysical(attType);
}

void Player::_ApplyWeaponDependentAuraMods(Item* item, WeaponAttackType attackType, bool apply)
{
    AuraList const& auraCritList = GetAurasByType(SPELL_AURA_MOD_CRIT_PERCENT);
    for (AuraList::const_iterator itr = auraCritList.begin(); itr != auraCritList.end(); ++itr)
        _ApplyWeaponDependentAuraCritMod(item, attackType, *itr, apply);

    AuraList const& auraDamageFlatList = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE);
    for (AuraList::const_iterator itr = auraDamageFlatList.begin(); itr != auraDamageFlatList.end(); ++itr)
        _ApplyWeaponDependentAuraDamageMod(item, attackType, *itr, apply);

    AuraList const& auraDamagePCTList = GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
    for (AuraList::const_iterator itr = auraDamagePCTList.begin(); itr != auraDamagePCTList.end(); ++itr)
        _ApplyWeaponDependentAuraDamageMod(item, attackType, *itr, apply);
}

void Player::_ApplyWeaponDependentAuraCritMod(Item* item, WeaponAttackType attackType, Aura* aura, bool apply)
{
    // generic not weapon specific case processes in aura code
    if (aura->GetSpellProto()->EquippedItemClass == -1)
        return;

    BaseModGroup mod = BASEMOD_END;
    switch (attackType)
    {
        case BASE_ATTACK:   mod = CRIT_PERCENTAGE;        break;
        case OFF_ATTACK:    mod = OFFHAND_CRIT_PERCENTAGE; break;
        case RANGED_ATTACK: mod = RANGED_CRIT_PERCENTAGE; break;
        default: return;
    }

    if (item->IsFitToSpellRequirements(aura->GetSpellProto()))
    {
        HandleBaseModValue(mod, FLAT_MOD, float(aura->GetModifier()->m_amount), apply);
    }
}

void Player::_ApplyWeaponDependentAuraDamageMod(Item* item, WeaponAttackType attackType, Aura* aura, bool apply)
{
    // ignore spell mods for not wands
    Modifier const* modifier = aura->GetModifier();
    if ((modifier->m_miscvalue & SPELL_SCHOOL_MASK_NORMAL) == 0 && (getClassMask() & CLASSMASK_WAND_USERS) == 0)
        return;

    // generic not weapon specific case processes in aura code
    if (aura->GetSpellProto()->EquippedItemClass == -1)
        return;

    UnitMods unitMod = UNIT_MOD_END;
    switch (attackType)
    {
        case BASE_ATTACK:   unitMod = UNIT_MOD_DAMAGE_MAINHAND; break;
        case OFF_ATTACK:    unitMod = UNIT_MOD_DAMAGE_OFFHAND;  break;
        case RANGED_ATTACK: unitMod = UNIT_MOD_DAMAGE_RANGED;   break;
        default: return;
    }

    UnitModifierType unitModType = TOTAL_VALUE;
    switch (modifier->m_auraname)
    {
        case SPELL_AURA_MOD_DAMAGE_DONE:         unitModType = TOTAL_VALUE; break;
        case SPELL_AURA_MOD_DAMAGE_PERCENT_DONE: unitModType = TOTAL_PCT;   break;
        default: return;
    }

    if (item->IsFitToSpellRequirements(aura->GetSpellProto()))
    {
        HandleStatModifier(unitMod, unitModType, float(modifier->m_amount), apply);
    }
}

void Player::ApplyItemEquipSpell(Item* item, bool apply, bool form_change)
{
    if (!item)
        return;

    ItemPrototype const* proto = item->GetProto();
    if (!proto)
        return;

    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        _Spell const& spellData = proto->Spells[i];

        // no spell
        if (!spellData.SpellId)
            continue;

        if (apply)
        {
            // apply only at-equip spells
            if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_EQUIP)
                continue;
        }
        else
        {
            // at un-apply remove all spells (not only at-apply, so any at-use active affects from item and etc)
            // except with at-use with negative charges, so allow consuming item spells (including with extra flag that prevent consume really)
            // applied to player after item remove from equip slot
            if (spellData.SpellTrigger == ITEM_SPELLTRIGGER_ON_USE && spellData.SpellCharges < 0)
                continue;
        }

        // check if it is valid spell
        SpellEntry const* spellproto = sSpellStore.LookupEntry(spellData.SpellId);
        if (!spellproto)
            continue;

        ApplyEquipSpell(spellproto, item, apply, form_change);
    }
}

void Player::ApplyEquipSpell(SpellEntry const* spellInfo, Item* item, bool apply, bool form_change)
{
    if (apply)
    {
        // Cannot be used in this stance/form
        if (GetErrorAtShapeshiftedCast(spellInfo, GetShapeshiftForm()) != SPELL_CAST_OK)
            return;

        if (form_change)                                    // check aura active state from other form
        {
            bool found = false;
            for (int k = 0; k < MAX_EFFECT_INDEX; ++k)
            {
                SpellAuraHolderBounds spair = GetSpellAuraHolderBounds(spellInfo->Id);
                for (SpellAuraHolderMap::const_iterator iter = spair.first; iter != spair.second; ++iter)
                {
                    if (!item || iter->second->GetCastItemGuid() == item->GetObjectGuid())
                    {
                        found = true;
                        break;
                    }
                }
                if (found)
                    break;
            }

            if (found)                                      // and skip re-cast already active aura at form change
                return;
        }

        DEBUG_LOG("WORLD: cast %s Equip spellId - %i", (item ? "item" : "itemset"), spellInfo->Id);

        CastSpell(this, spellInfo, true, item);
    }
    else
    {
        if (form_change)                                    // check aura compatibility
        {
            // Cannot be used in this stance/form
            if (GetErrorAtShapeshiftedCast(spellInfo, GetShapeshiftForm()) == SPELL_CAST_OK)
                return;                                     // and remove only not compatible at form change
        }

        if (item)
            RemoveAurasDueToItemSpell(item, spellInfo->Id); // un-apply all spells , not only at-equipped
        else
            RemoveAurasDueToSpell(spellInfo->Id);           // un-apply spell (item set case)
    }
}

void Player::UpdateEquipSpellsAtFormChange()
{
    for (int i = 0; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (m_items[i] && !m_items[i]->IsBroken())
        {
            ApplyItemEquipSpell(m_items[i], false, true);   // remove spells that not fit to form
            ApplyItemEquipSpell(m_items[i], true, true);    // add spells that fit form but not active
        }
    }

    // item set bonuses not dependent from item broken state
    for (size_t setindex = 0; setindex < ItemSetEff.size(); ++setindex)
    {
        ItemSetEffect* eff = ItemSetEff[setindex];
        if (!eff)
            continue;

        for (uint32 y = 0; y < 8; ++y)
        {
            SpellEntry const* spellInfo = eff->spells[y];
            if (!spellInfo)
                continue;

            ApplyEquipSpell(spellInfo, NULL, false, true);  // remove spells that not fit to form
            ApplyEquipSpell(spellInfo, NULL, true, true);   // add spells that fit form but not active
        }
    }
}

void Player::CastItemCombatSpell(Unit* Target, WeaponAttackType attType)
{
    Item* item = GetWeaponForAttack(attType, true, true);
    if (!item)
        return;

    ItemPrototype const* proto = item->GetProto();
    if (!proto)
        return;

    if (!Target || Target == this)
        return;

    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        _Spell const& spellData = proto->Spells[i];

        // no spell
        if (!spellData.SpellId)
            continue;

        // wrong triggering type
        if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_CHANCE_ON_HIT)
            continue;

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(spellData.SpellId);
        if (!spellInfo)
        {
            sLog.outError("WORLD: unknown Item spellid %i", spellData.SpellId);
            continue;
        }

        // not allow proc extra attack spell at extra attack
        if (m_extraAttacks && IsSpellHaveEffect(spellInfo, SPELL_EFFECT_ADD_EXTRA_ATTACKS))
            return;

        float chance = (float)spellInfo->procChance;

        if (spellData.SpellPPMRate)
        {
            uint32 WeaponSpeed = proto->Delay;
            chance = GetPPMProcChance(WeaponSpeed, spellData.SpellPPMRate);
        }
        else if (chance > 100.0f)
        {
            chance = GetWeaponProcChance();
        }

        if (roll_chance_f(chance))
            CastSpell(Target, spellInfo->Id, true, item);
    }

    // item combat enchantments
    for (int e_slot = 0; e_slot < MAX_ENCHANTMENT_SLOT; ++e_slot)
    {
        uint32 enchant_id = item->GetEnchantmentId(EnchantmentSlot(e_slot));
        SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if (!pEnchant) continue;
        for (int s = 0; s < 3; ++s)
        {
            uint32 proc_spell_id = pEnchant->spellid[s];

            if (pEnchant->type[s] != ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL)
                continue;

            SpellEntry const* spellInfo = sSpellStore.LookupEntry(proc_spell_id);
            if (!spellInfo)
            {
                sLog.outError("Player::CastItemCombatSpell Enchant %i, cast unknown spell %i", pEnchant->ID, proc_spell_id);
                continue;
            }

            // Use first rank to access spell item enchant procs
            float ppmRate = sSpellMgr.GetItemEnchantProcChance(spellInfo->Id);

            float chance = ppmRate
                           ? GetPPMProcChance(proto->Delay, ppmRate)
                           : pEnchant->amount[s] != 0 ? float(pEnchant->amount[s]) : GetWeaponProcChance();


            ApplySpellMod(spellInfo->Id, SPELLMOD_CHANCE_OF_SUCCESS, chance);

            if (roll_chance_f(chance))
            {
                if (IsPositiveSpell(spellInfo->Id))
                    CastSpell(this, spellInfo->Id, true, item);
                else
                    CastSpell(Target, spellInfo->Id, true, item);
            }
        }
    }
}

void Player::CastItemUseSpell(Item* item, SpellCastTargets const& targets)
{
    ItemPrototype const* proto = item->GetProto();

    // use triggered flag only for items with many spell casts and for not first cast
    int count = 0;

    // item spells casted at use
    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        _Spell const& spellData = proto->Spells[i];

        // no spell
        if (!spellData.SpellId)
            continue;

        // wrong triggering type
        if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_USE && spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_NO_DELAY_USE)
            continue;

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(spellData.SpellId);
        if (!spellInfo)
        {
            sLog.outError("Player::CastItemUseSpell: Item (Entry: %u) in have wrong spell id %u, ignoring", proto->ItemId, spellData.SpellId);
            continue;
        }

        Spell* spell = new Spell(this, spellInfo, (count > 0));
        spell->m_CastItem = item;
        spell->prepare(&targets);

        ++count;
    }
}

void Player::_RemoveAllItemMods()
{
    DEBUG_LOG("_RemoveAllItemMods start.");

    for (int i = 0; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (m_items[i])
        {
            ItemPrototype const* proto = m_items[i]->GetProto();
            if (!proto)
                continue;

            // item set bonuses not dependent from item broken state
            if (proto->ItemSet)
                RemoveItemsSetItem(this, proto);

            if (m_items[i]->IsBroken())
                continue;

            ApplyItemEquipSpell(m_items[i], false);
            ApplyEnchantment(m_items[i], false);
        }
    }

    for (int i = 0; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (m_items[i])
        {
            if (m_items[i]->IsBroken())
                continue;
            ItemPrototype const* proto = m_items[i]->GetProto();
            if (!proto)
                continue;

            uint32 attacktype = Player::GetAttackBySlot(i);
            if (attacktype < MAX_ATTACK)
                _ApplyWeaponDependentAuraMods(m_items[i], WeaponAttackType(attacktype), false);

            _ApplyItemBonuses(proto, i, false);

            if (i == EQUIPMENT_SLOT_RANGED)
                _ApplyAmmoBonuses();
        }
    }

    DEBUG_LOG("_RemoveAllItemMods complete.");
}

void Player::_ApplyAllItemMods()
{
    DEBUG_LOG("_ApplyAllItemMods start.");

    for (int i = 0; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (m_items[i])
        {
            if (m_items[i]->IsBroken())
                continue;

            ItemPrototype const* proto = m_items[i]->GetProto();
            if (!proto)
                continue;

            uint32 attacktype = Player::GetAttackBySlot(i);
            if (attacktype < MAX_ATTACK)
                _ApplyWeaponDependentAuraMods(m_items[i], WeaponAttackType(attacktype), true);

            _ApplyItemBonuses(proto, i, true);

            if (i == EQUIPMENT_SLOT_RANGED)
                _ApplyAmmoBonuses();
        }
    }

    for (int i = 0; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (m_items[i])
        {
            ItemPrototype const* proto = m_items[i]->GetProto();
            if (!proto)
                continue;

            // item set bonuses not dependent from item broken state
            if (proto->ItemSet)
                AddItemsSetItem(this, m_items[i]);

            if (m_items[i]->IsBroken())
                continue;

            ApplyItemEquipSpell(m_items[i], true);
            ApplyEnchantment(m_items[i], true);
        }
    }

    DEBUG_LOG("_ApplyAllItemMods complete.");
}

void Player::_ApplyAmmoBonuses()
{
    // check ammo
    uint32 ammo_id = GetUInt32Value(PLAYER_AMMO_ID);
    if (!ammo_id)
        return;

    float currentAmmoDPS;

    ItemPrototype const* ammo_proto = ObjectMgr::GetItemPrototype(ammo_id);
    if (!ammo_proto || ammo_proto->Class != ITEM_CLASS_PROJECTILE || !CheckAmmoCompatibility(ammo_proto))
        currentAmmoDPS = 0.0f;
    else
        currentAmmoDPS = ammo_proto->Damage[0].DamageMin;

    if (currentAmmoDPS == GetAmmoDPS())
        return;

    m_ammoDPS = currentAmmoDPS;

    if (CanModifyStats())
        UpdateDamagePhysical(RANGED_ATTACK);
}

bool Player::CheckAmmoCompatibility(const ItemPrototype* ammo_proto) const
{
    if (!ammo_proto)
        return false;

    // check ranged weapon
    Item* weapon = GetWeaponForAttack(RANGED_ATTACK, true, false);
    if (!weapon)
        return false;

    ItemPrototype const* weapon_proto = weapon->GetProto();
    if (!weapon_proto || weapon_proto->Class != ITEM_CLASS_WEAPON)
        return false;

    // check ammo ws. weapon compatibility
    switch (weapon_proto->SubClass)
    {
        case ITEM_SUBCLASS_WEAPON_BOW:
        case ITEM_SUBCLASS_WEAPON_CROSSBOW:
            if (ammo_proto->SubClass != ITEM_SUBCLASS_ARROW)
                return false;
            break;
        case ITEM_SUBCLASS_WEAPON_GUN:
            if (ammo_proto->SubClass != ITEM_SUBCLASS_BULLET)
                return false;
            break;
        default:
            return false;
    }

    return true;
}

/*  If in a battleground a player dies, and an enemy removes the insignia, the player's bones is lootable
    Called by remove insignia spell effect    */
void Player::RemovedInsignia(Player* looterPlr)
{
    if (!GetBattleGroundId())
        return;

    // If not released spirit, do it !
    if (m_deathTimer > 0)
    {
        m_deathTimer = 0;
        BuildPlayerRepop();
        RepopAtGraveyard();
    }

    Corpse* corpse = GetCorpse();
    if (!corpse)
        return;

    // We have to convert player corpse to bones, not to be able to resurrect there
    // SpawnCorpseBones isn't handy, 'cos it saves player while he in BG
    Corpse* bones = sObjectAccessor.ConvertCorpseForPlayer(GetObjectGuid(), true);
    if (!bones)
        return;

    // Now we must make bones lootable, and send player loot
    bones->SetFlag(CORPSE_FIELD_DYNAMIC_FLAGS, CORPSE_DYNFLAG_LOOTABLE);

    // We store the level of our player in the gold field
    // We retrieve this information at Player::SendLoot()
    bones->loot.gold = getLevel();
    bones->lootRecipient = looterPlr;
    looterPlr->SendLoot(bones->GetObjectGuid(), LOOT_INSIGNIA);
}

void Player::SendLootRelease(ObjectGuid guid)
{
    WorldPacket data(SMSG_LOOT_RELEASE_RESPONSE, (8 + 1));
    data << guid;
    data << uint8(1);
    SendDirectMessage(&data);
}

void Player::SendLoot(ObjectGuid guid, LootType loot_type)
{
    if (ObjectGuid lootGuid = GetLootGuid())
        m_session->DoLootRelease(lootGuid);

    Loot* loot = NULL;
    PermissionTypes permission = ALL_PERMISSION;

    DEBUG_LOG("Player::SendLoot");
    switch (guid.GetHigh())
    {
        case HIGHGUID_GAMEOBJECT:
        {
            DEBUG_LOG("       IS_GAMEOBJECT_GUID(guid)");
            GameObject* go = GetMap()->GetGameObject(guid);

            // not check distance for GO in case owned GO (fishing bobber case, for example)
            // And permit out of range GO with no owner in case fishing hole
            if (!go || (loot_type != LOOT_FISHINGHOLE && ((loot_type != LOOT_FISHING && loot_type != LOOT_FISHING_FAIL) || go->GetOwnerGuid() != GetObjectGuid()) && !go->IsWithinDistInMap(this, INTERACTION_DISTANCE)))
            {
                SendLootRelease(guid);
                return;
            }

            loot = &go->loot;

            Player* recipient = go->GetLootRecipient();
            if (!recipient)
            {
                go->SetLootRecipient(this);
                recipient = this;
            }

            // generate loot only if ready for open and spawned in world
            if (go->getLootState() == GO_READY && go->isSpawned())
            {
                uint32 lootid =  go->GetGOInfo()->GetLootId();
                if ((go->GetEntry() == BG_AV_OBJECTID_MINE_N || go->GetEntry() == BG_AV_OBJECTID_MINE_S))
                {
                    if (BattleGround* bg = GetBattleGround())
                        if (bg->GetTypeID() == BATTLEGROUND_AV)
                            if (!(((BattleGroundAV*)bg)->PlayerCanDoMineQuest(go->GetEntry(), GetTeam())))
                            {
                                SendLootRelease(guid);
                                return;
                            }
                }

                loot->clear();
                switch (loot_type)
                {
                    // Entry 0 in fishing loot template used for store junk fish loot at fishing fail it junk allowed by config option
                    // this is overwrite fishinghole loot for example
                    case LOOT_FISHING_FAIL:
                        loot->FillLoot(0, LootTemplates_Fishing, this, true);
                        break;
                    case LOOT_FISHING:
                        uint32 zone, subzone;
                        go->GetZoneAndAreaId(zone, subzone);
                        // if subzone loot exist use it
                        if (!loot->FillLoot(subzone, LootTemplates_Fishing, this, true, (subzone != zone)) && subzone != zone)
                            // else use zone loot (if zone diff. from subzone, must exist in like case)
                            loot->FillLoot(zone, LootTemplates_Fishing, this, true);
                        break;
                    default:
                        if (!lootid)
                            break;
                        DEBUG_LOG("       send normal GO loot");

                        loot->FillLoot(lootid, LootTemplates_Gameobject, this, false);
                        loot->generateMoneyLoot(go->GetGOInfo()->MinMoneyLoot, go->GetGOInfo()->MaxMoneyLoot);

                        if (go->GetGoType() == GAMEOBJECT_TYPE_CHEST && go->GetGOInfo()->chest.groupLootRules)
                        {
                            if (Group* group = go->GetGroupLootRecipient())
                            {
                                group->UpdateLooterGuid(go, true);

                                switch (group->GetLootMethod())
                                {
                                    case GROUP_LOOT:
                                        // GroupLoot delete items over threshold (threshold even not implemented), and roll them. Items with quality<threshold, round robin
                                        group->GroupLoot(go, loot);
                                        permission = GROUP_PERMISSION;
                                        break;
                                    case NEED_BEFORE_GREED:
                                        group->NeedBeforeGreed(go, loot);
                                        permission = GROUP_PERMISSION;
                                        break;
                                    case MASTER_LOOT:
                                        group->MasterLoot(go, loot);
                                        permission = MASTER_PERMISSION;
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }
                        break;
                }

                go->SetLootState(GO_ACTIVATED);
            }
            if (go->getLootState() == GO_ACTIVATED && go->GetGoType() == GAMEOBJECT_TYPE_CHEST && go->GetGOInfo()->chest.groupLootRules)
            {
                if (Group* group = go->GetGroupLootRecipient())
                {
                    if (group == GetGroup())
                    {
                        if (group->GetLootMethod() == FREE_FOR_ALL)
                            permission = ALL_PERMISSION;
                        else if (group->GetLooterGuid() == GetObjectGuid())
                        {
                            if (group->GetLootMethod() == MASTER_LOOT)
                                permission = MASTER_PERMISSION;
                            else
                                permission = ALL_PERMISSION;
                        }
                        else
                            permission = GROUP_PERMISSION;
                    }
                    else
                        permission = NONE_PERMISSION;
                }
                else if (recipient == this)
                    permission = ALL_PERMISSION;
                else
                    permission = NONE_PERMISSION;
            }
            break;
        }
        case HIGHGUID_ITEM:
        {
            Item* item = GetItemByGuid(guid);

            if (!item)
            {
                SendLootRelease(guid);
                return;
            }

            permission = OWNER_PERMISSION;

            loot = &item->loot;

            if (!item->HasGeneratedLoot())
            {
                item->loot.clear();

                switch (loot_type)
                {
                    case LOOT_DISENCHANTING:
                        loot->FillLoot(item->GetProto()->DisenchantID, LootTemplates_Disenchant, this, true);
                        item->SetLootState(ITEM_LOOT_TEMPORARY);
                        break;
                    default:
                        loot->FillLoot(item->GetEntry(), LootTemplates_Item, this, true, item->GetProto()->MaxMoneyLoot == 0);
                        loot->generateMoneyLoot(item->GetProto()->MinMoneyLoot, item->GetProto()->MaxMoneyLoot);
                        item->SetLootState(ITEM_LOOT_CHANGED);
                        break;
                }
            }
            break;
        }
        case HIGHGUID_CORPSE:                               // remove insignia
        {
            Corpse* bones = GetMap()->GetCorpse(guid);

            if (!bones || !((loot_type == LOOT_CORPSE) || (loot_type == LOOT_INSIGNIA)) || (bones->GetType() != CORPSE_BONES))
            {
                SendLootRelease(guid);
                return;
            }

            loot = &bones->loot;

            if (!bones->lootForBody)
            {
                bones->lootForBody = true;
                uint32 pLevel = bones->loot.gold;
                bones->loot.clear();
                if (GetBattleGround()->GetTypeID() == BATTLEGROUND_AV)
                    loot->FillLoot(0, LootTemplates_Creature, this, false);
                // It may need a better formula
                // Now it works like this: lvl10: ~6copper, lvl70: ~9silver
                bones->loot.gold = (uint32)(urand(50, 150) * 0.016f * pow(((float)pLevel) / 5.76f, 2.5f) * sWorld.getConfig(CONFIG_FLOAT_RATE_DROP_MONEY));
            }

            if (bones->lootRecipient != this)
                permission = NONE_PERMISSION;
            else
                permission = OWNER_PERMISSION;
            break;
        }
        case HIGHGUID_UNIT:
        {
            Creature* creature = GetMap()->GetCreature(guid);

            // must be in range and creature must be alive for pickpocket and must be dead for another loot
            if (!creature || creature->isAlive() != (loot_type == LOOT_PICKPOCKETING) || !creature->IsWithinDistInMap(this, INTERACTION_DISTANCE))
            {
                SendLootRelease(guid);
                return;
            }

            if (loot_type == LOOT_PICKPOCKETING && IsFriendlyTo(creature))
            {
                SendLootRelease(guid);
                return;
            }

            loot   = &creature->loot;

            if (loot_type == LOOT_PICKPOCKETING)
            {
                if (!creature->lootForPickPocketed)
                {
                    creature->lootForPickPocketed = true;
                    loot->clear();

                    if (uint32 lootid = creature->GetCreatureInfo()->PickpocketLootId)
                        loot->FillLoot(lootid, LootTemplates_Pickpocketing, this, false);

                    // Generate extra money for pick pocket loot
                    const uint32 a = urand(0, creature->getLevel() / 2);
                    const uint32 b = urand(0, getLevel() / 2);
                    loot->gold = uint32(10 * (a + b) * sWorld.getConfig(CONFIG_FLOAT_RATE_DROP_MONEY));
                    permission = OWNER_PERMISSION;
                }
            }
            else
            {
                // the player whose group may loot the corpse
                Player* recipient = creature->GetLootRecipient();
                if (!recipient)
                {
                    creature->SetLootRecipient(this);
                    recipient = this;
                }

                if (creature->lootForPickPocketed)
                {
                    creature->lootForPickPocketed = false;
                    loot->clear();
                }

                if (!creature->lootForBody)
                {
                    creature->lootForBody = true;
                    loot->clear();

                    if (uint32 lootid = creature->GetCreatureInfo()->LootId)
                        loot->FillLoot(lootid, LootTemplates_Creature, recipient, false);

                    loot->generateMoneyLoot(creature->GetCreatureInfo()->MinLootGold, creature->GetCreatureInfo()->MaxLootGold);

                    if (Group* group = creature->GetGroupLootRecipient())
                    {
                        group->UpdateLooterGuid(creature, true);

                        switch (group->GetLootMethod())
                        {
                            case GROUP_LOOT:
                                // GroupLoot delete items over threshold (threshold even not implemented), and roll them. Items with quality<threshold, round robin
                                group->GroupLoot(creature, loot);
                                break;
                            case NEED_BEFORE_GREED:
                                group->NeedBeforeGreed(creature, loot);
                                break;
                            case MASTER_LOOT:
                                group->MasterLoot(creature, loot);
                                break;
                            default:
                                break;
                        }
                    }
                }

                // possible only if creature->lootForBody && loot->empty() at spell cast check
                if (loot_type == LOOT_SKINNING)
                {
                    if (!creature->lootForSkin)
                    {
                        creature->lootForSkin = true;
                        loot->clear();
                        loot->FillLoot(creature->GetCreatureInfo()->SkinningLootId, LootTemplates_Skinning, this, false);

                        // let reopen skinning loot if will closed.
                        if (!loot->empty())
                            creature->SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);

                        permission = OWNER_PERMISSION;
                    }
                }
                // set group rights only for loot_type != LOOT_SKINNING
                else
                {
                    if (Group* group = creature->GetGroupLootRecipient())
                    {
                        if (group == GetGroup())
                        {
                            if (group->GetLootMethod() == FREE_FOR_ALL)
                                permission = ALL_PERMISSION;
                            else if (group->GetLooterGuid() == GetObjectGuid())
                            {
                                if (group->GetLootMethod() == MASTER_LOOT)
                                    permission = MASTER_PERMISSION;
                                else
                                    permission = ALL_PERMISSION;
                            }
                            else
                                permission = GROUP_PERMISSION;
                        }
                        else
                            permission = NONE_PERMISSION;
                    }
                    else if (recipient == this)
                        permission = OWNER_PERMISSION;
                    else
                        permission = NONE_PERMISSION;
                }
            }
            break;
        }
        default:
        {
            sLog.outError("%s is unsupported for looting.", guid.GetString().c_str());
            return;
        }
    }

    SetLootGuid(guid);

    // need know for proper finish item loots (internal pre-switch loot type set in different from 3.x code version)
    // in fact this meaning that it send same loot types for interesting cases like 3.x version code (skip pre-3.x client loot type limitaitons)
    loot->loot_type = loot_type;

    // LOOT_SKINNING, LOOT_PROSPECTING, LOOT_INSIGNIA and LOOT_FISHINGHOLE unsupported by client
    switch (loot_type)
    {
        case LOOT_SKINNING:     loot_type = LOOT_PICKPOCKETING; break;
        case LOOT_INSIGNIA:     loot_type = LOOT_PICKPOCKETING; break;
        case LOOT_FISHING_FAIL: loot_type = LOOT_FISHING;       break;
        case LOOT_FISHINGHOLE:  loot_type = LOOT_FISHING;       break;
        default: break;
    }

    WorldPacket data(SMSG_LOOT_RESPONSE, (9 + 50));         // we guess size
    data << ObjectGuid(guid);
    data << uint8(loot_type);
    data << LootView(*loot, this, permission);
    SendDirectMessage(&data);

    // add 'this' player as one of the players that are looting 'loot'
    if (permission != NONE_PERMISSION)
        loot->AddLooter(GetObjectGuid());

    if (loot_type == LOOT_CORPSE && !guid.IsItem())
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOOTING);
}

void Player::SendNotifyLootMoneyRemoved()
{
    WorldPacket data(SMSG_LOOT_CLEAR_MONEY, 0);
    GetSession()->SendPacket(&data);
}

void Player::SendNotifyLootItemRemoved(uint8 lootSlot)
{
    WorldPacket data(SMSG_LOOT_REMOVED, 1);
    data << uint8(lootSlot);
    GetSession()->SendPacket(&data);
}

void Player::SendUpdateWorldState(uint32 Field, uint32 Value)
{
    WorldPacket data(SMSG_UPDATE_WORLD_STATE, 8);
    data << Field;
    data << Value;
    GetSession()->SendPacket(&data);
}

void Player::SendInitWorldStates(uint32 zoneid)
{
    // data depends on zoneid/mapid...
    BattleGround* bg = GetBattleGround();
    uint32 mapid = GetMapId();

    DEBUG_LOG("Sending SMSG_INIT_WORLD_STATES to Map:%u, Zone: %u", mapid, zoneid);

    uint32 count = 0;                                       // count of world states in packet

    WorldPacket data(SMSG_INIT_WORLD_STATES, (4 + 4 + 2 + 6));
    data << uint32(mapid);                              // mapid
    data << uint32(zoneid);                             // zone id
    size_t count_pos = data.wpos();
    data << uint16(0);                                  // count of uint64 blocks, placeholder

    switch (zoneid)
    {
        case 139:                                       // Eastern Plaguelands
        case 1377:                                      // Silithus
            if (OutdoorPvP* outdoorPvP = sOutdoorPvPMgr.GetScript(zoneid))
                outdoorPvP->FillInitialWorldStates(data, count);
            break;
        case 2597:                                      // AV
            if (bg && bg->GetTypeID() == BATTLEGROUND_AV)
                bg->FillInitialWorldStates(data, count);
            break;
        case 3277:                                      // WS
            if (bg && bg->GetTypeID() == BATTLEGROUND_WS)
                bg->FillInitialWorldStates(data, count);
            break;
        case 3358:                                      // AB
            if (bg && bg->GetTypeID() == BATTLEGROUND_AB)
                bg->FillInitialWorldStates(data, count);
            break;
    }

    data.put<uint16>(count_pos, count);                 // set actual world state amount

    GetSession()->SendPacket(&data);
}

uint32 Player::GetXPRestBonus(uint32 xp)
{
    uint32 rested_bonus = (uint32)GetRestBonus();           // xp for each rested bonus

    if (rested_bonus > xp)                                  // max rested_bonus == xp or (r+x) = 200% xp
        rested_bonus = xp;

    SetRestBonus(GetRestBonus() - rested_bonus);

    DETAIL_LOG("Player gain %u xp (+ %u Rested Bonus). Rested points=%f", xp + rested_bonus, rested_bonus, GetRestBonus());
    return rested_bonus;
}

void Player::SetBindPoint(ObjectGuid guid)
{
    WorldPacket data(SMSG_BINDER_CONFIRM, 8);
    data << ObjectGuid(guid);
    GetSession()->SendPacket(&data);
}

void Player::SendTalentWipeConfirm(ObjectGuid guid)
{
    WorldPacket data(MSG_TALENT_WIPE_CONFIRM, (8 + 4));
    data << ObjectGuid(guid);
    data << uint32(resetTalentsCost());
    GetSession()->SendPacket(&data);
}

void Player::SendPetSkillWipeConfirm()
{
    Pet* pet = GetPet();
    if (!pet)
        return;
    WorldPacket data(SMSG_PET_UNLEARN_CONFIRM, (8 + 4));
    data << ObjectGuid(pet->GetObjectGuid());
    data << uint32(pet->resetTalentsCost());
    GetSession()->SendPacket(&data);
}

/*********************************************************/
/***                    STORAGE SYSTEM                 ***/
/*********************************************************/

void Player::SetVirtualItemSlot(uint8 i, Item* item)
{
    MANGOS_ASSERT(i < 3);
    if (i < 2 && item)
    {
        if (!item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
            return;
        uint32 charges = item->GetEnchantmentCharges(TEMP_ENCHANTMENT_SLOT);
        if (charges == 0)
            return;
        if (charges > 1)
            item->SetEnchantmentCharges(TEMP_ENCHANTMENT_SLOT, charges - 1);
        else if (charges <= 1)
        {
            ApplyEnchantment(item, TEMP_ENCHANTMENT_SLOT, false);
            item->ClearEnchantment(TEMP_ENCHANTMENT_SLOT);
        }
    }
}

void Player::SetSheath(SheathState sheathed)
{
    switch (sheathed)
    {
        case SHEATH_STATE_UNARMED:                          // no prepared weapon
            SetVirtualItemSlot(0, NULL);
            SetVirtualItemSlot(1, NULL);
            SetVirtualItemSlot(2, NULL);
            break;
        case SHEATH_STATE_MELEE:                            // prepared melee weapon
        {
            SetVirtualItemSlot(0, GetWeaponForAttack(BASE_ATTACK, true, true));
            SetVirtualItemSlot(1, GetWeaponForAttack(OFF_ATTACK, true, true));
            SetVirtualItemSlot(2, NULL);
        };  break;
        case SHEATH_STATE_RANGED:                           // prepared ranged weapon
            SetVirtualItemSlot(0, NULL);
            SetVirtualItemSlot(1, NULL);
            SetVirtualItemSlot(2, GetWeaponForAttack(RANGED_ATTACK, true, true));
            break;
        default:
            SetVirtualItemSlot(0, NULL);
            SetVirtualItemSlot(1, NULL);
            SetVirtualItemSlot(2, NULL);
            break;
    }
    Unit::SetSheath(sheathed);                              // this must visualize Sheath changing for other players...
}

uint8 Player::FindEquipSlot(ItemPrototype const* proto, uint32 slot, bool swap) const
{
    uint8 pClass = getClass();

    uint8 slots[4];
    slots[0] = NULL_SLOT;
    slots[1] = NULL_SLOT;
    slots[2] = NULL_SLOT;
    slots[3] = NULL_SLOT;
    switch (proto->InventoryType)
    {
        case INVTYPE_HEAD:
            slots[0] = EQUIPMENT_SLOT_HEAD;
            break;
        case INVTYPE_NECK:
            slots[0] = EQUIPMENT_SLOT_NECK;
            break;
        case INVTYPE_SHOULDERS:
            slots[0] = EQUIPMENT_SLOT_SHOULDERS;
            break;
        case INVTYPE_BODY:
            slots[0] = EQUIPMENT_SLOT_BODY;
            break;
        case INVTYPE_CHEST:
            slots[0] = EQUIPMENT_SLOT_CHEST;
            break;
        case INVTYPE_ROBE:
            slots[0] = EQUIPMENT_SLOT_CHEST;
            break;
        case INVTYPE_WAIST:
            slots[0] = EQUIPMENT_SLOT_WAIST;
            break;
        case INVTYPE_LEGS:
            slots[0] = EQUIPMENT_SLOT_LEGS;
            break;
        case INVTYPE_FEET:
            slots[0] = EQUIPMENT_SLOT_FEET;
            break;
        case INVTYPE_WRISTS:
            slots[0] = EQUIPMENT_SLOT_WRISTS;
            break;
        case INVTYPE_HANDS:
            slots[0] = EQUIPMENT_SLOT_HANDS;
            break;
        case INVTYPE_FINGER:
            slots[0] = EQUIPMENT_SLOT_FINGER1;
            slots[1] = EQUIPMENT_SLOT_FINGER2;
            break;
        case INVTYPE_TRINKET:
            slots[0] = EQUIPMENT_SLOT_TRINKET1;
            slots[1] = EQUIPMENT_SLOT_TRINKET2;
            break;
        case INVTYPE_CLOAK:
            slots[0] =  EQUIPMENT_SLOT_BACK;
            break;
        case INVTYPE_WEAPON:
        {
            slots[0] = EQUIPMENT_SLOT_MAINHAND;

            // suggest offhand slot only if know dual wielding
            // (this will be replace mainhand weapon at auto equip instead unwonted "you don't known dual wielding" ...
            if (CanDualWield())
                slots[1] = EQUIPMENT_SLOT_OFFHAND;
            break;
        };
        case INVTYPE_SHIELD:
            slots[0] = EQUIPMENT_SLOT_OFFHAND;
            break;
        case INVTYPE_RANGED:
            slots[0] = EQUIPMENT_SLOT_RANGED;
            break;
        case INVTYPE_2HWEAPON:
            slots[0] = EQUIPMENT_SLOT_MAINHAND;
            break;
        case INVTYPE_TABARD:
            slots[0] = EQUIPMENT_SLOT_TABARD;
            break;
        case INVTYPE_WEAPONMAINHAND:
            slots[0] = EQUIPMENT_SLOT_MAINHAND;
            break;
        case INVTYPE_WEAPONOFFHAND:
            slots[0] = EQUIPMENT_SLOT_OFFHAND;
            break;
        case INVTYPE_HOLDABLE:
            slots[0] = EQUIPMENT_SLOT_OFFHAND;
            break;
        case INVTYPE_THROWN:
            slots[0] = EQUIPMENT_SLOT_RANGED;
            break;
        case INVTYPE_RANGEDRIGHT:
            slots[0] = EQUIPMENT_SLOT_RANGED;
            break;
        case INVTYPE_BAG:
            slots[0] = INVENTORY_SLOT_BAG_START + 0;
            slots[1] = INVENTORY_SLOT_BAG_START + 1;
            slots[2] = INVENTORY_SLOT_BAG_START + 2;
            slots[3] = INVENTORY_SLOT_BAG_START + 3;
            break;
        case INVTYPE_RELIC:
        {
            switch (proto->SubClass)
            {
                case ITEM_SUBCLASS_ARMOR_LIBRAM:
                    if (pClass == CLASS_PALADIN)
                        slots[0] = EQUIPMENT_SLOT_RANGED;
                    break;
                case ITEM_SUBCLASS_ARMOR_IDOL:
                    if (pClass == CLASS_DRUID)
                        slots[0] = EQUIPMENT_SLOT_RANGED;
                    break;
                case ITEM_SUBCLASS_ARMOR_TOTEM:
                    if (pClass == CLASS_SHAMAN)
                        slots[0] = EQUIPMENT_SLOT_RANGED;
                    break;
                case ITEM_SUBCLASS_ARMOR_MISC:
                    if (pClass == CLASS_WARLOCK)
                        slots[0] = EQUIPMENT_SLOT_RANGED;
                    break;
            }
            break;
        }
        default :
            return NULL_SLOT;
    }

    if (slot != NULL_SLOT)
    {
        if (swap || !GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            for (int i = 0; i < 4; ++i)
            {
                if (slots[i] == slot)
                    return slot;
            }
        }
    }
    else
    {
        // search free slot at first
        for (int i = 0; i < 4; ++i)
        {
            if (slots[i] != NULL_SLOT && !GetItemByPos(INVENTORY_SLOT_BAG_0, slots[i]))
            {
                // in case 2hand equipped weapon offhand slot empty but not free
                if (slots[i] != EQUIPMENT_SLOT_OFFHAND || !IsTwoHandUsed())
                    return slots[i];
            }
        }

        // if not found free and can swap return first appropriate from used
        for (int i = 0; i < 4; ++i)
        {
            if (slots[i] != NULL_SLOT && swap)
                return slots[i];
        }
    }

    // no free position
    return NULL_SLOT;
}

InventoryResult Player::CanUnequipItems(uint32 item, uint32 count) const
{
    Item* pItem;
    uint32 tempcount = 0;

    InventoryResult res = EQUIP_ERR_OK;

    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
        {
            InventoryResult ires = CanUnequipItem(INVENTORY_SLOT_BAG_0 << 8 | i, false);
            if (ires == EQUIP_ERR_OK)
            {
                tempcount += pItem->GetCount();
                if (tempcount >= count)
                    return EQUIP_ERR_OK;
            }
            else
                res = ires;
        }
    }
    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
        {
            tempcount += pItem->GetCount();
            if (tempcount >= count)
                return EQUIP_ERR_OK;
        }
    }
    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
    {
        pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
        {
            tempcount += pItem->GetCount();
            if (tempcount >= count)
                return EQUIP_ERR_OK;
        }
    }
    Bag* pBag;
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pBag)
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
            {
                pItem = GetItemByPos(i, j);
                if (pItem && pItem->GetEntry() == item)
                {
                    tempcount += pItem->GetCount();
                    if (tempcount >= count)
                        return EQUIP_ERR_OK;
                }
            }
        }
    }

    // not found req. item count and have unequippable items
    return res;
}

uint32 Player::GetItemCount(uint32 item, bool inBankAlso, Item* skipItem) const
{
    uint32 count = 0;
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem != skipItem &&  pItem->GetEntry() == item)
            count += pItem->GetCount();
    }
    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
    {
        Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem != skipItem && pItem->GetEntry() == item)
            count += pItem->GetCount();
    }
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pBag)
            count += pBag->GetItemCount(item, skipItem);
    }

    if (inBankAlso)
    {
        for (int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
        {
            Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (pItem && pItem != skipItem && pItem->GetEntry() == item)
                count += pItem->GetCount();
        }
        for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        {
            Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (pBag)
                count += pBag->GetItemCount(item, skipItem);
        }
    }

    return count;
}

Item* Player::GetItemByEntry(uint32 item) const
{
     for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (pItem->GetEntry() == item)
                return pItem;

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (Item* itemPtr = pBag->GetItemByEntry(item))
                return itemPtr;

    return NULL;
}

Item* Player::GetItemByGuid(ObjectGuid guid) const
{
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (pItem->GetObjectGuid() == guid)
                return pItem;

    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (pItem->GetObjectGuid() == guid)
                return pItem;

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                if (Item* pItem = pBag->GetItemByPos(j))
                    if (pItem->GetObjectGuid() == guid)
                        return pItem;

    for (int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (pItem->GetObjectGuid() == guid)
                return pItem;

    for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                if (Item* pItem = pBag->GetItemByPos(j))
                    if (pItem->GetObjectGuid() == guid)
                        return pItem;

    return NULL;
}

Item* Player::GetItemByPos(uint16 pos) const
{
    uint8 bag = pos >> 8;
    uint8 slot = pos & 255;
    return GetItemByPos(bag, slot);
}

Item* Player::GetItemByPos(uint8 bag, uint8 slot) const
{
    if (bag == INVENTORY_SLOT_BAG_0 && (slot < BANK_SLOT_BAG_END || (slot >= KEYRING_SLOT_START && slot < KEYRING_SLOT_END)))
        return m_items[slot];
    else if ((bag >= INVENTORY_SLOT_BAG_START && bag < INVENTORY_SLOT_BAG_END)
             || (bag >= BANK_SLOT_BAG_START && bag < BANK_SLOT_BAG_END))
    {
        Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
        if (pBag)
            return pBag->GetItemByPos(slot);
    }
    return NULL;
}

Item* Player::GetWeaponForAttack(WeaponAttackType attackType, bool nonbroken, bool useable) const
{
    uint8 slot;
    switch (attackType)
    {
        case BASE_ATTACK:   slot = EQUIPMENT_SLOT_MAINHAND; break;
        case OFF_ATTACK:    slot = EQUIPMENT_SLOT_OFFHAND;  break;
        case RANGED_ATTACK: slot = EQUIPMENT_SLOT_RANGED;   break;
        default: return NULL;
    }

    Item* item = GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
    if (!item || item->GetProto()->Class != ITEM_CLASS_WEAPON)
        return NULL;

    if (useable && !CanUseEquippedWeapon(attackType))
        return NULL;

    if (nonbroken && item->IsBroken())
        return NULL;

    return item;
}

Item* Player::GetShield(bool useable) const
{
    Item* item = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!item || item->GetProto()->Class != ITEM_CLASS_ARMOR)
        return NULL;

    if (!useable)
        return item;

    if (item->IsBroken() || !CanUseEquippedWeapon(OFF_ATTACK))
        return NULL;

    return item;
}

uint32 Player::GetAttackBySlot(uint8 slot)
{
    switch (slot)
    {
        case EQUIPMENT_SLOT_MAINHAND: return BASE_ATTACK;
        case EQUIPMENT_SLOT_OFFHAND:  return OFF_ATTACK;
        case EQUIPMENT_SLOT_RANGED:   return RANGED_ATTACK;
        default:                      return MAX_ATTACK;
    }
}

bool Player::IsInventoryPos(uint8 bag, uint8 slot)
{
    if (bag == INVENTORY_SLOT_BAG_0 && slot == NULL_SLOT)
        return true;
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= INVENTORY_SLOT_ITEM_START && slot < INVENTORY_SLOT_ITEM_END))
        return true;
    if (bag >= INVENTORY_SLOT_BAG_START && bag < INVENTORY_SLOT_BAG_END)
        return true;
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= KEYRING_SLOT_START && slot < KEYRING_SLOT_END))
        return true;
    return false;
}

bool Player::IsEquipmentPos(uint8 bag, uint8 slot)
{
    if (bag == INVENTORY_SLOT_BAG_0 && (slot < EQUIPMENT_SLOT_END))
        return true;
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END))
        return true;
    return false;
}

bool Player::IsBankPos(uint8 bag, uint8 slot)
{
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= BANK_SLOT_ITEM_START && slot < BANK_SLOT_ITEM_END))
        return true;
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END))
        return true;
    if (bag >= BANK_SLOT_BAG_START && bag < BANK_SLOT_BAG_END)
        return true;
    return false;
}

bool Player::IsBagPos(uint16 pos)
{
    uint8 bag = pos >> 8;
    uint8 slot = pos & 255;
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END))
        return true;
    if (bag == INVENTORY_SLOT_BAG_0 && (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END))
        return true;
    return false;
}

bool Player::IsValidPos(uint8 bag, uint8 slot, bool explicit_pos) const
{
    // post selected
    if (bag == NULL_BAG && !explicit_pos)
        return true;

    if (bag == INVENTORY_SLOT_BAG_0)
    {
        // any post selected
        if (slot == NULL_SLOT && !explicit_pos)
            return true;

        // equipment
        if (slot < EQUIPMENT_SLOT_END)
            return true;

        // bag equip slots
        if (slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END)
            return true;

        // backpack slots
        if (slot >= INVENTORY_SLOT_ITEM_START && slot < INVENTORY_SLOT_ITEM_END)
            return true;

        // keyring slots
        if (slot >= KEYRING_SLOT_START && slot < KEYRING_SLOT_END)
            return true;

        // bank main slots
        if (slot >= BANK_SLOT_ITEM_START && slot < BANK_SLOT_ITEM_END)
            return true;

        // bank bag slots
        if (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END)
            return true;

        return false;
    }

    // bag content slots
    if (bag >= INVENTORY_SLOT_BAG_START && bag < INVENTORY_SLOT_BAG_END)
    {
        Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
        if (!pBag)
            return false;

        // any post selected
        if (slot == NULL_SLOT && !explicit_pos)
            return true;

        return slot < pBag->GetBagSize();
    }

    // bank bag content slots
    if (bag >= BANK_SLOT_BAG_START && bag < BANK_SLOT_BAG_END)
    {
        Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
        if (!pBag)
            return false;

        // any post selected
        if (slot == NULL_SLOT && !explicit_pos)
            return true;

        return slot < pBag->GetBagSize();
    }

    // where this?
    return false;
}


bool Player::HasItemCount(uint32 item, uint32 count, bool inBankAlso) const
{
    uint32 tempcount = 0;
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item && !pItem->IsInTrade())
        {
            tempcount += pItem->GetCount();
            if (tempcount >= count)
                return true;
        }
    }
    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
    {
        Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item && !pItem->IsInTrade())
        {
            tempcount += pItem->GetCount();
            if (tempcount >= count)
                return true;
        }
    }
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
            {
                Item* pItem = GetItemByPos(i, j);
                if (pItem && pItem->GetEntry() == item && !pItem->IsInTrade())
                {
                    tempcount += pItem->GetCount();
                    if (tempcount >= count)
                        return true;
                }
            }
        }
    }

    if (inBankAlso)
    {
        for (int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
        {
            Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (pItem && pItem->GetEntry() == item && !pItem->IsInTrade())
            {
                tempcount += pItem->GetCount();
                if (tempcount >= count)
                    return true;
            }
        }
        for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        {
            if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                {
                    Item* pItem = GetItemByPos(i, j);
                    if (pItem && pItem->GetEntry() == item && !pItem->IsInTrade())
                    {
                        tempcount += pItem->GetCount();
                        if (tempcount >= count)
                            return true;
                    }
                }
            }
        }
    }

    return false;
}

bool Player::HasItemWithIdEquipped(uint32 item, uint32 count, uint8 except_slot) const
{
    uint32 tempcount = 0;
    for (int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        if (i == int(except_slot))
            continue;

        Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i);
        if (pItem && pItem->GetEntry() == item)
        {
            tempcount += pItem->GetCount();
            if (tempcount >= count)
                return true;
        }
    }

    return false;
}

InventoryResult Player::_CanTakeMoreSimilarItems(uint32 entry, uint32 count, Item* pItem, uint32* no_space_count) const
{
    ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(entry);
    if (!pProto)
    {
        if (no_space_count)
            *no_space_count = count;
        return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
    }

    // no maximum
    if (pProto->MaxCount > 0)
    {
        uint32 curcount = GetItemCount(pProto->ItemId, true, pItem);

        if (curcount + count > uint32(pProto->MaxCount))
        {
            if (no_space_count)
                *no_space_count = count + curcount - pProto->MaxCount;
            return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
        }
    }


    return EQUIP_ERR_OK;
}

bool Player::HasItemTotemCategory(uint32 TotemCategory) const
{
    /*[-ZERO] Item *pItem;
     for(uint8 i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
     {
         pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
         if( pItem && IsTotemCategoryCompatiableWith(pItem->GetProto()->TotemCategory,TotemCategory ))
             return true;
     }
     for(uint8 i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
     {
         pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
         if( pItem && IsTotemCategoryCompatiableWith(pItem->GetProto()->TotemCategory,TotemCategory ))
             return true;
     }
     for(uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
     {
         if(Bag *pBag = (Bag*)GetItemByPos( INVENTORY_SLOT_BAG_0, i ))
         {
             for(uint32 j = 0; j < pBag->GetBagSize(); ++j)
             {
                 pItem = GetItemByPos( i, j );
                 if( pItem && IsTotemCategoryCompatiableWith(pItem->GetProto()->TotemCategory,TotemCategory ))
                     return true;
             }
         }
     } */
    return false;
}

InventoryResult Player::_CanStoreItem_InSpecificSlot(uint8 bag, uint8 slot, ItemPosCountVec& dest, ItemPrototype const* pProto, uint32& count, bool swap, Item* pSrcItem) const
{
    Item* pItem2 = GetItemByPos(bag, slot);

    // ignore move item (this slot will be empty at move)
    if (pItem2 == pSrcItem)
        pItem2 = NULL;

    uint32 need_space;

    // empty specific slot - check item fit to slot
    if (!pItem2 || swap)
    {
        if (bag == INVENTORY_SLOT_BAG_0)
        {
            // keyring case
            if (slot >= KEYRING_SLOT_START && slot < KEYRING_SLOT_START + GetMaxKeyringSize() && !(pProto->BagFamily == BAG_FAMILY_KEYS))
                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

            // prevent cheating
            if ((slot >= BUYBACK_SLOT_START && slot < BUYBACK_SLOT_END) || slot >= PLAYER_SLOT_END)
                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;
        }
        else
        {
            Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
            if (!pBag)
                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

            ItemPrototype const* pBagProto = pBag->GetProto();
            if (!pBagProto)
                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

            if (slot >= pBagProto->ContainerSlots)
                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

            if (!ItemCanGoIntoBag(pProto, pBagProto))
                return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;
        }

        // non empty stack with space
        need_space = pProto->Stackable;
    }
    // non empty slot, check item type
    else
    {
        // can be merged at least partly
        InventoryResult res  = pItem2->CanBeMergedPartlyWith(pProto);
        if (res != EQUIP_ERR_OK)
            return res;

        need_space = pProto->Stackable - pItem2->GetCount();
    }

    if (need_space > count)
        need_space = count;

    ItemPosCount newPosition = ItemPosCount((bag << 8) | slot, need_space);
    if (!newPosition.isContainedIn(dest))
    {
        dest.push_back(newPosition);
        count -= need_space;
    }
    return EQUIP_ERR_OK;
}

InventoryResult Player::_CanStoreItem_InBag(uint8 bag, ItemPosCountVec& dest, ItemPrototype const* pProto, uint32& count, bool merge, bool non_specialized, Item* pSrcItem, uint8 skip_bag, uint8 skip_slot) const
{
    // skip specific bag already processed in first called _CanStoreItem_InBag
    if (bag == skip_bag)
        return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

    // skip nonexistent bag or self targeted bag
    Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
    if (!pBag || pBag == pSrcItem)
        return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

    ItemPrototype const* pBagProto = pBag->GetProto();
    if (!pBagProto)
        return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

    // specialized bag mode or non-specilized
    if (non_specialized != (pBagProto->Class == ITEM_CLASS_CONTAINER && pBagProto->SubClass == ITEM_SUBCLASS_CONTAINER))
        return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

    if (!ItemCanGoIntoBag(pProto, pBagProto))
        return EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG;

    for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
    {
        // skip specific slot already processed in first called _CanStoreItem_InSpecificSlot
        if (j == skip_slot)
            continue;

        Item* pItem2 = GetItemByPos(bag, j);

        // ignore move item (this slot will be empty at move)
        if (pItem2 == pSrcItem)
            pItem2 = NULL;

        // if merge skip empty, if !merge skip non-empty
        if ((pItem2 != NULL) != merge)
            continue;

        uint32 need_space = pProto->GetMaxStackSize();

        if (pItem2)
        {
            // can be merged at least partly
            uint8 res  = pItem2->CanBeMergedPartlyWith(pProto);
            if (res != EQUIP_ERR_OK)
                continue;

            // decrease at current stacksize
            need_space -= pItem2->GetCount();
        }

        if (need_space > count)
            need_space = count;

        ItemPosCount newPosition = ItemPosCount((bag << 8) | j, need_space);
        if (!newPosition.isContainedIn(dest))
        {
            dest.push_back(newPosition);
            count -= need_space;

            if (count == 0)
                return EQUIP_ERR_OK;
        }
    }
    return EQUIP_ERR_OK;
}

InventoryResult Player::_CanStoreItem_InInventorySlots(uint8 slot_begin, uint8 slot_end, ItemPosCountVec& dest, ItemPrototype const* pProto, uint32& count, bool merge, Item* pSrcItem, uint8 skip_bag, uint8 skip_slot) const
{
    for (uint32 j = slot_begin; j < slot_end; ++j)
    {
        // skip specific slot already processed in first called _CanStoreItem_InSpecificSlot
        if (INVENTORY_SLOT_BAG_0 == skip_bag && j == skip_slot)
            continue;

        Item* pItem2 = GetItemByPos(INVENTORY_SLOT_BAG_0, j);

        // ignore move item (this slot will be empty at move)
        if (pItem2 == pSrcItem)
            pItem2 = NULL;

        // if merge skip empty, if !merge skip non-empty
        if ((pItem2 != NULL) != merge)
            continue;

        uint32 need_space = pProto->GetMaxStackSize();

        if (pItem2)
        {
            // can be merged at least partly
            uint8 res  = pItem2->CanBeMergedPartlyWith(pProto);
            if (res != EQUIP_ERR_OK)
                continue;

            // descrease at current stacksize
            need_space -= pItem2->GetCount();
        }

        if (need_space > count)
            need_space = count;

        ItemPosCount newPosition = ItemPosCount((INVENTORY_SLOT_BAG_0 << 8) | j, need_space);
        if (!newPosition.isContainedIn(dest))
        {
            dest.push_back(newPosition);
            count -= need_space;

            if (count == 0)
                return EQUIP_ERR_OK;
        }
    }
    return EQUIP_ERR_OK;
}

InventoryResult Player::_CanStoreItem(uint8 bag, uint8 slot, ItemPosCountVec& dest, uint32 entry, uint32 count, Item* pItem, bool swap, uint32* no_space_count) const
{
    DEBUG_LOG("STORAGE: CanStoreItem bag = %u, slot = %u, item = %u, count = %u", bag, slot, entry, count);

    ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(entry);
    if (!pProto)
    {
        if (no_space_count)
            *no_space_count = count;
        return swap ? EQUIP_ERR_ITEMS_CANT_BE_SWAPPED : EQUIP_ERR_ITEM_NOT_FOUND;
    }

    if (pItem)
    {
        // item used
        if (pItem->HasTemporaryLoot())
        {
            if (no_space_count)
                *no_space_count = count;
            return EQUIP_ERR_ALREADY_LOOTED;
        }

        if (pItem->IsBindedNotWith(this))
        {
            if (no_space_count)
                *no_space_count = count;
            return EQUIP_ERR_DONT_OWN_THAT_ITEM;
        }
    }

    // check count of items (skip for auto move for same player from bank)
    uint32 no_similar_count = 0;                            // can't store this amount similar items
    InventoryResult res = _CanTakeMoreSimilarItems(entry, count, pItem, &no_similar_count);
    if (res != EQUIP_ERR_OK)
    {
        if (count == no_similar_count)
        {
            if (no_space_count)
                *no_space_count = no_similar_count;
            return res;
        }
        count -= no_similar_count;
    }

    // in specific slot
    if (bag != NULL_BAG && slot != NULL_SLOT)
    {
        res = _CanStoreItem_InSpecificSlot(bag, slot, dest, pProto, count, swap, pItem);
        if (res != EQUIP_ERR_OK)
        {
            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return res;
        }

        if (count == 0)
        {
            if (no_similar_count == 0)
                return EQUIP_ERR_OK;

            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
        }
    }

    // not specific slot or have space for partly store only in specific slot

    // in specific bag
    if (bag != NULL_BAG)
    {
        // search stack in bag for merge to
        if (pProto->Stackable > 1)
        {
            if (bag == INVENTORY_SLOT_BAG_0)               // inventory
            {
                res = _CanStoreItem_InInventorySlots(KEYRING_SLOT_START, KEYRING_SLOT_END, dest, pProto, count, true, pItem, bag, slot);
                if (res != EQUIP_ERR_OK)
                {
                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return res;
                }

                if (count == 0)
                {
                    if (no_similar_count == 0)
                        return EQUIP_ERR_OK;

                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                }

                res = _CanStoreItem_InInventorySlots(INVENTORY_SLOT_ITEM_START, INVENTORY_SLOT_ITEM_END, dest, pProto, count, true, pItem, bag, slot);
                if (res != EQUIP_ERR_OK)
                {
                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return res;
                }

                if (count == 0)
                {
                    if (no_similar_count == 0)
                        return EQUIP_ERR_OK;

                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                }
            }
            else                                            // equipped bag
            {
                // we need check 2 time (specialized/non_specialized), use NULL_BAG to prevent skipping bag
                res = _CanStoreItem_InBag(bag, dest, pProto, count, true, false, pItem, NULL_BAG, slot);
                if (res != EQUIP_ERR_OK)
                    res = _CanStoreItem_InBag(bag, dest, pProto, count, true, true, pItem, NULL_BAG, slot);

                if (res != EQUIP_ERR_OK)
                {
                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return res;
                }

                if (count == 0)
                {
                    if (no_similar_count == 0)
                        return EQUIP_ERR_OK;

                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                }
            }
        }

        // search free slot in bag for place to
        if (bag == INVENTORY_SLOT_BAG_0)                    // inventory
        {
            // search free slot - keyring case
            if (pProto->BagFamily == BAG_FAMILY_KEYS)
            {
                uint32 keyringSize = GetMaxKeyringSize();
                res = _CanStoreItem_InInventorySlots(KEYRING_SLOT_START, KEYRING_SLOT_START + keyringSize, dest, pProto, count, false, pItem, bag, slot);
                if (res != EQUIP_ERR_OK)
                {
                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return res;
                }

                if (count == 0)
                {
                    if (no_similar_count == 0)
                        return EQUIP_ERR_OK;

                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                }
            }

            res = _CanStoreItem_InInventorySlots(INVENTORY_SLOT_ITEM_START, INVENTORY_SLOT_ITEM_END, dest, pProto, count, false, pItem, bag, slot);
            if (res != EQUIP_ERR_OK)
            {
                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return res;
            }

            if (count == 0)
            {
                if (no_similar_count == 0)
                    return EQUIP_ERR_OK;

                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
            }
        }
        else                                                // equipped bag
        {
            res = _CanStoreItem_InBag(bag, dest, pProto, count, false, false, pItem, NULL_BAG, slot);
            if (res != EQUIP_ERR_OK)
                res = _CanStoreItem_InBag(bag, dest, pProto, count, false, true, pItem, NULL_BAG, slot);

            if (res != EQUIP_ERR_OK)
            {
                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return res;
            }

            if (count == 0)
            {
                if (no_similar_count == 0)
                    return EQUIP_ERR_OK;

                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
            }
        }
    }

    // not specific bag or have space for partly store only in specific bag

    // search stack for merge to
    if (pProto->Stackable > 1)
    {
        res = _CanStoreItem_InInventorySlots(KEYRING_SLOT_START, KEYRING_SLOT_END, dest, pProto, count, true, pItem, bag, slot);
        if (res != EQUIP_ERR_OK)
        {
            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return res;
        }

        if (count == 0)
        {
            if (no_similar_count == 0)
                return EQUIP_ERR_OK;

            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
        }

        res = _CanStoreItem_InInventorySlots(INVENTORY_SLOT_ITEM_START, INVENTORY_SLOT_ITEM_END, dest, pProto, count, true, pItem, bag, slot);
        if (res != EQUIP_ERR_OK)
        {
            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return res;
        }

        if (count == 0)
        {
            if (no_similar_count == 0)
                return EQUIP_ERR_OK;

            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
        }

        if (pProto->BagFamily)
        {
            for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
            {
                res = _CanStoreItem_InBag(i, dest, pProto, count, true, false, pItem, bag, slot);
                if (res != EQUIP_ERR_OK)
                    continue;

                if (count == 0)
                {
                    if (no_similar_count == 0)
                        return EQUIP_ERR_OK;

                    if (no_space_count)
                        *no_space_count = count + no_similar_count;
                    return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
                }
            }
        }

        for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        {
            res = _CanStoreItem_InBag(i, dest, pProto, count, true, true, pItem, bag, slot);
            if (res != EQUIP_ERR_OK)
                continue;

            if (count == 0)
            {
                if (no_similar_count == 0)
                    return EQUIP_ERR_OK;

                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
            }
        }
    }

    // search free slot - special bag case
    if (pProto->BagFamily)
    {
        if (pProto->BagFamily == BAG_FAMILY_KEYS)
        {
            uint32 keyringSize = GetMaxKeyringSize();
            res = _CanStoreItem_InInventorySlots(KEYRING_SLOT_START, KEYRING_SLOT_START + keyringSize, dest, pProto, count, false, pItem, bag, slot);
            if (res != EQUIP_ERR_OK)
            {
                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return res;
            }

            if (count == 0)
            {
                if (no_similar_count == 0)
                    return EQUIP_ERR_OK;

                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
            }
        }

        for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        {
            res = _CanStoreItem_InBag(i, dest, pProto, count, false, false, pItem, bag, slot);
            if (res != EQUIP_ERR_OK)
                continue;

            if (count == 0)
            {
                if (no_similar_count == 0)
                    return EQUIP_ERR_OK;

                if (no_space_count)
                    *no_space_count = count + no_similar_count;
                return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
            }
        }
    }

    // Normally it would be impossible to autostore not empty bags
    if (pItem && pItem->IsBag() && !((Bag*)pItem)->IsEmpty())
        return EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG;

    // search free slot
    res = _CanStoreItem_InInventorySlots(INVENTORY_SLOT_ITEM_START, INVENTORY_SLOT_ITEM_END, dest, pProto, count, false, pItem, bag, slot);
    if (res != EQUIP_ERR_OK)
    {
        if (no_space_count)
            *no_space_count = count + no_similar_count;
        return res;
    }

    if (count == 0)
    {
        if (no_similar_count == 0)
            return EQUIP_ERR_OK;

        if (no_space_count)
            *no_space_count = count + no_similar_count;
        return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
    }

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        res = _CanStoreItem_InBag(i, dest, pProto, count, false, true, pItem, bag, slot);
        if (res != EQUIP_ERR_OK)
            continue;

        if (count == 0)
        {
            if (no_similar_count == 0)
                return EQUIP_ERR_OK;

            if (no_space_count)
                *no_space_count = count + no_similar_count;
            return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;
        }
    }

    if (no_space_count)
        *no_space_count = count + no_similar_count;

    return EQUIP_ERR_INVENTORY_FULL;
}

//////////////////////////////////////////////////////////////////////////
InventoryResult Player::CanStoreItems(Item** pItems, int count) const
{
    Item*    pItem2;

    // fill space table
    int inv_slot_items[INVENTORY_SLOT_ITEM_END - INVENTORY_SLOT_ITEM_START];
    int inv_bags[INVENTORY_SLOT_BAG_END - INVENTORY_SLOT_BAG_START][MAX_BAG_SIZE];
    int inv_keys[KEYRING_SLOT_END - KEYRING_SLOT_START];

    memset(inv_slot_items, 0, sizeof(int) * (INVENTORY_SLOT_ITEM_END - INVENTORY_SLOT_ITEM_START));
    memset(inv_bags, 0, sizeof(int) * (INVENTORY_SLOT_BAG_END - INVENTORY_SLOT_BAG_START)*MAX_BAG_SIZE);
    memset(inv_keys, 0, sizeof(int) * (KEYRING_SLOT_END - KEYRING_SLOT_START));

    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        pItem2 = GetItemByPos(INVENTORY_SLOT_BAG_0, i);

        if (pItem2 && !pItem2->IsInTrade())
        {
            inv_slot_items[i - INVENTORY_SLOT_ITEM_START] = pItem2->GetCount();
        }
    }

    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
    {
        pItem2 = GetItemByPos(INVENTORY_SLOT_BAG_0, i);

        if (pItem2 && !pItem2->IsInTrade())
        {
            inv_keys[i - KEYRING_SLOT_START] = pItem2->GetCount();
        }
    }

    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
            {
                pItem2 = GetItemByPos(i, j);
                if (pItem2 && !pItem2->IsInTrade())
                {
                    inv_bags[i - INVENTORY_SLOT_BAG_START][j] = pItem2->GetCount();
                }
            }
        }
    }

    // check free space for all items
    for (int k = 0; k < count; ++k)
    {
        Item*  pItem = pItems[k];

        // no item
        if (!pItem)  continue;

        DEBUG_LOG("STORAGE: CanStoreItems %i. item = %u, count = %u", k + 1, pItem->GetEntry(), pItem->GetCount());
        ItemPrototype const* pProto = pItem->GetProto();

        // strange item
        if (!pProto)
            return EQUIP_ERR_ITEM_NOT_FOUND;

        // item used
        if (pItem->HasTemporaryLoot())
            return EQUIP_ERR_ALREADY_LOOTED;

        // item it 'bind'
        if (pItem->IsBindedNotWith(this))
            return EQUIP_ERR_DONT_OWN_THAT_ITEM;

        Bag* pBag;
        ItemPrototype const* pBagProto;

        // item is 'one item only'
        InventoryResult res = CanTakeMoreSimilarItems(pItem);
        if (res != EQUIP_ERR_OK)
            return res;

        // search stack for merge to
        if (pProto->Stackable > 1)
        {
            bool b_found = false;

            for (int t = KEYRING_SLOT_START; t < KEYRING_SLOT_END; ++t)
            {
                pItem2 = GetItemByPos(INVENTORY_SLOT_BAG_0, t);
                if (pItem2 && pItem2->CanBeMergedPartlyWith(pProto) == EQUIP_ERR_OK && inv_keys[t - KEYRING_SLOT_START] + pItem->GetCount() <= pProto->GetMaxStackSize())
                {
                    inv_keys[t - KEYRING_SLOT_START] += pItem->GetCount();
                    b_found = true;
                    break;
                }
            }
            if (b_found) continue;

            for (int t = INVENTORY_SLOT_ITEM_START; t < INVENTORY_SLOT_ITEM_END; ++t)
            {
                pItem2 = GetItemByPos(INVENTORY_SLOT_BAG_0, t);
                if (pItem2 && pItem2->CanBeMergedPartlyWith(pProto) == EQUIP_ERR_OK && inv_slot_items[t - INVENTORY_SLOT_ITEM_START] + pItem->GetCount() <= pProto->GetMaxStackSize())
                {
                    inv_slot_items[t - INVENTORY_SLOT_ITEM_START] += pItem->GetCount();
                    b_found = true;
                    break;
                }
            }
            if (b_found) continue;

            for (int t = INVENTORY_SLOT_BAG_START; !b_found && t < INVENTORY_SLOT_BAG_END; ++t)
            {
                pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, t);
                if (pBag)
                {
                    for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                    {
                        pItem2 = GetItemByPos(t, j);
                        if (pItem2 && pItem2->CanBeMergedPartlyWith(pProto) == EQUIP_ERR_OK && inv_bags[t - INVENTORY_SLOT_BAG_START][j] + pItem->GetCount() <= pProto->GetMaxStackSize())
                        {
                            inv_bags[t - INVENTORY_SLOT_BAG_START][j] += pItem->GetCount();
                            b_found = true;
                            break;
                        }
                    }
                }
            }
            if (b_found) continue;
        }

        // special bag case
        if (pProto->BagFamily)
        {
            bool b_found = false;
            if (pProto->BagFamily == BAG_FAMILY_KEYS)
            {
                uint32 keyringSize = GetMaxKeyringSize();
                for (uint32 t = KEYRING_SLOT_START; t < KEYRING_SLOT_START + keyringSize; ++t)
                {
                    if (inv_keys[t - KEYRING_SLOT_START] == 0)
                    {
                        inv_keys[t - KEYRING_SLOT_START] = 1;
                        b_found = true;
                        break;
                    }
                }
            }

            if (b_found) continue;

            for (int t = INVENTORY_SLOT_BAG_START; !b_found && t < INVENTORY_SLOT_BAG_END; ++t)
            {
                pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, t);
                if (pBag)
                {
                    pBagProto = pBag->GetProto();

                    // not plain container check
                    if (pBagProto && (pBagProto->Class != ITEM_CLASS_CONTAINER || pBagProto->SubClass != ITEM_SUBCLASS_CONTAINER) &&
                            ItemCanGoIntoBag(pProto, pBagProto))
                    {
                        for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                        {
                            if (inv_bags[t - INVENTORY_SLOT_BAG_START][j] == 0)
                            {
                                inv_bags[t - INVENTORY_SLOT_BAG_START][j] = 1;
                                b_found = true;
                                break;
                            }
                        }
                    }
                }
            }
            if (b_found) continue;
        }

        // search free slot
        bool b_found = false;
        for (int t = INVENTORY_SLOT_ITEM_START; t < INVENTORY_SLOT_ITEM_END; ++t)
        {
            if (inv_slot_items[t - INVENTORY_SLOT_ITEM_START] == 0)
            {
                inv_slot_items[t - INVENTORY_SLOT_ITEM_START] = 1;
                b_found = true;
                break;
            }
        }
        if (b_found) continue;

        // search free slot in bags
        for (int t = INVENTORY_SLOT_BAG_START; !b_found && t < INVENTORY_SLOT_BAG_END; ++t)
        {
            pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, t);
            if (pBag)
            {
                pBagProto = pBag->GetProto();

                // special bag already checked
                if (pBagProto && (pBagProto->Class != ITEM_CLASS_CONTAINER || pBagProto->SubClass != ITEM_SUBCLASS_CONTAINER))
                    continue;

                for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                {
                    if (inv_bags[t - INVENTORY_SLOT_BAG_START][j] == 0)
                    {
                        inv_bags[t - INVENTORY_SLOT_BAG_START][j] = 1;
                        b_found = true;
                        break;
                    }
                }
            }
        }

        // no free slot found?
        if (!b_found)
            return EQUIP_ERR_INVENTORY_FULL;
    }

    return EQUIP_ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
InventoryResult Player::CanEquipNewItem(uint8 slot, uint16& dest, uint32 item, bool swap) const
{
    dest = 0;
    Item* pItem = Item::CreateItem(item, 1, this);
    if (pItem)
    {
        InventoryResult result = CanEquipItem(slot, dest, pItem, swap);
        delete pItem;
        return result;
    }

    return EQUIP_ERR_ITEM_NOT_FOUND;
}

InventoryResult Player::CanEquipItem(uint8 slot, uint16& dest, Item* pItem, bool swap, bool direct_action) const
{
    dest = 0;
    if (pItem)
    {
        DEBUG_LOG("STORAGE: CanEquipItem slot = %u, item = %u, count = %u", slot, pItem->GetEntry(), pItem->GetCount());
        ItemPrototype const* pProto = pItem->GetProto();
        if (pProto)
        {
            // item used
            if (pItem->HasTemporaryLoot())
                return EQUIP_ERR_ALREADY_LOOTED;

            if (pItem->IsBindedNotWith(this))
                return EQUIP_ERR_DONT_OWN_THAT_ITEM;

            // check count of items (skip for auto move for same player from bank)
            InventoryResult res = CanTakeMoreSimilarItems(pItem);
            if (res != EQUIP_ERR_OK)
                return res;

            // check this only in game
            if (direct_action)
            {
                // May be here should be more stronger checks; STUNNED checked
                // ROOT, CONFUSED, DISTRACTED, FLEEING this needs to be checked.
                if (hasUnitState(UNIT_STAT_STUNNED))
                    return EQUIP_ERR_YOU_ARE_STUNNED;

                // do not allow equipping gear except weapons, offhands, projectiles, relics in
                // - combat
                if (!pProto->CanChangeEquipStateInCombat())
                {
                    if (isInCombat())
                        return EQUIP_ERR_NOT_IN_COMBAT;
                }

                // prevent equip item in process logout
                if (GetSession()->isLogingOut())
                    return EQUIP_ERR_YOU_ARE_STUNNED;

                if (isInCombat() && pProto->Class == ITEM_CLASS_WEAPON && m_weaponChangeTimer != 0)
                    return EQUIP_ERR_CANT_DO_RIGHT_NOW;     // maybe exist better err

                if (IsNonMeleeSpellCasted(false))
                    return EQUIP_ERR_CANT_DO_RIGHT_NOW;
            }

            uint8 eslot = FindEquipSlot(pProto, slot, swap);
            if (eslot == NULL_SLOT)
                return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;

            InventoryResult msg = CanUseItem(pItem , direct_action);
            if (msg != EQUIP_ERR_OK)
                return msg;
            if (!swap && GetItemByPos(INVENTORY_SLOT_BAG_0, eslot))
                return EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE;

            // if swap ignore item (equipped also)
            if (InventoryResult res2 = CanEquipUniqueItem(pItem, swap ? eslot : uint8(NULL_SLOT)))
                return res2;

            // check unique-equipped special item classes
            if (pProto->Class == ITEM_CLASS_QUIVER)
            {
                for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
                {
                    if (Item* pBag = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                    {
                        if (pBag != pItem)
                        {
                            if (ItemPrototype const* pBagProto = pBag->GetProto())
                            {
                                if (pBagProto->Class == pProto->Class && (!swap || pBag->GetSlot() != eslot))
                                    return (pBagProto->SubClass == ITEM_SUBCLASS_AMMO_POUCH)
                                           ? EQUIP_ERR_CAN_EQUIP_ONLY1_AMMOPOUCH
                                           : EQUIP_ERR_CAN_EQUIP_ONLY1_QUIVER;
                            }
                        }
                    }
                }
            }

            uint32 type = pProto->InventoryType;

            if (eslot == EQUIPMENT_SLOT_OFFHAND)
            {
                if (type == INVTYPE_WEAPON || type == INVTYPE_WEAPONOFFHAND)
                {
                    if (!CanDualWield())
                        return EQUIP_ERR_CANT_DUAL_WIELD;
                }
                else if (type == INVTYPE_2HWEAPON)
                {
                    return EQUIP_ERR_CANT_DUAL_WIELD;
                }

                if (IsTwoHandUsed())
                    return EQUIP_ERR_CANT_EQUIP_WITH_TWOHANDED;
            }

            // equip two-hand weapon case (with possible unequip 2 items)
            if (type == INVTYPE_2HWEAPON)
            {
                if (eslot != EQUIPMENT_SLOT_MAINHAND)
                    return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;

                // offhand item must can be stored in inventory for offhand item and it also must be unequipped
                Item* offItem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                ItemPosCountVec off_dest;
                if (offItem && (!direct_action ||
                                CanUnequipItem(uint16(INVENTORY_SLOT_BAG_0) << 8 | EQUIPMENT_SLOT_OFFHAND, false) !=  EQUIP_ERR_OK ||
                                CanStoreItem(NULL_BAG, NULL_SLOT, off_dest, offItem, false) !=  EQUIP_ERR_OK))
                    return swap ? EQUIP_ERR_ITEMS_CANT_BE_SWAPPED : EQUIP_ERR_INVENTORY_FULL;
            }
            dest = ((INVENTORY_SLOT_BAG_0 << 8) | eslot);
            return EQUIP_ERR_OK;
        }
    }

    return !swap ? EQUIP_ERR_ITEM_NOT_FOUND : EQUIP_ERR_ITEMS_CANT_BE_SWAPPED;
}

InventoryResult Player::CanUnequipItem(uint16 pos, bool swap) const
{
    // Applied only to equipped items and bank bags
    if (!IsEquipmentPos(pos) && !IsBagPos(pos))
        return EQUIP_ERR_OK;

    Item* pItem = GetItemByPos(pos);

    // Applied only to existing equipped item
    if (!pItem)
        return EQUIP_ERR_OK;

    DEBUG_LOG("STORAGE: CanUnequipItem slot = %u, item = %u, count = %u", pos, pItem->GetEntry(), pItem->GetCount());

    ItemPrototype const* pProto = pItem->GetProto();
    if (!pProto)
        return EQUIP_ERR_ITEM_NOT_FOUND;

    // item used
    if (pItem->HasTemporaryLoot())
        return EQUIP_ERR_ALREADY_LOOTED;

    // do not allow unequipping gear except weapons, offhands, projectiles, relics in
    // - combat
    if (!pProto->CanChangeEquipStateInCombat())
    {
        if (isInCombat())
            return EQUIP_ERR_NOT_IN_COMBAT;
    }

    // prevent unequip item in process logout
    if (GetSession()->isLogingOut())
        return EQUIP_ERR_YOU_ARE_STUNNED;

    if (!swap && pItem->IsBag() && !((Bag*)pItem)->IsEmpty())
        return EQUIP_ERR_CAN_ONLY_DO_WITH_EMPTY_BAGS;

    return EQUIP_ERR_OK;
}

InventoryResult Player::CanBankItem(uint8 bag, uint8 slot, ItemPosCountVec& dest, Item* pItem, bool swap, bool not_loading) const
{
    if (!pItem)
        return swap ? EQUIP_ERR_ITEMS_CANT_BE_SWAPPED : EQUIP_ERR_ITEM_NOT_FOUND;

    uint32 count = pItem->GetCount();

    DEBUG_LOG("STORAGE: CanBankItem bag = %u, slot = %u, item = %u, count = %u", bag, slot, pItem->GetEntry(), pItem->GetCount());
    ItemPrototype const* pProto = pItem->GetProto();
    if (!pProto)
        return swap ? EQUIP_ERR_ITEMS_CANT_BE_SWAPPED : EQUIP_ERR_ITEM_NOT_FOUND;

    // item used
    if (pItem->HasTemporaryLoot())
        return EQUIP_ERR_ALREADY_LOOTED;

    if (pItem->IsBindedNotWith(this))
        return EQUIP_ERR_DONT_OWN_THAT_ITEM;

    // check count of items (skip for auto move for same player from bank)
    InventoryResult res = CanTakeMoreSimilarItems(pItem);
    if (res != EQUIP_ERR_OK)
        return res;

    // in specific slot
    if (bag != NULL_BAG && slot != NULL_SLOT)
    {
        if (slot >= BANK_SLOT_BAG_START && slot < BANK_SLOT_BAG_END)
        {
            if (!pItem->IsBag())
                return EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT;

            if (slot - BANK_SLOT_BAG_START >= GetBankBagSlotCount())
                return EQUIP_ERR_MUST_PURCHASE_THAT_BAG_SLOT;

            res = CanUseItem(pItem, not_loading);
            if (res != EQUIP_ERR_OK)
                return res;
        }

        res = _CanStoreItem_InSpecificSlot(bag, slot, dest, pProto, count, swap, pItem);
        if (res != EQUIP_ERR_OK)
            return res;

        if (count == 0)
            return EQUIP_ERR_OK;
    }

    // not specific slot or have space for partly store only in specific slot

    // in specific bag
    if (bag != NULL_BAG)
    {
        if (pProto->InventoryType == INVTYPE_BAG)
        {
            Bag* pBag = (Bag*)pItem;
            if (pBag && !pBag->IsEmpty())
                return EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG;
        }

        // search stack in bag for merge to
        if (pProto->Stackable > 1)
        {
            if (bag == INVENTORY_SLOT_BAG_0)
            {
                res = _CanStoreItem_InInventorySlots(BANK_SLOT_ITEM_START, BANK_SLOT_ITEM_END, dest, pProto, count, true, pItem, bag, slot);
                if (res != EQUIP_ERR_OK)
                    return res;

                if (count == 0)
                    return EQUIP_ERR_OK;
            }
            else
            {
                res = _CanStoreItem_InBag(bag, dest, pProto, count, true, false, pItem, NULL_BAG, slot);
                if (res != EQUIP_ERR_OK)
                    res = _CanStoreItem_InBag(bag, dest, pProto, count, true, true, pItem, NULL_BAG, slot);

                if (res != EQUIP_ERR_OK)
                    return res;

                if (count == 0)
                    return EQUIP_ERR_OK;
            }
        }

        // search free slot in bag
        if (bag == INVENTORY_SLOT_BAG_0)
        {
            res = _CanStoreItem_InInventorySlots(BANK_SLOT_ITEM_START, BANK_SLOT_ITEM_END, dest, pProto, count, false, pItem, bag, slot);
            if (res != EQUIP_ERR_OK)
                return res;

            if (count == 0)
                return EQUIP_ERR_OK;
        }
        else
        {
            res = _CanStoreItem_InBag(bag, dest, pProto, count, false, false, pItem, NULL_BAG, slot);
            if (res != EQUIP_ERR_OK)
                res = _CanStoreItem_InBag(bag, dest, pProto, count, false, true, pItem, NULL_BAG, slot);

            if (res != EQUIP_ERR_OK)
                return res;

            if (count == 0)
                return EQUIP_ERR_OK;
        }
    }

    // not specific bag or have space for partly store only in specific bag

    // search stack for merge to
    if (pProto->Stackable > 1)
    {
        // in slots
        res = _CanStoreItem_InInventorySlots(BANK_SLOT_ITEM_START, BANK_SLOT_ITEM_END, dest, pProto, count, true, pItem, bag, slot);
        if (res != EQUIP_ERR_OK)
            return res;

        if (count == 0)
            return EQUIP_ERR_OK;

        // in special bags
        if (pProto->BagFamily)
        {
            for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
            {
                res = _CanStoreItem_InBag(i, dest, pProto, count, true, false, pItem, bag, slot);
                if (res != EQUIP_ERR_OK)
                    continue;

                if (count == 0)
                    return EQUIP_ERR_OK;
            }
        }

        for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        {
            res = _CanStoreItem_InBag(i, dest, pProto, count, true, true, pItem, bag, slot);
            if (res != EQUIP_ERR_OK)
                continue;

            if (count == 0)
                return EQUIP_ERR_OK;
        }
    }

    // search free place in special bag
    if (pProto->BagFamily)
    {
        for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        {
            res = _CanStoreItem_InBag(i, dest, pProto, count, false, false, pItem, bag, slot);
            if (res != EQUIP_ERR_OK)
                continue;

            if (count == 0)
                return EQUIP_ERR_OK;
        }
    }

    // search free space
    res = _CanStoreItem_InInventorySlots(BANK_SLOT_ITEM_START, BANK_SLOT_ITEM_END, dest, pProto, count, false, pItem, bag, slot);
    if (res != EQUIP_ERR_OK)
        return res;

    if (count == 0)
        return EQUIP_ERR_OK;

    for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
    {
        res = _CanStoreItem_InBag(i, dest, pProto, count, false, true, pItem, bag, slot);
        if (res != EQUIP_ERR_OK)
            continue;

        if (count == 0)
            return EQUIP_ERR_OK;
    }
    return EQUIP_ERR_BANK_FULL;
}

InventoryResult Player::CanUseItem(Item* pItem, bool direct_action) const
{
    if (pItem)
    {
        DEBUG_LOG("STORAGE: CanUseItem item = %u", pItem->GetEntry());

        if (!isAlive() && direct_action)
            return EQUIP_ERR_YOU_ARE_DEAD;

        // if (isStunned())
        //    return EQUIP_ERR_YOU_ARE_STUNNED;

        ItemPrototype const* pProto = pItem->GetProto();
        if (pProto)
        {
            if (pItem->IsBindedNotWith(this))
                return EQUIP_ERR_DONT_OWN_THAT_ITEM;

            InventoryResult msg = CanUseItem(pProto, direct_action);
            if (msg != EQUIP_ERR_OK)
                return msg;

            if (pItem->GetSkill() != 0)
            {
                if (GetSkillValue(pItem->GetSkill()) == 0)
                    return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;
            }

            if (pProto->RequiredReputationFaction && uint32(GetReputationRank(pProto->RequiredReputationFaction)) < pProto->RequiredReputationRank)
                return EQUIP_ERR_CANT_EQUIP_REPUTATION;

            return EQUIP_ERR_OK;
        }
    }
    return EQUIP_ERR_ITEM_NOT_FOUND;
}

InventoryResult Player::CanUseItem(ItemPrototype const* pProto, bool direct_action) const
{
    // Used by group, function NeedBeforeGreed, to know if a prototype can be used by a player

    if (pProto)
    {
        if ((pProto->AllowableClass & getClassMask()) == 0 || (pProto->AllowableRace & getRaceMask()) == 0)
            return EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM;

        if (pProto->RequiredSkill != 0)
        {
            if (GetSkillValue(pProto->RequiredSkill) == 0)
                return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;
            else if (GetSkillValue(pProto->RequiredSkill) < pProto->RequiredSkillRank)
                return EQUIP_ERR_CANT_EQUIP_SKILL;
        }

        if (pProto->RequiredSpell != 0 && !HasSpell(pProto->RequiredSpell))
            return EQUIP_ERR_NO_REQUIRED_PROFICIENCY;

        if (direct_action && GetHonorHighestRankInfo().rank < (uint8)pProto->RequiredHonorRank)
            return EQUIP_ERR_CANT_EQUIP_RANK;

        if (getLevel() < pProto->RequiredLevel)
            return EQUIP_ERR_CANT_EQUIP_LEVEL_I;

        return EQUIP_ERR_OK;
    }
    return EQUIP_ERR_ITEM_NOT_FOUND;
}

InventoryResult Player::CanUseAmmo(uint32 item) const
{
    DEBUG_LOG("STORAGE: CanUseAmmo item = %u", item);
    if (!isAlive())
        return EQUIP_ERR_YOU_ARE_DEAD;
    // if( isStunned() )
    //    return EQUIP_ERR_YOU_ARE_STUNNED;
    ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(item);
    if (pProto)
    {
        if (pProto->InventoryType != INVTYPE_AMMO)
            return EQUIP_ERR_ONLY_AMMO_CAN_GO_HERE;

        InventoryResult msg = CanUseItem(pProto);
        if (msg != EQUIP_ERR_OK)
            return msg;

        /*if( GetReputationMgr().GetReputation() < pProto->RequiredReputation )
        /*if ( GetReputationMgr().GetReputation() < pProto->RequiredReputation )
        return EQUIP_ERR_CANT_EQUIP_REPUTATION;
        */

        return EQUIP_ERR_OK;
    }
    return EQUIP_ERR_ITEM_NOT_FOUND;
}

void Player::SetAmmo(uint32 item)
{
    if (!item)
        return;

    // already set
    if (GetUInt32Value(PLAYER_AMMO_ID) == item)
        return;

    // check ammo
    if (item)
    {
        InventoryResult msg = CanUseAmmo(item);
        if (msg != EQUIP_ERR_OK)
        {
            SendEquipError(msg, NULL, NULL, item);
            return;
        }
    }

    SetUInt32Value(PLAYER_AMMO_ID, item);

    _ApplyAmmoBonuses();
}

void Player::RemoveAmmo()
{
    SetUInt32Value(PLAYER_AMMO_ID, 0);

    m_ammoDPS = 0.0f;

    if (CanModifyStats())
        UpdateDamagePhysical(RANGED_ATTACK);
}

// Return stored item (if stored to stack, it can diff. from pItem). And pItem ca be deleted in this case.
Item* Player::StoreNewItem(ItemPosCountVec const& dest, uint32 item, bool update, int32 randomPropertyId)
{
    uint32 count = 0;
    for (ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end(); ++itr)
        count += itr->count;

    Item* pItem = Item::CreateItem(item, count, this, randomPropertyId);
    if (pItem)
    {
        ItemAddedQuestCheck(item, count);
        pItem = StoreItem(dest, pItem, update);
    }
    return pItem;
}

Item* Player::StoreItem(ItemPosCountVec const& dest, Item* pItem, bool update)
{
    if (!pItem)
        return NULL;

    Item* lastItem = pItem;

    for (ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end();)
    {
        uint16 pos = itr->pos;
        uint32 count = itr->count;

        ++itr;

        if (itr == dest.end())
        {
            lastItem = _StoreItem(pos, pItem, count, false, update);
            break;
        }

        lastItem = _StoreItem(pos, pItem, count, true, update);
    }

    return lastItem;
}

// Return stored item (if stored to stack, it can diff. from pItem). And pItem ca be deleted in this case.
Item* Player::_StoreItem(uint16 pos, Item* pItem, uint32 count, bool clone, bool update)
{
    if (!pItem)
        return NULL;

    uint8 bag = pos >> 8;
    uint8 slot = pos & 255;

    DEBUG_LOG("STORAGE: StoreItem bag = %u, slot = %u, item = %u, count = %u", bag, slot, pItem->GetEntry(), count);

    Item* pItem2 = GetItemByPos(bag, slot);

    if (!pItem2)
    {
        if (clone)
            pItem = pItem->CloneItem(count, this);
        else
            pItem->SetCount(count);

        if (!pItem)
            return NULL;

        if (pItem->GetProto()->Bonding == BIND_WHEN_PICKED_UP ||
                pItem->GetProto()->Bonding == BIND_QUEST_ITEM ||
                (pItem->GetProto()->Bonding == BIND_WHEN_EQUIPPED && IsBagPos(pos)))
            pItem->SetBinding(true);

        if (bag == INVENTORY_SLOT_BAG_0)
        {
            m_items[slot] = pItem;
            SetGuidValue(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2), pItem->GetObjectGuid());
            pItem->SetGuidValue(ITEM_FIELD_CONTAINED, GetObjectGuid());
            pItem->SetGuidValue(ITEM_FIELD_OWNER, GetObjectGuid());

            pItem->SetSlot(slot);
            pItem->SetContainer(NULL);

            if (IsInWorld() && update)
            {
                pItem->AddToWorld();
                pItem->SendCreateUpdateToPlayer(this);
            }

            pItem->SetState(ITEM_CHANGED, this);
        }
        else if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, bag))
        {
            pBag->StoreItem(slot, pItem, update);
            if (IsInWorld() && update)
            {
                pItem->AddToWorld();
                pItem->SendCreateUpdateToPlayer(this);
            }
            pItem->SetState(ITEM_CHANGED, this);
            pBag->SetState(ITEM_CHANGED, this);
        }

        AddEnchantmentDurations(pItem);
        AddItemDurations(pItem);

        return pItem;
    }
    else
    {
        if (pItem2->GetProto()->Bonding == BIND_WHEN_PICKED_UP ||
                pItem2->GetProto()->Bonding == BIND_QUEST_ITEM ||
                (pItem2->GetProto()->Bonding == BIND_WHEN_EQUIPPED && IsBagPos(pos)))
            pItem2->SetBinding(true);

        pItem2->SetCount(pItem2->GetCount() + count);
        if (IsInWorld() && update)
            pItem2->SendCreateUpdateToPlayer(this);

        if (!clone)
        {
            // delete item (it not in any slot currently)
            if (IsInWorld() && update)
            {
                pItem->RemoveFromWorld();
                pItem->DestroyForPlayer(this);
            }

            RemoveEnchantmentDurations(pItem);
            RemoveItemDurations(pItem);

            pItem->SetOwnerGuid(GetObjectGuid());           // prevent error at next SetState in case trade/mail/buy from vendor
            pItem->SetState(ITEM_REMOVED, this);
        }

        // AddItemDurations(pItem2); - pItem2 already have duration listed for player
        AddEnchantmentDurations(pItem2);

        pItem2->SetState(ITEM_CHANGED, this);

        return pItem2;
    }
}

Item* Player::EquipNewItem(uint16 pos, uint32 item, bool update)
{
    if (Item* pItem = Item::CreateItem(item, 1, this))
    {
        ItemAddedQuestCheck(item, 1);
        return EquipItem(pos, pItem, update);
    }

    return NULL;
}

Item* Player::EquipItem(uint16 pos, Item* pItem, bool update)
{
    AddEnchantmentDurations(pItem);
    AddItemDurations(pItem);

    uint8 bag = pos >> 8;
    uint8 slot = pos & 255;

    Item* pItem2 = GetItemByPos(bag, slot);

    if (!pItem2)
    {
        VisualizeItem(slot, pItem);

        if (isAlive())
        {
            ItemPrototype const* pProto = pItem->GetProto();

            // item set bonuses applied only at equip and removed at unequip, and still active for broken items
            if (pProto && pProto->ItemSet)
                AddItemsSetItem(this, pItem);

            _ApplyItemMods(pItem, slot, true);

            // Weapons and also Totem/Relic/Sigil/etc
            if (pProto && isInCombat() && (pProto->Class == ITEM_CLASS_WEAPON || pProto->InventoryType == INVTYPE_RELIC) && m_weaponChangeTimer == 0)
            {
                uint32 cooldownSpell = SPELL_ID_WEAPON_SWITCH_COOLDOWN_1_5s;

                if (getClass() == CLASS_ROGUE)
                    cooldownSpell = SPELL_ID_WEAPON_SWITCH_COOLDOWN_1_0s;

                SpellEntry const* spellProto = sSpellStore.LookupEntry(cooldownSpell);

                if (!spellProto)
                    sLog.outError("Weapon switch cooldown spell %u couldn't be found in Spell.dbc", cooldownSpell);
                else
                {
                    m_weaponChangeTimer = spellProto->StartRecoveryTime;

                    WorldPacket data(SMSG_SPELL_COOLDOWN, 8 + 4 + 4);
                    data << GetObjectGuid();
                    data << uint32(cooldownSpell);
                    data << uint32(0);
                    GetSession()->SendPacket(&data);
                }
            }
        }

        if (IsInWorld() && update)
        {
            pItem->AddToWorld();
            pItem->SendCreateUpdateToPlayer(this);
        }

        ApplyEquipCooldown(pItem);
    }
    else
    {
        pItem2->SetCount(pItem2->GetCount() + pItem->GetCount());
        if (IsInWorld() && update)
            pItem2->SendCreateUpdateToPlayer(this);

        // delete item (it not in any slot currently)
        // pItem->DeleteFromDB();
        if (IsInWorld() && update)
        {
            pItem->RemoveFromWorld();
            pItem->DestroyForPlayer(this);
        }

        RemoveEnchantmentDurations(pItem);
        RemoveItemDurations(pItem);

        pItem->SetOwnerGuid(GetObjectGuid());               // prevent error at next SetState in case trade/mail/buy from vendor
        pItem->SetState(ITEM_REMOVED, this);
        pItem2->SetState(ITEM_CHANGED, this);

        ApplyEquipCooldown(pItem2);

        // used by eluna
        sHookMgr->OnEquip(this, pItem2, bag, slot);
        return pItem2;
    }

    // used by eluna
    sHookMgr->OnEquip(this, pItem, bag, slot);

    return pItem;
}

void Player::QuickEquipItem(uint16 pos, Item* pItem)
{
    if (pItem)
    {
        AddEnchantmentDurations(pItem);
        AddItemDurations(pItem);

        uint8 slot = pos & 255;
        VisualizeItem(slot, pItem);

        if (IsInWorld())
        {
            pItem->AddToWorld();
            pItem->SendCreateUpdateToPlayer(this);
        }
    }
}

void Player::SetVisibleItemSlot(uint8 slot, Item* pItem)
{
    if (pItem)
    {
        SetGuidValue(PLAYER_VISIBLE_ITEM_1_CREATOR + (slot * MAX_VISIBLE_ITEM_OFFSET), pItem->GetGuidValue(ITEM_FIELD_CREATOR));

        int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * MAX_VISIBLE_ITEM_OFFSET);
        SetUInt32Value(VisibleBase + 0, pItem->GetEntry());

        for (int i = 0; i < MAX_INSPECTED_ENCHANTMENT_SLOT; ++i)
            SetUInt32Value(VisibleBase + 1 + i, pItem->GetEnchantmentId(EnchantmentSlot(i)));

        // Use SetInt16Value to prevent set high part to FFFF for negative value
        SetInt16Value(PLAYER_VISIBLE_ITEM_1_PROPERTIES + (slot * MAX_VISIBLE_ITEM_OFFSET), 0, pItem->GetItemRandomPropertyId());
        SetUInt32Value(PLAYER_VISIBLE_ITEM_1_PROPERTIES + 1 + (slot * MAX_VISIBLE_ITEM_OFFSET), pItem->GetItemSuffixFactor());
    }
    else
    {
        SetGuidValue(PLAYER_VISIBLE_ITEM_1_CREATOR + (slot * MAX_VISIBLE_ITEM_OFFSET), ObjectGuid());

        int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * MAX_VISIBLE_ITEM_OFFSET);
        SetUInt32Value(VisibleBase + 0, 0);

        for (int i = 0; i < MAX_INSPECTED_ENCHANTMENT_SLOT; ++i)
            SetUInt32Value(VisibleBase + 1 + i, 0);

        SetUInt32Value(PLAYER_VISIBLE_ITEM_1_PROPERTIES + 0 + (slot * MAX_VISIBLE_ITEM_OFFSET), 0);
        SetUInt32Value(PLAYER_VISIBLE_ITEM_1_PROPERTIES + 1 + (slot * MAX_VISIBLE_ITEM_OFFSET), 0);
    }
}

void Player::VisualizeItem(uint8 slot, Item* pItem)
{
    if (!pItem)
        return;

    // check also  BIND_WHEN_PICKED_UP and BIND_QUEST_ITEM for .additem or .additemset case by GM (not binded at adding to inventory)
    if (pItem->GetProto()->Bonding == BIND_WHEN_EQUIPPED || pItem->GetProto()->Bonding == BIND_WHEN_PICKED_UP || pItem->GetProto()->Bonding == BIND_QUEST_ITEM)
        pItem->SetBinding(true);

    DEBUG_LOG("STORAGE: EquipItem slot = %u, item = %u", slot, pItem->GetEntry());

    m_items[slot] = pItem;
    SetGuidValue(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2), pItem->GetObjectGuid());
    pItem->SetGuidValue(ITEM_FIELD_CONTAINED, GetObjectGuid());
    pItem->SetGuidValue(ITEM_FIELD_OWNER, GetObjectGuid());
    pItem->SetSlot(slot);
    pItem->SetContainer(NULL);

    if (slot < EQUIPMENT_SLOT_END)
        SetVisibleItemSlot(slot, pItem);

    pItem->SetState(ITEM_CHANGED, this);
}

void Player::RemoveItem(uint8 bag, uint8 slot, bool update)
{
    // note: removeitem does not actually change the item
    // it only takes the item out of storage temporarily
    // note2: if removeitem is to be used for delinking
    // the item must be removed from the player's updatequeue

    Item* pItem = GetItemByPos(bag, slot);
    if (pItem)
    {
        DEBUG_LOG("STORAGE: RemoveItem bag = %u, slot = %u, item = %u", bag, slot, pItem->GetEntry());

        RemoveEnchantmentDurations(pItem);
        RemoveItemDurations(pItem);

        if (bag == INVENTORY_SLOT_BAG_0)
        {
            if (slot < INVENTORY_SLOT_BAG_END)
            {
                ItemPrototype const* pProto = pItem->GetProto();
                // item set bonuses applied only at equip and removed at unequip, and still active for broken items

                if (pProto && pProto->ItemSet)
                    RemoveItemsSetItem(this, pProto);

                _ApplyItemMods(pItem, slot, false);

                // remove item dependent auras and casts (only weapon and armor slots)
                if (slot < EQUIPMENT_SLOT_END)
                {
                    RemoveItemDependentAurasAndCasts(pItem);

                    // remove held enchantments
                    if (slot == EQUIPMENT_SLOT_MAINHAND)
                        pItem->ClearEnchantment(PROP_ENCHANTMENT_SLOT_3);
                }
            }

            m_items[slot] = NULL;
            SetGuidValue(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2), ObjectGuid());

            if (slot < EQUIPMENT_SLOT_END)
                SetVisibleItemSlot(slot, NULL);
        }
        else
        {
            Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
            if (pBag)
                pBag->RemoveItem(slot, update);
        }
        pItem->SetGuidValue(ITEM_FIELD_CONTAINED, ObjectGuid());
        // pItem->SetGuidValue(ITEM_FIELD_OWNER, ObjectGuid()); not clear owner at remove (it will be set at store). This used in mail and auction code
        pItem->SetSlot(NULL_SLOT);
        if (IsInWorld() && update)
            pItem->SendCreateUpdateToPlayer(this);
    }
}

// Common operation need to remove item from inventory without delete in trade, auction, guild bank, mail....
void Player::MoveItemFromInventory(uint8 bag, uint8 slot, bool update)
{
    if (Item* it = GetItemByPos(bag, slot))
    {
        ItemRemovedQuestCheck(it->GetEntry(), it->GetCount());
        RemoveItem(bag, slot, update);
        it->RemoveFromUpdateQueueOf(this);
        if (it->IsInWorld())
        {
            it->RemoveFromWorld();
            it->DestroyForPlayer(this);
        }
    }
}

// Common operation need to add item from inventory without delete in trade, guild bank, mail....
void Player::MoveItemToInventory(ItemPosCountVec const& dest, Item* pItem, bool update, bool in_characterInventoryDB)
{
    // update quest counters
    ItemAddedQuestCheck(pItem->GetEntry(), pItem->GetCount());

    // store item
    Item* pLastItem = StoreItem(dest, pItem, update);

    // only set if not merged to existing stack (pItem can be deleted already but we can compare pointers any way)
    if (pLastItem == pItem)
    {
        // update owner for last item (this can be original item with wrong owner
        if (pLastItem->GetOwnerGuid() != GetObjectGuid())
            pLastItem->SetOwnerGuid(GetObjectGuid());

        // if this original item then it need create record in inventory
        // in case trade we already have item in other player inventory
        pLastItem->SetState(in_characterInventoryDB ? ITEM_CHANGED : ITEM_NEW, this);
    }
}

void Player::DestroyItem(uint8 bag, uint8 slot, bool update)
{
    Item* pItem = GetItemByPos(bag, slot);
    if (pItem)
    {
        DEBUG_LOG("STORAGE: DestroyItem bag = %u, slot = %u, item = %u", bag, slot, pItem->GetEntry());

        // start from destroy contained items (only equipped bag can have its)
        if (pItem->IsBag() && pItem->IsEquipped())          // this also prevent infinity loop if empty bag stored in bag==slot
        {
            for (int i = 0; i < MAX_BAG_SIZE; ++i)
                DestroyItem(slot, i, update);
        }

        if (pItem->HasFlag(ITEM_FIELD_FLAGS, ITEM_DYNFLAG_WRAPPED))
        {
            static SqlStatementID delGifts ;

            SqlStatement stmt = CharacterDatabase.CreateStatement(delGifts, "DELETE FROM character_gifts WHERE item_guid = ?");
            stmt.PExecute(pItem->GetGUIDLow());
        }

        RemoveEnchantmentDurations(pItem);
        RemoveItemDurations(pItem);

        ItemRemovedQuestCheck(pItem->GetEntry(), pItem->GetCount());

        if (bag == INVENTORY_SLOT_BAG_0)
        {
            SetGuidValue(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2), ObjectGuid());

            // equipment and equipped bags can have applied bonuses
            if (slot < INVENTORY_SLOT_BAG_END)
            {
                ItemPrototype const* pProto = pItem->GetProto();

                // item set bonuses applied only at equip and removed at unequip, and still active for broken items
                if (pProto && pProto->ItemSet)
                    RemoveItemsSetItem(this, pProto);

                _ApplyItemMods(pItem, slot, false);
            }

            if (slot < EQUIPMENT_SLOT_END)
            {
                // remove item dependent auras and casts (only weapon and armor slots)
                RemoveItemDependentAurasAndCasts(pItem);

                // equipment visual show
                SetVisibleItemSlot(slot, NULL);
            }

            m_items[slot] = NULL;
        }
        else if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, bag))
            pBag->RemoveItem(slot, update);

        if (IsInWorld() && update)
        {
            pItem->RemoveFromWorld();
            pItem->DestroyForPlayer(this);
        }

        // pItem->SetOwnerGUID(0);
        pItem->SetGuidValue(ITEM_FIELD_CONTAINED, ObjectGuid());
        pItem->SetSlot(NULL_SLOT);
        pItem->SetState(ITEM_REMOVED, this);
    }
}

void Player::DestroyItemCount(uint32 item, uint32 count, bool update, bool unequip_check)
{
    DEBUG_LOG("STORAGE: DestroyItemCount item = %u, count = %u", item, count);
    uint32 remcount = 0;

    // in inventory
    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (pItem->GetEntry() == item && !pItem->IsInTrade())
            {
                if (pItem->GetCount() + remcount <= count)
                {
                    // all items in inventory can unequipped
                    remcount += pItem->GetCount();
                    DestroyItem(INVENTORY_SLOT_BAG_0, i, update);

                    if (remcount >= count)
                        return;
                }
                else
                {
                    ItemRemovedQuestCheck(pItem->GetEntry(), count - remcount);
                    pItem->SetCount(pItem->GetCount() - count + remcount);
                    if (IsInWorld() && update)
                        pItem->SendCreateUpdateToPlayer(this);
                    pItem->SetState(ITEM_CHANGED, this);
                    return;
                }
            }
        }
    }

    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
    {
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (pItem->GetEntry() == item && !pItem->IsInTrade())
            {
                if (pItem->GetCount() + remcount <= count)
                {
                    // all keys can be unequipped
                    remcount += pItem->GetCount();
                    DestroyItem(INVENTORY_SLOT_BAG_0, i, update);

                    if (remcount >= count)
                        return;
                }
                else
                {
                    ItemRemovedQuestCheck(pItem->GetEntry(), count - remcount);
                    pItem->SetCount(pItem->GetCount() - count + remcount);
                    if (IsInWorld() && update)
                        pItem->SendCreateUpdateToPlayer(this);
                    pItem->SetState(ITEM_CHANGED, this);
                    return;
                }
            }
        }
    }

    // in inventory bags
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
            {
                if (Item* pItem = pBag->GetItemByPos(j))
                {
                    if (pItem->GetEntry() == item && !pItem->IsInTrade())
                    {
                        // all items in bags can be unequipped
                        if (pItem->GetCount() + remcount <= count)
                        {
                            remcount += pItem->GetCount();
                            DestroyItem(i, j, update);

                            if (remcount >= count)
                                return;
                        }
                        else
                        {
                            ItemRemovedQuestCheck(pItem->GetEntry(), count - remcount);
                            pItem->SetCount(pItem->GetCount() - count + remcount);
                            if (IsInWorld() && update)
                                pItem->SendCreateUpdateToPlayer(this);
                            pItem->SetState(ITEM_CHANGED, this);
                            return;
                        }
                    }
                }
            }
        }
    }

    // in equipment and bag list
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (pItem && pItem->GetEntry() == item && !pItem->IsInTrade())
            {
                if (pItem->GetCount() + remcount <= count)
                {
                    if (!unequip_check || CanUnequipItem(INVENTORY_SLOT_BAG_0 << 8 | i, false) == EQUIP_ERR_OK)
                    {
                        remcount += pItem->GetCount();
                        DestroyItem(INVENTORY_SLOT_BAG_0, i, update);

                        if (remcount >= count)
                            return;
                    }
                }
                else
                {
                    ItemRemovedQuestCheck(pItem->GetEntry(), count - remcount);
                    pItem->SetCount(pItem->GetCount() - count + remcount);
                    if (IsInWorld() && update)
                        pItem->SendCreateUpdateToPlayer(this);
                    pItem->SetState(ITEM_CHANGED, this);
                    return;
                }
            }
        }
    }
}

void Player::DestroyZoneLimitedItem(bool update, uint32 new_zone)
{
    DEBUG_LOG("STORAGE: DestroyZoneLimitedItem in map %u and area %u", GetMapId(), new_zone);

    // in inventory
    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (pItem->IsLimitedToAnotherMapOrZone(GetMapId(), new_zone))
                DestroyItem(INVENTORY_SLOT_BAG_0, i, update);

    for (int i = KEYRING_SLOT_START; i < KEYRING_SLOT_END; ++i)
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (pItem->IsLimitedToAnotherMapOrZone(GetMapId(), new_zone))
                DestroyItem(INVENTORY_SLOT_BAG_0, i, update);

    // in inventory bags
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                if (Item* pItem = pBag->GetItemByPos(j))
                    if (pItem->IsLimitedToAnotherMapOrZone(GetMapId(), new_zone))
                        DestroyItem(i, j, update);

    // in equipment and bag list
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (pItem->IsLimitedToAnotherMapOrZone(GetMapId(), new_zone))
                DestroyItem(INVENTORY_SLOT_BAG_0, i, update);
}

void Player::DestroyConjuredItems(bool update)
{
    // destroys all conjured items
    DEBUG_LOG("STORAGE: DestroyConjuredItems");

    // in inventory
    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (pItem->IsConjuredConsumable())
                DestroyItem(INVENTORY_SLOT_BAG_0, i, update);

    // in inventory bags
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                if (Item* pItem = pBag->GetItemByPos(j))
                    if (pItem->IsConjuredConsumable())
                        DestroyItem(i, j, update);

    // in equipment and bag list
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (pItem->IsConjuredConsumable())
                DestroyItem(INVENTORY_SLOT_BAG_0, i, update);
}

void Player::DestroyItemCount(Item* pItem, uint32& count, bool update)
{
    if (!pItem)
        return;

    DEBUG_LOG("STORAGE: DestroyItemCount item (GUID: %u, Entry: %u) count = %u", pItem->GetGUIDLow(), pItem->GetEntry(), count);

    if (pItem->GetCount() <= count)
    {
        count -= pItem->GetCount();

        DestroyItem(pItem->GetBagSlot(), pItem->GetSlot(), update);
    }
    else
    {
        ItemRemovedQuestCheck(pItem->GetEntry(), count);
        pItem->SetCount(pItem->GetCount() - count);
        count = 0;
        if (IsInWorld() && update)
            pItem->SendCreateUpdateToPlayer(this);
        pItem->SetState(ITEM_CHANGED, this);
    }
}

void Player::SplitItem(uint16 src, uint16 dst, uint32 count)
{
    uint8 srcbag = src >> 8;
    uint8 srcslot = src & 255;

    uint8 dstbag = dst >> 8;
    uint8 dstslot = dst & 255;

    Item* pSrcItem = GetItemByPos(srcbag, srcslot);
    if (!pSrcItem)
    {
        SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pSrcItem, NULL);
        return;
    }

    if (pSrcItem->HasGeneratedLoot())                       // prevent split looting item (stackable items can has only temporary loot and this meaning that loot window open)
    {
        // best error message found for attempting to split while looting
        SendEquipError(EQUIP_ERR_COULDNT_SPLIT_ITEMS, pSrcItem, NULL);
        return;
    }

    // not let split all items (can be only at cheating)
    if (pSrcItem->GetCount() == count)
    {
        SendEquipError(EQUIP_ERR_COULDNT_SPLIT_ITEMS, pSrcItem, NULL);
        return;
    }

    // not let split more existing items (can be only at cheating)
    if (pSrcItem->GetCount() < count)
    {
        SendEquipError(EQUIP_ERR_TRIED_TO_SPLIT_MORE_THAN_COUNT, pSrcItem, NULL);
        return;
    }

    DEBUG_LOG("STORAGE: SplitItem bag = %u, slot = %u, item = %u, count = %u", dstbag, dstslot, pSrcItem->GetEntry(), count);
    Item* pNewItem = pSrcItem->CloneItem(count, this);
    if (!pNewItem)
    {
        SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pSrcItem, NULL);
        return;
    }

    if (IsInventoryPos(dst))
    {
        // change item amount before check (for unique max count check)
        pSrcItem->SetCount(pSrcItem->GetCount() - count);

        ItemPosCountVec dest;
        InventoryResult msg = CanStoreItem(dstbag, dstslot, dest, pNewItem, false);
        if (msg != EQUIP_ERR_OK)
        {
            delete pNewItem;
            pSrcItem->SetCount(pSrcItem->GetCount() + count);
            SendEquipError(msg, pSrcItem, NULL);
            return;
        }

        if (IsInWorld())
            pSrcItem->SendCreateUpdateToPlayer(this);
        pSrcItem->SetState(ITEM_CHANGED, this);
        StoreItem(dest, pNewItem, true);
    }
    else if (IsBankPos(dst))
    {
        // change item amount before check (for unique max count check)
        pSrcItem->SetCount(pSrcItem->GetCount() - count);

        ItemPosCountVec dest;
        InventoryResult msg = CanBankItem(dstbag, dstslot, dest, pNewItem, false);
        if (msg != EQUIP_ERR_OK)
        {
            delete pNewItem;
            pSrcItem->SetCount(pSrcItem->GetCount() + count);
            SendEquipError(msg, pSrcItem, NULL);
            return;
        }

        if (IsInWorld())
            pSrcItem->SendCreateUpdateToPlayer(this);
        pSrcItem->SetState(ITEM_CHANGED, this);
        BankItem(dest, pNewItem, true);
    }
    else if (IsEquipmentPos(dst))
    {
        // change item amount before check (for unique max count check), provide space for splitted items
        pSrcItem->SetCount(pSrcItem->GetCount() - count);

        uint16 dest;
        InventoryResult msg = CanEquipItem(dstslot, dest, pNewItem, false);
        if (msg != EQUIP_ERR_OK)
        {
            delete pNewItem;
            pSrcItem->SetCount(pSrcItem->GetCount() + count);
            SendEquipError(msg, pSrcItem, NULL);
            return;
        }

        if (IsInWorld())
            pSrcItem->SendCreateUpdateToPlayer(this);
        pSrcItem->SetState(ITEM_CHANGED, this);
        EquipItem(dest, pNewItem, true);
        AutoUnequipOffhandIfNeed();
    }
}

void Player::SwapItem(uint16 src, uint16 dst)
{
    uint8 srcbag = src >> 8;
    uint8 srcslot = src & 255;

    uint8 dstbag = dst >> 8;
    uint8 dstslot = dst & 255;

    Item* pSrcItem = GetItemByPos(srcbag, srcslot);
    Item* pDstItem = GetItemByPos(dstbag, dstslot);

    if (!pSrcItem)
        return;

    DEBUG_LOG("STORAGE: SwapItem bag = %u, slot = %u, item = %u", dstbag, dstslot, pSrcItem->GetEntry());

    if (!isAlive())
    {
        SendEquipError(EQUIP_ERR_YOU_ARE_DEAD, pSrcItem, pDstItem);
        return;
    }

    // SRC checks

    // check unequip potability for equipped items and bank bags
    if (IsEquipmentPos(src) || IsBagPos(src))
    {
        // bags can be swapped with empty bag slots, or with empty bag (items move possibility checked later)
        InventoryResult msg = CanUnequipItem(src, !IsBagPos(src) || IsBagPos(dst) || (pDstItem && pDstItem->IsBag() && ((Bag*)pDstItem)->IsEmpty()));
        if (msg != EQUIP_ERR_OK)
        {
            SendEquipError(msg, pSrcItem, pDstItem);
            return;
        }
    }

    // prevent put equipped/bank bag in self
    if (IsBagPos(src) && srcslot == dstbag)
    {
        SendEquipError(EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG, pSrcItem, pDstItem);
        return;
    }

    // prevent put equipped/bank bag in self
    if (IsBagPos(dst) && dstslot == srcbag)
    {
        SendEquipError(EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG, pDstItem, pSrcItem);
        return;
    }

    // DST checks

    if (pDstItem)
    {
        // check unequip potability for equipped items and bank bags
        if (IsEquipmentPos(dst) || IsBagPos(dst))
        {
            // bags can be swapped with empty bag slots, or with empty bag (items move possibility checked later)
            InventoryResult msg = CanUnequipItem(dst, !IsBagPos(dst) || IsBagPos(src) || (pSrcItem->IsBag() && ((Bag*)pSrcItem)->IsEmpty()));
            if (msg != EQUIP_ERR_OK)
            {
                SendEquipError(msg, pSrcItem, pDstItem);
                return;
            }
        }
    }

    // NOW this is or item move (swap with empty), or swap with another item (including bags in bag possitions)
    // or swap empty bag with another empty or not empty bag (with items exchange)

    // Move case
    if (!pDstItem)
    {
        if (IsInventoryPos(dst))
        {
            ItemPosCountVec dest;
            InventoryResult msg = CanStoreItem(dstbag, dstslot, dest, pSrcItem, false);
            if (msg != EQUIP_ERR_OK)
            {
                SendEquipError(msg, pSrcItem, NULL);
                return;
            }

            RemoveItem(srcbag, srcslot, true);
            StoreItem(dest, pSrcItem, true);
        }
        else if (IsBankPos(dst))
        {
            ItemPosCountVec dest;
            InventoryResult msg = CanBankItem(dstbag, dstslot, dest, pSrcItem, false);
            if (msg != EQUIP_ERR_OK)
            {
                SendEquipError(msg, pSrcItem, NULL);
                return;
            }

            RemoveItem(srcbag, srcslot, true);
            BankItem(dest, pSrcItem, true);
        }
        else if (IsEquipmentPos(dst))
        {
            uint16 dest;
            InventoryResult msg = CanEquipItem(dstslot, dest, pSrcItem, false);
            if (msg != EQUIP_ERR_OK)
            {
                SendEquipError(msg, pSrcItem, NULL);
                return;
            }

            RemoveItem(srcbag, srcslot, true);
            EquipItem(dest, pSrcItem, true);
            AutoUnequipOffhandIfNeed();
        }

        return;
    }

    // attempt merge to / fill target item
    if (!pSrcItem->IsBag() && !pDstItem->IsBag())
    {
        InventoryResult msg;
        ItemPosCountVec sDest;
        uint16 eDest;
        if (IsInventoryPos(dst))
            msg = CanStoreItem(dstbag, dstslot, sDest, pSrcItem, false);
        else if (IsBankPos(dst))
            msg = CanBankItem(dstbag, dstslot, sDest, pSrcItem, false);
        else if (IsEquipmentPos(dst))
            msg = CanEquipItem(dstslot, eDest, pSrcItem, false);
        else
            return;

        // can be merge/fill
        if (msg == EQUIP_ERR_OK)
        {
            if (pSrcItem->GetCount() + pDstItem->GetCount() <= pSrcItem->GetProto()->GetMaxStackSize())
            {
                RemoveItem(srcbag, srcslot, true);

                if (IsInventoryPos(dst))
                    StoreItem(sDest, pSrcItem, true);
                else if (IsBankPos(dst))
                    BankItem(sDest, pSrcItem, true);
                else if (IsEquipmentPos(dst))
                {
                    EquipItem(eDest, pSrcItem, true);
                    AutoUnequipOffhandIfNeed();
                }
            }
            else
            {
                pSrcItem->SetCount(pSrcItem->GetCount() + pDstItem->GetCount() - pSrcItem->GetProto()->GetMaxStackSize());
                pDstItem->SetCount(pSrcItem->GetProto()->GetMaxStackSize());
                pSrcItem->SetState(ITEM_CHANGED, this);
                pDstItem->SetState(ITEM_CHANGED, this);
                if (IsInWorld())
                {
                    pSrcItem->SendCreateUpdateToPlayer(this);
                    pDstItem->SendCreateUpdateToPlayer(this);
                }
            }
            return;
        }
    }

    // impossible merge/fill, do real swap
    InventoryResult msg;

    // check src->dest move possibility
    ItemPosCountVec sDest;
    uint16 eDest = 0;
    if (IsInventoryPos(dst))
        msg = CanStoreItem(dstbag, dstslot, sDest, pSrcItem, true);
    else if (IsBankPos(dst))
        msg = CanBankItem(dstbag, dstslot, sDest, pSrcItem, true);
    else if (IsEquipmentPos(dst))
    {
        msg = CanEquipItem(dstslot, eDest, pSrcItem, true);
        if (msg == EQUIP_ERR_OK)
            msg = CanUnequipItem(eDest, true);
    }

    if (msg != EQUIP_ERR_OK)
    {
        SendEquipError(msg, pSrcItem, pDstItem);
        return;
    }

    // check dest->src move possibility
    ItemPosCountVec sDest2;
    uint16 eDest2 = 0;
    if (IsInventoryPos(src))
        msg = CanStoreItem(srcbag, srcslot, sDest2, pDstItem, true);
    else if (IsBankPos(src))
        msg = CanBankItem(srcbag, srcslot, sDest2, pDstItem, true);
    else if (IsEquipmentPos(src))
    {
        msg = CanEquipItem(srcslot, eDest2, pDstItem, true);
        if (msg == EQUIP_ERR_OK)
            msg = CanUnequipItem(eDest2, true);
    }

    if (msg != EQUIP_ERR_OK)
    {
        SendEquipError(msg, pDstItem, pSrcItem);
        return;
    }

    // Check bag swap with item exchange (one from empty in not bag possition (equipped (not possible in fact) or store)
    if (pSrcItem->IsBag() && pDstItem->IsBag())
    {
        Bag* emptyBag = NULL;
        Bag* fullBag = NULL;
        if (((Bag*)pSrcItem)->IsEmpty() && !IsBagPos(src))
        {
            emptyBag = (Bag*)pSrcItem;
            fullBag  = (Bag*)pDstItem;
        }
        else if (((Bag*)pDstItem)->IsEmpty() && !IsBagPos(dst))
        {
            emptyBag = (Bag*)pDstItem;
            fullBag  = (Bag*)pSrcItem;
        }

        // bag swap (with items exchange) case
        if (emptyBag && fullBag)
        {
            ItemPrototype const* emotyProto = emptyBag->GetProto();

            uint32 count = 0;

            for (uint32 i = 0; i < fullBag->GetBagSize(); ++i)
            {
                Item* bagItem = fullBag->GetItemByPos(i);
                if (!bagItem)
                    continue;

                ItemPrototype const* bagItemProto = bagItem->GetProto();
                if (!bagItemProto || !ItemCanGoIntoBag(bagItemProto, emotyProto))
                {
                    // one from items not go to empty target bag
                    SendEquipError(EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG, pSrcItem, pDstItem);
                    return;
                }

                ++count;
            }

            if (count > emptyBag->GetBagSize())
            {
                // too small targeted bag
                SendEquipError(EQUIP_ERR_ITEMS_CANT_BE_SWAPPED, pSrcItem, pDstItem);
                return;
            }

            // Items swap
            count = 0;                                      // will pos in new bag
            for (uint32 i = 0; i < fullBag->GetBagSize(); ++i)
            {
                Item* bagItem = fullBag->GetItemByPos(i);
                if (!bagItem)
                    continue;

                fullBag->RemoveItem(i, true);
                emptyBag->StoreItem(count, bagItem, true);
                bagItem->SetState(ITEM_CHANGED, this);

                ++count;
            }
        }
    }

    // now do moves, remove...
    RemoveItem(dstbag, dstslot, false);
    RemoveItem(srcbag, srcslot, false);

    // add to dest
    if (IsInventoryPos(dst))
        StoreItem(sDest, pSrcItem, true);
    else if (IsBankPos(dst))
        BankItem(sDest, pSrcItem, true);
    else if (IsEquipmentPos(dst))
        EquipItem(eDest, pSrcItem, true);

    // add to src
    if (IsInventoryPos(src))
        StoreItem(sDest2, pDstItem, true);
    else if (IsBankPos(src))
        BankItem(sDest2, pDstItem, true);
    else if (IsEquipmentPos(src))
        EquipItem(eDest2, pDstItem, true);

    AutoUnequipOffhandIfNeed();
}

void Player::AddItemToBuyBackSlot(Item* pItem)
{
    if (pItem)
    {
        uint32 slot = m_currentBuybackSlot;
        // if current back slot non-empty search oldest or free
        if (m_items[slot])
        {
            uint32 oldest_time = GetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1);
            uint32 oldest_slot = BUYBACK_SLOT_START;

            for (uint32 i = BUYBACK_SLOT_START + 1; i < BUYBACK_SLOT_END; ++i)
            {
                // found empty
                if (!m_items[i])
                {
                    slot = i;
                    break;
                }

                uint32 i_time = GetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + i - BUYBACK_SLOT_START);

                if (oldest_time > i_time)
                {
                    oldest_time = i_time;
                    oldest_slot = i;
                }
            }

            // find oldest
            slot = oldest_slot;
        }

        RemoveItemFromBuyBackSlot(slot, true);
        DEBUG_LOG("STORAGE: AddItemToBuyBackSlot item = %u, slot = %u", pItem->GetEntry(), slot);

        m_items[slot] = pItem;
        time_t base = time(NULL);
        uint32 etime = uint32(base - m_logintime + (30 * 3600));
        uint32 eslot = slot - BUYBACK_SLOT_START;

        SetGuidValue(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + (eslot * 2), pItem->GetObjectGuid());
        if (ItemPrototype const* pProto = pItem->GetProto())
            SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + eslot, pProto->SellPrice * pItem->GetCount());
        else
            SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + eslot, 0);
        SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + eslot, (uint32)etime);

        // move to next (for non filled list is move most optimized choice)
        if (m_currentBuybackSlot < BUYBACK_SLOT_END - 1)
            ++m_currentBuybackSlot;
    }
}

Item* Player::GetItemFromBuyBackSlot(uint32 slot)
{
    DEBUG_LOG("STORAGE: GetItemFromBuyBackSlot slot = %u", slot);
    if (slot >= BUYBACK_SLOT_START && slot < BUYBACK_SLOT_END)
        return m_items[slot];
    return NULL;
}

void Player::RemoveItemFromBuyBackSlot(uint32 slot, bool del)
{
    DEBUG_LOG("STORAGE: RemoveItemFromBuyBackSlot slot = %u", slot);
    if (slot >= BUYBACK_SLOT_START && slot < BUYBACK_SLOT_END)
    {
        Item* pItem = m_items[slot];
        if (pItem)
        {
            pItem->RemoveFromWorld();
            if (del) pItem->SetState(ITEM_REMOVED, this);
        }

        m_items[slot] = NULL;

        uint32 eslot = slot - BUYBACK_SLOT_START;
        SetGuidValue(PLAYER_FIELD_VENDORBUYBACK_SLOT_1 + (eslot * 2), ObjectGuid());
        SetUInt32Value(PLAYER_FIELD_BUYBACK_PRICE_1 + eslot, 0);
        SetUInt32Value(PLAYER_FIELD_BUYBACK_TIMESTAMP_1 + eslot, 0);

        // if current backslot is filled set to now free slot
        if (m_items[m_currentBuybackSlot])
            m_currentBuybackSlot = slot;
    }
}

void Player::SendEquipError(InventoryResult msg, Item* pItem, Item* pItem2, uint32 itemid /*= 0*/) const
{
    DEBUG_LOG("WORLD: Sent SMSG_INVENTORY_CHANGE_FAILURE (%u)", msg);
    WorldPacket data(SMSG_INVENTORY_CHANGE_FAILURE, (msg == EQUIP_ERR_CANT_EQUIP_LEVEL_I ? 22 : (msg == EQUIP_ERR_OK ? 1 : 18)));
    data << uint8(msg);

    if (msg != EQUIP_ERR_OK)
    {
        if (msg == EQUIP_ERR_CANT_EQUIP_LEVEL_I)
        {
            ItemPrototype const* proto = pItem ? pItem->GetProto() : ObjectMgr::GetItemPrototype(itemid);
            data << uint32(proto ? proto->RequiredLevel : 0);
        }
        data << (pItem ? pItem->GetObjectGuid() : ObjectGuid());
        data << (pItem2 ? pItem2->GetObjectGuid() : ObjectGuid());
        data << uint8(0);                                   // bag type subclass, used with EQUIP_ERR_EVENT_AUTOEQUIP_BIND_CONFIRM and EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG2
    }
    GetSession()->SendPacket(&data);
}

void Player::SendBuyError(BuyResult msg, Creature* pCreature, uint32 item, uint32 param)
{
    DEBUG_LOG("WORLD: Sent SMSG_BUY_FAILED");
    WorldPacket data(SMSG_BUY_FAILED, (8 + 4 + 4 + 1));
    data << (pCreature ? pCreature->GetObjectGuid() : ObjectGuid());
    data << uint32(item);
    if (param > 0)
        data << uint32(param);
    data << uint8(msg);
    GetSession()->SendPacket(&data);
}

void Player::SendSellError(SellResult msg, Creature* pCreature, ObjectGuid itemGuid, uint32 param)
{
    DEBUG_LOG("WORLD: Sent SMSG_SELL_ITEM");
    WorldPacket data(SMSG_SELL_ITEM, (8 + 8 + (param ? 4 : 0) + 1)); // last check 2.0.10
    data << (pCreature ? pCreature->GetObjectGuid() : ObjectGuid());
    data << ObjectGuid(itemGuid);
    if (param > 0)
        data << uint32(param);
    data << uint8(msg);
    GetSession()->SendPacket(&data);
}

void Player::TradeCancel(bool sendback)
{
    if (m_trade)
    {
        Player* trader = m_trade->GetTrader();

        // send yellow "Trade canceled" message to both traders
        if (sendback)
            GetSession()->SendCancelTrade();

        trader->GetSession()->SendCancelTrade();

        // cleanup
        delete m_trade;
        m_trade = NULL;
        delete trader->m_trade;
        trader->m_trade = NULL;
    }
}

void Player::UpdateItemDuration(uint32 time, bool realtimeonly)
{
    if (m_itemDuration.empty())
        return;

    DEBUG_LOG("Player::UpdateItemDuration(%u,%u)", time, realtimeonly);

    for (ItemDurationList::const_iterator itr = m_itemDuration.begin(); itr != m_itemDuration.end();)
    {
        Item* item = *itr;
        ++itr;                                              // current element can be erased in UpdateDuration

        if ((realtimeonly && (item->GetProto()->ExtraFlags & ITEM_EXTRA_REAL_TIME_DURATION)) || !realtimeonly)
            item->UpdateDuration(this, time);
    }
}

void Player::UpdateEnchantTime(uint32 time)
{
    for (EnchantDurationList::iterator itr = m_enchantDuration.begin(), next; itr != m_enchantDuration.end(); itr = next)
    {
        MANGOS_ASSERT(itr->item);
        next = itr;
        if (!itr->item->GetEnchantmentId(itr->slot))
        {
            next = m_enchantDuration.erase(itr);
        }
        else if (itr->leftduration <= time)
        {
            ApplyEnchantment(itr->item, itr->slot, false, false);
            itr->item->ClearEnchantment(itr->slot);
            next = m_enchantDuration.erase(itr);
        }
        else if (itr->leftduration > time)
        {
            itr->leftduration -= time;
            ++next;
        }
    }
}

void Player::AddEnchantmentDurations(Item* item)
{
    for (int x = 0; x < MAX_ENCHANTMENT_SLOT; ++x)
    {
        if (!item->GetEnchantmentId(EnchantmentSlot(x)))
            continue;

        uint32 duration = item->GetEnchantmentDuration(EnchantmentSlot(x));
        if (duration > 0)
            AddEnchantmentDuration(item, EnchantmentSlot(x), duration);
    }
}

void Player::RemoveEnchantmentDurations(Item* item)
{
    for (EnchantDurationList::iterator itr = m_enchantDuration.begin(); itr != m_enchantDuration.end();)
    {
        if (itr->item == item)
        {
            // save duration in item
            item->SetEnchantmentDuration(EnchantmentSlot(itr->slot), itr->leftduration);
            itr = m_enchantDuration.erase(itr);
        }
        else
            ++itr;
    }
}

void Player::RemoveAllEnchantments(EnchantmentSlot slot)
{
    // remove enchantments from equipped items first to clean up the m_enchantDuration list
    for (EnchantDurationList::iterator itr = m_enchantDuration.begin(), next; itr != m_enchantDuration.end(); itr = next)
    {
        next = itr;
        if (itr->slot == slot)
        {
            if (itr->item && itr->item->GetEnchantmentId(slot))
            {
                // remove from stats
                ApplyEnchantment(itr->item, slot, false, false);
                // remove visual
                itr->item->ClearEnchantment(slot);
            }
            // remove from update list
            next = m_enchantDuration.erase(itr);
        }
        else
            ++next;
    }

    // remove enchants from inventory items
    // NOTE: no need to remove these from stats, since these aren't equipped
    // in inventory
    for (int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (pItem->GetEnchantmentId(slot))
                pItem->ClearEnchantment(slot);

    // in inventory bags
    for (int i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Bag* pBag = (Bag*)GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                if (Item* pItem = pBag->GetItemByPos(j))
                    if (pItem->GetEnchantmentId(slot))
                        pItem->ClearEnchantment(slot);
}

// duration == 0 will remove item enchant
void Player::AddEnchantmentDuration(Item* item, EnchantmentSlot slot, uint32 duration)
{
    if (!item)
        return;

    if (slot >= MAX_ENCHANTMENT_SLOT)
        return;

    for (EnchantDurationList::iterator itr = m_enchantDuration.begin(); itr != m_enchantDuration.end(); ++itr)
    {
        if (itr->item == item && itr->slot == slot)
        {
            itr->item->SetEnchantmentDuration(itr->slot, itr->leftduration);
            m_enchantDuration.erase(itr);
            break;
        }
    }
    if (item && duration > 0)
    {
        GetSession()->SendItemEnchantTimeUpdate(GetObjectGuid(), item->GetObjectGuid(), slot, uint32(duration / 1000));
        m_enchantDuration.push_back(EnchantDuration(item, slot, duration));
    }
}

void Player::ApplyEnchantment(Item* item, bool apply)
{
    for (uint32 slot = 0; slot < MAX_ENCHANTMENT_SLOT; ++slot)
        ApplyEnchantment(item, EnchantmentSlot(slot), apply);
}

void Player::ApplyEnchantment(Item* item, EnchantmentSlot slot, bool apply, bool apply_dur, bool ignore_condition)
{
    if (!item)
        return;

    if (!item->IsEquipped())
        return;

    if (slot >= MAX_ENCHANTMENT_SLOT)
        return;

    uint32 enchant_id = item->GetEnchantmentId(slot);
    if (!enchant_id)
        return;

    SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
    if (!pEnchant)
        return;

    if (!item->IsBroken())
    {
        for (int s = 0; s < 3; ++s)
        {
            uint32 enchant_display_type = pEnchant->type[s];
            uint32 enchant_amount = pEnchant->amount[s];
            uint32 enchant_spell_id = pEnchant->spellid[s];

            switch (enchant_display_type)
            {
                case ITEM_ENCHANTMENT_TYPE_NONE:
                    break;
                case ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL:
                    // processed in Player::CastItemCombatSpell
                    break;
                case ITEM_ENCHANTMENT_TYPE_DAMAGE:
                    if (item->GetSlot() == EQUIPMENT_SLOT_MAINHAND)
                        HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_VALUE, float(enchant_amount), apply);
                    else if (item->GetSlot() == EQUIPMENT_SLOT_OFFHAND)
                        HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_VALUE, float(enchant_amount), apply);
                    else if (item->GetSlot() == EQUIPMENT_SLOT_RANGED)
                        HandleStatModifier(UNIT_MOD_DAMAGE_RANGED, TOTAL_VALUE, float(enchant_amount), apply);
                    break;
                case ITEM_ENCHANTMENT_TYPE_EQUIP_SPELL:
                {
                    if (enchant_spell_id)
                    {
                        if (apply)
                            CastSpell(this, enchant_spell_id, true, item);
                        else
                            RemoveAurasDueToItemSpell(item, enchant_spell_id);
                    }
                    break;
                }
                case ITEM_ENCHANTMENT_TYPE_RESISTANCE:
                    HandleStatModifier(UnitMods(UNIT_MOD_RESISTANCE_START + enchant_spell_id), TOTAL_VALUE, float(enchant_amount), apply);
                    break;
                case ITEM_ENCHANTMENT_TYPE_STAT:
                {
                    DEBUG_LOG("Adding %u to stat nb %u", enchant_amount, enchant_spell_id);
                    switch (enchant_spell_id)
                    {
                        case ITEM_MOD_MANA:
                            DEBUG_LOG("+ %u MANA", enchant_amount);
                            HandleStatModifier(UNIT_MOD_MANA, BASE_VALUE, float(enchant_amount), apply);
                            break;
                        case ITEM_MOD_HEALTH:
                            DEBUG_LOG("+ %u HEALTH", enchant_amount);
                            HandleStatModifier(UNIT_MOD_HEALTH, BASE_VALUE, float(enchant_amount), apply);
                            break;
                        case ITEM_MOD_AGILITY:
                            DEBUG_LOG("+ %u AGILITY", enchant_amount);
                            HandleStatModifier(UNIT_MOD_STAT_AGILITY, TOTAL_VALUE, float(enchant_amount), apply);
                            ApplyStatBuffMod(STAT_AGILITY, float(enchant_amount), apply);
                            break;
                        case ITEM_MOD_STRENGTH:
                            DEBUG_LOG("+ %u STRENGTH", enchant_amount);
                            HandleStatModifier(UNIT_MOD_STAT_STRENGTH, TOTAL_VALUE, float(enchant_amount), apply);
                            ApplyStatBuffMod(STAT_STRENGTH, float(enchant_amount), apply);
                            break;
                        case ITEM_MOD_INTELLECT:
                            DEBUG_LOG("+ %u INTELLECT", enchant_amount);
                            HandleStatModifier(UNIT_MOD_STAT_INTELLECT, TOTAL_VALUE, float(enchant_amount), apply);
                            ApplyStatBuffMod(STAT_INTELLECT, float(enchant_amount), apply);
                            break;
                        case ITEM_MOD_SPIRIT:
                            DEBUG_LOG("+ %u SPIRIT", enchant_amount);
                            HandleStatModifier(UNIT_MOD_STAT_SPIRIT, TOTAL_VALUE, float(enchant_amount), apply);
                            ApplyStatBuffMod(STAT_SPIRIT, float(enchant_amount), apply);
                            break;
                        case ITEM_MOD_STAMINA:
                            DEBUG_LOG("+ %u STAMINA", enchant_amount);
                            HandleStatModifier(UNIT_MOD_STAT_STAMINA, TOTAL_VALUE, float(enchant_amount), apply);
                            ApplyStatBuffMod(STAT_STAMINA, float(enchant_amount), apply);
                            break;
                        default:
                            break;
                    }
                    break;
                }
                case ITEM_ENCHANTMENT_TYPE_TOTEM:           // Shaman Rockbiter Weapon
                {
                    if (getClass() == CLASS_SHAMAN)
                    {
                        float addValue = 0.0f;
                        if (item->GetSlot() == EQUIPMENT_SLOT_MAINHAND)
                        {
                            addValue = float(enchant_amount * item->GetProto()->Delay / 1000.0f);
                            HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_VALUE, addValue, apply);
                        }
                        else if (item->GetSlot() == EQUIPMENT_SLOT_OFFHAND)
                        {
                            addValue = float(enchant_amount * item->GetProto()->Delay / 1000.0f);
                            HandleStatModifier(UNIT_MOD_DAMAGE_OFFHAND, TOTAL_VALUE, addValue, apply);
                        }
                    }
                    break;
                }
                default:
                    sLog.outError("Unknown item enchantment (id = %d) display type: %d", enchant_id, enchant_display_type);
                    break;
            }                                               /*switch(enchant_display_type)*/
        }                                                   /*for*/
    }

    // visualize enchantment at player and equipped items
    if (slot < MAX_INSPECTED_ENCHANTMENT_SLOT)
    {
        int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (item->GetSlot() * MAX_VISIBLE_ITEM_OFFSET);
        SetUInt32Value(VisibleBase + 1 + slot, apply ? item->GetEnchantmentId(slot) : 0);
    }

    if (apply_dur)
    {
        if (apply)
        {
            // set duration
            uint32 duration = item->GetEnchantmentDuration(slot);
            if (duration > 0)
                AddEnchantmentDuration(item, slot, duration);
        }
        else
        {
            // duration == 0 will remove EnchantDuration
            AddEnchantmentDuration(item, slot, 0);
        }
    }
}

void Player::SendEnchantmentDurations()
{
    for (EnchantDurationList::const_iterator itr = m_enchantDuration.begin(); itr != m_enchantDuration.end(); ++itr)
    {
        GetSession()->SendItemEnchantTimeUpdate(GetObjectGuid(), itr->item->GetObjectGuid(), itr->slot, uint32(itr->leftduration) / 1000);
    }
}

void Player::SendItemDurations()
{
    for (ItemDurationList::const_iterator itr = m_itemDuration.begin(); itr != m_itemDuration.end(); ++itr)
    {
        (*itr)->SendTimeUpdate(this);
    }
}

void Player::SendNewItem(Item* item, uint32 count, bool received, bool created, bool broadcast)
{
    if (!item)                                              // prevent crash
        return;

    // last check 2.0.10
    WorldPacket data(SMSG_ITEM_PUSH_RESULT, (8 + 4 + 4 + 4 + 1 + 4 + 4 + 4 + 4 + 4));
    data << GetObjectGuid();                                // player GUID
    data << uint32(received);                               // 0=looted, 1=from npc
    data << uint32(created);                                // 0=received, 1=created
    data << uint32(1);                                      // IsShowChatMessage
    data << uint8(item->GetBagSlot());                      // bagslot
    // item slot, but when added to stack: 0xFFFFFFFF
    data << uint32((item->GetCount() == count) ? item->GetSlot() : -1);
    data << uint32(item->GetEntry());                       // item id
    data << uint32(item->GetItemSuffixFactor());            // SuffixFactor
    data << uint32(item->GetItemRandomPropertyId());        // random item property id
    data << uint32(count);                                  // count of items
    data << uint32(GetItemCount(item->GetEntry()));         // count of items in inventory

    if (broadcast && GetGroup())
        GetGroup()->BroadcastPacket(&data, true);
    else
        GetSession()->SendPacket(&data);
}

/*********************************************************/
/***                    GOSSIP SYSTEM                  ***/
/*********************************************************/

void Player::PrepareGossipMenu(WorldObject* pSource, uint32 menuId)
{
    PlayerMenu* pMenu = PlayerTalkClass;
    pMenu->ClearMenus();

    pMenu->GetGossipMenu().SetMenuId(menuId);

    GossipMenuItemsMapBounds pMenuItemBounds = sObjectMgr.GetGossipMenuItemsMapBounds(menuId);

    // prepares quest menu when true
    bool canSeeQuests = menuId == GetDefaultGossipMenuForSource(pSource);

    // if canSeeQuests (the default, top level menu) and no menu options exist for this, use options from default options
    if (pMenuItemBounds.first == pMenuItemBounds.second && canSeeQuests)
        pMenuItemBounds = sObjectMgr.GetGossipMenuItemsMapBounds(0);

    for (GossipMenuItemsMap::const_iterator itr = pMenuItemBounds.first; itr != pMenuItemBounds.second; ++itr)
    {
        bool hasMenuItem = true;
        bool isGMSkipConditionCheck = false;

        if (itr->second.conditionId && !sObjectMgr.IsPlayerMeetToCondition(itr->second.conditionId, this, GetMap(), pSource, CONDITION_FROM_GOSSIP_OPTION))
        {
            if (isGameMaster())                             // Let GM always see menu items regardless of conditions
                isGMSkipConditionCheck = true;
            else
            {
                if (itr->second.option_id == GOSSIP_OPTION_QUESTGIVER)
                    canSeeQuests = false;
                continue;                                   // Skip this option
            }
        }

        if (pSource->GetTypeId() == TYPEID_UNIT)
        {
            Creature* pCreature = (Creature*)pSource;

            uint32 npcflags = pCreature->GetUInt32Value(UNIT_NPC_FLAGS);

            if (!(itr->second.npc_option_npcflag & npcflags))
                continue;

            switch (itr->second.option_id)
            {
                case GOSSIP_OPTION_GOSSIP:
                    break;
                case GOSSIP_OPTION_QUESTGIVER:
                    hasMenuItem = false;
                    break;
                case GOSSIP_OPTION_ARMORER:
                    hasMenuItem = false;                    // added in special mode
                    break;
                case GOSSIP_OPTION_SPIRITHEALER:
                    if (!isDead())
                        hasMenuItem = false;
                    break;
                case GOSSIP_OPTION_VENDOR:
                {
                    VendorItemData const* vItems = pCreature->GetVendorItems();
                    VendorItemData const* tItems = pCreature->GetVendorTemplateItems();
                    if ((!vItems || vItems->Empty()) && (!tItems || tItems->Empty()))
                    {
                        sLog.outErrorDb("Creature %u (Entry: %u) have UNIT_NPC_FLAG_VENDOR but have empty trading item list.", pCreature->GetGUIDLow(), pCreature->GetEntry());
                        hasMenuItem = false;
                    }
                    break;
                }
                case GOSSIP_OPTION_TRAINER:
                    if (!pCreature->IsTrainerOf(this, false))
                        hasMenuItem = false;
                    break;
                case GOSSIP_OPTION_UNLEARNTALENTS:
                    if (!pCreature->CanTrainAndResetTalentsOf(this))
                        hasMenuItem = false;
                    break;
                case GOSSIP_OPTION_UNLEARNPETSKILLS:
                    if (!GetPet() || GetPet()->getPetType() != HUNTER_PET || GetPet()->m_spells.size() <= 1 || pCreature->GetCreatureInfo()->TrainerType != TRAINER_TYPE_PETS || pCreature->GetCreatureInfo()->TrainerClass != CLASS_HUNTER)
                        hasMenuItem = false;
                    break;
                case GOSSIP_OPTION_TAXIVENDOR:
                    if (GetSession()->SendLearnNewTaxiNode(pCreature))
                        return;
                    break;
                case GOSSIP_OPTION_BATTLEFIELD:
                    if (!pCreature->CanInteractWithBattleMaster(this, false))
                        hasMenuItem = false;
                    break;
                case GOSSIP_OPTION_STABLEPET:
                    if (getClass() != CLASS_HUNTER)
                        hasMenuItem = false;
                    break;
                case GOSSIP_OPTION_SPIRITGUIDE:
                case GOSSIP_OPTION_INNKEEPER:
                case GOSSIP_OPTION_BANKER:
                case GOSSIP_OPTION_PETITIONER:
                case GOSSIP_OPTION_TABARDDESIGNER:
                case GOSSIP_OPTION_AUCTIONEER:
                    break;                                  // no checks
                default:
                    sLog.outErrorDb("Creature entry %u have unknown gossip option %u for menu %u", pCreature->GetEntry(), itr->second.option_id, itr->second.menu_id);
                    hasMenuItem = false;
                    break;
            }
        }
        else if (pSource->GetTypeId() == TYPEID_GAMEOBJECT)
        {
            GameObject* pGo = (GameObject*)pSource;

            switch (itr->second.option_id)
            {
                case GOSSIP_OPTION_QUESTGIVER:
                    hasMenuItem = false;
                    break;
                case GOSSIP_OPTION_GOSSIP:
                    if (pGo->GetGoType() != GAMEOBJECT_TYPE_QUESTGIVER && pGo->GetGoType() != GAMEOBJECT_TYPE_GOOBER)
                        hasMenuItem = false;
                    break;
                default:
                    hasMenuItem = false;
                    break;
            }
        }

        if (hasMenuItem)
        {
            std::string strOptionText = itr->second.option_text;
            std::string strBoxText = itr->second.box_text;

            int loc_idx = GetSession()->GetSessionDbLocaleIndex();

            if (loc_idx >= 0)
            {
                uint32 idxEntry = MAKE_PAIR32(menuId, itr->second.id);

                if (GossipMenuItemsLocale const* no = sObjectMgr.GetGossipMenuItemsLocale(idxEntry))
                {
                    if (no->OptionText.size() > (size_t)loc_idx && !no->OptionText[loc_idx].empty())
                        strOptionText = no->OptionText[loc_idx];

                    if (no->BoxText.size() > (size_t)loc_idx && !no->BoxText[loc_idx].empty())
                        strBoxText = no->BoxText[loc_idx];
                }
            }

            if (isGMSkipConditionCheck)
            {
                strOptionText.append(" (");
                strOptionText.append(GetSession()->GetMangosString(LANG_GM_ON));
                strOptionText.append(")");
            }

            pMenu->GetGossipMenu().AddMenuItem(itr->second.option_icon, strOptionText, 0, itr->second.option_id, strBoxText, itr->second.box_coded);
            pMenu->GetGossipMenu().AddGossipMenuItemData(itr->second.action_menu_id, itr->second.action_poi_id, itr->second.action_script_id);
        }
    }

    if (canSeeQuests)
        PrepareQuestMenu(pSource->GetObjectGuid());

    // some gossips aren't handled in normal way ... so we need to do it this way .. TODO: handle it in normal way ;-)
    /*if (pMenu->Empty())
    {
        if (pCreature->HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_TRAINER))
        {
            // output error message if need
            pCreature->IsTrainerOf(this, true);
        }

        if (pCreature->HasFlag(UNIT_NPC_FLAGS,UNIT_NPC_FLAG_BATTLEMASTER))
        {
            // output error message if need
            pCreature->CanInteractWithBattleMaster(this, true);
        }
    }*/
}

void Player::SendPreparedGossip(WorldObject* pSource)
{
    if (!pSource)
        return;

    if (pSource->GetTypeId() == TYPEID_UNIT)
    {
        // in case no gossip flag and quest menu not empty, open quest menu (client expect gossip menu with this flag)
        if (!((Creature*)pSource)->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP) && !PlayerTalkClass->GetQuestMenu().Empty())
        {
            SendPreparedQuest(pSource->GetObjectGuid());
            return;
        }
    }
    else if (pSource->GetTypeId() == TYPEID_GAMEOBJECT)
    {
        // probably need to find a better way here
        if (!PlayerTalkClass->GetGossipMenu().GetMenuId() && !PlayerTalkClass->GetQuestMenu().Empty())
        {
            SendPreparedQuest(pSource->GetObjectGuid());
            return;
        }
    }

    // in case non empty gossip menu (that not included quests list size) show it
    // (quest entries from quest menu will be included in list)

    uint32 textId = GetGossipTextId(pSource);

    if (uint32 menuId = PlayerTalkClass->GetGossipMenu().GetMenuId())
        textId = GetGossipTextId(menuId, pSource);

    PlayerTalkClass->SendGossipMenu(textId, pSource->GetObjectGuid());
}

void Player::OnGossipSelect(WorldObject* pSource, uint32 gossipListId)
{
    GossipMenu& gossipmenu = PlayerTalkClass->GetGossipMenu();

    if (gossipListId >= gossipmenu.MenuItemCount())
        return;

    GossipMenuItem const&  menu_item = gossipmenu.GetItem(gossipListId);

    uint32 gossipOptionId = menu_item.m_gOptionId;
    ObjectGuid guid = pSource->GetObjectGuid();

    if (pSource->GetTypeId() == TYPEID_GAMEOBJECT)
    {
        if (gossipOptionId > GOSSIP_OPTION_QUESTGIVER)
        {
            sLog.outError("Player guid %u request invalid gossip option for GameObject entry %u", GetGUIDLow(), pSource->GetEntry());
            return;
        }
    }

    GossipMenuItemData pMenuData = gossipmenu.GetItemData(gossipListId);

    switch (gossipOptionId)
    {
        case GOSSIP_OPTION_GOSSIP:
        {
            if (pMenuData.m_gAction_poi)
                PlayerTalkClass->SendPointOfInterest(pMenuData.m_gAction_poi);

            // send new menu || close gossip || stay at current menu
            if (pMenuData.m_gAction_menu > 0)
            {
                PrepareGossipMenu(pSource, uint32(pMenuData.m_gAction_menu));
                SendPreparedGossip(pSource);
            }
            else if (pMenuData.m_gAction_menu < 0)
            {
                PlayerTalkClass->CloseGossip();
                TalkedToCreature(pSource->GetEntry(), pSource->GetObjectGuid());
            }

            break;
        }
        case GOSSIP_OPTION_SPIRITHEALER:
            if (isDead())
                ((Creature*)pSource)->CastSpell(((Creature*)pSource), 17251, true, NULL, NULL, GetObjectGuid());
            break;
        case GOSSIP_OPTION_QUESTGIVER:
            PrepareQuestMenu(guid);
            SendPreparedQuest(guid);
            break;
        case GOSSIP_OPTION_VENDOR:
        case GOSSIP_OPTION_ARMORER:
            GetSession()->SendListInventory(guid);
            break;
        case GOSSIP_OPTION_STABLEPET:
            GetSession()->SendStablePet(guid);
            break;
        case GOSSIP_OPTION_TRAINER:
            GetSession()->SendTrainerList(guid);
            break;
        case GOSSIP_OPTION_UNLEARNTALENTS:
            PlayerTalkClass->CloseGossip();
            SendTalentWipeConfirm(guid);
            break;
        case GOSSIP_OPTION_UNLEARNPETSKILLS:
            PlayerTalkClass->CloseGossip();
            SendPetSkillWipeConfirm();
            break;
        case GOSSIP_OPTION_TAXIVENDOR:
            GetSession()->SendTaxiMenu(((Creature*)pSource));
            break;
        case GOSSIP_OPTION_INNKEEPER:
            PlayerTalkClass->CloseGossip();
            SetBindPoint(guid);
            break;
        case GOSSIP_OPTION_BANKER:
            GetSession()->SendShowBank(guid);
            break;
        case GOSSIP_OPTION_PETITIONER:
            PlayerTalkClass->CloseGossip();
            GetSession()->SendPetitionShowList(guid);
            break;
        case GOSSIP_OPTION_TABARDDESIGNER:
            PlayerTalkClass->CloseGossip();
            GetSession()->SendTabardVendorActivate(guid);
            break;
        case GOSSIP_OPTION_AUCTIONEER:
            GetSession()->SendAuctionHello(((Creature*)pSource));
            break;
        case GOSSIP_OPTION_SPIRITGUIDE:
            PrepareGossipMenu(pSource);
            SendPreparedGossip(pSource);
            break;
        case GOSSIP_OPTION_BATTLEFIELD:
        {
            BattleGroundTypeId bgTypeId = sBattleGroundMgr.GetBattleMasterBG(pSource->GetEntry());

            if (bgTypeId == BATTLEGROUND_TYPE_NONE)
            {
                sLog.outError("a user (guid %u) requested battlegroundlist from a npc who is no battlemaster", GetGUIDLow());
                return;
            }

            GetSession()->SendBattlegGroundList(guid, bgTypeId);
            break;
        }
    }

    if (pMenuData.m_gAction_script)
    {
        if (pSource->GetTypeId() == TYPEID_UNIT)
            GetMap()->ScriptsStart(sGossipScripts, pMenuData.m_gAction_script, pSource, this, Map::SCRIPT_EXEC_PARAM_UNIQUE_BY_SOURCE);
        else if (pSource->GetTypeId() == TYPEID_GAMEOBJECT)
            GetMap()->ScriptsStart(sGossipScripts, pMenuData.m_gAction_script, this, pSource, Map::SCRIPT_EXEC_PARAM_UNIQUE_BY_TARGET);
    }
}

uint32 Player::GetGossipTextId(WorldObject* pSource)
{
    if (!pSource || pSource->GetTypeId() != TYPEID_UNIT)
        return DEFAULT_GOSSIP_MESSAGE;

    if (uint32 pos = sObjectMgr.GetNpcGossip(((Creature*)pSource)->GetGUIDLow()))
        return pos;

    return DEFAULT_GOSSIP_MESSAGE;
}

uint32 Player::GetGossipTextId(uint32 menuId, WorldObject* pSource)
{
    uint32 textId = DEFAULT_GOSSIP_MESSAGE;

    if (!menuId)
        return textId;

    uint32 scriptId = 0;
    uint32 lastConditionId = 0;

    GossipMenusMapBounds pMenuBounds = sObjectMgr.GetGossipMenusMapBounds(menuId);
    for (GossipMenusMap::const_iterator itr = pMenuBounds.first; itr != pMenuBounds.second; ++itr)
    {
        // Take the text that has the highest conditionId of all fitting
        // No condition and no text with condition found OR higher and fitting condition found
        if ((!itr->second.conditionId && !lastConditionId) ||
                (itr->second.conditionId > lastConditionId && sObjectMgr.IsPlayerMeetToCondition(itr->second.conditionId, this, GetMap(), pSource, CONDITION_FROM_GOSSIP_MENU)))
        {
            lastConditionId = itr->second.conditionId;
            textId = itr->second.text_id;
            scriptId = itr->second.script_id;
        }
    }

    // Start related script
    if (scriptId)
        GetMap()->ScriptsStart(sGossipScripts, scriptId, this, pSource, Map::SCRIPT_EXEC_PARAM_UNIQUE_BY_TARGET);

    return textId;
}

uint32 Player::GetDefaultGossipMenuForSource(WorldObject* pSource)
{
    if (pSource->GetTypeId() == TYPEID_UNIT)
        return ((Creature*)pSource)->GetCreatureInfo()->GossipMenuId;
    else if (pSource->GetTypeId() == TYPEID_GAMEOBJECT)
        return ((GameObject*)pSource)->GetGOInfo()->GetGossipMenuId();

    return 0;
}

/*********************************************************/
/***                    QUEST SYSTEM                   ***/
/*********************************************************/

void Player::PrepareQuestMenu(ObjectGuid guid)
{
    QuestRelationsMapBounds rbounds;
    QuestRelationsMapBounds irbounds;

    // pets also can have quests
    if (Creature* pCreature = GetMap()->GetAnyTypeCreature(guid))
    {
        rbounds = sObjectMgr.GetCreatureQuestRelationsMapBounds(pCreature->GetEntry());
        irbounds = sObjectMgr.GetCreatureQuestInvolvedRelationsMapBounds(pCreature->GetEntry());
    }
    else
    {
        // we should obtain map pointer from GetMap() in 99% of cases. Special case
        // only for quests which cast teleport spells on player
        Map* _map = IsInWorld() ? GetMap() : sMapMgr.FindMap(GetMapId(), GetInstanceId());
        MANGOS_ASSERT(_map);

        if (GameObject* pGameObject = _map->GetGameObject(guid))
        {
            rbounds = sObjectMgr.GetGOQuestRelationsMapBounds(pGameObject->GetEntry());
            irbounds = sObjectMgr.GetGOQuestInvolvedRelationsMapBounds(pGameObject->GetEntry());
        }
        else
            return;
    }

    QuestMenu& qm = PlayerTalkClass->GetQuestMenu();
    qm.ClearMenu();

    for (QuestRelationsMap::const_iterator itr = irbounds.first; itr != irbounds.second; ++itr)
    {
        uint32 quest_id = itr->second;

        Quest const* pQuest = sObjectMgr.GetQuestTemplate(quest_id);

        if (!pQuest || !pQuest->IsActive())
            continue;

        QuestStatus status = GetQuestStatus(quest_id);

        if (status == QUEST_STATUS_COMPLETE && !GetQuestRewardStatus(quest_id))
            qm.AddMenuItem(quest_id, DIALOG_STATUS_REWARD_REP);
        else if (status == QUEST_STATUS_INCOMPLETE)
            qm.AddMenuItem(quest_id, DIALOG_STATUS_INCOMPLETE);
        else if (status == QUEST_STATUS_AVAILABLE)
            qm.AddMenuItem(quest_id, DIALOG_STATUS_CHAT);
    }

    for (QuestRelationsMap::const_iterator itr = rbounds.first; itr != rbounds.second; ++itr)
    {
        uint32 quest_id = itr->second;

        Quest const* pQuest = sObjectMgr.GetQuestTemplate(quest_id);

        if (!pQuest || !pQuest->IsActive())
            continue;

        QuestStatus status = GetQuestStatus(quest_id);

        if (pQuest->IsAutoComplete() && CanTakeQuest(pQuest, false))
            qm.AddMenuItem(quest_id, DIALOG_STATUS_REWARD_REP);
        else if (status == QUEST_STATUS_NONE && CanTakeQuest(pQuest, false))
            qm.AddMenuItem(quest_id, DIALOG_STATUS_AVAILABLE);
    }
}

void Player::SendPreparedQuest(ObjectGuid guid)
{
    QuestMenu& questMenu = PlayerTalkClass->GetQuestMenu();
    if (questMenu.Empty())
        return;

    QuestMenuItem const& qmi0 = questMenu.GetItem(0);

    uint32 status = qmi0.m_qIcon;

    // single element case
    if (questMenu.MenuItemCount() == 1)
    {
        // Auto open -- maybe also should verify there is no greeting
        uint32 quest_id = qmi0.m_qId;
        Quest const* pQuest = sObjectMgr.GetQuestTemplate(quest_id);

        if (pQuest)
        {
            if (status == DIALOG_STATUS_REWARD_REP && !GetQuestRewardStatus(quest_id))
                PlayerTalkClass->SendQuestGiverRequestItems(pQuest, guid, CanRewardQuest(pQuest, false), true);
            else if (status == DIALOG_STATUS_INCOMPLETE)
                PlayerTalkClass->SendQuestGiverRequestItems(pQuest, guid, false, true);
            // Send completable on repeatable quest if player don't have quest
            else if (pQuest->IsRepeatable())
                PlayerTalkClass->SendQuestGiverRequestItems(pQuest, guid, CanCompleteRepeatableQuest(pQuest), true);
            else
                PlayerTalkClass->SendQuestGiverQuestDetails(pQuest, guid, true);
        }
    }
    // multiply entries
    else
    {
        QEmote qe;
        qe._Delay = 0;
        qe._Emote = 0;
        std::string title = "";

        // need pet case for some quests
        if (Creature* pCreature = GetMap()->GetAnyTypeCreature(guid))
        {
            uint32 textid = GetGossipTextId(pCreature);

            GossipText const* gossiptext = sObjectMgr.GetGossipText(textid);
            if (!gossiptext)
            {
                qe._Delay = 0;                              // TEXTEMOTE_MESSAGE;              // zyg: player emote
                qe._Emote = 0;                              // TEXTEMOTE_HELLO;                // zyg: NPC emote
                title = "";
            }
            else
            {
                qe = gossiptext->Options[0].Emotes[0];

                int loc_idx = GetSession()->GetSessionDbLocaleIndex();

                std::string title0 = gossiptext->Options[0].Text_0;
                std::string title1 = gossiptext->Options[0].Text_1;
                sObjectMgr.GetNpcTextLocaleStrings0(textid, loc_idx, &title0, &title1);

                title = !title0.empty() ? title0 : title1;
            }
        }
        PlayerTalkClass->SendQuestGiverQuestList(qe, title, guid);
    }
}

bool Player::IsActiveQuest(uint32 quest_id) const
{
    QuestStatusMap::const_iterator itr = mQuestStatus.find(quest_id);

    return itr != mQuestStatus.end() && itr->second.m_status != QUEST_STATUS_NONE;
}

bool Player::IsCurrentQuest(uint32 quest_id, uint8 completed_or_not) const
{
    QuestStatusMap::const_iterator itr = mQuestStatus.find(quest_id);
    if (itr == mQuestStatus.end())
        return false;

    switch (completed_or_not)
    {
        case 1:
            return itr->second.m_status == QUEST_STATUS_INCOMPLETE;
        case 2:
            return itr->second.m_status == QUEST_STATUS_COMPLETE && !itr->second.m_rewarded;
        default:
            return itr->second.m_status == QUEST_STATUS_INCOMPLETE || (itr->second.m_status == QUEST_STATUS_COMPLETE && !itr->second.m_rewarded);
    }
}

Quest const* Player::GetNextQuest(ObjectGuid guid, Quest const* pQuest)
{
    QuestRelationsMapBounds rbounds;

    if (Creature* pCreature = GetMap()->GetAnyTypeCreature(guid))
    {
        rbounds = sObjectMgr.GetCreatureQuestRelationsMapBounds(pCreature->GetEntry());
    }
    else
    {
        // we should obtain map pointer from GetMap() in 99% of cases. Special case
        // only for quests which cast teleport spells on player
        Map* _map = IsInWorld() ? GetMap() : sMapMgr.FindMap(GetMapId(), GetInstanceId());
        MANGOS_ASSERT(_map);

        if (GameObject* pGameObject = _map->GetGameObject(guid))
        {
            rbounds = sObjectMgr.GetGOQuestRelationsMapBounds(pGameObject->GetEntry());
        }
        else
            return NULL;
    }

    uint32 nextQuestID = pQuest->GetNextQuestInChain();
    for (QuestRelationsMap::const_iterator itr = rbounds.first; itr != rbounds.second; ++itr)
    {
        if (itr->second == nextQuestID)
            return sObjectMgr.GetQuestTemplate(nextQuestID);
    }

    return NULL;
}

/**
 * Check if a player could see a start quest
 * Basic Quest-taking requirements: Class, Race, Skill, Quest-Line, ...
 * Check if the quest-level is not too high (related config value CONFIG_INT32_QUEST_HIGH_LEVEL_HIDE_DIFF)
 */
bool Player::CanSeeStartQuest(Quest const* pQuest) const
{
    if (SatisfyQuestClass(pQuest, false) && SatisfyQuestRace(pQuest, false) && SatisfyQuestSkill(pQuest, false) &&
            SatisfyQuestExclusiveGroup(pQuest, false) && SatisfyQuestReputation(pQuest, false) &&
            SatisfyQuestPreviousQuest(pQuest, false) && SatisfyQuestNextChain(pQuest, false) &&
            SatisfyQuestPrevChain(pQuest, false) &&
            pQuest->IsActive())
    {
        int32 highLevelDiff = sWorld.getConfig(CONFIG_INT32_QUEST_HIGH_LEVEL_HIDE_DIFF);
        if (highLevelDiff < 0)
            return true;
        return getLevel() + uint32(highLevelDiff) >= pQuest->GetMinLevel();
    }

    return false;
}

bool Player::CanTakeQuest(Quest const* pQuest, bool msg) const
{
    return SatisfyQuestStatus(pQuest, msg) && SatisfyQuestExclusiveGroup(pQuest, msg) &&
           SatisfyQuestClass(pQuest, msg) && SatisfyQuestRace(pQuest, msg) && SatisfyQuestLevel(pQuest, msg) &&
           SatisfyQuestSkill(pQuest, msg) && SatisfyQuestReputation(pQuest, msg) &&
           SatisfyQuestPreviousQuest(pQuest, msg) && SatisfyQuestTimed(pQuest, msg) &&
           SatisfyQuestNextChain(pQuest, msg) && SatisfyQuestPrevChain(pQuest, msg) &&
           pQuest->IsActive();
}

bool Player::CanAddQuest(Quest const* pQuest, bool msg) const
{
    if (!SatisfyQuestLog(msg))
        return false;

    if (!CanGiveQuestSourceItemIfNeed(pQuest))
        return false;

    return true;
}

bool Player::CanCompleteQuest(uint32 quest_id) const
{
    if (!quest_id)
        return false;

    QuestStatusMap::const_iterator q_itr = mQuestStatus.find(quest_id);

    // some quests can be auto taken and auto completed in one step
    QuestStatus status = q_itr != mQuestStatus.end() ? q_itr->second.m_status : QUEST_STATUS_NONE;

    if (status == QUEST_STATUS_COMPLETE)
        return false;                                       // not allow re-complete quest

    Quest const* qInfo = sObjectMgr.GetQuestTemplate(quest_id);

    if (!qInfo)
        return false;

    // only used for "flag" quests and not real in-game quests
    if (qInfo->HasQuestFlag(QUEST_FLAGS_AUTO_REWARDED))
    {
        // a few checks, not all "satisfy" is needed
        if (SatisfyQuestPreviousQuest(qInfo, false) && SatisfyQuestLevel(qInfo, false) &&
                SatisfyQuestSkill(qInfo, false) && SatisfyQuestRace(qInfo, false) && SatisfyQuestClass(qInfo, false))
            return true;

        return false;
    }

    // auto complete quest
    if (qInfo->IsAutoComplete() && CanTakeQuest(qInfo, false))
        return true;

    if (status != QUEST_STATUS_INCOMPLETE)
        return false;

    // incomplete quest have status data
    QuestStatusData const& q_status = q_itr->second;

    if (qInfo->HasSpecialFlag(QUEST_SPECIAL_FLAG_DELIVER))
    {
        for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        {
            if (qInfo->ReqItemCount[i] != 0 && q_status.m_itemcount[i] < qInfo->ReqItemCount[i])
                return false;
        }
    }

    if (qInfo->HasSpecialFlag(QuestSpecialFlags(QUEST_SPECIAL_FLAG_KILL_OR_CAST | QUEST_SPECIAL_FLAG_SPEAKTO)))
    {
        for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        {
            if (qInfo->ReqCreatureOrGOId[i] == 0)
                continue;

            if (qInfo->ReqCreatureOrGOCount[i] != 0 && q_status.m_creatureOrGOcount[i] < qInfo->ReqCreatureOrGOCount[i])
                return false;
        }
    }

    if (qInfo->HasSpecialFlag(QUEST_SPECIAL_FLAG_EXPLORATION_OR_EVENT) && !q_status.m_explored)
        return false;

    if (qInfo->HasSpecialFlag(QUEST_SPECIAL_FLAG_TIMED) && q_status.m_timer == 0)
        return false;

    if (qInfo->GetRewOrReqMoney() < 0)
    {
        if (GetMoney() < uint32(-qInfo->GetRewOrReqMoney()))
            return false;
    }

    uint32 repFacId = qInfo->GetRepObjectiveFaction();
    if (repFacId && GetReputationMgr().GetReputation(repFacId) < qInfo->GetRepObjectiveValue())
        return false;

    return true;
}

bool Player::CanCompleteRepeatableQuest(Quest const* pQuest) const
{
    // Solve problem that player don't have the quest and try complete it.
    // if repeatable she must be able to complete event if player don't have it.
    // Seem that all repeatable quest are DELIVER Flag so, no need to add more.
    if (!CanTakeQuest(pQuest, false))
        return false;

    if (pQuest->HasSpecialFlag(QUEST_SPECIAL_FLAG_DELIVER))
        for (int i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
            if (pQuest->ReqItemId[i] && pQuest->ReqItemCount[i] && !HasItemCount(pQuest->ReqItemId[i], pQuest->ReqItemCount[i]))
                return false;

    if (!CanRewardQuest(pQuest, false))
        return false;

    return true;
}

bool Player::CanRewardQuest(Quest const* pQuest, bool msg) const
{
    // not auto complete quest and not completed quest (only cheating case, then ignore without message)
    if (!pQuest->IsAutoComplete() && GetQuestStatus(pQuest->GetQuestId()) != QUEST_STATUS_COMPLETE)
        return false;

    // rewarded and not repeatable quest (only cheating case, then ignore without message)
    if (GetQuestRewardStatus(pQuest->GetQuestId()))
        return false;

    // prevent receive reward with quest items in bank
    if (pQuest->HasSpecialFlag(QUEST_SPECIAL_FLAG_DELIVER))
    {
        for (int i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
        {
            if (pQuest->ReqItemCount[i] != 0 &&
                    GetItemCount(pQuest->ReqItemId[i]) < pQuest->ReqItemCount[i])
            {
                if (msg)
                    SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, NULL, NULL, pQuest->ReqItemId[i]);

                return false;
            }
        }
    }

    // prevent receive reward with low money and GetRewOrReqMoney() < 0
    if (pQuest->GetRewOrReqMoney() < 0 && GetMoney() < uint32(-pQuest->GetRewOrReqMoney()))
        return false;

    return true;
}

bool Player::CanRewardQuest(Quest const* pQuest, uint32 reward, bool msg) const
{
    // prevent receive reward with quest items in bank or for not completed quest
    if (!CanRewardQuest(pQuest, msg))
        return false;

    if (pQuest->GetRewChoiceItemsCount() > 0)
    {
        if (pQuest->RewChoiceItemId[reward])
        {
            ItemPosCountVec dest;
            InventoryResult res = CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, pQuest->RewChoiceItemId[reward], pQuest->RewChoiceItemCount[reward]);
            if (res != EQUIP_ERR_OK)
            {
                SendEquipError(res, NULL, NULL, pQuest->RewChoiceItemId[reward]);
                return false;
            }
        }
    }

    if (pQuest->GetRewItemsCount() > 0)
    {
        for (uint32 i = 0; i < pQuest->GetRewItemsCount(); ++i)
        {
            if (pQuest->RewItemId[i])
            {
                ItemPosCountVec dest;
                InventoryResult res = CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, pQuest->RewItemId[i], pQuest->RewItemCount[i]);
                if (res != EQUIP_ERR_OK)
                {
                    SendEquipError(res, NULL, NULL);
                    return false;
                }
            }
        }
    }

    return true;
}

void Player::SendPetTameFailure(PetTameFailureReason reason)
{
    WorldPacket data(SMSG_PET_TAME_FAILURE, 1);
    data << uint8(reason);
    GetSession()->SendPacket(&data);
}

void Player::AddQuest(Quest const* pQuest, Object* questGiver)
{
    uint16 log_slot = FindQuestSlot(0);
    MANGOS_ASSERT(log_slot < MAX_QUEST_LOG_SIZE);

    uint32 quest_id = pQuest->GetQuestId();

    // if not exist then created with set uState==NEW and rewarded=false
    QuestStatusData& questStatusData = mQuestStatus[quest_id];

    // check for repeatable quests status reset
    questStatusData.m_status = QUEST_STATUS_INCOMPLETE;
    questStatusData.m_explored = false;

    if (pQuest->HasSpecialFlag(QUEST_SPECIAL_FLAG_DELIVER))
    {
        for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
            questStatusData.m_itemcount[i] = 0;
    }

    if (pQuest->HasSpecialFlag(QuestSpecialFlags(QUEST_SPECIAL_FLAG_KILL_OR_CAST | QUEST_SPECIAL_FLAG_SPEAKTO)))
    {
        for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
            questStatusData.m_creatureOrGOcount[i] = 0;
    }

    if (pQuest->GetRepObjectiveFaction())
        if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(pQuest->GetRepObjectiveFaction()))
            GetReputationMgr().SetVisible(factionEntry);

    uint32 qtime = 0;
    if (pQuest->HasSpecialFlag(QUEST_SPECIAL_FLAG_TIMED))
    {
        uint32 limittime = pQuest->GetLimitTime();

        // shared timed quest
        if (questGiver && questGiver->GetTypeId() == TYPEID_PLAYER)
            limittime = ((Player*)questGiver)->getQuestStatusMap()[quest_id].m_timer / IN_MILLISECONDS;

        AddTimedQuest(quest_id);
        questStatusData.m_timer = limittime * IN_MILLISECONDS;
        qtime = static_cast<uint32>(time(NULL)) + limittime;
    }
    else
        questStatusData.m_timer = 0;

    SetQuestSlot(log_slot, quest_id, qtime);

    if (questStatusData.uState != QUEST_NEW)
        questStatusData.uState = QUEST_CHANGED;

    // quest accept scripts
    if (questGiver)
    {
        switch (questGiver->GetTypeId())
        {
            case TYPEID_UNIT:
                sScriptMgr.OnQuestAccept(this, (Creature*)questGiver, pQuest);
                break;
            case TYPEID_ITEM:
            case TYPEID_CONTAINER:
                sScriptMgr.OnQuestAccept(this, (Item*)questGiver, pQuest);
                break;
            case TYPEID_GAMEOBJECT:
                sScriptMgr.OnQuestAccept(this, (GameObject*)questGiver, pQuest);
                break;
        }

        // starting initial DB quest script
        if (pQuest->GetQuestStartScript() != 0)
            GetMap()->ScriptsStart(sQuestStartScripts, pQuest->GetQuestStartScript(), questGiver, this, Map::SCRIPT_EXEC_PARAM_UNIQUE_BY_SOURCE);
    }

    // remove start item if not need
    if (questGiver && questGiver->isType(TYPEMASK_ITEM))
    {
        // destroy not required for quest finish quest starting item
        bool notRequiredItem = true;
        for (int i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
        {
            if (pQuest->ReqItemId[i] == questGiver->GetEntry())
            {
                notRequiredItem = false;
                break;
            }
        }

        if (pQuest->GetSrcItemId() == questGiver->GetEntry())
            notRequiredItem = false;

        if (notRequiredItem)
            DestroyItem(((Item*)questGiver)->GetBagSlot(), ((Item*)questGiver)->GetSlot(), true);
    }

    GiveQuestSourceItemIfNeed(pQuest);

    AdjustQuestReqItemCount(pQuest, questStatusData);

    // Some spells applied at quest activation
    uint32 zone, area;
    GetZoneAndAreaId(zone, area);
    SpellAreaForAreaMapBounds saBounds = sSpellMgr.GetSpellAreaForAreaMapBounds(zone);
    for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        itr->second->ApplyOrRemoveSpellIfCan(this, zone, area, true);
    if (area != zone)
    {
        saBounds = sSpellMgr.GetSpellAreaForAreaMapBounds(area);
        for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
            itr->second->ApplyOrRemoveSpellIfCan(this, zone, area, true);
    }
    saBounds = sSpellMgr.GetSpellAreaForAreaMapBounds(0);
    for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        itr->second->ApplyOrRemoveSpellIfCan(this, zone, area, true);

    UpdateForQuestWorldObjects();
}

void Player::CompleteQuest(uint32 quest_id)
{
    if (quest_id)
    {
        SetQuestStatus(quest_id, QUEST_STATUS_COMPLETE);

        uint16 log_slot = FindQuestSlot(quest_id);
        if (log_slot < MAX_QUEST_LOG_SIZE)
            SetQuestSlotState(log_slot, QUEST_STATE_COMPLETE);

        if (Quest const* qInfo = sObjectMgr.GetQuestTemplate(quest_id))
        {
            if (qInfo->HasQuestFlag(QUEST_FLAGS_AUTO_REWARDED))
                RewardQuest(qInfo, 0, this, false);
        }
    }
}

void Player::IncompleteQuest(uint32 quest_id)
{
    if (quest_id)
    {
        SetQuestStatus(quest_id, QUEST_STATUS_INCOMPLETE);

        uint16 log_slot = FindQuestSlot(quest_id);
        if (log_slot < MAX_QUEST_LOG_SIZE)
            RemoveQuestSlotState(log_slot, QUEST_STATE_COMPLETE);
    }
}

void Player::RewardQuest(Quest const* pQuest, uint32 reward, Object* questGiver, bool announce)
{
    uint32 quest_id = pQuest->GetQuestId();

    for (int i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
    {
        if (pQuest->ReqItemId[i])
            DestroyItemCount(pQuest->ReqItemId[i], pQuest->ReqItemCount[i], true);
    }

    RemoveTimedQuest(quest_id);

    if (BattleGround* bg = GetBattleGround())
        if (bg->GetTypeID() == BATTLEGROUND_AV)
            ((BattleGroundAV*)bg)->HandleQuestComplete(pQuest->GetQuestId(), this);

    if (pQuest->GetRewChoiceItemsCount() > 0)
    {
        if (uint32 itemId = pQuest->RewChoiceItemId[reward])
        {
            ItemPosCountVec dest;
            if (CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, pQuest->RewChoiceItemCount[reward]) == EQUIP_ERR_OK)
            {
                Item* item = StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));
                SendNewItem(item, pQuest->RewChoiceItemCount[reward], true, false);
            }
        }
    }

    if (pQuest->GetRewItemsCount() > 0)
    {
        for (uint32 i = 0; i < pQuest->GetRewItemsCount(); ++i)
        {
            if (uint32 itemId = pQuest->RewItemId[i])
            {
                ItemPosCountVec dest;
                if (CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, pQuest->RewItemCount[i]) == EQUIP_ERR_OK)
                {
                    Item* item = StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));
                    SendNewItem(item, pQuest->RewItemCount[i], true, false);
                }
            }
        }
    }

    RewardReputation(pQuest);

    uint16 log_slot = FindQuestSlot(quest_id);
    if (log_slot < MAX_QUEST_LOG_SIZE)
        SetQuestSlot(log_slot, 0);

    QuestStatusData& q_status = mQuestStatus[quest_id];

    // Used for client inform but rewarded only in case not max level
    uint32 xp = uint32(pQuest->XPValue(this) * sWorld.getConfig(CONFIG_FLOAT_RATE_XP_QUEST));

    if (getLevel() < sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
        GiveXP(xp , NULL);
    else
        ModifyMoney(int32(pQuest->GetRewMoneyMaxLevel() * sWorld.getConfig(CONFIG_FLOAT_RATE_DROP_MONEY)));

    // Give player extra money if GetRewOrReqMoney > 0 and get ReqMoney if negative
    ModifyMoney(pQuest->GetRewOrReqMoney());

    // Send reward mail
    if (uint32 mail_template_id = pQuest->GetRewMailTemplateId())
        MailDraft(mail_template_id).SendMailTo(this, questGiver, MAIL_CHECK_MASK_HAS_BODY, pQuest->GetRewMailDelaySecs());

    if (!pQuest->IsRepeatable())
        SetQuestStatus(quest_id, QUEST_STATUS_COMPLETE);
    else
        SetQuestStatus(quest_id, QUEST_STATUS_NONE);

    q_status.m_rewarded = true;
    if (q_status.uState != QUEST_NEW)
        q_status.uState = QUEST_CHANGED;

    if (announce)
        SendQuestReward(pQuest, xp, questGiver);

    bool handled = false;

    switch (questGiver->GetTypeId())
    {
        case TYPEID_UNIT:
            handled = sScriptMgr.OnQuestRewarded(this, (Creature*)questGiver, pQuest);
            break;
        case TYPEID_GAMEOBJECT:
            handled = sScriptMgr.OnQuestRewarded(this, (GameObject*)questGiver, pQuest);
            break;
    }

    if (!handled && pQuest->GetQuestCompleteScript() != 0)
        GetMap()->ScriptsStart(sQuestEndScripts, pQuest->GetQuestCompleteScript(), questGiver, this, Map::SCRIPT_EXEC_PARAM_UNIQUE_BY_SOURCE);

    // cast spells after mark quest complete (some spells have quest completed state reqyurements in spell_area data)
    if (pQuest->GetRewSpellCast() > 0)
        CastSpell(this, pQuest->GetRewSpellCast(), true);
    else if (pQuest->GetRewSpell() > 0)
        CastSpell(this, pQuest->GetRewSpell(), true);

    // remove auras from spells with quest reward state limitations
    // Some spells applied at quest reward
    uint32 zone, area;
    GetZoneAndAreaId(zone, area);
    SpellAreaForAreaMapBounds saBounds = sSpellMgr.GetSpellAreaForAreaMapBounds(zone);
    for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        itr->second->ApplyOrRemoveSpellIfCan(this, zone, area, false);
    if (area != zone)
    {
        saBounds = sSpellMgr.GetSpellAreaForAreaMapBounds(area);
        for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
            itr->second->ApplyOrRemoveSpellIfCan(this, zone, area, false);
    }
    saBounds = sSpellMgr.GetSpellAreaForAreaMapBounds(0);
    for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        itr->second->ApplyOrRemoveSpellIfCan(this, zone, area, false);
}

void Player::FailQuest(uint32 questId)
{
    if (Quest const* pQuest = sObjectMgr.GetQuestTemplate(questId))
    {
        SetQuestStatus(questId, QUEST_STATUS_FAILED);

        uint16 log_slot = FindQuestSlot(questId);

        if (log_slot < MAX_QUEST_LOG_SIZE)
        {
            SetQuestSlotTimer(log_slot, 1);
            SetQuestSlotState(log_slot, QUEST_STATE_FAIL);
        }

        if (pQuest->HasSpecialFlag(QUEST_SPECIAL_FLAG_TIMED))
        {
            QuestStatusData& q_status = mQuestStatus[questId];

            RemoveTimedQuest(questId);
            q_status.m_timer = 0;

            SendQuestTimerFailed(questId);
        }
        else
            SendQuestFailed(questId);
    }
}

bool Player::SatisfyQuestSkill(Quest const* qInfo, bool msg) const
{
    uint32 skill = qInfo->GetRequiredSkill();

    // skip 0 case RequiredSkill
    if (skill == 0)
        return true;

    // check skill value
    if (GetSkillValue(skill) < qInfo->GetRequiredSkillValue())
    {
        if (msg)
            SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);

        return false;
    }

    return true;
}

bool Player::SatisfyQuestLevel(Quest const* qInfo, bool msg) const
{
    if (getLevel() < qInfo->GetMinLevel())
    {
        if (msg)
            SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);

        return false;
    }

    return true;
}

bool Player::SatisfyQuestLog(bool msg) const
{
    // exist free slot
    if (FindQuestSlot(0) < MAX_QUEST_LOG_SIZE)
        return true;

    if (msg)
    {
        WorldPacket data(SMSG_QUESTLOG_FULL, 0);
        GetSession()->SendPacket(&data);
        DEBUG_LOG("WORLD: Sent SMSG_QUESTLOG_FULL");
    }
    return false;
}

bool Player::SatisfyQuestPreviousQuest(Quest const* qInfo, bool msg) const
{
    // No previous quest (might be first quest in a series)
    if (qInfo->prevQuests.empty())
        return true;

    for (Quest::PrevQuests::const_iterator iter = qInfo->prevQuests.begin(); iter != qInfo->prevQuests.end(); ++iter)
    {
        uint32 prevId = abs(*iter);

        QuestStatusMap::const_iterator i_prevstatus = mQuestStatus.find(prevId);
        Quest const* qPrevInfo = sObjectMgr.GetQuestTemplate(prevId);

        if (qPrevInfo && i_prevstatus != mQuestStatus.end())
        {
            // If any of the positive previous quests completed, return true
            if (*iter > 0 && i_prevstatus->second.m_rewarded)
            {
                // skip one-from-all exclusive group
                if (qPrevInfo->GetExclusiveGroup() >= 0)
                    return true;

                // each-from-all exclusive group ( < 0)
                // can be start if only all quests in prev quest exclusive group completed and rewarded
                ExclusiveQuestGroupsMapBounds bounds = sObjectMgr.GetExclusiveQuestGroupsMapBounds(qPrevInfo->GetExclusiveGroup());

                MANGOS_ASSERT(bounds.first != bounds.second); // always must be found if qPrevInfo->ExclusiveGroup != 0

                for (ExclusiveQuestGroupsMap::const_iterator iter2 = bounds.first; iter2 != bounds.second; ++iter2)
                {
                    uint32 exclude_Id = iter2->second;

                    // skip checked quest id, only state of other quests in group is interesting
                    if (exclude_Id == prevId)
                        continue;

                    QuestStatusMap::const_iterator i_exstatus = mQuestStatus.find(exclude_Id);

                    // alternative quest from group also must be completed and rewarded(reported)
                    if (i_exstatus == mQuestStatus.end() || !i_exstatus->second.m_rewarded)
                    {
                        if (msg)
                            SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);

                        return false;
                    }
                }
                return true;
            }
            // If any of the negative previous quests active, return true
            if (*iter < 0 && IsCurrentQuest(prevId))
            {
                // skip one-from-all exclusive group
                if (qPrevInfo->GetExclusiveGroup() >= 0)
                    return true;

                // each-from-all exclusive group ( < 0)
                // can be start if only all quests in prev quest exclusive group active
                ExclusiveQuestGroupsMapBounds bounds = sObjectMgr.GetExclusiveQuestGroupsMapBounds(qPrevInfo->GetExclusiveGroup());

                MANGOS_ASSERT(bounds.first != bounds.second); // always must be found if qPrevInfo->ExclusiveGroup != 0

                for (ExclusiveQuestGroupsMap::const_iterator iter2 = bounds.first; iter2 != bounds.second; ++iter2)
                {
                    uint32 exclude_Id = iter2->second;

                    // skip checked quest id, only state of other quests in group is interesting
                    if (exclude_Id == prevId)
                        continue;

                    // alternative quest from group also must be active
                    if (!IsCurrentQuest(exclude_Id))
                    {
                        if (msg)
                            SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);

                        return false;
                    }
                }
                return true;
            }
        }
    }

    // Has only positive prev. quests in non-rewarded state
    // and negative prev. quests in non-active state
    if (msg)
        SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);

    return false;
}

bool Player::SatisfyQuestClass(Quest const* qInfo, bool msg) const
{
    uint32 reqClass = qInfo->GetRequiredClasses();

    if (reqClass == 0)
        return true;

    if ((reqClass & getClassMask()) == 0)
    {
        if (msg)
            SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);

        return false;
    }

    return true;
}

bool Player::SatisfyQuestRace(Quest const* qInfo, bool msg) const
{
    uint32 reqraces = qInfo->GetRequiredRaces();

    if (reqraces == 0)
        return true;

    if ((reqraces & getRaceMask()) == 0)
    {
        if (msg)
            SendCanTakeQuestResponse(INVALIDREASON_QUEST_FAILED_WRONG_RACE);

        return false;
    }

    return true;
}

bool Player::SatisfyQuestReputation(Quest const* qInfo, bool msg) const
{
    uint32 fIdMin = qInfo->GetRequiredMinRepFaction();      // Min required rep
    if (fIdMin && GetReputationMgr().GetReputation(fIdMin) < qInfo->GetRequiredMinRepValue())
    {
        if (msg)
            SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);

        return false;
    }

    uint32 fIdMax = qInfo->GetRequiredMaxRepFaction();      // Max required rep
    if (fIdMax && GetReputationMgr().GetReputation(fIdMax) >= qInfo->GetRequiredMaxRepValue())
    {
        if (msg)
            SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);

        return false;
    }

    return true;
}

bool Player::SatisfyQuestStatus(Quest const* qInfo, bool msg) const
{
    QuestStatusMap::const_iterator itr = mQuestStatus.find(qInfo->GetQuestId());

    if (itr != mQuestStatus.end() && itr->second.m_status != QUEST_STATUS_NONE)
    {
        if (msg)
            SendCanTakeQuestResponse(INVALIDREASON_QUEST_ALREADY_ON);

        return false;
    }

    return true;
}

bool Player::SatisfyQuestTimed(Quest const* qInfo, bool msg) const
{
    if (!m_timedquests.empty() && qInfo->HasSpecialFlag(QUEST_SPECIAL_FLAG_TIMED))
    {
        if (msg)
            SendCanTakeQuestResponse(INVALIDREASON_QUEST_ONLY_ONE_TIMED);

        return false;
    }

    return true;
}

bool Player::SatisfyQuestExclusiveGroup(Quest const* qInfo, bool msg) const
{
    // non positive exclusive group, if > 0 then can be start if any other quest in exclusive group already started/completed
    if (qInfo->GetExclusiveGroup() <= 0)
        return true;

    ExclusiveQuestGroupsMapBounds bounds = sObjectMgr.GetExclusiveQuestGroupsMapBounds(qInfo->GetExclusiveGroup());

    MANGOS_ASSERT(bounds.first != bounds.second);           // must always be found if qInfo->ExclusiveGroup != 0

    for (ExclusiveQuestGroupsMap::const_iterator iter = bounds.first; iter != bounds.second; ++iter)
    {
        uint32 exclude_Id = iter->second;

        // skip checked quest id, only state of other quests in group is interesting
        if (exclude_Id == qInfo->GetQuestId())
            continue;

        QuestStatusMap::const_iterator i_exstatus = mQuestStatus.find(exclude_Id);

        // alternative quest already started or completed
        if (i_exstatus != mQuestStatus.end() &&
                (i_exstatus->second.m_status == QUEST_STATUS_COMPLETE || i_exstatus->second.m_status == QUEST_STATUS_INCOMPLETE))
        {
            if (msg)
                SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);

            return false;
        }
    }

    return true;
}

bool Player::SatisfyQuestNextChain(Quest const* qInfo, bool msg) const
{
    if (!qInfo->GetNextQuestInChain())
        return true;

    // next quest in chain already started or completed
    QuestStatusMap::const_iterator itr = mQuestStatus.find(qInfo->GetNextQuestInChain());
    if (itr != mQuestStatus.end() &&
            (itr->second.m_status == QUEST_STATUS_COMPLETE || itr->second.m_status == QUEST_STATUS_INCOMPLETE))
    {
        if (msg)
            SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);

        return false;
    }

    // check for all quests further up the chain
    // only necessary if there are quest chains with more than one quest that can be skipped
    // return SatisfyQuestNextChain( qInfo->GetNextQuestInChain(), msg );
    return true;
}

bool Player::SatisfyQuestPrevChain(Quest const* qInfo, bool msg) const
{
    // No previous quest in chain
    if (qInfo->prevChainQuests.empty())
        return true;

    for (Quest::PrevChainQuests::const_iterator iter = qInfo->prevChainQuests.begin(); iter != qInfo->prevChainQuests.end(); ++iter)
    {
        uint32 prevId = *iter;

        // If any of the previous quests in chain active, return false
        if (IsCurrentQuest(prevId))
        {
            if (msg)
                SendCanTakeQuestResponse(INVALIDREASON_DONT_HAVE_REQ);

            return false;
        }

        // check for all quests further down the chain
        // only necessary if there are quest chains with more than one quest that can be skipped
        // if( !SatisfyQuestPrevChain( prevId, msg ) )
        //    return false;
    }

    // No previous quest in chain active
    return true;
}

bool Player::CanGiveQuestSourceItemIfNeed(Quest const* pQuest, ItemPosCountVec* dest) const
{
    if (uint32 srcitem = pQuest->GetSrcItemId())
    {
        uint32 count = pQuest->GetSrcItemCount();

        // player already have max amount required item (including bank), just report success
        uint32 has_count = GetItemCount(srcitem, true);
        if (has_count >= count)
            return true;

        count -= has_count;                                 // real need amount

        InventoryResult msg;
        if (!dest)
        {
            ItemPosCountVec destTemp;
            msg = CanStoreNewItem(NULL_BAG, NULL_SLOT, destTemp, srcitem, count);
        }
        else
            msg = CanStoreNewItem(NULL_BAG, NULL_SLOT, *dest, srcitem, count);

        if (msg == EQUIP_ERR_OK)
            return true;
        else
            SendEquipError(msg, NULL, NULL, srcitem);
        return false;
    }

    return true;
}

void Player::GiveQuestSourceItemIfNeed(Quest const* pQuest)
{
    ItemPosCountVec dest;
    if (CanGiveQuestSourceItemIfNeed(pQuest, &dest) && !dest.empty())
    {
        uint32 count = 0;
        for (ItemPosCountVec::const_iterator c_itr = dest.begin(); c_itr != dest.end(); ++c_itr)
            count += c_itr->count;

        Item* item = StoreNewItem(dest, pQuest->GetSrcItemId(), true);
        SendNewItem(item, count, true, false);
    }
}


bool Player::TakeQuestSourceItem(uint32 quest_id, bool msg)
{
    Quest const* qInfo = sObjectMgr.GetQuestTemplate(quest_id);
    if (qInfo)
    {
        uint32 srcitem = qInfo->GetSrcItemId();
        if (srcitem > 0)
        {
            uint32 count = qInfo->GetSrcItemCount();
            if (count <= 0)
                count = 1;

            // exist one case when destroy source quest item not possible:
            // non un-equippable item (equipped non-empty bag, for example)
            InventoryResult res = CanUnequipItems(srcitem, count);
            if (res != EQUIP_ERR_OK)
            {
                if (msg)
                    SendEquipError(res, NULL, NULL, srcitem);
                return false;
            }

            DestroyItemCount(srcitem, count, true, true);
        }
    }
    return true;
}

bool Player::GetQuestRewardStatus(uint32 quest_id) const
{
    Quest const* qInfo = sObjectMgr.GetQuestTemplate(quest_id);
    if (qInfo)
    {
        // for repeatable quests: rewarded field is set after first reward only to prevent getting XP more than once
        QuestStatusMap::const_iterator itr = mQuestStatus.find(quest_id);
        if (itr != mQuestStatus.end() && itr->second.m_status != QUEST_STATUS_NONE
                && !qInfo->IsRepeatable())
            return itr->second.m_rewarded;

        return false;
    }
    return false;
}

QuestStatus Player::GetQuestStatus(uint32 quest_id) const
{
    if (quest_id)
    {
        QuestStatusMap::const_iterator itr = mQuestStatus.find(quest_id);
        if (itr != mQuestStatus.end())
            return itr->second.m_status;
    }
    return QUEST_STATUS_NONE;
}

bool Player::CanShareQuest(uint32 quest_id) const
{
    if (Quest const* qInfo = sObjectMgr.GetQuestTemplate(quest_id))
        if (qInfo->HasQuestFlag(QUEST_FLAGS_SHARABLE))
            return IsCurrentQuest(quest_id);

    return false;
}

void Player::SetQuestStatus(uint32 quest_id, QuestStatus status)
{
    if (sObjectMgr.GetQuestTemplate(quest_id))
    {
        QuestStatusData& q_status = mQuestStatus[quest_id];

        q_status.m_status = status;

        if (q_status.uState != QUEST_NEW)
            q_status.uState = QUEST_CHANGED;
    }

    UpdateForQuestWorldObjects();
}

// not used in MaNGOS, but used in scripting code
uint32 Player::GetReqKillOrCastCurrentCount(uint32 quest_id, int32 entry)
{
    Quest const* qInfo = sObjectMgr.GetQuestTemplate(quest_id);
    if (!qInfo)
        return 0;

    for (int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j)
        if (qInfo->ReqCreatureOrGOId[j] == entry)
            return mQuestStatus[quest_id].m_creatureOrGOcount[j];

    return 0;
}

void Player::AdjustQuestReqItemCount(Quest const* pQuest, QuestStatusData& questStatusData)
{
    if (pQuest->HasSpecialFlag(QUEST_SPECIAL_FLAG_DELIVER))
    {
        for (int i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
        {
            uint32 reqitemcount = pQuest->ReqItemCount[i];
            if (reqitemcount != 0)
            {
                uint32 curitemcount = GetItemCount(pQuest->ReqItemId[i], true);

                questStatusData.m_itemcount[i] = std::min(curitemcount, reqitemcount);
                if (questStatusData.uState != QUEST_NEW) questStatusData.uState = QUEST_CHANGED;
            }
        }
    }
}

uint16 Player::FindQuestSlot(uint32 quest_id) const
{
    for (uint16 i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
        if (GetQuestSlotQuestId(i) == quest_id)
            return i;

    return MAX_QUEST_LOG_SIZE;
}

void Player::AreaExploredOrEventHappens(uint32 questId)
{
    if (questId)
    {
        uint16 log_slot = FindQuestSlot(questId);
        if (log_slot < MAX_QUEST_LOG_SIZE)
        {
            QuestStatusData& q_status = mQuestStatus[questId];

            if (!q_status.m_explored)
            {
                SetQuestSlotState(log_slot, QUEST_STATE_COMPLETE);
                SendQuestCompleteEvent(questId);
                q_status.m_explored = true;

                if (q_status.uState != QUEST_NEW)
                    q_status.uState = QUEST_CHANGED;
            }
        }
        if (CanCompleteQuest(questId))
            CompleteQuest(questId);
    }
}

// not used in mangosd, function for external script library
void Player::GroupEventHappens(uint32 questId, WorldObject const* pEventObject)
{
    if (Group* pGroup = GetGroup())
    {
        for (GroupReference* itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player* pGroupGuy = itr->getSource();

            // for any leave or dead (with not released body) group member at appropriate distance
            if (pGroupGuy && pGroupGuy->IsAtGroupRewardDistance(pEventObject) && !pGroupGuy->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
                pGroupGuy->AreaExploredOrEventHappens(questId);
        }
    }
    else
        AreaExploredOrEventHappens(questId);
}

void Player::ItemAddedQuestCheck(uint32 entry, uint32 count)
{
    for (int i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if (questid == 0)
            continue;

        QuestStatusData& q_status = mQuestStatus[questid];

        if (q_status.m_status != QUEST_STATUS_INCOMPLETE)
            continue;

        Quest const* qInfo = sObjectMgr.GetQuestTemplate(questid);
        if (!qInfo || !qInfo->HasSpecialFlag(QUEST_SPECIAL_FLAG_DELIVER))
            continue;

        for (int j = 0; j < QUEST_ITEM_OBJECTIVES_COUNT; ++j)
        {
            uint32 reqitem = qInfo->ReqItemId[j];
            if (reqitem == entry)
            {
                uint32 reqitemcount = qInfo->ReqItemCount[j];
                uint32 curitemcount = q_status.m_itemcount[j];
                if (curitemcount < reqitemcount)
                {
                    uint32 additemcount = (curitemcount + count <= reqitemcount ? count : reqitemcount - curitemcount);
                    q_status.m_itemcount[j] += additemcount;
                    if (q_status.uState != QUEST_NEW)
                        q_status.uState = QUEST_CHANGED;

                    SendQuestUpdateAddItem(qInfo, j, additemcount);
                }
                if (CanCompleteQuest(questid))
                    CompleteQuest(questid);
                return;
            }
        }
    }
    UpdateForQuestWorldObjects();
}

void Player::ItemRemovedQuestCheck(uint32 entry, uint32 count)
{
    for (int i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if (!questid)
            continue;
        Quest const* qInfo = sObjectMgr.GetQuestTemplate(questid);
        if (!qInfo)
            continue;
        if (!qInfo->HasSpecialFlag(QUEST_SPECIAL_FLAG_DELIVER))
            continue;

        for (int j = 0; j < QUEST_ITEM_OBJECTIVES_COUNT; ++j)
        {
            uint32 reqitem = qInfo->ReqItemId[j];
            if (reqitem == entry)
            {
                QuestStatusData& q_status = mQuestStatus[questid];

                uint32 reqitemcount = qInfo->ReqItemCount[j];
                uint32 curitemcount;
                if (q_status.m_status != QUEST_STATUS_COMPLETE)
                    curitemcount = q_status.m_itemcount[j];
                else
                    curitemcount = GetItemCount(entry, true);
                if (curitemcount < reqitemcount + count)
                {
                    uint32 remitemcount = (curitemcount <= reqitemcount ? count : count + reqitemcount - curitemcount);
                    q_status.m_itemcount[j] = curitemcount - remitemcount;
                    if (q_status.uState != QUEST_NEW) q_status.uState = QUEST_CHANGED;

                    IncompleteQuest(questid);
                }
                return;
            }
        }
    }
    UpdateForQuestWorldObjects();
}

void Player::KilledMonster(CreatureInfo const* cInfo, ObjectGuid guid)
{
    if (cInfo->Entry)
        KilledMonsterCredit(cInfo->Entry, guid);

    for (int i = 0; i < MAX_KILL_CREDIT; ++i)
        if (cInfo->KillCredit[i])
            KilledMonsterCredit(cInfo->KillCredit[i], guid);
}

void Player::KilledMonsterCredit(uint32 entry, ObjectGuid guid)
{
    uint32 addkillcount = 1;

    for (int i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if (!questid)
            continue;

        Quest const* qInfo = sObjectMgr.GetQuestTemplate(questid);
        if (!qInfo)
            continue;
        // just if !ingroup || !noraidgroup || raidgroup
        QuestStatusData& q_status = mQuestStatus[questid];
        if (q_status.m_status == QUEST_STATUS_INCOMPLETE && (!GetGroup() || !GetGroup()->isRaidGroup() || qInfo->IsAllowedInRaid()))
        {
            if (qInfo->HasSpecialFlag(QUEST_SPECIAL_FLAG_KILL_OR_CAST))
            {
                for (int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j)
                {
                    // skip GO activate objective or none
                    if (qInfo->ReqCreatureOrGOId[j] <= 0)
                        continue;

                    // skip Cast at creature objective
                    if (qInfo->ReqSpell[j] != 0)
                        continue;

                    uint32 reqkill = qInfo->ReqCreatureOrGOId[j];

                    if (reqkill == entry)
                    {
                        uint32 reqkillcount = qInfo->ReqCreatureOrGOCount[j];
                        uint32 curkillcount = q_status.m_creatureOrGOcount[j];
                        if (curkillcount < reqkillcount)
                        {
                            q_status.m_creatureOrGOcount[j] = curkillcount + addkillcount;
                            if (q_status.uState != QUEST_NEW)
                                q_status.uState = QUEST_CHANGED;

                            SendQuestUpdateAddCreatureOrGo(qInfo, guid, j, q_status.m_creatureOrGOcount[j]);
                        }

                        if (CanCompleteQuest(questid))
                            CompleteQuest(questid);

                        // same objective target can be in many active quests, but not in 2 objectives for single quest (code optimization).
                        continue;
                    }
                }
            }
        }
    }
}

void Player::CastedCreatureOrGO(uint32 entry, ObjectGuid guid, uint32 spell_id, bool original_caster)
{
    bool isCreature = guid.IsCreature();

    uint32 addCastCount = 1;
    for (int i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if (!questid)
            continue;

        Quest const* qInfo = sObjectMgr.GetQuestTemplate(questid);
        if (!qInfo)
            continue;

        if (!original_caster && !qInfo->HasQuestFlag(QUEST_FLAGS_SHARABLE))
            continue;

        if (!qInfo->HasSpecialFlag(QUEST_SPECIAL_FLAG_KILL_OR_CAST))
            continue;

        QuestStatusData& q_status = mQuestStatus[questid];

        if (q_status.m_status != QUEST_STATUS_INCOMPLETE)
            continue;

        for (int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j)
        {
            // skip kill creature objective (0) or wrong spell casts
            if (qInfo->ReqSpell[j] != spell_id)
                continue;

            uint32 reqTarget = 0;

            if (isCreature)
            {
                // creature activate objectives
                if (qInfo->ReqCreatureOrGOId[j] > 0)
                    // checked at quest_template loading
                    reqTarget = qInfo->ReqCreatureOrGOId[j];
            }
            else
            {
                // GO activate objective
                if (qInfo->ReqCreatureOrGOId[j] < 0)
                    // checked at quest_template loading
                    reqTarget = - qInfo->ReqCreatureOrGOId[j];
            }

            // other not this creature/GO related objectives
            if (reqTarget != entry)
                continue;

            uint32 reqCastCount = qInfo->ReqCreatureOrGOCount[j];
            uint32 curCastCount = q_status.m_creatureOrGOcount[j];
            if (curCastCount < reqCastCount)
            {
                q_status.m_creatureOrGOcount[j] = curCastCount + addCastCount;
                if (q_status.uState != QUEST_NEW)
                    q_status.uState = QUEST_CHANGED;

                SendQuestUpdateAddCreatureOrGo(qInfo, guid, j, q_status.m_creatureOrGOcount[j]);
            }

            if (CanCompleteQuest(questid))
                CompleteQuest(questid);

            // same objective target can be in many active quests, but not in 2 objectives for single quest (code optimization).
            break;
        }
    }
}

void Player::TalkedToCreature(uint32 entry, ObjectGuid guid)
{
    uint32 addTalkCount = 1;
    for (int i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if (!questid)
            continue;

        Quest const* qInfo = sObjectMgr.GetQuestTemplate(questid);
        if (!qInfo)
            continue;

        QuestStatusData& q_status = mQuestStatus[questid];

        if (q_status.m_status == QUEST_STATUS_INCOMPLETE)
        {
            if (qInfo->HasSpecialFlag(QuestSpecialFlags(QUEST_SPECIAL_FLAG_KILL_OR_CAST | QUEST_SPECIAL_FLAG_SPEAKTO)))
            {
                for (int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j)
                {
                    // skip spell casts and Gameobject objectives
                    if (qInfo->ReqSpell[j] > 0 || qInfo->ReqCreatureOrGOId[j] < 0)
                        continue;

                    uint32 reqTarget = 0;

                    if (qInfo->ReqCreatureOrGOId[j] > 0)    // creature activate objectives
                        // checked at quest_template loading
                        reqTarget = qInfo->ReqCreatureOrGOId[j];
                    else
                        continue;

                    if (reqTarget == entry)
                    {
                        uint32 reqTalkCount = qInfo->ReqCreatureOrGOCount[j];
                        uint32 curTalkCount = q_status.m_creatureOrGOcount[j];
                        if (curTalkCount < reqTalkCount)
                        {
                            q_status.m_creatureOrGOcount[j] = curTalkCount + addTalkCount;
                            if (q_status.uState != QUEST_NEW) q_status.uState = QUEST_CHANGED;

                            SendQuestUpdateAddCreatureOrGo(qInfo, guid, j, q_status.m_creatureOrGOcount[j]);
                        }
                        if (CanCompleteQuest(questid))
                            CompleteQuest(questid);

                        // same objective target can be in many active quests, but not in 2 objectives for single quest (code optimization).
                        continue;
                    }
                }
            }
        }
    }
}

void Player::MoneyChanged(uint32 count)
{
    for (int i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if (!questid)
            continue;

        Quest const* qInfo = sObjectMgr.GetQuestTemplate(questid);
        if (qInfo && qInfo->GetRewOrReqMoney() < 0)
        {
            QuestStatusData& q_status = mQuestStatus[questid];

            if (q_status.m_status == QUEST_STATUS_INCOMPLETE)
            {
                if (int32(count) >= -qInfo->GetRewOrReqMoney())
                {
                    if (CanCompleteQuest(questid))
                        CompleteQuest(questid);
                }
            }
            else if (q_status.m_status == QUEST_STATUS_COMPLETE)
            {
                if (int32(count) < -qInfo->GetRewOrReqMoney())
                    IncompleteQuest(questid);
            }
        }
    }
}

void Player::ReputationChanged(FactionEntry const* factionEntry)
{
    for (int i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        if (uint32 questid = GetQuestSlotQuestId(i))
        {
            if (Quest const* qInfo = sObjectMgr.GetQuestTemplate(questid))
            {
                if (qInfo->GetRepObjectiveFaction() == factionEntry->ID)
                {
                    QuestStatusData& q_status = mQuestStatus[questid];
                    if (q_status.m_status == QUEST_STATUS_INCOMPLETE)
                    {
                        if (GetReputationMgr().GetReputation(factionEntry) >= qInfo->GetRepObjectiveValue())
                            if (CanCompleteQuest(questid))
                                CompleteQuest(questid);
                    }
                    else if (q_status.m_status == QUEST_STATUS_COMPLETE)
                    {
                        if (GetReputationMgr().GetReputation(factionEntry) < qInfo->GetRepObjectiveValue())
                            IncompleteQuest(questid);
                    }
                }
            }
        }
    }
}

bool Player::HasQuestForItem(uint32 itemid) const
{
    for (int i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if (questid == 0)
            continue;

        QuestStatusMap::const_iterator qs_itr = mQuestStatus.find(questid);
        if (qs_itr == mQuestStatus.end())
            continue;

        QuestStatusData const& q_status = qs_itr->second;

        if (q_status.m_status == QUEST_STATUS_INCOMPLETE)
        {
            Quest const* qinfo = sObjectMgr.GetQuestTemplate(questid);
            if (!qinfo)
                continue;

            // hide quest if player is in raid-group and quest is no raid quest
            if (GetGroup() && GetGroup()->isRaidGroup() && !qinfo->IsAllowedInRaid() && !InBattleGround())
                continue;

            // There should be no mixed ReqItem/ReqSource drop
            // This part for ReqItem drop
            for (int j = 0; j < QUEST_ITEM_OBJECTIVES_COUNT; ++j)
            {
                if (itemid == qinfo->ReqItemId[j] && q_status.m_itemcount[j] < qinfo->ReqItemCount[j])
                    return true;
            }
            // This part - for ReqSource
            for (int j = 0; j < QUEST_SOURCE_ITEM_IDS_COUNT; ++j)
            {
                // examined item is a source item
                if (qinfo->ReqSourceId[j] == itemid)
                {
                    ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(itemid);

                    // 'unique' item
                    if (pProto->MaxCount && GetItemCount(itemid, true) < pProto->MaxCount)
                        return true;

                    // allows custom amount drop when not 0
                    if (qinfo->ReqSourceCount[j])
                    {
                        if (GetItemCount(itemid, true) < qinfo->ReqSourceCount[j])
                            return true;
                    }
                    else if (GetItemCount(itemid, true) < pProto->Stackable)
                        return true;
                }
            }
        }
    }
    return false;
}

// Used for quests having some event (explore, escort, "external event") as quest objective.
void Player::SendQuestCompleteEvent(uint32 quest_id)
{
    if (quest_id)
    {
        WorldPacket data(SMSG_QUESTUPDATE_COMPLETE, 4);
        data << uint32(quest_id);
        GetSession()->SendPacket(&data);
        DEBUG_LOG("WORLD: Sent SMSG_QUESTUPDATE_COMPLETE quest = %u", quest_id);
    }
}

void Player::SendQuestReward(Quest const* pQuest, uint32 XP, Object* questGiver)
{
    Player* pPlayer = m_session->GetPlayer();

    uint32 questid = pQuest->GetQuestId();
    DEBUG_LOG("WORLD: Sent SMSG_QUESTGIVER_QUEST_COMPLETE quest = %u", questid);
    WorldPacket data(SMSG_QUESTGIVER_QUEST_COMPLETE, (4 + 4 + 4 + 4 + 4 + pQuest->GetRewItemsCount() * 8));
    data << uint32(questid);
    data << uint32(0x03);

    if (getLevel() < sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
    {
        data << uint32(XP);
        data << uint32(pQuest->GetRewOrReqMoney());
    }
    else
    {
        data << uint32(0);
        data << uint32(pQuest->GetRewOrReqMoney() + int32(pQuest->GetRewMoneyMaxLevel() * sWorld.getConfig(CONFIG_FLOAT_RATE_DROP_MONEY)));
    }
    data << uint32(pQuest->GetRewItemsCount());             // max is 5

    for (uint32 i = 0; i < pQuest->GetRewItemsCount(); ++i)
    {
        if (pQuest->RewItemId[i] > 0)
            data << pQuest->RewItemId[i] << pQuest->RewItemCount[i];
        else
            data << uint32(0) << uint32(0);
    }
    GetSession()->SendPacket(&data);

    // used by eluna
    if (Creature* pCreature = questGiver->ToCreature())
        sHookMgr->OnQuestComplete(pPlayer, pCreature, pQuest);

    // used by eluna
    if (GameObject* pGameObject = questGiver->ToGameObject())
        sHookMgr->OnQuestComplete(pPlayer, pGameObject, pQuest);
}

void Player::SendQuestFailed(uint32 quest_id)
{
    if (quest_id)
    {
        WorldPacket data(SMSG_QUESTGIVER_QUEST_FAILED, 4);
        data << uint32(quest_id);
        GetSession()->SendPacket(&data);
        DEBUG_LOG("WORLD: Sent SMSG_QUESTGIVER_QUEST_FAILED");
    }
}

void Player::SendQuestTimerFailed(uint32 quest_id)
{
    if (quest_id)
    {
        WorldPacket data(SMSG_QUESTUPDATE_FAILEDTIMER, 4);
        data << uint32(quest_id);
        GetSession()->SendPacket(&data);
        DEBUG_LOG("WORLD: Sent SMSG_QUESTUPDATE_FAILEDTIMER");
    }
}

void Player::SendCanTakeQuestResponse(uint32 msg) const
{
    WorldPacket data(SMSG_QUESTGIVER_QUEST_INVALID, 4);
    data << uint32(msg);
    GetSession()->SendPacket(&data);
    DEBUG_LOG("WORLD: Sent SMSG_QUESTGIVER_QUEST_INVALID");
}

void Player::SendQuestConfirmAccept(const Quest* pQuest, Player* pReceiver)
{
    if (pReceiver)
    {
        int loc_idx = pReceiver->GetSession()->GetSessionDbLocaleIndex();
        std::string title = pQuest->GetTitle();
        sObjectMgr.GetQuestLocaleStrings(pQuest->GetQuestId(), loc_idx, &title);

        WorldPacket data(SMSG_QUEST_CONFIRM_ACCEPT, (4 + title.size() + 8));
        data << uint32(pQuest->GetQuestId());
        data << title;
        data << GetObjectGuid();
        pReceiver->GetSession()->SendPacket(&data);

        DEBUG_LOG("WORLD: Sent SMSG_QUEST_CONFIRM_ACCEPT");
    }
}

void Player::SendPushToPartyResponse(Player* pPlayer, uint32 msg)
{
    if (pPlayer)
    {
        WorldPacket data(MSG_QUEST_PUSH_RESULT, (8 + 4 + 1));
        data << pPlayer->GetObjectGuid();
        data << uint32(msg);                                // valid values: 0-8
        data << uint8(0);
        GetSession()->SendPacket(&data);
        DEBUG_LOG("WORLD: Sent MSG_QUEST_PUSH_RESULT");
    }
}

void Player::SendQuestUpdateAddItem(Quest const* pQuest, uint32 item_idx, uint32 count)
{
    DEBUG_LOG("WORLD: Sent SMSG_QUESTUPDATE_ADD_ITEM");
    WorldPacket data(SMSG_QUESTUPDATE_ADD_ITEM, (4 + 4));
    data << pQuest->ReqItemId[item_idx];
    data << count;
    GetSession()->SendPacket(&data);
}

void Player::SendQuestUpdateAddCreatureOrGo(Quest const* pQuest, ObjectGuid guid, uint32 creatureOrGO_idx, uint32 count)
{
    MANGOS_ASSERT(count < 64 && "mob/GO count store in 6 bits 2^6 = 64 (0..63)");

    int32 entry = pQuest->ReqCreatureOrGOId[ creatureOrGO_idx ];
    if (entry < 0)
        // client expected gameobject template id in form (id|0x80000000)
        entry = (-entry) | 0x80000000;

    WorldPacket data(SMSG_QUESTUPDATE_ADD_KILL, (4 * 4 + 8));
    DEBUG_LOG("WORLD: Sent SMSG_QUESTUPDATE_ADD_KILL");
    data << uint32(pQuest->GetQuestId());
    data << uint32(entry);
    data << uint32(count);
    data << uint32(pQuest->ReqCreatureOrGOCount[ creatureOrGO_idx ]);
    data << guid;
    GetSession()->SendPacket(&data);

    uint16 log_slot = FindQuestSlot(pQuest->GetQuestId());
    if (log_slot < MAX_QUEST_LOG_SIZE)
        SetQuestSlotCounter(log_slot, creatureOrGO_idx, count);
}

/*********************************************************/
/***                   LOAD SYSTEM                     ***/
/*********************************************************/

void Player::_LoadBGData(QueryResult* result)
{
    if (!result)
        return;

    // Expecting only one row
    Field* fields = result->Fetch();
    /* bgInstanceID, bgTeam, x, y, z, o, map */
    m_bgData.bgInstanceID = fields[0].GetUInt32();
    m_bgData.bgTeam       = Team(fields[1].GetUInt32());
    m_bgData.joinPos      = WorldLocation(fields[6].GetUInt32(),    // Map
                                          fields[2].GetFloat(),     // X
                                          fields[3].GetFloat(),     // Y
                                          fields[4].GetFloat(),     // Z
                                          fields[5].GetFloat());    // Orientation

    delete result;
}

bool Player::LoadPositionFromDB(ObjectGuid guid, uint32& mapid, float& x, float& y, float& z, float& o, bool& in_flight)
{
    QueryResult* result = CharacterDatabase.PQuery("SELECT position_x,position_y,position_z,orientation,map,taxi_path FROM characters WHERE guid = '%u'", guid.GetCounter());
    if (!result)
        return false;

    Field* fields = result->Fetch();

    x = fields[0].GetFloat();
    y = fields[1].GetFloat();
    z = fields[2].GetFloat();
    o = fields[3].GetFloat();
    mapid = fields[4].GetUInt32();
    in_flight = !fields[5].GetCppString().empty();

    delete result;
    return true;
}

void Player::_LoadIntoDataField(const char* data, uint32 startOffset, uint32 count)
{
    if (!data)
        return;

    Tokens tokens = StrSplit(data, " ");

    if (tokens.size() != count)
        return;

    Tokens::iterator iter;
    uint32 index;
    for (iter = tokens.begin(), index = 0; index < count; ++iter, ++index)
    {
        m_uint32Values[startOffset + index] = atol((*iter).c_str());
    }
}

bool Player::LoadFromDB(ObjectGuid guid, SqlQueryHolder* holder)
{
    //       0     1        2     3     4      5       6      7   8      9            10            11
    // SELECT guid, account, name, race, class, gender, level, xp, money, playerBytes, playerBytes2, playerFlags,"
    // 12          13          14          15   16           17        18         19         20         21          22           23                 24
    //"position_x, position_y, position_z, map, orientation, taximask, cinematic, totaltime, leveltime, rest_bonus, logout_time, is_logout_resting, resettalents_cost,"
    // 25                 26       27       28       29       30         31           32            33        34    35      36                 37
    //"resettalents_time, trans_x, trans_y, trans_z, trans_o, transguid, extra_flags, stable_slots, at_login, zone, online, death_expire_time, taxi_path,
    // 38                  39              40                   41                        42
    //"honor_highest_rank, honor_standing, stored_honor_rating, stored_dishonorablekills, stored_honorable_kills,"
    // 43               44
    //"watchedFaction,  drunk,"
    // 45      46      47      48      49      50      51             52              53      54
    //"health, power1, power2, power3, power4, power5, exploredZones, equipmentCache, ammoId, actionBars  FROM characters WHERE guid = '%u'", GUID_LOPART(m_guid));
    QueryResult* result = holder->GetResult(PLAYER_LOGIN_QUERY_LOADFROM);

    if (!result)
    {
        sLog.outError("%s not found in table `characters`, can't load. ", guid.GetString().c_str());
        return false;
    }

    Field* fields = result->Fetch();

    uint32 dbAccountId = fields[1].GetUInt32();

    // check if the character's account in the db and the logged in account match.
    // player should be able to load/delete character only with correct account!
    if (dbAccountId != GetSession()->GetAccountId())
    {
        sLog.outError("%s loading from wrong account (is: %u, should be: %u)",
                      guid.GetString().c_str(), GetSession()->GetAccountId(), dbAccountId);
        delete result;
        return false;
    }

    Object::_Create(guid.GetCounter(), 0, HIGHGUID_PLAYER);

    m_name = fields[2].GetCppString();

    // check name limitations
    if (ObjectMgr::CheckPlayerName(m_name) != CHAR_NAME_SUCCESS ||
            (GetSession()->GetSecurity() == SEC_PLAYER && sObjectMgr.IsReservedName(m_name)))
    {
        delete result;
        CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE guid ='%u'",
                                   uint32(AT_LOGIN_RENAME), guid.GetCounter());
        return false;
    }

    // overwrite possible wrong/corrupted guid
    SetGuidValue(OBJECT_FIELD_GUID, guid);

    // overwrite some data fields
    SetByteValue(UNIT_FIELD_BYTES_0, 0, fields[3].GetUInt8()); // race
    SetByteValue(UNIT_FIELD_BYTES_0, 1, fields[4].GetUInt8()); // class

    uint8 gender = fields[5].GetUInt8() & 0x01;             // allowed only 1 bit values male/female cases (for fit drunk gender part)
    SetByteValue(UNIT_FIELD_BYTES_0, 2, gender);            // gender

    SetByteValue(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_UNK3 | UNIT_BYTE2_FLAG_UNK5);

    SetUInt32Value(UNIT_FIELD_LEVEL, fields[6].GetUInt8());
    SetUInt32Value(PLAYER_XP, fields[7].GetUInt32());

    _LoadIntoDataField(fields[51].GetString(), PLAYER_EXPLORED_ZONES_1, PLAYER_EXPLORED_ZONES_SIZE);

    InitDisplayIds();                                       // model, scale and model data

    uint32 money = fields[8].GetUInt32();
    if (money > MAX_MONEY_AMOUNT)
        money = MAX_MONEY_AMOUNT;
    SetMoney(money);

    SetUInt32Value(PLAYER_BYTES, fields[9].GetUInt32());
    SetUInt32Value(PLAYER_BYTES_2, fields[10].GetUInt32());

    m_drunk = fields[44].GetUInt16();

    SetUInt16Value(PLAYER_BYTES_3, 0, (m_drunk & 0xFFFE) | gender);

    SetUInt32Value(PLAYER_FLAGS, fields[11].GetUInt32());
    SetInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, fields[43].GetInt32());

    SetUInt32Value(PLAYER_AMMO_ID, fields[53].GetUInt32());

    // Action bars state
    SetByteValue(PLAYER_FIELD_BYTES, 2, fields[54].GetUInt8());

    // cleanup inventory related item value fields (its will be filled correctly in _LoadInventory)
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        SetGuidValue(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2), ObjectGuid());
        SetVisibleItemSlot(slot, NULL);

        delete m_items[slot];
        m_items[slot] = NULL;
    }

    DEBUG_FILTER_LOG(LOG_FILTER_PLAYER_STATS, "Load Basic value of player %s is: ", m_name.c_str());
    outDebugStatsValues();

    // Need to call it to initialize m_team (m_team can be calculated from race)
    // Other way is to saves m_team into characters table.
    setFactionForRace(getRace());
    SetCharm(NULL);

    // load home bind and check in same time class/race pair, it used later for restore broken positions
    if (!_LoadHomeBind(holder->GetResult(PLAYER_LOGIN_QUERY_LOADHOMEBIND)))
    {
        delete result;
        return false;
    }

    InitPrimaryProfessions();                               // to max set before any spell loaded

    // init saved position, and fix it later if problematic
    uint32 transGUID = fields[30].GetUInt32();
    Relocate(fields[12].GetFloat(), fields[13].GetFloat(), fields[14].GetFloat(), fields[16].GetFloat());
    SetLocationMapId(fields[15].GetUInt32());

    _LoadGroup(holder->GetResult(PLAYER_LOGIN_QUERY_LOADGROUP));

    m_highest_rank.rank        = fields[38].GetUInt32();
    m_highest_rank  = MaNGOS::Honor::CalculateRankInfo(m_highest_rank);
    m_standing_pos             = fields[39].GetUInt32();
    m_stored_honor             = fields[40].GetFloat();
    m_stored_dishonorableKills = fields[41].GetUInt32();
    m_stored_honorableKills    = fields[42].GetUInt32();

    _LoadHonorCP(holder->GetResult(PLAYER_LOGIN_QUERY_LOADHONORCP));

    _LoadBoundInstances(holder->GetResult(PLAYER_LOGIN_QUERY_LOADBOUNDINSTANCES));

    if (!IsPositionValid())
    {
        sLog.outError("%s have invalid coordinates (X: %f Y: %f Z: %f O: %f). Teleport to default race/class locations.",
                      guid.GetString().c_str(), GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
        RelocateToHomebind();

        transGUID = 0;

        m_movementInfo.ClearTransportData();
    }

    _LoadBGData(holder->GetResult(PLAYER_LOGIN_QUERY_LOADBGDATA));

    if (m_bgData.bgInstanceID)                              // saved in BattleGround
    {
        BattleGround* currentBg = sBattleGroundMgr.GetBattleGround(m_bgData.bgInstanceID, BATTLEGROUND_TYPE_NONE);

        bool player_at_bg = currentBg && currentBg->IsPlayerInBattleGround(GetObjectGuid());

        if (player_at_bg && currentBg->GetStatus() != STATUS_WAIT_LEAVE)
        {
            BattleGroundQueueTypeId bgQueueTypeId = sBattleGroundMgr.BGQueueTypeId(currentBg->GetTypeID());
            AddBattleGroundQueueId(bgQueueTypeId);

            m_bgData.bgTypeID = currentBg->GetTypeID();     // bg data not marked as modified

            // join player to battleground group
            currentBg->EventPlayerLoggedIn(this, GetObjectGuid());
            currentBg->AddOrSetPlayerToCorrectBgGroup(this, GetObjectGuid(), m_bgData.bgTeam);

            SetInviteForBattleGroundQueueType(bgQueueTypeId, currentBg->GetInstanceID());
        }
        else
        {
            // leave bg
            if (player_at_bg)
                currentBg->RemovePlayerAtLeave(GetObjectGuid(), false, true);

            // move to bg enter point
            const WorldLocation& _loc = GetBattleGroundEntryPoint();
            SetLocationMapId(_loc.mapid);
            Relocate(_loc.coord_x, _loc.coord_y, _loc.coord_z, _loc.orientation);

            // We are not in BG anymore
            SetBattleGroundId(0, BATTLEGROUND_TYPE_NONE);
            // remove outdated DB data in DB
            _SaveBGData();
        }
    }
    else
    {
        MapEntry const* mapEntry = sMapStore.LookupEntry(GetMapId());
        // if server restart after player save in BG or area
        // player can have current coordinates in to BG map, fix this
        if (!mapEntry || mapEntry->IsBattleGround())
        {
            const WorldLocation& _loc = GetBattleGroundEntryPoint();
            SetLocationMapId(_loc.mapid);
            Relocate(_loc.coord_x, _loc.coord_y, _loc.coord_z, _loc.orientation);

            // We are not in BG anymore
            SetBattleGroundId(0, BATTLEGROUND_TYPE_NONE);
            // remove outdated DB data in DB
            _SaveBGData();
        }
    }

    if (transGUID != 0)
    {
        m_movementInfo.SetTransportData(ObjectGuid(HIGHGUID_MO_TRANSPORT, transGUID), fields[26].GetFloat(), fields[27].GetFloat(), fields[28].GetFloat(), fields[29].GetFloat(), 0);

        if (!MaNGOS::IsValidMapCoord(
                    GetPositionX() + m_movementInfo.GetTransportPos()->x, GetPositionY() + m_movementInfo.GetTransportPos()->y,
                    GetPositionZ() + m_movementInfo.GetTransportPos()->z, GetOrientation() + m_movementInfo.GetTransportPos()->o) ||
                // transport size limited
                m_movementInfo.GetTransportPos()->x > 50 || m_movementInfo.GetTransportPos()->y > 50 || m_movementInfo.GetTransportPos()->z > 50)
        {
            sLog.outError("%s have invalid transport coordinates (X: %f Y: %f Z: %f O: %f). Teleport to default race/class locations.",
                          guid.GetString().c_str(), GetPositionX() + m_movementInfo.GetTransportPos()->x, GetPositionY() + m_movementInfo.GetTransportPos()->y,
                          GetPositionZ() + m_movementInfo.GetTransportPos()->z, GetOrientation() + m_movementInfo.GetTransportPos()->o);

            RelocateToHomebind();

            m_movementInfo.ClearTransportData();

            transGUID = 0;
        }
    }

    if (transGUID != 0)
    {
        for (MapManager::TransportSet::const_iterator iter = sMapMgr.m_Transports.begin(); iter != sMapMgr.m_Transports.end(); ++iter)
        {
            if ((*iter)->GetGUIDLow() == transGUID)
            {
                m_transport = *iter;
                m_transport->AddPassenger(this);
                SetLocationMapId(m_transport->GetMapId());
                break;
            }
        }

        if (!m_transport)
        {
            sLog.outError("%s have problems with transport guid (%u). Teleport to default race/class locations.",
                          guid.GetString().c_str(), transGUID);

            RelocateToHomebind();

            m_movementInfo.ClearTransportData();

            transGUID = 0;
        }
    }

    // player bounded instance saves loaded in _LoadBoundInstances, group versions at group loading
    DungeonPersistentState* state = GetBoundInstanceSaveForSelfOrGroup(GetMapId());

    // load the player's map here if it's not already loaded
    SetMap(sMapMgr.CreateMap(GetMapId(), this));

    // if the player is in an instance and it has been reset in the meantime teleport him to the entrance
    if (GetInstanceId() && !state)
    {
        AreaTrigger const* at = sObjectMgr.GetMapEntranceTrigger(GetMapId());
        if (at)
            Relocate(at->target_X, at->target_Y, at->target_Z, at->target_Orientation);
        else
            sLog.outError("Player %s(GUID: %u) logged in to a reset instance (map: %u) and there is no area-trigger leading to this map. Thus he can't be ported back to the entrance. This _might_ be an exploit attempt.", GetName(), GetGUIDLow(), GetMapId());
    }

    SaveRecallPosition();

    time_t now = time(NULL);
    time_t logoutTime = time_t(fields[22].GetUInt64());

    // since last logout (in seconds)
    uint32 time_diff = uint32(now - logoutTime);

    // set value, including drunk invisibility detection
    // calculate sobering. after 15 minutes logged out, the player will be sober again
    float soberFactor;
    if (time_diff > 15 * MINUTE)
        soberFactor = 0;
    else
        soberFactor = 1 - time_diff / (15.0f * MINUTE);
    uint16 newDrunkenValue = uint16(soberFactor * m_drunk);
    SetDrunkValue(newDrunkenValue);

    m_cinematic = fields[18].GetUInt32();
    m_Played_time[PLAYED_TIME_TOTAL] = fields[19].GetUInt32();
    m_Played_time[PLAYED_TIME_LEVEL] = fields[20].GetUInt32();

    m_resetTalentsCost = fields[24].GetUInt32();
    m_resetTalentsTime = time_t(fields[25].GetUInt64());

    // reserve some flags
    uint32 old_safe_flags = GetUInt32Value(PLAYER_FLAGS) & (PLAYER_FLAGS_HIDE_CLOAK | PLAYER_FLAGS_HIDE_HELM);

    if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GM))
        SetUInt32Value(PLAYER_FLAGS, 0 | old_safe_flags);

    m_taxi.LoadTaxiMask(fields[17].GetString());

    uint32 extraflags = fields[31].GetUInt32();

    m_stableSlots = fields[32].GetUInt32();
    if (m_stableSlots > MAX_PET_STABLES)
    {
        sLog.outError("Player can have not more %u stable slots, but have in DB %u", MAX_PET_STABLES, uint32(m_stableSlots));
        m_stableSlots = MAX_PET_STABLES;
    }

    m_atLoginFlags = fields[33].GetUInt32();

    m_deathExpireTime = (time_t)fields[36].GetUInt64();
    if (m_deathExpireTime > now + MAX_DEATH_COUNT * DEATH_EXPIRE_STEP)
        m_deathExpireTime = now + MAX_DEATH_COUNT * DEATH_EXPIRE_STEP - 1;

    std::string taxi_nodes = fields[37].GetCppString();

    // clear channel spell data (if saved at channel spell casting)
    SetChannelObjectGuid(ObjectGuid());
    SetUInt32Value(UNIT_CHANNEL_SPELL, 0);

    // clear charm/summon related fields
    SetCharm(NULL);
    SetPet(NULL);
    SetTargetGuid(ObjectGuid());
    SetCharmerGuid(ObjectGuid());
    SetOwnerGuid(ObjectGuid());
    SetCreatorGuid(ObjectGuid());

    // reset some aura modifiers before aura apply

    SetGuidValue(PLAYER_FARSIGHT, ObjectGuid());
    SetUInt32Value(PLAYER_TRACK_CREATURES, 0);
    SetUInt32Value(PLAYER_TRACK_RESOURCES, 0);

    // cleanup aura list explicitly before skill load where some spells can be applied
    RemoveAllAuras();

    // make sure the unit is considered out of combat for proper loading
    ClearInCombat();

    // make sure the unit is considered not in duel for proper loading
    SetGuidValue(PLAYER_DUEL_ARBITER, ObjectGuid());
    SetUInt32Value(PLAYER_DUEL_TEAM, 0);

    // reset stats before loading any modifiers
    InitStatsForLevel();

    // is it need, only in pre-2.x used and field byte removed later?
    if (getPowerType() == POWER_RAGE || getPowerType() == POWER_MANA)
        SetByteValue(UNIT_FIELD_BYTES_1, 1, 0xEE);

    // rest bonus can only be calculated after InitStatsForLevel()
    m_rest_bonus = fields[21].GetFloat();

    if (time_diff > 0)
    {
        // speed collect rest bonus in offline, in logout, far from tavern, city (section/in hour)
        float bubble0 = 0.031f;
        // speed collect rest bonus in offline, in logout, in tavern, city (section/in hour)
        float bubble1 = 0.125f;
        float bubble = fields[23].GetUInt32() > 0
                       ? bubble1 * sWorld.getConfig(CONFIG_FLOAT_RATE_REST_OFFLINE_IN_TAVERN_OR_CITY)
                       : bubble0 * sWorld.getConfig(CONFIG_FLOAT_RATE_REST_OFFLINE_IN_WILDERNESS);

        SetRestBonus(GetRestBonus() + time_diff * ((float)GetUInt32Value(PLAYER_NEXT_LEVEL_XP) / 72000)*bubble);
    }

    // load skills after InitStatsForLevel because it triggering aura apply also
    _LoadSkills(holder->GetResult(PLAYER_LOGIN_QUERY_LOADSKILLS));

    // apply original stats mods before spell loading or item equipment that call before equip _RemoveStatsMods()

    // Mail
    _LoadMails(holder->GetResult(PLAYER_LOGIN_QUERY_LOADMAILS));
    _LoadMailedItems(holder->GetResult(PLAYER_LOGIN_QUERY_LOADMAILEDITEMS));
    UpdateNextMailTimeAndUnreads();

    _LoadAuras(holder->GetResult(PLAYER_LOGIN_QUERY_LOADAURAS), time_diff);

    // add ghost flag (must be after aura load: PLAYER_FLAGS_GHOST set in aura)
    if (HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        m_deathState = DEAD;

    _LoadSpells(holder->GetResult(PLAYER_LOGIN_QUERY_LOADSPELLS));

    // after spell load
    InitTalentForLevel();
    learnDefaultSpells();

    // after spell load, learn rewarded spell if need also
    _LoadQuestStatus(holder->GetResult(PLAYER_LOGIN_QUERY_LOADQUESTSTATUS));

    // must be before inventory (some items required reputation check)
    m_reputationMgr.LoadFromDB(holder->GetResult(PLAYER_LOGIN_QUERY_LOADREPUTATION));

    _LoadInventory(holder->GetResult(PLAYER_LOGIN_QUERY_LOADINVENTORY), time_diff);
    _LoadItemLoot(holder->GetResult(PLAYER_LOGIN_QUERY_LOADITEMLOOT));

    // update items with duration and realtime
    UpdateItemDuration(time_diff, true);

    _LoadActions(holder->GetResult(PLAYER_LOGIN_QUERY_LOADACTIONS));

    m_social = sSocialMgr.LoadFromDB(holder->GetResult(PLAYER_LOGIN_QUERY_LOADSOCIALLIST), GetObjectGuid());

    if (!m_taxi.LoadTaxiDestinationsFromString(taxi_nodes, GetTeam()))
    {
        // problems with taxi path loading
        TaxiNodesEntry const* nodeEntry = NULL;
        if (uint32 node_id = m_taxi.GetTaxiSource())
            nodeEntry = sTaxiNodesStore.LookupEntry(node_id);

        if (!nodeEntry)                                     // don't know taxi start node, to homebind
        {
            sLog.outError("Character %u have wrong data in taxi destination list, teleport to homebind.", GetGUIDLow());
            RelocateToHomebind();
        }
        else                                                // have start node, to it
        {
            sLog.outError("Character %u have too short taxi destination list, teleport to original node.", GetGUIDLow());
            SetLocationMapId(nodeEntry->map_id);
            Relocate(nodeEntry->x, nodeEntry->y, nodeEntry->z, 0.0f);
        }

        // we can be relocated from taxi and still have an outdated Map pointer!
        // so we need to get a new Map pointer!
        SetMap(sMapMgr.CreateMap(GetMapId(), this));
        SaveRecallPosition();                           // save as recall also to prevent recall and fall from sky

        m_taxi.ClearTaxiDestinations();
    }

    if (uint32 node_id = m_taxi.GetTaxiSource())
    {
        // save source node as recall coord to prevent recall and fall from sky
        TaxiNodesEntry const* nodeEntry = sTaxiNodesStore.LookupEntry(node_id);
        MANGOS_ASSERT(nodeEntry);                           // checked in m_taxi.LoadTaxiDestinationsFromString
        m_recallMap = nodeEntry->map_id;
        m_recallX = nodeEntry->x;
        m_recallY = nodeEntry->y;
        m_recallZ = nodeEntry->z;

        // flight will started later
    }

    // has to be called after last Relocate() in Player::LoadFromDB
    SetFallInformation(0, GetPositionZ());

    _LoadSpellCooldowns(holder->GetResult(PLAYER_LOGIN_QUERY_LOADSPELLCOOLDOWNS));

    // Spell code allow apply any auras to dead character in load time in aura/spell/item loading
    // Do now before stats re-calculation cleanup for ghost state unexpected auras
    if (!isAlive())
        RemoveAllAurasOnDeath();

    // apply all stat bonuses from items and auras
    SetCanModifyStats(true);
    UpdateAllStats();

    // restore remembered power/health values (but not more max values)
    uint32 savedhealth = fields[45].GetUInt32();
    SetHealth(savedhealth > GetMaxHealth() ? GetMaxHealth() : savedhealth);
    for (uint32 i = 0; i < MAX_POWERS; ++i)
    {
        uint32 savedpower = fields[46 + i].GetUInt32();
        SetPower(Powers(i), savedpower > GetMaxPower(Powers(i)) ? GetMaxPower(Powers(i)) : savedpower);
    }

    DEBUG_FILTER_LOG(LOG_FILTER_PLAYER_STATS, "The value of player %s after load item and aura is: ", m_name.c_str());
    outDebugStatsValues();

    // all fields read
    delete result;

    // GM state
    if (GetSession()->GetSecurity() > SEC_PLAYER)
    {
        switch (sWorld.getConfig(CONFIG_UINT32_GM_LOGIN_STATE))
        {
            default:
            case 0:                      break;             // disable
            case 1: SetGameMaster(true); break;             // enable
            case 2:                                         // save state
                if (extraflags & PLAYER_EXTRA_GM_ON)
                    SetGameMaster(true);
                break;
        }

        switch (sWorld.getConfig(CONFIG_UINT32_GM_VISIBLE_STATE))
        {
            default:
            case 0: SetGMVisible(false); break;             // invisible
            case 1:                      break;             // visible
            case 2:                                         // save state
                if (extraflags & PLAYER_EXTRA_GM_INVISIBLE)
                    SetGMVisible(false);
                break;
        }

        switch (sWorld.getConfig(CONFIG_UINT32_GM_ACCEPT_TICKETS))
        {
            default:
            case 0:                        break;           // disable
            case 1: SetAcceptTicket(true); break;           // enable
            case 2:                                         // save state
                if (extraflags & PLAYER_EXTRA_GM_ACCEPT_TICKETS)
                    SetAcceptTicket(true);
                break;
        }

        switch (sWorld.getConfig(CONFIG_UINT32_GM_CHAT))
        {
            default:
            case 0:                  break;                 // disable
            case 1: SetGMChat(true); break;                 // enable
            case 2:                                         // save state
                if (extraflags & PLAYER_EXTRA_GM_CHAT)
                    SetGMChat(true);
                break;
        }

        switch (sWorld.getConfig(CONFIG_UINT32_GM_WISPERING_TO))
        {
            default:
            case 0:                          break;         // disable
            case 1: SetAcceptWhispers(true); break;         // enable
            case 2:                                         // save state
                if (extraflags & PLAYER_EXTRA_ACCEPT_WHISPERS)
                    SetAcceptWhispers(true);
                break;
        }
    }

    return true;
}

bool Player::isAllowedToLoot(Creature* creature)
{
    // never tapped by any (mob solo kill)
    if (!creature->HasFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TAPPED))
        return false;

    if (Player* recipient = creature->GetLootRecipient())
    {
        if (recipient == this)
            return true;

        if (Group* otherGroup = recipient->GetGroup())
        {
            Group* thisGroup = GetGroup();
            if (!thisGroup)
                return false;

            return thisGroup == otherGroup;
        }
        return false;
    }
    else
        // prevent other players from looting if the recipient got disconnected
        return !creature->HasLootRecipient();
}

void Player::_LoadActions(QueryResult* result)
{
    m_actionButtons.clear();

    // QueryResult *result = CharacterDatabase.PQuery("SELECT button,action,type FROM character_action WHERE guid = '%u' ORDER BY button",GetGUIDLow());

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            uint8 button = fields[0].GetUInt8();
            uint32 action = fields[1].GetUInt32();
            uint8 type = fields[2].GetUInt8();

            if (ActionButton* ab = addActionButton(button, action, type))
                ab->uState = ACTIONBUTTON_UNCHANGED;
            else
            {
                sLog.outError("  ...at loading, and will deleted in DB also");

                // Will deleted in DB at next save (it can create data until save but marked as deleted)
                m_actionButtons[button].uState = ACTIONBUTTON_DELETED;
            }
        }
        while (result->NextRow());

        delete result;
    }
}

void Player::_LoadAuras(QueryResult* result, uint32 timediff)
{
    // RemoveAllAuras(); -- some spells casted before aura load, for example in LoadSkills, aura list explicitly cleaned early

    // all aura related fields
    for (int i = UNIT_FIELD_AURA; i <= UNIT_FIELD_AURASTATE; ++i)
        SetUInt32Value(i, 0);

    // QueryResult *result = CharacterDatabase.PQuery("SELECT caster_guid,item_guid,spell,stackcount,remaincharges,basepoints0,basepoints1,basepoints2,periodictime0,periodictime1,periodictime2,maxduration,remaintime,effIndexMask FROM character_aura WHERE guid = '%u'",GetGUIDLow());

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            ObjectGuid caster_guid = ObjectGuid(fields[0].GetUInt64());
            uint32 item_lowguid = fields[1].GetUInt32();
            uint32 spellid = fields[2].GetUInt32();
            uint32 stackcount = fields[3].GetUInt32();
            uint32 remaincharges = fields[4].GetUInt32();
            int32  damage[MAX_EFFECT_INDEX];
            uint32 periodicTime[MAX_EFFECT_INDEX];

            for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
            {
                damage[i] = fields[i + 5].GetInt32();
                periodicTime[i] = fields[i + 8].GetUInt32();
            }

            int32 maxduration = fields[11].GetInt32();
            int32 remaintime = fields[12].GetInt32();
            uint32 effIndexMask = fields[13].GetUInt32();

            SpellEntry const* spellproto = sSpellStore.LookupEntry(spellid);
            if (!spellproto)
            {
                sLog.outError("Unknown spell (spellid %u), ignore.", spellid);
                continue;
            }

            if (remaintime != -1 && !IsPositiveSpell(spellproto))
            {
                if (remaintime / IN_MILLISECONDS <= int32(timediff))
                    continue;

                remaintime -= timediff * IN_MILLISECONDS;
            }

            // prevent wrong values of remaincharges
            if (spellproto->procCharges == 0)
                remaincharges = 0;

            if (!spellproto->StackAmount)
                stackcount = 1;
            else if (spellproto->StackAmount < stackcount)
                stackcount = spellproto->StackAmount;
            else if (!stackcount)
                stackcount = 1;

            SpellAuraHolder* holder = CreateSpellAuraHolder(spellproto, this, NULL);
            holder->SetLoadedState(caster_guid, ObjectGuid(HIGHGUID_ITEM, item_lowguid), stackcount, remaincharges, maxduration, remaintime);

            for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
            {
                if ((effIndexMask & (1 << i)) == 0)
                    continue;

                Aura* aura = CreateAura(spellproto, SpellEffectIndex(i), NULL, holder, this);
                if (!damage[i])
                    damage[i] = aura->GetModifier()->m_amount;

                aura->SetLoadedState(damage[i], periodicTime[i]);
                holder->AddAura(aura, SpellEffectIndex(i));
            }

            if (!holder->IsEmptyHolder())
            {
                // reset stolen single target auras
                if (caster_guid != GetObjectGuid() && holder->GetTrackedAuraType() == TRACK_AURA_TYPE_SINGLE_TARGET)
                    holder->SetTrackedAuraType(TRACK_AURA_TYPE_NOT_TRACKED);

                AddSpellAuraHolder(holder);
                DETAIL_LOG("Added auras from spellid %u", spellproto->Id);
            }
            else
                delete holder;
        }
        while (result->NextRow());
        delete result;
    }

    if (getClass() == CLASS_WARRIOR && !HasAuraType(SPELL_AURA_MOD_SHAPESHIFT))
        CastSpell(this, SPELL_ID_PASSIVE_BATTLE_STANCE, true);
}

void Player::LoadCorpse()
{
    if (isAlive())
    {
        sObjectAccessor.ConvertCorpseForPlayer(GetObjectGuid());
    }
    else
    {
        if (Corpse* corpse = GetCorpse())
        {
            ApplyModByteFlag(PLAYER_FIELD_BYTES, 0, PLAYER_FIELD_BYTE_RELEASE_TIMER, corpse && !sMapStore.LookupEntry(corpse->GetMapId())->Instanceable());
        }
        else
        {
            // Prevent Dead Player login without corpse
            ResurrectPlayer(0.5f);
        }
    }
}

void Player::_LoadInventory(QueryResult* result, uint32 timediff)
{
    // QueryResult *result = CharacterDatabase.PQuery("SELECT data,bag,slot,item,item_template FROM character_inventory JOIN item_instance ON character_inventory.item = item_instance.guid WHERE character_inventory.guid = '%u' ORDER BY bag,slot", GetGUIDLow());
    std::map<uint32, Bag*> bagMap;                          // fast guid lookup for bags
    // NOTE: the "order by `bag`" is important because it makes sure
    // the bagMap is filled before items in the bags are loaded
    // NOTE2: the "order by `slot`" is needed because mainhand weapons are (wrongly?)
    // expected to be equipped before offhand items (TODO: fixme)

    uint32 zone = GetZoneId();

    if (result)
    {
        std::list<Item*> problematicItems;

        // prevent items from being added to the queue when stored
        m_itemUpdateQueueBlocked = true;
        do
        {
            Field* fields = result->Fetch();
            uint32 bag_guid  = fields[1].GetUInt32();
            uint8  slot      = fields[2].GetUInt8();
            uint32 item_lowguid = fields[3].GetUInt32();
            uint32 item_id   = fields[4].GetUInt32();

            ItemPrototype const* proto = ObjectMgr::GetItemPrototype(item_id);

            if (!proto)
            {
                CharacterDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", item_lowguid);
                CharacterDatabase.PExecute("DELETE FROM item_instance WHERE guid = '%u'", item_lowguid);
                sLog.outError("Player::_LoadInventory: Player %s has an unknown item (id: #%u) in inventory, deleted.", GetName(), item_id);
                continue;
            }

            Item* item = NewItemOrBag(proto);

            if (!item->LoadFromDB(item_lowguid, fields, GetObjectGuid()))
            {
                sLog.outError("Player::_LoadInventory: Player %s has broken item (id: #%u) in inventory, deleted.", GetName(), item_id);
                CharacterDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", item_lowguid);
                item->FSetState(ITEM_REMOVED);
                item->SaveToDB();                           // it also deletes item object !
                continue;
            }

            // not allow have in alive state item limited to another map/zone
            if (isAlive() && item->IsLimitedToAnotherMapOrZone(GetMapId(), zone))
            {
                CharacterDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", item_lowguid);
                item->FSetState(ITEM_REMOVED);
                item->SaveToDB();                           // it also deletes item object !
                continue;
            }

            // "Conjured items disappear if you are logged out for more than 15 minutes"
            if (timediff > 15 * MINUTE && (item->GetProto()->Flags & ITEM_FLAG_CONJURED))
            {
                CharacterDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", item_lowguid);
                item->FSetState(ITEM_REMOVED);
                item->SaveToDB();                           // it also deletes item object !
                continue;
            }

            bool success = true;

            // the item/bag is not in a bag
            if (!bag_guid)
            {
                item->SetContainer(NULL);
                item->SetSlot(slot);

                if (IsInventoryPos(INVENTORY_SLOT_BAG_0, slot))
                {
                    ItemPosCountVec dest;
                    if (CanStoreItem(INVENTORY_SLOT_BAG_0, slot, dest, item, false) == EQUIP_ERR_OK)
                        item = StoreItem(dest, item, true);
                    else
                        success = false;
                }
                else if (IsEquipmentPos(INVENTORY_SLOT_BAG_0, slot))
                {
                    uint16 dest;
                    if (CanEquipItem(slot, dest, item, false, false) == EQUIP_ERR_OK)
                        QuickEquipItem(dest, item);
                    else
                        success = false;
                }
                else if (IsBankPos(INVENTORY_SLOT_BAG_0, slot))
                {
                    ItemPosCountVec dest;
                    if (CanBankItem(INVENTORY_SLOT_BAG_0, slot, dest, item, false, false) == EQUIP_ERR_OK)
                        item = BankItem(dest, item, true);
                    else
                        success = false;
                }

                if (success)
                {
                    // store bags that may contain items in them
                    if (item->IsBag() && IsBagPos(item->GetPos()))
                        bagMap[item_lowguid] = (Bag*)item;
                }
            }
            // the item/bag in a bag
            else
            {
                item->SetSlot(NULL_SLOT);
                // the item is in a bag, find the bag
                std::map<uint32, Bag*>::const_iterator itr = bagMap.find(bag_guid);
                if (itr != bagMap.end() && slot < itr->second->GetBagSize())
                {
                    ItemPosCountVec dest;
                    if (CanStoreItem(itr->second->GetSlot(), slot, dest, item, false) == EQUIP_ERR_OK)
                        item = StoreItem(dest, item, true);
                    else
                        success = false;
                }
                else
                    success = false;
            }

            // item's state may have changed after stored
            if (success)
            {
                item->SetState(ITEM_UNCHANGED, this);

                // restore container unchanged state also
                if (item->GetContainer())
                    item->GetContainer()->SetState(ITEM_UNCHANGED, this);
            }
            else
            {
                sLog.outError("Player::_LoadInventory: Player %s has item (GUID: %u Entry: %u) can't be loaded to inventory (Bag GUID: %u Slot: %u) by some reason, will send by mail.", GetName(), item_lowguid, item_id, bag_guid, slot);
                CharacterDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", item_lowguid);
                problematicItems.push_back(item);
            }
        }
        while (result->NextRow());

        delete result;
        m_itemUpdateQueueBlocked = false;

        // send by mail problematic items
        while (!problematicItems.empty())
        {
            std::string subject = GetSession()->GetMangosString(LANG_NOT_EQUIPPED_ITEM);

            // fill mail
            MailDraft draft(subject);

            for (int i = 0; !problematicItems.empty() && i < MAX_MAIL_ITEMS; ++i)
            {
                Item* item = problematicItems.front();
                problematicItems.pop_front();

                draft.AddItem(item);
            }

            draft.SendMailTo(this, MailSender(this, MAIL_STATIONERY_GM), MAIL_CHECK_MASK_COPIED);
        }
    }

    // if(isAlive())
    _ApplyAllItemMods();
}

void Player::_LoadHonorCP(QueryResult* result)
{
    if (result)
    {
        m_honorCP.clear();

        do
        {
            Field* fields = result->Fetch();

            HonorCP CP;
            CP.victimType       = fields[0].GetUInt8();
            CP.victimID         = fields[1].GetUInt32();
            CP.honorPoints      = fields[2].GetFloat();
            CP.date             = fields[3].GetUInt32();
            CP.type             = fields[4].GetUInt8();
            CP.state            = HK_UNCHANGED;
            CP.isKill           = isKill(CP.victimType);

            m_honorCP.push_back(CP);
        }
        while (result->NextRow());

        delete result;
    }
}

void Player::_LoadItemLoot(QueryResult* result)
{
    // QueryResult *result = CharacterDatabase.PQuery("SELECT guid,itemid,amount,property FROM item_loot WHERE guid = '%u'", GetGUIDLow());

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 item_guid   = fields[0].GetUInt32();

            Item* item = GetItemByGuid(ObjectGuid(HIGHGUID_ITEM, item_guid));

            if (!item)
            {
                CharacterDatabase.PExecute("DELETE FROM item_loot WHERE guid = '%u'", item_guid);
                sLog.outError("Player::_LoadItemLoot: Player %s has loot for nonexistent item (GUID: %u) in `item_loot`, deleted.", GetName(), item_guid);
                continue;
            }

            item->LoadLootFromDB(fields);
        }
        while (result->NextRow());

        delete result;
    }
}

// load mailed item which should receive current player
void Player::_LoadMailedItems(QueryResult* result)
{
    // data needs to be at first place for Item::LoadFromDB
    //         0     1        2          3
    // "SELECT data, mail_id, item_guid, item_template FROM mail_items JOIN item_instance ON item_guid = guid WHERE receiver = '%u'", GUID_LOPART(m_guid)
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        uint32 mail_id       = fields[1].GetUInt32();
        uint32 item_guid_low = fields[2].GetUInt32();
        uint32 item_template = fields[3].GetUInt32();

        Mail* mail = GetMail(mail_id);
        if (!mail)
            continue;
        mail->AddItem(item_guid_low, item_template);

        ItemPrototype const* proto = ObjectMgr::GetItemPrototype(item_template);

        if (!proto)
        {
            sLog.outError("Player %u has unknown item_template (ProtoType) in mailed items(GUID: %u template: %u) in mail (%u), deleted.", GetGUIDLow(), item_guid_low, item_template, mail->messageID);
            CharacterDatabase.PExecute("DELETE FROM mail_items WHERE item_guid = '%u'", item_guid_low);
            CharacterDatabase.PExecute("DELETE FROM item_instance WHERE guid = '%u'", item_guid_low);
            continue;
        }

        Item* item = NewItemOrBag(proto);

        if (!item->LoadFromDB(item_guid_low, fields, GetObjectGuid()))
        {
            sLog.outError("Player::_LoadMailedItems - Item in mail (%u) doesn't exist !!!! - item guid: %u, deleted from mail", mail->messageID, item_guid_low);
            CharacterDatabase.PExecute("DELETE FROM mail_items WHERE item_guid = '%u'", item_guid_low);
            item->FSetState(ITEM_REMOVED);
            item->SaveToDB();                               // it also deletes item object !
            continue;
        }

        AddMItem(item);
    }
    while (result->NextRow());

    delete result;
}

void Player::_LoadMails(QueryResult* result)
{
    m_mail.clear();
    //        0  1           2      3        4       5          6           7            8     9   10      11         12             13
    //"SELECT id,messageType,sender,receiver,subject,itemTextId,expire_time,deliver_time,money,cod,checked,stationery,mailTemplateId,has_items FROM mail WHERE receiver = '%u' ORDER BY id DESC",GetGUIDLow()
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        Mail* m = new Mail;
        m->messageID = fields[0].GetUInt32();
        m->messageType = fields[1].GetUInt8();
        m->sender = fields[2].GetUInt32();
        m->receiverGuid = ObjectGuid(HIGHGUID_PLAYER, fields[3].GetUInt32());
        m->subject = fields[4].GetCppString();
        m->itemTextId = fields[5].GetUInt32();
        m->expire_time = (time_t)fields[6].GetUInt64();
        m->deliver_time = (time_t)fields[7].GetUInt64();
        m->money = fields[8].GetUInt32();
        m->COD = fields[9].GetUInt32();
        m->checked = fields[10].GetUInt32();
        m->stationery = fields[11].GetUInt8();
        m->mailTemplateId = fields[12].GetInt16();
        m->has_items = fields[13].GetBool();                // true, if mail have items or mail have template and items generated (maybe none)

        if (m->mailTemplateId && !sMailTemplateStore.LookupEntry(m->mailTemplateId))
        {
            sLog.outError("Player::_LoadMail - Mail (%u) have nonexistent MailTemplateId (%u), remove at load", m->messageID, m->mailTemplateId);
            m->mailTemplateId = 0;
        }

        m->state = MAIL_STATE_UNCHANGED;

        m_mail.push_back(m);

        if (m->mailTemplateId && !m->has_items)
            m->prepareTemplateItems(this);
    }
    while (result->NextRow());
    delete result;
}

void Player::LoadPet()
{
    // fixme: the pet should still be loaded if the player is not in world
    // just not added to the map
    if (IsInWorld())
    {
        Pet* pet = new Pet;
        if (!pet->LoadPetFromDB(this, 0, 0, true))
            delete pet;
    }
}

void Player::_LoadQuestStatus(QueryResult* result)
{
    mQuestStatus.clear();

    uint32 slot = 0;

    ////                                                     0      1       2         3         4      5          6          7          8          9           10          11          12
    // QueryResult *result = CharacterDatabase.PQuery("SELECT quest, status, rewarded, explored, timer, mobcount1, mobcount2, mobcount3, mobcount4, itemcount1, itemcount2, itemcount3, itemcount4 FROM character_queststatus WHERE guid = '%u'", GetGUIDLow());

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 quest_id = fields[0].GetUInt32();
            // used to be new, no delete?
            Quest const* pQuest = sObjectMgr.GetQuestTemplate(quest_id);
            if (pQuest)
            {
                // find or create
                QuestStatusData& questStatusData = mQuestStatus[quest_id];

                uint32 qstatus = fields[1].GetUInt32();
                if (qstatus < MAX_QUEST_STATUS)
                    questStatusData.m_status = QuestStatus(qstatus);
                else
                {
                    questStatusData.m_status = QUEST_STATUS_NONE;
                    sLog.outError("Player %s have invalid quest %d status (%d), replaced by QUEST_STATUS_NONE(0).", GetName(), quest_id, qstatus);
                }

                questStatusData.m_rewarded = (fields[2].GetUInt8() > 0);
                questStatusData.m_explored = (fields[3].GetUInt8() > 0);

                time_t quest_time = time_t(fields[4].GetUInt64());

                if (pQuest->HasSpecialFlag(QUEST_SPECIAL_FLAG_TIMED) && !GetQuestRewardStatus(quest_id) && questStatusData.m_status != QUEST_STATUS_NONE)
                {
                    AddTimedQuest(quest_id);

                    if (quest_time <= sWorld.GetGameTime())
                        questStatusData.m_timer = 1;
                    else
                        questStatusData.m_timer = uint32(quest_time - sWorld.GetGameTime()) * IN_MILLISECONDS;
                }
                else
                    quest_time = 0;

                questStatusData.m_creatureOrGOcount[0] = fields[5].GetUInt32();
                questStatusData.m_creatureOrGOcount[1] = fields[6].GetUInt32();
                questStatusData.m_creatureOrGOcount[2] = fields[7].GetUInt32();
                questStatusData.m_creatureOrGOcount[3] = fields[8].GetUInt32();
                questStatusData.m_itemcount[0] = fields[9].GetUInt32();
                questStatusData.m_itemcount[1] = fields[10].GetUInt32();
                questStatusData.m_itemcount[2] = fields[11].GetUInt32();
                questStatusData.m_itemcount[3] = fields[12].GetUInt32();

                questStatusData.uState = QUEST_UNCHANGED;

                // add to quest log
                if (slot < MAX_QUEST_LOG_SIZE &&
                        ((questStatusData.m_status == QUEST_STATUS_INCOMPLETE ||
                          questStatusData.m_status == QUEST_STATUS_COMPLETE ||
                          questStatusData.m_status == QUEST_STATUS_FAILED) &&
                         (!questStatusData.m_rewarded || pQuest->IsRepeatable())))
                {
                    SetQuestSlot(slot, quest_id, uint32(quest_time));

                    if (questStatusData.m_explored)
                        SetQuestSlotState(slot, QUEST_STATE_COMPLETE);

                    if (questStatusData.m_status == QUEST_STATUS_COMPLETE)
                        SetQuestSlotState(slot, QUEST_STATE_COMPLETE);

                    if (questStatusData.m_status == QUEST_STATUS_FAILED)
                        SetQuestSlotState(slot, QUEST_STATE_FAIL);

                    for (uint8 idx = 0; idx < QUEST_OBJECTIVES_COUNT; ++idx)
                        if (questStatusData.m_creatureOrGOcount[idx])
                            SetQuestSlotCounter(slot, idx, questStatusData.m_creatureOrGOcount[idx]);

                    ++slot;
                }

                if (questStatusData.m_rewarded)
                {
                    // learn rewarded spell if unknown
                    learnQuestRewardedSpells(pQuest);
                }

                DEBUG_LOG("Quest status is {%u} for quest {%u} for player (GUID: %u)", questStatusData.m_status, quest_id, GetGUIDLow());
            }
        }
        while (result->NextRow());

        delete result;
    }

    // clear quest log tail
    for (uint16 i = slot; i < MAX_QUEST_LOG_SIZE; ++i)
        SetQuestSlot(i, 0);
}

void Player::_LoadSpells(QueryResult* result)
{
    // QueryResult *result = CharacterDatabase.PQuery("SELECT spell,active,disabled FROM character_spell WHERE guid = '%u'",GetGUIDLow());

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 spell_id = fields[0].GetUInt32();

            addSpell(spell_id, fields[1].GetBool(), false, false, fields[2].GetBool());
        }
        while (result->NextRow());

        delete result;
    }
}

void Player::_LoadGroup(QueryResult* result)
{
    // QueryResult *result = CharacterDatabase.PQuery("SELECT groupId FROM group_member WHERE memberGuid='%u'", GetGUIDLow());
    if (result)
    {
        uint32 groupId = (*result)[0].GetUInt32();
        delete result;

        if (Group* group = sObjectMgr.GetGroupById(groupId))
        {
            uint8 subgroup = group->GetMemberGroup(GetObjectGuid());
            SetGroup(group, subgroup);
        }
    }
}

void Player::_LoadBoundInstances(QueryResult* result)
{
    m_boundInstances.clear();

    Group* group = GetGroup();

    // QueryResult *result = CharacterDatabase.PQuery("SELECT id, permanent, map, resettime FROM character_instance LEFT JOIN instance ON instance = id WHERE guid = '%u'", GUID_LOPART(m_guid));
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            bool perm = fields[1].GetBool();
            uint32 mapId = fields[2].GetUInt32();
            uint32 instanceId = fields[0].GetUInt32();
            time_t resetTime = (time_t)fields[3].GetUInt64();
            // the resettime for normal instances is only saved when the InstanceSave is unloaded
            // so the value read from the DB may be wrong here but only if the InstanceSave is loaded
            // and in that case it is not used

            MapEntry const* mapEntry = sMapStore.LookupEntry(mapId);
            if (!mapEntry || !mapEntry->IsDungeon())
            {
                sLog.outError("_LoadBoundInstances: player %s(%d) has bind to nonexistent or not dungeon map %d", GetName(), GetGUIDLow(), mapId);
                CharacterDatabase.PExecute("DELETE FROM character_instance WHERE guid = '%u' AND instance = '%u'", GetGUIDLow(), instanceId);
                continue;
            }

            if (!perm && group)
            {
                sLog.outError("_LoadBoundInstances: %s is in group (Id: %d) but has a non-permanent character bind to map %d,%d",
                              GetGuidStr().c_str(), group->GetId(), mapId, instanceId);
                CharacterDatabase.PExecute("DELETE FROM character_instance WHERE guid = '%u' AND instance = '%u'",
                                           GetGUIDLow(), instanceId);
                continue;
            }

            // since non permanent binds are always solo bind, they can always be reset
            DungeonPersistentState* state = (DungeonPersistentState*)sMapPersistentStateMgr.AddPersistentState(mapEntry, instanceId, resetTime, !perm, true);
            if (state) BindToInstance(state, perm, true);
        }
        while (result->NextRow());
        delete result;
    }
}

InstancePlayerBind* Player::GetBoundInstance(uint32 mapid)
{
    const MapEntry* entry = sMapStore.LookupEntry(mapid);

    BoundInstancesMap::iterator itr = m_boundInstances.find(mapid);
    if (itr != m_boundInstances.end())
        return &itr->second;
    else
        return NULL;
}

void Player::UnbindInstance(uint32 mapid, bool unload)
{
    BoundInstancesMap::iterator itr = m_boundInstances.find(mapid);
    UnbindInstance(itr, unload);
}

void Player::UnbindInstance(BoundInstancesMap::iterator& itr, bool unload)
{
    if (itr != m_boundInstances.end())
    {
        if (!unload)
            CharacterDatabase.PExecute("DELETE FROM character_instance WHERE guid = '%u' AND instance = '%u'",
                                       GetGUIDLow(), itr->second.state->GetInstanceId());
        itr->second.state->RemovePlayer(this);              // state can become invalid
        m_boundInstances.erase(itr++);
    }
}

InstancePlayerBind* Player::BindToInstance(DungeonPersistentState* state, bool permanent, bool load)
{
    if (state)
    {
        InstancePlayerBind& bind = m_boundInstances[state->GetMapId()];
        if (bind.state)
        {
            // update the state when the group kills a boss
            if (permanent != bind.perm || state != bind.state)
                if (!load)
                    CharacterDatabase.PExecute("UPDATE character_instance SET instance = '%u', permanent = '%u' WHERE guid = '%u' AND instance = '%u'",
                                               state->GetInstanceId(), permanent, GetGUIDLow(), bind.state->GetInstanceId());
        }
        else
        {
            if (!load)
                CharacterDatabase.PExecute("INSERT INTO character_instance (guid, instance, permanent) VALUES ('%u', '%u', '%u')",
                                           GetGUIDLow(), state->GetInstanceId(), permanent);
        }

        if (bind.state != state)
        {
            if (bind.state)
                bind.state->RemovePlayer(this);
            state->AddPlayer(this);
        }

        if (permanent)
            state->SetCanReset(false);

        bind.state = state;
        bind.perm = permanent;
        if (!load)
            DEBUG_LOG("Player::BindToInstance: %s(%d) is now bound to map %d, instance %d",
                      GetName(), GetGUIDLow(), state->GetMapId(), state->GetInstanceId());
        // used by eluna
        sHookMgr->OnBindToInstance(this, (Difficulty)0, state->GetMapId(), permanent);
        return &bind;
    }
    else
        return NULL;
}

DungeonPersistentState* Player::GetBoundInstanceSaveForSelfOrGroup(uint32 mapid)
{
    MapEntry const* mapEntry = sMapStore.LookupEntry(mapid);
    if (!mapEntry)
        return NULL;

    InstancePlayerBind* pBind = GetBoundInstance(mapid);
    DungeonPersistentState* state = pBind ? pBind->state : NULL;

    // the player's permanent player bind is taken into consideration first
    // then the player's group bind and finally the solo bind.
    if (!pBind || !pBind->perm)
    {
        InstanceGroupBind* groupBind = NULL;
        if (Group* group = GetGroup())
            if ((groupBind = group->GetBoundInstance(mapid)))
                state = groupBind->state;
    }

    return state;
}

void Player::SendRaidInfo()
{
    uint32 counter = 0;

    WorldPacket data(SMSG_RAID_INSTANCE_INFO, 4);

    size_t p_counter = data.wpos();
    data << uint32(counter);                                // placeholder

    time_t now = time(NULL);

    for (BoundInstancesMap::const_iterator itr = m_boundInstances.begin(); itr != m_boundInstances.end(); ++itr)
    {
        if (itr->second.perm)
        {
            DungeonPersistentState* state = itr->second.state;
            data << uint32(state->GetMapId());              // map id
            data << uint32(state->GetResetTime() - time(NULL));
            data << uint32(state->GetInstanceId());         // instance id
            counter++;
        }
    }

    data.put<uint32>(p_counter, counter);
    GetSession()->SendPacket(&data);
}

/*
- called on every successful teleportation to a map
*/
void Player::SendSavedInstances()
{
    bool hasBeenSaved = false;
    WorldPacket data;

    for (BoundInstancesMap::const_iterator itr = m_boundInstances.begin(); itr != m_boundInstances.end(); ++itr)
    {
        if (itr->second.perm)                               // only permanent binds are sent
        {
            hasBeenSaved = true;
            break;
        }
    }

    // Send opcode 811. true or false means, whether you have current raid instances
    data.Initialize(SMSG_UPDATE_INSTANCE_OWNERSHIP);
    data << uint32(hasBeenSaved);
    GetSession()->SendPacket(&data);

    if (!hasBeenSaved)
        return;

    for (BoundInstancesMap::const_iterator itr = m_boundInstances.begin(); itr != m_boundInstances.end(); ++itr)
    {
        if (itr->second.perm)
        {
            data.Initialize(SMSG_UPDATE_LAST_INSTANCE);
            data << uint32(itr->second.state->GetMapId());
            GetSession()->SendPacket(&data);
        }
    }
}

/// convert the player's binds to the group
void Player::ConvertInstancesToGroup(Player* player, Group* group, ObjectGuid player_guid)
{
    bool has_binds = false;
    bool has_solo = false;

    if (player)
    {
        player_guid = player->GetObjectGuid();
        if (!group)
            group = player->GetGroup();
    }

    MANGOS_ASSERT(player_guid);

    // copy all binds to the group, when changing leader it's assumed the character
    // will not have any solo binds

    if (player)
    {
        for (BoundInstancesMap::iterator itr = player->m_boundInstances.begin(); itr != player->m_boundInstances.end();)
        {
            has_binds = true;

            if (group)
                group->BindToInstance(itr->second.state, itr->second.perm, true);

            // permanent binds are not removed
            if (!itr->second.perm)
            {
                // increments itr in call
                player->UnbindInstance(itr, true);
                has_solo = true;
            }
            else
                ++itr;
        }
    }

    uint32 player_lowguid = player_guid.GetCounter();

    // if the player's not online we don't know what binds it has
    if (!player || !group || has_binds)
        CharacterDatabase.PExecute("INSERT INTO group_instance SELECT guid, instance, permanent FROM character_instance WHERE guid = '%u'", player_lowguid);

    // the following should not get executed when changing leaders
    if (!player || has_solo)
        CharacterDatabase.PExecute("DELETE FROM character_instance WHERE guid = '%u' AND permanent = 0", player_lowguid);
}

bool Player::_LoadHomeBind(QueryResult* result)
{
    PlayerInfo const* info = sObjectMgr.GetPlayerInfo(getRace(), getClass());
    if (!info)
    {
        sLog.outError("Player have incorrect race/class pair. Can't be loaded.");
        return false;
    }

    bool ok = false;
    // QueryResult *result = CharacterDatabase.PQuery("SELECT map,zone,position_x,position_y,position_z FROM character_homebind WHERE guid = '%u'", GUID_LOPART(playerGuid));
    if (result)
    {
        Field* fields = result->Fetch();
        m_homebindMapId = fields[0].GetUInt32();
        m_homebindAreaId = fields[1].GetUInt16();
        m_homebindX = fields[2].GetFloat();
        m_homebindY = fields[3].GetFloat();
        m_homebindZ = fields[4].GetFloat();
        delete result;

        MapEntry const* bindMapEntry = sMapStore.LookupEntry(m_homebindMapId);

        // accept saved data only for valid position (and non instanceable), and accessable
        if (MapManager::IsValidMapCoord(m_homebindMapId, m_homebindX, m_homebindY, m_homebindZ) &&
                !bindMapEntry->Instanceable())
        {
            ok = true;
        }
        else
            CharacterDatabase.PExecute("DELETE FROM character_homebind WHERE guid = '%u'", GetGUIDLow());
    }

    if (!ok)
    {
        m_homebindMapId = info->mapId;
        m_homebindAreaId = info->areaId;
        m_homebindX = info->positionX;
        m_homebindY = info->positionY;
        m_homebindZ = info->positionZ;

        CharacterDatabase.PExecute("INSERT INTO character_homebind (guid,map,zone,position_x,position_y,position_z) VALUES ('%u', '%u', '%u', '%f', '%f', '%f')", GetGUIDLow(), m_homebindMapId, (uint32)m_homebindAreaId, m_homebindX, m_homebindY, m_homebindZ);
    }

    DEBUG_LOG("Setting player home position: mapid is: %u, zoneid is %u, X is %f, Y is %f, Z is %f",
              m_homebindMapId, m_homebindAreaId, m_homebindX, m_homebindY, m_homebindZ);

    return true;
}

/*********************************************************/
/***                   SAVE SYSTEM                     ***/
/*********************************************************/

void Player::SaveToDB()
{
    // we should assure this: ASSERT((m_nextSave != sWorld.getConfig(CONFIG_UINT32_INTERVAL_SAVE)));
    // delay auto save at any saves (manual, in code, or autosave)
    m_nextSave = sWorld.getConfig(CONFIG_UINT32_INTERVAL_SAVE);

    // lets allow only players in world to be saved
    if (IsBeingTeleportedFar())
    {
        ScheduleDelayedOperation(DELAYED_SAVE_PLAYER);
        return;
    }

    DEBUG_FILTER_LOG(LOG_FILTER_PLAYER_STATS, "The value of player %s at save: ", m_name.c_str());
    outDebugStatsValues();

    CharacterDatabase.BeginTransaction();

    UpdateHonor();

    static SqlStatementID delChar ;
    static SqlStatementID insChar ;

    SqlStatement stmt = CharacterDatabase.CreateStatement(delChar, "DELETE FROM characters WHERE guid = ?");
    stmt.PExecute(GetGUIDLow());

    SqlStatement uberInsert = CharacterDatabase.CreateStatement(insChar, "INSERT INTO characters (guid,account,name,race,class,gender,level,xp,money,playerBytes,playerBytes2,playerFlags,"
                              "map, position_x, position_y, position_z, orientation, "
                              "taximask, online, cinematic, "
                              "totaltime, leveltime, rest_bonus, logout_time, is_logout_resting, resettalents_cost, resettalents_time, "
                              "trans_x, trans_y, trans_z, trans_o, transguid, extra_flags, stable_slots, at_login, zone, "
                              "death_expire_time, taxi_path, "
                              "honor_highest_rank, honor_standing, stored_honor_rating , stored_dishonorable_kills, stored_honorable_kills, "
                              "watchedFaction, drunk, health, power1, power2, power3, "
                              "power4, power5, exploredZones, equipmentCache, ammoId, actionBars) "
                              "VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,"
                              "?, ?, ?, ?, ?, "
                              "?, ?, ?, "
                              "?, ?, ?, ?, ?, ?, ?, "
                              "?, ?, ?, ?, ?, ?, ?, ?, ?, "
                              "?, ?, "
                              "?, ?, ?, ?, ?, "
                              "?, ?, ?, ?, ?, ?, "
                              "?, ?, ?, ?, ?, ?) ");

    uberInsert.addUInt32(GetGUIDLow());
    uberInsert.addUInt32(GetSession()->GetAccountId());
    uberInsert.addString(m_name);
    uberInsert.addUInt8(getRace());
    uberInsert.addUInt8(getClass());
    uberInsert.addUInt8(getGender());
    uberInsert.addUInt32(getLevel());
    uberInsert.addUInt32(GetUInt32Value(PLAYER_XP));
    uberInsert.addUInt32(GetMoney());
    uberInsert.addUInt32(GetUInt32Value(PLAYER_BYTES));
    uberInsert.addUInt32(GetUInt32Value(PLAYER_BYTES_2));
    uberInsert.addUInt32(GetUInt32Value(PLAYER_FLAGS));

    if (!IsBeingTeleported())
    {
        uberInsert.addUInt32(GetMapId());
        uberInsert.addFloat(finiteAlways(GetPositionX()));
        uberInsert.addFloat(finiteAlways(GetPositionY()));
        uberInsert.addFloat(finiteAlways(GetPositionZ()));
        uberInsert.addFloat(finiteAlways(GetOrientation()));
    }
    else
    {
        uberInsert.addUInt32(GetTeleportDest().mapid);
        uberInsert.addFloat(finiteAlways(GetTeleportDest().coord_x));
        uberInsert.addFloat(finiteAlways(GetTeleportDest().coord_y));
        uberInsert.addFloat(finiteAlways(GetTeleportDest().coord_z));
        uberInsert.addFloat(finiteAlways(GetTeleportDest().orientation));
    }

    std::ostringstream ss;
    ss << m_taxi;                                   // string with TaxiMaskSize numbers
    uberInsert.addString(ss);

    uberInsert.addUInt32(IsInWorld() ? 1 : 0);

    uberInsert.addUInt32(m_cinematic);

    uberInsert.addUInt32(m_Played_time[PLAYED_TIME_TOTAL]);
    uberInsert.addUInt32(m_Played_time[PLAYED_TIME_LEVEL]);

    uberInsert.addFloat(finiteAlways(m_rest_bonus));
    uberInsert.addUInt64(uint64(time(NULL)));
    uberInsert.addUInt32(HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) ? 1 : 0);
    // save, far from tavern/city
    // save, but in tavern/city
    uberInsert.addUInt32(m_resetTalentsCost);
    uberInsert.addUInt64(uint64(m_resetTalentsTime));

    uberInsert.addFloat(finiteAlways(m_movementInfo.GetTransportPos()->x));
    uberInsert.addFloat(finiteAlways(m_movementInfo.GetTransportPos()->y));
    uberInsert.addFloat(finiteAlways(m_movementInfo.GetTransportPos()->z));
    uberInsert.addFloat(finiteAlways(m_movementInfo.GetTransportPos()->o));
    if (m_transport)
        uberInsert.addUInt32(m_transport->GetGUIDLow());
    else
        uberInsert.addUInt32(0);

    uberInsert.addUInt32(m_ExtraFlags);

    uberInsert.addUInt32(uint32(m_stableSlots));            // to prevent save uint8 as char

    uberInsert.addUInt32(uint32(m_atLoginFlags));

    uberInsert.addUInt32(IsInWorld() ? GetZoneId() : GetCachedZoneId());

    uberInsert.addUInt64(uint64(m_deathExpireTime));

    ss << m_taxi.SaveTaxiDestinationsToString();       // string
    uberInsert.addString(ss);

    uberInsert.addUInt32(uint32(m_highest_rank.rank));
    uberInsert.addInt32(m_standing_pos);
    uberInsert.addFloat(finiteAlways(m_stored_honor));
    uberInsert.addUInt32(m_stored_dishonorableKills);
    uberInsert.addUInt32(m_stored_honorableKills);

    // FIXME: at this moment send to DB as unsigned, including unit32(-1)
    uberInsert.addUInt32(GetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX));

    uberInsert.addUInt16(uint16(GetUInt32Value(PLAYER_BYTES_3) & 0xFFFE));

    uberInsert.addUInt32(GetHealth());

    for (uint32 i = 0; i < MAX_POWERS; ++i)
        uberInsert.addUInt32(GetPower(Powers(i)));

    for (uint32 i = 0; i < PLAYER_EXPLORED_ZONES_SIZE; ++i) // string
    {
        ss << GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + i) << " ";
    }
    uberInsert.addString(ss);

    for (uint32 i = 0; i < EQUIPMENT_SLOT_END; ++i)         // string: item id, ench (perm/temp)
    {
        ss << GetUInt32Value(PLAYER_VISIBLE_ITEM_1_0 + i * MAX_VISIBLE_ITEM_OFFSET) << " ";

        uint32 ench1 = GetUInt32Value(PLAYER_VISIBLE_ITEM_1_0 + i * MAX_VISIBLE_ITEM_OFFSET + 1 + PERM_ENCHANTMENT_SLOT);
        uint32 ench2 = GetUInt32Value(PLAYER_VISIBLE_ITEM_1_0 + i * MAX_VISIBLE_ITEM_OFFSET + 1 + TEMP_ENCHANTMENT_SLOT);
        ss << uint32(MAKE_PAIR32(ench1, ench2)) << " ";
    }
    uberInsert.addString(ss);

    uberInsert.addUInt32(GetUInt32Value(PLAYER_AMMO_ID));

    uberInsert.addUInt32(uint32(GetByteValue(PLAYER_FIELD_BYTES, 2)));

    uberInsert.Execute();

    if (m_mailsUpdated)                                     // save mails only when needed
        _SaveMail();

    _SaveBGData();
    _SaveInventory();
    _SaveQuestStatus();
    _SaveSpells();
    _SaveSpellCooldowns();
    _SaveActions();
    _SaveAuras();
    _SaveSkills();
    m_reputationMgr.SaveToDB();
    _SaveHonorCP();
    GetSession()->SaveTutorialsData();                      // changed only while character in game

    CharacterDatabase.CommitTransaction();

    // check if stats should only be saved on logout
    // save stats can be out of transaction
    if (m_session->isLogingOut() || !sWorld.getConfig(CONFIG_BOOL_STATS_SAVE_ONLY_ON_LOGOUT))
        _SaveStats();

    // save pet (hunter pet level and experience and all type pets health/mana).
    if (Pet* pet = GetPet())
        pet->SavePetToDB(PET_SAVE_AS_CURRENT);
}

// fast save function for item/money cheating preventing - save only inventory and money state
void Player::SaveInventoryAndGoldToDB()
{
    _SaveInventory();
    SaveGoldToDB();
}

void Player::SaveGoldToDB()
{
    static SqlStatementID updateGold ;

    SqlStatement stmt = CharacterDatabase.CreateStatement(updateGold, "UPDATE characters SET money = ? WHERE guid = ?");
    stmt.PExecute(GetMoney(), GetGUIDLow());
}

void Player::_SaveActions()
{
    static SqlStatementID insertAction ;
    static SqlStatementID updateAction ;
    static SqlStatementID deleteAction ;

    for (ActionButtonList::iterator itr = m_actionButtons.begin(); itr != m_actionButtons.end();)
    {
        switch (itr->second.uState)
        {
            case ACTIONBUTTON_NEW:
            {
                SqlStatement stmt = CharacterDatabase.CreateStatement(insertAction, "INSERT INTO character_action (guid,button,action,type) VALUES (?, ?, ?, ?)");
                stmt.addUInt32(GetGUIDLow());
                stmt.addUInt32(uint32(itr->first));
                stmt.addUInt32(itr->second.GetAction());
                stmt.addUInt32(uint32(itr->second.GetType()));
                stmt.Execute();
                itr->second.uState = ACTIONBUTTON_UNCHANGED;
                ++itr;
            }
            break;
            case ACTIONBUTTON_CHANGED:
            {
                SqlStatement stmt = CharacterDatabase.CreateStatement(updateAction, "UPDATE character_action  SET action = ?, type = ? WHERE guid = ? AND button = ?");
                stmt.addUInt32(itr->second.GetAction());
                stmt.addUInt32(uint32(itr->second.GetType()));
                stmt.addUInt32(GetGUIDLow());
                stmt.addUInt32(uint32(itr->first));
                stmt.Execute();
                itr->second.uState = ACTIONBUTTON_UNCHANGED;
                ++itr;
            }
            break;
            case ACTIONBUTTON_DELETED:
            {
                SqlStatement stmt = CharacterDatabase.CreateStatement(deleteAction, "DELETE FROM character_action WHERE guid = ? AND button = ?");
                stmt.addUInt32(GetGUIDLow());
                stmt.addUInt32(uint32(itr->first));
                stmt.Execute();
                m_actionButtons.erase(itr++);
            }
            break;
            default:
                ++itr;
                break;
        }
    }
}

void Player::_SaveAuras()
{
    static SqlStatementID deleteAuras ;
    static SqlStatementID insertAuras ;

    SqlStatement stmt = CharacterDatabase.CreateStatement(deleteAuras, "DELETE FROM character_aura WHERE guid = ?");
    stmt.PExecute(GetGUIDLow());

    SpellAuraHolderMap const& auraHolders = GetSpellAuraHolderMap();

    if (auraHolders.empty())
        return;

    stmt = CharacterDatabase.CreateStatement(insertAuras, "INSERT INTO character_aura (guid, caster_guid, item_guid, spell, stackcount, remaincharges, "
            "basepoints0, basepoints1, basepoints2, periodictime0, periodictime1, periodictime2, maxduration, remaintime, effIndexMask) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    for (SpellAuraHolderMap::const_iterator itr = auraHolders.begin(); itr != auraHolders.end(); ++itr)
    {
        SpellAuraHolder* holder = itr->second;
        // skip all holders from spells that are passive or channeled
        // save singleTarget auras if self cast.
        bool selfCastHolder = holder->GetCasterGuid() == GetObjectGuid();
        TrackedAuraType trackedType = holder->GetTrackedAuraType();
        if (!holder->IsPassive() && !IsChanneledSpell(holder->GetSpellProto()) &&
                (trackedType == TRACK_AURA_TYPE_NOT_TRACKED || (trackedType == TRACK_AURA_TYPE_SINGLE_TARGET && selfCastHolder)))
        {
            int32  damage[MAX_EFFECT_INDEX];
            uint32 periodicTime[MAX_EFFECT_INDEX];
            uint32 effIndexMask = 0;

            for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
            {
                damage[i] = 0;
                periodicTime[i] = 0;

                if (Aura* aur = holder->GetAuraByEffectIndex(SpellEffectIndex(i)))
                {
                    // don't save not own area auras
                    if (aur->IsAreaAura() && holder->GetCasterGuid() != GetObjectGuid())
                        continue;

                    damage[i] = aur->GetModifier()->m_amount;
                    periodicTime[i] = aur->GetModifier()->periodictime;
                    effIndexMask |= (1 << i);
                }
            }

            if (!effIndexMask)
                continue;

            stmt.addUInt32(GetGUIDLow());
            stmt.addUInt64(holder->GetCasterGuid().GetRawValue());
            stmt.addUInt32(holder->GetCastItemGuid().GetCounter());
            stmt.addUInt32(holder->GetId());
            stmt.addUInt32(holder->GetStackAmount());
            stmt.addUInt8(holder->GetAuraCharges());

            for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
                stmt.addInt32(damage[i]);

            for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
                stmt.addUInt32(periodicTime[i]);

            stmt.addInt32(holder->GetAuraMaxDuration());
            stmt.addInt32(holder->GetAuraDuration());
            stmt.addUInt32(effIndexMask);
            stmt.Execute();
        }
    }
}

void Player::_SaveInventory()
{
    // force items in buyback slots to new state
    // and remove those that aren't already
    for (uint8 i = BUYBACK_SLOT_START; i < BUYBACK_SLOT_END; ++i)
    {
        Item* item = m_items[i];
        if (!item || item->GetState() == ITEM_NEW) continue;

        static SqlStatementID delInv ;
        static SqlStatementID delItemInst ;

        SqlStatement stmt = CharacterDatabase.CreateStatement(delInv, "DELETE FROM character_inventory WHERE item = ?");
        stmt.PExecute(item->GetGUIDLow());

        stmt = CharacterDatabase.CreateStatement(delItemInst, "DELETE FROM item_instance WHERE guid = ?");
        stmt.PExecute(item->GetGUIDLow());

        m_items[i]->FSetState(ITEM_NEW);
    }

    // update enchantment durations
    for (EnchantDurationList::const_iterator itr = m_enchantDuration.begin(); itr != m_enchantDuration.end(); ++itr)
    {
        itr->item->SetEnchantmentDuration(itr->slot, itr->leftduration);
    }

    // if no changes
    if (m_itemUpdateQueue.empty()) return;

    // do not save if the update queue is corrupt
    bool error = false;
    for (size_t i = 0; i < m_itemUpdateQueue.size(); ++i)
    {
        Item* item = m_itemUpdateQueue[i];
        if (!item || item->GetState() == ITEM_REMOVED) continue;
        Item* test = GetItemByPos(item->GetBagSlot(), item->GetSlot());

        if (test == NULL)
        {
            sLog.outError("Player(GUID: %u Name: %s)::_SaveInventory - the bag(%d) and slot(%d) values for the item with guid %d are incorrect, the player doesn't have an item at that position!", GetGUIDLow(), GetName(), item->GetBagSlot(), item->GetSlot(), item->GetGUIDLow());
            error = true;
        }
        else if (test != item)
        {
            sLog.outError("Player(GUID: %u Name: %s)::_SaveInventory - the bag(%d) and slot(%d) values for the item with guid %d are incorrect, the item with guid %d is there instead!", GetGUIDLow(), GetName(), item->GetBagSlot(), item->GetSlot(), item->GetGUIDLow(), test->GetGUIDLow());
            error = true;
        }
    }

    if (error)
    {
        sLog.outError("Player::_SaveInventory - one or more errors occurred save aborted!");
        ChatHandler(this).SendSysMessage(LANG_ITEM_SAVE_FAILED);
        return;
    }

    static SqlStatementID insertInventory ;
    static SqlStatementID updateInventory ;
    static SqlStatementID deleteInventory ;

    for (size_t i = 0; i < m_itemUpdateQueue.size(); ++i)
    {
        Item* item = m_itemUpdateQueue[i];
        if (!item) continue;

        Bag* container = item->GetContainer();
        uint32 bag_guid = container ? container->GetGUIDLow() : 0;

        switch (item->GetState())
        {
            case ITEM_NEW:
            {
                SqlStatement stmt = CharacterDatabase.CreateStatement(insertInventory, "INSERT INTO character_inventory (guid,bag,slot,item,item_template) VALUES (?, ?, ?, ?, ?)");
                stmt.addUInt32(GetGUIDLow());
                stmt.addUInt32(bag_guid);
                stmt.addUInt8(item->GetSlot());
                stmt.addUInt32(item->GetGUIDLow());
                stmt.addUInt32(item->GetEntry());
                stmt.Execute();
            }
            break;
            case ITEM_CHANGED:
            {
                SqlStatement stmt = CharacterDatabase.CreateStatement(updateInventory, "UPDATE character_inventory SET guid = ?, bag = ?, slot = ?, item_template = ? WHERE item = ?");
                stmt.addUInt32(GetGUIDLow());
                stmt.addUInt32(bag_guid);
                stmt.addUInt8(item->GetSlot());
                stmt.addUInt32(item->GetEntry());
                stmt.addUInt32(item->GetGUIDLow());
                stmt.Execute();
            }
            break;
            case ITEM_REMOVED:
            {
                SqlStatement stmt = CharacterDatabase.CreateStatement(deleteInventory, "DELETE FROM character_inventory WHERE item = ?");
                stmt.PExecute(item->GetGUIDLow());
            }
            break;
            case ITEM_UNCHANGED:
                break;
        }

        item->SaveToDB();                                   // item have unchanged inventory record and can be save standalone
    }
    m_itemUpdateQueue.clear();
}

void Player::_SaveHonorCP()
{
    HonorCPMap tempList;

    for (HonorCPMap::iterator itr = m_honorCP.begin(); itr != m_honorCP.end() ; ++itr)
    {
        switch (itr->state)
        {
            case HK_OLD:
                // pending kills will be stored in characters table
                itr->state = HK_DELETED;
                break;
            case HK_NEW:
                CharacterDatabase.PExecute("INSERT INTO character_honor_cp (guid,victim_type,victim,honor,date,type) "
                                           " VALUES (%u,%u,%u,%f,%u,%u)", GetGUIDLow(), itr->victimType, itr->victimID, itr->honorPoints , itr->date, itr->type);
                itr->state = HK_UNCHANGED;
                tempList.push_back(*itr);
                break;
            case HK_UNCHANGED:
                tempList.push_back(*itr);
                break;
            default: // deleted case
                break;
        }
    }

    m_honorCP.clear();
    m_honorCP = tempList;
    tempList.clear();
}

void Player::_SaveMail()
{
    static SqlStatementID updateMail ;
    static SqlStatementID deleteMailItems ;

    static SqlStatementID deleteItem ;
    static SqlStatementID deleteItemText;
    static SqlStatementID deleteMain ;
    static SqlStatementID deleteItems ;

    for (PlayerMails::iterator itr = m_mail.begin(); itr != m_mail.end(); ++itr)
    {
        Mail* m = (*itr);
        if (m->state == MAIL_STATE_CHANGED)
        {
            SqlStatement stmt = CharacterDatabase.CreateStatement(updateMail, "UPDATE mail SET itemTextId = ?,has_items = ?, expire_time = ?, deliver_time = ?, money = ?, cod = ?, checked = ? WHERE id = ?");
            stmt.addUInt32(m->itemTextId);
            stmt.addUInt32(m->HasItems() ? 1 : 0);
            stmt.addUInt64(uint64(m->expire_time));
            stmt.addUInt64(uint64(m->deliver_time));
            stmt.addUInt32(m->money);
            stmt.addUInt32(m->COD);
            stmt.addUInt32(m->checked);
            stmt.addUInt32(m->messageID);
            stmt.Execute();

            if (!m->removedItems.empty())
            {
                stmt = CharacterDatabase.CreateStatement(deleteMailItems, "DELETE FROM mail_items WHERE item_guid = ?");

                for (std::vector<uint32>::const_iterator itr2 = m->removedItems.begin(); itr2 != m->removedItems.end(); ++itr2)
                    stmt.PExecute(*itr2);

                m->removedItems.clear();
            }
            m->state = MAIL_STATE_UNCHANGED;
        }
        else if (m->state == MAIL_STATE_DELETED)
        {
            if (m->HasItems())
            {
                SqlStatement stmt = CharacterDatabase.CreateStatement(deleteItem, "DELETE FROM item_instance WHERE guid = ?");
                for (MailItemInfoVec::const_iterator itr2 = m->items.begin(); itr2 != m->items.end(); ++itr2)
                    stmt.PExecute(itr2->item_guid);
            }

            if (m->itemTextId)
            {
                SqlStatement stmt = CharacterDatabase.CreateStatement(deleteItemText, "DELETE FROM item_text WHERE id = ?");
                stmt.PExecute(m->itemTextId);
            }

            SqlStatement stmt = CharacterDatabase.CreateStatement(deleteMain, "DELETE FROM mail WHERE id = ?");
            stmt.PExecute(m->messageID);

            stmt = CharacterDatabase.CreateStatement(deleteItems, "DELETE FROM mail_items WHERE mail_id = ?");
            stmt.PExecute(m->messageID);
        }
    }

    // deallocate deleted mails...
    for (PlayerMails::iterator itr = m_mail.begin(); itr != m_mail.end();)
    {
        if ((*itr)->state == MAIL_STATE_DELETED)
        {
            Mail* m = *itr;
            m_mail.erase(itr);
            delete m;
            itr = m_mail.begin();
        }
        else
            ++itr;
    }

    m_mailsUpdated = false;
}

void Player::_SaveQuestStatus()
{
    static SqlStatementID insertQuestStatus ;

    static SqlStatementID updateQuestStatus ;

    // we don't need transactions here.
    for (QuestStatusMap::iterator i = mQuestStatus.begin(); i != mQuestStatus.end(); ++i)
    {
        switch (i->second.uState)
        {
            case QUEST_NEW :
            {
                SqlStatement stmt = CharacterDatabase.CreateStatement(insertQuestStatus, "INSERT INTO character_queststatus (guid,quest,status,rewarded,explored,timer,mobcount1,mobcount2,mobcount3,mobcount4,itemcount1,itemcount2,itemcount3,itemcount4) "
                                    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

                stmt.addUInt32(GetGUIDLow());
                stmt.addUInt32(i->first);
                stmt.addUInt8(i->second.m_status);
                stmt.addUInt8(i->second.m_rewarded);
                stmt.addUInt8(i->second.m_explored);
                stmt.addUInt64(uint64(i->second.m_timer / IN_MILLISECONDS + sWorld.GetGameTime()));
                for (int k = 0; k < QUEST_OBJECTIVES_COUNT; ++k)
                    stmt.addUInt32(i->second.m_creatureOrGOcount[k]);
                for (int k = 0; k < QUEST_OBJECTIVES_COUNT; ++k)
                    stmt.addUInt32(i->second.m_itemcount[k]);
                stmt.Execute();
            }
            break;
            case QUEST_CHANGED :
            {
                SqlStatement stmt = CharacterDatabase.CreateStatement(updateQuestStatus, "UPDATE character_queststatus SET status = ?,rewarded = ?,explored = ?,timer = ?,"
                                    "mobcount1 = ?,mobcount2 = ?,mobcount3 = ?,mobcount4 = ?,itemcount1 = ?,itemcount2 = ?,itemcount3 = ?,itemcount4 = ?  WHERE guid = ? AND quest = ?");

                stmt.addUInt8(i->second.m_status);
                stmt.addUInt8(i->second.m_rewarded);
                stmt.addUInt8(i->second.m_explored);
                stmt.addUInt64(uint64(i->second.m_timer / IN_MILLISECONDS + sWorld.GetGameTime()));
                for (int k = 0; k < QUEST_OBJECTIVES_COUNT; ++k)
                    stmt.addUInt32(i->second.m_creatureOrGOcount[k]);
                for (int k = 0; k < QUEST_OBJECTIVES_COUNT; ++k)
                    stmt.addUInt32(i->second.m_itemcount[k]);
                stmt.addUInt32(GetGUIDLow());
                stmt.addUInt32(i->first);
                stmt.Execute();
            }
            break;
            case QUEST_UNCHANGED:
                break;
        };
        i->second.uState = QUEST_UNCHANGED;
    }
}

void Player::_SaveSkills()
{
    static SqlStatementID delSkills ;
    static SqlStatementID insSkills ;
    static SqlStatementID updSkills ;

    // we don't need transactions here.
    for (SkillStatusMap::iterator itr = mSkillStatus.begin(); itr != mSkillStatus.end();)
    {
        if (itr->second.uState == SKILL_UNCHANGED)
        {
            ++itr;
            continue;
        }

        if (itr->second.uState == SKILL_DELETED)
        {
            SqlStatement stmt = CharacterDatabase.CreateStatement(delSkills, "DELETE FROM character_skills WHERE guid = ? AND skill = ?");
            stmt.PExecute(GetGUIDLow(), itr->first);
            mSkillStatus.erase(itr++);
            continue;
        }

        uint32 valueData = GetUInt32Value(PLAYER_SKILL_VALUE_INDEX(itr->second.pos));
        uint16 value = SKILL_VALUE(valueData);
        uint16 max = SKILL_MAX(valueData);

        switch (itr->second.uState)
        {
            case SKILL_NEW:
            {
                SqlStatement stmt = CharacterDatabase.CreateStatement(insSkills, "INSERT INTO character_skills (guid, skill, value, max) VALUES (?, ?, ?, ?)");
                stmt.PExecute(GetGUIDLow(), itr->first, value, max);
            }
            break;
            case SKILL_CHANGED:
            {
                SqlStatement stmt = CharacterDatabase.CreateStatement(updSkills, "UPDATE character_skills SET value = ?, max = ? WHERE guid = ? AND skill = ?");
                stmt.PExecute(value, max, GetGUIDLow(), itr->first);
            }
            break;
            case SKILL_UNCHANGED:
            case SKILL_DELETED:
                MANGOS_ASSERT(false);
                break;
        };
        itr->second.uState = SKILL_UNCHANGED;

        ++itr;
    }
}

void Player::_SaveSpells()
{
    static SqlStatementID delSpells ;
    static SqlStatementID insSpells ;

    SqlStatement stmtDel = CharacterDatabase.CreateStatement(delSpells, "DELETE FROM character_spell WHERE guid = ? and spell = ?");
    SqlStatement stmtIns = CharacterDatabase.CreateStatement(insSpells, "INSERT INTO character_spell (guid,spell,active,disabled) VALUES (?, ?, ?, ?)");

    for (PlayerSpellMap::iterator itr = m_spells.begin(), next = m_spells.begin(); itr != m_spells.end();)
    {
        if (itr->second.state == PLAYERSPELL_REMOVED || itr->second.state == PLAYERSPELL_CHANGED)
            stmtDel.PExecute(GetGUIDLow(), itr->first);

        // add only changed/new not dependent spells
        if (!itr->second.dependent && (itr->second.state == PLAYERSPELL_NEW || itr->second.state == PLAYERSPELL_CHANGED))
            stmtIns.PExecute(GetGUIDLow(), itr->first, uint8(itr->second.active ? 1 : 0), uint8(itr->second.disabled ? 1 : 0));

        if (itr->second.state == PLAYERSPELL_REMOVED)
            m_spells.erase(itr++);
        else
        {
            itr->second.state = PLAYERSPELL_UNCHANGED;
            ++itr;
        }
    }
}

// save player stats -- only for external usage
// real stats will be recalculated on player login
void Player::_SaveStats()
{
    // check if stat saving is enabled and if char level is high enough
    if (!sWorld.getConfig(CONFIG_UINT32_MIN_LEVEL_STAT_SAVE) || getLevel() < sWorld.getConfig(CONFIG_UINT32_MIN_LEVEL_STAT_SAVE))
        return;

    static SqlStatementID delStats ;
    static SqlStatementID insertStats ;

    SqlStatement stmt = CharacterDatabase.CreateStatement(delStats, "DELETE FROM character_stats WHERE guid = ?");
    stmt.PExecute(GetGUIDLow());

    stmt = CharacterDatabase.CreateStatement(insertStats, "INSERT INTO character_stats (guid, maxhealth, maxpower1, maxpower2, maxpower3, maxpower4, maxpower5, "
            "strength, agility, stamina, intellect, spirit, armor, resHoly, resFire, resNature, resFrost, resShadow, resArcane, "
            "blockPct, dodgePct, parryPct, critPct, rangedCritPct, attackPower, rangedAttackPower) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    stmt.addUInt32(GetGUIDLow());
    stmt.addUInt32(GetMaxHealth());
    for (int i = 0; i < MAX_POWERS; ++i)
        stmt.addUInt32(GetMaxPower(Powers(i)));
    for (int i = 0; i < MAX_STATS; ++i)
        stmt.addFloat(GetStat(Stats(i)));
    // armor + school resistances
    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        stmt.addUInt32(GetResistance(SpellSchools(i)));
    stmt.addFloat(GetFloatValue(PLAYER_BLOCK_PERCENTAGE));
    stmt.addFloat(GetFloatValue(PLAYER_DODGE_PERCENTAGE));
    stmt.addFloat(GetFloatValue(PLAYER_PARRY_PERCENTAGE));
    stmt.addFloat(GetFloatValue(PLAYER_CRIT_PERCENTAGE));
    stmt.addFloat(GetFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE));
    stmt.addUInt32(GetUInt32Value(UNIT_FIELD_ATTACK_POWER));
    stmt.addUInt32(GetUInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER));

    stmt.Execute();
}

void Player::outDebugStatsValues() const
{
    // optimize disabled debug output
    if (!sLog.HasLogLevelOrHigher(LOG_LVL_DEBUG) || sLog.HasLogFilter(LOG_FILTER_PLAYER_STATS))
        return;

    sLog.outDebug("HP is: \t\t\t%u\t\tMP is: \t\t\t%u", GetMaxHealth(), GetMaxPower(POWER_MANA));
    sLog.outDebug("AGILITY is: \t\t%f\t\tSTRENGTH is: \t\t%f", GetStat(STAT_AGILITY), GetStat(STAT_STRENGTH));
    sLog.outDebug("INTELLECT is: \t\t%f\t\tSPIRIT is: \t\t%f", GetStat(STAT_INTELLECT), GetStat(STAT_SPIRIT));
    sLog.outDebug("STAMINA is: \t\t%f", GetStat(STAT_STAMINA));
    sLog.outDebug("Armor is: \t\t%u\t\tBlock is: \t\t%f", GetArmor(), GetFloatValue(PLAYER_BLOCK_PERCENTAGE));
    sLog.outDebug("HolyRes is: \t\t%u\t\tFireRes is: \t\t%u", GetResistance(SPELL_SCHOOL_HOLY), GetResistance(SPELL_SCHOOL_FIRE));
    sLog.outDebug("NatureRes is: \t\t%u\t\tFrostRes is: \t\t%u", GetResistance(SPELL_SCHOOL_NATURE), GetResistance(SPELL_SCHOOL_FROST));
    sLog.outDebug("ShadowRes is: \t\t%u\t\tArcaneRes is: \t\t%u", GetResistance(SPELL_SCHOOL_SHADOW), GetResistance(SPELL_SCHOOL_ARCANE));
    sLog.outDebug("MIN_DAMAGE is: \t\t%f\tMAX_DAMAGE is: \t\t%f", GetFloatValue(UNIT_FIELD_MINDAMAGE), GetFloatValue(UNIT_FIELD_MAXDAMAGE));
    sLog.outDebug("MIN_OFFHAND_DAMAGE is: \t%f\tMAX_OFFHAND_DAMAGE is: \t%f", GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE), GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE));
    sLog.outDebug("MIN_RANGED_DAMAGE is: \t%f\tMAX_RANGED_DAMAGE is: \t%f", GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE), GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE));
    sLog.outDebug("ATTACK_TIME is: \t%u\t\tRANGE_ATTACK_TIME is: \t%u", GetAttackTime(BASE_ATTACK), GetAttackTime(RANGED_ATTACK));
}

/*********************************************************/
/***               FLOOD FILTER SYSTEM                 ***/
/*********************************************************/

void Player::UpdateSpeakTime()
{
    // ignore chat spam protection for GMs in any mode
    if (GetSession()->GetSecurity() > SEC_PLAYER)
        return;

    time_t current = time(NULL);
    if (m_speakTime > current)
    {
        uint32 max_count = sWorld.getConfig(CONFIG_UINT32_CHATFLOOD_MESSAGE_COUNT);
        if (!max_count)
            return;

        ++m_speakCount;
        if (m_speakCount >= max_count)
        {
            // prevent overwrite mute time, if message send just before mutes set, for example.
            time_t new_mute = current + sWorld.getConfig(CONFIG_UINT32_CHATFLOOD_MUTE_TIME);
            if (GetSession()->m_muteTime < new_mute)
                GetSession()->m_muteTime = new_mute;

            m_speakCount = 0;
        }
    }
    else
        m_speakCount = 0;

    m_speakTime = current + sWorld.getConfig(CONFIG_UINT32_CHATFLOOD_MESSAGE_DELAY);
}

bool Player::CanSpeak() const
{
    return  GetSession()->m_muteTime <= time(NULL);
}

/*********************************************************/
/***              LOW LEVEL FUNCTIONS:Notifiers        ***/
/*********************************************************/

void Player::SendAttackSwingNotInRange()
{
    WorldPacket data(SMSG_ATTACKSWING_NOTINRANGE, 0);
    GetSession()->SendPacket(&data);
}

void Player::SavePositionInDB(ObjectGuid guid, uint32 mapid, float x, float y, float z, float o, uint32 zone)
{
    std::ostringstream ss;
    ss << "UPDATE characters SET position_x='" << x << "',position_y='" << y
       << "',position_z='" << z << "',orientation='" << o << "',map='" << mapid
       << "',zone='" << zone << "',trans_x='0',trans_y='0',trans_z='0',"
       << "transguid='0',taxi_path='' WHERE guid='" << guid.GetCounter() << "'";
    DEBUG_LOG("%s", ss.str().c_str());
    CharacterDatabase.Execute(ss.str().c_str());
}

void Player::SetUInt32ValueInArray(Tokens& tokens, uint16 index, uint32 value)
{
    char buf[11];
    snprintf(buf, 11, "%u", value);

    if (index >= tokens.size())
        return;

    tokens[index] = buf;
}

void Player::SendAttackSwingNotStanding()
{
    WorldPacket data(SMSG_ATTACKSWING_NOTSTANDING, 0);
    GetSession()->SendPacket(&data);
}

void Player::SendAttackSwingDeadTarget()
{
    WorldPacket data(SMSG_ATTACKSWING_DEADTARGET, 0);
    GetSession()->SendPacket(&data);
}

void Player::SendAttackSwingCantAttack()
{
    WorldPacket data(SMSG_ATTACKSWING_CANT_ATTACK, 0);
    GetSession()->SendPacket(&data);
}

void Player::SendAttackSwingCancelAttack()
{
    WorldPacket data(SMSG_CANCEL_COMBAT, 0);
    GetSession()->SendPacket(&data);
}

void Player::SendAttackSwingBadFacingAttack()
{
    WorldPacket data(SMSG_ATTACKSWING_BADFACING, 0);
    GetSession()->SendPacket(&data);
}

void Player::SendAutoRepeatCancel()
{
    WorldPacket data(SMSG_CANCEL_AUTO_REPEAT, 0);
    GetSession()->SendPacket(&data);
}

void Player::SendExplorationExperience(uint32 Area, uint32 Experience)
{
    WorldPacket data(SMSG_EXPLORATION_EXPERIENCE, 8);
    data << uint32(Area);
    data << uint32(Experience);
    GetSession()->SendPacket(&data);
}

void Player::SendResetFailedNotify(uint32 mapid)
{
    WorldPacket data(SMSG_RESET_FAILED_NOTIFY, 4);
    data << uint32(mapid);
    GetSession()->SendPacket(&data);
}

/// Reset all solo instances and optionally send a message on success for each
void Player::ResetInstances(InstanceResetMethod method)
{
    // method can be INSTANCE_RESET_ALL, INSTANCE_RESET_GROUP_JOIN


    for (BoundInstancesMap::iterator itr = m_boundInstances.begin(); itr != m_boundInstances.end();)
    {
        DungeonPersistentState* state = itr->second.state;
        const MapEntry* entry = sMapStore.LookupEntry(itr->first);
        if (!entry || !state->CanReset())
        {
            ++itr;
            continue;
        }

        if (method == INSTANCE_RESET_ALL)
        {
            // the "reset all instances" method can only reset normal maps
            if (entry->map_type == MAP_RAID)
            {
                ++itr;
                continue;
            }
        }

        // if the map is loaded, reset it
        if (Map* map = sMapMgr.FindMap(state->GetMapId(), state->GetInstanceId()))
            if (map->IsDungeon())
                ((DungeonMap*)map)->Reset(method);

        // since this is a solo instance there should not be any players inside
        if (method == INSTANCE_RESET_ALL)
            SendResetInstanceSuccess(state->GetMapId());

        state->DeleteFromDB();
        m_boundInstances.erase(itr++);

        // the following should remove the instance save from the manager and delete it as well
        state->RemovePlayer(this);
    }
}

void Player::SendResetInstanceSuccess(uint32 MapId)
{
    WorldPacket data(SMSG_INSTANCE_RESET, 4);
    data << uint32(MapId);
    GetSession()->SendPacket(&data);
}

void Player::SendResetInstanceFailed(uint32 reason, uint32 MapId)
{
    // TODO: find what other fail reasons there are besides players in the instance
    WorldPacket data(SMSG_INSTANCE_RESET_FAILED, 4);
    data << uint32(reason);
    data << uint32(MapId);
    GetSession()->SendPacket(&data);
}

/*********************************************************/
/***              Update timers                        ***/
/*********************************************************/

void Player::UpdateContestedPvP(uint32 diff)
{
    if (!m_contestedPvPTimer || isInCombat())
        return;
    if (m_contestedPvPTimer <= diff)
    {
        ResetContestedPvP();
    }
    else
        m_contestedPvPTimer -= diff;
}

void Player::UpdatePvPFlag(time_t currTime)
{
    if (!IsPvP())
        return;
    if (pvpInfo.endTimer == 0 || currTime < (pvpInfo.endTimer + 300))
        return;

    UpdatePvP(false);
}

void Player::SetFFAPvP(bool state)
{
    if (state)
        SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP);
    else
        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP);
}

void Player::UpdateDuelFlag(time_t currTime)
{
    if (!duel || duel->startTimer == 0 || currTime < duel->startTimer + 3)
        return;

    // used by eluna
    sHookMgr->OnDuelStart(this, duel->opponent);

    SetUInt32Value(PLAYER_DUEL_TEAM, 1);
    duel->opponent->SetUInt32Value(PLAYER_DUEL_TEAM, 2);

    duel->startTimer = 0;
    duel->startTime  = currTime;
    duel->opponent->duel->startTimer = 0;
    duel->opponent->duel->startTime  = currTime;
}

void Player::RemovePet(PetSaveMode mode)
{
    if (Pet* pet = GetPet())
        pet->Unsummon(mode, this);
}

void Player::RemoveMiniPet()
{
    if (Pet* pet = GetMiniPet())
        pet->Unsummon(PET_SAVE_AS_DELETED);
}

Pet* Player::GetMiniPet() const
{
    if (m_miniPetGuid.IsEmpty())
        return NULL;

    return GetMap()->GetPet(m_miniPetGuid);
}

void Player::Say(const std::string& text, const uint32 language)
{
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_SAY, text.c_str(), Language(language), GetChatTag(), GetObjectGuid(), GetName());
    SendMessageToSetInRange(&data, sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_SAY), true);
}

void Player::Yell(const std::string& text, const uint32 language)
{
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_YELL, text.c_str(), Language(language), GetChatTag(), GetObjectGuid(), GetName());
    SendMessageToSetInRange(&data, sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_YELL), true);
}

void Player::TextEmote(const std::string& text)
{
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_EMOTE, text.c_str(), LANG_UNIVERSAL, GetChatTag(), GetObjectGuid(), GetName());
    SendMessageToSetInRange(&data, sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_TEXTEMOTE), true, !sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_CHAT));
}

void Player::Whisper(const std::string& text, uint32 language, ObjectGuid receiver)
{
    if (language != LANG_ADDON)                             // if not addon data
        language = LANG_UNIVERSAL;                          // whispers should always be readable

    Player* rPlayer = sObjectMgr.GetPlayer(receiver);

    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_WHISPER, text.c_str(), Language(language), GetChatTag(), GetObjectGuid(), GetName());
    rPlayer->GetSession()->SendPacket(&data);

    // not send confirmation for addon messages
    if (language != LANG_ADDON)
    {
        data.clear();
        ChatHandler::BuildChatPacket(data, CHAT_MSG_WHISPER_INFORM, text.c_str(), Language(language), CHAT_TAG_NONE, rPlayer->GetObjectGuid());
        GetSession()->SendPacket(&data);
    }

    if (!isAcceptWhispers())
    {
        SetAcceptWhispers(true);
        ChatHandler(this).SendSysMessage(LANG_COMMAND_WHISPERON);
    }

    // announce afk or dnd message
    if (rPlayer->isAFK())
        ChatHandler(this).PSendSysMessage(LANG_PLAYER_AFK, rPlayer->GetName(), rPlayer->autoReplyMsg.c_str());
    else if (rPlayer->isDND())
        ChatHandler(this).PSendSysMessage(LANG_PLAYER_DND, rPlayer->GetName(), rPlayer->autoReplyMsg.c_str());
}

void Player::PetSpellInitialize()
{
    Pet* pet = GetPet();

    if (!pet)
        return;

    DEBUG_LOG("Pet Spells Groups");

    CharmInfo* charmInfo = pet->GetCharmInfo();

    WorldPacket data(SMSG_PET_SPELLS, 8 + 4 + 1 + 1 + 2 + 4 * MAX_UNIT_ACTION_BAR_INDEX + 1 + 1);
    data << pet->GetObjectGuid();
    data << uint32(0);
    data << uint8(charmInfo->GetReactState()) << uint8(charmInfo->GetCommandState()) << uint16(0);

    // action bar loop
    charmInfo->BuildActionBar(&data);

    size_t spellsCountPos = data.wpos();

    // spells count
    uint8 addlist = 0;
    data << uint8(addlist);                                 // placeholder

    if (pet->IsPermanentPetFor(this))
    {
        // spells loop
        for (PetSpellMap::const_iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end(); ++itr)
        {
            if (itr->second.state == PETSPELL_REMOVED)
                continue;

            data << uint32(MAKE_UNIT_ACTION_BUTTON(itr->first, itr->second.active));
            ++addlist;
        }
    }

    data.put<uint8>(spellsCountPos, addlist);

    uint8 cooldownsCount = pet->m_CreatureSpellCooldowns.size() + pet->m_CreatureCategoryCooldowns.size();
    data << uint8(cooldownsCount);

    time_t curTime = time(NULL);

    for (CreatureSpellCooldowns::const_iterator itr = pet->m_CreatureSpellCooldowns.begin(); itr != pet->m_CreatureSpellCooldowns.end(); ++itr)
    {
        time_t cooldown = (itr->second > curTime) ? (itr->second - curTime) * IN_MILLISECONDS : 0;

        data << uint16(itr->first);                         // spellid
        data << uint16(0);                                  // spell category?
        data << uint32(cooldown);                           // cooldown
        data << uint32(0);                                  // category cooldown
    }

    for (CreatureSpellCooldowns::const_iterator itr = pet->m_CreatureCategoryCooldowns.begin(); itr != pet->m_CreatureCategoryCooldowns.end(); ++itr)
    {
        time_t cooldown = (itr->second > curTime) ? (itr->second - curTime) * IN_MILLISECONDS : 0;

        data << uint16(itr->first);                         // spellid
        data << uint16(0);                                  // spell category?
        data << uint32(0);                                  // cooldown
        data << uint32(cooldown);                           // category cooldown
    }

    GetSession()->SendPacket(&data);
}

void Player::PossessSpellInitialize()
{
    Unit* charm = GetCharm();

    if (!charm)
        return;

    CharmInfo* charmInfo = charm->GetCharmInfo();

    if (!charmInfo)
    {
        sLog.outError("Player::PossessSpellInitialize(): charm (GUID: %u TypeId: %u) has no charminfo!", charm->GetGUIDLow(), charm->GetTypeId());
        return;
    }

    WorldPacket data(SMSG_PET_SPELLS, 8 + 4 + 4 + 4 * MAX_UNIT_ACTION_BAR_INDEX + 1 + 1);
    data << charm->GetObjectGuid();
    data << uint32(0);
    data << uint32(0);

    charmInfo->BuildActionBar(&data);

    data << uint8(0);                                       // spells count
    data << uint8(0);                                       // cooldowns count

    GetSession()->SendPacket(&data);
}

void Player::CharmSpellInitialize()
{
    Unit* charm = GetCharm();

    if (!charm)
        return;

    CharmInfo* charmInfo = charm->GetCharmInfo();
    if (!charmInfo)
    {
        sLog.outError("Player::CharmSpellInitialize(): the player's charm (GUID: %u TypeId: %u) has no charminfo!", charm->GetGUIDLow(), charm->GetTypeId());
        return;
    }

    uint8 addlist = 0;

    if (charm->GetTypeId() != TYPEID_PLAYER)
    {
        CreatureInfo const* cinfo = ((Creature*)charm)->GetCreatureInfo();

        if (cinfo && cinfo->CreatureType == CREATURE_TYPE_DEMON && getClass() == CLASS_WARLOCK)
        {
            for (uint32 i = 0; i < CREATURE_MAX_SPELLS; ++i)
            {
                if (charmInfo->GetCharmSpell(i)->GetAction())
                    ++addlist;
            }
        }
    }

    WorldPacket data(SMSG_PET_SPELLS, 8 + 4 + 1 + 1 + 2 + 4 * MAX_UNIT_ACTION_BAR_INDEX + 1 + 4 * addlist + 1);
    data << charm->GetObjectGuid();
    data << uint32(0x00000000);

    if (charm->GetTypeId() != TYPEID_PLAYER)
        data << uint8(charmInfo->GetReactState()) << uint8(charmInfo->GetCommandState()) << uint16(0);
    else
        data << uint8(0) << uint8(0) << uint16(0);

    charmInfo->BuildActionBar(&data);

    data << uint8(addlist);

    if (addlist)
    {
        for (uint32 i = 0; i < CREATURE_MAX_SPELLS; ++i)
        {
            CharmSpellEntry* cspell = charmInfo->GetCharmSpell(i);
            if (cspell->GetAction())
                data << uint32(cspell->packedData);
        }
    }

    data << uint8(0);                                       // cooldowns count

    GetSession()->SendPacket(&data);
}

void Player::RemovePetActionBar()
{
    WorldPacket data(SMSG_PET_SPELLS, 8);
    data << ObjectGuid();
    SendDirectMessage(&data);
}

bool Player::IsAffectedBySpellmod(SpellEntry const* spellInfo, SpellModifier* mod, Spell const* spell)
{
    if (!mod || !spellInfo)
        return false;

    if (mod->charges == -1 && mod->lastAffected)            // marked as expired but locked until spell casting finish
    {
        // prevent apply to any spell except spell that trigger expire
        if (spell)
        {
            if (mod->lastAffected != spell)
                return false;
        }
        else if (mod->lastAffected != FindCurrentSpellBySpellId(spellInfo->Id))
            return false;
    }

    return mod->isAffectedOnSpell(spellInfo);
}

void Player::AddSpellMod(SpellModifier* mod, bool apply)
{
    uint16 opcode = (mod->type == SPELLMOD_FLAT) ? SMSG_SET_FLAT_SPELL_MODIFIER : SMSG_SET_PCT_SPELL_MODIFIER;

    for (int eff = 0; eff < 64; ++eff)
    {
        uint64 _mask = uint64(1) << eff;
        if (mod->mask.IsFitToFamilyMask(_mask))
        {
            int32 val = 0;
            for (SpellModList::const_iterator itr = m_spellMods[mod->op].begin(); itr != m_spellMods[mod->op].end(); ++itr)
            {
                if ((*itr)->type == mod->type && ((*itr)->mask.IsFitToFamilyMask(_mask)))
                    val += (*itr)->value;
            }
            val += apply ? mod->value : -(mod->value);
            WorldPacket data(opcode, (1 + 1 + 4));
            data << uint8(eff);
            data << uint8(mod->op);
            data << int32(val);
            SendDirectMessage(&data);
        }
    }

    if (apply)
        m_spellMods[mod->op].push_back(mod);
    else
    {
        if (mod->charges == -1)
            --m_SpellModRemoveCount;
        m_spellMods[mod->op].remove(mod);
        delete mod;
    }
}

SpellModifier* Player::GetSpellMod(SpellModOp op, uint32 spellId) const
{
    for (SpellModList::const_iterator itr = m_spellMods[op].begin(); itr != m_spellMods[op].end(); ++itr)
        if ((*itr)->spellId == spellId)
            return *itr;

    return NULL;
}

void Player::RemoveSpellMods(Spell const* spell)
{
    if (!spell || (m_SpellModRemoveCount == 0))
        return;

    for (int i = 0; i < MAX_SPELLMOD; ++i)
    {
        for (SpellModList::const_iterator itr = m_spellMods[i].begin(); itr != m_spellMods[i].end();)
        {
            SpellModifier* mod = *itr;
            ++itr;

            if (mod && mod->charges == -1 && (mod->lastAffected == spell || mod->lastAffected == NULL))
            {
                RemoveAurasDueToSpell(mod->spellId);
                if (m_spellMods[i].empty())
                    break;
                else
                    itr = m_spellMods[i].begin();
            }
        }
    }
}

void Player::ResetSpellModsDueToCanceledSpell (Spell const* spell)
{
    for(int i = 0; i < MAX_SPELLMOD; ++i )
    {
        for (SpellModList::const_iterator itr = m_spellMods[i].begin(); itr != m_spellMods[i].end(); ++itr)
        {
            SpellModifier *mod = *itr;

            if (mod->lastAffected != spell)
                continue;

            mod->lastAffected = NULL;

            if (mod->charges == -1)
            {
                mod->charges = 1;
                if (m_SpellModRemoveCount > 0)
                    --m_SpellModRemoveCount;
            }
            else if (mod->charges > 0)
                ++mod->charges;
        }
    }
}

// send Proficiency
void Player::SendProficiency(ItemClass itemClass, uint32 itemSubclassMask)
{
    WorldPacket data(SMSG_SET_PROFICIENCY, 1 + 4);
    data << uint8(itemClass) << uint32(itemSubclassMask);
    GetSession()->SendPacket(&data);
}

void Player::RemovePetitionsAndSigns(ObjectGuid guid)
{
    uint32 lowguid = guid.GetCounter();

    QueryResult* result = CharacterDatabase.PQuery("SELECT ownerguid,petitionguid FROM petition_sign WHERE playerguid = '%u'", lowguid);
    if (result)
    {
        do                                                  // this part effectively does nothing, since the deletion / modification only takes place _after_ the PetitionQuery. Though I don't know if the result remains intact if I execute the delete query beforehand.
        {
            // and SendPetitionQueryOpcode reads data from the DB
            Field* fields = result->Fetch();
            ObjectGuid ownerguid   = ObjectGuid(HIGHGUID_PLAYER, fields[0].GetUInt32());
            ObjectGuid petitionguid = ObjectGuid(HIGHGUID_ITEM, fields[1].GetUInt32());

            // send update if charter owner in game
            Player* owner = sObjectMgr.GetPlayer(ownerguid);
            if (owner)
                owner->GetSession()->SendPetitionQueryOpcode(petitionguid);
        }
        while (result->NextRow());

        delete result;

        CharacterDatabase.PExecute("DELETE FROM petition_sign WHERE playerguid = '%u'", lowguid);
    }

    CharacterDatabase.BeginTransaction();
    CharacterDatabase.PExecute("DELETE FROM petition WHERE ownerguid = '%u'", lowguid);
    CharacterDatabase.PExecute("DELETE FROM petition_sign WHERE ownerguid = '%u'", lowguid);
    CharacterDatabase.CommitTransaction();
}

void Player::SetRestBonus(float rest_bonus_new)
{
    // Prevent resting on max level
    if (getLevel() >= sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
        rest_bonus_new = 0;

    if (rest_bonus_new < 0)
        rest_bonus_new = 0;

    float rest_bonus_max = (float)GetUInt32Value(PLAYER_NEXT_LEVEL_XP) * 1.5f / 2.0f;

    if (rest_bonus_new > rest_bonus_max)
        m_rest_bonus = rest_bonus_max;
    else
        m_rest_bonus = rest_bonus_new;

    // update data for client
    if (m_rest_bonus > 10)
        SetByteValue(PLAYER_BYTES_2, 3, REST_STATE_RESTED);
    else if (m_rest_bonus <= 1)
        SetByteValue(PLAYER_BYTES_2, 3, REST_STATE_NORMAL);

    // RestTickUpdate
    SetUInt32Value(PLAYER_REST_STATE_EXPERIENCE, uint32(m_rest_bonus));
}

void Player::HandleStealthedUnitsDetection()
{
    std::list<Unit*> stealthedUnits;

    MaNGOS::AnyStealthedCheck u_check(this);
    MaNGOS::UnitListSearcher<MaNGOS::AnyStealthedCheck > searcher(stealthedUnits, u_check);
    Cell::VisitAllObjects(this, searcher, MAX_PLAYER_STEALTH_DETECT_RANGE);

    WorldObject const* viewPoint = GetCamera().GetBody();

    for (std::list<Unit*>::const_iterator i = stealthedUnits.begin(); i != stealthedUnits.end(); ++i)
    {
        if ((*i) == this)
            continue;

        bool hasAtClient = HaveAtClient((*i));
        bool hasDetected = (*i)->isVisibleForOrDetect(this, viewPoint, true);

        if (hasDetected)
        {
            if (!hasAtClient)
            {
                ObjectGuid i_guid = (*i)->GetObjectGuid();
                (*i)->SendCreateUpdateToPlayer(this);
                m_clientGUIDs.insert(i_guid);

                DEBUG_FILTER_LOG(LOG_FILTER_VISIBILITY_CHANGES, "%s is detected in stealth by player %u. Distance = %f", i_guid.GetString().c_str(), GetGUIDLow(), GetDistance(*i));

                // target aura duration for caster show only if target exist at caster client
                // send data at target visibility change (adding to client)
                if ((*i) != this && (*i)->isType(TYPEMASK_UNIT))
                    SendAuraDurationsForTarget(*i);
            }
        }
        else
        {
            if (hasAtClient)
            {
                (*i)->DestroyForPlayer(this);
                m_clientGUIDs.erase((*i)->GetObjectGuid());
            }
        }
    }
}

bool Player::ActivateTaxiPathTo(std::vector<uint32> const& nodes, Creature* npc /*= NULL*/, uint32 spellid /*= 0*/)
{
    if (nodes.size() < 2)
        return false;

    // not let cheating with start flight in time of logout process || if casting not finished || while in combat || if not use Spell's with EffectSendTaxi
    if (GetSession()->isLogingOut() || isInCombat())
    {
        GetSession()->SendActivateTaxiReply(ERR_TAXIPLAYERBUSY);
        return false;
    }

    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE))
        return false;

    // taximaster case
    if (npc)
    {
        // not let cheating with start flight mounted
        if (IsMounted())
        {
            GetSession()->SendActivateTaxiReply(ERR_TAXIPLAYERALREADYMOUNTED);
            return false;
        }

        if (IsInDisallowedMountForm())
        {
            GetSession()->SendActivateTaxiReply(ERR_TAXIPLAYERSHAPESHIFTED);
            return false;
        }

        // not let cheating with start flight in time of logout process || if casting not finished || while in combat || if not use Spell's with EffectSendTaxi
        if (IsNonMeleeSpellCasted(false))
        {
            GetSession()->SendActivateTaxiReply(ERR_TAXIPLAYERBUSY);
            return false;
        }
    }
    // cast case or scripted call case
    else
    {
        RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

        if (IsInDisallowedMountForm())
            RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT);

        if (Spell* spell = GetCurrentSpell(CURRENT_GENERIC_SPELL))
            if (spell->m_spellInfo->Id != spellid)
                InterruptSpell(CURRENT_GENERIC_SPELL, false);

        InterruptSpell(CURRENT_AUTOREPEAT_SPELL, false);

        if (Spell* spell = GetCurrentSpell(CURRENT_CHANNELED_SPELL))
            if (spell->m_spellInfo->Id != spellid)
                InterruptSpell(CURRENT_CHANNELED_SPELL, true);
    }

    uint32 sourcenode = nodes[0];

    // starting node too far away (cheat?)
    TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(sourcenode);
    if (!node)
    {
        GetSession()->SendActivateTaxiReply(ERR_TAXINOSUCHPATH);
        return false;
    }

    // check node starting pos data set case if provided
    if (node->x != 0.0f || node->y != 0.0f || node->z != 0.0f)
    {
        if (node->map_id != GetMapId() ||
                (node->x - GetPositionX()) * (node->x - GetPositionX()) +
                (node->y - GetPositionY()) * (node->y - GetPositionY()) +
                (node->z - GetPositionZ()) * (node->z - GetPositionZ()) >
                (2 * INTERACTION_DISTANCE) * (2 * INTERACTION_DISTANCE) * (2 * INTERACTION_DISTANCE))
        {
            GetSession()->SendActivateTaxiReply(ERR_TAXITOOFARAWAY);
            return false;
        }
    }
    // node must have pos if taxi master case (npc != NULL)
    else if (npc)
    {
        GetSession()->SendActivateTaxiReply(ERR_TAXIUNSPECIFIEDSERVERERROR);
        return false;
    }

    // Prepare to flight start now

    // stop combat at start taxi flight if any
    CombatStop();

    // stop trade (client cancel trade at taxi map open but cheating tools can be used for reopen it)
    TradeCancel(true);

    // clean not finished taxi path if any
    m_taxi.ClearTaxiDestinations();

    // 0 element current node
    m_taxi.AddTaxiDestination(sourcenode);

    // fill destinations path tail
    uint32 sourcepath = 0;
    uint32 totalcost = 0;

    uint32 prevnode = sourcenode;
    uint32 lastnode = 0;

    for (uint32 i = 1; i < nodes.size(); ++i)
    {
        uint32 path, cost;

        lastnode = nodes[i];
        sObjectMgr.GetTaxiPath(prevnode, lastnode, path, cost);

        if (!path)
        {
            m_taxi.ClearTaxiDestinations();
            return false;
        }

        totalcost += cost;

        if (prevnode == sourcenode)
            sourcepath = path;

        m_taxi.AddTaxiDestination(lastnode);

        prevnode = lastnode;
    }

    // get mount model (in case non taximaster (npc==NULL) allow more wide lookup)
    uint32 mount_display_id = sObjectMgr.GetTaxiMountDisplayId(sourcenode, GetTeam(), npc == NULL);

    // in spell case allow 0 model
    if ((mount_display_id == 0 && spellid == 0) || sourcepath == 0)
    {
        GetSession()->SendActivateTaxiReply(ERR_TAXIUNSPECIFIEDSERVERERROR);

        m_taxi.ClearTaxiDestinations();
        return false;
    }

    uint32 money = GetMoney();

    if (npc)
        totalcost = (uint32)ceil(totalcost * GetReputationPriceDiscount(npc));

    if (money < totalcost)
    {
        GetSession()->SendActivateTaxiReply(ERR_TAXINOTENOUGHMONEY);

        m_taxi.ClearTaxiDestinations();
        return false;
    }

    // Checks and preparations done, DO FLIGHT
    ModifyMoney(-(int32)totalcost);

    // prevent stealth flight
    RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);

    GetSession()->SendActivateTaxiReply(ERR_TAXIOK);
    GetSession()->SendDoFlight(mount_display_id, sourcepath);

    return true;
}

bool Player::ActivateTaxiPathTo(uint32 taxi_path_id, uint32 spellid /*= 0*/)
{
    TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(taxi_path_id);
    if (!entry)
        return false;

    std::vector<uint32> nodes;

    nodes.resize(2);
    nodes[0] = entry->from;
    nodes[1] = entry->to;

    return ActivateTaxiPathTo(nodes, NULL, spellid);
}

void Player::ContinueTaxiFlight()
{
    uint32 sourceNode = m_taxi.GetTaxiSource();
    if (!sourceNode)
        return;

    DEBUG_LOG("WORLD: Restart character %u taxi flight", GetGUIDLow());

    uint32 mountDisplayId = sObjectMgr.GetTaxiMountDisplayId(sourceNode, GetTeam(), true);
    uint32 path = m_taxi.GetCurrentTaxiPath();

    // search appropriate start path node
    uint32 startNode = 0;

    TaxiPathNodeList const& nodeList = sTaxiPathNodesByPath[path];

    float distPrev = MAP_SIZE * MAP_SIZE;
    float distNext =
        (nodeList[0].x - GetPositionX()) * (nodeList[0].x - GetPositionX()) +
        (nodeList[0].y - GetPositionY()) * (nodeList[0].y - GetPositionY()) +
        (nodeList[0].z - GetPositionZ()) * (nodeList[0].z - GetPositionZ());

    for (uint32 i = 1; i < nodeList.size(); ++i)
    {
        TaxiPathNodeEntry const& node = nodeList[i];
        TaxiPathNodeEntry const& prevNode = nodeList[i - 1];

        // skip nodes at another map
        if (node.mapid != GetMapId())
            continue;

        distPrev = distNext;

        distNext =
            (node.x - GetPositionX()) * (node.x - GetPositionX()) +
            (node.y - GetPositionY()) * (node.y - GetPositionY()) +
            (node.z - GetPositionZ()) * (node.z - GetPositionZ());

        float distNodes =
            (node.x - prevNode.x) * (node.x - prevNode.x) +
            (node.y - prevNode.y) * (node.y - prevNode.y) +
            (node.z - prevNode.z) * (node.z - prevNode.z);

        if (distNext + distPrev < distNodes)
        {
            startNode = i;
            break;
        }
    }

    GetSession()->SendDoFlight(mountDisplayId, path, startNode);
}

void Player::ProhibitSpellSchool(SpellSchoolMask idSchoolMask, uint32 unTimeMs)
{
    // last check 1.12
    WorldPacket data(SMSG_SPELL_COOLDOWN, 8 + m_spells.size() * 8);
    data << GetObjectGuid();
    time_t curTime = time(NULL);
    for (PlayerSpellMap::const_iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        if (itr->second.state == PLAYERSPELL_REMOVED)
            continue;
        uint32 unSpellId = itr->first;
        SpellEntry const* spellInfo = sSpellStore.LookupEntry(unSpellId);
        if (!spellInfo)
        {
            MANGOS_ASSERT(spellInfo);
            continue;
        }

        // Not send cooldown for this spells
        if (spellInfo->HasAttribute(SPELL_ATTR_DISABLED_WHILE_ACTIVE))
            continue;

        if ((idSchoolMask & GetSpellSchoolMask(spellInfo)) && GetSpellCooldownDelay(unSpellId) < unTimeMs)
        {
            data << uint32(unSpellId);
            data << uint32(unTimeMs);                       // in m.secs
            AddSpellCooldown(unSpellId, 0, curTime + unTimeMs / IN_MILLISECONDS);
        }
    }
    GetSession()->SendPacket(&data);
}

void Player::InitDataForForm(bool reapplyMods)
{
    ShapeshiftForm form = GetShapeshiftForm();

    switch (form)
    {
        case FORM_CAT:
        {
            SetAttackTime(BASE_ATTACK, 1000);               // Speed 1
            SetAttackTime(OFF_ATTACK, 1000);                // Speed 1

            if (getPowerType() != POWER_ENERGY)
                setPowerType(POWER_ENERGY);
            break;
        }
        case FORM_BEAR:
        case FORM_DIREBEAR:
        {
            SetAttackTime(BASE_ATTACK, 2500);               // Speed 2.5
            SetAttackTime(OFF_ATTACK, 2500);                // Speed 2.5

            if (getPowerType() != POWER_RAGE)
                setPowerType(POWER_RAGE);
            break;
        }
        default:                                            // 0, for example
        {
            SetRegularAttackTime();

            ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(getClass());
            if (cEntry && cEntry->powerType < MAX_POWERS && uint32(getPowerType()) != cEntry->powerType)
                setPowerType(Powers(cEntry->powerType));

            break;
        }
    }

    // update auras at form change, ignore this at mods reapply (.reset stats/etc) when form not change.
    if (!reapplyMods)
        UpdateEquipSpellsAtFormChange();

    UpdateAttackPowerAndDamage();
    UpdateAttackPowerAndDamage(true);
}

void Player::InitDisplayIds()
{
    PlayerInfo const* info = sObjectMgr.GetPlayerInfo(getRace(), getClass());
    if (!info)
    {
        sLog.outError("Player %u has incorrect race/class pair. Can't init display ids.", GetGUIDLow());
        return;
    }

    uint8 gender = getGender();
    switch (gender)
    {
        case GENDER_FEMALE:
            // workaround for tauren scale
            if (getRace() == RACE_TAUREN)
                SetObjectScale(DEFAULT_TAUREN_FEMALE_SCALE);
            else
                SetObjectScale(DEFAULT_OBJECT_SCALE);

            SetDisplayId(info->displayId_f);
            SetNativeDisplayId(info->displayId_f);
            break;
        case GENDER_MALE:
            // workaround for tauren scale
            if (getRace() == RACE_TAUREN)
                SetObjectScale(DEFAULT_TAUREN_MALE_SCALE);
            else
                SetObjectScale(DEFAULT_OBJECT_SCALE);

            SetDisplayId(info->displayId_m);
            SetNativeDisplayId(info->displayId_m);
            break;
        default:
            sLog.outError("Invalid gender %u for player", gender);
            return;
    }
}

// Return true is the bought item has a max count to force refresh of window by caller
bool Player::BuyItemFromVendor(ObjectGuid vendorGuid, uint32 item, uint8 count, uint8 bag, uint8 slot)
{
    // cheating attempt
    if (count < 1) count = 1;

    if (!isAlive())
        return false;

    ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(item);
    if (!pProto)
    {
        SendBuyError(BUY_ERR_CANT_FIND_ITEM, NULL, item, 0);
        return false;
    }

    Creature* pCreature = GetNPCIfCanInteractWith(vendorGuid, UNIT_NPC_FLAG_VENDOR);
    if (!pCreature)
    {
        DEBUG_LOG("WORLD: BuyItemFromVendor - %s not found or you can't interact with him.", vendorGuid.GetString().c_str());
        SendBuyError(BUY_ERR_DISTANCE_TOO_FAR, NULL, item, 0);
        return false;
    }

    VendorItemData const* vItems = pCreature->GetVendorItems();
    VendorItemData const* tItems = pCreature->GetVendorTemplateItems();
    if ((!vItems || vItems->Empty()) && (!tItems || tItems->Empty()))
    {
        SendBuyError(BUY_ERR_CANT_FIND_ITEM, pCreature, item, 0);
        return false;
    }

    uint32 vCount = vItems ? vItems->GetItemCount() : 0;
    uint32 tCount = tItems ? tItems->GetItemCount() : 0;

    size_t vendorslot = vItems ? vItems->FindItemSlot(item) : vCount;
    if (vendorslot >= vCount)
        vendorslot = vCount + (tItems ? tItems->FindItemSlot(item) : tCount);

    if (vendorslot >= vCount + tCount)
    {
        SendBuyError(BUY_ERR_CANT_FIND_ITEM, pCreature, item, 0);
        return false;
    }

    VendorItem const* crItem = vendorslot < vCount ? vItems->GetItem(vendorslot) : tItems->GetItem(vendorslot - vCount);
    if (!crItem || crItem->item != item)                    // store diff item (cheating)
    {
        SendBuyError(BUY_ERR_CANT_FIND_ITEM, pCreature, item, 0);
        return false;
    }

    uint32 totalCount = pProto->BuyCount * count;

    // check current item amount if it limited
    if (crItem->maxcount != 0)
    {
        if (pCreature->GetVendorItemCurrentCount(crItem) < totalCount)
        {
            SendBuyError(BUY_ERR_ITEM_ALREADY_SOLD, pCreature, item, 0);
            return false;
        }
    }

    uint32 reqFaction = pProto->RequiredReputationFaction;
    if (!reqFaction && pProto->RequiredReputationRank > 0)
        reqFaction = pCreature->getFactionTemplateEntry()->faction;

    if (uint32(GetReputationRank(reqFaction)) < pProto->RequiredReputationRank)
    {
        SendBuyError(BUY_ERR_REPUTATION_REQUIRE, pCreature, item, 0);
        return false;
    }

    // not check level requiremnt for normal items (PvP related bonus items is another case)
    if (pProto->RequiredHonorRank && (GetHonorHighestRankInfo().rank < (uint8)pProto->RequiredHonorRank || getLevel() < pProto->RequiredLevel))
    {
        SendBuyError(BUY_ERR_RANK_REQUIRE, pCreature, item, 0);
        return false;
    }

    if (crItem->conditionId && !isGameMaster() && !sObjectMgr.IsPlayerMeetToCondition(crItem->conditionId, this, pCreature->GetMap(), pCreature, CONDITION_FROM_VENDOR))
    {
        SendBuyError(BUY_ERR_CANT_FIND_ITEM, pCreature, item, 0);
        return false;
    }

    uint32 price = pProto->BuyPrice * count;

    // reputation discount
    price = uint32(floor(price * GetReputationPriceDiscount(pCreature)));

    if (GetMoney() < price)
    {
        SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, pCreature, item, 0);
        return false;
    }

    Item* pItem = NULL;

    if ((bag == NULL_BAG && slot == NULL_SLOT) || IsInventoryPos(bag, slot))
    {
        ItemPosCountVec dest;
        InventoryResult msg = CanStoreNewItem(bag, slot, dest, item, totalCount);
        if (msg != EQUIP_ERR_OK)
        {
            SendEquipError(msg, NULL, NULL, item);
            return false;
        }

        ModifyMoney(-int32(price));

        pItem = StoreNewItem(dest, item, true);
    }
    else if (IsEquipmentPos(bag, slot))
    {
        if (totalCount != 1)
        {
            SendEquipError(EQUIP_ERR_ITEM_CANT_BE_EQUIPPED, NULL, NULL);
            return false;
        }

        uint16 dest;
        InventoryResult msg = CanEquipNewItem(slot, dest, item, false);
        if (msg != EQUIP_ERR_OK)
        {
            SendEquipError(msg, NULL, NULL, item);
            return false;
        }

        ModifyMoney(-int32(price));

        pItem = EquipNewItem(dest, item, true);

        if (pItem)
            AutoUnequipOffhandIfNeed();
    }
    else
    {
        SendEquipError(EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT, NULL, NULL);
        return false;
    }

    if (!pItem)
        return false;

    uint32 new_count = pCreature->UpdateVendorItemCurrentCount(crItem, totalCount);

    WorldPacket data(SMSG_BUY_ITEM, 8 + 4 + 4 + 4);
    data << pCreature->GetObjectGuid();
    data << uint32(vendorslot + 1);                 // numbered from 1 at client
    data << uint32(crItem->maxcount > 0 ? new_count : 0xFFFFFFFF);
    data << uint32(count);
    GetSession()->SendPacket(&data);

    SendNewItem(pItem, totalCount, true, false, false);

    return crItem->maxcount != 0;
}

void Player::UpdateHomebindTime(uint32 time)
{
    // GMs never get homebind timer online
    if (m_InstanceValid || isGameMaster())
    {
        if (m_HomebindTimer)                                // instance valid, but timer not reset
        {
            // hide reminder
            WorldPacket data(SMSG_RAID_GROUP_ONLY, 4 + 4);
            data << uint32(0);
            data << uint32(ERR_RAID_GROUP_NONE);            // error used only when timer = 0
            GetSession()->SendPacket(&data);
        }
        // instance is valid, reset homebind timer
        m_HomebindTimer = 0;
    }
    else if (m_HomebindTimer > 0)
    {
        if (time >= m_HomebindTimer)
        {
            // teleport to homebind location
            TeleportTo(m_homebindMapId, m_homebindX, m_homebindY, m_homebindZ, GetOrientation());
        }
        else
            m_HomebindTimer -= time;
    }
    else
    {
        // instance is invalid, start homebind timer
        m_HomebindTimer = 60000;
        // send message to player
        WorldPacket data(SMSG_RAID_GROUP_ONLY, 4 + 4);
        data << uint32(m_HomebindTimer);
        data << uint32(ERR_RAID_GROUP_NONE);                // error used only when timer = 0
        GetSession()->SendPacket(&data);
        DEBUG_LOG("PLAYER: Player '%s' (GUID: %u) will be teleported to homebind in 60 seconds", GetName(), GetGUIDLow());
    }
}

void Player::UpdatePvP(bool state, bool ovrride)
{
    if (!state || ovrride)
    {
        SetPvP(state);
        pvpInfo.endTimer = 0;
    }
    else
    {
        if (pvpInfo.endTimer != 0)
            pvpInfo.endTimer = time(NULL);
        else
            SetPvP(state);
    }
}

void Player::AddSpellAndCategoryCooldowns(SpellEntry const* spellInfo, uint32 itemId, Spell* spell, bool infinityCooldown)
{
    // init cooldown values
    uint32 cat   = 0;
    int32 rec    = -1;
    int32 catrec = -1;

    // some special item spells without correct cooldown in SpellInfo
    // cooldown information stored in item prototype
    // This used in same way in WorldSession::HandleItemQuerySingleOpcode data sending to client.

    if (itemId)
    {
        if (ItemPrototype const* proto = ObjectMgr::GetItemPrototype(itemId))
        {
            for (int idx = 0; idx < MAX_ITEM_PROTO_SPELLS; ++idx)
            {
                if (proto->Spells[idx].SpellId == spellInfo->Id)
                {
                    cat    = proto->Spells[idx].SpellCategory;
                    rec    = proto->Spells[idx].SpellCooldown;
                    catrec = proto->Spells[idx].SpellCategoryCooldown;
                    break;
                }
            }
        }
    }

    // if no cooldown found above then base at DBC data
    if (rec < 0 && catrec < 0)
    {
        cat = spellInfo->Category;
        rec = spellInfo->RecoveryTime;
        catrec = spellInfo->CategoryRecoveryTime;
    }

    time_t curTime = time(NULL);

    time_t catrecTime;
    time_t recTime;

    // overwrite time for selected category
    if (infinityCooldown)
    {
        // use +MONTH as infinity mark for spell cooldown (will checked as MONTH/2 at save ans skipped)
        // but not allow ignore until reset or re-login
        catrecTime = catrec > 0 ? curTime + infinityCooldownDelay : 0;
        recTime    = rec    > 0 ? curTime + infinityCooldownDelay : catrecTime;
    }
    else
    {
        // shoot spells used equipped item cooldown values already assigned in GetAttackTime(RANGED_ATTACK)
        // prevent 0 cooldowns set by another way
        if (rec <= 0 && catrec <= 0 && (cat == 76 || cat == 351))
            rec = GetAttackTime(RANGED_ATTACK);

        // Now we have cooldown data (if found any), time to apply mods
        if (rec > 0)
            ApplySpellMod(spellInfo->Id, SPELLMOD_COOLDOWN, rec, spell);

        if (catrec > 0)
            ApplySpellMod(spellInfo->Id, SPELLMOD_COOLDOWN, catrec, spell);

        // replace negative cooldowns by 0
        if (rec < 0) rec = 0;
        if (catrec < 0) catrec = 0;

        // no cooldown after applying spell mods
        if (rec == 0 && catrec == 0)
            return;

        catrecTime = catrec ? curTime + catrec / IN_MILLISECONDS : 0;
        recTime    = rec ? curTime + rec / IN_MILLISECONDS : catrecTime;
    }

    // self spell cooldown
    if (recTime > 0)
        AddSpellCooldown(spellInfo->Id, itemId, recTime);

    // category spells
    if (cat && catrec > 0)
    {
        SpellCategoryStore::const_iterator i_scstore = sSpellCategoryStore.find(cat);
        if (i_scstore != sSpellCategoryStore.end())
        {
            for (SpellCategorySet::const_iterator i_scset = i_scstore->second.begin(); i_scset != i_scstore->second.end(); ++i_scset)
            {
                if (*i_scset == spellInfo->Id)              // skip main spell, already handled above
                    continue;

                AddSpellCooldown(*i_scset, itemId, catrecTime);
            }
        }
    }
}

void Player::AddSpellCooldown(uint32 spellid, uint32 itemid, time_t end_time)
{
    SpellCooldown sc;
    sc.end = end_time;
    sc.itemid = itemid;
    m_spellCooldowns[spellid] = sc;
}

void Player::SendCooldownEvent(SpellEntry const* spellInfo, uint32 itemId, Spell* spell)
{
    // start cooldowns at server side, if any
    AddSpellAndCategoryCooldowns(spellInfo, itemId, spell);

    // Send activate cooldown timer (possible 0) at client side
    WorldPacket data(SMSG_COOLDOWN_EVENT, (4 + 8));
    data << uint32(spellInfo->Id);
    data << GetObjectGuid();
    SendDirectMessage(&data);
}

void Player::SetBattleGroundEntryPoint(Player* leader /*= NULL*/)
{
    // chat command use case, or non-group join
    if (!leader || !leader->IsInWorld() || leader->IsTaxiFlying() || leader->GetMap()->IsDungeon() || leader->GetMap()->IsBattleGround())
        leader = this;

    if (leader->IsInWorld() && !leader->IsTaxiFlying())
    {
        // If map is dungeon find linked graveyard
        if (leader->GetMap()->IsDungeon())
        {
            if (const WorldSafeLocsEntry* entry = sObjectMgr.GetClosestGraveYard(leader->GetPositionX(), leader->GetPositionY(), leader->GetPositionZ(), leader->GetMapId(), leader->GetTeam()))
            {
                m_bgData.joinPos = WorldLocation(entry->map_id, entry->x, entry->y, entry->z, 0.0f);
                m_bgData.m_needSave = true;
                return;
            }
            else
                sLog.outError("SetBattleGroundEntryPoint: Dungeon map %u has no linked graveyard, setting home location as entry point.", leader->GetMapId());
        }
        // If new entry point is not BG or arena set it
        else if (!leader->GetMap()->IsBattleGround())
        {
            m_bgData.joinPos = WorldLocation(leader->GetMapId(), leader->GetPositionX(), leader->GetPositionY(), leader->GetPositionZ(), leader->GetOrientation());
            m_bgData.m_needSave = true;
            return;
        }
    }

    // In error cases use homebind position
    m_bgData.joinPos = WorldLocation(m_homebindMapId, m_homebindX, m_homebindY, m_homebindZ, 0.0f);
    m_bgData.m_needSave = true;
}

void Player::LeaveBattleground(bool teleportToEntryPoint)
{
    if (BattleGround* bg = GetBattleGround())
    {
        bg->RemovePlayerAtLeave(GetObjectGuid(), teleportToEntryPoint, true);

        // call after remove to be sure that player resurrected for correct cast
        if (!isGameMaster() && sWorld.getConfig(CONFIG_BOOL_BATTLEGROUND_CAST_DESERTER))
        {
            if (bg->GetStatus() == STATUS_IN_PROGRESS || bg->GetStatus() == STATUS_WAIT_JOIN)
            {
                // lets check if player was teleported from BG and schedule delayed Deserter spell cast
                if (IsBeingTeleportedFar())
                {
                    ScheduleDelayedOperation(DELAYED_SPELL_CAST_DESERTER);
                    return;
                }

                CastSpell(this, 26013, true);               // Deserter
            }
        }
    }
}

bool Player::CanJoinToBattleground() const
{
    // check Deserter debuff
    if (GetDummyAura(26013))
        return false;

    return true;
}

bool Player::IsVisibleInGridForPlayer(Player* pl) const
{
    // gamemaster in GM mode see all, including ghosts
    if (pl->isGameMaster() && GetSession()->GetSecurity() <= pl->GetSession()->GetSecurity())
        return true;

    // player see dead player/ghost from own group/raid
    if (IsInSameRaidWith(pl))
        return true;

    // Live player see live player or dead player with not realized corpse
    if (pl->isAlive() || pl->m_deathTimer > 0)
        return isAlive() || m_deathTimer > 0;

    // Ghost see other friendly ghosts, that's for sure
    if (!(isAlive() || m_deathTimer > 0) && IsFriendlyTo(pl))
        return true;

    // Dead player see live players near own corpse
    if (isAlive())
    {
        if (Corpse* corpse = pl->GetCorpse())
        {
            // 20 - aggro distance for same level, 25 - max additional distance if player level less that creature level
            if (corpse->IsWithinDistInMap(this, (20 + 25) * sWorld.getConfig(CONFIG_FLOAT_RATE_CREATURE_AGGRO)))
                return true;
        }
    }

    // and not see any other
    return false;
}

bool Player::IsVisibleGloballyFor(Player* u) const
{
    if (!u)
        return false;

    // Always can see self
    if (u == this)
        return true;

    // Visible units, always are visible for all players
    if (GetVisibility() == VISIBILITY_ON)
        return true;

    // GMs are visible for higher gms (or players are visible for gms)
    if (u->GetSession()->GetSecurity() > SEC_PLAYER)
        return GetSession()->GetSecurity() <= u->GetSession()->GetSecurity();

    // non faction visibility non-breakable for non-GMs
    if (GetVisibility() == VISIBILITY_OFF)
        return false;

    // non-gm stealth/invisibility not hide from global player lists
    return true;
}

template<class T>
inline void BeforeVisibilityDestroy(T* /*t*/, Player* /*p*/)
{
}

template<>
inline void BeforeVisibilityDestroy<Creature>(Creature* t, Player* p)
{
    if (p->GetPetGuid() == t->GetObjectGuid() && ((Creature*)t)->IsPet())
        ((Pet*)t)->Unsummon(PET_SAVE_REAGENTS);
}

void Player::UpdateVisibilityOf(WorldObject const* viewPoint, WorldObject* target)
{
    if (HaveAtClient(target))
    {
        if (!target->isVisibleForInState(this, viewPoint, true))
        {
            ObjectGuid t_guid = target->GetObjectGuid();

            if (target->GetTypeId() == TYPEID_UNIT)
                BeforeVisibilityDestroy<Creature>((Creature*)target, this);

            target->DestroyForPlayer(this);
            m_clientGUIDs.erase(t_guid);

            DEBUG_FILTER_LOG(LOG_FILTER_VISIBILITY_CHANGES, "UpdateVisibilityOf: %s out of range for player %u. Distance = %f", t_guid.GetString().c_str(), GetGUIDLow(), GetDistance(target));
        }
    }
    else
    {
        if (target->isVisibleForInState(this, viewPoint, false))
        {
            target->SendCreateUpdateToPlayer(this);
            if (target->GetTypeId() != TYPEID_GAMEOBJECT || !((GameObject*)target)->IsTransport())
                m_clientGUIDs.insert(target->GetObjectGuid());

            DEBUG_FILTER_LOG(LOG_FILTER_VISIBILITY_CHANGES, "UpdateVisibilityOf: %s is visible now for player %u. Distance = %f", target->GetGuidStr().c_str(), GetGUIDLow(), GetDistance(target));

            // target aura duration for caster show only if target exist at caster client
            // send data at target visibility change (adding to client)
            if (target != this && target->isType(TYPEMASK_UNIT))
                SendAuraDurationsForTarget((Unit*)target);
        }
    }
}

template<class T>
inline void UpdateVisibilityOf_helper(GuidSet& s64, T* target)
{
    s64.insert(target->GetObjectGuid());
}

template<>
inline void UpdateVisibilityOf_helper(GuidSet& s64, GameObject* target)
{
    if (!target->IsTransport())
        s64.insert(target->GetObjectGuid());
}

template<class T>
void Player::UpdateVisibilityOf(WorldObject const* viewPoint, T* target, UpdateData& data, std::set<WorldObject*>& visibleNow)
{
    if (HaveAtClient(target))
    {
        if (!target->isVisibleForInState(this, viewPoint, true))
        {
            BeforeVisibilityDestroy<T>(target, this);

            ObjectGuid t_guid = target->GetObjectGuid();

            target->BuildOutOfRangeUpdateBlock(&data);
            m_clientGUIDs.erase(t_guid);

            DEBUG_FILTER_LOG(LOG_FILTER_VISIBILITY_CHANGES, "UpdateVisibilityOf(TemplateV): %s is out of range for %s. Distance = %f", t_guid.GetString().c_str(), GetGuidStr().c_str(), GetDistance(target));
        }
    }
    else
    {
        if (target->isVisibleForInState(this, viewPoint, false))
        {
            visibleNow.insert(target);
            target->BuildCreateUpdateBlockForPlayer(&data, this);
            UpdateVisibilityOf_helper(m_clientGUIDs, target);

            DEBUG_FILTER_LOG(LOG_FILTER_VISIBILITY_CHANGES, "UpdateVisibilityOf(TemplateV): %s is visible now for %s. Distance = %f", target->GetGuidStr().c_str(), GetGuidStr().c_str(), GetDistance(target));
        }
    }
}

template void Player::UpdateVisibilityOf(WorldObject const* viewPoint, Player*        target, UpdateData& data, std::set<WorldObject*>& visibleNow);
template void Player::UpdateVisibilityOf(WorldObject const* viewPoint, Creature*      target, UpdateData& data, std::set<WorldObject*>& visibleNow);
template void Player::UpdateVisibilityOf(WorldObject const* viewPoint, Corpse*        target, UpdateData& data, std::set<WorldObject*>& visibleNow);
template void Player::UpdateVisibilityOf(WorldObject const* viewPoint, GameObject*    target, UpdateData& data, std::set<WorldObject*>& visibleNow);
template void Player::UpdateVisibilityOf(WorldObject const* viewPoint, DynamicObject* target, UpdateData& data, std::set<WorldObject*>& visibleNow);

void Player::InitPrimaryProfessions()
{
    uint32 maxProfs = GetSession()->GetSecurity() < AccountTypes(sWorld.getConfig(CONFIG_UINT32_TRADE_SKILL_GMIGNORE_MAX_PRIMARY_COUNT))
                      ? sWorld.getConfig(CONFIG_UINT32_MAX_PRIMARY_TRADE_SKILL) : 10;
    SetFreePrimaryProfessions(maxProfs);
}

void Player::SetComboPoints()
{
    Unit* combotarget = ObjectAccessor::GetUnit(*this, m_comboTargetGuid);
    if (combotarget)
    {
        SetGuidValue(PLAYER_FIELD_COMBO_TARGET, combotarget->GetObjectGuid());
        SetByteValue(PLAYER_FIELD_BYTES, 1, m_comboPoints);
    }
    /*else
    {
        // can be NULL, and then points=0. Use unknown; to reset points of some sort?
        data << PackedGuid();
        data << uint8(0);
        GetSession()->SendPacket(&data);
    }*/
}

void Player::AddComboPoints(Unit* target, int8 count)
{
    if (!count)
        return;

    // without combo points lost (duration checked in aura)
    RemoveSpellsCausingAura(SPELL_AURA_RETAIN_COMBO_POINTS);

    if (target->GetObjectGuid() == m_comboTargetGuid)
    {
        m_comboPoints += count;
    }
    else
    {
        if (m_comboTargetGuid)
            if (Unit* target2 = ObjectAccessor::GetUnit(*this, m_comboTargetGuid))
                target2->RemoveComboPointHolder(GetGUIDLow());

        m_comboTargetGuid = target->GetObjectGuid();
        m_comboPoints = count;

        target->AddComboPointHolder(GetGUIDLow());
    }

    if (m_comboPoints > 5) m_comboPoints = 5;
    if (m_comboPoints < 0) m_comboPoints = 0;

    SetComboPoints();
}

void Player::ClearComboPoints()
{
    if (!m_comboTargetGuid)
        return;

    // without combopoints lost (duration checked in aura)
    RemoveSpellsCausingAura(SPELL_AURA_RETAIN_COMBO_POINTS);

    m_comboPoints = 0;

    SetComboPoints();

    if (Unit* target = ObjectAccessor::GetUnit(*this, m_comboTargetGuid))
        target->RemoveComboPointHolder(GetGUIDLow());

    m_comboTargetGuid.Clear();
}

void Player::SetGroup(Group* group, int8 subgroup)
{
    if (group == NULL)
        m_group.unlink();
    else
    {
        // never use SetGroup without a subgroup unless you specify NULL for group
        MANGOS_ASSERT(subgroup >= 0);
        m_group.link(group, this);
        m_group.setSubGroup((uint8)subgroup);
    }
}

void Player::SendInitialPacketsBeforeAddToMap()
{
    WorldPacket data(SMSG_SET_REST_START, 4);
    data << uint32(0);                                      // unknown, may be rest state time or experience
    GetSession()->SendPacket(&data);

    // Homebind
    data.Initialize(SMSG_BINDPOINTUPDATE, 5 * 4);
    data << m_homebindX << m_homebindY << m_homebindZ;
    data << (uint32) m_homebindMapId;
    data << (uint32) m_homebindAreaId;
    GetSession()->SendPacket(&data);

    // SMSG_SET_PROFICIENCY
    // SMSG_UPDATE_AURA_DURATION

    // tutorial stuff
    GetSession()->SendTutorialsData();
    SendInitialSpells();
    SendInitialActionButtons();
    m_reputationMgr.SendInitialReputations();
    UpdateHonor();

    // SMSG_SET_AURA_SINGLE

    data.Initialize(SMSG_LOGIN_SETTIMESPEED, 4 + 4);
    data << uint32(secsToTimeBitFields(sWorld.GetGameTime()));
    data << (float)0.01666667f;                             // game speed
    GetSession()->SendPacket(&data);

    // set fly flag if in fly form or taxi flight to prevent visually drop at ground in showup moment
    if (IsTaxiFlying())
        m_movementInfo.AddMovementFlag(MOVEFLAG_FLYING);

    SetMover(this);
}

void Player::SendInitialPacketsAfterAddToMap()
{
    // update zone
    uint32 newzone, newarea;
    GetZoneAndAreaId(newzone, newarea);
    UpdateZone(newzone, newarea);                           // also call SendInitWorldStates();

    CastSpell(this, 836, true);                             // LOGINEFFECT

    // set some aura effects that send packet to player client after add player to map
    // SendMessageToSet not send it to player not it map, only for aura that not changed anything at re-apply
    // same auras state lost at far teleport, send it one more time in this case also
    static const AuraType auratypes[] =
    {
        SPELL_AURA_MOD_FEAR,     SPELL_AURA_TRANSFORM,                 SPELL_AURA_WATER_WALK,
        SPELL_AURA_FEATHER_FALL, SPELL_AURA_HOVER,                     SPELL_AURA_SAFE_FALL,
        SPELL_AURA_NONE
    };
    for (AuraType const* itr = &auratypes[0]; itr && itr[0] != SPELL_AURA_NONE; ++itr)
    {
        Unit::AuraList const& auraList = GetAurasByType(*itr);
        if (!auraList.empty())
            auraList.front()->ApplyModifier(true, true);
    }

    if (HasAuraType(SPELL_AURA_MOD_STUN) || HasAuraType(SPELL_AURA_MOD_ROOT))
        SetRoot(true);

    SendEnchantmentDurations();                             // must be after add to map
    SendItemDurations();                                    // must be after add to map
}

void Player::SendUpdateToOutOfRangeGroupMembers()
{
    if (m_groupUpdateMask == GROUP_UPDATE_FLAG_NONE)
        return;
    if (Group* group = GetGroup())
        group->UpdatePlayerOutOfRange(this);

    m_groupUpdateMask = GROUP_UPDATE_FLAG_NONE;
    m_auraUpdateMask = 0;
    if (Pet* pet = GetPet())
        pet->ResetAuraUpdateMask();
}

void Player::SendTransferAbortedByLockStatus(MapEntry const* mapEntry, AreaLockStatus lockStatus, uint32 miscRequirement)
{
    MANGOS_ASSERT(mapEntry);

    DEBUG_LOG("SendTransferAbortedByLockStatus: Called for %s on map %u, LockAreaStatus %u, miscRequirement %u)", GetGuidStr().c_str(), mapEntry->MapID, lockStatus, miscRequirement);

    switch (lockStatus)
    {
        case AREA_LOCKSTATUS_TOO_LOW_LEVEL:
            GetSession()->SendAreaTriggerMessage(GetSession()->GetMangosString(LANG_LEVEL_MINREQUIRED), miscRequirement);
            break;
        case AREA_LOCKSTATUS_ZONE_IN_COMBAT:
            GetSession()->SendTransferAborted(mapEntry->MapID, TRANSFER_ABORT_ZONE_IN_COMBAT);
            break;
        case AREA_LOCKSTATUS_INSTANCE_IS_FULL:
            GetSession()->SendTransferAborted(mapEntry->MapID, TRANSFER_ABORT_MAX_PLAYERS);
            break;
        case AREA_LOCKSTATUS_QUEST_NOT_COMPLETED:
            if (mapEntry->IsContinent())               // do not report anything for quest areatrigge
            {
                DEBUG_LOG("SendTransferAbortedByLockStatus: LockAreaStatus %u, do not teleport, no message sent (mapId %u)", lockStatus, mapEntry->MapID);
                break;
            }
            // ToDo: SendAreaTriggerMessage or Transfer Abort for these cases!
            break;
        case AREA_LOCKSTATUS_MISSING_ITEM:
            if (AreaTrigger const* at = sObjectMgr.GetMapEntranceTrigger(mapEntry->MapID))
                GetSession()->SendAreaTriggerMessage(GetSession()->GetMangosString(LANG_LEVEL_MINREQUIRED_AND_ITEM), at->requiredLevel, sObjectMgr.GetItemPrototype(miscRequirement)->Name1);
            break;
        case AREA_LOCKSTATUS_NOT_ALLOWED:
        case AREA_LOCKSTATUS_RAID_LOCKED:
        case AREA_LOCKSTATUS_UNKNOWN_ERROR:
            // ToDo: SendAreaTriggerMessage or Transfer Abort for these cases!
            break;
        case AREA_LOCKSTATUS_OK:
            sLog.outError("SendTransferAbortedByLockStatus: LockAreaStatus AREA_LOCKSTATUS_OK received for %s (mapId %u)", GetGuidStr().c_str(), mapEntry->MapID);
            MANGOS_ASSERT(false);
            break;
        default:
            sLog.outError("SendTransfertAbortedByLockstatus: unhandled LockAreaStatus %u, when %s attempts to enter in map %u", lockStatus, GetGuidStr().c_str(), mapEntry->MapID);
            break;
    }
}

void Player::SendInstanceResetWarning(uint32 mapid, uint32 time)
{
    // type of warning, based on the time remaining until reset
    uint32 type;
    if (time > 3600)
        type = RAID_INSTANCE_WELCOME;
    else if (time > 900 && time <= 3600)
        type = RAID_INSTANCE_WARNING_HOURS;
    else if (time > 300 && time <= 900)
        type = RAID_INSTANCE_WARNING_MIN;
    else
        type = RAID_INSTANCE_WARNING_MIN_SOON;

    WorldPacket data(SMSG_RAID_INSTANCE_MESSAGE, 4 + 4 + 4);
    data << uint32(type);
    data << uint32(mapid);
    data << uint32(time);
    GetSession()->SendPacket(&data);
}

void Player::ApplyEquipCooldown(Item* pItem)
{
    if (pItem->GetProto()->Flags & ITEM_FLAG_NO_EQUIP_COOLDOWN)
        return;

    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        _Spell const& spellData = pItem->GetProto()->Spells[i];

        // no spell
        if (!spellData.SpellId)
            continue;

        // wrong triggering type (note: ITEM_SPELLTRIGGER_ON_NO_DELAY_USE not have cooldown)
        if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_USE)
            continue;

        AddSpellCooldown(spellData.SpellId, pItem->GetEntry(), time(NULL) + 30);

        WorldPacket data(SMSG_ITEM_COOLDOWN, 12);
        data << ObjectGuid(pItem->GetObjectGuid());
        data << uint32(spellData.SpellId);
        GetSession()->SendPacket(&data);
    }
}

void Player::resetSpells()
{
    // not need after this call
    if (HasAtLoginFlag(AT_LOGIN_RESET_SPELLS))
        RemoveAtLoginFlag(AT_LOGIN_RESET_SPELLS, true);

    // make full copy of map (spells removed and marked as deleted at another spell remove
    // and we can't use original map for safe iterative with visit each spell at loop end
    PlayerSpellMap smap = GetSpellMap();

    for (PlayerSpellMap::const_iterator iter = smap.begin(); iter != smap.end(); ++iter)
        removeSpell(iter->first, false, false);             // only iter->first can be accessed, object by iter->second can be deleted already

    learnDefaultSpells();
    learnQuestRewardedSpells();
}

void Player::learnDefaultSpells()
{
    // learn default race/class spells
    PlayerInfo const* info = sObjectMgr.GetPlayerInfo(getRace(), getClass());
    for (PlayerCreateInfoSpells::const_iterator itr = info->spell.begin(); itr != info->spell.end(); ++itr)
    {
        uint32 tspell = *itr;
        DEBUG_LOG("PLAYER (Class: %u Race: %u): Adding initial spell, id = %u", uint32(getClass()), uint32(getRace()), tspell);
        if (!IsInWorld())                                   // will send in INITIAL_SPELLS in list anyway at map add
            addSpell(tspell, true, true, true, false);
        else                                                // but send in normal spell in game learn case
            learnSpell(tspell, true);
    }
}

void Player::learnQuestRewardedSpells(Quest const* quest)
{
    uint32 spell_id = quest->GetRewSpellCast();

    // skip quests without rewarded spell
    if (!spell_id)
        return;

    SpellEntry const* spellInfo = sSpellStore.LookupEntry(spell_id);
    if (!spellInfo)
        return;

    // check learned spells state
    bool found = false;
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (spellInfo->Effect[i] == SPELL_EFFECT_LEARN_SPELL && !HasSpell(spellInfo->EffectTriggerSpell[i]))
        {
            found = true;
            break;
        }
    }

    // skip quests with not teaching spell or already known spell
    if (!found)
        return;

    // prevent learn non first rank unknown profession and second specialization for same profession)
    uint32 learned_0 = spellInfo->EffectTriggerSpell[EFFECT_INDEX_0];
    if (sSpellMgr.GetSpellRank(learned_0) > 1 && !HasSpell(learned_0))
    {
        // not have first rank learned (unlearned prof?)
        uint32 first_spell = sSpellMgr.GetFirstSpellInChain(learned_0);
        if (!HasSpell(first_spell))
            return;

        SpellEntry const* learnedInfo = sSpellStore.LookupEntry(learned_0);
        if (!learnedInfo)
            return;

        // specialization
        if (learnedInfo->Effect[EFFECT_INDEX_0] == SPELL_EFFECT_TRADE_SKILL && learnedInfo->Effect[EFFECT_INDEX_1] == 0)
        {
            // search other specialization for same prof
            for (PlayerSpellMap::const_iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
            {
                if (itr->second.state == PLAYERSPELL_REMOVED || itr->first == learned_0)
                    continue;

                SpellEntry const* itrInfo = sSpellStore.LookupEntry(itr->first);
                if (!itrInfo)
                    return;

                // compare only specializations
                if (itrInfo->Effect[EFFECT_INDEX_0] != SPELL_EFFECT_TRADE_SKILL || itrInfo->Effect[EFFECT_INDEX_1] != 0)
                    continue;

                // compare same chain spells
                if (sSpellMgr.GetFirstSpellInChain(itr->first) != first_spell)
                    continue;

                // now we have 2 specialization, learn possible only if found is lesser specialization rank
                if (!sSpellMgr.IsHighRankOfSpell(learned_0, itr->first))
                    return;
            }
        }
    }

    CastSpell(this, spell_id, true);
}

void Player::learnQuestRewardedSpells()
{
    // learn spells received from quest completing
    for (QuestStatusMap::const_iterator itr = mQuestStatus.begin(); itr != mQuestStatus.end(); ++itr)
    {
        // skip no rewarded quests
        if (!itr->second.m_rewarded)
            continue;

        Quest const* quest = sObjectMgr.GetQuestTemplate(itr->first);
        if (!quest)
            continue;

        learnQuestRewardedSpells(quest);
    }
}

void Player::learnSkillRewardedSpells(uint32 skill_id, uint32 skill_value)
{
    uint32 raceMask  = getRaceMask();
    uint32 classMask = getClassMask();
    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        SkillLineAbilityEntry const* pAbility = sSkillLineAbilityStore.LookupEntry(j);
        if (!pAbility || pAbility->skillId != skill_id || pAbility->learnOnGetSkill != ABILITY_LEARNED_ON_GET_PROFESSION_SKILL)
            continue;
        // Check race if set
        if (pAbility->racemask && !(pAbility->racemask & raceMask))
            continue;
        // Check class if set
        if (pAbility->classmask && !(pAbility->classmask & classMask))
            continue;

        if (sSpellStore.LookupEntry(pAbility->spellId))
        {
            // need unlearn spell
            if (skill_value < pAbility->req_skill_value)
                removeSpell(pAbility->spellId);
            // need learn
            else if (!IsInWorld())
                addSpell(pAbility->spellId, true, true, true, false);
            else
                learnSpell(pAbility->spellId, true);
        }
    }
}

void Player::SendAuraDurationsForTarget(Unit* target)
{
    SpellAuraHolderMap const& auraHolders = target->GetSpellAuraHolderMap();
    for (SpellAuraHolderMap::const_iterator itr = auraHolders.begin(); itr != auraHolders.end(); ++itr)
    {
        SpellAuraHolder* holder = itr->second;

        if (holder->GetAuraSlot() >= MAX_AURAS || holder->IsPassive() || holder->GetCasterGuid() != GetObjectGuid())
            continue;

        holder->SendAuraDurationForCaster(this);
    }
}

BattleGround* Player::GetBattleGround() const
{
    if (GetBattleGroundId() == 0)
        return NULL;

    return sBattleGroundMgr.GetBattleGround(GetBattleGroundId(), m_bgData.bgTypeID);
}

bool Player::GetBGAccessByLevel(BattleGroundTypeId bgTypeId) const
{
    // get a template bg instead of running one
    BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
    if (!bg)
        return false;

    if (getLevel() < bg->GetMinLevel() || getLevel() > bg->GetMaxLevel())
        return false;

    return true;
}

uint32 Player::GetMinLevelForBattleGroundBracketId(BattleGroundBracketId bracket_id, BattleGroundTypeId bgTypeId)
{
    if (bracket_id < 1)
        return 0;

    if (bracket_id > BG_BRACKET_ID_LAST)
        bracket_id = BG_BRACKET_ID_LAST;

    BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
    assert(bg);
    return 10 * bracket_id + bg->GetMinLevel();
}

uint32 Player::GetMaxLevelForBattleGroundBracketId(BattleGroundBracketId bracket_id, BattleGroundTypeId bgTypeId)
{
    if (bracket_id >= BG_BRACKET_ID_LAST)
        return 255;                                         // hardcoded max level

    return GetMinLevelForBattleGroundBracketId(bracket_id, bgTypeId) + 10;
}

BattleGroundBracketId Player::GetBattleGroundBracketIdFromLevel(BattleGroundTypeId bgTypeId) const
{
    BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
    assert(bg);
    if (getLevel() < bg->GetMinLevel())
        return BG_BRACKET_ID_FIRST;

    uint32 bracket_id = (getLevel() - bg->GetMinLevel()) / 10;
    if (bracket_id > MAX_BATTLEGROUND_BRACKETS)
        return BG_BRACKET_ID_LAST;

    return BattleGroundBracketId(bracket_id);
}

float Player::GetReputationPriceDiscount(Creature const* pCreature) const
{
    FactionTemplateEntry const* vendor_faction = pCreature->getFactionTemplateEntry();
    if (!vendor_faction || !vendor_faction->faction)
        return 1.0f;
    
    uint32 discount = 100;
    ReputationRank rank = GetReputationRank(vendor_faction->faction);   // get repution rank for that specific vendor faction
    if (rank >= REP_HONORED)                                            // give 10% reduction if rank is at least honored
        discount -= 10;

    if (GetHonorRankInfo().visualRank >= 3)                             // get pvp grade
    {
        if (FactionTemplateEntry const* player_faction = getFactionTemplateEntry())
        {
            if (player_faction->IsFriendlyTo(*vendor_faction))          // check if its friendly faction (not neutral)
                discount -=10;                                          // give 10% discount if grade is at least sergent
        }
    }
    return float (discount / 100.0f);
}

/**
 * Check spell availability for training base at SkillLineAbility/SkillRaceClassInfo data.
 * Checked allowed race/class and dependent from race/class allowed min level
 *
 * @param spell_id  checked spell id
 * @param pReqlevel if arg provided then function work in view mode (level check not applied but detected minlevel returned to var by arg pointer.
                    if arg not provided then considered train action mode and level checked
 * @return          true if spell available for show in trainer list (with skip level check) or training.
 */
bool Player::IsSpellFitByClassAndRace(uint32 spell_id, uint32* pReqlevel /*= NULL*/) const
{
    uint32 racemask  = getRaceMask();
    uint32 classmask = getClassMask();

    SkillLineAbilityMapBounds bounds = sSpellMgr.GetSkillLineAbilityMapBounds(spell_id);
    if (bounds.first == bounds.second)
        return true;

    for (SkillLineAbilityMap::const_iterator _spell_idx = bounds.first; _spell_idx != bounds.second; ++_spell_idx)
    {
        SkillLineAbilityEntry const* abilityEntry = _spell_idx->second;
        // skip wrong race skills
        if (abilityEntry->racemask && (abilityEntry->racemask & racemask) == 0)
            continue;

        // skip wrong class skills
        if (abilityEntry->classmask && (abilityEntry->classmask & classmask) == 0)
            continue;

        SkillRaceClassInfoMapBounds bounds = sSpellMgr.GetSkillRaceClassInfoMapBounds(abilityEntry->skillId);
        for (SkillRaceClassInfoMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
        {
            SkillRaceClassInfoEntry const* skillRCEntry = itr->second;
            if ((skillRCEntry->raceMask & racemask) && (skillRCEntry->classMask & classmask))
            {
                if (skillRCEntry->flags & ABILITY_SKILL_NONTRAINABLE)
                    return false;

                if (pReqlevel)                              // show trainers list case
                {
                    if (skillRCEntry->reqLevel)
                    {
                        *pReqlevel = skillRCEntry->reqLevel;
                        return true;
                    }
                }
                else                                        // check availble case at train
                {
                    if (skillRCEntry->reqLevel && getLevel() < skillRCEntry->reqLevel)
                        return false;
                }
            }
        }

        return true;
    }

    return false;
}

bool Player::HasQuestForGO(int32 GOId) const
{
    for (int i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questid = GetQuestSlotQuestId(i);
        if (questid == 0)
            continue;

        QuestStatusMap::const_iterator qs_itr = mQuestStatus.find(questid);
        if (qs_itr == mQuestStatus.end())
            continue;

        QuestStatusData const& qs = qs_itr->second;

        if (qs.m_status == QUEST_STATUS_INCOMPLETE)
        {
            Quest const* qinfo = sObjectMgr.GetQuestTemplate(questid);
            if (!qinfo)
                continue;

            if (GetGroup() && GetGroup()->isRaidGroup() && !qinfo->IsAllowedInRaid())
                continue;

            for (int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j)
            {
                if (qinfo->ReqCreatureOrGOId[j] >= 0)       // skip non GO case
                    continue;

                if ((-1)*GOId == qinfo->ReqCreatureOrGOId[j] && qs.m_creatureOrGOcount[j] < qinfo->ReqCreatureOrGOCount[j])
                    return true;
            }
        }
    }
    return false;
}

void Player::UpdateForQuestWorldObjects()
{
    if (m_clientGUIDs.empty())
        return;

    // UpdateData udata;
    // WorldPacket packet;
    for (GuidSet::const_iterator itr = m_clientGUIDs.begin(); itr != m_clientGUIDs.end(); ++itr)
    {
        if (itr->IsGameObject())
        {
            if (GameObject* obj = GetMap()->GetGameObject(*itr))
                // obj->BuildValuesUpdateBlockForPlayer(&udata,this);
                obj->SendCreateUpdateToPlayer(this); //[-ZERO] we must send create packet because of GAMEOBJECT_FLAGS change (not dynamic) - probably incorrect
        }
    }
    // udata.BuildPacket(&packet);
    // GetSession()->SendPacket(&packet);
}

void Player::SummonIfPossible(bool agree)
{
    if (!agree)
    {
        m_summon_expire = 0;
        return;
    }

    // expire and auto declined
    if (m_summon_expire < time(NULL))
        return;

    // stop taxi flight at summon
    if (IsTaxiFlying())
    {
        GetMotionMaster()->MovementExpired();
        m_taxi.ClearTaxiDestinations();
    }

    // drop flag at summon
    // this code can be reached only when GM is summoning player who carries flag, because player should be immune to summoning spells when he carries flag
    if (BattleGround* bg = GetBattleGround())
        bg->EventPlayerDroppedFlag(this);

    m_summon_expire = 0;

    TeleportTo(m_summon_mapid, m_summon_x, m_summon_y, m_summon_z, GetOrientation());
}

void Player::RemoveItemDurations(Item* item)
{
    for (ItemDurationList::iterator itr = m_itemDuration.begin(); itr != m_itemDuration.end(); ++itr)
    {
        if (*itr == item)
        {
            m_itemDuration.erase(itr);
            break;
        }
    }
}

void Player::AddItemDurations(Item* item)
{
    if (item->GetUInt32Value(ITEM_FIELD_DURATION))
    {
        m_itemDuration.push_back(item);
        item->SendTimeUpdate(this);
    }
}

void Player::AutoUnequipOffhandIfNeed()
{
    Item* offItem = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!offItem)
        return;

    // need unequip offhand for 2h-weapon
    if (!IsTwoHandUsed())
        return;

    ItemPosCountVec off_dest;
    uint8 off_msg = CanStoreItem(NULL_BAG, NULL_SLOT, off_dest, offItem, false);
    if (off_msg == EQUIP_ERR_OK)
    {
        RemoveItem(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND, true);
        StoreItem(off_dest, offItem, true);
    }
    else
    {
        MoveItemFromInventory(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND, true);
        CharacterDatabase.BeginTransaction();
        offItem->DeleteFromInventoryDB();                   // deletes item from character's inventory
        offItem->SaveToDB();                                // recursive and not have transaction guard into self, item not in inventory and can be save standalone
        CharacterDatabase.CommitTransaction();

        std::string subject = GetSession()->GetMangosString(LANG_NOT_EQUIPPED_ITEM);
        MailDraft(subject).AddItem(offItem).SendMailTo(this, MailSender(this, MAIL_STATIONERY_GM), MAIL_CHECK_MASK_COPIED);
    }
}

bool Player::HasItemFitToSpellReqirements(SpellEntry const* spellInfo, Item const* ignoreItem)
{
    if (spellInfo->EquippedItemClass < 0)
        return true;

    // scan other equipped items for same requirements (mostly 2 daggers/etc)
    // for optimize check 2 used cases only
    switch (spellInfo->EquippedItemClass)
    {
        case ITEM_CLASS_WEAPON:
        {
            for (int i = EQUIPMENT_SLOT_MAINHAND; i < EQUIPMENT_SLOT_TABARD; ++i)
                if (Item* item = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                    if (item != ignoreItem && item->IsFitToSpellRequirements(spellInfo))
                        return true;
            break;
        }
        case ITEM_CLASS_ARMOR:
        {
            // tabard not have dependent spells
            for (int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_MAINHAND; ++i)
                if (Item* item = GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                    if (item != ignoreItem && item->IsFitToSpellRequirements(spellInfo))
                        return true;

            // shields can be equipped to offhand slot
            if (Item* item = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND))
                if (item != ignoreItem && item->IsFitToSpellRequirements(spellInfo))
                    return true;

            // ranged slot can have some armor subclasses
            if (Item* item = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
                if (item != ignoreItem && item->IsFitToSpellRequirements(spellInfo))
                    return true;

            break;
        }
        default:
            sLog.outError("HasItemFitToSpellReqirements: Not handled spell requirement for item class %u", spellInfo->EquippedItemClass);
            break;
    }

    return false;
}

bool Player::CanNoReagentCast(SpellEntry const* spellInfo) const
{
    // don't take reagents for spells with SPELL_ATTR_EX5_NO_REAGENT_WHILE_PREP
//[-ZERO]    if (spellInfo->AttributesEx5 & SPELL_ATTR_EX5_NO_REAGENT_WHILE_PREP &&
//        HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREPARATION))
//        return true;

    return false;
}

void Player::RemoveItemDependentAurasAndCasts(Item* pItem)
{
    SpellAuraHolderMap& auras = GetSpellAuraHolderMap();
    for (SpellAuraHolderMap::const_iterator itr = auras.begin(); itr != auras.end();)
    {
        SpellAuraHolder* holder = itr->second;

        // skip passive (passive item dependent spells work in another way) and not self applied auras
        SpellEntry const* spellInfo = holder->GetSpellProto();
        if (holder->IsPassive() ||  holder->GetCasterGuid() != GetObjectGuid())
        {
            ++itr;
            continue;
        }

        // skip if not item dependent or have alternative item
        if (HasItemFitToSpellReqirements(spellInfo, pItem))
        {
            ++itr;
            continue;
        }

        // no alt item, remove aura, restart check
        RemoveAurasDueToSpell(holder->GetId());
        itr = auras.begin();
    }

    // currently casted spells can be dependent from item
    for (uint32 i = 0; i < CURRENT_MAX_SPELL; ++i)
        if (Spell* spell = GetCurrentSpell(CurrentSpellTypes(i)))
            if (spell->getState() != SPELL_STATE_DELAYED && !HasItemFitToSpellReqirements(spell->m_spellInfo, pItem))
                InterruptSpell(CurrentSpellTypes(i));
}

uint32 Player::GetResurrectionSpellId()
{
    // search priceless resurrection possibilities
    uint32 prio = 0;
    uint32 spell_id = 0;
    AuraList const& dummyAuras = GetAurasByType(SPELL_AURA_DUMMY);
    for (AuraList::const_iterator itr = dummyAuras.begin(); itr != dummyAuras.end(); ++itr)
    {
        // Soulstone Resurrection                           // prio: 3 (max, non death persistent)
        if (prio < 2 && (*itr)->GetSpellProto()->SpellVisual == 99 && (*itr)->GetSpellProto()->SpellIconID == 92)
        {
            switch ((*itr)->GetId())
            {
                case 20707: spell_id =  3026; break;        // rank 1
                case 20762: spell_id = 20758; break;        // rank 2
                case 20763: spell_id = 20759; break;        // rank 3
                case 20764: spell_id = 20760; break;        // rank 4
                case 20765: spell_id = 20761; break;        // rank 5
                case 27239: spell_id = 27240; break;        // rank 6
                default:
                    sLog.outError("Unhandled spell %u: S.Resurrection", (*itr)->GetId());
                    continue;
            }

            prio = 3;
        }
        // Twisting Nether                                  // prio: 2 (max)
        else if ((*itr)->GetId() == 23701 && roll_chance_i(10))
        {
            prio = 2;
            spell_id = 23700;
        }
    }

    // Reincarnation (passive spell)                        // prio: 1
    if (prio < 1 && HasSpell(20608) && !HasSpellCooldown(21169) && HasItemCount(17030, EFFECT_INDEX_1))
        spell_id = 21169;

    return spell_id;
}

// Used in triggers for check "Only to targets that grant experience or honor" req
bool Player::isHonorOrXPTarget(Unit* pVictim) const
{
    uint32 v_level = pVictim->getLevel();
    uint32 k_grey  = MaNGOS::XP::GetGrayLevel(getLevel());

    // Victim level less gray level
    if (v_level <= k_grey)
        return false;

    if (pVictim->GetTypeId() == TYPEID_UNIT)
    {
        if (((Creature*)pVictim)->IsTotem() ||
                ((Creature*)pVictim)->IsPet() ||
                ((Creature*)pVictim)->GetCreatureInfo()->ExtraFlags & CREATURE_EXTRA_FLAG_NO_XP_AT_KILL)
            return false;
    }
    return true;
}

void Player::RewardSinglePlayerAtKill(Unit* pVictim)
{
    bool PvP = pVictim->isCharmedOwnedByPlayerOrPlayer();

    uint32 xp = PvP ? 0 : MaNGOS::XP::Gain(this, pVictim);

    // honor can be in PvP and !PvP (racial leader) cases
    RewardHonor(pVictim, 1);

    // xp and reputation only in !PvP case
    if (!PvP)
    {
        RewardReputation(pVictim, 1);
        GiveXP(xp, pVictim);

        if (Pet* pet = GetPet())
            pet->GivePetXP(xp);

        // normal creature (not pet/etc) can be only in !PvP case
        if (pVictim->GetTypeId() == TYPEID_UNIT)
            KilledMonster(((Creature*)pVictim)->GetCreatureInfo(), pVictim->GetObjectGuid());
    }
}

void Player::RewardPlayerAndGroupAtEvent(uint32 creature_id, WorldObject* pRewardSource)
{
    MANGOS_ASSERT((!GetGroup() || pRewardSource) && "Player::RewardPlayerAndGroupAtEvent called for Group-Case but no source for range searching provided");

    ObjectGuid creature_guid = pRewardSource && pRewardSource->GetTypeId() == TYPEID_UNIT ? pRewardSource->GetObjectGuid() : ObjectGuid();

    // prepare data for near group iteration
    if (Group* pGroup = GetGroup())
    {
        for (GroupReference* itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player* pGroupGuy = itr->getSource();
            if (!pGroupGuy)
                continue;

            if (!pGroupGuy->IsAtGroupRewardDistance(pRewardSource))
                continue;                                   // member (alive or dead) or his corpse at req. distance

            // quest objectives updated only for alive group member or dead but with not released body
            if (pGroupGuy->isAlive() || !pGroupGuy->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
                pGroupGuy->KilledMonsterCredit(creature_id, creature_guid);
        }
    }
    else                                                    // if (!pGroup)
        KilledMonsterCredit(creature_id, creature_guid);
}

void Player::RewardPlayerAndGroupAtCast(WorldObject* pRewardSource, uint32 spellid)
{
    // prepare data for near group iteration
    if (Group* pGroup = GetGroup())
    {
        for (GroupReference* itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player* pGroupGuy = itr->getSource();
            if (!pGroupGuy)
                continue;

            if (!pGroupGuy->IsAtGroupRewardDistance(pRewardSource))
                continue;                               // member (alive or dead) or his corpse at req. distance

            // quest objectives updated only for alive group member or dead but with not released body
            if (pGroupGuy->isAlive() || !pGroupGuy->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
                pGroupGuy->CastedCreatureOrGO(pRewardSource->GetEntry(), pRewardSource->GetObjectGuid(), spellid, pGroupGuy == this);
        }
    }
    else                                                    // if (!pGroup)
        CastedCreatureOrGO(pRewardSource->GetEntry(), pRewardSource->GetObjectGuid(), spellid);
}

bool Player::IsAtGroupRewardDistance(WorldObject const* pRewardSource) const
{
    if (pRewardSource->IsWithinDistInMap(this, sWorld.getConfig(CONFIG_FLOAT_GROUP_XP_DISTANCE)))
        return true;

    if (isAlive())
        return false;

    Corpse* corpse = GetCorpse();
    if (!corpse)
        return false;

    return pRewardSource->IsWithinDistInMap(corpse, sWorld.getConfig(CONFIG_FLOAT_GROUP_XP_DISTANCE));
}

uint32 Player::GetBaseWeaponSkillValue(WeaponAttackType attType) const
{
    Item* item = GetWeaponForAttack(attType, true, true);

    // unarmed only with base attack
    if (attType != BASE_ATTACK && !item)
        return 0;

    // weapon skill or (unarmed for base attack)
    uint32  skill = item ? item->GetSkill() : uint32(SKILL_UNARMED);
    return GetBaseSkillValue(skill);
}

void Player::ResurectUsingRequestData()
{
    /// Teleport before resurrecting by player, otherwise the player might get attacked from creatures near his corpse
    if (m_resurrectGuid.IsPlayer())
        TeleportTo(m_resurrectMap, m_resurrectX, m_resurrectY, m_resurrectZ, GetOrientation());

    // we cannot resurrect player when we triggered far teleport
    // player will be resurrected upon teleportation
    if (IsBeingTeleportedFar())
    {
        ScheduleDelayedOperation(DELAYED_RESURRECT_PLAYER);
        return;
    }

    ResurrectPlayer(0.0f, false);

    if (GetMaxHealth() > m_resurrectHealth)
        SetHealth(m_resurrectHealth);
    else
        SetHealth(GetMaxHealth());

    if (GetMaxPower(POWER_MANA) > m_resurrectMana)
        SetPower(POWER_MANA, m_resurrectMana);
    else
        SetPower(POWER_MANA, GetMaxPower(POWER_MANA));

    SetPower(POWER_RAGE, 0);

    SetPower(POWER_ENERGY, GetMaxPower(POWER_ENERGY));

    SpawnCorpseBones();
}

void Player::SetClientControl(Unit* target, uint8 allowMove)
{
    WorldPacket data(SMSG_CLIENT_CONTROL_UPDATE, target->GetPackGUID().size() + 1);
    data << target->GetPackGUID();
    data << uint8(allowMove);
    GetSession()->SendPacket(&data);
}

void Player::UpdateZoneDependentAuras()
{
    // Some spells applied at enter into zone (with subzones), aura removed in UpdateAreaDependentAuras that called always at zone->area update
    SpellAreaForAreaMapBounds saBounds = sSpellMgr.GetSpellAreaForAreaMapBounds(m_zoneUpdateId);
    for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        itr->second->ApplyOrRemoveSpellIfCan(this, m_zoneUpdateId, 0, true);
}

void Player::UpdateAreaDependentAuras()
{
    // remove auras from spells with area limitations
    for (SpellAuraHolderMap::iterator iter = m_spellAuraHolders.begin(); iter != m_spellAuraHolders.end();)
    {
        // use m_zoneUpdateId for speed: UpdateArea called from UpdateZone or instead UpdateZone in both cases m_zoneUpdateId up-to-date
        if (sSpellMgr.GetSpellAllowedInLocationError(iter->second->GetSpellProto(), GetMapId(), m_zoneUpdateId, m_areaUpdateId, this) != SPELL_CAST_OK)
        {
            RemoveSpellAuraHolder(iter->second);
            iter = m_spellAuraHolders.begin();
        }
        else
            ++iter;
    }

    // some auras applied at subzone enter
    SpellAreaForAreaMapBounds saBounds = sSpellMgr.GetSpellAreaForAreaMapBounds(m_areaUpdateId);
    for (SpellAreaForAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        itr->second->ApplyOrRemoveSpellIfCan(this, m_zoneUpdateId, m_areaUpdateId, true);
}

uint32 Player::GetCorpseReclaimDelay(bool pvp) const
{
    if ((pvp && !sWorld.getConfig(CONFIG_BOOL_DEATH_CORPSE_RECLAIM_DELAY_PVP)) ||
            (!pvp && !sWorld.getConfig(CONFIG_BOOL_DEATH_CORPSE_RECLAIM_DELAY_PVE)))
    {
        return corpseReclaimDelay[0];
    }

    time_t now = time(NULL);
    // 0..2 full period
    uint32 count = (now < m_deathExpireTime) ? uint32((m_deathExpireTime - now) / DEATH_EXPIRE_STEP) : 0;
    return corpseReclaimDelay[count];
}

void Player::UpdateCorpseReclaimDelay()
{
    bool pvp = m_ExtraFlags & PLAYER_EXTRA_PVP_DEATH;

    if ((pvp && !sWorld.getConfig(CONFIG_BOOL_DEATH_CORPSE_RECLAIM_DELAY_PVP)) ||
            (!pvp && !sWorld.getConfig(CONFIG_BOOL_DEATH_CORPSE_RECLAIM_DELAY_PVE)))
        return;

    time_t now = time(NULL);
    if (now < m_deathExpireTime)
    {
        // full and partly periods 1..3
        uint32 count = uint32((m_deathExpireTime - now) / DEATH_EXPIRE_STEP + 1);
        if (count < MAX_DEATH_COUNT)
            m_deathExpireTime = now + (count + 1) * DEATH_EXPIRE_STEP;
        else
            m_deathExpireTime = now + MAX_DEATH_COUNT * DEATH_EXPIRE_STEP;
    }
    else
        m_deathExpireTime = now + DEATH_EXPIRE_STEP;
}

void Player::SendCorpseReclaimDelay(bool load)
{
    Corpse* corpse = GetCorpse();
    if (!corpse)
        return;

    uint32 delay;
    if (load)
    {
        if (corpse->GetGhostTime() > m_deathExpireTime)
            return;

        bool pvp = corpse->GetType() == CORPSE_RESURRECTABLE_PVP;

        uint32 count;
        if ((pvp && sWorld.getConfig(CONFIG_BOOL_DEATH_CORPSE_RECLAIM_DELAY_PVP)) ||
                (!pvp && sWorld.getConfig(CONFIG_BOOL_DEATH_CORPSE_RECLAIM_DELAY_PVE)))
        {
            count = uint32(m_deathExpireTime - corpse->GetGhostTime()) / DEATH_EXPIRE_STEP;
            if (count >= MAX_DEATH_COUNT)
                count = MAX_DEATH_COUNT - 1;
        }
        else
            count = 0;

        time_t expected_time = corpse->GetGhostTime() + corpseReclaimDelay[count];

        time_t now = time(NULL);
        if (now >= expected_time)
            return;

        delay = uint32(expected_time - now);
    }
    else
        delay = GetCorpseReclaimDelay(corpse->GetType() == CORPSE_RESURRECTABLE_PVP);

    //! corpse reclaim delay 30 * 1000ms or longer at often deaths
    WorldPacket data(SMSG_CORPSE_RECLAIM_DELAY, 4);
    data << uint32(delay * IN_MILLISECONDS);
    GetSession()->SendPacket(&data);
}

Player* Player::GetNextRandomRaidMember(float radius)
{
    Group* pGroup = GetGroup();
    if (!pGroup)
        return NULL;

    std::vector<Player*> nearMembers;
    nearMembers.reserve(pGroup->GetMembersCount());

    for (GroupReference* itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player* Target = itr->getSource();

        // IsHostileTo check duel and controlled by enemy
        if (Target && Target != this && IsWithinDistInMap(Target, radius) &&
                !Target->HasInvisibilityAura() && !IsHostileTo(Target))
            nearMembers.push_back(Target);
    }

    if (nearMembers.empty())
        return NULL;

    uint32 randTarget = urand(0, nearMembers.size() - 1);
    return nearMembers[randTarget];
}

PartyResult Player::CanUninviteFromGroup() const
{
    const Group* grp = GetGroup();
    if (!grp)
        return ERR_NOT_IN_GROUP;

    if (!grp->IsLeader(GetObjectGuid()) && !grp->IsAssistant(GetObjectGuid()))
        return ERR_NOT_LEADER;

    if (InBattleGround())
        return ERR_INVITE_RESTRICTED;

    return ERR_PARTY_RESULT_OK;
}

void Player::SetBattleGroundRaid(Group* group, int8 subgroup)
{
    // we must move references from m_group to m_originalGroup
    SetOriginalGroup(GetGroup(), GetSubGroup());

    m_group.unlink();
    m_group.link(group, this);
    m_group.setSubGroup((uint8)subgroup);
}

void Player::RemoveFromBattleGroundRaid()
{
    // remove existing reference
    m_group.unlink();
    if (Group* group = GetOriginalGroup())
    {
        m_group.link(group, this);
        m_group.setSubGroup(GetOriginalSubGroup());
    }
    SetOriginalGroup(NULL);
}

void Player::SetOriginalGroup(Group* group, int8 subgroup)
{
    if (group == NULL)
        m_originalGroup.unlink();
    else
    {
        // never use SetOriginalGroup without a subgroup unless you specify NULL for group
        MANGOS_ASSERT(subgroup >= 0);
        m_originalGroup.link(group, this);
        m_originalGroup.setSubGroup((uint8)subgroup);
    }
}

void Player::UpdateUnderwaterState(Map* m, float x, float y, float z)
{
    GridMapLiquidData liquid_status;
    GridMapLiquidStatus res = m->GetTerrain()->getLiquidStatus(x, y, z, MAP_ALL_LIQUIDS, &liquid_status);
    if (!res)
    {
        m_MirrorTimerFlags &= ~(UNDERWATER_INWATER | UNDERWATER_INLAVA | UNDERWATER_INSLIME | UNDERWATER_INDARKWATER);
        if (m_lastLiquid && m_lastLiquid->SpellId)
            RemoveAurasDueToSpell(m_lastLiquid->SpellId);
        m_lastLiquid = NULL;
        return;
    }

    if (uint32 liqEntry = liquid_status.entry)
    {
        LiquidTypeEntry const* liquid = sLiquidTypeStore.LookupEntry(liqEntry);
        if (m_lastLiquid && m_lastLiquid->SpellId && m_lastLiquid->Id != liqEntry)
            RemoveAurasDueToSpell(m_lastLiquid->SpellId);

        if (liquid && liquid->SpellId)
        {
            if (res & (LIQUID_MAP_UNDER_WATER | LIQUID_MAP_IN_WATER))
            {
                if (!HasAura(liquid->SpellId))
                    CastSpell(this, liquid->SpellId, true);
            }
            else
                RemoveAurasDueToSpell(liquid->SpellId);
        }

        m_lastLiquid = liquid;
    }
    else if (m_lastLiquid && m_lastLiquid->SpellId)
    {
        RemoveAurasDueToSpell(m_lastLiquid->SpellId);
        m_lastLiquid = NULL;
    }

    // All liquids type - check under water position
    if (liquid_status.type_flags & (MAP_LIQUID_TYPE_WATER | MAP_LIQUID_TYPE_OCEAN | MAP_LIQUID_TYPE_MAGMA | MAP_LIQUID_TYPE_SLIME))
    {
        if (res & LIQUID_MAP_UNDER_WATER)
            m_MirrorTimerFlags |= UNDERWATER_INWATER;
        else
            m_MirrorTimerFlags &= ~UNDERWATER_INWATER;
    }

    // Allow travel in dark water on taxi or transport
    if ((liquid_status.type_flags & MAP_LIQUID_TYPE_DARK_WATER) && !IsTaxiFlying() && !GetTransport())
        m_MirrorTimerFlags |= UNDERWATER_INDARKWATER;
    else
        m_MirrorTimerFlags &= ~UNDERWATER_INDARKWATER;

    // in lava check, anywhere in lava level
    if (liquid_status.type_flags & MAP_LIQUID_TYPE_MAGMA)
    {
        if (res & (LIQUID_MAP_UNDER_WATER | LIQUID_MAP_IN_WATER | LIQUID_MAP_WATER_WALK))
            m_MirrorTimerFlags |= UNDERWATER_INLAVA;
        else
            m_MirrorTimerFlags &= ~UNDERWATER_INLAVA;
    }
    // in slime check, anywhere in slime level
    if (liquid_status.type_flags & MAP_LIQUID_TYPE_SLIME)
    {
        if (res & (LIQUID_MAP_UNDER_WATER | LIQUID_MAP_IN_WATER | LIQUID_MAP_WATER_WALK))
            m_MirrorTimerFlags |= UNDERWATER_INSLIME;
        else
            m_MirrorTimerFlags &= ~UNDERWATER_INSLIME;
    }
}

void Player::SetCanParry(bool value)
{
    if (m_canParry == value)
        return;

    m_canParry = value;
    UpdateParryPercentage();
}

void Player::SetCanBlock(bool value)
{
    if (m_canBlock == value)
        return;

    m_canBlock = value;
    UpdateBlockPercentage();
}

bool ItemPosCount::isContainedIn(ItemPosCountVec const& vec) const
{
    for (ItemPosCountVec::const_iterator itr = vec.begin(); itr != vec.end(); ++itr)
        if (itr->pos == pos)
            return true;

    return false;
}

bool Player::CanUseBattleGroundObject()
{
    // TODO : some spells gives player ForceReaction to one faction (ReputationMgr::ApplyForceReaction)
    // maybe gameobject code should handle that ForceReaction usage
    return (isAlive() &&                                    // living
            // the following two are incorrect, because invisible/stealthed players should get visible when they click on flag
            !HasStealthAura() &&                            // not stealthed
            !HasInvisibilityAura() &&                       // visible
            !isTotalImmune());                              // vulnerable (not immune)
}

bool Player::isTotalImmune()
{
    AuraList const& immune = GetAurasByType(SPELL_AURA_SCHOOL_IMMUNITY);

    uint32 immuneMask = 0;
    for (AuraList::const_iterator itr = immune.begin(); itr != immune.end(); ++itr)
    {
        immuneMask |= (*itr)->GetModifier()->m_miscvalue;
        if (immuneMask & SPELL_SCHOOL_MASK_ALL)             // total immunity
            return true;
    }
    return false;
}

void Player::AutoStoreLoot(WorldObject const* lootTarget, uint32 loot_id, LootStore const& store, bool broadcast, uint8 bag, uint8 slot)
{
    Loot loot(lootTarget);
    loot.FillLoot(loot_id, store, this, true);

    AutoStoreLoot(loot, broadcast, bag, slot);
}

void Player::AutoStoreLoot(Loot& loot, bool broadcast, uint8 bag, uint8 slot)
{
    uint32 max_slot = loot.GetMaxSlotInLootFor(this);
    for (uint32 i = 0; i < max_slot; ++i)
    {
        LootItem* lootItem = loot.LootItemInSlot(i, this);

        ItemPosCountVec dest;
        InventoryResult msg = CanStoreNewItem(bag, slot, dest, lootItem->itemid, lootItem->count);
        if (msg != EQUIP_ERR_OK && slot != NULL_SLOT)
            msg = CanStoreNewItem(bag, NULL_SLOT, dest, lootItem->itemid, lootItem->count);
        if (msg != EQUIP_ERR_OK && bag != NULL_BAG)
            msg = CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, lootItem->itemid, lootItem->count);
        if (msg != EQUIP_ERR_OK)
        {
            SendEquipError(msg, NULL, NULL, lootItem->itemid);
            continue;
        }

        Item* pItem = StoreNewItem(dest, lootItem->itemid, true, lootItem->randomPropertyId);
        SendNewItem(pItem, lootItem->count, false, false, broadcast);
    }
}

Item* Player::ConvertItem(Item* item, uint32 newItemId)
{
    uint16 pos = item->GetPos();

    Item* pNewItem = Item::CreateItem(newItemId, 1, this);
    if (!pNewItem)
        return NULL;

    // copy enchantments
    for (uint8 j = PERM_ENCHANTMENT_SLOT; j <= TEMP_ENCHANTMENT_SLOT; ++j)
    {
        if (item->GetEnchantmentId(EnchantmentSlot(j)))
            pNewItem->SetEnchantment(EnchantmentSlot(j), item->GetEnchantmentId(EnchantmentSlot(j)),
                                     item->GetEnchantmentDuration(EnchantmentSlot(j)), item->GetEnchantmentCharges(EnchantmentSlot(j)));
    }

    // copy durability
    if (item->GetUInt32Value(ITEM_FIELD_DURABILITY) < item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY))
    {
        double loosePercent = 1 - item->GetUInt32Value(ITEM_FIELD_DURABILITY) / double(item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY));
        DurabilityLoss(pNewItem, loosePercent);
    }

    if (IsInventoryPos(pos))
    {
        ItemPosCountVec dest;
        InventoryResult msg = CanStoreItem(item->GetBagSlot(), item->GetSlot(), dest, pNewItem, true);
        // ignore cast/combat time restriction
        if (msg == EQUIP_ERR_OK)
        {
            DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
            return StoreItem(dest, pNewItem, true);
        }
    }
    else if (IsBankPos(pos))
    {
        ItemPosCountVec dest;
        InventoryResult msg = CanBankItem(item->GetBagSlot(), item->GetSlot(), dest, pNewItem, true);
        // ignore cast/combat time restriction
        if (msg == EQUIP_ERR_OK)
        {
            DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
            return BankItem(dest, pNewItem, true);
        }
    }
    else if (IsEquipmentPos(pos))
    {
        uint16 dest;
        InventoryResult msg = CanEquipItem(item->GetSlot(), dest, pNewItem, true, false);
        // ignore cast/combat time restriction
        if (msg == EQUIP_ERR_OK)
        {
            DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
            pNewItem = EquipItem(dest, pNewItem, true);
            AutoUnequipOffhandIfNeed();
            return pNewItem;
        }
    }

    // fail
    delete pNewItem;
    return NULL;
}

uint32 Player::CalculateTalentsPoints() const
{
    uint32 talentPointsForLevel = getLevel() < 10 ? 0 : getLevel() - 9;
    return uint32(talentPointsForLevel * sWorld.getConfig(CONFIG_FLOAT_RATE_TALENT));
}

struct DoPlayerLearnSpell
{
    DoPlayerLearnSpell(Player& _player) : player(_player) {}
    void operator()(uint32 spell_id) { player.learnSpell(spell_id, false); }
    Player& player;
};

void Player::learnSpellHighRank(uint32 spellid)
{
    learnSpell(spellid, false);

    DoPlayerLearnSpell worker(*this);
    sSpellMgr.doForHighRanks(spellid, worker);
}

void Player::_LoadSkills(QueryResult* result)
{
    //                                                           0      1      2
    // SetPQuery(PLAYER_LOGIN_QUERY_LOADSKILLS,          "SELECT skill, value, max FROM character_skills WHERE guid = '%u'", GUID_LOPART(m_guid));

    uint32 count = 0;
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            uint16 skill    = fields[0].GetUInt16();
            uint16 value    = fields[1].GetUInt16();
            uint16 max      = fields[2].GetUInt16();

            SkillLineEntry const* pSkill = sSkillLineStore.LookupEntry(skill);
            if (!pSkill)
            {
                sLog.outError("Character %u has skill %u that does not exist.", GetGUIDLow(), skill);
                continue;
            }

            // set fixed skill ranges
            switch (GetSkillRangeType(pSkill, false))
            {
                case SKILL_RANGE_LANGUAGE:                  // 300..300
                    value = max = 300;
                    break;
                case SKILL_RANGE_MONO:                      // 1..1, grey monolite bar
                    value = max = 1;
                    break;
                default:
                    break;
            }

            if (value == 0)
            {
                sLog.outError("Character %u has skill %u with value 0. Will be deleted.", GetGUIDLow(), skill);
                CharacterDatabase.PExecute("DELETE FROM character_skills WHERE guid = '%u' AND skill = '%u' ", GetGUIDLow(), skill);
                continue;
            }

            SetUInt32Value(PLAYER_SKILL_INDEX(count), MAKE_PAIR32(skill, 0));
            SetUInt32Value(PLAYER_SKILL_VALUE_INDEX(count), MAKE_SKILL_VALUE(value, max));
            SetUInt32Value(PLAYER_SKILL_BONUS_INDEX(count), 0);

            mSkillStatus.insert(SkillStatusMap::value_type(skill, SkillStatusData(count, SKILL_UNCHANGED)));

            learnSkillRewardedSpells(skill, value);

            ++count;

            if (count >= PLAYER_MAX_SKILLS)                 // client limit
            {
                sLog.outError("Character %u has more than %u skills.", GetGUIDLow(), PLAYER_MAX_SKILLS);
                break;
            }
        }
        while (result->NextRow());
        delete result;
    }

    for (; count < PLAYER_MAX_SKILLS; ++count)
    {
        SetUInt32Value(PLAYER_SKILL_INDEX(count), 0);
        SetUInt32Value(PLAYER_SKILL_VALUE_INDEX(count), 0);
        SetUInt32Value(PLAYER_SKILL_BONUS_INDEX(count), 0);
    }
}

InventoryResult Player::CanEquipUniqueItem(Item* pItem, uint8 eslot) const
{
    ItemPrototype const* pProto = pItem->GetProto();

    // proto based limitations
    if (InventoryResult res = CanEquipUniqueItem(pProto, eslot))
        return res;

    return EQUIP_ERR_OK;
}

InventoryResult Player::CanEquipUniqueItem(ItemPrototype const* itemProto, uint8 except_slot) const
{
    // check unique-equipped on item
    if (itemProto->Flags & ITEM_FLAG_UNIQUE_EQUIPPED)
    {
        // there is an equip limit on this item
        if (HasItemWithIdEquipped(itemProto->ItemId, 1, except_slot))
            return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;
    }

    return EQUIP_ERR_OK;
}

void Player::HandleFall(MovementInfo const& movementInfo)
{
    // calculate total z distance of the fall
    float z_diff = m_lastFallZ - movementInfo.GetPos()->z;
    DEBUG_LOG("zDiff = %f", z_diff);

    // Players with low fall distance, Feather Fall or physical immunity (charges used) are ignored
    // 14.57 can be calculated by resolving damageperc formula below to 0
    if (z_diff >= 14.57f && !isDead() && !isGameMaster() && !HasMovementFlag(MOVEFLAG_ONTRANSPORT) &&
            !HasAuraType(SPELL_AURA_HOVER) && !HasAuraType(SPELL_AURA_FEATHER_FALL) &&
            !IsImmunedToDamage(SPELL_SCHOOL_MASK_NORMAL))
    {
        // Safe fall, fall height reduction
        int32 safe_fall = GetTotalAuraModifier(SPELL_AURA_SAFE_FALL);

        float damageperc = 0.018f * (z_diff - safe_fall) - 0.2426f;

        if (damageperc > 0)
        {
            uint32 damage = (uint32)(damageperc * GetMaxHealth() * sWorld.getConfig(CONFIG_FLOAT_RATE_DAMAGE_FALL));

            float height = movementInfo.GetPos()->z;
            UpdateAllowedPositionZ(movementInfo.GetPos()->x, movementInfo.GetPos()->y, height);

            if (damage > 0)
            {
                // Prevent fall damage from being more than the player maximum health
                if (damage > GetMaxHealth())
                    damage = GetMaxHealth();

                // Gust of Wind
                if (GetDummyAura(43621))
                    damage = GetMaxHealth() / 2;

                EnvironmentalDamage(DAMAGE_FALL, damage);
            }

            // Z given by moveinfo, LastZ, FallTime, WaterZ, MapZ, Damage, Safefall reduction
            DEBUG_LOG("FALLDAMAGE z=%f sz=%f pZ=%f FallTime=%d mZ=%f damage=%d SF=%d" , movementInfo.GetPos()->z, height, GetPositionZ(), movementInfo.GetFallTime(), height, damage, safe_fall);
        }
    }
}

void Player::LearnTalent(uint32 talentId, uint32 talentRank)
{
    uint32 CurTalentPoints = GetFreeTalentPoints();

    if (CurTalentPoints == 0)
        return;

    if (talentRank >= MAX_TALENT_RANK)
        return;

    TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentId);

    if (!talentInfo)
        return;

    TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);

    if (!talentTabInfo)
        return;

    // prevent learn talent for different class (cheating)
    if ((getClassMask() & talentTabInfo->ClassMask) == 0)
        return;

    // find current max talent rank
    uint32 curtalent_maxrank = 0;
    for (int32 k = MAX_TALENT_RANK - 1; k > -1; --k)
    {
        if (talentInfo->RankID[k] && HasSpell(talentInfo->RankID[k]))
        {
            curtalent_maxrank = k + 1;
            break;
        }
    }

    // we already have same or higher talent rank learned
    if (curtalent_maxrank >= (talentRank + 1))
        return;

    // check if we have enough talent points
    if (CurTalentPoints < (talentRank - curtalent_maxrank + 1))
        return;

    // Check if it requires another talent
    if (talentInfo->DependsOn > 0)
    {
        if (TalentEntry const* depTalentInfo = sTalentStore.LookupEntry(talentInfo->DependsOn))
        {
            bool hasEnoughRank = false;
            for (int i = talentInfo->DependsOnRank; i < MAX_TALENT_RANK; ++i)
            {
                if (depTalentInfo->RankID[i] != 0)
                    if (HasSpell(depTalentInfo->RankID[i]))
                        hasEnoughRank = true;
            }

            if (!hasEnoughRank)
                return;
        }
    }

    // Check if it requires spell
    if (talentInfo->DependsOnSpell && !HasSpell(talentInfo->DependsOnSpell))
        return;

    // Find out how many points we have in this field
    uint32 spentPoints = 0;

    uint32 tTab = talentInfo->TalentTab;
    if (talentInfo->Row > 0)
    {
        unsigned int numRows = sTalentStore.GetNumRows();
        for (unsigned int i = 0; i < numRows; ++i)          // Loop through all talents.
        {
            // Someday, someone needs to revamp
            const TalentEntry* tmpTalent = sTalentStore.LookupEntry(i);
            if (tmpTalent)                                  // the way talents are tracked
            {
                if (tmpTalent->TalentTab == tTab)
                {
                    for (int j = 0; j < MAX_TALENT_RANK; ++j)
                    {
                        if (tmpTalent->RankID[j] != 0)
                        {
                            if (HasSpell(tmpTalent->RankID[j]))
                            {
                                spentPoints += j + 1;
                            }
                        }
                    }
                }
            }
        }
    }

    // not have required min points spent in talent tree
    if (spentPoints < (talentInfo->Row * MAX_TALENT_RANK))
        return;

    // spell not set in talent.dbc
    uint32 spellid = talentInfo->RankID[talentRank];
    if (spellid == 0)
    {
        sLog.outError("Talent.dbc have for talent: %u Rank: %u spell id = 0", talentId, talentRank);
        return;
    }

    // already known
    if (HasSpell(spellid))
        return;

    // learn! (other talent ranks will unlearned at learning)
    learnSpell(spellid, false);
    DETAIL_LOG("TalentID: %u Rank: %u Spell: %u\n", talentId, talentRank, spellid);
}

void Player::UpdateFallInformationIfNeed(MovementInfo const& minfo, uint16 opcode)
{
    if (m_lastFallTime >= minfo.GetFallTime() || m_lastFallZ <= minfo.GetPos()->z || opcode == MSG_MOVE_FALL_LAND)
        SetFallInformation(minfo.GetFallTime(), minfo.GetPos()->z);
}

void Player::UnsummonPetTemporaryIfAny()
{
    Pet* pet = GetPet();
    if (!pet)
        return;

    if (!m_temporaryUnsummonedPetNumber && pet->isControlled() && !pet->isTemporarySummoned())
        m_temporaryUnsummonedPetNumber = pet->GetCharmInfo()->GetPetNumber();

    pet->Unsummon(PET_SAVE_AS_CURRENT, this);
}

void Player::ResummonPetTemporaryUnSummonedIfAny()
{
    if (!m_temporaryUnsummonedPetNumber)
        return;

    // not resummon in not appropriate state
    if (IsPetNeedBeTemporaryUnsummoned())
        return;

    if (GetPetGuid())
        return;

    Pet* NewPet = new Pet;
    if (!NewPet->LoadPetFromDB(this, 0, m_temporaryUnsummonedPetNumber, true))
        delete NewPet;

    m_temporaryUnsummonedPetNumber = 0;
}

void Player::_SaveBGData()
{
    // nothing save
    if (!m_bgData.m_needSave)
        return;

    static SqlStatementID delBGData ;
    static SqlStatementID insBGData ;

    SqlStatement stmt =  CharacterDatabase.CreateStatement(delBGData, "DELETE FROM character_battleground_data WHERE guid = ?");

    stmt.PExecute(GetGUIDLow());

    if (m_bgData.bgInstanceID)
    {
        stmt = CharacterDatabase.CreateStatement(insBGData, "INSERT INTO character_battleground_data VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
        /* guid, bgInstanceID, bgTeam, x, y, z, o, map */
        stmt.addUInt32(GetGUIDLow());
        stmt.addUInt32(m_bgData.bgInstanceID);
        stmt.addUInt32(uint32(m_bgData.bgTeam));
        stmt.addFloat(m_bgData.joinPos.coord_x);
        stmt.addFloat(m_bgData.joinPos.coord_y);
        stmt.addFloat(m_bgData.joinPos.coord_z);
        stmt.addFloat(m_bgData.joinPos.orientation);
        stmt.addUInt32(m_bgData.joinPos.mapid);

        stmt.Execute();
    }

    m_bgData.m_needSave = false;
}

void Player::ModifyMoney(int32 d)
{
    // used by eluna
    sHookMgr->OnMoneyChanged(this, d);

    if (d < 0)
        SetMoney(GetMoney() > uint32(-d) ? GetMoney() + d : 0);
    else
        SetMoney(GetMoney() < uint32(MAX_MONEY_AMOUNT - d) ? GetMoney() + d : MAX_MONEY_AMOUNT);

}

void Player::RemoveAtLoginFlag(AtLoginFlags f, bool in_db_also /*= false*/)
{
    m_atLoginFlags &= ~f;

    if (in_db_also)
        CharacterDatabase.PExecute("UPDATE characters set at_login = at_login & ~ %u WHERE guid ='%u'", uint32(f), GetGUIDLow());
}

void Player::SendClearCooldown(uint32 spell_id, Unit* target)
{
    WorldPacket data(SMSG_CLEAR_COOLDOWN, 4 + 8);
    data << uint32(spell_id);
    data << target->GetObjectGuid();
    SendDirectMessage(&data);
}

void Player::BuildTeleportAckMsg(WorldPacket& data, float x, float y, float z, float ang) const
{
    MovementInfo mi = m_movementInfo;
    mi.ChangePosition(x, y, z, ang);

    data.Initialize(MSG_MOVE_TELEPORT_ACK, 41);
    data << GetPackGUID();
    data << uint32(0);                                      // this value increments every time
    data << mi;
}

bool Player::HasMovementFlag(MovementFlags f) const
{
    return m_movementInfo.HasMovementFlag(f);
}

void Player::SetHomebindToLocation(WorldLocation const& loc, uint32 area_id)
{
    m_homebindMapId = loc.mapid;
    m_homebindAreaId = area_id;
    m_homebindX = loc.coord_x;
    m_homebindY = loc.coord_y;
    m_homebindZ = loc.coord_z;

    // update sql homebind
    CharacterDatabase.PExecute("UPDATE character_homebind SET map = '%u', zone = '%u', position_x = '%f', position_y = '%f', position_z = '%f' WHERE guid = '%u'",
                               m_homebindMapId, m_homebindAreaId, m_homebindX, m_homebindY, m_homebindZ, GetGUIDLow());
}

Object* Player::GetObjectByTypeMask(ObjectGuid guid, TypeMask typemask)
{
    switch (guid.GetHigh())
    {
        case HIGHGUID_ITEM:
            if (typemask & TYPEMASK_ITEM)
                return GetItemByGuid(guid);
            break;
        case HIGHGUID_PLAYER:
            if (GetObjectGuid() == guid)
                return this;
            if ((typemask & TYPEMASK_PLAYER) && IsInWorld())
                return ObjectAccessor::FindPlayer(guid);
            break;
        case HIGHGUID_GAMEOBJECT:
            if ((typemask & TYPEMASK_GAMEOBJECT) && IsInWorld())
                return GetMap()->GetGameObject(guid);
            break;
        case HIGHGUID_UNIT:
            if ((typemask & TYPEMASK_UNIT) && IsInWorld())
                return GetMap()->GetCreature(guid);
            break;
        case HIGHGUID_PET:
            if ((typemask & TYPEMASK_UNIT) && IsInWorld())
                return GetMap()->GetPet(guid);
            break;
        case HIGHGUID_DYNAMICOBJECT:
            if ((typemask & TYPEMASK_DYNAMICOBJECT) && IsInWorld())
                return GetMap()->GetDynamicObject(guid);
            break;
        case HIGHGUID_TRANSPORT:
        case HIGHGUID_CORPSE:
        case HIGHGUID_MO_TRANSPORT:
            break;
    }

    return NULL;
}

void Player::SetRestType(RestType n_r_type, uint32 areaTriggerId /*= 0*/)
{
    rest_type = n_r_type;

    if (rest_type == REST_TYPE_NO)
    {
        RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);

        // Set player to FFA PVP when not in rested environment.
        if (sWorld.IsFFAPvPRealm())
            SetFFAPvP(true);
    }
    else
    {
        SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING);

        inn_trigger_id = areaTriggerId;
        time_inn_enter = time(NULL);

        if (sWorld.IsFFAPvPRealm())
            SetFFAPvP(false);
    }
}

void Player::SendDuelCountdown(uint32 counter)
{
    WorldPacket data(SMSG_DUEL_COUNTDOWN, 4);
    data << uint32(counter);                                // seconds
    GetSession()->SendPacket(&data);
}

bool Player::IsImmuneToSpellEffect(SpellEntry const* spellInfo, SpellEffectIndex index, bool castOnSelf) const
{
    switch (spellInfo->Effect[index])
    {
        case SPELL_EFFECT_ATTACK_ME:
            return true;
        default:
            break;
    }
    switch (spellInfo->EffectApplyAuraName[index])
    {
        case SPELL_AURA_MOD_TAUNT:
            return true;
        default:
            break;
    }
    return Unit::IsImmuneToSpellEffect(spellInfo, index, castOnSelf);
}

void Player::KnockBackFrom(Unit* target, float horizontalSpeed, float verticalSpeed)
{
    float angle = this == target ? GetOrientation() + M_PI_F : target->GetAngle(this);
    GetSession()->SendKnockBack(angle, horizontalSpeed, verticalSpeed);
}

AreaLockStatus Player::GetAreaTriggerLockStatus(AreaTrigger const* at, uint32& miscRequirement)
{
    miscRequirement = 0;

    if (!at)
        return AREA_LOCKSTATUS_UNKNOWN_ERROR;

    MapEntry const* mapEntry = sMapStore.LookupEntry(at->target_mapId);
    if (!mapEntry)
        return AREA_LOCKSTATUS_UNKNOWN_ERROR;

    // Gamemaster can always enter
    if (isGameMaster())
        return AREA_LOCKSTATUS_OK;

    // Level Requirements
    if (getLevel() < at->requiredLevel && !sWorld.getConfig(CONFIG_BOOL_INSTANCE_IGNORE_LEVEL))
    {
        miscRequirement = at->requiredLevel;
        return AREA_LOCKSTATUS_TOO_LOW_LEVEL;
    }

    // Raid Requirements
    if (mapEntry->IsRaid() && !sWorld.getConfig(CONFIG_BOOL_INSTANCE_IGNORE_RAID))
        if (!GetGroup() || !GetGroup()->isRaidGroup())
            return AREA_LOCKSTATUS_RAID_LOCKED;

    // Item Requirements: must have requiredItem OR requiredItem2, report the first one that's missing
    if (at->requiredItem)
    {
        if (!HasItemCount(at->requiredItem, 1) &&
                (!at->requiredItem2 || !HasItemCount(at->requiredItem2, 1)))
        {
            miscRequirement = at->requiredItem;
            return AREA_LOCKSTATUS_MISSING_ITEM;
        }
    }
    else if (at->requiredItem2 && !HasItemCount(at->requiredItem2, 1))
    {
        miscRequirement = at->requiredItem2;
        return AREA_LOCKSTATUS_MISSING_ITEM;
    }

    // Quest Requirements
    if (at->requiredQuest && !GetQuestRewardStatus(at->requiredQuest))
    {
        miscRequirement = at->requiredQuest;
        return AREA_LOCKSTATUS_QUEST_NOT_COMPLETED;
    }

    // If the map is not created, assume it is possible to enter it.
    DungeonPersistentState* state = GetBoundInstanceSaveForSelfOrGroup(at->target_mapId);
    Map* map = sMapMgr.FindMap(at->target_mapId, state ? state->GetInstanceId() : 0);

    // Map's state check
    if (map && map->IsDungeon())
    {
        // cannot enter if the instance is full (player cap), GMs don't count
        if (((DungeonMap*)map)->GetPlayersCountExceptGMs() >= ((DungeonMap*)map)->GetMaxPlayers())
            return AREA_LOCKSTATUS_INSTANCE_IS_FULL;

        // In Combat check
        if (map && map->GetInstanceData() && map->GetInstanceData()->IsEncounterInProgress())
            return AREA_LOCKSTATUS_ZONE_IN_COMBAT;

        // Bind Checks
        InstancePlayerBind* pBind = GetBoundInstance(at->target_mapId);
        if (pBind && pBind->perm && pBind->state != state)
            return AREA_LOCKSTATUS_HAS_BIND;
        if (pBind && pBind->perm && pBind->state != map->GetPersistentState())
            return AREA_LOCKSTATUS_HAS_BIND;
    }

    return AREA_LOCKSTATUS_OK;
};
