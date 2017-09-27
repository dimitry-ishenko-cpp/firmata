////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_ENCODER_HPP
#define FIRMATA_ENCODER_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/callback.hpp"
#include "firmata/pin.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
class encoder
{
public:
    ////////////////////
    encoder(pin&, pin&);
    ~encoder();

    encoder(const encoder&) = delete;
    encoder(encoder&&) = delete;

    encoder& operator=(const encoder&) = delete;
    encoder& operator=(encoder&&) = delete;

    ////////////////////
    using int_callback = callback<void(int)>;
    using void_callback = callback<void()>;

    cbid on_rotate(int_callback);
    cbid on_rotate_cw(void_callback);
    cbid on_rotate_ccw(void_callback);

    void remove_callback(cbid);

private:
    ////////////////////
    pin& pin1_; pin& pin2_; cbid id_;

    call_chain<int_callback>  rotate_     { 0 };
    call_chain<void_callback> rotate_cw_  { 1 };
    call_chain<void_callback> rotate_ccw_ { 2 };

    void pin_state_changed(int);

    enum { no, cw, ccw } step_ = no;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
