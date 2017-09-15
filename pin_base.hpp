////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL v3.
// For full terms see COPYING or visit https://www.gnu.org/licenses/gpl.html

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_PIN_BASE_HPP
#define FIRMATA_PIN_BASE_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/types.hpp"

#include <functional>
#include <map>
#include <set>
#include <utility> // std::swap

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
struct pin_base
{
    ////////////////////
    pin_base() noexcept = default;
    pin_base(const pin_base&) = delete;
    pin_base(pin_base&& other) noexcept { swap(other); }

    pin_base& operator=(const pin_base&) = delete;
    pin_base& operator=(pin_base&& other) noexcept { swap(other); return *this; }

    void swap(pin_base& other) noexcept
    {
        using std::swap;
        swap(digital_,  other.digital_ );
        swap(analog_,   other.analog_  );
        swap(modes_,    other.modes_   );
        swap(reses_,    other.reses_   );
        swap(mode_,     other.mode_    );
        swap(value_,    other.value_   );
        swap(state_,    other.state_   );
        swap(fn_mode_,  other.fn_mode_ );
        swap(fn_value_, other.fn_value_);
    }

    ////////////////////
    void digital(pos digital) { digital_ = digital; }
    void analog(pos analog) { analog_ = analog; }

    void add(firmata::mode mode) { modes_.insert(mode); }
    void add(firmata::mode mode, firmata::res res) { reses_.emplace(mode, res); }

    void mode(firmata::mode mode) { mode_ = mode; }

    void value(int value) { value_ = value; }
    void state(int value) { state_ = value; }

    ////////////////////
    using fn_mode = std::function<void(pin_base*, firmata::mode)>;
    using fn_value = std::function<void(pin_base*, int)>;

    void set(fn_mode fn) { fn_mode_ = std::move(fn); }
    void set(fn_value fn) { fn_value_ = std::move(fn); }

protected:
    ////////////////////
    pos digital_ = npos, analog_ = npos;

    std::set<firmata::mode> modes_;
    // resolution for each mode
    std::map<firmata::mode, firmata::res> reses_;

    firmata::mode mode_;
    int value_ = 0;
    int state_ = 0;

    ////////////////////
    fn_mode fn_mode_;
    fn_value fn_value_;

    void set_mode(firmata::mode mode) { if(fn_mode_) fn_mode_(this, mode); }
    void set_value(int value) { if(fn_value_) fn_value_(this, value); }
};

////////////////////////////////////////////////////////////////////////////////
inline void swap(pin_base& lhs, pin_base& rhs) noexcept { lhs.swap(rhs); }

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
