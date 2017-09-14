////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL v3.
// For full terms see COPYING or visit https://www.gnu.org/licenses/gpl.html

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_CONTROL_HPP
#define FIRMATA_CONTROL_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/io.hpp"
#include "firmata/pin.hpp"

#include <tuple>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
class control
{
public:
    ////////////////////
    explicit control(io::base*);

    ////////////////////
    auto const& version() const { return version_; }
    auto const& firmware() const { return firmware_; }

    void reset();

    ////////////////////
    auto pin_count() const noexcept { return pins_.size(); }

    auto const& pin(pos n) const { return pins_.at(n); }
    auto& pin(pos n) { return pins_.at(n); }

    auto analog_count() const noexcept { return analog_.size(); }

    auto const& analog(pos n) const { return *analog_.at(n); }
    auto& analog(pos n) { return *analog_.at(n); }

private:
    ////////////////////
    io::base* io_;

    std::tuple<int, int> version_ { 0, 0 };
    std::string firmware_;

    std::vector<firmata::pin> pins_;

    // analog to digital pin map
    using iterator = decltype(pins_)::iterator;
    std::vector<iterator> analog_;

    ////////////////////
    // get specific reply discarding others
    payload read_until(msg_id);

    void query_firmware();
    void query_capability();
    void query_analog_mapping();
    void query_state();

    // for debugging
    void info();
};


////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
