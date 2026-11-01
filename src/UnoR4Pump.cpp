#include "UnoR4Pump.hpp"

#include <Arduino.h>
#include <EEPROM.h>

namespace UnoR4 {
namespace Irrigation {

namespace {

constexpr uint32_t kFiveSecondsMilliseconds = 5000UL;
constexpr uint8_t kDefaultScheduleHour = 12U;
constexpr uint8_t kDefaultScheduleMinute = 30U;
constexpr uint8_t kScheduleMagic = 0xC5U;
constexpr int kScheduleMagicAddress = 0;
constexpr int kScheduleHourAddress = 1;
constexpr int kScheduleMinuteAddress = 2;

uint32_t calendarDayKey(const UnoR4::Timekeeping::DateTime& dateTime) {
  return static_cast<uint32_t>(dateTime.year) * 10000UL +
         static_cast<uint32_t>(dateTime.month) * 100UL +
         dateTime.day;
}

}  // namespace

/** Stores relay configuration and begins in the stopped state. */
Pump::Pump(Gpio::DigitalPin relayPin, RelayLogic relayLogic)
    : relayPin_(relayPin),
      relayLogic_(relayLogic),
      startedAtMilliseconds_(0UL),
      runDurationMilliseconds_(0UL),
      running_(false),
      scheduleHour_(kDefaultScheduleHour),
      scheduleMinute_(kDefaultScheduleMinute),
      lastScheduledPumpRunDay_(0UL),
      scheduleSetupActive_(false),
      scheduleSetupField_(PumpScheduleField::Hour) {
}

/** Configures the relay pin as output before ensuring the relay is inactive. */
void Pump::initialize() {
  Gpio::DigitalGpio::configureAsOutput(relayPin_);
  running_ = false;
  startedAtMilliseconds_ = 0UL;
  runDurationMilliseconds_ = 0UL;
  writeRelay(false);

  // EEPROM bytes form a small versioned record; invalid values use defaults.
  if (EEPROM.read(kScheduleMagicAddress) == kScheduleMagic) {
    const uint8_t savedHour = EEPROM.read(kScheduleHourAddress);
    const uint8_t savedMinute = EEPROM.read(kScheduleMinuteAddress);
    if (savedHour < 24U && savedMinute < 60U) {
      scheduleHour_ = savedHour;
      scheduleMinute_ = savedMinute;
    }
  }
}

/** Starts a timed run and records the start time for non-blocking maintenance. */
bool Pump::startFor(uint32_t durationMilliseconds) {
  if (running_ || durationMilliseconds == 0UL) {
    return false;
  }

  startedAtMilliseconds_ = millis();
  runDurationMilliseconds_ = durationMilliseconds;
  running_ = true;
  writeRelay(true);
  return true;
}

/** Starts a manual run without an automatic stop time. */
bool Pump::start() {
  if (running_) {
    return false;
  }

  startedAtMilliseconds_ = 0UL;
  runDurationMilliseconds_ = 0UL;
  running_ = true;
  writeRelay(true);
  return true;
}

/** Forces the relay inactive and clears timing information immediately. */
void Pump::stop() {
  writeRelay(false);
  startedAtMilliseconds_ = 0UL;
  runDurationMilliseconds_ = 0UL;
  running_ = false;
}

/** Stops the pump when the configured elapsed duration has passed. */
void Pump::maintain() {
  if (!running_) {
    return;
  }

  // A zero duration identifies a manual run with no automatic stop.
  if (runDurationMilliseconds_ == 0UL) {
    return;
  }

  const uint32_t elapsedMilliseconds =
      millis() - startedAtMilliseconds_;
  if (elapsedMilliseconds >= runDurationMilliseconds_) {
    stop();
  }
}

/** Returns whether this module currently considers the relay energized. */
bool Pump::isRunning() const {
  return running_;
}

/** Loads the daily schedule or its defaults and begins setup at the hour field. */
void Pump::advanceScheduleSetup() {
  if (!scheduleSetupActive_) {
    scheduleSetupActive_ = true;
    scheduleSetupField_ = PumpScheduleField::Hour;
    return;
  }

  if (scheduleSetupField_ == PumpScheduleField::Hour) {
    scheduleSetupField_ = PumpScheduleField::Minute;
    return;
  }

  saveSchedule();
  scheduleSetupActive_ = false;
}

/** Restores and persists the factory default pump schedule of 12:30 PM. */
void Pump::resetScheduleToDefault() {
  scheduleHour_ = kDefaultScheduleHour;
  scheduleMinute_ = kDefaultScheduleMinute;
  scheduleSetupActive_ = false;
  saveSchedule();
}

/** Adjusts the active schedule field while wrapping within valid clock limits. */
void Pump::adjustSchedule(int delta) {
  if (!scheduleSetupActive_) {
    return;
  }

  if (scheduleSetupField_ == PumpScheduleField::Hour) {
    const int hour = (scheduleHour_ + delta + 24) % 24;
    scheduleHour_ = static_cast<uint8_t>(hour);
    return;
  }

  const int minute = (scheduleMinute_ + delta + 60) % 60;
  scheduleMinute_ = static_cast<uint8_t>(minute);
}

/** Returns whether buttons are currently changing a pump schedule field. */
bool Pump::isScheduleSetupActive() const {
  return scheduleSetupActive_;
}

/** Returns which schedule component is being adjusted. */
PumpScheduleField Pump::scheduleSetupField() const {
  return scheduleSetupField_;
}

/** Returns the saved schedule hour in 24-hour format. */
uint8_t Pump::scheduleHour() const {
  return scheduleHour_;
}

/** Returns the saved schedule minute. */
uint8_t Pump::scheduleMinute() const {
  return scheduleMinute_;
}

/** Starts a daily timed run once per IST calendar day at the saved minute. */
bool Pump::maintainSchedule(const Timekeeping::DateTime& indianStandardTime,
                            uint32_t durationMilliseconds) {
  const uint32_t today = calendarDayKey(indianStandardTime);
  const bool scheduledMinute =
      indianStandardTime.hour == scheduleHour_ &&
      indianStandardTime.minute == scheduleMinute_;

  if (!scheduledMinute || today == lastScheduledPumpRunDay_) {
    return false;
  }

  lastScheduledPumpRunDay_ = today;
  if (running_) {
    return false;
  }

  return startFor(durationMilliseconds);
}

/** Stores schedule values in EEPROM with the schedule validity marker. */
void Pump::saveSchedule() {
  EEPROM.write(kScheduleMagicAddress, kScheduleMagic);
  EEPROM.write(kScheduleHourAddress, scheduleHour_);
  EEPROM.write(kScheduleMinuteAddress, scheduleMinute_);
}

/** Applies relay active-high or active-low polarity to the output pin. */
void Pump::writeRelay(bool energized) {
  const bool relayPinIsHigh =
      relayLogic_ == RelayLogic::ActiveHigh ? energized : !energized;
  Gpio::DigitalGpio::write(relayPin_, relayPinIsHigh);
}

}  // namespace Irrigation
}  // namespace UnoR4
