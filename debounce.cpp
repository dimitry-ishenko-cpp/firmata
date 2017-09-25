////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "debounce.hpp"
#include <functional>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
constexpr msec debounce::delay;

////////////////////////////////////////////////////////////////////////////////
int debounce::on_state_changed(firmata::pin& pin, state_callback fn)
{
    map_.emplace(id_, std::make_unique<debounced>(io_, pin, std::move(fn), delay_));
    return id_++;
}

////////////////////////////////////////////////////////////////////////////////
void debounce::remove_callback(int id) { map_.erase(id); }

////////////////////////////////////////////////////////////////////////////////
debounce::debounced::debounced(asio::io_service& io, firmata::pin& pin, state_callback fn, msec& delay) :
    pin_(pin), state_(pin_.state()), fn_(std::move(fn)), timer_(io), delay_(delay)
{
    using namespace std::placeholders;
    id_ = pin_.on_state_changed(std::bind(&debounced::pin_state_changed, this, _1));
}

////////////////////////////////////////////////////////////////////////////////
debounce::debounced::~debounced() noexcept
{
    timer_.cancel();
    pin_.remove_callback(id_);
}

////////////////////////////////////////////////////////////////////////////////
void debounce::debounced::pin_state_changed(int)
{
    timer_.expires_from_now(delay_);
    timer_.async_wait([&](const asio::error_code& ec)
    {
        if(ec != asio::error::operation_aborted && pin_.state() != state_)
            fn_(state_ = pin_.state());
    });
}

////////////////////////////////////////////////////////////////////////////////
}
