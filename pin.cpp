////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL v3.
// For full terms see COPYING or visit https://www.gnu.org/licenses/gpl.html

////////////////////////////////////////////////////////////////////////////////
#include "firmata/pin.hpp"

#include <stdexcept>
#include <utility> // std::swap

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
void pin::mode(firmata::mode mode)
{
    if(supports(mode))
    {
        if(delegate_.pin_mode)
        {
            std::swap(mode_, mode);
            delegate_.pin_mode(this, mode_, mode);
        }
    }
    else throw std::invalid_argument("firmata::pin::mode(): unsupported mode");
}

////////////////////////////////////////////////////////////////////////////////
void pin::value(int value)
{
    if(mode_ == digital_out)
    {
        if(delegate_.digital_value)
        {
            value_ = bool(value);
            delegate_.digital_value(this, value_);
        }
    }
    else if(mode_ == pwm)
    {
        if(delegate_.analog_value)
        {
            value_ = value;
            delegate_.analog_value(this, value_);
        }
    }
    else throw std::invalid_argument("firmata::pin::value(): invalid mode");
}

////////////////////////////////////////////////////////////////////////////////
void pin::on_state_changed(state_callback fn)
{ state_callback_.push_back(std::move(fn)); }

////////////////////////////////////////////////////////////////////////////////
void pin::change_state(int state)
{
    if(state != state_)
    {
        state_ = state;
        for(auto& fn : state_callback_) fn(state_);
    }
}

////////////////////////////////////////////////////////////////////////////////
}
