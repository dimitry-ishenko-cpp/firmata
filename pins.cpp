////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "firmata/pins.hpp"
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
firmata::pin& pins::get(firmata::mode mode, firmata::pos pos)
{
    if(mode == analog_in)
    {
        for(auto& pin : pins_)
            if(pin.analog() == pos) return pin;
    }
    else
    {
        for(auto& pin : pins_)
            if(pin.supports(mode) && 0 == pos--) return pin;
    }

    throw std::out_of_range("Pin not found");
}

////////////////////////////////////////////////////////////////////////////////
const firmata::pin& pins::get(firmata::mode mode, firmata::pos pos) const
{
    if(mode == analog_in)
    {
        for(auto& pin : pins_)
            if(pin.analog() == pos) return pin;
    }
    else
    {
        for(auto& pin : pins_)
            if(pin.supports(mode) && 0 == pos--) return pin;
    }

    throw std::out_of_range("Pin not found");
}

////////////////////////////////////////////////////////////////////////////////
}
