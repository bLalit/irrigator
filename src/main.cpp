#include <Arduino.h>

#include "UnoR4Gpio.hpp"
#include "UnoR4WiFi.hpp"
#include "WiFiCredentials.h"

using UnoR4::Gpio::DigitalGpio;
using UnoR4::Gpio::DigitalPin;
using UnoR4::Wifi::Module;
using UnoR4::Wifi::NetworkCredentials;

static const NetworkCredentials networkCredentials = {
    IrrigatorWiFi::kNetworkName,
    IrrigatorWiFi::kNetworkPassword,
};

static Module wifiModule(networkCredentials, 15000UL, 10000UL);

// Runs once when the board powers on or resets. Put one-time hardware,
// serial, and network initialization here.
void setup() {
  // Configure the board LED as an output for connection and activity indication.
  DigitalGpio::configureAsOutput(DigitalPin::D13);

  Serial.begin(115200);
  // Wait briefly so serial diagnostics are not lost after power-on or reset.
  while (!Serial) {
    delay(10);
  }

  wifiModule.connectAndPrintStatus();
}

// Runs continuously after setup() completes. Put the board's ongoing tasks
// here; this function repeats while the board is powered and has not reset.
void loop() {
  // Retry the Wi-Fi connection according to the module's configured interval if it drops.
  wifiModule.maintainConnection();

  // Blink the onboard LED so the program is visibly running.
  DigitalGpio::writeHigh(DigitalPin::D13);
  delay(500);
  DigitalGpio::writeLow(DigitalPin::D13);
  delay(500);
}