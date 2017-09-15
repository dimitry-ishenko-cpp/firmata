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
}

////////////////////////////////////////////////////////////////////////////////
void control::reset()
{
    command_.reset();
    command_.query_state(pins_);
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

        cout << "  digital: " << int(pin.digital()) << endl;
        if(pin.analog() != npos) cout << "  analog: " << int(pin.analog()) << endl;

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
