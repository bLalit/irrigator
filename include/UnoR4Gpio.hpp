#pragma once

#include <stdint.h>

namespace UnoR4 {
namespace Gpio {

enum class DigitalPin : uint8_t {
  D0 = 0,
  D1 = 1,
  D2 = 2,
  D3 = 3,
  D4 = 4,
  D5 = 5,
  D6 = 6,
  D7 = 7,
  D8 = 8,
  D9 = 9,
  D10 = 10,
  D11 = 11,
  D12 = 12,
  D13 = 13,
};

enum class AnalogInputPin : uint8_t {
  A0 = 14,
  A1 = 15,
  A2 = 16,
  A3 = 17,
  A4 = 18,
  A5 = 19,
};

enum class CommunicationPin : uint8_t {
  UartRxD0 = 0,
  UartTxD1 = 1,
  SpiMosiD11 = 11,
  SpiMisoD12 = 12,
  SpiClockD13 = 13,
  SpiChipSelectD10 = 10,
  I2cSdaA4 = 18,
  I2cSclA5 = 19,
};

enum class BuiltInOutputPin : uint8_t {
  LedD13 = 13,
};

class DigitalPins {
 public:
  static uint32_t mask(DigitalPin pin);
  static uint32_t mask(uint8_t arduinoPinNumber);
  static uint8_t number(DigitalPin pin);
  static bool isValid(uint8_t arduinoPinNumber);

  static constexpr uint32_t kAllPinsMask =
      (1UL << 0) | (1UL << 1) | (1UL << 2) | (1UL << 3) |
      (1UL << 4) | (1UL << 5) | (1UL << 6) | (1UL << 7) |
      (1UL << 8) | (1UL << 9) | (1UL << 10) | (1UL << 11) |
      (1UL << 12) | (1UL << 13);
};

class AnalogInputPins {
 public:
  static uint32_t mask(AnalogInputPin pin);
  static uint8_t number(AnalogInputPin pin);
  static uint8_t adcChannel(AnalogInputPin pin);

  static constexpr uint32_t kAllPinsMask =
      (1UL << 14) | (1UL << 15) | (1UL << 16) |
      (1UL << 17) | (1UL << 18) | (1UL << 19);
};

class CommunicationPins {
 public:
  static uint32_t mask(CommunicationPin pin);
  static uint8_t number(CommunicationPin pin);
};

class BuiltInOutputs {
 public:
  static uint32_t mask(BuiltInOutputPin pin);
  static uint8_t number(BuiltInOutputPin pin);
};

class DigitalGpio {
 public:
  static void configureAsInput(DigitalPin pin);
  static void configureAsInputWithPullup(DigitalPin pin);
  static void configureAsOutput(DigitalPin pin);
  static bool read(DigitalPin pin);
  static void writeHigh(DigitalPin pin);
  static void writeLow(DigitalPin pin);
  static void write(DigitalPin pin, bool high);
};

uint32_t pinMask(uint8_t arduinoPinNumber);
bool isArduinoPinValid(uint8_t arduinoPinNumber);

}  // namespace Gpio
}  // namespace UnoR4
