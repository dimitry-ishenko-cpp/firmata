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
encoder::encoder(pin& pin1, pin& pin2) : pin1_(&pin1), pin2_(&pin2)
{
    if(pin1_->mode() != digital_in && pin1_->mode() != pullup_in)
        throw std::invalid_argument("Invalid pin1 mode");

    if(pin2_->mode() != digital_in && pin2_->mode() != pullup_in)
        throw std::invalid_argument("Invalid pin2 mode");

    using namespace std::placeholders;
    id_ = pin1_->on_state_changed(std::bind(&encoder::pin_state_changed, this, _1));
}

////////////////////////////////////////////////////////////////////////////////
encoder::~encoder() { if(pin1_) pin1_->remove_call(id_); }

////////////////////////////////////////////////////////////////////////////////
void encoder::swap(encoder& rhs) noexcept
{
    using namespace std::placeholders;
    using std::swap;

    swap(pin1_, rhs.pin1_);
    swap(pin2_, rhs.pin2_);
    swap(id_  , rhs.id_  );
    if(pin1_)
    {
        pin1_->remove_call(id_);
        id_ = pin1_->on_state_changed(std::bind(&encoder::pin_state_changed, this, _1));
    }
    if(rhs.pin1_)
    {
        rhs.pin1_->remove_call(rhs.id_);
        rhs.id_ = rhs.pin1_->on_state_changed(std::bind(&encoder::pin_state_changed, &rhs, _1));
    }
    swap(rotate_    , rhs.rotate_    );
    swap(rotate_cw_ , rhs.rotate_cw_ );
    swap(rotate_ccw_, rhs.rotate_ccw_);
    swap(step_      , rhs.step_      );
}

////////////////////////////////////////////////////////////////////////////////
cid encoder::on_rotate(int_call fn) { return rotate_.insert(std::move(fn)); }
cid encoder::on_rotate_cw(void_call fn) { return rotate_cw_.insert(std::move(fn)); }
cid encoder::on_rotate_ccw(void_call fn) { return rotate_ccw_.insert(std::move(fn)); }

bool encoder::remove_call(cid id)
{
    return rotate_.erase(id) || rotate_cw_.erase(id) || rotate_ccw_.erase(id);
}

////////////////////////////////////////////////////////////////////////////////
void encoder::pin_state_changed(int state)
{
    if(state)
    {
        auto step = pin2_->state() ? cw : ccw;
        if(step == step_)
        {
            switch(step)
            {
            case  no: break;
            case  cw: rotate_( 1); rotate_cw_(); break;
            case ccw: rotate_(-1); rotate_ccw_(); break;
            }
        }
        step_ = no;
    }
    else step_ = pin2_->state() ? ccw : cw;
}

////////////////////////////////////////////////////////////////////////////////
}
