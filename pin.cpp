////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "firmata/pin.hpp"
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
void pin::swap(pin& rhs) noexcept
{
    using std::swap;
    swap(pos_     , rhs.pos_     );
    swap(analog_  , rhs.analog_  );
    swap(modes_   , rhs.modes_   );
    swap(reses_   , rhs.reses_   );
    swap(delegate_, rhs.delegate_);
    swap(mode_    , rhs.mode_    );
    swap(value_   , rhs.value_   );
    swap(state_   , rhs.state_   );
    swap(changed_ , rhs.changed_ );
    swap(low_     , rhs.low_     );
    swap(high_    , rhs.high_    );
}

////////////////////////////////////////////////////////////////////////////////
void pin::mode(firmata::mode mode)
{
    if(!supports(mode)) throw std::invalid_argument("Unsupported mode");
    if(!delegate_) throw std::logic_error("Invalid state");

    if(mode_ == digital_in || mode_ == pullup_in)
        delegate_->report_digital(pos_, false);
    else if(mode_ == analog_in)
        delegate_->report_analog(analog_, false);

    mode_ = mode;
    delegate_->pin_mode(pos_, mode_);

    if(mode_ == digital_in || mode_ == pullup_in)
        delegate_->report_digital(pos_, true);
    else if(mode_ == analog_in)
        delegate_->report_analog(analog_, true);
}

////////////////////////////////////////////////////////////////////////////////
void pin::value(int value)
{
    if(!delegate_) throw std::logic_error("Invalid state");

    if(mode_ == digital_out)
    {
        value_ = bool(value);
        delegate_->digital_value(pos_, value_);
    }
    else if(mode_ == pwm)
    {
        value_ = value;
        delegate_->analog_value(pos_, value_);
    }
    else throw std::invalid_argument("Invalid mode");
}

////////////////////////////////////////////////////////////////////////////////
cid pin::on_state_changed(int_call fn) { return changed_.insert(std::move(fn)); }
cid pin::on_state_low(void_call fn) { return low_.insert(std::move(fn)); }
cid pin::on_state_high(void_call fn) { return high_.insert(std::move(fn)); }

bool pin::remove_call(cid id)
{
    return changed_.erase(id) || low_.erase(id) || high_.erase(id);
}

////////////////////////////////////////////////////////////////////////////////
void pin::state(int state)
{
    if(state_ != state)
    {
        changed_(state_ = state);
        if(state_) high_(); else low_();
    }
}

////////////////////////////////////////////////////////////////////////////////
}
