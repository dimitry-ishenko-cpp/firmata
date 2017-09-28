////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_COMMAND_HPP
#define FIRMATA_COMMAND_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/io_base.hpp"
#include "firmata/pins.hpp"
#include "firmata/types.hpp"

#include <array>
#include <bitset>
#include <string>
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
struct timeout_error : public std::runtime_error { using std::runtime_error::runtime_error; };

////////////////////////////////////////////////////////////////////////////////
// Send and receive Firmata commands
//
class command
{
public:
    ////////////////////
    explicit command(io_base& io) : io_(io) { }

    command(const command&) = delete;
    command(command&&) = delete;

    command& operator=(const command&) = delete;
    command& operator=(command&&) = delete;

    ////////////////////
    // enable/disable reporting for a digital pin
    void report_digital(pos, bool);
    // enable/disable reporting for an analog pin
    void report_analog(pos, bool);

    // set digital pin value
    void digital_value(pos, bool);
    // set analog pin value
    void analog_value(pos, int);

    // set pin mode
    void pin_mode(pos, mode);
    // set pin state (used by control)
    void pin_state(pin&, int);

    // reset host
    void reset();
    // send string to host
    void string(const std::string&);

    // query protocol version
    protocol query_version(const msec&);
    // query firmware name & version
    firmware query_firmware(const msec&);
    // query capability (pins, modes and reses)
    pins query_capability(const msec&);
    // query analog pin mapping
    void query_analog_mapping(pins&, const msec&);
    // query current pin state
    void query_state(pins&, const msec&);

    // enable reporting for all inputs
    // and disable for all outputs
    void set_report(pins&);

private:
    ////////////////////
    io_base& io_;

    // ports that are currently being monitored
    std::array<std::bitset<8>, port_count> ports_;

    // wait for specific message
    payload wait_until(msg_id, const msec&);
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
