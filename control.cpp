////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "firmata/control.hpp"

#include <iostream>
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
using namespace std::chrono_literals;
control::msec control::time_ = 100ms;

////////////////////////////////////////////////////////////////////////////////
control::control(io_base* io, bool dont_reset) : io_(io), cmd_(io)
{
    if(!dont_reset) cmd_.reset();

    protocol_ = cmd_.query_version(time_);
    firmware_ = cmd_.query_firmware(time_);

    pins_ = cmd_.query_capability(time_);
    cmd_.query_analog_mapping(pins_, time_);
    cmd_.query_state(pins_, time_);

    cmd_.set_report(pins_);

    using namespace std::placeholders;
    id_ = io_->on_read(std::bind(&control::async_read, this, _1, _2));
}

////////////////////////////////////////////////////////////////////////////////
control::~control() noexcept { io_->remove_callback(id_); }

////////////////////////////////////////////////////////////////////////////////
void control::reset()
{
    cmd_.reset();
    cmd_.query_state(pins_, time_);
    cmd_.set_report(pins_);
}

////////////////////////////////////////////////////////////////////////////////
std::size_t control::pin_count(firmata::mode mode) const noexcept
{
    std::size_t count = 0;
    for(auto const& pin : pins_) if(pin.supports(mode)) ++count;

    return count;
}

////////////////////////////////////////////////////////////////////////////////
firmata::pin& control::pin(firmata::mode mode, firmata::pos pos)
{
    if(mode == analog_in)
        for(auto& pin : pins_)
        { if(pin.analog() == pos) return pin; }
    else
        for(auto& pin : pins_)
        { if(pin.supports(mode) && 0 == pos--) return pin; }

    throw std::out_of_range("Pin not found");
}

////////////////////////////////////////////////////////////////////////////////
const firmata::pin& control::pin(firmata::mode mode, firmata::pos pos) const
{
    if(mode == analog_in)
        for(auto const& pin : pins_)
        { if(pin.analog() == pos) return pin; }
    else
        for(auto const& pin : pins_)
        { if(pin.supports(mode) && 0 == pos--) return pin; }

    throw std::out_of_range("Pin not found");
}

////////////////////////////////////////////////////////////////////////////////
void control::async_read(msg_id id, const payload& data)
{
    if(id >= port_value_base && id < port_value_end)
    {
        auto pos = 8 * static_cast<int>(id - port_value_base);
        auto value = to_value(data);

        for(auto n = 0; n < 8 && pos < pins_.size(); ++n, ++pos)
        {
            auto& pin = pins_.at(pos);
            if(pin.mode() == digital_in || pin.mode() == pullup_in)
            {
                pin.change_state(bool(value & (1 << n)));
            }
        }
    }
    else if(id >= analog_value_base && id < analog_value_end)
    {
        auto pos = static_cast<firmata::pos>(id - analog_value_base);

        for(auto& pin : pins_)
            if(pin.analog() == pos)
            {
                if(pin.mode() == analog_in)
                    pin.change_state(to_value(data));
                break;
            }
    }
    else if(id == string_data)
    {
        std::string s = to_string(data);
        if(string_ != s) chain_(string_ = std::move(s));
    }
}

////////////////////////////////////////////////////////////////////////////////
namespace
{

auto to_string(firmata::mode mode)
{
    switch(mode)
    {
    case digital_in : return "digital_in" ;
    case digital_out: return "digital_out";
    case analog_in  : return "analog_in"  ;
    case pwm        : return "pwm"        ;
    case servo      : return "servo"      ;
    case shift      : return "shift"      ;
    case i2c        : return "i2c"        ;
    case onewire    : return "onewire"    ;
    case stepper    : return "stepper"    ;
    case encoder    : return "encoder"    ;
    case serial     : return "serial"     ;
    case pullup_in  : return "pullup_in"  ;
    default         : return "UNKNOWN"    ;
    };
}

}

////////////////////////////////////////////////////////////////////////////////
void control::info()
{
    using namespace std;

    cout << "Protocol: "
         << protocol().major << "." << protocol().minor
         << endl;

    cout << "Firmware: "
         << firmware().name  << " "
         << firmware().major << "." << firmware().minor
         << endl;

    for(auto& pin : pins_)
    {
        cout << "Pin " << int(pin.pos()) << ":" << endl;
        if(pin.analog() != npos)
            cout << "  analog: " << int(pin.analog()) << endl;

        cout << "  mode: " << to_string(pin.mode()) << endl;
        cout << "  res: " << pin.res() << endl;

        cout << "  modes: ";
        for(auto mode : pin.modes()) cout << to_string(mode) << " ";
        cout << endl;

        cout << "  state: " << pin.state() << endl;
    }
}

////////////////////////////////////////////////////////////////////////////////
}
