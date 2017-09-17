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
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
class control
{
public:
    ////////////////////
    explicit control(io::base*);
    ~control() noexcept { io_->reset_async(); }

    ////////////////////
    auto const& protocol() const noexcept { return protocol_; }
    auto const& firmware() const noexcept { return firmware_; }

    void reset();

    void string(const std::string&);
    auto const& string() const noexcept { return string_; }

    template<typename Rep, typename Period>
    void sample_rate(const std::chrono::duration<Rep, Period>&);

    ////////////////////
    auto pin_begin() noexcept { return pins_.begin(); }
    auto pin_begin() const noexcept { return pins_.begin(); }
    auto pin_cbegin() const noexcept { return pins_.cbegin(); }

    auto pin_rbegin() noexcept { return pins_.rbegin(); }
    auto pin_rbegin() const noexcept { return pins_.rbegin(); }
    auto pin_crbegin() const noexcept { return pins_.crbegin(); }

    auto pin_end() noexcept { return pins_.end(); }
    auto pin_end() const noexcept { return pins_.end(); }
    auto pin_cend() const noexcept { return pins_.cend(); }

    auto pin_rend() noexcept { return pins_.rend(); }
    auto pin_rend() const noexcept { return pins_.rend(); }
    auto pin_crend() const noexcept { return pins_.crend(); }

    ////////////////////
    auto pin_count() const noexcept { return pins_.size(); }
    auto& pin(pos n) { return pins_.at(n); }
    auto const& pin(pos n) const { return pins_.at(n); }

    std::size_t analog_count() const;
    firmata::pin& analog(pos);
    const firmata::pin& analog(pos) const;

private:
    ////////////////////
    io::base* io_;

    firmata::protocol protocol_;
    firmata::firmware firmware_;
    firmata::pins pins_;
    std::string string_;

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

    void pin_mode(pos, mode);
    void pin_value(pos, int);

    void report_digital(pos, bool);
    void report_analog(pos, bool);

    using msec = std::chrono::milliseconds;
    void sample_rate(const msec&);

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
