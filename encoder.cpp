////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "encoder.hpp"

#include <functional>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
encoder::encoder(pin& pin1, pin& pin2) : pin1_(pin1), pin2_(pin2)
{
    using namespace std::placeholders;
    id_ = pin1_.on_state_low(std::bind(&encoder::pin_state_low, this));
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
void encoder::pin_state_low()
{
    if(pin2_.state()) { count_--; ccw_(); }
    else { count_++; cw_(); }

    changed_(count_);
}

////////////////////////////////////////////////////////////////////////////////
}
