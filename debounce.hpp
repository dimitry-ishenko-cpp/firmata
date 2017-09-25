////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_DEBOUNCE_HPP
#define FIRMATA_DEBOUNCE_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/pin.hpp"
#include "firmata/types.hpp"

#include "asio_or_boost.hpp"
#include <chrono>
#include <map>
#include <memory>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
class debounce
{
public:
    ////////////////////
    template<typename Rep, typename Period>
    debounce(asio::io_service& io, const std::chrono::duration<Rep, Period>& delay) :
        debounce(io, std::chrono::duration_cast<msec>(delay))
    { }

    explicit debounce(asio::io_service& io, const msec& time = delay) :
        io_(io), delay_(time)
    { }

    debounce(const debounce&) = delete;
    debounce(debounce&&) = delete;

    debounce& operator=(const debounce&) = delete;
    debounce& operator=(debounce&&) = delete;

    ////////////////////
    using state_callback = pin::state_callback;
    int on_state_changed(pin&, state_callback);
    void remove_callback(int);

    static constexpr msec delay { 5 };

private:
    ////////////////////
    asio::io_service& io_;
    msec delay_;

    class debounced
    {
    public:
        ////////////////////
        debounced(asio::io_service&, pin&, state_callback, msec&);
        ~debounced() noexcept;

    private:
        ////////////////////
        pin& pin_;
        int id_;
        int state_;

        state_callback fn_;
        void pin_state_changed(int);

        asio::system_timer timer_;
        msec delay_;
    };

    // NB: using smart pointer due to asio::system_timer
    // not supporting move semantics
    std::map<int, std::unique_ptr<debounced>> map_;
    int id_ = 0;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
