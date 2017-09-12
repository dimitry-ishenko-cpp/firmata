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
enum msg_id : dword;

constexpr byte start_sysex = 0xf0;
constexpr byte end_sysex = 0xf7;

constexpr msg_id sysex(byte sysex_id) noexcept
{ return static_cast<msg_id>(dword(start_sysex) + (dword(sysex_id) << 8)); }

constexpr bool is_sysex(msg_id id) noexcept
{ return byte(id & 0xff) == start_sysex; }

constexpr msg_id ext_sysex(word ext_id) noexcept
{ return static_cast<msg_id>(dword(start_sysex) + (dword(ext_id) << 16)); }

constexpr bool is_ext_sysex(msg_id id) noexcept
{ return is_sysex(id) && 0 == ((id >> 8) & 0xff); }

// message id includes standard id,
// as well as optional sysex id and extended id
enum msg_id : dword
{
    version_query           = 0xf9,
    version_response        = version_query,

    reset                   = 0xff,

    analog_map_query        = sysex(0x69),
    analog_map_response     = sysex(0x6a),

    capabilities_query      = sysex(0x6b),
    capabilities_response   = sysex(0x6c),

    firmware_query          = sysex(0x79),
    firmware_response       = firmware_query,
};

// get message size based on whether
// itsa standard, sysex or extended sysex message
constexpr auto size(msg_id id) noexcept
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

// pin type
enum type { digital, analog };

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
