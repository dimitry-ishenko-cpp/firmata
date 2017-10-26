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

firmata::serial_port device(io, "/dev/ttyACM0");
device.set(57600_baud);

firmata::client arduino(device);

for(auto& pin : arduino.pins())
{
    if(pin.mode() == analog_in) pin.mode(digital_out);
}

////////////////////
// connect KY-040 rotary encoder:
// CLK ->  D0
// DT  ->  D1
// SW  ->  D2
// +   -> +5V
// GND -> GND
//
arduino.pin(D0).mode(digital_in);
arduino.pin(D1).mode(digital_in);

firmata::encoder encoder(arduino.pin(D0), arduino.pin(D1));

encoder.on_rotate([](int step)
{
    static int count = 0;

    count += step;
    std::cout << "encoder: count=" << count << std::endl;
});

encoder.on_rotate_cw ([](){ std::cout << "encoder: cw" << std::endl; });
encoder.on_rotate_ccw([](){ std::cout << "encoder: ccw" << std::endl; });

arduino.pin(D2).mode(pullup_in);

arduino.pin(D2).on_state_changed([](int state)
{
    std::cout << "encoder: switch=" << state << std::endl;
});

arduino.pin(D2).on_state_low ([](){ std::cout << "encoder: press" << std::endl; });
arduino.pin(D2).on_state_high([](){ std::cout << "encoder: release" << std::endl; });

////////////////////
// connect ON/OFF switch:
// 1 ->  D7
// 2 -> GND
//
auto& pin7 = arduino.pin(D7);
pin7.mode(pullup_in);

firmata::debounce debounce(io);

debounce.on_state_changed(pin7, [](int state)
{
    std::cout << "D7: state=" << state << std::endl;
});
debounce.on_state_low (pin7, [](){ std::cout << "D7: closed" << std::endl; });
debounce.on_state_high(pin7, [](){ std::cout << "D7: open" << std::endl; });

////////////////////
// connect 10K pot:
// 1 -> +5V
// 2 ->  A3
// 3 -> GND
//
auto& analog3 = arduino.pin(A3);
analog3.mode(analog_in);

// connect LED:
// + -> ... (first pin that supports PWM)
// - -> GND
//
auto& pwm0 = arduino.pin(pwm, 0); // find first pin with PWM support
pwm0.mode(pwm);

std::cout << "D" << +pwm0.pos() << " supports PWM" << std::endl;

// control LED brightness using 10K pot
//
analog3.on_state_changed([&](int state)
{
    static auto pwm0_max = 1 << pwm0.res();
    static auto analog3_max = 1 << analog3.res();

    auto value = state * pwm0_max / analog3_max;
    pwm0.value(value);
});

////////////////////
// blink led on pin D13 every 500ms
//
asio::system_timer blink_timer(io);

using handler = std::function<void(const asio::error_code&)>;
handler blink_led;

blink_timer.async_wait(blink_led = [&](const asio::error_code&)
{
    auto value = arduino.pin(D13).value();
    arduino.pin(D13).value( !value );

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
