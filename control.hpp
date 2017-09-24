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
#include "firmata/command.hpp"
#include "firmata/io_base.hpp"
#include "firmata/pin.hpp"
#include "firmata/types.hpp"

#include <algorithm>
#include <chrono>
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
    ~control() noexcept;

    control(const control&) = delete;
    control(control&&) = delete;

    control& operator=(const control&) = delete;
    control& operator=(control&&) = delete;

    ////////////////////
    auto const& protocol() const noexcept { return protocol_; }
    auto const& firmware() const noexcept { return firmware_; }

    void reset();

    template<typename Rep, typename Period>
    static void timeout(const std::chrono::duration<Rep, Period>&);
    static void timeout(const msec& time) noexcept { time_ = time; }

    static auto const& timeout() noexcept { return time_; }

    ////////////////////
    // send string to host
    void string(const std::string& s) { cmd_.string(s); }

    // last string received from host
    auto const& string() const noexcept { return string_; }

    using string_callback = callback<void(const std::string&)>;
    int on_string_changed(string_callback fn) { return chain_.add(std::move(fn)); }
    void remove_callback(int id) { chain_.remove(id); }

    ////////////////////
    auto& pins() const noexcept { return proxy_; }

    auto& pin(pos n) { return pins_.at(n); }
    auto const& pin(pos n) const { return pins_.at(n); }

    auto& pin(analog n) { return pin(analog_in, n); }
    auto const& pin(analog n) const { return pin(analog_in, n); }

    firmata::pin& pin(mode, pos);
    const firmata::pin& pin(mode, pos) const;

    ////////////////////
    void info(); // for debugging

private:
    ////////////////////
    control(io_base*, bool dont_reset);

    io_base* io_;
    int id_;
    command cmd_;

    firmata::protocol protocol_;
    firmata::firmware firmware_;

    std::vector<firmata::pin> pins_;

    struct proxy
    {
        auto begin() const noexcept { return parent->pins_.begin(); }
        auto end() const noexcept { return parent->pins_.begin(); }

        auto count() const noexcept { return parent->pins_.size(); }
        auto count(mode n) const noexcept
        {
            return std::count_if(begin(), end(),
                [&](auto& pin){ return pin.supports(n); }
            );
        }

        control* parent;
    }
    proxy_ { this };

    std::string string_;
    callback_chain<string_callback> chain_;

    void async_read(msg_id, const payload&);

    static msec time_;
};

////////////////////////////////////////////////////////////////////////////////
template<typename Rep, typename Period>
void control::timeout(const std::chrono::duration<Rep, Period>& time)
{ timeout(std::chrono::duration_cast<msec>(time)); }

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
