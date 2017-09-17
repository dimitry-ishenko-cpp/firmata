# Firmata Client Library in C++

This is a client side implementation of the [Firmata protocol](https://github.com/firmata/protocol)
written in modern C++ using asio library.

## Getting Started

### Prerequisites

Relatively modern compiler with support for C++14.

[Asio C++ library.](https://think-async.com)

### Usage

Example:

```cpp
asio::io_service io;

using namespace firmata;
using namespace firmata::io::literals;

// open serial port
io::serial device(io, "/dev/ttyACM0");
device.set(57600_baud);

// connect to Firmata host
control arduino(&device);

// (optional) change all input pins to output
std::for_each(arduino.pin_begin(), arduino.pin_end(),
    [](auto& pin){ if(is_input(pin.mode())) pin.mode(digital_out); }
);

// change pin 0 to digital input...
arduino.pin(0).mode(pullup_in);

// ... and monitor it for state changes
arduino.pin(0).on_state_changed([](int state)
{
    std::cout << "pin 0 = " << state << std::endl;
});

using namespace std::chrono_literals;
using handler = std::function<void(const asio::error_code&)>;

// create timer
asio::system_timer timer(io);

// blink led on pin 13 every 500ms
handler blink_led = [&](const asio::error_code&)
{
    // invert pin value
    auto& pin = arduino.pin(13);
    pin.value(!pin.value());

    // restart timer
    timer.expires_at(timer.expires_at() + 500ms);
    timer.async_wait(blink_led);
};

// start timer
timer.expires_from_now(0ms);
timer.async_wait(blink_led);

io.run();
```

## Authors

* **Dimitry Ishenko** - dimitry (dot) ishenko (at) (gee) mail (dot) com

## License

This project is licensed under the GNU GENERAL PUBLIC LICENSE Version 3.
See the [GPL-3.0.md](GPL-3.0.md) file for details.

## Acknowledgments
