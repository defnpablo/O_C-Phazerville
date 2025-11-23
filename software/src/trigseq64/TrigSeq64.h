#pragma once

#include <cstdint>
#include <bitset>

class TrigSeq64 {
public:
  static constexpr std::size_t MAX_STEPS = 64;
  static constexpr std::size_t PAGE_LENGHT = 16;
  static constexpr std::size_t PAGE_COUNT = 4;

  std::bitset<MAX_STEPS> steps;

  uint8_t page_cursor = 0;
  uint8_t step_cursor = 0;
  uint8_t playhead_cursor = 0;
  uint8_t end_cursor = 15;
};
