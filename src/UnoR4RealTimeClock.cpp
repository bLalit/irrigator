#include "UnoR4RealTimeClock.hpp"

#include <RTC.h>

namespace UnoR4 {
namespace Timekeeping {

namespace {

/** Returns true when year is divisible by four, excluding most century years. */
bool isLeapYear(int year) {
  return (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
}

/** Returns the number of days in a month, or zero for an invalid month. */
uint8_t daysInMonth(int year, uint8_t month) {
  switch (month) {
    case 2:
      return isLeapYear(year) ? 29 : 28;
    case 4:
    case 6:
    case 9:
    case 11:
      return 30;
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
      return 31;
    default:
      return 0;
  }
}

/** Validates the supported RTC date range and calendar field values. */
bool isValidDateTime(const DateTime& dateTime) {
  if (dateTime.year < 2000 || dateTime.year > 2099) {
    return false;
  }
  if (dateTime.month < 1 || dateTime.month > 12) {
    return false;
  }
  if (dateTime.day < 1 || dateTime.day > daysInMonth(dateTime.year, dateTime.month)) {
    return false;
  }
  if (dateTime.hour > 23 || dateTime.minute > 59 || dateTime.second > 59) {
    return false;
  }
  return true;
}

/** Converts the project month value to the Arduino RTC month enum. */
Month asArduinoMonth(uint8_t month) {
  return static_cast<Month>(month - 1);
}

/** Converts the Arduino RTC month enum to the project month value. */
uint8_t asUnoMonth(Month month) {
  return static_cast<uint8_t>(month) + 1;
}

/** Calculates the day of the week required by the Arduino RTC API. */
DayOfWeek calculateDayOfWeek(const DateTime& dateTime) {
  struct tm calendarTime = {};
  calendarTime.tm_year = dateTime.year - 1900;
  calendarTime.tm_mon = dateTime.month - 1;
  calendarTime.tm_mday = dateTime.day;
  calendarTime.tm_hour = dateTime.hour;
  calendarTime.tm_min = dateTime.minute;
  calendarTime.tm_sec = dateTime.second;

  mktime(&calendarTime);
  switch (calendarTime.tm_wday) {
    case 0:
      return DayOfWeek::SUNDAY;
    case 1:
      return DayOfWeek::MONDAY;
    case 2:
      return DayOfWeek::TUESDAY;
    case 3:
      return DayOfWeek::WEDNESDAY;
    case 4:
      return DayOfWeek::THURSDAY;
    case 5:
      return DayOfWeek::FRIDAY;
    default:
      return DayOfWeek::SATURDAY;
  }
}

}  // namespace

/** Creates an RTC wrapper in the uninitialized state. */
RealTimeClock::RealTimeClock()
    : initialized_(false) {
}

/** Initializes the on-board hardware clock and records whether it is ready. */
ClockResult RealTimeClock::initialize() {
  initialized_ = RTC.begin();
  return initialized_ ? ClockResult::Success : ClockResult::HardwareError;
}

/** Reports whether initialize() completed successfully. */
bool RealTimeClock::isInitialized() const {
  return initialized_;
}

/** Reports whether the initialized RTC hardware is actively running. */
bool RealTimeClock::isRunning() const {
  return initialized_ && RTC.isRunning();
}

/** Writes calendar fields to the RTC after validation and initialization checks. */
ClockResult RealTimeClock::setDateTime(const DateTime& dateTime) {
  if (!isValidDateTime(dateTime)) {
    return ClockResult::InvalidDateTime;
  }
  if (!isInitialized()) {
    return ClockResult::NotInitialized;
  }

  RTCTime rtcTime(dateTime.day,
                  asArduinoMonth(dateTime.month),
                  dateTime.year,
                  dateTime.hour,
                  dateTime.minute,
                  dateTime.second,
                  calculateDayOfWeek(dateTime),
                  SaveLight::SAVING_TIME_INACTIVE);

  return RTC.setTime(rtcTime) ? ClockResult::Success : ClockResult::HardwareError;
}

/** Writes a Unix timestamp to the RTC. Negative timestamps are rejected. */
ClockResult RealTimeClock::setUnixTime(time_t unixTime) {
  if (unixTime < 0) {
    return ClockResult::InvalidDateTime;
  }
  if (!isInitialized()) {
    return ClockResult::NotInitialized;
  }

  RTCTime rtcTime(unixTime);
  return RTC.setTime(rtcTime)
             ? ClockResult::Success
             : ClockResult::HardwareError;
}

/** Reads the current calendar fields from the RTC. */
ClockResult RealTimeClock::getDateTime(DateTime& dateTime) const {
  if (!isInitialized()) {
    return ClockResult::NotInitialized;
  }

  RTCTime rtcTime;
  if (!RTC.getTime(rtcTime)) {
    return ClockResult::HardwareError;
  }

  dateTime.year = rtcTime.getYear();
  dateTime.month = asUnoMonth(rtcTime.getMonth());
  dateTime.day = rtcTime.getDayOfMonth();
  dateTime.hour = rtcTime.getHour();
  dateTime.minute = rtcTime.getMinutes();
  dateTime.second = rtcTime.getSeconds();
  return ClockResult::Success;
}

/** Reads the current time as Unix seconds since 1970-01-01 UTC. */
ClockResult RealTimeClock::getUnixTime(time_t& unixTime) const {
  if (!isInitialized()) {
    unixTime = 0;
    return ClockResult::NotInitialized;
  }

  RTCTime rtcTime;
  if (!RTC.getTime(rtcTime)) {
    unixTime = 0;
    return ClockResult::HardwareError;
  }

  unixTime = rtcTime.getUnixTime();
  return unixTime >= 0 ? ClockResult::Success : ClockResult::HardwareError;
}

/** Formats the current RTC time as an ISO 8601 string, or an empty string on failure. */
String RealTimeClock::toIso8601() const {
  DateTime dateTime{};
  if (getDateTime(dateTime) != ClockResult::Success) {
    return String();
  }

  char value[20];
  snprintf(value, sizeof(value), "%04d-%02u-%02uT%02u:%02u:%02u",
           dateTime.year, dateTime.month, dateTime.day,
           dateTime.hour, dateTime.minute, dateTime.second);
  return String(value);
}

}  // namespace Timekeeping
}  // namespace UnoR4
