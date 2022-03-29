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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <functional>

#include "Config/Config.h"
#include "Log.h"
#include "Metric.h"

metric::measurement::measurement(std::string name, std::function<bool()> condition)
    : m_name(name), m_condition(std::move(condition))
{
}

metric::measurement::measurement(std::string name, std::string key, boost::any value, std::function<bool()> condition)
    : m_name(name), m_condition(std::move(condition))
{
    add_field(key, value);
}

metric::measurement::measurement(std::string name, std::string key, boost::any value, std::map<std::string, std::string> tags, std::function<bool()> condition)
    : m_name(name), m_tags(tags), m_condition(std::move(condition))
{
    add_field(key, value);
}

metric::measurement::measurement(std::string name, std::map<std::string, std::string> tags, std::function<bool()> condition)
    : m_name(name), m_tags(tags), m_condition(std::move(condition))
{
}

metric::measurement::~measurement()
{
    if(m_condition())
        metric::instance().report(m_name, m_fields, m_tags);
}

void metric::measurement::add_tag(std::string key, std::string value)
{
    m_tags.insert({key, value});
}

void metric::measurement::add_field(std::string key, boost::any value)
{
    m_fields.insert({key, value});
}

const boost::any metric::measurement::get_field(std::string key)
{
    if (m_fields.find(key) != m_fields.end())
        return m_fields[key];

    return boost::any();
}

void metric::measurement::set_condition(std::function<bool()> condition)
{
    m_condition = std::move(condition);
}

metric::metric::metric()
{
    initialize();
}

metric::metric::~metric()
{
    if (!m_enabled)
        return;

    m_writeService.post([&] {
        m_sendTimer->cancel();
    });

    m_queueServiceWork.reset();
    m_writeServiceWork.reset();

    m_queueServiceThread.join();
    m_writeServiceThread.join();
}

void metric::metric::initialize()
{
    if (!(m_enabled = sConfig.GetBoolDefault("Metric.Enable", false)))
        return;

    m_connectionInfo = {
        sConfig.GetStringDefault("Metric.Address", "127.0.0.1"),
        sConfig.GetIntDefault("Metric.Port", 8086),
        sConfig.GetStringDefault("Metric.Database", "perfd"),
        sConfig.GetStringDefault("Metric.Username", ""),
        sConfig.GetStringDefault("Metric.Password", "")
    };

    m_sendTimer.reset(new boost::asio::deadline_timer(m_writeService));
    m_queueServiceWork.reset(new boost::asio::io_service::work(m_queueService));
    m_writeServiceWork.reset(new boost::asio::io_service::work(m_writeService));

    // Start up service thread that will process all queued tasks
    m_queueServiceThread = std::thread([&] {
        m_queueService.run();
    });

    m_writeServiceThread = std::thread([&] {
        m_writeService.run();
    });

    schedule_timer();
}

metric::metric& metric::metric::instance()
{
    static metric instance;
    return instance;
}

void metric::metric::reload_config()
{
    if (!m_enabled)
    {
        initialize();
        return;
    }

    m_writeService.post([&]
    {
        m_connectionInfo = {
            sConfig.GetStringDefault("Metric.Address", "127.0.0.1"),
            sConfig.GetIntDefault("Metric.Port", 8086),
            sConfig.GetStringDefault("Metric.Database", "perfd"),
            sConfig.GetStringDefault("Metric.Username", ""),
            sConfig.GetStringDefault("Metric.Password", "")
        };
    });
}

void metric::metric::report(std::string measurement, std::string key, boost::any value, std::map<std::string, std::string> tags)
{
    report(measurement, { { key, value } }, tags);
}

void metric::metric::report(std::string measurement, std::map<std::string, boost::any> fields, std::map<std::string, std::string> tags)
{
    if (!m_enabled)
        return;

    m_queueService.post([&, measurement, fields, tags]
    {
        std::lock_guard<std::mutex> guard(m_queueWriteLock);
        m_measurementQueue.push_back(std::unique_ptr<Measurement>(new Measurement(measurement, tags, fields)));
    });
}

