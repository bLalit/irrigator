#include <Arduino.h>

#include "UnoR4Button.hpp"
#include "UnoR4Gpio.hpp"
#include "UnoR4LedMatrix.hpp"
#include "UnoR4Pump.hpp"
#include "UnoR4RealTimeClock.hpp"
#include "UnoR4WiFi.hpp"
#include "WiFiCredentials.h"

using UnoR4::Gpio::DigitalGpio;
using UnoR4::Gpio::DigitalPin;
using UnoR4::Wifi::Module;
using UnoR4::Wifi::NetworkCredentials;
using UnoR4::Timekeeping::ClockResult;
using UnoR4::Timekeeping::RealTimeClock;
using UnoR4::Irrigation::Pump;
using UnoR4::Irrigation::RelayLogic;
using UnoR4::Display::LedMatrixClock;
using UnoR4::Input::Button;

static const NetworkCredentials networkCredentials = {
    IrrigatorWiFi::kNetworkName,
    IrrigatorWiFi::kNetworkPassword,
};

static Module wifiModule(networkCredentials, 15000UL, 10000UL);
static RealTimeClock realTimeClock;
static Pump pump(DigitalPin::D7, RelayLogic::ActiveLow);
static LedMatrixClock ledMatrixClock;
static Button manualPumpButton(DigitalPin::D5);
static Button setPumpTimeButton(DigitalPin::D2);
static Button increasePumpTimeButton(DigitalPin::D3);
static Button decreasePumpTimeButton(DigitalPin::D4);
static Button defaultPumpTimeButton(DigitalPin::D8);
static uint32_t lastClockPrintMilliseconds = 0;
static uint32_t lastClockSyncMilliseconds = 0UL;
static bool wifiWasConnected = false;
static uint32_t lastTimeDiagnosticsMilliseconds = 0UL;

constexpr uint32_t kClockSyncIntervalMilliseconds = 60UL * 60UL * 1000UL;

/** Synchronizes the RTC through NTP and reports whether IST was updated. */
static void synchronizeClockFromNtp() {
  if (!wifiModule.isConnected()) {
    Serial.println("IST synchronization skipped: Wi-Fi disconnected.");
    return;
  }

  Serial.print("Wi-Fi connected; local IP: ");
  Serial.println(wifiModule.details().localAddress);

  const ClockResult result = realTimeClock.synchronizeFromNtp();
  if (result == ClockResult::Success) {
    Serial.print("IST synchronized: ");
    Serial.println(realTimeClock.toIndianStandardIso8601());
    return;
  }

  Serial.print("IST synchronization failed; result: ");
  Serial.println(static_cast<int>(result));
}

/** Prints UTC and IST separately to make time-conversion faults diagnosable. */
static void printClockDiagnostics() {
  UnoR4::Timekeeping::DateTime utc{};
  UnoR4::Timekeeping::DateTime ist{};
  time_t unixTime = 0;

  Serial.print("RTC initialized: ");
  Serial.println(realTimeClock.isInitialized() ? "yes" : "no");

  if (realTimeClock.getDateTime(utc) == ClockResult::Success &&
      realTimeClock.getIndianStandardTime(ist) == ClockResult::Success &&
      realTimeClock.getUnixTime(unixTime) == ClockResult::Success) {
    Serial.print("RTC UTC Unix: ");
    Serial.println(static_cast<unsigned long>(unixTime));
    Serial.print("RTC UTC: ");
    Serial.print(utc.year);
    Serial.print("-");
    Serial.print(utc.month);
    Serial.print("-");
    Serial.print(utc.day);
    Serial.print(" ");
    Serial.print(utc.hour);
    Serial.print(":");
    Serial.print(utc.minute);
    Serial.print(":");
    Serial.println(utc.second);
    Serial.print("Calculated IST: ");
    Serial.println(realTimeClock.toIndianStandardIso8601());
  } else {
    Serial.println("RTC UTC/IST read failed.");
  }
}

/** Updates the daily pump schedule maintained by the pump module. */
static void maintainPumpSchedule() {
  UnoR4::Timekeeping::DateTime now{};
  if (realTimeClock.getIndianStandardTime(now) != ClockResult::Success) {
    return;
  }

  if (pump.maintainSchedule(now, 5000UL)) {
    Serial.print("Scheduled pump run started at ");
    Serial.print(pump.scheduleHour());
    Serial.print(":");
    Serial.println(pump.scheduleMinute());
  }
}

