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
    void count(int n) noexcept { count_ = n; }
    auto count() const noexcept { return count_; }

    ////////////////////
    using int_callback = callback<void(int)>;
    using void_callback = callback<void()>;

    cbid on_count_changed(int_callback);
    cbid on_rotate_cw(void_callback);
    cbid on_rotate_ccw(void_callback);

    void remove_callback(cbid);

private:
    ////////////////////
    pin& pin1_; pin& pin2_; cbid id_;

    call_chain<int_callback> changed_ { 0 };
    call_chain<void_callback> cw_     { 1 };
    call_chain<void_callback> ccw_    { 2 };

    void pin_state_low();
    int count_ = 0;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
