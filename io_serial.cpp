////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL v3.
// For full terms see COPYING or visit https://www.gnu.org/licenses/gpl.html

////////////////////////////////////////////////////////////////////////////////
#include "firmata/io_serial.hpp"

#include <asio.hpp>
#include <thread>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{
namespace io
{

////////////////////////////////////////////////////////////////////////////////
serial::serial(asio::io_service& io, const std::string& device) :
    port_(io, device)
{ }

////////////////////////////////////////////////////////////////////////////////
void serial::set(baud_rate    baud) { port_.set_option(asio::serial_port::baud_rate(baud)); }
void serial::set(flow_control flow) { port_.set_option(asio::serial_port::flow_control(flow)); }
void serial::set(parity       pari) { port_.set_option(asio::serial_port::parity(pari)); }
void serial::set(stop_bits    bits) { port_.set_option(asio::serial_port::stop_bits(bits)); }
void serial::set(char_size    bits) { port_.set_option(asio::serial_port::character_size(bits)); }

////////////////////////////////////////////////////////////////////////////////
void serial::write(msg_id id, const payload& data)
{
    std::vector<asio::const_buffer> message;

        message.push_back(asio::buffer(&id, size(id)));
    if(data.size())
        message.push_back(asio::buffer(data));
    if(is_sysex(id))
        message.push_back(asio::buffer(&end_sysex, sizeof(end_sysex)));

    asio::write(port_, message);

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(4ms);
}

////////////////////////////////////////////////////////////////////////////////
std::tuple<msg_id, payload> serial::read()
{
    msg_id id = no_id;
    payload message;

    auto saved = fn_;
    reset_async([&](msg_id _id, const payload& _message)
    {
        id = _id; message = _message;
    });

    do port_.get_io_service().run_one(); while(id == no_id);
    reset_async(saved);

    return std::make_tuple(id, message);
}

////////////////////////////////////////////////////////////////////////////////
void serial::reset_async(callback fn)
{
    if(fn_ && !fn)
    {
        asio::error_code ec;
        port_.cancel(ec);
    }
    if(!fn_ && fn) sched_async();

    fn_ = std::move(fn);
}

////////////////////////////////////////////////////////////////////////////////
void serial::sched_async()
{
    asio::async_read(port_, store_,
        asio::transfer_at_least(3),
        std::bind(&serial::async_read, this, std::placeholders::_1)
    );
}

////////////////////////////////////////////////////////////////////////////////
std::tuple<msg_id, payload> serial::parse_one()
{
    msg_id id = no_id;
    payload message;

    do
    {
        // need at least 3 bytes (minimum message)
        if(store_.size() < 3) break;

        auto begin = asio::buffers_begin(store_.data());
        auto end = asio::buffers_end(store_.data());
        auto ci = begin;

        // get message id
        id = static_cast<msg_id>(*ci++);

        // if this is a sysex message, get sysex id
        if(is_sysex(id))
        {
            id = sysex(*ci++);

            // if this is an extended sysex message, get extended id
            if(is_ext_sysex(id))
            {
                // need 2 bytes for extended id
                if(store_.size() < 4) break;

                id = ext_sysex(word(*ci++) + (word(*ci++) << 7));
            }

            // find end_sysex
            for(auto ci_end = ci; ci_end != end; ++ci_end)
                if(byte(*ci_end) == end_sysex)
                {
                    message.insert(message.end(), ci, ci_end);
                    store_.consume(ci_end - begin + 1);

                    break;
                }
        }
        else
        {
            // standard message
            message.insert(message.end(), ci, ci + 2);
            store_.consume(3);
        }
    }
    while(false);

    return std::make_tuple(id, message);
}

////////////////////////////////////////////////////////////////////////////////
void serial::async_read(const asio::error_code& ec)
{
    if(ec) return;

    while(store_.size() >= 3)
    {
        msg_id id;
        payload message;
        std::tie(id, message) = parse_one();

        // incomplete or no message
        if(message.empty()) break;

        if(fn_) fn_(id, message);
    }

    sched_async();
}

////////////////////////////////////////////////////////////////////////////////
}
}
