#include "UnoR4Gpio.hpp"

#include <Arduino.h>

namespace UnoR4 {
namespace Gpio {

namespace {

uint32_t bitMask(uint8_t arduinoPinNumber) {
  return pinMask(arduinoPinNumber);
}

}  // namespace

uint32_t pinMask(uint8_t arduinoPinNumber) {
  return isArduinoPinValid(arduinoPinNumber)
             ? (1UL << arduinoPinNumber)
             : 0UL;
}

bool isArduinoPinValid(uint8_t arduinoPinNumber) {
  return arduinoPinNumber <= 19;
}

uint32_t DigitalPins::mask(DigitalPin pin) {
  return bitMask(number(pin));
}

uint32_t DigitalPins::mask(uint8_t arduinoPinNumber) {
  return arduinoPinNumber <= 13 ? bitMask(arduinoPinNumber) : 0UL;
}

uint8_t DigitalPins::number(DigitalPin pin) {
  return static_cast<uint8_t>(pin);
}

bool DigitalPins::isValid(uint8_t arduinoPinNumber) {
  return arduinoPinNumber <= 13;
}

uint32_t AnalogInputPins::mask(AnalogInputPin pin) {
  return bitMask(number(pin));
}

uint8_t AnalogInputPins::number(AnalogInputPin pin) {
  return static_cast<uint8_t>(pin);
}

uint8_t AnalogInputPins::adcChannel(AnalogInputPin pin) {
  return number(pin) - 14;
}

uint32_t CommunicationPins::mask(CommunicationPin pin) {
  return bitMask(number(pin));
}

uint8_t CommunicationPins::number(CommunicationPin pin) {
  return static_cast<uint8_t>(pin);
}

uint32_t BuiltInOutputs::mask(BuiltInOutputPin pin) {
  return bitMask(number(pin));
}

uint8_t BuiltInOutputs::number(BuiltInOutputPin pin) {
  return static_cast<uint8_t>(pin);
}

void DigitalGpio::configureAsInput(DigitalPin pin) {
  pinMode(DigitalPins::number(pin), INPUT);
}

void DigitalGpio::configureAsInputWithPullup(DigitalPin pin) {
  pinMode(DigitalPins::number(pin), INPUT_PULLUP);
}

void DigitalGpio::configureAsOutput(DigitalPin pin) {
  pinMode(DigitalPins::number(pin), OUTPUT);
}

bool DigitalGpio::read(DigitalPin pin) {
  return digitalRead(DigitalPins::number(pin)) == HIGH;
}

void DigitalGpio::writeHigh(DigitalPin pin) {
  digitalWrite(DigitalPins::number(pin), HIGH);
}

void DigitalGpio::writeLow(DigitalPin pin) {
  digitalWrite(DigitalPins::number(pin), LOW);
}

void DigitalGpio::write(DigitalPin pin, bool high) {
  high ? writeHigh(pin) : writeLow(pin);
}

}  // namespace Gpio
}  // namespace UnoR4
