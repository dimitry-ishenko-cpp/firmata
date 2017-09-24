////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_SERIAL_PORT_HPP
#define FIRMATA_SERIAL_PORT_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/io_base.hpp"
#include "firmata/types.hpp"

#ifdef ASIO_STANDALONE
    #include <asio.hpp>
    #include <asio/system_timer.hpp>
#else
    #include <boost/asio.hpp>
    #include <boost/asio/system_timer.hpp>
    namespace asio { using namespace boost::asio; }
    namespace asio { using boost::system::error_code; }
#endif

#include <string>
#include <tuple>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
enum baud_rate : unsigned { };

namespace literals
{
    constexpr baud_rate operator"" _baud(unsigned long long n) noexcept
    { return static_cast<baud_rate>(n); }
}

using flow_control = asio::serial_port::flow_control::type;
using parity = asio::serial_port::parity::type;
using stop_bits = asio::serial_port::stop_bits::type;
using char_size = bits;

////////////////////////////////////////////////////////////////////////////////
class serial_port : public io_base
{
public:
    ////////////////////
    serial_port(asio::io_service& io, const std::string& device);
    virtual ~serial_port() noexcept;

    serial_port(const serial_port&) = delete;
    serial_port(serial_port&&) = delete;

    serial_port& operator=(const serial_port&) = delete;
    serial_port& operator=(serial_port&&) = delete;

    ////////////////////
    void set(baud_rate);
    void set(flow_control);
    void set(parity);
    void set(stop_bits);
    void set(char_size);

    ////////////////////
    virtual void write(msg_id, const payload& = { }) override;

    virtual int on_read(read_callback) override;
    virtual void remove_callback(int id) override;

    // block until condition
    virtual bool wait_until(const condition&, const msec&) override;

private:
    ////////////////////
    asio::serial_port port_;
    asio::system_timer timer_;

    std::vector<byte> overall_;
    char one_[128];

    void sched_async();
    void async_read(const asio::error_code&, std::size_t);
    std::tuple<msg_id, payload> parse_one();
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
