#include <Arduino.h>

#include "UnoR4Gpio.hpp"
#include "UnoR4RealTimeClock.hpp"
#include "UnoR4WiFi.hpp"
#include "WiFiCredentials.h"

using UnoR4::Gpio::DigitalGpio;
using UnoR4::Gpio::DigitalPin;
using UnoR4::Wifi::Module;
using UnoR4::Wifi::NetworkCredentials;
using UnoR4::Timekeeping::ClockResult;
using UnoR4::Timekeeping::RealTimeClock;

static const NetworkCredentials networkCredentials = {
    IrrigatorWiFi::kNetworkName,
    IrrigatorWiFi::kNetworkPassword,
};

static Module wifiModule(networkCredentials, 15000UL, 10000UL);
static RealTimeClock realTimeClock;
static uint32_t lastClockPrintMilliseconds = 0;
static bool wifiWasConnected = false;

/** Synchronizes the RTC through NTP and reports whether IST was updated. */
static void synchronizeClockFromNtp() {
  if (!wifiModule.isConnected()) {
    Serial.println("IST synchronization skipped: Wi-Fi disconnected.");
    return;
  }

  const ClockResult result = realTimeClock.synchronizeFromNtp();
  if (result == ClockResult::Success) {
    Serial.print("IST synchronized: ");
    Serial.println(realTimeClock.toIndianStandardIso8601());
    return;
  }

  Serial.println("IST synchronization failed.");
}

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
  if (realTimeClock.initialize() != ClockResult::Success) {
    Serial.println("RTC initialization failed.");
    return;
  }

  wifiWasConnected = wifiModule.isConnected();
  synchronizeClockFromNtp();
}

// Runs continuously after setup() completes. Put the board's ongoing tasks
// here; this function repeats while the board is powered and has not reset.
void loop() {
  // Retry the Wi-Fi connection according to the module's configured interval if it drops.
  wifiModule.maintainConnection();

  // Detect a new Wi-Fi connection and synchronize once before the next loop.
  const bool wifiIsConnected = wifiModule.isConnected();
  if (wifiIsConnected && !wifiWasConnected) {
    synchronizeClockFromNtp();
  }
  wifiWasConnected = wifiIsConnected;

  if (realTimeClock.isInitialized() &&
      millis() - lastClockPrintMilliseconds >= 30000UL) {
    lastClockPrintMilliseconds = millis();
    Serial.print("IST time: ");
    Serial.println(realTimeClock.toIndianStandardIso8601());
  }

  // Blink the onboard LED so the program is visibly running.
  DigitalGpio::writeHigh(DigitalPin::D13);
  delay(500);
  DigitalGpio::writeLow(DigitalPin::D13);
  delay(500);
}