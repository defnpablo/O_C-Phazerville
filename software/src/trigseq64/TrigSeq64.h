#pragma once

#include <cstdint>
#include <bitset>

// Data-only class for a 64-step trigger sequencer.
// - `steps` holds 64 boolean steps in a fixed-size bitset (step 0 = index 0).
// - Separate fields store page, selected step, sequence end, and playhead.
class TrigSeq64 {
public:
  static constexpr std::size_t MAX_STEPS = 64;
  static constexpr std::size_t PAGE_LENGHT = 16;
  static constexpr std::size_t PAGE_COUNT = 4;

  std::bitset<MAX_STEPS> steps;

  uint8_t selected_page = 0;
  uint8_t selected_step = 0;
  uint8_t sequence_end = 16;
  uint8_t playhead = 0;
};
