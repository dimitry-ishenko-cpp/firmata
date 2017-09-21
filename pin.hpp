////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_PIN_HPP
#define FIRMATA_PIN_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/types.hpp"

#include <functional>
#include <map>
#include <set>
#include <utility> // std::swap
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

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
        swap(pos_,      other.pos_     );
        swap(analog_,   other.analog_  );
        swap(modes_,    other.modes_   );
        swap(reses_,    other.reses_   );
        swap(mode_,     other.mode_    );
        swap(value_,    other.value_   );
        swap(state_,    other.state_   );
        swap(delegate_, other.delegate_);
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

    using state_callback = std::function<void(int)>;
    void on_state_changed(state_callback);

private:
    ////////////////////
    firmata::pos pos_ = npos, analog_ = npos;

    std::set<firmata::mode> modes_;
    // resolution for each mode
    std::map<firmata::mode, firmata::res> reses_;

    firmata::mode mode_;
    int value_ = 0;

    int state_ = 0;
    std::vector<state_callback> state_callback_;
    void change_state(int);

    ////////////////////
    struct
    {
        using pos = firmata::pos;
        using mode = firmata::mode;

        std::function<void(pin*, mode /*now*/, mode /*before*/)> pin_mode;
        std::function<void(pin*, bool)> digital_value;
        std::function<void(pin*, int)> analog_value;
    }
    delegate_;

    friend class control;
};

////////////////////////////////////////////////////////////////////////////////
inline void swap(pin& lhs, pin& rhs) noexcept { lhs.swap(rhs); }

////////////////////////////////////////////////////////////////////////////////
using pins = std::vector<pin>;

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
