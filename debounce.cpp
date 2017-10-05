////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "debounce.hpp"

#include <functional>
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
cid debounce::on_state_changed(firmata::pin& pin, pin::int_call fn)
{
    if(!io_) throw std::logic_error("Invalid state");

    cid id(0, id_++);
    chain_.emplace(id, std::unique_ptr<bounce>
        { new bounce(*io_, time_, pin, std::move(fn)) }
    );
    return id;
}

////////////////////////////////////////////////////////////////////////////////
cid debounce::on_state_low(firmata::pin& pin, pin::void_call fn)
{
    return on_state_changed(pin, [=](int state){ if(!state) fn(); });
}

////////////////////////////////////////////////////////////////////////////////
cid debounce::on_state_high(firmata::pin& pin, pin::void_call fn)
{
    return on_state_changed(pin, [=](int state){ if(state) fn(); });
}

////////////////////////////////////////////////////////////////////////////////
debounce::bounce::bounce(asio::io_service& io, const msec& time, firmata::pin& pin, pin::int_call fn) :
    pin_(pin), state_(pin_.state()), time_(time), timer_(io), fn_(std::move(fn))
{
    if(pin_.mode() == digital_in || pin_.mode() == pullup_in)
    {
        using namespace std::placeholders;
        id_ = pin_.on_state_changed(std::bind(&bounce::pin_state_changed, this, _1));
    }
    else throw std::invalid_argument("Invalid pin mode");
}

////////////////////////////////////////////////////////////////////////////////
debounce::bounce::~bounce() noexcept
{
    timer_.cancel();
    pin_.remove_call(id_);
}

////////////////////////////////////////////////////////////////////////////////
void debounce::bounce::pin_state_changed(int state)
{
    timer_.expires_from_now(time_);
    timer_.async_wait([=](const asio::error_code& ec)
    {
        if(ec != asio::error::operation_aborted && state != state_)
            fn_(state_ = state);
    });
}

////////////////////////////////////////////////////////////////////////////////
}
