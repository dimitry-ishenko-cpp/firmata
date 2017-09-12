////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL v3.
// For full terms see COPYING or visit https://www.gnu.org/licenses/gpl.html

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_IO_HPP
#define FIRMATA_IO_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/types.hpp"

#include <asio/io_service.hpp>
#include <asio/serial_port.hpp>
#include <string>
#include <tuple>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{
namespace io
{

////////////////////////////////////////////////////////////////////////////////
class base
{
public:
    ////////////////////
    virtual void send(type, const payload& = { }) = 0;
    virtual std::tuple<type, payload> recv() = 0;
};

////////////////////////////////////////////////////////////////////////////////
enum baud_rate : unsigned { };

using flow_control = asio::serial_port::flow_control::type;
using parity = asio::serial_port::parity::type;
using stop_bits = asio::serial_port::stop_bits::type;

enum char_size : unsigned { };

////////////////////////////////////////////////////////////////////////////////
namespace literals
{

////////////////////////////////////////////////////////////////////////////////
constexpr baud_rate operator"" _baud(unsigned long long n) noexcept
{ return static_cast<baud_rate>(n); }

constexpr char_size operator"" _bits(unsigned long long n) noexcept
{ return static_cast<char_size>(n); }

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
class serial : public base
{
public:
    ////////////////////
    serial(asio::io_service& io, const std::string& device);

    void set(baud_rate);
    void set(flow_control);
    void set(parity);
    void set(stop_bits);
    void set(char_size);

    virtual void send(type, const payload& = { }) override;
    virtual std::tuple<type, payload> recv() override;

private:
    ////////////////////
    asio::serial_port port_;
};

////////////////////////////////////////////////////////////////////////////////
}
}

////////////////////////////////////////////////////////////////////////////////
#endif
