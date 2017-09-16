////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL v3.
// For full terms see COPYING or visit https://www.gnu.org/licenses/gpl.html

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_PIN_HPP
#define FIRMATA_PIN_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/types.hpp"

#include <functional>
#include <map>
#include <set>
#include <utility> // std::swap
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
class pin
{
public:
    ////////////////////
    pin() noexcept = default;
    pin(const pin&) = delete;
    pin(pin&& other) noexcept { swap(other); }

    pin& operator=(const pin&) = delete;
    pin& operator=(pin&& other) noexcept { swap(other); return *this; }

    void swap(pin& other) noexcept
    {
        using std::swap;
        swap(digital_,  other.digital_ );
        swap(analog_,   other.analog_  );
        swap(modes_,    other.modes_   );
        swap(reses_,    other.reses_   );
        swap(mode_,     other.mode_    );
        swap(value_,    other.value_   );
        swap(state_,    other.state_   );
        swap(fn_mode_,  other.fn_mode_ );
        swap(fn_value_, other.fn_value_);
    }

    ////////////////////
    auto pos() const noexcept { return digital_; }
    auto analog_pos() const noexcept { return analog_; }

    auto const& modes() const noexcept { return modes_; }
    bool supports(firmata::mode mode) const noexcept { return modes_.count(mode); }

    auto mode() const noexcept { return mode_; }
    void mode(firmata::mode mode) { fn_mode_(*this, mode); }

    auto res() const noexcept { return reses_.at(mode_); }

    auto value() const noexcept { return value_; }
    void value(int value) { fn_value_(*this, value); }

    auto state() const noexcept { return state_; }

protected:
    ////////////////////
    using fn_mode = std::function<void(pin&, firmata::mode)>;
    using fn_value = std::function<void(pin&, int)>;

    class delegate
    {
        pin* pin_;

    public:
        explicit delegate(pin* p) : pin_(p) { }

        void digital(firmata::pos digital) { pin_->digital_ = digital; }
        void analog(firmata::pos analog) { pin_->analog_ = analog; }

        void add(firmata::mode mode) { pin_->modes_.insert(mode); }
        void add(firmata::mode mode, firmata::res res) { pin_->reses_.emplace(mode, res); }

        void mode(firmata::mode mode) { pin_->mode_ = mode; }

        void value(int value) { pin_->value_ = value; }
        void state(int value) { pin_->state_ = value; }
    };

    // control class modifies internal pin state
    // through the delegate class
    delegate delegate_ { this };

    pin(firmata::pos pos, fn_mode fm, fn_value fv) :
        digital_(pos), fn_mode_(fm), fn_value_(fv)
    { }

    friend class control;

private:
    ////////////////////
    firmata::pos digital_ = npos, analog_ = npos;

    std::set<firmata::mode> modes_;
    // resolution for each mode
    std::map<firmata::mode, firmata::res> reses_;

    firmata::mode mode_;

    int value_ = 0;
    int state_ = 0;

    ////////////////////
    fn_mode fn_mode_;
    fn_value fn_value_;
};

////////////////////////////////////////////////////////////////////////////////
inline void swap(pin& lhs, pin& rhs) noexcept { lhs.swap(rhs); }

////////////////////////////////////////////////////////////////////////////////
using pins = std::vector<pin>;

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
