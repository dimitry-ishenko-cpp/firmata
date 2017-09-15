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
    auto const& firmware() const { return firmware_; }

    void reset();

    ////////////////////
    auto pin_count() const noexcept { return pins_.size(); }

    auto const& pin(pos n) const { return pins_.at(n); }
    auto& pin(pos n) { return pins_.at(n); }

private:
    ////////////////////
    io::base* io_;
    firmata::firmware firmware_;
    std::vector<firmata::pin> pins_;

    ////////////////////
    // get specific reply discarding others
    payload read_until(msg_id);

    void async_read(msg_id, const payload&);

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
