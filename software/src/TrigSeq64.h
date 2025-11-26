#pragma once

#include <cstdint>
#include <bitset>
#include <array>

class TrigSeq64 {
public:
  static constexpr std::size_t MAX_STEPS   = 64;
  static constexpr std::size_t PAGE_LENGTH = 16;
  static constexpr std::size_t PAGE_COUNT  = 4;
  
  static float min_probability() { return 0.1f; }
  static float max_probability() { return 1.0f; }
  static float probability_increment() { return 0.1f; }

  TrigSeq64()
  : steps_()
  , page_cursor_(0)
  , step_cursor_(0)
  , playhead_cursor_(0)
  , end_cursor_(15)
  {
    probabilities_.fill(max_probability());
  }

  TrigSeq64(const std::bitset<MAX_STEPS>& steps,
            uint8_t playhead_cursor,
            uint8_t step_cursor,
            uint8_t end_cursor,
            uint8_t page_cursor)
  : steps_(steps)
  , page_cursor_(page_cursor)
  , step_cursor_(step_cursor)
  , playhead_cursor_(playhead_cursor)
  , end_cursor_(end_cursor)
  {
    probabilities_.fill(max_probability());
  }

  TrigSeq64(const std::bitset<MAX_STEPS>& steps,
            uint8_t playhead_cursor,
            uint8_t step_cursor,
            uint8_t end_cursor,
            uint8_t page_cursor,
            const std::array<float, MAX_STEPS>& probabilities)
  : steps_(steps)
  , page_cursor_(page_cursor)
  , step_cursor_(step_cursor)
  , playhead_cursor_(playhead_cursor)
  , end_cursor_(end_cursor)
  , probabilities_(probabilities)
  {}

  const std::bitset<MAX_STEPS>& steps() const { return steps_; }
  uint8_t page_cursor()                 const { return page_cursor_; }
  uint8_t step_cursor()                 const { return step_cursor_; }
  uint8_t playhead_cursor()             const { return playhead_cursor_; }
  uint8_t end_cursor()                  const { return end_cursor_; }
  
  float get_step_probability(uint8_t step_index) const {
    return probabilities_[step_index];
  }
  
  float get_step_cursor_probability() const {
    return probabilities_[step_cursor_];
  }

  uint8_t get_step_cursor_page()        const { return step_cursor_ / PAGE_LENGTH; }
  uint8_t get_playhead_page()           const { return playhead_cursor_ / PAGE_LENGTH; }
  uint8_t get_end_cursor_page()         const { return end_cursor_ / PAGE_LENGTH; }

  bool is_playhead_step_on()            const { return steps_[playhead_cursor_]; }

  void reset_playhead() {
    playhead_cursor_ = 0;
  }

  void advance_playhead() {
    if (playhead_cursor_ >= end_cursor_) {
      reset_playhead();
    } else {
      ++playhead_cursor_;
    }
  }

  void advance_page() {
    ++page_cursor_;
    if (page_cursor_ >= PAGE_COUNT) {
      page_cursor_ = 0;
    }
  }

  void retreat_page() {
    if (page_cursor_ == 0) {
      page_cursor_ = PAGE_COUNT - 1;
    } else {
      --page_cursor_;
    }
  }

  void advance_end_cursor() {
    if (end_cursor_ < static_cast<uint8_t>(MAX_STEPS - 1)) {
      ++end_cursor_;
      // Update page if end cursor moved to a different page
      page_cursor_ = get_end_cursor_page();
    }
  }

  void retreat_end_cursor() {
    if (end_cursor_ > 0) {
      --end_cursor_;
      // Update page if end cursor moved to a different page
      page_cursor_ = get_end_cursor_page();
    }
  }

  void set_end_cursor_to_page_end() {
    end_cursor_ = page_cursor_ * PAGE_LENGTH + PAGE_LENGTH - 1;
  }

  void advance_step_cursor() {
    uint8_t page_start = page_cursor_ * PAGE_LENGTH;
    uint8_t page_end = page_start + PAGE_LENGTH - 1;
    
    // If step cursor is outside current page, teleport to first step of current page
    if (step_cursor_ < page_start || step_cursor_ > page_end) {
      step_cursor_ = page_start;
    } else if (step_cursor_ >= page_end) {
      step_cursor_ = page_start;
    } else {
      ++step_cursor_;
    }
  }

  void retreat_step_cursor() {
    uint8_t page_start = page_cursor_ * PAGE_LENGTH;
    uint8_t page_end = page_start + PAGE_LENGTH - 1;
    
    // If step cursor is outside current page, teleport to last step of current page
    if (step_cursor_ < page_start || step_cursor_ > page_end) {
      step_cursor_ = page_end;
    } else if (step_cursor_ <= page_start) {
      step_cursor_ = page_end;
    } else {
      --step_cursor_;
    }
  }

  void toggle_step_cursor() {
    steps_.flip(step_cursor_);
  }

  void clear_page() {
    uint8_t page_start = page_cursor_ * PAGE_LENGTH;
    for (uint8_t i = 0; i < PAGE_LENGTH; ++i) {
      steps_.reset(page_start + i);
    }
  }

  void fill_page() {
    uint8_t page_start = page_cursor_ * PAGE_LENGTH;
    for (uint8_t i = 0; i < PAGE_LENGTH; ++i) {
      steps_.set(page_start + i);
    }
  }

  void increase_step_cursor_probability() {
    if (probabilities_[step_cursor_] < max_probability()) {
      probabilities_[step_cursor_] += probability_increment();
    }
  }

  void decrease_step_cursor_probability() {
    if (probabilities_[step_cursor_] > min_probability()) {
      probabilities_[step_cursor_] -= probability_increment();
      // Clamp to minimum to avoid floating point precision issues
      if (probabilities_[step_cursor_] < min_probability()) {
        probabilities_[step_cursor_] = min_probability();
      }
    }
  }

  void reset_step_cursor_probability() {
    probabilities_[step_cursor_] = max_probability();
  }

  void set_page_probabilities_max() {
    uint8_t page_start = page_cursor_ * PAGE_LENGTH;
    for (uint8_t i = 0; i < PAGE_LENGTH; ++i) {
      probabilities_[page_start + i] = max_probability();
    }
  }

  void set_page_probabilities_min() {
    uint8_t page_start = page_cursor_ * PAGE_LENGTH;
    for (uint8_t i = 0; i < PAGE_LENGTH; ++i) {
      probabilities_[page_start + i] = min_probability();
    }
  }

private:
  std::bitset<MAX_STEPS> steps_;
  uint8_t page_cursor_;
  uint8_t step_cursor_;
  uint8_t playhead_cursor_;
  uint8_t end_cursor_;
  std::array<float, MAX_STEPS> probabilities_;
};