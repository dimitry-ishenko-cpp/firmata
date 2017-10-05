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

msec control::time_ = 100ms; // default read timeout

////////////////////////////////////////////////////////////////////////////////
control::control(io_base* io, bool dont_reset) : io_(io)
{
    using namespace std::placeholders;

    delegate_.report_digital = std::bind(&control::report_digital, this, _1, _2);
    delegate_.report_analog = std::bind(&control::report_analog, this, _1, _2);
    delegate_.digital_value = std::bind(&control::digital_value, this, _1, _2);
    delegate_.analog_value = std::bind(&control::analog_value, this, _1, _2);
    delegate_.pin_mode = std::bind(&control::pin_mode, this, _1, _2);

    if(!dont_reset) reset_();

    query_version();
    query_firmware();

    query_capability();
    query_analog_mapping();
    query_state();

    set_report();

    id_ = io_->on_read(std::bind(&control::async_read, this, _1, _2));
}

////////////////////////////////////////////////////////////////////////////////
control::~control() noexcept { if(io_) io_->remove_call(id_); }

////////////////////////////////////////////////////////////////////////////////
void control::swap(control& rhs) noexcept
{
    using namespace std::placeholders;
    using std::swap;

    swap(io_, rhs.io_);
    swap(id_, rhs.id_);
    if(io_)
    {
        io_->remove_call(id_);
        id_ = io_->on_read(std::bind(&control::async_read, this, _1, _2));
    }
    if(rhs.io_)
    {
        rhs.io_->remove_call(rhs.id_);
        rhs.id_ = rhs.io_->on_read(std::bind(&control::async_read, &rhs, _1, _2));
    }
    swap(protocol_, rhs.protocol_);
    swap(firmware_, rhs.firmware_);
    swap(pins_    , rhs.pins_    );
    for(auto& pin: pins_) pin.delegate_ = &delegate_;
    for(auto& pin: rhs.pins_) pin.delegate_ = &rhs.delegate_;
    swap(string_  , rhs.string_  );
    swap(chain_   , rhs.chain_   );
    swap(ports_   , rhs.ports_   );
}

////////////////////////////////////////////////////////////////////////////////
void control::reset()
{
    if(!io_) throw std::logic_error("Invalid state");

    reset_();
    query_state();
    set_report();
}

////////////////////////////////////////////////////////////////////////////////
void control::report_digital(firmata::pos pos, bool value)
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
void control::report_analog(firmata::pos pos, bool value)
{
    if(pos != npos && pos < analog_count)
    {
        auto id = static_cast<msg_id>(report_analog_base + pos);
        io_->write(id, { value });
    }
}

////////////////////////////////////////////////////////////////////////////////
void control::digital_value(firmata::pos pos, bool value)
{
    io_->write(firmata::digital_value, { pos, value });
}

////////////////////////////////////////////////////////////////////////////////
void control::analog_value(firmata::pos pos, int value)
{
    payload data = to_data(value);
    data.insert(data.begin(), pos);

    io_->write(ext_analog_value, data);
}

////////////////////////////////////////////////////////////////////////////////
void control::pin_mode(firmata::pos pos, firmata::mode mode)
{
    io_->write(firmata::pin_mode, { pos, mode });
}

////////////////////////////////////////////////////////////////////////////////
void control::reset_() { io_->write(firmata::reset); }

////////////////////////////////////////////////////////////////////////////////
void control::string(const std::string& s)
{
    if(!io_) throw std::logic_error("Invalid state");
    io_->write(string_data, to_data(s));
}

////////////////////////////////////////////////////////////////////////////////
void control::query_version()
{
    io_->write(version);
    auto data = wait_until(version);

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
    auto data = wait_until(firmware_response);

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
    auto data = wait_until(capability_response);

    firmata::pos pos = 0;
    firmata::pin pin(pos, &delegate_);

    for(auto ci = data.begin(); ci < data.end(); ++ci)
        if(*ci == 0x7f)
        {
            pins_.push_back(firmata::pin(++pos, &delegate_));

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
    auto data = wait_until(analog_mapping_response);

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
        auto data = wait_until(pin_state_response);

        if(data.size() >= 3 && data[0] == pin.pos())
        {
            auto mode = static_cast<firmata::mode>(data[1]);
            auto state = to_value(data.begin() + 2, data.end());

            pin.mode_ = mode;
            pin.state(state);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void control::set_report()
{
    // enable reporting for all inputs
    // and disable for all outputs
    for(auto& pin : pins_)
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
void control::async_read(msg_id id, const payload& data)
{
    if(id >= port_value_base && id < port_value_end)
    {
        auto pos = 8 * static_cast<int>(id - port_value_base);
        auto value = to_value(data);

        for(auto n = 0; n < 8 && pos < pins_.count(); ++n, ++pos)
        {
            auto& pin = pins_.get(pos);
            if(pin.mode() == digital_in || pin.mode() == pullup_in)
            {
                // set pin state through cmd_,
                // since pin::state(int) is private
                pin.state(bool(value & (1 << n)));
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
                {
                    // set pin state through cmd_,
                    // since pin::state(int) is private
                    pin.state(to_value(data));
                }
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
payload control::wait_until(msg_id reply_id)
{
    payload reply_data;
    auto id = io_->on_read([&](msg_id id, const payload& data)
        { if(id == reply_id) reply_data = data; }
    );

    if(!io_->wait_until([&](){ return reply_data.size(); }, time_))
    {
        io_->remove_call(id);
        throw timeout_error("Read timed out");
    }

    io_->remove_call(id);
    return reply_data;
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
        cout << "Pin " << +pin.pos() << ":" << endl;
        if(pin.analog() != npos)
            cout << "  analog: " << +pin.analog() << endl;

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
