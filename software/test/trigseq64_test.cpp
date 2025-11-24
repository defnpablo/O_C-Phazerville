#include <cstdint>
#include <bitset>

class TrigSeq64 {
public:
  static constexpr std::size_t MAX_STEPS   = 64;
  static constexpr std::size_t PAGE_LENGTH = 16;
  static constexpr std::size_t PAGE_COUNT  = 4;

  TrigSeq64()
  : steps_()
  , page_cursor_(0)
  , step_cursor_(0)
  , playhead_cursor_(0)
  , end_cursor_(15)
  {}

  TrigSeq64(const std::bitset<MAX_STEPS>& steps,
            uint8_t page_cursor,
            uint8_t step_cursor,
            uint8_t playhead_cursor,
            uint8_t end_cursor)
  : steps_(steps)
  , page_cursor_(page_cursor)
  , step_cursor_(step_cursor)
  , playhead_cursor_(playhead_cursor)
  , end_cursor_(end_cursor)
  {}

  const std::bitset<MAX_STEPS>& steps() const { return steps_; }
  uint8_t page_cursor()                 const { return page_cursor_; }
  uint8_t step_cursor()                 const { return step_cursor_; }
  uint8_t playhead_cursor()             const { return playhead_cursor_; }
  uint8_t end_cursor()                  const { return end_cursor_; }

  void reset() {
    playhead_cursor_ = 0;
  }

  void advance_step() {
    if (playhead_cursor_ >= end_cursor_) {
      reset();
    } else {
      ++playhead_cursor_;
    }
  }

private:
  std::bitset<MAX_STEPS> steps_;
  uint8_t page_cursor_;
  uint8_t step_cursor_;
  uint8_t playhead_cursor_;
  uint8_t end_cursor_;
};