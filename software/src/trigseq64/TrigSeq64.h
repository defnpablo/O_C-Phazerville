#pragma once

#include <cstdint>
#include <bitset>

class TrigSeq64 {
public:
  static constexpr std::size_t MAX_STEPS   = 64;
  static constexpr std::size_t PAGE_LENGTH = 16;
  static constexpr std::size_t PAGE_COUNT  = 4;

  const std::bitset<MAX_STEPS>& steps() const { return steps_; }
  uint8_t page_cursor()                 const { return page_cursor_; }
  uint8_t step_cursor()                 const { return step_cursor_; }
  uint8_t playhead_cursor()             const { return playhead_cursor_; }
  uint8_t end_cursor()                  const { return end_cursor_; }

private:
  std::bitset<MAX_STEPS> steps_{};

  uint8_t page_cursor_     = 0;
  uint8_t step_cursor_     = 0;
  uint8_t playhead_cursor_ = 0;
  uint8_t end_cursor_      = 15;
};