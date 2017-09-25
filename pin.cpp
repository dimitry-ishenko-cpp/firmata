////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "firmata/command.hpp"
#include "firmata/pin.hpp"

#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
void pin::mode(firmata::mode mode)
{
    if(!supports(mode)) throw std::invalid_argument("Unsupported mode");
    if(!cmd_) throw std::logic_error("Invalid state");

    if(mode_ == digital_in || mode_ == pullup_in)
        cmd_->report_digital(pos_, false);
    else if(mode_ == analog_in)
        cmd_->report_analog(analog_, false);

    mode_ = mode;
    cmd_->pin_mode(pos_, mode_);

    if(mode_ == digital_in || mode_ == pullup_in)
        cmd_->report_digital(pos_, true);
    else if(mode_ == analog_in)
        cmd_->report_analog(analog_, true);
}

////////////////////////////////////////////////////////////////////////////////
void pin::value(int value)
{
    if(!cmd_) throw std::logic_error("Invalid state");

    if(mode_ == digital_out)
    {
        value_ = bool(value);
        cmd_->digital_value(pos_, value_);
    }
    else if(mode_ == pwm)
    {
        value_ = value;
        cmd_->analog_value(pos_, value_);
    }
    else throw std::invalid_argument("Invalid mode");
}

////////////////////////////////////////////////////////////////////////////////
void pin::change_state(int s) { if(state_ != s) chain_(state_ = s); }

////////////////////////////////////////////////////////////////////////////////
}
