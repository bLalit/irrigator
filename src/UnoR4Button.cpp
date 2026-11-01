#include "UnoR4Button.hpp"

#include <Arduino.h>

namespace UnoR4 {
namespace Input {

namespace {

/** Returns elapsed time while correctly handling an Arduino millis() wrap. */
uint32_t elapsedMilliseconds(uint32_t startMilliseconds) {
  return millis() - startMilliseconds;
}

}  // namespace

/** Stores button timing configuration and starts in the released state. */
Button::Button(Gpio::DigitalPin pin, uint32_t debounceMilliseconds)
    : pin_(pin),
      debounceMilliseconds_(debounceMilliseconds),
      candidateStartedAtMilliseconds_(0UL),
      rawPressed_(false),
      stablePressed_(false),
      pressPending_(false) {
}

/** Uses the internal pull-up so the unpressed level remains HIGH. */
void Button::initialize() {
  Gpio::DigitalGpio::configureAsInputWithPullup(pin_);
  rawPressed_ = !Gpio::DigitalGpio::read(pin_);
  stablePressed_ = rawPressed_;
  candidateStartedAtMilliseconds_ = millis();
  pressPending_ = false;
}

/** Debounces the raw level and emits an event on a stable press transition. */
void Button::update() {
  const bool newlyReadPressed = !Gpio::DigitalGpio::read(pin_);
  const uint32_t nowMilliseconds = millis();

  if (newlyReadPressed != rawPressed_) {
    rawPressed_ = newlyReadPressed;
    candidateStartedAtMilliseconds_ = nowMilliseconds;
    return;
  }

  if (rawPressed_ == stablePressed_ ||
      elapsedMilliseconds(candidateStartedAtMilliseconds_) <
          debounceMilliseconds_) {
    return;
  }

  stablePressed_ = rawPressed_;
  if (stablePressed_) {
    pressPending_ = true;
  }
}

/** Consumes the pending press event so each physical press is reported once. */
bool Button::wasPressed() {
  if (!pressPending_) {
    return false;
  }

  pressPending_ = false;
  return true;
}

}  // namespace Input
}  // namespace UnoR4
