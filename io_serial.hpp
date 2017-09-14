////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL v3.
// For full terms see COPYING or visit https://www.gnu.org/licenses/gpl.html

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_IO_SERIAL_HPP
#define FIRMATA_IO_SERIAL_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/io_base.hpp"
#include "firmata/types.hpp"

#include <asio/io_service.hpp>
#include <asio/serial_port.hpp>
#include <asio/streambuf.hpp>
#include <string>
#include <tuple>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{
namespace io
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
class serial : public base
{
public:
    ////////////////////
    serial(asio::io_service& io, const std::string& device);
    virtual ~serial() noexcept;

    void set(baud_rate);
    void set(flow_control);
    void set(parity);
    void set(stop_bits);
    void set(char_size);

    virtual void write(msg_id, const payload& = { }) override;
    virtual std::tuple<msg_id, payload> read() override;

    virtual void reset_async(callback = nullptr) override;

private:
    ////////////////////
    asio::serial_port port_;

    void sched_read();

    asio::streambuf store_;
    void async_read(const asio::error_code&);

    callback fn_;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
namespace literals { using namespace io::literals; }

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
