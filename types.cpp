////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013-2017 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL v3.
// For full terms see COPYING or visit https://www.gnu.org/licenses/gpl.html

////////////////////////////////////////////////////////////////////////////////
#include "firmata/types.hpp"

#include <iterator>
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////
namespace firmata
{

////////////////////////////////////////////////////////////////////////////////
std::string to_string(payload::const_iterator begin, payload::const_iterator end)
{
    std::string s;
    for(auto ci = begin; ci < std::prev(end); ++ci)
        s += char(*ci + (*++ci << 7));
    return s;
}

////////////////////////////////////////////////////////////////////////////////
int to_value(payload::const_iterator begin, payload::const_iterator end)
{
    // TODO: check for overflow
    int value = 0;
    for(auto ci = begin; ci < end; ++ci)
        value += int(*ci) << (7 * (ci - begin));
    return value;
}

////////////////////////////////////////////////////////////////////////////////
payload to_data(int value)
{
    payload data;
    do
    {
        data.push_back(value & 0x7f);
        value >>= 7;
    }
    while(value);
    return data;
}

////////////////////////////////////////////////////////////////////////////////
}
