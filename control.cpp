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
#include <functional>
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

    using namespace std::placeholders;
    io_->set_callback(std::bind(&control::async_read, this, _1, _2));
}

////////////////////////////////////////////////////////////////////////////////
void control::reset()
{
    io_->write(firmata::reset);
    query_state();
}

////////////////////////////////////////////////////////////////////////////////
payload control::read_until(msg_id reply_id)
{
    msg_id id;
    payload data;

    do { std::tie(id, data) = io_->read(); }
    while(id != reply_id);

    return data;
}

////////////////////////////////////////////////////////////////////////////////
void control::async_read(msg_id, const payload&)
{

}

////////////////////////////////////////////////////////////////////////////////
void control::query_firmware()
{
    io_->write(firmware_query);
    auto data = read_until(firmware_response);

    assert(data.size() >= 2);

    version_ = std::make_tuple(data[0], data[1]);
    firmware_ = to_string(data.begin() + 2, data.end());
}

////////////////////////////////////////////////////////////////////////////////
void control::query_capability()
{
    io_->write(capability_query);
    auto data = read_until(capability_response);

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
            auto mode = static_cast<firmata::mode>(*ci);
            auto res = static_cast<firmata::res>(*++ci);

            pin.modes_.insert(mode);
            pin.reses_.emplace(mode, res);
        }

    assert(pin.modes_.empty());
}

////////////////////////////////////////////////////////////////////////////////
void control::query_analog_mapping()
{
    io_->write(analog_mapping_query);
    auto data = read_until(analog_mapping_response);

    firmata::pos max = 0;
    analog_.resize(pins_.size(), pins_.end());

    auto pi = pins_.begin();
    for(auto ci = data.begin(); ci < data.end(); ++ci, ++pi)
    {
        auto pos = *ci;
        if(pos != 0x7f)
        {
            assert(pi < pins_.end());
            assert(pos < analog_.size());

            pi->analog_ = pos;
            analog_[pos] = pi;

            max = std::max(max, pos);
        }
    }

    analog_.resize(max);

    for(auto pi : analog_) assert(pi != pins_.end());
}

////////////////////////////////////////////////////////////////////////////////
void control::query_state()
{
    for(auto& pin : pins_)
    {
        io_->write(pin_state_query, { pin.pos(digital) });
        auto data = read_until(pin_state_response);

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

        cout << "  digital: " << int(pin.pos(digital)) << endl;
        if(pin.pos(firmata::analog) != invalid)
            cout << "  analog: " << int(pin.pos(firmata::analog)) << endl;

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
