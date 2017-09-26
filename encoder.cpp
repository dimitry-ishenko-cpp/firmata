////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "encoder.hpp"

#include <functional>
#include <stdexcept>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
encoder::encoder(pin& pin1, pin& pin2) : pin1_(pin1), pin2_(pin2)
{
    if((pin1.mode() == digital_in || pin1.mode() == pullup_in)
    && (pin2.mode() == digital_in || pin2.mode() == pullup_in))
    {
        using namespace std::placeholders;
        id_ = pin1_.on_state_changed(std::bind(&encoder::pin_state_changed, this, _1));
    }
    else throw std::invalid_argument("Invalid pin mode");
}

////////////////////////////////////////////////////////////////////////////////
encoder::~encoder() { pin1_.remove_callback(id_); }

////////////////////////////////////////////////////////////////////////////////
cbid encoder::on_count_changed(int_callback fn) { return changed_.add(std::move(fn)); }
cbid encoder::on_rotate_cw(void_callback fn) { return cw_.add(std::move(fn)); }
cbid encoder::on_rotate_ccw(void_callback fn) { return ccw_.add(std::move(fn)); }

void encoder::remove_callback(cbid id)
{
    changed_.remove(id) || cw_.remove(id) || ccw_.remove(id);
}

////////////////////////////////////////////////////////////////////////////////
void encoder::pin_state_changed(int state)
{
    if(state)
    {
        step_ += pin2_.state() ? 1 : -1;

             if(step_ ==  2) {  cw_(); changed_(++count_); }
        else if(step_ == -2) { ccw_(); changed_(--count_); }

        step_ = 0;
    }
    else step_ += pin2_.state() ? -1 : 1;
}

////////////////////////////////////////////////////////////////////////////////
}
