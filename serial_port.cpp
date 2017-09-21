////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "firmata/serial_port.hpp"

#include <algorithm>
#include <iterator>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
serial_port::serial_port(asio::io_service& io, const std::string& device) :
    port_(io, device)
{ }

////////////////////////////////////////////////////////////////////////////////
void serial_port::set(baud_rate    baud) { port_.set_option(asio::serial_port::baud_rate(baud)); }
void serial_port::set(flow_control flow) { port_.set_option(asio::serial_port::flow_control(flow)); }
void serial_port::set(parity       pari) { port_.set_option(asio::serial_port::parity(pari)); }
void serial_port::set(stop_bits    bits) { port_.set_option(asio::serial_port::stop_bits(bits)); }
void serial_port::set(char_size    bits) { port_.set_option(asio::serial_port::character_size(bits)); }

////////////////////////////////////////////////////////////////////////////////
void serial_port::write(msg_id id, const payload& data)
{
    std::vector<asio::const_buffer> message;

        message.push_back(asio::buffer(&id, size(id)));
    if(data.size())
        message.push_back(asio::buffer(data));
    if(is_sysex(id))
        message.push_back(asio::buffer(&end_sysex, sizeof(end_sysex)));

    asio::write(port_, message);
}

////////////////////////////////////////////////////////////////////////////////
std::tuple<msg_id, payload> serial_port::read()
{
    msg_id id;
    payload data;

    do
    {
        // read into one buffer
        auto n = port_.read_some(asio::buffer(one_));

        // copy data into overall buffer
        overall_.insert(overall_.end(), std::begin(one_), std::next(one_, n));

        std::tie(id, data) = parse_one();
    }
    while(data.empty());

    return std::make_tuple(id, std::move(data));
}

////////////////////////////////////////////////////////////////////////////////
void serial_port::reset_async(callback fn)
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
void serial_port::sched_async()
{
    // read into one buffer
    port_.async_read_some(asio::buffer(one_),
        std::bind(&serial_port::async_read, this, std::placeholders::_1, std::placeholders::_2)
    );
}

////////////////////////////////////////////////////////////////////////////////
std::tuple<msg_id, payload> serial_port::parse_one()
{
    msg_id id;
    payload data;

    do
    {
        // discard garbage
        auto gi = std::find_if(overall_.begin(), overall_.end(),
            [](auto ch){ return ch >= 0x80 && ch != end_sysex; }
        );
        overall_.erase(overall_.begin(), gi);

        // check for minimum message size
        if(overall_.size() < 3) break;

        auto ci = overall_.begin();

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
                if(overall_.size() < 4) break;
                id = ext_sysex(word(*ci++) + (word(*ci++) << 7));
            }

            // find end_sysex
            for(auto ci_end = ci; ci_end != overall_.end(); ++ci_end)
                if(*ci_end == end_sysex)
                {
                    data.insert(data.end(), ci, ci_end);
                    overall_.erase(overall_.begin(), std::next(ci_end)); // chomp end_sysex
                    break;
                }
        }
        else
        {
            // standard message
            auto ci_end = ci + 2;
            data.insert(data.end(), ci, ci_end);
            overall_.erase(overall_.begin(), ci_end);
        }
    }
    while(false);

    return std::make_tuple(id, std::move(data));
}

////////////////////////////////////////////////////////////////////////////////
void serial_port::async_read(const asio::error_code& ec, std::size_t n)
{
    if(ec) return;

    // copy data into overall buffer
    overall_.insert(overall_.end(), std::begin(one_), std::next(one_, n));

    for(;;)
    {
        msg_id id;
        payload data;
        std::tie(id, data) = parse_one();

        if(data.empty()) break;

        if(fn_) fn_(id, data);
    }

    sched_async();
}

////////////////////////////////////////////////////////////////////////////////
}
