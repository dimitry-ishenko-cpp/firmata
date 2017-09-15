////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL v3.
// For full terms see COPYING or visit https://www.gnu.org/licenses/gpl.html

////////////////////////////////////////////////////////////////////////////////
#include "command.hpp"
#include <cassert>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
protocol command::query_version()
{
    io_->write(version);
    auto data = read_until(version);

    assert(data.size() == 2);

    return protocol { data[0], data[1] };
}

////////////////////////////////////////////////////////////////////////////////
firmware command::query_firmware()
{
    io_->write(firmware_query);
    auto data = read_until(firmware_response);

    assert(data.size() >= 2);

    return firmware
    {
        data[0], data[1],
        to_string(data.begin() + 2, data.end())
    };
}

////////////////////////////////////////////////////////////////////////////////
pins command::query_capability()
{
    io_->write(capability_query);
    auto data = read_until(capability_response);

    pos digital = 0;
    firmata::pin pin(digital);
    pins all;

    for(auto ci = data.begin(); ci < data.end(); ++ci)
        if(*ci == 0x7f)
        {
            all.push_back(firmata::pin(++digital));

            using std::swap;
            swap(pin, all.back());
        }
        else
        {
            auto mode = static_cast<firmata::mode>(*ci);
            auto res = static_cast<firmata::res>(*++ci);

            pin.modes_.insert(mode);
            pin.reses_.emplace(mode, res);
        }

    // no garbage at end
    assert(pin.modes_.empty());

    return all;
}

////////////////////////////////////////////////////////////////////////////////
void command::query_analog_mapping(pins& all)
{
    io_->write(analog_mapping_query);
    auto data = read_until(analog_mapping_response);

    auto pi = all.begin();
    for(auto ci = data.begin(); ci < data.end(); ++ci, ++pi)
        if(*ci != 0x7f)
        {
            assert(pi < all.end());
            pi->analog_ = *ci;
        }
}

////////////////////////////////////////////////////////////////////////////////
void command::query_state(pins& all)
{
    for(auto& pin : all)
    {
        io_->write(pin_state_query, { pin.digital() });
        auto data = read_until(pin_state_response);

        assert(data.size() >= 3);
        assert(data[0] == pin.digital());

        pin.mode_ = static_cast<mode>(data[1]);
        pin.state_ = to_value(data.begin() + 2, data.end());
    }
}

////////////////////////////////////////////////////////////////////////////////
void command::reset() { io_->write(firmata::reset); }

////////////////////////////////////////////////////////////////////////////////
payload command::read_until(msg_id reply_id)
{
    msg_id id;
    payload data;

    do { std::tie(id, data) = io_->read(); }
    while(id != reply_id);

    return data;
}

////////////////////////////////////////////////////////////////////////////////
}
