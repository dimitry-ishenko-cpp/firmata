////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL v3.
// For full terms see COPYING or visit https://www.gnu.org/licenses/gpl.html

////////////////////////////////////////////////////////////////////////////////
#include "command.hpp"

#include <algorithm>
#include <cassert>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
namespace
{

inline auto& coerce(pin& p) noexcept
{ return reinterpret_cast<pin_base&>(p); }

}

////////////////////////////////////////////////////////////////////////////////
protocol command::query_version()
{
    io_->write(version);
    auto data = read_until(version);

    assert(data.size() == 2);

    return protocol { data[0], data[1] };
}

////////////////////////////////////////////////////////////////////////////////
firmware command::query_firmware()
{
    io_->write(firmware_query);
    auto data = read_until(firmware_response);

    assert(data.size() >= 2);

    return firmware
    {
        data[0], data[1],
        to_string(data.begin() + 2, data.end())
    };
}

////////////////////////////////////////////////////////////////////////////////
pins command::query_capability()
{
    io_->write(capability_query);
    auto data = read_until(capability_response);

    pos digital = 0;
    firmata::pin pin;
    pins all;

    for(auto ci = data.begin(); ci < data.end(); ++ci)
        if(*ci == 0x7f)
        {
            coerce(pin).digital(digital++);
            all.push_back(firmata::pin());

            using std::swap;
            swap(pin, all.back());
        }
        else
        {
            auto mode = static_cast<firmata::mode>(*ci);
            auto res = static_cast<firmata::res>(*++ci);

            coerce(pin).add(mode);
            coerce(pin).add(mode, res);
        }

    // no garbage at end
    assert(pin.modes().empty());

    return all;
}

////////////////////////////////////////////////////////////////////////////////
void command::query_analog_mapping(pins& all)
{
    io_->write(analog_mapping_query);
    auto data = read_until(analog_mapping_response);

    auto pi = all.begin();
    for(auto ci = data.begin(); ci < data.end(); ++ci, ++pi)
        if(*ci != 0x7f)
        {
            assert(pi < all.end());
            coerce(*pi).analog(*ci);
        }
}

////////////////////////////////////////////////////////////////////////////////
void command::query_state(pins& all)
{
    for(auto& pin : all)
    {
        io_->write(pin_state_query, { pin.digital() });
        auto data = read_until(pin_state_response);

        assert(data.size() >= 3);
        assert(data[0] == pin.digital());

        coerce(pin).mode(static_cast<mode>(data[1]));
        coerce(pin).state(to_value(data.begin() + 2, data.end()));
    }
}

////////////////////////////////////////////////////////////////////////////////
void command::pin_mode(firmata::pin& pin, firmata::mode mode)
{
    io_->write(firmata::pin_mode, { pin.digital(), mode });
}

////////////////////////////////////////////////////////////////////////////////
void command::digital_value(firmata::pin& pin, bool value)
{
    io_->write(firmata::digital_value, { pin.digital(), value });
}

////////////////////////////////////////////////////////////////////////////////
void command::analog_value(firmata::pin& pin, int value)
{
    assert(pin.analog() != npos);

    if(pin.analog() <= 15 && value <= 16383)
    {
        auto id = static_cast<msg_id>(analog_value_base + pin.analog());
        io_->write(id, to_data(value));
    }
    else
    {
        payload data = to_data(value);
        data.insert(data.begin(), pin.analog());

        io_->write(ext_analog_value, data);
    }
}

////////////////////////////////////////////////////////////////////////////////
void command::report_analog(firmata::pin& pin, bool value)
{
    assert(pin.analog() != npos);
    assert(pin.analog() <= 15);

    auto id = static_cast<msg_id>(report_analog_base + pin.analog());
    io_->write(id, { value });
}

////////////////////////////////////////////////////////////////////////////////
void command::report_digital(firmata::pin& pin, bool value)
{
    int port = pin.digital() / 8;
    int bit  = pin.digital() % 8;

    ports_[port].set(bit, value);

    assert(port <= 15);
    auto id = static_cast<msg_id>(report_port_base + port);
    io_->write(id, { ports_[port].any() });
}

////////////////////////////////////////////////////////////////////////////////
void command::sample_rate(const msec& time)
{
    int value = std::min<int>(time.count(), 16383);
    io_->write(firmata::sample_rate, to_data(value & 0x7f));
}

////////////////////////////////////////////////////////////////////////////////
void command::reset() { io_->write(firmata::reset); }

////////////////////////////////////////////////////////////////////////////////
payload command::read_until(msg_id reply_id)
{
    msg_id id;
    payload data;

    do { std::tie(id, data) = io_->read(); }
    while(id != reply_id);

    return data;
}

////////////////////////////////////////////////////////////////////////////////
}
