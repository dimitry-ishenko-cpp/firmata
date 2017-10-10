////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_CONTROL_HPP
#define FIRMATA_CONTROL_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/call_chain.hpp"
#include "firmata/io_base.hpp"
#include "firmata/pins.hpp"
#include "firmata/types.hpp"

#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <string>
#include <stdexcept>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
struct timeout_error : public std::runtime_error { using std::runtime_error::runtime_error; };

namespace literals { enum dont_reset_t { dont_reset }; }
using namespace literals;

////////////////////////////////////////////////////////////////////////////////
class control
{
public:
    ////////////////////
    control() = default;
    explicit control(io_base& io) : control(&io, false) { }
    control(io_base& io, dont_reset_t) : control(&io, true) { }
    ~control() noexcept;

    control(const control&) = delete;
    control(control&& rhs) noexcept { swap(rhs); }

    control& operator=(const control&) = delete;
    control& operator=(control&& rhs) noexcept { swap(rhs); return *this; }

    void swap(control&) noexcept;

    ////////////////////
    bool valid() const noexcept { return io_; }
    explicit operator bool() const noexcept { return valid(); }

    ////////////////////
    // protocol version
    auto const& protocol() const noexcept { return protocol_; }
    // firmware name & version
    auto const& firmware() const noexcept { return firmware_; }

    // reset host
    void reset();

    // set new read timeout
    template<typename Rep, typename Period>
    static void timeout(const std::chrono::duration<Rep, Period>&);
    static void timeout(const msec& time) noexcept { time_ = time; }

    // current read timeout
    static auto const& timeout() noexcept { return time_; }

    ////////////////////
    // send string to host
    void string(const std::string& s);

    // last string received from host
    auto const& string() const noexcept { return string_; }

    using string_call = call<void(const std::string&)>;

    // install string changed callback
    cid on_string_changed(string_call fn) { return chain_.insert(std::move(fn)); }

    // remove callback
    bool remove_call(cid id) { return chain_.erase(id); }

    ////////////////////
    // get all pins (for use in range-based "for" loops)
    auto& pins() noexcept { return pins_; }
    auto const& pins() const noexcept { return pins_; }

    // get pin
    auto& pin(pos n) { return pins_.get(n); }
    auto const& pin(pos n) const { return pins_.get(n); }

    // get analog pin
    auto& pin(analog n) { return pins_.get(analog_in, n); }
    auto const& pin(analog n) const { return pins_.get(analog_in, n); }

    // get pin that supports certain mode
    auto& pin(mode m, pos n) { return pins_.get(m, n); }
    auto const& pin(mode m, pos n) const { return pins_.get(m, n); }

    ////////////////////
    // print out host info (for debugging only)
    void info();

private:
    ////////////////////
    control(io_base*, bool dont_reset);

    io_base* io_ = nullptr;
    cid id_;

    firmata::protocol protocol_ { 0, 0 };
    firmata::firmware firmware_ { 0, 0 };

    firmata::pins pins_;
    firmata::pin::delegate delegate_;

    std::string string_;
    call_chain<string_call> chain_;

    static msec time_;

    ////////////////////
    // enable/disable reporting for a digital pin
    void report_digital(pos, bool);
    // enable/disable reporting for an analog pin
    void report_analog(pos, bool);

    // set digital pin value
    void digital_value(pos, bool);
    // set analog pin value
    void analog_value(pos, int);

    // set pin mode
    void pin_mode(pos, mode);

    // reset host
    void reset_();

    // query protocol version
    void query_version();
    // query firmware name & version
    void query_firmware();
    // query capability (pins, modes and reses)
    void query_capability();
    // query analog pin mapping
    void query_analog_mapping();
    // query current pin state
    void query_state();

    // enable reporting for all inputs
    // and disable for all outputs
    void set_report();

    // read messages from host
    void async_read(msg_id, const payload&);

    ////////////////////
    // ports that are currently being monitored
    std::array<std::bitset<8>, port_count> ports_;

    // wait for specific message
    payload wait_until(msg_id);
};

////////////////////////////////////////////////////////////////////////////////
template<typename Rep, typename Period>
inline void
control::timeout(const std::chrono::duration<Rep, Period>& time)
{ timeout(std::chrono::duration_cast<msec>(time)); }

////////////////////////////////////////////////////////////////////////////////
inline void swap(control& lhs, control& rhs) noexcept { lhs.swap(rhs); }

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
