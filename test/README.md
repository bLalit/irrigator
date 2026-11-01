# Tests

The Wi-Fi tests run directly on the Arduino Uno R4 WiFi.

## Run

Connect the Uno by USB and run:

```bash
pio test -e uno_r4_wifi
```

The tests avoid joining a real Wi-Fi network. They validate credentials, access-point parameter checks, disconnected state, network details, and the Uno R4 Wi-Fi firmware interface.
