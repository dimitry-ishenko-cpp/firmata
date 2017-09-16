////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
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

constexpr bool on = true;
constexpr bool off = false;

// number of bits (eg, res, char_size, etc.)
enum bits : byte { };

namespace literals
{
    constexpr bits operator"" _bits(unsigned long long n) noexcept
    { return static_cast<bits>(n); }
}

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
    no_id                   = 0,

    port_value_base         = 0x90,
    port_value_end          = 0x9f + 1,

    report_analog_base      = 0xc0,
    report_analog_end       = 0xcf + 1,

    report_port_base        = 0xd0,
    report_port_end         = 0xdf + 1,

    analog_value_base       = 0xe0,
    analog_value_end        = 0xef + 1,

    pin_mode                = 0xf4,
    digital_value           = 0xf5,

    version                 = 0xf9,
    reset                   = 0xff,

    analog_mapping_query    = sysex(0x69),
    analog_mapping_response = sysex(0x6a),

    capability_query        = sysex(0x6b),
    capability_response     = sysex(0x6c),

    pin_state_query         = sysex(0x6d),
    pin_state_response      = sysex(0x6e),

    ext_analog_value        = sysex(0x6f),

    string_data             = sysex(0x71),

    firmware_query          = sysex(0x79),
    firmware_response       = firmware_query,

    sample_rate             = sysex(0x7a),
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

// convert 7-bit message data to value
int to_value(payload::const_iterator begin, payload::const_iterator end);

// convert value to 7-bit message data
payload to_data(int);

////////////////////////////////////////////////////////////////////////////////
// protocol version
struct protocol
{
    int major, minor;
};

// firmware version & name
struct firmware
{
    int major, minor;
    std::string name;
};

////////////////////////////////////////////////////////////////////////////////
// pin number
using pos = byte;
constexpr pos npos = -1;

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

constexpr inline bool digital(mode m) noexcept
{ return m == digital_in || m == digital_out || m == pullup_in; }

constexpr inline bool analog(mode m) noexcept
{ return m == analog_in || m == pwm; }

constexpr inline bool input(mode m) noexcept
{ return m == digital_in || m == analog_in || m == pullup_in; }

constexpr inline bool output(mode m) noexcept
{ return m == digital_out || m == pwm || m == servo; }


// mode resolution
using res = bits;

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
