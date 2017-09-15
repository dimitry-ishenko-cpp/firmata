////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL v3.
// For full terms see COPYING or visit https://www.gnu.org/licenses/gpl.html

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_COMMAND_HPP
#define FIRMATA_COMMAND_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/io.hpp"
#include "firmata/pin.hpp"
#include "firmata/types.hpp"

#include <bitset>
#include <chrono>
#include <map>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
class command
{
public:
    ////////////////////
    explicit command(io::base* io) : io_(io) { }

    ////////////////////
    protocol query_version();
    firmware query_firmware();

    pins query_capability();
    void query_analog_mapping(pins&);
    void query_state(pins&);

    void pin_mode(pin&, mode);

    void digital_value(pin&, bool);
    void analog_value(pin&, int);

    void report_analog(pin&, bool);
    void report_digital(pin&, bool);

    using msec = std::chrono::milliseconds;
    void sample_rate(const msec&);

    void reset();

private:
    ////////////////////
    io::base* io_;

    // map of pins by port that are currently being monitored
    std::map<int, std::bitset<8>> ports_;

    // get specific reply discarding others
    payload read_until(msg_id);
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
