////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL v3.
// For full terms see COPYING or visit https://www.gnu.org/licenses/gpl.html

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_PIN_HPP
#define FIRMATA_PIN_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/types.hpp"

#include <map>
#include <set>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
class pin
{
public:
    ////////////////////
    pin(const pin&) = delete;
    pin(pin&& other) noexcept { swap(other); }

    pin& operator=(const pin&) = delete;
    pin& operator=(pin&& other) noexcept { swap(other); return *this; }

    void swap(pin& other) noexcept
    {
        using std::swap;
        swap(digital_, other.digital_);
        swap(analog_,  other.analog_ );
        swap(modes_,   other.modes_  );
        swap(mode_,    other.mode_   );
        swap(reses_,   other.reses_  );
        swap(value_,   other.value_  );
        swap(state_,   other.state_  );
    }

    ////////////////////
    firmata::pos digital() const noexcept { return digital_; }
    firmata::pos analog() const noexcept { return analog_; }

    auto const& modes() const noexcept { return modes_; }
    bool supports(firmata::mode n) const noexcept { return modes_.count(n); }

    auto mode() const noexcept { return mode_; }
    auto res() const noexcept { return reses_.at(mode_); }

    auto value() const noexcept { return value_; }
    auto state() const noexcept { return state_; }

private:
    ////////////////////
    firmata::pos digital_ = npos, analog_ = npos;

    std::set<firmata::mode> modes_;
    firmata::mode mode_;

    // resolution for each mode
    std::map<firmata::mode, firmata::res> reses_;

    int value_ = 0;
    int state_ = 0;

    ////////////////////
    pin() noexcept = default;
    friend class control;
};

////////////////////////////////////////////////////////////////////////////////
inline void swap(pin& lhs, pin& rhs) noexcept { lhs.swap(rhs); }

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
