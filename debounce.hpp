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
#include <utility>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
// Pin debouncer
//
class debounce
{
public:
    ////////////////////
    debounce() noexcept = default;

    template<typename Rep, typename Period>
    debounce(asio::io_service& io, const std::chrono::duration<Rep, Period>& time) :
        debounce(io, std::chrono::duration_cast<msec>(time))
    { }

    explicit debounce(asio::io_service& io, const msec& time = msec(5)) :
        io_(&io), time_(time)
    { }

    debounce(const debounce&) = delete;
    debounce(debounce&& rhs) noexcept { swap(rhs); }

    debounce& operator=(const debounce&) = delete;
    debounce& operator=(debounce&& rhs) noexcept { swap(rhs); return *this; }

    void swap(debounce& rhs) noexcept
    {
        using std::swap;
        swap(io_   , rhs.io_   );
        swap(time_ , rhs.time_ );
        swap(chain_, rhs.chain_);
        swap(id_   , rhs.id_   );
   }

    ////////////////////
    // install state changed/low/high callback
    cid on_state_changed(pin&, pin::int_call);
    cid on_state_low(pin&, pin::void_call);
    cid on_state_high(pin&, pin::void_call);

    // remove callback
    bool remove_call(cid id) { return chain_.erase(id); }

private:
    ////////////////////
    asio::io_service* io_ = nullptr;
    msec time_;

    struct bounce
    {
        ////////////////////
        bounce(asio::io_service&, const msec&, pin&, pin::int_call);
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
inline void swap(debounce& lhs, debounce& rhs) noexcept { lhs.swap(rhs); }

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
