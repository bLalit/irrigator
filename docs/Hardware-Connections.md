# Hardware Connections

This document lists the Irrigator project's external connections. Read the
relay-module and pump labels before applying power because relay-board
terminology and trigger polarity vary by manufacturer.

## Project pin assignments

| Function | Uno R4 pin | Connection direction | Notes |
| --- | --- | --- | --- |
| Pump relay control | D7 | Uno to relay | Active-low firmware setting |
| SET button | D2 | Button to ground | Internal pull-up enabled in firmware |
| UP button | D3 | Button to ground | Internal pull-up enabled in firmware |
| DOWN button | D4 | Button to ground | Internal pull-up enabled in firmware |
| RUN/STOP button | D5 | Button to ground | Internal pull-up enabled in firmware |
| Relay/pump ground reference | GND | Common ground | Keep relay-control ground separated if the relay module requires isolated coil power |

## Pump relay and pump power supply

### Uno control side

For a single-channel relay module:

| Relay module terminal | Connect to |
| --- | --- |
| IN / SIG | Uno D7 |
| VCC | Uno 5V if the module supports 5 V control |
| GND | Uno GND |

If the relay module uses a separate relay-coil supply:

| Relay module connection | Connect to |
| --- | --- |
| JD-VCC or relay-coil VCC | External 5 V supply positive |
| VCC or logic supply | Uno 5V, if required by the module |
| GND | Common ground with the Uno logic ground |

Follow the silkscreen and supplied jumper instructions for the relay
module. Do not connect an external relay-coil supply to the Uno header unless
the module documentation explicitly requires it.

The firmware is configured as active-low. This means D7 is driven low to
energize the relay. If the relay board is active-high, change the relay
configuration in [main.cpp](/Users/lalitbhat/Documents/PlatformIO/Projects/Uno_irrigator/src/main.cpp)
to `RelayLogic::ActiveHigh`.

### Pump switched side

Use a pump supply sized for both the pump's operating current and startup
current:

```text
Pump supply + ---------------- COM relay terminal
Pump supply - ---------------- Pump negative terminal
Relay NO -------------------- Pump positive terminal
```

Leave relay `NC` disconnected unless a fail-safe normally-energized circuit
is intentionally required.

Example layout:

```text
                 +5 V pump supply
                        |
                       COM
Pump supply (-) ------ NO ------ Pump (+)
       |
     Pump (-)
```

### Pump power rules

- Do not connect the pump directly to Uno D7 or the Uno 5V pin.
- Do not back-feed a separate pump supply into the Uno.
- Connect Uno ground and pump-supply ground only as required by relay-module
  isolation instructions.
- Add a flyback diode or a relay rated for the pump load for inductive DC
  pump wiring.
- Verify the relay contact rating exceeds the pump's operating and startup
  current.
- Keep pump wires and their supply separate from GPIO and button wires.
- Disconnect pump output power while initially testing relay control.

## Buttons

Each button uses one terminal as the signal connection and the other terminal
as ground:

```text
Uno D2 ----- SET button ---- GND
Uno D3 ----- UP button ----- GND
Uno D4 ----- DOWN button --- GND
Uno D5 ----- RUN/STOP ----- GND
```

The matching Uno pins are configured as inputs with internal pull-ups.
Therefore:

- Open button: signal reads HIGH.
- Pressed button: signal reads LOW.

Example breadboard connection:

```text
+--------------------- Uno breadboard row ----------------------+
|                                                               |
| D2 ----o       SET button       o------------------------- GND |
|                                                               |
| D3 ----o       UP button        o------------------------- GND |
|                                                               |
| D4 ----o       DOWN button      o------------------------- GND |
|                                                               |
| D5 ----o       RUN/STOP button  o------------------------- GND |
+---------------------------------------------------------------+
```

For reduced contact bounce, an optional 100 nF capacitor may be connected
between each button signal pin and GND. Keep each capacitor physically close
to the button.

## Safety checks before energizing the pump

1. Verify the relay module's silkscreen terminal names.
2. Verify active-high versus active-low behavior.
3. Verify relay coil voltage and logic voltage ratings.
4. Verify the relay contact voltage and current ratings.
5. Verify pump supply polarity.
6. Disconnect the pump and test that D7 controls only the relay first.
7. Confirm that all grounds are connected as shown by the relay module's
   isolation design.
8. Test the pump with water only after the relay switching and power supply
   wiring have been inspected.
