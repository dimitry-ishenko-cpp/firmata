////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "firmata/command.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
void command::report_digital(firmata::pos pos, bool value)
{
    std::size_t port = pos / 8, bit = pos % 8;

    if(port < ports_.size())
    {
        bool before = ports_[port].any();
        ports_[port].set(bit, value);
        bool now = ports_[port].any();

        auto id = static_cast<msg_id>(report_port_base + port);

        if(before && !now) io_->write(id, { false });
        else if(!before && now) io_->write(id, { true });
    }
}

////////////////////////////////////////////////////////////////////////////////
void command::report_analog(firmata::pos pos, bool value)
{
    if(pos != npos && pos < analog_count)
    {
        auto id = static_cast<msg_id>(report_analog_base + pos);
        io_->write(id, { value });
    }
}

////////////////////////////////////////////////////////////////////////////////
void command::digital_value(firmata::pos pos, bool value)
{
    io_->write(firmata::digital_value, { pos, value });
}

////////////////////////////////////////////////////////////////////////////////
void command::analog_value(firmata::pos pos, int value)
{
    payload data = to_data(value);
    data.insert(data.begin(), pos);

    io_->write(ext_analog_value, data);
}

////////////////////////////////////////////////////////////////////////////////
void command::pin_mode(firmata::pos pos, firmata::mode mode)
{
    io_->write(firmata::pin_mode, { pos, mode });
}

////////////////////////////////////////////////////////////////////////////////
void command::pin_state(firmata::pin& pin, int state)
{
    pin.change_state(state);
}

////////////////////////////////////////////////////////////////////////////////
void command::reset() { io_->write(firmata::reset); }

////////////////////////////////////////////////////////////////////////////////
void command::string(const std::string& s)
{
    io_->write(string_data, to_data(s));
}

////////////////////////////////////////////////////////////////////////////////
protocol command::query_version(const msec& time)
{
    io_->write(version);
    auto data = wait_until(version, time);

    firmata::protocol protocol { };
    if(data.size() == 2)
    {
        protocol.major = data[0];
        protocol.minor = data[1];
    }
    return protocol;
}

////////////////////////////////////////////////////////////////////////////////
firmware command::query_firmware(const msec& time)
{
    io_->write(firmware_query);
    auto data = wait_until(firmware_response, time);

    firmata::firmware firmware { };
    if(data.size() >= 2)
    {
        firmware.major = data[0];
        firmware.minor = data[1];
        firmware.name = to_string(data.begin() + 2, data.end());
    }
    return firmware;
}

////////////////////////////////////////////////////////////////////////////////
firmata::pins command::query_capability(const msec& time)
{
    io_->write(capability_query);
    auto data = wait_until(capability_response, time);

    firmata::pins pins;
    firmata::pos pos = 0;
    firmata::pin pin(pos, this);

    for(auto ci = data.begin(); ci < data.end(); ++ci)
        if(*ci == 0x7f)
        {
            pins.push_back(firmata::pin(++pos, this));

            using std::swap;
            swap(pin, pins.back());
        }
        else
        {
            auto mode = static_cast<firmata::mode>(*ci);
            auto res = static_cast<firmata::res>(*++ci);

            pin.modes_.insert(mode);
            pin.reses_.emplace(mode, res);
        }

    return pins;
}

////////////////////////////////////////////////////////////////////////////////
void command::query_analog_mapping(firmata::pins& pins, const msec& time)
{
    io_->write(analog_mapping_query);
    auto data = wait_until(analog_mapping_response, time);

    auto pi = pins.begin();
    for(auto ci = data.begin(); ci != data.end() && pi != pins.end(); ++ci, ++pi)
        if(*ci != 0x7f) pi->analog_ = *ci;
}

////////////////////////////////////////////////////////////////////////////////
void command::query_state(firmata::pins& pins, const msec& time)
{
    for(auto& pin : pins)
    {
        io_->write(pin_state_query, { pin.pos() });
        auto data = wait_until(pin_state_response, time);

        if(data.size() >= 3 && data[0] == pin.pos())
        {
            auto mode = static_cast<firmata::mode>(data[1]);
            auto state = to_value(data.begin() + 2, data.end());

            pin.mode_ = mode;
            pin.change_state(state);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void command::set_report(firmata::pins& pins)
{
    // enable reporting for all inputs and disable for all outputs
    for(auto& pin : pins)
        switch(pin.mode())
        {
        case digital_in:
        case pullup_in:
            report_digital(pin.pos(), true);
            break;

        case digital_out:
        case pwm:
            report_digital(pin.pos(), false);
            break;

        case analog_in:
            report_analog(pin.analog(), true);
            break;

        default: break;
        }
}

////////////////////////////////////////////////////////////////////////////////
payload command::wait_until(msg_id reply_id, const msec& time)
{
    payload reply_data;
    auto id = io_->on_read([&](msg_id id, const payload& data)
        { if(id == reply_id) reply_data = data; }
    );

    if(!io_->wait_until([&](){ return !reply_data.empty(); }, time))
        throw timeout_error("Read timed out");

    io_->remove_callback(id);
    return reply_data;
}

////////////////////////////////////////////////////////////////////////////////
}
