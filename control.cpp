////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL v3.
// For full terms see COPYING or visit https://www.gnu.org/licenses/gpl.html

////////////////////////////////////////////////////////////////////////////////
#include "firmata/control.hpp"

#include <algorithm>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
control::control(io::base* io) : io_(io)
{
    query_firmware();
    query_capability();
    query_analog_mapping();
    query_state();

    //info();
}

////////////////////////////////////////////////////////////////////////////////
void control::reset()
{
    io_->send(firmata::reset);
    query_state();
}

////////////////////////////////////////////////////////////////////////////////
payload control::get(msg_id reply_id)
{
    msg_id id;
    payload data;

    do { std::tie(id, data) = io_->recv(); }
    while(id != reply_id);

    return data;
}

////////////////////////////////////////////////////////////////////////////////
void control::query_firmware()
{
    io_->send(firmware_query);
    auto data = get(firmware_response);

    assert(data.size() >= 2);

    version_ = std::make_tuple(data[0], data[1]);
    firmware_ = to_string(data.begin() + 2, data.end());
}

////////////////////////////////////////////////////////////////////////////////
void control::query_capability()
{
    io_->send(capability_query);
    auto data = get(capability_response);

    firmata::pin pin;
    firmata::pos pos = 0;
    for(auto ci = data.begin(); ci < data.end(); ++ci)
        if(*ci == 0x7f)
        {
            pin.digital_ = pos;
            pins_.push_back(std::move(pin));

            pin = firmata::pin(); ++pos;
        }
        else
        {
            pin.modes_.insert(mode(*ci));
            pin.reses_.emplace(mode(*ci), res(*++ci));
        }

    assert(pin.modes_.empty());
}

////////////////////////////////////////////////////////////////////////////////
void control::query_analog_mapping()
{
    io_->send(analog_mapping_query);
    auto data = get(analog_mapping_response);

    pos size = 0;
    analog_.resize(pins_.size(), pins_.end());

    auto pi = pins_.begin();
    for(auto ci = data.begin(); ci < data.end(); ++ci, ++pi)
        if(*ci != 0x7f)
        {
            assert(pi < pins_.end());
            assert(*ci < analog_.size());

            pi->analog_ = *ci;
            analog_[*ci] = pi;

            size = std::max(size, *ci);
        }

    analog_.resize(size);
    for(auto pi : analog_) assert(pi != pins_.end());
}

////////////////////////////////////////////////////////////////////////////////
void control::query_state()
{
    for(auto& pin : pins_)
    {
        io_->send(pin_state_query, { pin.pos(digital) });
        auto data = get(pin_state_response);

        assert(data.size() >= 3);
        assert(data[0] == pin.pos(digital));

        pin.mode_ = static_cast<mode>(data[1]);
        pin.state_ = to_value(data.begin() + 2, data.end());
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

    int major, minor;
    std::tie(major, minor) = version();
    cout << "FIRMWARE: " << firmware() << " " << major << "." << minor << endl;

    for(auto n = 0; n < pin_count(); ++n)
    {
        cout << "PIN" << endl;
        auto& pin = this->pin(n);

        cout << "  digital: " << (int)pin.pos(digital) << endl;
        if(pin.pos(firmata::analog) != invalid)
            cout << "  analog: " << (int)pin.pos(firmata::analog) << endl;

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
