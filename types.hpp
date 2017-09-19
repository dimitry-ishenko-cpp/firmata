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
inline auto to_string(const payload& data) { return to_string(data.begin(), data.end()); }

// convert 7-bit message data to value
int to_value(payload::const_iterator begin, payload::const_iterator end);
inline auto to_value(const payload& data) { return to_value(data.begin(), data.end()); }

// convert string to 7-bit message data
payload to_data(const std::string&);

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

enum digital : pos
{
    D0,   D1,   D2,   D3,   D4,   D5,   D6,   D7,   D8,   D9,
    D10,  D11,  D12,  D13,  D14,  D15,  D16,  D17,  D18,  D19,
    D20,  D21,  D22,  D23,  D24,  D25,  D26,  D27,  D28,  D29,
    D30,  D31,  D32,  D33,  D34,  D35,  D36,  D37,  D38,  D39,
    D40,  D41,  D42,  D43,  D44,  D45,  D46,  D47,  D48,  D49,
    D50,  D51,  D52,  D53,  D54,  D55,  D56,  D57,  D58,  D59,
    D60,  D61,  D62,  D63,  D64,  D65,  D66,  D67,  D68,  D69,
    D70,  D71,  D72,  D73,  D74,  D75,  D76,  D77,  D78,  D79,
    D80,  D81,  D82,  D83,  D84,  D85,  D86,  D87,  D88,  D89,
    D90,  D91,  D92,  D93,  D94,  D95,  D96,  D97,  D98,  D99,
    D100, D101, D102, D103, D104, D105, D106, D107, D108, D109,
    D110, D111, D112, D113, D114, D115, D116, D117, D118, D119,
    D120, D121, D122, D123, D124, D125, D126, D127,
};

enum analog : pos
{
    A0,  A1, A2,  A3,  A4,  A5,  A6,  A7,
    A8,  A9, A10, A11, A12, A13, A14, A15,
};

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