void metric::metric::schedule_timer()
{
    using namespace std::placeholders;

    if (!m_sendTimer)
        return;

    m_sendTimer->expires_from_now(boost::posix_time::seconds(1));
    m_sendTimer->async_wait(std::bind(&metric::metric::prepare_send, this, _1));
}

void metric::metric::prepare_send(const boost::system::error_code& ec)
{
    if (ec)
    {
        if (ec != boost::asio::error::operation_aborted)
            sLog.outError("metric::metric::handle_timeout aborted, %s", ec.message().c_str());

        return;
    }

    send();
    schedule_timer();
}

void metric::metric::send()
{
    std::vector<std::unique_ptr<Measurement>> measurements;

    // Scope swapping
    {
        std::lock_guard<std::mutex> guard(m_queueWriteLock);
        std::swap(measurements, m_measurementQueue);
    }

    sLog.outDetail("Sending %zu measurements!", measurements.size());

    using boost::asio::ip::tcp;

    boost::system::error_code error;

    // Hostname resolution
    tcp::resolver resolver(m_writeService);
    tcp::resolver::query query(m_connectionInfo.hostname, std::to_string(m_connectionInfo.port));
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, error);

    if (error)
    {
        sLog.outError("metric::metric::send resolve aborted, %s", error.message().c_str());
        return;
    }

    error = boost::asio::error::host_not_found;

    tcp::resolver::iterator end;

    tcp::socket socket(m_writeService);
    while (error && endpoint_iterator != end)
    {
        socket.close();
        socket.connect(*endpoint_iterator++, error);
    }

    if (error)
    {
        sLog.outError("metric::metric::send connect aborted, %s", error.message().c_str());
        return;
    }

    std::stringstream payload;
    for (auto const& measurement : measurements)
    {
        if (&measurement != &measurements.front())
            payload << "\n";

        payload << *measurement;
    }

    boost::asio::streambuf request;
    std::ostream request_stream(&request);

    // Write request
    request_stream << "POST " << "/write?db=" << m_connectionInfo.database << "&u=" << m_connectionInfo.username << "&p=" << m_connectionInfo.password << " HTTP/1.1\r\n";
    request_stream << "Host: " << m_connectionInfo.hostname << "\r\n";
    request_stream << "Content-Length:" << std::to_string(payload.tellp()) << "\r\n";
    request_stream << "Connection: close\r\n\r\n";
    request_stream << payload.rdbuf();

    // Send the request.
    boost::asio::write(socket, request, error);
    if (error)
    {
        sLog.outError("metric::metric::send write aborted, %s", error.message().c_str());
        return;
    }

    // Read the response status line. The response streambuf will automatically
    // grow to accommodate the entire line. The growth may be limited by passing
    // a maximum size to the streambuf constructor.
    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\r\n", error);
    if (error)
    {
        sLog.outError("metric::metric::send read_until aborted, %s", error.message().c_str());
        return;
    }

    // Check that response is OK.
    std::string http_version;
    unsigned int status_code;
    std::string status_message;

    std::istream response_stream(&response);
    response_stream >> http_version;
    response_stream >> status_code;
    std::getline(response_stream, status_message);

    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
    {
        sLog.outError("metric::metric::send received invalid response");
        return;
    }

    if (status_code < 200 || status_code >= 300)
    {
        // Should restore measurements back into queue

        sLog.outError("metric::metric::send response returned with status code %u", status_code);

        // Read the response headers, which are terminated by a blank line.
        // if this errors out its ok
        boost::asio::read_until(socket, response, "\r\n\r\n", error);

        // Process the response headers.
        std::string header;
        while (std::getline(response_stream, header) && header != "\r")
            std::cout << header << "\n";

        std::cout << "\n";

        // Write whatever content we already have to output.
        if (response.size() > 0)
            std::cout << &response;

        // Read until EOF, writing data to output as we go.
        while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
            std::cout << &response;

        if (error != boost::asio::error::eof)
            throw boost::system::system_error(error);

        return;
    }
}
