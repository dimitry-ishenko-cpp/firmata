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
#include <asio/io_service.hpp>
#include <asio/serial_port.hpp>
#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
using  byte = std::uint8_t;
using  word = std::uint16_t;
using dword = std::uint32_t;

enum type : dword;

constexpr byte start_sysex = 0xf0;
constexpr byte end_sysex = 0xf7;
constexpr byte ext_id = 0x00;

////////////////////////////////////////////////////////////////////////////////
constexpr type sysex(byte id) noexcept
{ return static_cast<type>(dword(start_sysex) + (dword(id) << 8)); }

constexpr bool is_sysex(type id) noexcept
{ return byte(id & 0xff) == start_sysex; }

constexpr type ext_sysex(word id) noexcept
{ return static_cast<type>(dword(start_sysex) + (dword(ext_id) << 8) + (dword(id) << 16)); }

constexpr bool is_ext_sysex(type id) noexcept
{ return is_sysex(id) && byte((id >> 8) & 0xff) == ext_id; }

////////////////////////////////////////////////////////////////////////////////
enum type : dword
{
    ver_query   = 0xf9,
    ver_reply   = ver_query,

    reset       = 0xff,

    fw_query    = sysex(0x79),
    fw_reply    = fw_query,
};

using payload = std::vector<byte>;

std::string to_string(payload::const_iterator begin, payload::const_iterator end);

////////////////////////////////////////////////////////////////////////////////
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
