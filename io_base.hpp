////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_IO_BASE_HPP
#define FIRMATA_IO_BASE_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/call_chain.hpp"
#include "firmata/types.hpp"

#include <functional>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
// Communication with Firmata host
//
class io_base
{
public:
    ////////////////////
    // write message to host
    virtual void write(msg_id, const payload& = { }) = 0;

    using read_call = call<void(msg_id, const payload&)>;

    // install read callback
    virtual cid on_read(read_call fn) { return chain_.insert(std::move(fn)); }

    // remove read callback
    virtual bool remove_call(cid id) { return chain_.erase(id); }

    using condition = std::function<bool()>;

    // block until condition or timeout
    virtual bool wait_until(const condition&, const msec&) = 0;

protected:
    ////////////////////
    call_chain<read_call> chain_;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
