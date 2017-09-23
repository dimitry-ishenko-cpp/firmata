////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_CONTROL_HPP
#define FIRMATA_CONTROL_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/callback.hpp"
#include "firmata/io_base.hpp"
#include "firmata/pin.hpp"
#include "firmata/types.hpp"

#include <array>
#include <bitset>
#include <chrono>
#include <string>

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

    ~control() noexcept;

    control(const control&) = delete;
    control(control&&) = delete;

    control& operator=(const control&) = delete;
    control& operator=(control&&) = delete;

    ////////////////////
    auto const& protocol() const noexcept { return protocol_; }
    auto const& firmware() const noexcept { return firmware_; }

    void reset();

    using msec = std::chrono::milliseconds;

    template<typename Rep, typename Period>
    void sample_rate(const std::chrono::duration<Rep, Period>&);
    void sample_rate(const msec&);

    template<typename Rep, typename Period>
    static void timeout(const std::chrono::duration<Rep, Period>&);
    static void timeout(const msec& time) noexcept { timeout_ = time; }

    ////////////////////
    void string(const std::string&);
    auto const& string() const noexcept { return string_; }

    using string_callback = callback<void(const std::string&)>;
    int on_string_changed(string_callback fn) { return chain_.add(std::move(fn)); }
    void remove_callback(int id) { chain_.remove(id); }

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
    int id_;

    firmata::protocol protocol_;
    firmata::firmware firmware_;
    firmata::pins pins_;
    std::string string_;

    callback_chain<string_callback> chain_;
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

    void pin_mode(const firmata::pin&, mode now, mode before);

    void digital_value(const firmata::pin&, bool);
    void analog_value(const firmata::pin&, int);

    void report_digital(const firmata::pin&, bool);
    void report_analog(const firmata::pin&, bool);

    static msec timeout_;

    void async_read(msg_id, const payload&);
};

////////////////////////////////////////////////////////////////////////////////
template<typename Rep, typename Period>
void control::sample_rate(const std::chrono::duration<Rep, Period>& time)
{ sample_rate(std::chrono::duration_cast<msec>(time)); }

////////////////////////////////////////////////////////////////////////////////
template<typename Rep, typename Period>
void control::timeout(const std::chrono::duration<Rep, Period>& time)
{ timeout(std::chrono::duration_cast<msec>(time)); }

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
