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
    debounce(asio::io_service& io, const std::chrono::duration<Rep, Period>& time) :
        debounce(io, std::chrono::duration_cast<msec>(time))
    { }

    explicit debounce(asio::io_service& io, const msec& time = msec(5)) :
        io_(io), time_(time)
    { }

    debounce(const debounce&) = delete;
    debounce(debounce&&) = delete;

    debounce& operator=(const debounce&) = delete;
    debounce& operator=(debounce&&) = delete;

    ////////////////////
    cid on_state_changed(pin&, pin::int_call);
    cid on_state_low(pin&, pin::void_call);
    cid on_state_high(pin&, pin::void_call);

    void remove_callback(cid);

private:
    ////////////////////
    asio::io_service& io_;
    msec time_;

    struct bounce
    {
        ////////////////////
        bounce(asio::io_service&, msec&, pin&, pin::int_call);
        ~bounce() noexcept;

    private:
        ////////////////////
        pin& pin_; int state_; cid id_;

        msec time_;
        asio::system_timer timer_;

        pin::int_call fn_;
        void pin_state_changed(int);
    };

    std::map<cid, std::unique_ptr<bounce>> chain_;
    int id_ = 0;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
