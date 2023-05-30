/*
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 *
 * This is private software and may not be shared under any circumstances,
 * absent permission of namreeb.
 */

#ifndef __ANTISPAM_HPP_
#define __ANTISPAM_HPP_

#include "Entities/ObjectGuid.h"
#include "Util/Timer.h"

#include "../priority.hpp"
#include "../cyclic.hpp"

#include <string>
#include <unordered_set>
#include <mutex>
#include <vector>
#include <memory>

class ChatHandler;

namespace NamreebAnticheat
{
enum SpamAction
{
    SPAM_ACTION_NONE = 0,
    SPAM_ACTION_SILENCE,
    SPAM_ACTION_BAN,
    SPAM_ACTION_MAX
};

class Antispam : public std::enable_shared_from_this<Antispam>
{
    private:
        const uint32 _account;
        const uint32 _creationTime;

        mutable std::mutex _mutex;

        uint32 _whisperCount;
        uint32 _sayCount;
        uint32 _yellCount;
        uint32 _channelCount;
        uint32 _mailCount;
        uint32 _channelInviteCount;
        uint32 _partyInviteCount;

        std::unordered_set<ObjectGuid> _whisperTargets;
        std::unordered_set<ObjectGuid> _mailTargets;
        std::unordered_set<ObjectGuid> _channelInviteTargets;
        std::unordered_set<ObjectGuid> _partyInviteTargets;

        // messages pending analysis
        std::vector<std::string> _pendingMessages;

        // log of the most recent messages
        nam::cyclic<std::string, 10> _recentMessages;

        // unique messages as determined by fuzzy string comparison
        std::vector<std::pair<uint32, std::string> > _uniqueMessages;

        // log of highest scoring blacklist messages, along with which blacklist entries contributed to the score.
        // this container is updated by analysis thread, so results are not available in real time.
        nam::priority<std::string, uint32, 5> _topBlacklistedMessages;

        // how many times during this session and previous recent sessions has the blacklist been violated
        uint32 _blacklistCount;

        // time of the last GM notification
        uint32 _lastNotification;

        // how many total messages have been sent out
        uint32 Total() const { return _whisperCount + _sayCount + _yellCount + _channelCount + _mailCount + _channelInviteCount; }

        // how many messages per minute are being sent, or zero if still under grace period
        float Rate() const;

        // send GM notification of possible spammer
        void Notify(const char *format, ...);

        // send GM notification of flagrant spammer and silence account (when automatic silencing is enabled)
        void Silence(const char *format, ...);

        // new message for analysis, assumes mutex is locked
        void NewMessage(const std::string &msg);

        // how far have they traveled, possibly zero if there has been no recent movement
        float GetDistanceTraveled() const;

        // computes repetition score, assumes mutex is locked
        uint32 RepetitionScore() const;

    public:
        Antispam(uint32 account);

        uint32 GetBlacklistCount() const { return _blacklistCount; }
        uint32 GetAccount() const { return _account; }
        std::string GetInfo() const;

        void AutoReply(const std::string &msg);
        void Whisper(const std::string &msg, const ObjectGuid &to);
        void Say(const std::string &msg);
        void Yell(const std::string &msg);
        void Channel(const std::string &msg);
        void Mail(const std::string &subject, const std::string &body, const ObjectGuid &to);
        void ChannelInvite(const std::string &channelName, const ObjectGuid &to);
        void PartyInvite(const ObjectGuid& to);

        // analyze results for this session to determine if spamming is taking place, and act accordingly
        void Analyze();
};
}

#endif /* !__ANTISPAM_HPP_ */
