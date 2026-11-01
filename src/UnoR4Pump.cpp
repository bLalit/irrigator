#include "UnoR4Pump.hpp"

#include <Arduino.h>

namespace UnoR4 {
namespace Irrigation {

namespace {

constexpr uint32_t kFiveSecondsMilliseconds = 5000UL;

}  // namespace

/** Stores relay configuration and begins in the stopped state. */
Pump::Pump(Gpio::DigitalPin relayPin, RelayLogic relayLogic)
    : relayPin_(relayPin),
      relayLogic_(relayLogic),
      startedAtMilliseconds_(0UL),
      runDurationMilliseconds_(0UL),
      running_(false) {
}

/** Configures the relay pin as output before ensuring the relay is inactive. */
void Pump::initialize() {
  Gpio::DigitalGpio::configureAsOutput(relayPin_);
  running_ = false;
  startedAtMilliseconds_ = 0UL;
  runDurationMilliseconds_ = 0UL;
  writeRelay(false);
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

/** Applies relay active-high or active-low polarity to the output pin. */
void Pump::writeRelay(bool energized) {
  const bool relayPinIsHigh =
      relayLogic_ == RelayLogic::ActiveHigh ? energized : !energized;
  Gpio::DigitalGpio::write(relayPin_, relayPinIsHigh);
}

}  // namespace Irrigation
}  // namespace UnoR4
