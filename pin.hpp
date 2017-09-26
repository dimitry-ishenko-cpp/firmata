////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_PIN_HPP
#define FIRMATA_PIN_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/callback.hpp"
#include "firmata/types.hpp"

#include <map>
#include <set>
#include <utility> // std::swap

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
class command;

////////////////////////////////////////////////////////////////////////////////
class pin
{
public:
    ////////////////////
    pin() noexcept = default;
    pin(const pin&) = delete;
    pin(pin&& other) noexcept { swap(other); }

    pin& operator=(const pin&) = delete;
    pin& operator=(pin&& other) noexcept { swap(other); return *this; }

    void swap(pin& other) noexcept
    {
        using std::swap;
        swap(pos_,    other.pos_   );
        swap(analog_, other.analog_);
        swap(modes_,  other.modes_ );
        swap(reses_,  other.reses_ );
        swap(cmd_,    other.cmd_   );
        swap(mode_,   other.mode_  );
        swap(value_,  other.value_ );
        swap(state_,  other.state_ );
        swap(chain_,  other.chain_ );
    }

    ////////////////////
    auto pos() const noexcept { return pos_; }
    auto analog() const noexcept { return analog_; }

    auto const& modes() const noexcept { return modes_; }
    bool supports(firmata::mode mode) const noexcept { return modes_.count(mode); }

    auto mode() const noexcept { return mode_; }
    void mode(firmata::mode);

    auto res() const noexcept { return reses_.at(mode_); }

    auto value() const noexcept { return value_; }
    void value(int);

    ////////////////////
    auto state() const noexcept { return state_; }

    using state_callback = callback<void(int)>;
    cbid on_state_changed(state_callback fn) { return chain_.add(std::move(fn)); }
    void remove_callback(cbid id) { chain_.remove(id); }

private:
    ////////////////////
    firmata::pos pos_ = npos, analog_ = npos;

    std::set<firmata::mode> modes_;
    // resolution for each mode
    std::map<firmata::mode, firmata::res> reses_;

    command* cmd_ = nullptr;

    firmata::mode mode_;
    int value_ = 0;

    int state_ = 0;
    call_chain<state_callback> chain_;
    void change_state(int);

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
