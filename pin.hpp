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
#include "firmata/pin_base.hpp"
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
class pin : private pin_base
{
public:
    ////////////////////
    pin() noexcept = default;
    pin(const pin&) = delete;
    pin(pin&&) noexcept = default;

    pin& operator=(const pin&) = delete;
    pin& operator=(pin&&) noexcept = default;

    ////////////////////
    auto digital() const noexcept { return digital_; }
    auto analog() const noexcept { return analog_; }

    auto const& modes() const noexcept { return modes_; }
    bool supports(firmata::mode mode) const noexcept { return modes_.count(mode); }

    auto mode() const noexcept { return mode_; }
    void mode(firmata::mode mode) { set_mode(mode); }

    auto res() const noexcept { return reses_.at(mode_); }

    auto value() const noexcept { return value_; }
    void value(int n) { set_value(n); }

    auto state() const noexcept { return state_; }
};

////////////////////////////////////////////////////////////////////////////////
using pins = std::vector<pin>;

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
