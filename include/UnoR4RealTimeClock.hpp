#pragma once

#include <Arduino.h>

#include <stdint.h>

namespace UnoR4 {
namespace Timekeeping {

/** Calendar fields stored by the board RTC. year uses the full four-digit value. */
struct DateTime {
  int year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
};

enum class ClockResult : uint8_t {
  Success,
  NotInitialized,
  InvalidDateTime,
  HardwareError,
};

/** Owns initialization and access to the Uno R4 on-board real-time clock. */
class RealTimeClock {
 public:
  RealTimeClock();

  /** Initializes the RTC hardware. Call once during setup(). */
  ClockResult initialize();

  /** Returns true after initialize() succeeds. */
  bool isInitialized() const;

  /** Returns true when the RTC is running and supplying a valid clock. */
  bool isRunning() const;

  /** Sets the clock from a validated calendar date and time. */
  ClockResult setDateTime(const DateTime& dateTime);

  /** Sets the clock from a Unix timestamp expressed as seconds since 1970-01-01 UTC. */
  ClockResult setUnixTime(time_t unixTime);

  /** Returns the current calendar date and time. */
  ClockResult getDateTime(DateTime& dateTime) const;

  /** Returns the current Unix timestamp, or zero with the returned status on failure. */
  ClockResult getUnixTime(time_t& unixTime) const;

  /** Returns a readable ISO 8601 value such as 2026-11-01T16:30:00. */
  String toIso8601() const;

 private:
  bool initialized_;
};

}  // namespace Timekeeping
}  // namespace UnoR4
