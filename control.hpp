////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_CONTROL_HPP
#define FIRMATA_CONTROL_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/io_base.hpp"
#include "firmata/pin.hpp"
#include "firmata/types.hpp"

#include <array>
#include <bitset>
#include <chrono>
#include <functional>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
enum dont_reset_t { dont_reset };

////////////////////////////////////////////////////////////////////////////////
class control
{
public:
    ////////////////////
    explicit control(io_base* io) : control(io, false) { }
    control(io_base* io, dont_reset_t) : control(io, true) { }
    ~control() noexcept { io_->read_callback(nullptr); }

    ////////////////////
    auto const& protocol() const noexcept { return protocol_; }
    auto const& firmware() const noexcept { return firmware_; }

    void reset();

    using msec = std::chrono::milliseconds;

    template<typename Rep, typename Period>
    void sample_rate(const std::chrono::duration<Rep, Period>&);
    void sample_rate(const msec&);

    ////////////////////
    void string(const std::string&);
    auto const& string() const noexcept { return string_; }

    using string_callback = std::function<void(const std::string&)>;
    void on_string_changed(string_callback fn);

    ////////////////////
    auto pin_begin() noexcept { return pins_.begin(); }
    auto pin_begin() const noexcept { return pins_.begin(); }

    auto pin_end() noexcept { return pins_.end(); }
    auto pin_end() const noexcept { return pins_.end(); }

    ////////////////////
    auto pin_count() const noexcept { return pins_.size(); }
    auto& pin(pos n) { return pins_.at(n); }
    auto const& pin(pos n) const { return pins_.at(n); }

    auto& pin(analog n) { return pin(analog_in, n); }
    auto const& pin(analog n) const { return pin(analog_in, n); }

    std::size_t pin_count(mode) const noexcept;
    firmata::pin& pin(mode, pos);
    const firmata::pin& pin(mode, pos) const;

    ////////////////////
    void info(); // for debugging

private:
    ////////////////////
    control(io_base*, bool dont_reset);

    io_base* io_;

    firmata::protocol protocol_;
    firmata::firmware firmware_;
    firmata::pins pins_;
    std::string string_;

    std::vector<string_callback> string_callback_;
    void change_string(std::string);

    // ports that are currently being monitored
    std::array<std::bitset<8>, port_count> ports_;

    ////////////////////
    payload wait_until(msg_id);

    void query_version();
    void query_firmware();

    void query_capability();
    void query_analog_mapping();
    void query_state();

    void report_all();

    void pin_mode(firmata::pin*, mode now, mode before);

    void digital_value(firmata::pin*, bool);
    void analog_value(firmata::pin*, int);

    void report_digital(firmata::pin*, bool);
    void report_analog(firmata::pin*, bool);

    void async_read(msg_id, const payload&);
};

////////////////////////////////////////////////////////////////////////////////
template<typename Rep, typename Period>
void control::sample_rate(const std::chrono::duration<Rep, Period>& time)
{ sample_rate(std::chrono::duration_cast<msec>(time)); }

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
