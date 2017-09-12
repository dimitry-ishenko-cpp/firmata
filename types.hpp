////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL v3.
// For full terms see COPYING or visit https://www.gnu.org/licenses/gpl.html

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_TYPES_HPP
#define FIRMATA_TYPES_HPP

////////////////////////////////////////////////////////////////////////////////
#include <cstdint>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
// basic types
using  byte = std::uint8_t;
using  word = std::uint16_t;
using dword = std::uint32_t;

////////////////////////////////////////////////////////////////////////////////
enum type : dword;

constexpr byte start_sysex = 0xf0;
constexpr byte end_sysex = 0xf7;

constexpr type sysex(byte id) noexcept
{ return static_cast<type>(dword(start_sysex) + (dword(id) << 8)); }

constexpr bool is_sysex(type id) noexcept
{ return byte(id & 0xff) == start_sysex; }

constexpr type ext_sysex(word id) noexcept
{ return static_cast<type>(dword(start_sysex) + (dword(id) << 16)); }

constexpr bool is_ext_sysex(type id) noexcept
{ return is_sysex(id) && 0 == ((id >> 8) & 0xff); }

// message type includes message id,
// and optional sysex id and extended id
enum type : dword
{
    ver_query   = 0xf9,
    ver_reply   = ver_query,

    reset       = 0xff,

    caps_query  = sysex(0x6b),
    caps_reply  = sysex(0x6c),

    fw_query    = sysex(0x79),
    fw_reply    = fw_query,
};

// get message size based on whether
// itsa standard, sysex or extended sysex message
constexpr auto size(type id) noexcept
{ return is_ext_sysex(id) ? sizeof(dword) : is_sysex(id) ? sizeof(word) : sizeof(byte); }

////////////////////////////////////////////////////////////////////////////////
// message data
using payload = std::vector<byte>;

// convert 7-bit message data to string
std::string to_string(payload::const_iterator begin, payload::const_iterator end);

////////////////////////////////////////////////////////////////////////////////
// number of bits (eg, resolution, etc.)
enum bits : byte { };

namespace literals
{
    constexpr bits operator"" _bits(unsigned long long n) noexcept
    { return static_cast<bits>(n); }
}

////////////////////////////////////////////////////////////////////////////////
// pin number
using pos = byte;
constexpr pos invalid = -1;

// pin mode
enum mode : byte
{
    digital_in  =  0,
    digital_out =  1,
    analog_in   =  2,
    pwm         =  3,
    servo       =  4,
    shift       =  5,
    i2c         =  6,
    onewire     =  7,
    stepper     =  8,
    encoder     =  9,
    serial      = 10,
    pullup_in   = 11,
};

// mode resolution
using res = bits;

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