/** Handles SET, UP, and DOWN while configuring the daily pump time. */
static void maintainPumpTimeButtons() {
  if (defaultPumpTimeButton.wasPressed()) {
    pump.resetScheduleToDefault();
    Serial.println("Pump schedule reset to 12:30 PM.");
    return;
  }

  if (setPumpTimeButton.wasPressed()) {
    pump.advanceScheduleSetup();

    if (!pump.isScheduleSetupActive()) {
      Serial.print("Pump schedule saved: ");
      Serial.print(pump.scheduleHour());
      Serial.print(":");
      Serial.println(pump.scheduleMinute());
    } else {
      Serial.print("Set pump schedule ");
      Serial.println(pump.scheduleSetupField() ==
                           UnoR4::Irrigation::PumpScheduleField::Hour
                           ? "hour."
                           : "minute.");
    }
    return;
  }

  if (increasePumpTimeButton.wasPressed()) {
    pump.adjustSchedule(1);
  } else if (decreasePumpTimeButton.wasPressed()) {
    pump.adjustSchedule(-1);
  }

  if (pump.isScheduleSetupActive()) {
    Serial.print("Pump schedule: ");
    Serial.print(pump.scheduleHour());
    Serial.print(":");
    Serial.println(pump.scheduleMinute());
  }
}

/** Toggles between an indefinite manual pump run and an immediate stop. */
static void maintainManualPumpControl() {
  if (!manualPumpButton.wasPressed()) {
    return;
  }

  if (pump.isRunning()) {
    pump.stop();
    Serial.println("Manual pump stopped.");
    return;
  }

  pump.start();
  Serial.println("Manual pump started.");
}

// Runs once when the board powers on or resets. Put one-time hardware,
// serial, and network initialization here.
void setup() {
  // Configure the board LED as an output for connection and activity indication.
  DigitalGpio::configureAsOutput(DigitalPin::D13);
  pump.initialize();
  ledMatrixClock.initialize();
  manualPumpButton.initialize();
  setPumpTimeButton.initialize();
  increasePumpTimeButton.initialize();
  decreasePumpTimeButton.initialize();
  defaultPumpTimeButton.initialize();

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
  lastClockSyncMilliseconds = millis();
  ledMatrixClock.displayCurrentTime(realTimeClock);
}

// Runs continuously after setup() completes. Put the board's ongoing tasks
// here; this function repeats while the board is powered and has not reset.
void loop() {
  // Retry the Wi-Fi connection according to the module's configured interval if it drops.
  wifiModule.maintainConnection();
  manualPumpButton.update();
  setPumpTimeButton.update();
  increasePumpTimeButton.update();
  decreasePumpTimeButton.update();
  defaultPumpTimeButton.update();
  pump.maintain();
  maintainPumpSchedule();
  maintainManualPumpControl();
  maintainPumpTimeButtons();

  // While editing, show the pump schedule so UP/DOWN changes are visible.
  // Outside setup mode, continue showing the current IST clock.
  if (pump.isScheduleSetupActive()) {
    ledMatrixClock.displayTime(pump.scheduleHour(), pump.scheduleMinute());
  } else {
    ledMatrixClock.displayCurrentTime(realTimeClock);
  }

  // Detect a new Wi-Fi connection and synchronize once before the next loop.
  const bool wifiIsConnected = wifiModule.isConnected();
  if (wifiIsConnected && !wifiWasConnected) {
    synchronizeClockFromNtp();
    lastClockSyncMilliseconds = millis();
  }
  wifiWasConnected = wifiIsConnected;

  // Correct for RTC crystal drift while Wi-Fi remains continuously connected.
  if (wifiIsConnected &&
      millis() - lastClockSyncMilliseconds >= kClockSyncIntervalMilliseconds) {
    synchronizeClockFromNtp();
    lastClockSyncMilliseconds = millis();
  }

  if (realTimeClock.isInitialized() &&
      millis() - lastClockPrintMilliseconds >= 30000UL) {
    lastClockPrintMilliseconds = millis();
    Serial.print("IST time: ");
    Serial.println(realTimeClock.toIndianStandardIso8601());
  }

  if (millis() - lastTimeDiagnosticsMilliseconds >= 30000UL) {
    lastTimeDiagnosticsMilliseconds = millis();
    printClockDiagnostics();
  }

  // Blink the onboard LED so the program is visibly running.
  DigitalGpio::writeHigh(DigitalPin::D13);
  delay(500);
  DigitalGpio::writeLow(DigitalPin::D13);
  delay(500);
}