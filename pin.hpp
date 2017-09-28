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
class command;

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

    void swap(pin& rhs) noexcept
    {
        using std::swap;
        swap(pos_,     rhs.pos_    );
        swap(analog_,  rhs.analog_ );
        swap(modes_,   rhs.modes_  );
        swap(reses_,   rhs.reses_  );
        swap(cmd_,     rhs.cmd_    );
        swap(mode_,    rhs.mode_   );
        swap(value_,   rhs.value_  );
        swap(state_,   rhs.state_  );
        swap(changed_, rhs.changed_);
        swap(low_,     rhs.low_    );
        swap(high_,    rhs.high_   );
    }

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
    void remove_call(cid);

private:
    ////////////////////
    firmata::pos pos_ = npos, analog_ = npos;

    std::set<firmata::mode> modes_;
    std::map<firmata::mode, firmata::res> reses_; // res (bits) for each mode

    // pointer to command for setting new mode/value
    command* cmd_ = nullptr;

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
    pin(firmata::pos pos, command* cmd) : pos_(pos), cmd_(cmd) { }
    friend class command;
};

////////////////////////////////////////////////////////////////////////////////
inline void swap(pin& lhs, pin& rhs) noexcept { lhs.swap(rhs); }

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
