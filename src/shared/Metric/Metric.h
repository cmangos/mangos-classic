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

#ifndef MANGOSSERVER_METRIC_H
#define MANGOSSERVER_METRIC_H

#include <boost/any.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <thread>
#include <vector>

#include "Measurement.h"
#include "Common.h"

struct MetricConnectionInfo
{
    std::string hostname;
    int32 port;
    std::string database;
    std::string username;
    std::string password;
};

namespace metric
{
    class measurement
    {
        public:
            measurement(std::string name, std::function<bool()> condition = [] { return true; });
            measurement(std::string name, std::string key, boost::any value, std::function<bool()> condition = [] { return true; });
            measurement(std::string name, std::string key, boost::any value, std::map<std::string, std::string> tags, std::function<bool()> condition = [] { return true; });
            measurement(std::string name, std::map<std::string, std::string> tags, std::function<bool()> condition = [] { return true; });
            virtual ~measurement();

            void add_tag(std::string key, std::string value);
            void add_field(std::string key, boost::any value);
            const boost::any get_field(std::string key);

        protected:
            void set_condition(std::function<bool()> condition);

        private:
            std::string m_name;
            std::map<std::string, std::string> m_tags;
            std::map<std::string, boost::any> m_fields;
            std::chrono::system_clock::time_point m_timestamp;
            std::function<bool()> m_condition;
    };

    template <class precision>
    class duration : public measurement
    {
        public:
            duration(std::string name)
                : measurement(name), m_startTime(std::chrono::high_resolution_clock::now())
            {}

            duration(std::string name, std::map<std::string, std::string> tags)
                : measurement(name, tags), m_startTime(std::chrono::high_resolution_clock::now())
            {}

            duration(std::string name, std::map<std::string, std::string> tags, int64 threshold)
                : measurement(name, tags), m_startTime(std::chrono::high_resolution_clock::now())
            {
                auto condition = [&, threshold] {
                    auto value = get_field("duration");
                    if (value.empty())
                        return false;

                    auto duration = boost::any_cast<int64>(value);
                    return duration >= threshold;
                };

                set_condition(std::move(condition));
            }

            ~duration()
            {
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<precision>(endTime - m_startTime).count();

                add_field("duration", static_cast<int64>(duration));
            }

        private:
            std::chrono::high_resolution_clock::time_point m_startTime;
    };

    class metric
    {
        public:
            metric();
            ~metric();

            void initialize();
            static metric& instance();

            void reload_config();

            void report(std::string measurement, std::string key, boost::any value, std::map<std::string, std::string> tags = {});
            void report(std::string measurement, std::map<std::string, boost::any> fields, std::map<std::string, std::string> tags = {});

        private:
            boost::asio::io_service m_queueService;
            boost::asio::io_service m_writeService;

            std::unique_ptr<boost::asio::deadline_timer> m_sendTimer;
            std::unique_ptr<boost::asio::io_service::work> m_queueServiceWork;
            std::unique_ptr<boost::asio::io_service::work> m_writeServiceWork;
            std::thread m_queueServiceThread;
            std::thread m_writeServiceThread;

            bool m_enabled;
            MetricConnectionInfo m_connectionInfo;

            std::mutex m_queueWriteLock;
            std::vector<std::unique_ptr<Measurement>> m_measurementQueue;

            void schedule_timer();
            void prepare_send(const boost::system::error_code& ec);
            void send();
    };
}

#endif // MANGOSSERVER_METRIC_H
