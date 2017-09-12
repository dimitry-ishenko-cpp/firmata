////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL v3.
// For full terms see COPYING or visit https://www.gnu.org/licenses/gpl.html

////////////////////////////////////////////////////////////////////////////////
#include "firmata/control.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
control::control(io::base* io) : io_(io)
{
    ////////////////////
    // get version and firmware
    io_->send(firmware_query);
    payload data = get(firmware_response);

    version_ = std::make_tuple(data[0], data[1]);
    firmware_ = to_string(data.begin() + 2, data.end());

    ////////////////////
    // get capabilities
    io_->send(capabilities_query);
    data = get(capabilities_response);

    firmata::pin pin;
    firmata::pos pos = 0;
    for(auto ci = data.begin(); ci < data.end(); ++ci)
    {
        if(*ci == 0x7f)
        {
            pin.digital_ = pos;
            pins_.push_back(std::move(pin));

            pin = firmata::pin(); ++pos;
        }
        else pin.modes_.emplace(mode(*ci), res(*++ci));
    }

    ////////////////////
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
void control::query_state()
{
}

////////////////////////////////////////////////////////////////////////////////
}
