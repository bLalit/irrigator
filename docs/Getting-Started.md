# Uno Irrigator Getting Started

This project targets the Arduino Uno R4 WiFi using PlatformIO.

For Arduino header, power, PWM, analog, and communication-pin details, see [Uno-R4-WiFi-Pinout.md](Uno-R4-WiFi-Pinout.md).

## Build

From the project directory:

```bash
pio run
```

To build a specific environment:

```bash
pio run -e uno_r4_wifi
```

The PlatformIO CLI is installed in VS Code at:

```text
~/.platformio/penv/bin/pio
```

If `pio` is not in the shell `PATH`, use that full path or the VS Code PlatformIO buttons.

## Upload

Connect the Uno R4 WiFi by USB and run:

```bash
pio run -e uno_r4_wifi -t upload
```

To specify the detected serial port:

```bash
pio run -e uno_r4_wifi -t upload --upload-port /dev/cu.usbmodem9888E0089B2C2
```

The upload port can change after reconnecting the board. Confirm it with:

```bash
pio device list
```

## Unit Tests

PlatformIO unit tests are located in folders under `test`. Each test suite is
stored in its own folder, such as `test/test_<module_name>`.

Compile all unit tests without uploading them to the board:

```bash
pio test -e uno_r4_wifi --without-uploading
```

Run the tests on the connected Uno R4 WiFi:

```bash
pio test -e uno_r4_wifi
```

To specify the board's USB serial port explicitly:

```bash
pio test -e uno_r4_wifi --upload-port /dev/cu.usbmodem9888E0089B2C2
```

The port can change after the board is disconnected or reconnected. List the current ports with:

```bash
pio device list
```

The tests print their Unity results over serial. If the PlatformIO test command waits for output, press the board's reset button once and wait for the output to appear.

Each test suite should remain focused on one module and avoid unrelated hardware
or network activity unless that behavior is part of the module contract. Add new
suites under `test/test_<module_name>`, with `test_main.cpp` containing the Unity
test setup and test functions.

## Running Without a Computer

After a successful upload, the compiled program is stored in the Uno's flash memory. The board runs `setup()` once and then `loop()` repeatedly, without VS Code, PlatformIO, or a laptop connection.

The board needs a suitable stable power source. Power can come from USB, or from the board's VIN/USB power input according to its voltage specifications. USB-connected sensors or relays can require a separate power supply if their current demand exceeds the board's USB or pin limits.

Disconnecting USB does not erase an uploaded sketch. Removing or interrupting board power stops execution. When suitable power returns, the Uno starts the uploaded sketch again. Backing the Uno's power supply with batteries or another suitable source is possible, but proper regulator, fuse, and diode protections should be used.

## Useful PlatformIO Commands

```bash
# Clean previous build output
pio run -t clean

# Build and upload
pio run -e uno_r4_wifi -t upload

# Monitor serial output on the detected port
pio device monitor

# Monitor on an explicit port
pio device monitor --port /dev/cu.usbmodem9888E0089B2C2
```
