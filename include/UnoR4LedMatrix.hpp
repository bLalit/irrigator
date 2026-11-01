#pragma once

#include <stdint.h>

#include "UnoR4RealTimeClock.hpp"

namespace UnoR4 {
namespace Display {

/** Displays hours and minutes on the Uno R4 on-board 12 x 8 LED matrix. */
class LedMatrixClock {
 public:
  /** Initializes the matrix hardware. Returns false when initialization fails. */
  bool initialize();

  /** Displays the current IST hours and minutes from the supplied RTC. */
  bool displayCurrentTime(const Timekeeping::RealTimeClock& clock);

  /** Displays hours on the top row and minutes on the bottom row. */
  bool displayTime(uint8_t hour, uint8_t minute);

  /** Clears every LED on the matrix. */
  void clear();

 private:
  bool initialized_;
};

}  // namespace Display
}  // namespace UnoR4
