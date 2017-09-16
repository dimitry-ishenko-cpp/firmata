////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
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
#include "firmata/types.hpp"

#include <bitset>
#include <chrono>
#include <map>

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
    auto const& protocol() const noexcept { return protocol_; }
    auto const& firmware() const noexcept { return firmware_; }

    void reset();

    template<typename Rep, typename Period>
    void sample_rate(const std::chrono::duration<Rep, Period>&);

    ////////////////////
    auto const& pins() const noexcept { return pins_; }

private:
    ////////////////////
    io::base* io_;

    firmata::protocol protocol_;
    firmata::firmware firmware_;
    firmata::pins pins_;

    // ports that are currently being monitored
    std::map<int, std::bitset<8>> ports_;

    ////////////////////
    // get specific reply discarding others
    payload read_until(msg_id);

    void query_version();
    void query_firmware();

    void query_capability();
    void query_analog_mapping();
    void query_state();

    void report_all();

    void fn_mode(pos, mode);
    void fn_value(pos, int);

    ////////////////////
    void pin_mode(pos, mode);

    void digital_value(pos, bool);
    void analog_value(pos, int);

    void report_digital(pos, bool);
    void report_analog(pos, bool);

    using msec = std::chrono::milliseconds;
    void sample_rate(const msec&);

    void reset_();

    void async_read(msg_id, const payload&);

    ////////////////////
    void info(); // for debugging
};

////////////////////////////////////////////////////////////////////////////////
template<typename Rep, typename Period>
void control::sample_rate(const std::chrono::duration<Rep, Period>& time)
{ sample_rate(std::chrono::duration_cast<msec>(time)); }

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
