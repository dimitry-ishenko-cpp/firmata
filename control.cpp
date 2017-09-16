////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL v3.
// For full terms see COPYING or visit https://www.gnu.org/licenses/gpl.html

////////////////////////////////////////////////////////////////////////////////
#include "firmata/control.hpp"

#include <algorithm>
#include <functional>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
namespace
{

constexpr inline bool digital(const pin& p) noexcept
{ return p.mode() == digital_in || p.mode() == digital_out || p.mode() == pullup_in; }

constexpr inline bool analog(const pin& p) noexcept
{ return p.mode() == analog_in || p.mode() == pwm; }

constexpr inline bool input(const pin& p) noexcept
{ return p.mode() == digital_in || p.mode() == analog_in || p.mode() == pullup_in; }

}

////////////////////////////////////////////////////////////////////////////////
control::control(io::base* io) :
    command_(io)
{
    protocol_ = command_.query_version();
    firmware_ = command_.query_firmware();

    pins_ = command_.query_capability();
    command_.query_analog_mapping(pins_);
    command_.query_state(pins_);

    //info();

    using namespace std::placeholders;
    io->reset_async(std::bind(&control::async_read, this, _1, _2));

    ////////////////////
    for(auto& pin : pins_)
    {
        pin.delegate_.fn_mode(std::bind(&control::mode, this, _1, _2));
        pin.delegate_.fn_value(std::bind(&control::value, this, _1, _2));

        if(input(pin))
        {
                if(digital(pin)) command_.report_digital(pin.pos(), true);
            else if(analog(pin)) command_.report_analog(pin.analog_pos(), true);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void control::reset()
{
    command_.reset();
    command_.query_state(pins_);
}

////////////////////////////////////////////////////////////////////////////////
void control::mode(firmata::pin& pin, firmata::mode mode)
{
    if(input(pin))
    {
        if(digital(pin))
            command_.report_digital(pin.pos(), false);
        else if(analog(pin))
            command_.report_analog(pin.analog_pos(), false);
    }

    pin.delegate_.mode(mode);
    command_.pin_mode(pin.pos(), mode);

    if(input(pin))
    {
            if(digital(pin)) command_.report_digital(pin.pos(), true);
        else if(analog(pin)) command_.report_analog(pin.analog_pos(), true);
    }
}

////////////////////////////////////////////////////////////////////////////////
void control::value(firmata::pin& pin, int value)
{
        if(digital(pin)) command_.digital_value(pin.pos(), value);
    else if(analog(pin)) command_.analog_value(pin.analog_pos(), value);
}

////////////////////////////////////////////////////////////////////////////////
void control::async_read(msg_id, const payload&)
{

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

    cout << "PROTOCOL: "
         << protocol().major << "." << protocol().minor
         << endl;

    cout << "FIRMWARE: "
         << firmware().name  << " "
         << firmware().major << "." << firmware().minor
         << endl;

    for(auto n = 0; n < pin_count(); ++n)
    {
        cout << "PIN" << endl;
        auto& pin = this->pin(n);

        cout << "  digital: " << int(pin.pos()) << endl;
        if(pin.analog_pos() != npos)
            cout << "  analog: " << int(pin.analog_pos()) << endl;

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
