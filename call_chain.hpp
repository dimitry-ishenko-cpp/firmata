////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef FIRMATA_CALL_CHAIN_HPP
#define FIRMATA_CALL_CHAIN_HPP

////////////////////////////////////////////////////////////////////////////////
#include <functional>
#include <map>
#include <tuple>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
// Alias for function
template<typename Fn>
using call = std::function<Fn>;

// Call id
using cid = std::tuple<unsigned, unsigned>;

////////////////////////////////////////////////////////////////////////////////
// Chain of functions
template<typename Fn>
struct call_chain
{
    ////////////////////
    call_chain(unsigned token = 0) noexcept : token_(token), id_(0) { }

    call_chain(const call_chain&) = delete;
    call_chain(call_chain&&) noexcept = default;

    call_chain& operator=(const call_chain&) = delete;
    call_chain& operator=(call_chain&&) noexcept = default;

    ////////////////////
    auto insert(Fn fn)
    {
        cid id(token_, id_++);
        chain_.emplace(id, std::move(fn));
        return id;
    }

    bool erase(cid id) { return chain_.erase(id); }
    void clear() { chain_.clear(); }

    auto empty() const noexcept { return chain_.empty(); }
    auto size() const noexcept { return chain_.size(); }

    template<typename... Args>
    void operator()(Args&&... args)
    {
        for(auto const& fn : chain_) fn.second(std::forward<Args>(args)...);
    }

private:
    ////////////////////
    unsigned token_, id_;
    std::map<cid, Fn> chain_;
};

////////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////
#endif
