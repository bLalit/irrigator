#pragma once

#include <stdint.h>

#include "UnoR4Gpio.hpp"
#include "UnoR4RealTimeClock.hpp"

namespace UnoR4 {
namespace Irrigation {

enum class RelayLogic : uint8_t {
  ActiveHigh,
  ActiveLow,
};

enum class PumpScheduleField : uint8_t {
  Hour,
  Minute,
};

/** Controls a pump through a relay without blocking the Arduino loop. */
class Pump {
 public:
  Pump(Gpio::DigitalPin relayPin, RelayLogic relayLogic = RelayLogic::ActiveLow);

  /** Configures the relay pin and returns it to the inactive state. */
  void initialize();

  /** Starts the pump for a caller-selected duration without blocking. */
  bool startFor(uint32_t durationMilliseconds);

  /** Starts the pump until stop() is called. */
  bool start();

  /** Stops the pump immediately and clears the active run timer. */
  void stop();

  /** Updates relay state and must be called from loop(). */
  void maintain();

  /** Starts or advances schedule setup: hour, minute, then save. */
  void advanceScheduleSetup();

  /** Changes the currently selected hour or minute schedule field by delta. */
  void adjustSchedule(int delta);

  /** Restores 12:30 PM and saves it as the persistent pump schedule. */
  void resetScheduleToDefault();

  /** Returns true while the user schedule setup sequence is active. */
  bool isScheduleSetupActive() const;

  /** Returns the schedule field currently available for adjustment. */
  PumpScheduleField scheduleSetupField() const;

  /** Returns the configured schedule hour in 24-hour IST format. */
  uint8_t scheduleHour() const;

  /** Returns the configured schedule minute. */
  uint8_t scheduleMinute() const;

  /**
   * Starts one timed pump run when the supplied IST time matches the saved
   * schedule. The same calendar day will not trigger more than once.
   */
  bool maintainSchedule(const Timekeeping::DateTime& indianStandardTime,
                        uint32_t durationMilliseconds);

  /** Returns true while the pump relay is commanded on. */
  bool isRunning() const;

 private:
  void writeRelay(bool energized);
  void saveSchedule();

  Gpio::DigitalPin relayPin_;
  RelayLogic relayLogic_;
  uint32_t startedAtMilliseconds_;
  uint32_t runDurationMilliseconds_;
  bool running_;
  uint8_t scheduleHour_;
  uint8_t scheduleMinute_;
  uint32_t lastScheduledPumpRunDay_;
  bool scheduleSetupActive_;
  PumpScheduleField scheduleSetupField_;
};

}  // namespace Irrigation
}  // namespace UnoR4
