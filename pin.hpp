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
        swap(fn_mode_,  other.fn_mode_ );
        swap(value_,    other.value_   );
        swap(fn_value_, other.fn_value_);
        swap(state_,    other.state_   );
    }

    ////////////////////
    auto pos() const noexcept { return digital_; }
    auto analog_pos() const noexcept { return analog_; }

    auto const& modes() const noexcept { return modes_; }
    bool supports(firmata::mode mode) const noexcept { return modes_.count(mode); }

    auto mode() const noexcept { return mode_; }
    void mode(firmata::mode mode) const { fn_mode_(digital_, mode); }

    auto res() const noexcept { return reses_.at(mode_); }

    auto value() const noexcept { return value_; }
    void value(int value) const { fn_value_(digital_, value); }

    auto state() const noexcept { return state_; }

protected:
    ////////////////////
    // mode and value setters (handled by control)
    using fn_mode = std::function<void(firmata::pos, firmata::mode)>;
    using fn_value = std::function<void(firmata::pos, int)>;

    pin(firmata::pos pos, fn_mode mode, fn_value value) :
        digital_(pos), fn_mode_(std::move(mode)), fn_value_(std::move(value))
    { }

    friend class control;

private:
    ////////////////////
    firmata::pos digital_ = npos, analog_ = npos;

    std::set<firmata::mode> modes_;
    // resolution for each mode
    std::map<firmata::mode, firmata::res> reses_;

    firmata::mode mode_;
    fn_mode fn_mode_;

    int value_ = 0;
    fn_value fn_value_;

    int state_ = 0;
};

////////////////////////////////////////////////////////////////////////////////
inline void swap(pin& lhs, pin& rhs) noexcept { lhs.swap(rhs); }

////////////////////////////////////////////////////////////////////////////////
using pins = std::vector<pin>;

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
