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
#include "firmata/pin.hpp"
#include "firmata/types.hpp"

#include <array>
#include <bitset>
#include <chrono>
#include <string>
#include <stdexcept>
#include <vector>

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
    explicit command(io_base* io) : io_(io) { }

    command(const command&) = delete;
    command(command&&) = delete;

    command& operator=(const command&) = delete;
    command& operator=(command&&) = delete;

    ////////////////////
    void report_digital(pos, bool);
    void report_analog(pos, bool);

    void digital_value(pos, bool);
    void analog_value(pos, int);

    void pin_mode(pos, mode);

    void reset();
    void string(const std::string&);

    using msec = std::chrono::milliseconds;

    protocol query_version(const msec&);
    firmware query_firmware(const msec&);
    std::vector<pin> query_capability(const msec&);
    void query_analog_mapping(std::vector<pin>&, const msec&);
    void query_state(std::vector<pin>&, const msec&);

    void set_report(std::vector<pin>&);

private:
    ////////////////////
    io_base* io_;

    // ports that are currently being monitored
    std::array<std::bitset<8>, port_count> ports_;

    payload wait_until(msg_id, const msec&);
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
