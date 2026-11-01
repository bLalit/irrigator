#include "UnoR4LedMatrix.hpp"

#include <Arduino_LED_Matrix.h>

namespace UnoR4 {
namespace Display {

namespace {

constexpr uint8_t kRows = 8;
constexpr uint8_t kColumns = 12;
constexpr uint8_t kDigitColumns = 3;
constexpr uint8_t kDigitRows = 4;

// Digits are rendered as compact 3-column by 4-row patterns.
constexpr uint8_t kDigitZero[kDigitRows][kDigitColumns] = {
    {1, 1, 1}, {1, 0, 1}, {1, 0, 1}, {1, 1, 1}};
constexpr uint8_t kDigitOne[kDigitRows][kDigitColumns] = {
    {0, 1, 0}, {1, 1, 0}, {0, 1, 0}, {1, 1, 1}};
constexpr uint8_t kDigitTwo[kDigitRows][kDigitColumns] = {
    {1, 1, 1}, {0, 0, 1}, {1, 1, 0}, {1, 1, 1}};
constexpr uint8_t kDigitThree[kDigitRows][kDigitColumns] = {
    {1, 1, 1}, {0, 1, 1}, {0, 0, 1}, {1, 1, 1}};
constexpr uint8_t kDigitFour[kDigitRows][kDigitColumns] = {
    {1, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 0, 1}};
constexpr uint8_t kDigitFive[kDigitRows][kDigitColumns] = {
    {1, 1, 1}, {1, 1, 0}, {0, 0, 1}, {1, 1, 1}};
constexpr uint8_t kDigitSix[kDigitRows][kDigitColumns] = {
    {1, 1, 0}, {1, 1, 1}, {1, 0, 1}, {1, 1, 1}};
constexpr uint8_t kDigitSeven[kDigitRows][kDigitColumns] = {
    {1, 1, 1}, {0, 0, 1}, {0, 1, 0}, {0, 1, 0}};
constexpr uint8_t kDigitEight[kDigitRows][kDigitColumns] = {
    {1, 1, 1}, {1, 1, 1}, {1, 0, 1}, {1, 1, 1}};
constexpr uint8_t kDigitNine[kDigitRows][kDigitColumns] = {
    {1, 1, 1}, {1, 0, 1}, {1, 1, 1}, {0, 0, 1}};

ArduinoLEDMatrix matrix;

/** Returns the glyph for one decimal digit. */
const uint8_t (*digitGlyph(uint8_t digit))[kDigitColumns] {
  switch (digit) {
    case 1: return kDigitOne;
    case 2: return kDigitTwo;
    case 3: return kDigitThree;
    case 4: return kDigitFour;
    case 5: return kDigitFive;
    case 6: return kDigitSix;
    case 7: return kDigitSeven;
    case 8: return kDigitEight;
    case 9: return kDigitNine;
    default: return kDigitZero;
  }
}

/** Draws one digit pair centered horizontally on the selected matrix row. */
void drawDigitPair(uint8_t (&frame)[kRows][kColumns],
                   uint8_t value,
                   uint8_t topRow) {
  const uint8_t tens = value / 10U;
  const uint8_t ones = value % 10U;
  const uint8_t (*leftGlyph)[kDigitColumns] = digitGlyph(tens);
  const uint8_t (*rightGlyph)[kDigitColumns] = digitGlyph(ones);

  for (uint8_t row = 0; row < kDigitRows; ++row) {
    for (uint8_t column = 0; column < kDigitColumns; ++column) {
      frame[topRow + row][1 + column] = leftGlyph[row][column];
      frame[topRow + row][8 + column] = rightGlyph[row][column];
    }
  }
}

}  // namespace

/** Initializes the hardware matrix and verifies successful startup. */
bool LedMatrixClock::initialize() {
  initialized_ = matrix.begin();
  if (initialized_) {
    matrix.clear();
  }
  return initialized_;
}

/** Reads IST from the RTC and displays only its hour and minute fields. */
bool LedMatrixClock::displayCurrentTime(const Timekeeping::RealTimeClock& clock) {
  Timekeeping::DateTime currentTime{};
  if (clock.getIndianStandardTime(currentTime) != Timekeeping::ClockResult::Success) {
    return false;
  }
  if (currentTime.year < 2020) {
    clear();
    return false;
  }
  return displayTime(currentTime.hour, currentTime.minute);
}

/** Validates the time, builds a fresh bitmap, and renders it on the matrix. */
bool LedMatrixClock::displayTime(uint8_t hour, uint8_t minute) {
  if (!initialized_ || hour > 23U || minute > 59U) {
    return false;
  }

  uint8_t frame[kRows][kColumns] = {};
  drawDigitPair(frame, hour, 0U);
  drawDigitPair(frame, minute, 4U);
  matrix.renderBitmap(frame, kRows, kColumns);
  return true;
}

/** Clears the matrix when it has been initialized. */
void LedMatrixClock::clear() {
  if (initialized_) {
    matrix.clear();
  }
}

}  // namespace Display
}  // namespace UnoR4
