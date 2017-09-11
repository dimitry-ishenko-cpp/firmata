////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL v3.
// For full terms see COPYING or visit https://www.gnu.org/licenses/gpl.html

////////////////////////////////////////////////////////////////////////////////
#include "firmata/io.hpp"

#include <asio.hpp>
#include <iterator>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
std::string to_string(payload::const_iterator begin, payload::const_iterator end)
{
    std::string s;
    for(auto pos = begin; pos < std::prev(end); ++pos)
        s += char(*pos + (*(++pos) << 7));
    return s;
}

////////////////////////////////////////////////////////////////////////////////
namespace io
{

////////////////////////////////////////////////////////////////////////////////
serial::serial(asio::io_service& io, const std::string& device) :
    port_(io, device)
{ }

////////////////////////////////////////////////////////////////////////////////
void serial::set(baud_rate baud)
{
    port_.set_option(asio::serial_port::baud_rate(baud));
}

////////////////////////////////////////////////////////////////////////////////
void serial::set(flow_control flow)
{
    port_.set_option(asio::serial_port::flow_control(flow));
}

////////////////////////////////////////////////////////////////////////////////
void serial::set(parity pari)
{
    port_.set_option(asio::serial_port::parity(pari));
}

////////////////////////////////////////////////////////////////////////////////
void serial::set(stop_bits bits)
{
    port_.set_option(asio::serial_port::stop_bits(bits));
}

////////////////////////////////////////////////////////////////////////////////
void serial::set(char_size bits)
{
    port_.set_option(asio::serial_port::character_size(bits));
}

////////////////////////////////////////////////////////////////////////////////
void serial::send(type id, const payload& data)
{
    std::vector<asio::const_buffer> message;

    message.push_back(asio::buffer(&id,
        is_ext_sysex(id) ? sizeof(dword)
                         : is_sysex(id) ? sizeof(word)
                                        : sizeof(byte))
    );
    if(data.size()) message.push_back(asio::buffer(data));
    if(is_sysex(id)) message.push_back(asio::buffer(&end_sysex, sizeof(byte)));

    asio::write(port_, message);
}

////////////////////////////////////////////////////////////////////////////////
std::tuple<type, payload> serial::recv()
{
    type id;
    payload message;

    // get message id
    asio::read(port_, asio::buffer(&id, sizeof(byte)));

    // if this is a sysex message, get sysex id
    if(is_sysex(id))
    {
        byte sid;
        asio::read(port_, asio::buffer(&sid, sizeof(sid)));
        id = sysex(sid);

        // if this is an extended sysex message, get extended id
        if(is_ext_sysex(id))
        {
            word eid;
            asio::read(port_, asio::buffer(&eid, sizeof(eid)));
            id = ext_sysex(eid);
        }

        // get payload
        asio::streambuf sb;
        asio::read_until(port_, sb, end_sysex);

        message.insert(message.end(),
            asio::buffers_begin(sb.data()),
            asio::buffers_end(sb.data())
        );
        message.pop_back(); // remove end_sysex
    }
    else
    {
        // get payload
        message.resize(2);
        asio::read(port_, asio::buffer(message));
    }

    return std::make_tuple(id, std::move(message));
}

////////////////////////////////////////////////////////////////////////////////
}
}
