#include <Arduino.h>

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

static const NetworkCredentials networkCredentials = {
    IrrigatorWiFi::kNetworkName,
    IrrigatorWiFi::kNetworkPassword,
};

static Module wifiModule(networkCredentials, 15000UL, 10000UL);
static RealTimeClock realTimeClock;
static Pump pump(DigitalPin::D7, RelayLogic::ActiveLow);
static LedMatrixClock ledMatrixClock;
static uint32_t lastClockPrintMilliseconds = 0;
static bool wifiWasConnected = false;
static uint32_t lastScheduledPumpRunDay = 0UL;
static uint32_t lastTimeDiagnosticsMilliseconds = 0UL;

constexpr uint8_t kPumpStartHourIst = 19U;
constexpr uint8_t kPumpStartMinuteIst = 03U;

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

/** Returns a date key used to prevent starting the scheduled pump twice per day. */
static uint32_t calendarDayKey(const UnoR4::Timekeeping::DateTime& now) {
  return static_cast<uint32_t>(now.year) * 10000UL +
         static_cast<uint32_t>(now.month) * 100UL +
         static_cast<uint32_t>(now.day);
}

/** Starts the pump once at 18:25 IST when the RTC clock reaches that minute. */
static void maintainPumpSchedule() {
  UnoR4::Timekeeping::DateTime now{};
  if (realTimeClock.getIndianStandardTime(now) != ClockResult::Success) {
    return;
  }

  const uint32_t today = calendarDayKey(now);
  const bool scheduledMinute =
      now.hour == kPumpStartHourIst &&
      now.minute == kPumpStartMinuteIst;

  if (scheduledMinute &&
      today != lastScheduledPumpRunDay &&
      !pump.isRunning()) {
    pump.startFor(5000UL); // Start the pump for 5 seconds
    lastScheduledPumpRunDay = today;
    Serial.println("Scheduled pump run started at 18:25 IST.");
  }
}

// Runs once when the board powers on or resets. Put one-time hardware,
// serial, and network initialization here.
void setup() {
  // Configure the board LED as an output for connection and activity indication.
  DigitalGpio::configureAsOutput(DigitalPin::D13);
  pump.initialize();
  ledMatrixClock.initialize();

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
  ledMatrixClock.displayCurrentTime(realTimeClock);
}

// Runs continuously after setup() completes. Put the board's ongoing tasks
// here; this function repeats while the board is powered and has not reset.
void loop() {
  // Retry the Wi-Fi connection according to the module's configured interval if it drops.
  wifiModule.maintainConnection();
  pump.maintain();
  maintainPumpSchedule();
  ledMatrixClock.displayCurrentTime(realTimeClock);

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