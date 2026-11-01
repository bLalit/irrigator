#pragma once

#include <stdint.h>

#include "UnoR4Gpio.hpp"

namespace UnoR4 {
namespace Irrigation {

enum class RelayLogic : uint8_t {
  ActiveHigh,
  ActiveLow,
};

/** Controls a pump through a relay without blocking the Arduino loop. */
class Pump {
 public:
  Pump(Gpio::DigitalPin relayPin, RelayLogic relayLogic = RelayLogic::ActiveLow);

  /** Configures the relay pin and returns it to the inactive state. */
  void initialize();

  /** Starts the pump for a caller-selected duration without blocking. */
  bool startFor(uint32_t durationMilliseconds);

  /** Stops the pump immediately and clears the active run timer. */
  void stop();

  /** Updates relay state and must be called from loop(). */
  void maintain();

  /** Returns true while the pump relay is commanded on. */
  bool isRunning() const;

 private:
  void writeRelay(bool energized);

  Gpio::DigitalPin relayPin_;
  RelayLogic relayLogic_;
  uint32_t startedAtMilliseconds_;
  uint32_t runDurationMilliseconds_;
  bool running_;
};

}  // namespace Irrigation
}  // namespace UnoR4
