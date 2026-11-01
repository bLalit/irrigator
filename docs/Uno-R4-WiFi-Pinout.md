# Arduino Uno R4 WiFi Pinout Reference

This reference describes the Arduino Uno R4 WiFi using Arduino pin names. Always compare the labels and silkscreen on the physical board before connecting external hardware.

## Board overview

| Area | Arduino Uno pin names |
| --- | --- |
| Digital I/O | D0-D13 |
| Analog input | A0-A5 |
| Analog aliases | A0 = D14 through A5 = D19 |
| PWM output | D3, D5, D6, D9, D10, D11 |
| Primary I2C | SDA = A4, SCL = A5 |
| SPI | MOSI = D11, MISO = D12, SCK = D13, SS = D10 |
| Hardware UART | RX = D0, TX = D1 |
| Built-in LED | LED_BUILTIN = D13 |
| Built-in RGB LED | User-programmable LEDs at board locations L, TX, RX, and matrix addressable LEDs |

## Digital header

| Header pin | Arduino constant | Primary uses and notes |
| --- | --- | --- |
| D0 | `0` | Hardware UART RX; also digital I/O |
| D1 | `1` | Hardware UART TX; also digital I/O |
| D2 | `2` | Digital I/O / interrupt-capable |
| D3 | `3` | PWM capable |
| D4 | `4` | Digital I/O |
| D5 | `5` | PWM capable |
| D6 | `6` | PWM capable |
| D7 | `7` | Digital I/O |
| D8 | `8` | Digital I/O |
| D9 | `9` | PWM capable |
| D10 | `10` | PWM capable; SPI SS commonly used |
| D11 | `11` | PWM capable; SPI MOSI |
| D12 | `12` | Digital I/O; SPI MISO |
| D13 | `13` | Built-in LED; SPI SCK |

Digital pins use `0` for LOW and `1` or HIGH for HIGH. Do not connect more current than the board and pins are rated to supply. Use a transistor or relay driver for pumps, solenoid valves, motors, and similar loads.

## Analog and mixed-signal pins

| Arduino name | Digital alias | Function |
| --- | --- | --- |
| A0 | D14 | Analog input; can also provide D14 digital I/O |
| A0 | — | DAC-capable on the Uno R4 WiFi |
| A1 | D15 | Analog input; also digital I/O |
| A2 | D16 | Analog input; also digital I/O |
| A3 | D17 | Analog input; also digital I/O |
| A4 | D18 | Analog input; also digital I/O; primary I2C SDA |
| A5 | D19 | Analog input; also digital I/O; primary I2C SCL |

Example Arduino code:

```cpp
int moistureValue = analogRead(A0);       // Read 0-1023 by default
digitalWrite(14, HIGH);                   // Arduino digital alias for A0/D14
analogWrite(D6, 128);                     // Approximately 50% PWM, where supported
```

## Communication interfaces

### Serial UART

| Interface | Arduino functions on Uno headers |
| --- | --- |
| Main USB serial | `Serial` |
| Hardware serial | `Serial1`: RX on D0, TX on D1 |

Pins D0 and D1 are used by the Uno's external hardware serial header. Do not permanently attach another device that drives D0/D1 while the board is uploading unless the external circuitry is compatible.

### I2C

The Uno R4 WiFi primary I2C pins are:

```cpp
// Wire.begin(); uses SDA and SCL.
SDA = A4
SCL = A5
```

A Qwiic connector is also present for compatible 3.3 V I2C modules. Check module voltage ratings before connecting it.

### SPI

Use these digital pins for the Uno's SPI header:

```cpp
MOSI = D11
MISO = D12
SCK  = D13
SS   = D10
```

## Power pins

| Pin | Purpose |
| --- | --- |
| VIN | External power input. Use the board's specified input voltage range; USB power and VIN must be managed safely |
| 5V | 5 V supply output when the board is powered through a valid source |
| 3.3V | 3.3 V regulated supply |
| GND | Ground return; connect all external supplies to a common ground |
| IOREF | Pin reference voltage information |
| RESET | Reset the board when driven LOW |

Recommended operating voltage and logic is 5 V; the board's UNO header is 5 V tolerant where documented. The Uno R4 has a dedicated 3.3 V supply. Use 3.3 V logic unless the connected module is explicitly 5 V compatible.

Do not back-feed an external supply into the 5V pin unless the required diode and power-source protections are understood. For relays, pumps, solenoids, and other inductive loads, use a separate supply and a flyback/protective circuit appropriate to the load.

## Irrigator project pin assignments

Until the irrigation hardware is selected, no project-specific pins should be treated as final. Suggested names in firmware should follow this pattern:

```cpp
constexpr uint8_t SOIL_MOISTURE_SENSOR_PIN = A0;
constexpr uint8_t VALVE_RELAY_1_PIN = D2;
constexpr uint8_t VALVE_RELAY_2_PIN = D4;
constexpr uint8_t VALVE_RELAY_3_PIN = D7;
constexpr uint8_t VALVE_RELAY_4_PIN = D8;
```

Before wiring:

- Verify that the relay board is active-HIGH or active-LOW.
- Confirm whether it accepts 3.3 V or 5 V control.
- Use opto-isolated relay modules or separate transistor drivers when appropriate.
- Keep pump power and high-current wiring separate from signal wiring.
- Do not power a valve relay coil or pump directly from a Uno output pin.
- Add a common ground while keeping high-current grounds routed to prevent signal interference.

## Arduino definitions in C++

PlatformIO and the Arduino core already define names such as `LED_BUILTIN`, `D2`, `A0`, `SDA`, and `SCL`.

Prefer named constants in project code:

```cpp
#include <Arduino.h>

constexpr uint8_t pumpRelayPin = D2;
constexpr uint8_t moistureSensorPin = A0;

void setup() {
  pinMode(pumpRelayPin, OUTPUT);
  digitalWrite(pumpRelayPin, LOW);
}

void loop() {
  int moisture = analogRead(moistureSensorPin);
}
```

Pin aliases such as `D2` and numeric `2` refer to the same header pin. Numeric Arduino pin numbers are more portable within Arduino sketches; physical header positions should never be used as the identifier.

## See also

- Arduino board overview: https://docs.arduino.cc/hardware/uno-r4-wifi/
- Arduino UNO R4 WiFi pinout: https://docs.arduino.cc/resources/pinouts/ABX00087-full-pinout.pdf
- PlatformIO board configuration: https://docs.platformio.org/en/latest/boards/renesas-ra/uno_r4_wifi.html
