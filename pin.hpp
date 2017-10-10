////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_PIN_HPP
#define FIRMATA_PIN_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/call_chain.hpp"
#include "firmata/types.hpp"

#include <map>
#include <set>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
class control;

////////////////////////////////////////////////////////////////////////////////
// Firmata pin
//
class pin
{
public:
    ////////////////////
    pin() noexcept = default;
    pin(const pin&) = delete;
    pin(pin&& rhs) noexcept { swap(rhs); }

    pin& operator=(const pin&) = delete;
    pin& operator=(pin&& rhs) noexcept { swap(rhs); return *this; }

    void swap(pin&) noexcept;

    ////////////////////
    bool valid() const noexcept { return pos_ != npos; }
    explicit operator bool() const noexcept { return valid(); }

    ////////////////////
    // pin position (number)
    auto pos() const noexcept { return pos_; }
    // analong position
    auto analog() const noexcept { return analog_; }

    // supported modes
    auto const& modes() const noexcept { return modes_; }
    bool supports(firmata::mode mode) const noexcept { return modes_.count(mode); }

    // current mode
    auto mode() const noexcept { return mode_; }
    // set new mode
    void mode(firmata::mode);

    // current mode res (in bits)
    auto res() const noexcept { return reses_.at(mode_); }

    // current value
    auto value() const noexcept { return value_; }
    // set new value
    void value(int);

    // current state
    auto state() const noexcept { return state_; }

    ////////////////////
    using int_call = call<void(int)>;
    using void_call = call<void()>;

    // install state changed/low/high callback
    cid on_state_changed(int_call);
    cid on_state_low(void_call);
    cid on_state_high(void_call);

    // remove callback
    bool remove_call(cid);

private:
    ////////////////////
    firmata::pos pos_ = npos, analog_ = npos;

    std::set<firmata::mode> modes_;
    std::map<firmata::mode, firmata::res> reses_; // res (bits) for each mode

    firmata::mode mode_; // current mode
    int value_ = 0; // current value
    int state_ = 0; // current state

    // state changed/low/high call chains
    call_chain< int_call> changed_ { 0 };
    call_chain<void_call> low_     { 1 };
    call_chain<void_call> high_    { 2 };

    // set new state
    void state(int);

    ////////////////////
    // delegate for setting new mode/value
    struct delegate
    {
        call<void(firmata::pos, bool)> report_digital;
        call<void(firmata::pos, bool)> report_analog;

        call<void(firmata::pos, bool)> digital_value;
        call<void(firmata::pos, int)> analog_value;

        call<void(firmata::pos, firmata::mode)> pin_mode;
    };

    delegate* delegate_ = nullptr;

    ////////////////////
    pin(firmata::pos pos, delegate* del) : pos_(pos), delegate_(del) { }
    friend class control;
};

////////////////////////////////////////////////////////////////////////////////
inline void swap(pin& lhs, pin& rhs) noexcept { lhs.swap(rhs); }

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
