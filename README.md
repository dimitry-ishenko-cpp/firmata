# Firmata Client Library in C++

This is a client side implementation of the [Firmata protocol](https://github.com/firmata/protocol)
written in modern C++ using Asio library.

## Getting Started

### Prerequisites

* Relatively modern compiler with support for C++14.

* [Asio C++ library.](https://think-async.com)

### Usage

Example:

```cpp
////////////////////
asio::io_service io;

using namespace firmata;
using namespace firmata::io::literals;

// open serial port
io::serial device(io, "/dev/ttyACM0");
device.set(57600_baud);

// connect to Firmata host
control arduino(&device);

////////////////////
// switch all analog input pins to output
std::for_each(arduino.pin_begin(), arduino.pin_end(),
    [](auto& pin){ if(pin.mode() == analog_in) pin.mode(digital_out); }
);

////////////////////
// switch pin D0 to digital input
arduino.pin(D0).mode(pullup_in);

// monitor it for state changes
arduino.pin(D0).on_state_changed([&](int state)
{
    std::cout << "pin D0 = " << (state ? "high" : "low") << std::endl;
});

// switch pin A3 to analog input
arduino.pin(A3).mode(analog_in);

// monitor it for state changes
arduino.pin(A3).on_state_changed([&](int state)
{
    static int before = 0;
    if(std::abs(before - state) >= 10)
    {
        std::cout << "pin A3 = " << state << std::endl;
        before = state;
    }
});

////////////////////
using namespace std::chrono_literals;
using handler = std::function<void(const asio::error_code&)>;

// create timer
asio::system_timer blink_timer(io);
handler blink_fn;

// blink led on pin D13 every 500ms
blink_timer.async_wait(blink_fn = [&](const asio::error_code&)
{
    // invert pin value
    auto& pin = arduino.pin(D13);
    pin.value(!pin.value());

    // restart timer
    blink_timer.expires_at(blink_timer.expires_at() + 500ms);
    blink_timer.async_wait(blink_fn);
});

// start timer
blink_timer.expires_from_now(0ms);

////////////////////
// find first pin that supports pwm
// and switch it to pwm mode
arduino.pin(pwm, 0).mode(pwm);
std::cout << "D" << int(arduino.pin(pwm, 0).pos()) << " is a PWM pin" << std::endl;

// create timer
asio::system_timer pwm_timer(io);
handler pwm_fn;

// dim lights up and down
pwm_timer.async_wait(pwm_fn = [&](const asio::error_code&)
{
    static int cycle = 0, step = 1;

    // set pwm value
    arduino.pin(pwm, 0).value(cycle);

    // inc/dec cycle
    cycle += step;
    if(cycle == 0 || cycle == 255) step = -step;

    // restart timer
    pwm_timer.expires_at(pwm_timer.expires_at() + 10ms);
    pwm_timer.async_wait(pwm_fn);
});

// start timer
pwm_timer.expires_from_now(0ms);

////////////////////
io.run();
```

## Authors

* **Dimitry Ishenko** - dimitry (dot) ishenko (at) (gee) mail (dot) com

## License

This project is distributed under the GNU GPL license. See the
[LICENSE.md](LICENSE.md) file for details.

## Acknowledgments
