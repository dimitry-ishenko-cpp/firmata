////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL v3.
// For full terms see COPYING or visit https://www.gnu.org/licenses/gpl.html

////////////////////////////////////////////////////////////////////////////////
#include "firmata/control.hpp"
#include <algorithm>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
control::control(io::base* io) : io_(io)
{
    query_firmware();
    query_capabilities();
    query_analog_map();
    query_state();
}

////////////////////////////////////////////////////////////////////////////////
void control::reset()
{
    io_->send(firmata::reset);
    query_state();
}

////////////////////////////////////////////////////////////////////////////////
payload control::get(msg_id reply_id)
{
    msg_id id;
    payload data;

    do { std::tie(id, data) = io_->recv(); }
    while(id != reply_id);

    return data;
}

////////////////////////////////////////////////////////////////////////////////
void control::query_firmware()
{
    io_->send(firmware_query);
    auto data = get(firmware_response);

    assert(data.size() >= 2);

    version_ = std::make_tuple(data[0], data[1]);
    firmware_ = to_string(data.begin() + 2, data.end());
}

////////////////////////////////////////////////////////////////////////////////
void control::query_capabilities()
{
    io_->send(capabilities_query);
    auto data = get(capabilities_response);

    firmata::pin pin;
    firmata::pos pos = 0;
    for(auto ci = data.begin(); ci < data.end(); ++ci)
        if(*ci == 0x7f)
        {
            pin.digital_ = pos;
            pins_.push_back(std::move(pin));

            pin = firmata::pin(); ++pos;
        }
        else
        {
            pin.modes_.insert(mode(*ci));
            pin.reses_.emplace(mode(*ci), res(*++ci));
        }

    assert(pin.modes_.empty());
}

////////////////////////////////////////////////////////////////////////////////
void control::query_analog_map()
{
    io_->send(analog_map_query);
    auto data = get(analog_map_response);

    pos size = 0;
    analogs_.resize(pins_.size(), pins_.end());

    auto pi = pins_.begin();
    for(auto ci = data.begin(); ci < data.end(); ++ci, ++pi)
        if(*ci != 0x7f)
        {
            assert(pi < pins_.end());
            assert(*ci < analogs_.size());

            pi->analog_ = *ci;
            analogs_[*ci] = pi;

            size = std::max(size, *ci);
        }

    analogs_.resize(size);
    for(auto pi : analogs_) assert(pi != pins_.end());
}

////////////////////////////////////////////////////////////////////////////////
void control::query_state()
{
    for(auto& pin : pins_)
    {
        io_->send(pin_state_query, { pin.pos(digital) });
        auto data = get(pin_state_response);

        assert(data.size() >= 3);
        assert(data[0] == pin.pos(digital));

        pin.mode_ = static_cast<mode>(data[1]);
        pin.state_ = to_value(data.begin() + 2, data.end());
    }
}

////////////////////////////////////////////////////////////////////////////////
}
