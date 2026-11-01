#pragma once

#include <stdint.h>

#include "UnoR4Gpio.hpp"

namespace UnoR4 {
namespace Input {

/** Detects one debounced press event for a normally-open, active-low button. */
class Button {
 public:
  Button(Gpio::DigitalPin pin, uint32_t debounceMilliseconds = 30UL);

  /** Configures the input pin and initializes its stable state. */
  void initialize();

  /**
   * Updates the button state and records a press event when a valid
   * active-low transition is detected.
   */
  void update();

  /** Returns true once and clears the press event after it is consumed. */
  bool wasPressed();

 private:
  Gpio::DigitalPin pin_;
  uint32_t debounceMilliseconds_;
  uint32_t candidateStartedAtMilliseconds_;
  bool rawPressed_;
  bool stablePressed_;
  bool pressPending_;
};

}  // namespace Input
}  // namespace UnoR4
