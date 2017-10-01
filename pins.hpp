////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_PINS_HPP
#define FIRMATA_PINS_HPP

////////////////////////////////////////////////////////////////////////////////
#include "firmata/pin.hpp"
#include "firmata/types.hpp"

#include <algorithm>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
// Collection of pins
//
class pins : private std::vector<pin>
{
    using base = std::vector<pin>;

public:
    ////////////////////
    pins() noexcept = default;

    pins(const pin&) = delete;
    pins(pins&&) noexcept = default;

    pins& operator=(const pins&) = delete;
    pins& operator=(pins&&) noexcept = default;

    ////////////////////
    using base::begin;
    using base::end;
    using base::cbegin;
    using base::cend;
    using base::rbegin;
    using base::rend;
    using base::crbegin;
    using base::crend;

    // number of pins
    auto count() const noexcept { return size(); }
    // number of pins that support certain mode
    auto count(mode n) const noexcept
    {
        return std::count_if(begin(), end(),
            [&](auto& pin){ return pin.supports(n); }
        );
    }

    ////////////////////
    // get pin
    auto& get(pos n) { return at(n); }
    const auto& get(pos n) const { return at(n); }

    // get analog pin
    auto& get(analog n) { return get(analog_in, n); }
    auto const& get(analog n) const { return get(analog_in, n); }

    // get pin that supports certain mode
    firmata::pin& get(mode, pos);
    const firmata::pin& get(mode, pos) const;

    friend class control;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
