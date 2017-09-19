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
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
control::control(io::base* io) : io_(io)
{
    io_->write(firmata::reset);

    query_version();
    query_firmware();

    query_capability();
    query_analog_mapping();
    query_state();
    report_all();

    //info();

    using namespace std::placeholders;
    io_->reset_async(std::bind(&control::async_read, this, _1, _2));
}

////////////////////////////////////////////////////////////////////////////////
void control::reset()
{
    io_->write(firmata::reset);

    query_state();
    report_all();
}

////////////////////////////////////////////////////////////////////////////////
void control::string(const std::string& string)
{
    io_->write(string_data, to_data(string));
}

////////////////////////////////////////////////////////////////////////////////
std::size_t control::analog_count() const
{
    return std::count_if(pin_cbegin(), pin_cend(),
        [](auto& pin){ return pin.analog() != npos; }
    );
}

////////////////////////////////////////////////////////////////////////////////
firmata::pin& control::analog(firmata::pos pos)
{
    auto ni = std::find_if(pin_begin(), pin_end(),
        [=](auto& pin){ return pin.analog() == pos; }
    );
    return ni == pin_end() ? throw std::out_of_range("firmata::control::analog(): pin not found")
                           : *ni;
}

////////////////////////////////////////////////////////////////////////////////
const firmata::pin& control::analog(firmata::pos pos) const
{
    auto ni = std::find_if(pin_cbegin(), pin_cend(),
        [=](auto& pin){ return pin.analog() == pos; }
    );
    return ni == pin_end() ? throw std::out_of_range("firmata::control::analog(): pin not found")
                           : *ni;
}

////////////////////////////////////////////////////////////////////////////////
void control::on_string_changed(control::string_callback fn)
{ string_callback_.push_back(std::move(fn)); }

////////////////////////////////////////////////////////////////////////////////
void control::change_string(std::string string)
{
    if(string != string_)
    {
        string_ = std::move(string);
        for(auto& fn : string_callback_) fn(string_);
    }
}

////////////////////////////////////////////////////////////////////////////////
payload control::read_until(msg_id reply_id)
{
    msg_id id;
    payload data;
    do std::tie(id, data) = io_->read(); while(id != reply_id);

    return data;
}

////////////////////////////////////////////////////////////////////////////////
void control::query_version()
{
    io_->write(version);
    auto data = read_until(version);

    if(data.size() == 2)
    {
        protocol_.major = data[0];
        protocol_.minor = data[1];
    }
}

////////////////////////////////////////////////////////////////////////////////
void control::query_firmware()
{
    io_->write(firmware_query);
    auto data = read_until(firmware_response);

    if(data.size() >= 2)
    {
        firmware_.major = data[0];
        firmware_.minor = data[1];
        firmware_.name = to_string(data.begin() + 2, data.end());
    }
}

////////////////////////////////////////////////////////////////////////////////
void control::query_capability()
{
    io_->write(capability_query);
    auto data = read_until(capability_response);

    using namespace std::placeholders;
    auto pin_mode      = std::bind(&control::pin_mode, this, _1, _2, _3);
    auto digital_value = std::bind(&control::digital_value, this, _1, _2);
    auto analog_value  = std::bind(&control::analog_value, this, _1, _2);

    firmata::pos pos = 0;
    firmata::pin pin;

    for(auto ci = data.begin(); ci < data.end(); ++ci)
        if(*ci == 0x7f)
        {
            pin.pos_ = pos++;
            pin.delegate_ = { pin_mode, digital_value, analog_value };

            pins_.push_back(firmata::pin());

            using std::swap;
            swap(pin, pins_.back());
        }
        else
        {
            auto mode = static_cast<firmata::mode>(*ci);
            auto res = static_cast<firmata::res>(*++ci);

            pin.modes_.insert(mode);
            pin.reses_.emplace(mode, res);
        }
}

////////////////////////////////////////////////////////////////////////////////
void control::query_analog_mapping()
{
    io_->write(analog_mapping_query);
    auto data = read_until(analog_mapping_response);

    auto pi = pins_.begin();
    for(auto ci = data.begin(); ci != data.end() && pi != pins_.end(); ++ci, ++pi)
        if(*ci != 0x7f) pi->analog_ = *ci;
}

////////////////////////////////////////////////////////////////////////////////
void control::query_state()
{
    for(auto& pin : pins_)
    {
        io_->write(pin_state_query, { pin.pos() });
        auto data = read_until(pin_state_response);

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
void control::report_all()
{
    // enable reporting for all inputs
    // and disable for all outputs
    for(auto& pin : pins_)
        switch(pin.mode())
        {
        case digital_in:
        case pullup_in:
            report_digital(&pin, true);
            break;

        case digital_out:
        case pwm:
            report_digital(&pin, false);
            break;

        case analog_in:
            report_analog(&pin, true);
            break;

        default: break;
        }
}

////////////////////////////////////////////////////////////////////////////////
void control::pin_mode(firmata::pin* pin, mode now, mode before)
{
    if(before == digital_in || before == pullup_in)
        report_digital(pin, false);
    else if(before == analog_in)
        report_analog(pin, false);

    io_->write(firmata::pin_mode, { pin->pos(), now });

    if(now == digital_in || now == pullup_in)
        report_digital(pin, true);
    else if(now == analog_in) report_analog(pin, true);
}

////////////////////////////////////////////////////////////////////////////////
void control::digital_value(firmata::pin* pin, bool value)
{
    io_->write(firmata::digital_value, { pin->pos(), value });
}

////////////////////////////////////////////////////////////////////////////////
void control::analog_value(firmata::pin* pin, int value)
{
    payload data = to_data(value);
    data.insert(data.begin(), pin->pos());

    io_->write(ext_analog_value, data);
}

////////////////////////////////////////////////////////////////////////////////
void control::report_digital(firmata::pin* pin, bool value)
{
    int port = pin->pos() / 8, bit = pin->pos() % 8;

    if(port <= 15)
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
void control::report_analog(firmata::pin* pin, bool value)
{
    if(pin->analog() != npos && pin->analog() <= 15)
    {
        auto id = static_cast<msg_id>(report_analog_base + pin->analog());
        io_->write(id, { value });
    }
}

////////////////////////////////////////////////////////////////////////////////
void control::sample_rate(const msec& time)
{
    int value = std::min<int>(time.count(), 16383);
    io_->write(firmata::sample_rate, to_data(value));
}

////////////////////////////////////////////////////////////////////////////////
void control::async_read(msg_id id, const payload& data)
{
    if(id >= port_value_base && id < port_value_end)
    {
        auto pos = 8 * static_cast<int>(id - port_value_base);
        auto value = to_value(data);

        for(auto n = 0; n < 8; ++n, ++pos)
        {
            auto& pin = pins_.at(pos);

            if(pin.mode() == digital_in
            || pin.mode() == pullup_in)
            pin.change_state(bool(value & (1 << n)));
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
    else if(id == string_data) change_string(to_string(data));
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

    for(auto& pin : pins_)
    {
        cout << "PIN" << endl;
        cout << "  digital: " << int(pin.pos()) << endl;
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
