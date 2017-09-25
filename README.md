# Firmata Client Library in C++

This is a client side implementation of the [Firmata protocol](https://github.com/firmata/protocol)
written in modern C++ using Asio library.

## Getting Started

### Prerequisites

* Relatively modern compiler with support for C++14.

* [Standalone Asio C++ library](https://think-async.com) or the [Boost library](https://www.boost.org).

### Installation

Add as a submodule into an existing project:
```
git submodule add https://github.com/dimitry-ishenko/firmata.git
```

### Usage

Example:

```cpp
////////////////////
using namespace firmata::literals;
using namespace std::chrono_literals;

asio::io_service io;

// open serial port
firmata::serial_port device(io, "/dev/ttyACM0");
device.set(57600_baud);

// connect to Firmata host
firmata::control arduino(&device);

// switch all analog input pins to output
for(auto& pin : arduino.pins())
    if(pin.mode() == analog_in) pin.mode(digital_out);

////////////////////
// switch pin D0 to digital input
arduino.pin(D0).mode(pullup_in);

// monitor pin D0 for state changes
arduino.pin(D0).on_state_changed([&](int state)
{
    std::cout << "D0=" << (state ? "on" : "off") << std::endl;
});

////////////////////
// find first pin that supports pwm and switch it to pwm mode
auto& pwm0 = arduino.pin(pwm, 0);
pwm0.mode(pwm);

std::cout << "D" << +pwm0.pos() << " supports PWM" << std::endl;

// switch pin A3 to analog input
auto& analog3 = arduino.pin(A3);
analog3.mode(analog_in);

auto pwm0_max = 1 << pwm0.res();
auto analog3_max = 1 << analog3.res();

// monitor pin A3 and control the pwm pin
analog3.on_state_changed([&](int state)
{
    static int before = 0;
    if(std::abs(before - state) >= 10)
    {
        auto value = state * pwm0_max / analog3_max;
        pwm0.value(value);

        before = state;
    }
});

////////////////////
// create timer
asio::system_timer blink_timer(io);

using handler = std::function<void(const asio::error_code&)>;
handler blink_led;

// blink led on pin D13 every 500ms
blink_timer.async_wait(blink_led = [&](const asio::error_code&)
{
    // invert pin value
    auto& pin = arduino.pin(D13);
    pin.value(!pin.value());

    // restart timer
    blink_timer.expires_at(blink_timer.expires_at() + 500ms);
    blink_timer.async_wait(blink_led);
});

// start timer
blink_timer.expires_from_now(0ms);

////////////////////
io.run();
```

## Authors

* **Dimitry Ishenko** - dimitry (dot) ishenko (at) (gee) mail (dot) com

## License

This project is distributed under the GNU GPL license. See the
[LICENSE.md](LICENSE.md) file for details.

## Acknowledgments
